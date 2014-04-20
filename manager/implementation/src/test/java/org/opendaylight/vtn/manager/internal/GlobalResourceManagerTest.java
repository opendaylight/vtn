/*
 * Copyright (c) 2013-2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal;

import java.net.InetAddress;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Hashtable;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Properties;
import java.util.Set;
import java.util.Timer;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ConcurrentMap;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;

import javax.transaction.HeuristicMixedException;
import javax.transaction.HeuristicRollbackException;
import javax.transaction.NotSupportedException;
import javax.transaction.RollbackException;
import javax.transaction.SystemException;
import javax.transaction.Transaction;

import org.apache.felix.dm.impl.ComponentImpl;

import org.junit.After;
import org.junit.Test;

import org.opendaylight.controller.clustering.services.CacheConfigException;
import org.opendaylight.controller.clustering.services.CacheExistException;
import org.opendaylight.controller.clustering.services.IClusterGlobalServices;
import org.opendaylight.controller.clustering.services.IClusterServices.cacheMode;
import org.opendaylight.controller.sal.core.Node;
import org.opendaylight.controller.sal.core.NodeConnector;
import org.opendaylight.controller.sal.utils.NodeConnectorCreator;
import org.opendaylight.controller.sal.utils.NodeCreator;
import org.opendaylight.controller.sal.utils.StatusCode;
import org.opendaylight.vtn.manager.MacAddressEntry;
import org.opendaylight.vtn.manager.VBridgeConfig;
import org.opendaylight.vtn.manager.VBridgeIfPath;
import org.opendaylight.vtn.manager.VBridgePath;
import org.opendaylight.vtn.manager.VTNException;
import org.opendaylight.vtn.manager.VTenantConfig;
import org.opendaylight.vtn.manager.VTenantPath;
import org.opendaylight.vtn.manager.internal.cluster.ClusterEventId;
import org.opendaylight.vtn.manager.internal.cluster.MacTableEntry;
import org.opendaylight.vtn.manager.internal.cluster.MacTableEntryId;
import org.opendaylight.vtn.manager.internal.cluster.MapReference;
import org.opendaylight.vtn.manager.internal.cluster.MapType;
import org.opendaylight.vtn.manager.internal.cluster.NodeVlan;
import org.opendaylight.vtn.manager.internal.cluster.PortVlan;
import org.opendaylight.vtn.manager.internal.cluster.VlanMapPath;

/**
 * JUnit test for {@link GlobalResourceManager}.
 */
public class GlobalResourceManagerTest extends TestBase {
    /**
     * Pseudo node identifier which indicates that the node is unspecified.
     */
    private static final String  NODEID_ANY = "ANY";

    /**
     * Initial revision number of the global configuration.
     */
    private static final int  CONFIG_REV_INIT = 0;

    /**
     * The name of the cluster cache which keeps revision identifier of
     * configuration per mapping type.
     */
    private static final String CACHE_CONFREVISION = "vtn.confrev";

    /**
     * Cluster cache name associated with
     * {@link GlobalResourceManager#vlanMaps}.
     */
    private static final String  CACHE_VLANMAP = "vtn.vlanmap";

    /**
     * Cluster cache name associated with
     * {@link GlobalResourceManager#portMaps}.
     */
    private static final String  CACHE_PORTMAP = "vtn.portmap";

    /**
     * A {@link GlobalResourceManager} instance.
     */
    private GlobalResourceManager  resourceManager;

    /**
     * Stub class of {@link IClusterGlobalServices}.
     * Some methods in this class always return null.
     */
    class ClusterServiceStub implements IClusterGlobalServices {
        private ConcurrentMap<String, ConcurrentMap<?, ?>> caches
            = new ConcurrentHashMap<String, ConcurrentMap<?, ?>>();

        @Override
        public ConcurrentMap<?, ?> createCache(String cacheName,
                Set<cacheMode> cMode) throws CacheExistException,
                CacheConfigException {
            ConcurrentMap<?, ?> res = this.caches.get(cacheName);
            if (res == null) {
                res = new ConcurrentHashMap<Object, Object>();
                this.caches.put(cacheName, res);
                return res;
            }
            throw new CacheExistException();
        }

        @Override
        public ConcurrentMap<?, ?> getCache(String cacheName) {
            return this.caches.get(cacheName);
        }

        @Override
        public void destroyCache(String cacheName) {
            this.caches.remove(cacheName);
        }

        @Override
        public boolean existCache(String cacheName) {
            return (this.caches.get(cacheName) != null);
        }

        @Override
        public Set<String> getCacheList() {
            return this.caches.keySet();
        }

        @Override
        public Properties getCacheProperties(String cacheName) {
            return null;
        }

        @Override
        public void tbegin() throws NotSupportedException, SystemException {

        }

        @Override
        public void tbegin(long timeout, TimeUnit unit)
            throws NotSupportedException, SystemException {
        }

        @Override
        public void tcommit() throws RollbackException,
                HeuristicMixedException, HeuristicRollbackException,
                SecurityException, IllegalStateException, SystemException {

        }

        @Override
        public void trollback() throws IllegalStateException,
                SecurityException, SystemException {
        }

        @Override
        public Transaction tgetTransaction() throws SystemException {
            return null;
        }

        @Override
        public InetAddress getCoordinatorAddress() {
            return null;
        }

        @Override
        public List<InetAddress> getClusteredControllers() {
            return null;
        }

        @Override
        public InetAddress getMyAddress() {
            return null;
        }

        @Override
        public boolean amICoordinator() {
            return true;
        }

        @Override
        public void removeContainerCaches(String containerName) {

        }
    }

    /**
     * Tear down the test environment.
     */
    @After
    public void tearDown() {
        GlobalResourceManager grsc = resourceManager;
        if (grsc != null) {
            resourceManager = null;
            grsc.destroy();
        }
    }

    /**
     * Test method for
     * {@link GlobalResourceManager#init(org.apache.felix.dm.Component)},
     * {@link GlobalResourceManager#destroy()},
     * {@link GlobalResourceManager#getTimer()}.
     */
    @Test
    public void testInitDestroy() {
        ComponentImpl c = new ComponentImpl(null, null, null);
        GlobalResourceManager grsc = new GlobalResourceManager();
        TestStub stubObj = new TestStub(0);
        IClusterGlobalServices cs = (IClusterGlobalServices) stubObj;

        Hashtable<String, String> properties = new Hashtable<String, String>();
        properties.put("containerName", "default");
        c.setServiceProperties(properties);

        grsc.destroy();
        grsc.init(c);

        ConcurrentMap<Short, String> vmap
            = (ConcurrentMap<Short, String>) cs.getCache(CACHE_VLANMAP);
        assertNull(vmap);

        ConcurrentMap<Short, String> pmap
            = (ConcurrentMap<Short, String>) cs.getCache(CACHE_PORTMAP);
        assertNull(pmap);

        InetAddress loopback = InetAddress.getLoopbackAddress();
        ClusterEventId evid = new  ClusterEventId();
        assertEquals(loopback, evid.getControllerAddress());

        grsc.destroy();


        List<IClusterGlobalServices> csSet = new ArrayList<IClusterGlobalServices>();

        // set TestStub to cluster global service.
        csSet.add(stubObj);

        // in case IP address of controller is not loopback address.
        TestStubCluster cls = new TestStubCluster(2);
        csSet.add(cls);

        // in case methods of cluster service return null.
        ClusterServiceStub clsnull = new ClusterServiceStub();
        csSet.add(clsnull);

        for (IClusterGlobalServices service : csSet) {
            grsc.setClusterGlobalService(service);
            grsc.init(c);

            Timer tm = grsc.getTimer();
            assertNotNull(tm);

            vmap = (ConcurrentMap<Short, String>) cs.getCache(CACHE_VLANMAP);
            assertNotNull(vmap);
            assertEquals(0, vmap.size());

            pmap = (ConcurrentMap<Short, String>) cs.getCache(CACHE_PORTMAP);
            assertNotNull(pmap);
            assertEquals(0, pmap.size());

            evid = new  ClusterEventId();
            if (service.getMyAddress() != null
                    && !service.getMyAddress().equals(loopback)) {
                assertEquals(service.getMyAddress(), evid.getControllerAddress());
            } else {
                assertEquals(loopback, evid.getControllerAddress());
            }

            grsc.destroy();
        }
    }

    /**
     * Test method for
     * {@link GlobalResourceManager#setClusterGlobalService(IClusterGlobalServices)},
     * {@link GlobalResourceManager#unsetClusterGlobalService(IClusterGlobalServices)}.
     */
    @Test
    public void testSetUnsetClusterGlobalService() {
        ComponentImpl c = new ComponentImpl(null, null, null);
        GlobalResourceManager grsc = new GlobalResourceManager();
        TestStub stubObj = new TestStub(0);
        TestStub stubObj2 = new TestStub(0);

        Hashtable<String, String> properties = new Hashtable<String, String>();
        properties.put("containerName", "default");
        c.setServiceProperties(properties);

        grsc.setClusterGlobalService(stubObj);

        grsc.unsetClusterGlobalService(stubObj2);

        grsc.unsetClusterGlobalService(stubObj);
    }

    /**
     * Test method for
     * {@link GlobalResourceManager#registerVlanMap(VTNManagerImpl,VlanMapPath,NodeVlan,boolean)},
     * {@link GlobalResourceManager#unregisterVlanMap(VTNManagerImpl,VlanMapPath,NodeVlan,boolean)}.
     */
    @Test
    public void testRegisterVlanMap() {
        List<String> containerNames = new ArrayList<String>();
        containerNames.add("default");
        containerNames.add("tenant");

        HashMap<NodeVlan, VlanMapPath> mappings =
            new HashMap<NodeVlan, VlanMapPath>();
        TestStub stubObj = new TestStub(0);
        IClusterGlobalServices cs = (IClusterGlobalServices)stubObj;
        GlobalResourceManager grsc = setupGlobalResourceManager(stubObj);
        String other = "other";
        VTNManagerImpl otherMgr = setupVTNManager(grsc, stubObj, other);

        Node node0 = NodeCreator.createOFNode(Long.valueOf(0L));
        Node node1 = NodeCreator.createOFNode(Long.valueOf(1L));
        ConfRevisionMap revMap = getRevisionCache(cs);
        int updateFailure = 0;

        String tname = "tenant";
        String bname = "bridge";
        VBridgePath bpath = new VBridgePath(tname, bname);
        VlanMapPath invalidPath = new VlanMapPath(bpath, "unknown");

        for (String containerName : containerNames) {
            VTNManagerImpl mgr = setupVTNManager(grsc, stubObj, containerName);

            int revision = getRevision(revMap);
            short vlan = 0;
            NodeVlan nvlan0 = new NodeVlan(null, vlan);
            String mapId = createVlanMapId(nvlan0);
            VlanMapPath vpath = new VlanMapPath(bpath, mapId);
            MapReference ref = null;

            try {
                revMap.activateTest(updateFailure);
                ref = grsc.registerVlanMap(mgr, vpath, nvlan0, true);
                mappings.put(nvlan0, vpath);
            } catch (Exception e) {
                unexpected(e);
            }
            revision = checkRevision(revMap, true, revision, updateFailure);
            assertEquals(null, ref);
            checkMapCache(cs, MapType.VLAN, containerName, vpath, nvlan0);

            // add the same VlanMap with different bpath
            String bnamenew = "bridgenew";
            VBridgePath bpathnew = new VBridgePath(tname, bnamenew);
            VlanMapPath vpathnew = new VlanMapPath(bpathnew, mapId);
            try {
                revMap.activateTest(updateFailure);
                ref = grsc.registerVlanMap(mgr, vpathnew, nvlan0, true);
            } catch (Exception e) {
                unexpected(e);
            }
            revision = checkRevision(revMap, false, revision, updateFailure);

            MapReference required =
                new MapReference(MapType.VLAN, containerName, vpath);
            assertEquals(required, ref);
            checkMapCache(cs, MapType.VLAN, containerName, vpath, nvlan0);

            try {
                revMap.activateTest(updateFailure);
                ref = grsc.registerVlanMap(otherMgr, vpathnew, nvlan0, true);
            } catch (Exception e) {
                unexpected(e);
            }
            revision = checkRevision(revMap, false, revision, updateFailure);
            assertEquals(required, ref);
            checkMapCache(cs, MapType.VLAN, containerName, vpath, nvlan0);

            // Register VLAN mapping with the same VLAN ID and a specific
            // node.
            NodeVlan nvlan0_0 = new NodeVlan(node0, vlan);
            String mapId0_0 = createVlanMapId(nvlan0_0);
            VlanMapPath vpath0_0 = new VlanMapPath(bpathnew, mapId0_0);
            try {
                revMap.activateTest(updateFailure);
                ref = grsc.registerVlanMap(otherMgr, vpath0_0, nvlan0_0, true);
                mappings.put(nvlan0_0, vpath0_0);
            } catch (Exception e) {
                unexpected(e);
            }
            revision = checkRevision(revMap, true, revision, updateFailure);
            assertEquals(null, ref);
            checkMapCache(cs, MapType.VLAN, containerName, vpath, nvlan0);
            checkMapCache(cs, MapType.VLAN, other, vpath0_0, nvlan0_0);

            // add with different VLAN ID
            NodeVlan nvlan4095_0 = new NodeVlan(node0, (short)4095);
            String mapId4095_0 = createVlanMapId(nvlan4095_0);
            VlanMapPath vpath4095_0 = new VlanMapPath(bpathnew, mapId4095_0);
            try {
                revMap.activateTest(updateFailure);
                ref = grsc.registerVlanMap(mgr, vpath4095_0, nvlan4095_0, true);
                mappings.put(nvlan4095_0, vpath4095_0);
            } catch (Exception e) {
                unexpected(e);
            }
            revision = checkRevision(revMap, true, revision, updateFailure);
            assertEquals(null, ref);
            checkMapCache(cs, MapType.VLAN, containerName, vpath4095_0,
                          nvlan4095_0);

            try {
                revMap.activateTest(updateFailure);
                grsc.unregisterVlanMap(mgr, mappings.remove(nvlan0), nvlan0,
                                       true);
            } catch (Exception e) {
                unexpected(e);
            }
            revision = checkRevision(revMap, true, revision, updateFailure);
            checkMapCache(cs, MapType.VLAN, null, null, nvlan0);
            checkMapCache(cs, MapType.VLAN, other, vpath0_0, nvlan0_0);

            // check if other setting exist after unregister entry a VLAN ID == 0
            NodeVlan nvlan4095 = new NodeVlan(null, (short)4095);
            String mapId4095 = createVlanMapId(nvlan4095);
            VlanMapPath vpath4095 = new VlanMapPath(bpath, mapId4095);
            try {
                revMap.activateTest(updateFailure);
                ref = grsc.registerVlanMap(mgr, vpath4095, nvlan4095, true);
                mappings.put(nvlan4095, vpath4095);
            } catch (Exception e) {
                unexpected(e);
            }
            revision = checkRevision(revMap, true, revision, updateFailure);
            assertEquals(null, ref);
            checkMapCache(cs, MapType.VLAN, containerName, vpath4095,
                          nvlan4095);
            checkMapCache(cs, MapType.VLAN, containerName, vpath4095_0,
                          nvlan4095_0);

            // Try to register VLAN mapping with specifying VLAN:0 again.
            try {
                VlanMapPath p = new VlanMapPath(bpath, mapId0_0);
                revMap.activateTest(updateFailure);
                ref = grsc.registerVlanMap(mgr, p, nvlan0_0, true);
            } catch (Exception e) {
                unexpected(e);
            }
            revision = checkRevision(revMap, false, revision, updateFailure);
            required = new MapReference(MapType.VLAN, other, vpath0_0);
            assertEquals(required, ref);
            checkMapCache(cs, MapType.VLAN, null, null, nvlan0);
            checkMapCache(cs, MapType.VLAN, other, vpath0_0, nvlan0_0);

            try {
                revMap.activateTest(updateFailure);
                ref = grsc.registerVlanMap(mgr, vpath, nvlan0, true);
                mappings.put(nvlan0, vpath);
            } catch (Exception e) {
                unexpected(e);
            }
            revision = checkRevision(revMap, true, revision, updateFailure);
            assertEquals(null, ref);
            checkMapCache(cs, MapType.VLAN, containerName, vpath, nvlan0);
            checkMapCache(cs, MapType.VLAN, other, vpath0_0, nvlan0_0);

            // Try to map VLAN:4095 on Node:1 to bpath.
            NodeVlan nvlan4095_1 = new NodeVlan(node1, (short)4095);
            String mapId4095_1 = createVlanMapId(nvlan4095_1);
            VlanMapPath vpath4095_1 = new VlanMapPath(bpath, mapId4095_1);
            try {
                revMap.activateTest(updateFailure);
                ref = grsc.registerVlanMap(mgr, vpath4095_1, nvlan4095_1, true);
                mappings.put(nvlan4095_1, vpath4095_1);
            } catch (Exception e) {
                unexpected(e);
            }
            revision = checkRevision(revMap, true, revision, updateFailure);
            assertEquals(null, ref);
            checkMapCache(cs, MapType.VLAN, containerName, vpath4095,
                          nvlan4095);
            checkMapCache(cs, MapType.VLAN, containerName, vpath4095_0,
                          nvlan4095_0);
            checkMapCache(cs, MapType.VLAN, containerName, vpath4095_1,
                          nvlan4095_1);

            // Unregister VLAN:0 on Node:0.
            try {
                revMap.activateTest(updateFailure);
                grsc.unregisterVlanMap(otherMgr, vpath0_0, nvlan0_0, true);
                mappings.put(nvlan0_0, vpath0_0);
            } catch (Exception e) {
                unexpected(e);
            }
            revision = checkRevision(revMap, true, revision, updateFailure);
            checkMapCache(cs, MapType.VLAN, containerName, vpath, nvlan0);
            checkMapCache(cs, MapType.VLAN, null, null, nvlan0_0);

            // Try to unregister VLAN mapping which does not exist.
            NodeVlan[] invalid = {
                new NodeVlan(null, (short)1),
                new NodeVlan(node0, (short)1),
                nvlan0_0,
            };

            for (NodeVlan nv: invalid) {
                try {
                    revMap.activateTest(updateFailure);
                    grsc.unregisterVlanMap(mgr, vpath, nv, true);
                    fail("Exception must be thrown.");
                } catch (VTNException e) {
                    assertEquals(StatusCode.NOTFOUND, e.getStatus().getCode());
                }
                revision = checkRevision(revMap, false, revision,
                                         updateFailure);
            }

            for (NodeVlan nv: mappings.keySet()) {
                try {
                    revMap.activateTest(updateFailure);
                    grsc.unregisterVlanMap(mgr, invalidPath, nv, true);
                    fail("Exception must be thrown.");
                } catch (VTNException e) {
                    assertEquals(StatusCode.NOTFOUND, e.getStatus().getCode());
                }
                revision = checkRevision(revMap, false, revision,
                                         updateFailure);
            }

            ConcurrentMap<Short, String> vmap =
                (ConcurrentMap<Short, String>)cs.getCache(CACHE_VLANMAP);

            try {
                NodeVlan[] nvlans = {
                    nvlan0, nvlan4095, nvlan4095_0, nvlan4095_1
                };
                int size = nvlans.length;
                assertEquals(size, vmap.size());

                for (NodeVlan nv: nvlans) {
                    revMap.activateTest(updateFailure);
                    grsc.unregisterVlanMap(mgr, mappings.remove(nv), nv, true);
                    size--;
                    assertEquals(size, vmap.size());
                    revision = checkRevision(revMap, true, revision,
                                             updateFailure);
                }
            } catch (VTNException e) {
                unexpected(e);
            }

            updateFailure += 3;
        }
    }

    /**
     * Test method for
     * {@link GlobalResourceManager#registerPortMap(VTNManagerImpl,VBridgeIfPath,PortVlan,PortVlan,boolean)},
     * {@link GlobalResourceManager#unregisterPortMap(VTNManagerImpl,VBridgeIfPath,PortVlan,boolean)}.
     */
    @Test
    public void testRegisterPortMap() {
        short[] vlans = { 0, 10, 4095 };
        List<String> containerNames = new ArrayList<String>();
        containerNames.add("default");
        containerNames.add("tenant");

        HashMap<PortVlan, VBridgeIfPath> mappings =
            new HashMap<PortVlan, VBridgeIfPath>();
        TestStub stubObj = new TestStub(0);
        IClusterGlobalServices cs = (IClusterGlobalServices)stubObj;
        GlobalResourceManager grsc = setupGlobalResourceManager(stubObj);
        VTNManagerImpl otherMgr = setupVTNManager(grsc, stubObj, "other");

        ConfRevisionMap revMap = getRevisionCache(cs);
        int updateFailure = 0;

        String tname = "tenant";
        String bname = "bridge";
        String ifname = "interface";
        VBridgeIfPath bifpath = new VBridgeIfPath(tname, bname, ifname);
        VBridgeIfPath invalidPath = new VBridgeIfPath(tname, bname, "unknown");

        for (String containerName : containerNames) {
            VTNManagerImpl mgr = setupVTNManager(grsc, stubObj, containerName);

            for (NodeConnector nc : createNodeConnectors(3, false)) {
                for (short vlan : vlans) {
                    int revision = getRevision(revMap);

                    String emsg = "(NodeConnector)" + nc.toString()
                            + ",(vlan)" + vlan;

                    PortVlan pv = new PortVlan(nc, vlan);
                    MapReference ref = null;
                    try {
                        revMap.activateTest(updateFailure);
                        ref = grsc.registerPortMap(mgr, bifpath, pv, null,
                                                   true);
                        mappings.put(pv, bifpath);
                    } catch (Exception e) {
                        unexpected(e);
                    }
                    revision = checkRevision(revMap, true, revision,
                                             updateFailure);
                    assertEquals(emsg, null, ref);
                    checkMapCache(cs, MapType.PORT, containerName, bifpath,
                                  pv);

                    // add conflict PortVlan
                    String bnamenew = "bridgenew";
                    String ifnamenew = "interfacenew";
                    VBridgeIfPath bifpathnew =
                        new VBridgeIfPath(tname, bnamenew, ifnamenew);
                    try {
                        revMap.activateTest(updateFailure);
                        ref = grsc.registerPortMap(mgr, bifpathnew, pv, null,
                                                   true);
                    } catch (Exception e) {
                        unexpected(e);
                    }
                    revision = checkRevision(revMap, false, revision,
                                             updateFailure);
                    MapReference required =
                        new MapReference(MapType.PORT, containerName, bifpath);
                    assertEquals(emsg, required, ref);
                    checkMapCache(cs, MapType.PORT, containerName, bifpath,
                                  pv);

                    try {
                        revMap.activateTest(updateFailure);
                        ref = grsc.registerPortMap(otherMgr, bifpathnew, pv,
                                                   null, true);
                    } catch (Exception e) {
                        unexpected(e);
                    }
                    revision = checkRevision(revMap, false, revision,
                                             updateFailure);
                    assertEquals(emsg, required, ref);
                    checkMapCache(cs, MapType.PORT, containerName, bifpath,
                                  pv);

                    // Map nc/vlan:100 to bifpathnew.
                    PortVlan pvlan100 = new PortVlan(nc, (short)100);
                    try {
                        revMap.activateTest(updateFailure);
                        ref = grsc.registerPortMap(mgr, bifpathnew, pvlan100,
                                                   null, true);
                        mappings.put(pvlan100, bifpathnew);
                    } catch (Exception e) {
                        unexpected(e);
                    }
                    revision = checkRevision(revMap, true, revision,
                                             updateFailure);
                    assertEquals(revision, getRevision(revMap));
                    assertEquals(emsg, null, ref);

                    // Replace VLAN mapped to bifpathnew with nc/vlan:4094.
                    PortVlan pvnew = new PortVlan(nc, (short)4094);
                    try {
                        revMap.activateTest(updateFailure);
                        ref = grsc.registerPortMap(mgr, bifpathnew, pvnew,
                                                   pvlan100, true);
                        mappings.put(pvnew, bifpathnew);
                        mappings.remove(pvlan100);
                    } catch (Exception e) {
                        unexpected(e);
                    }
                    revision = checkRevision(revMap, true, revision,
                                             updateFailure);
                    assertEquals(revision, getRevision(revMap));
                    assertEquals(emsg, null, ref);

                    ConcurrentMap<PortVlan, MapReference> portmap =
                        (ConcurrentMap<PortVlan, MapReference>)cs.getCache(CACHE_PORTMAP);
                    checkMapCache(cs, MapType.PORT, containerName, bifpathnew,
                                  pvnew);
                    checkMapCache(cs, MapType.PORT, null, null, pvlan100);

                    // check if another entry exist after calling unregisterPortMap()
                    try {
                        revMap.activateTest(updateFailure);
                        ref = grsc.registerPortMap(mgr, bifpath, pvnew, null,
                                                   true);
                    } catch (Exception e) {
                        unexpected(e);
                    }
                    revision = checkRevision(revMap, false, revision,
                                             updateFailure);
                    required = new MapReference(MapType.PORT, containerName,
                                                bifpathnew);
                    assertEquals(emsg, required, ref);
                    checkMapCache(cs, MapType.PORT, containerName, bifpathnew,
                                  pvnew);

                    try {
                        revMap.activateTest(updateFailure);
                        ref = grsc.registerPortMap(mgr, bifpathnew, pv, null,
                                                   true);
                    } catch (Exception e) {
                        unexpected(e);
                    }
                    revision = checkRevision(revMap, false, revision,
                                             updateFailure);
                    required = new MapReference(MapType.PORT, containerName,
                                                bifpath);
                    assertEquals(emsg, required, ref);
                    checkMapCache(cs, MapType.PORT, containerName, bifpath,
                                  pv);

                    try {
                        revMap.activateTest(updateFailure);
                        grsc.unregisterPortMap(mgr, bifpath, pv, true);
                        mappings.remove(pv);
                    } catch (Exception e) {
                        unexpected(e);
                    }
                    revision = checkRevision(revMap, true, revision,
                                             updateFailure);
                    checkMapCache(cs, MapType.PORT, null, null, pv);

                    try {
                        revMap.activateTest(updateFailure);
                        ref = grsc.registerPortMap(mgr, bifpathnew, pv, null,
                                                   true);
                        mappings.put(pv, bifpathnew);
                    } catch (Exception e) {
                        unexpected(e);
                    }
                    revision = checkRevision(revMap, true, revision,
                                             updateFailure);
                    assertEquals(emsg, null, ref);
                    checkMapCache(cs, MapType.PORT, containerName, bifpathnew,
                                  pv);

                    try {
                        PortVlan pvlan = new PortVlan(nc, (short)1);
                        revMap.activateTest(updateFailure);
                        grsc.unregisterPortMap(mgr, bifpath, pvlan, true);
                        fail("Exception must be thrown.");
                    } catch (VTNException e) {
                        assertEquals(StatusCode.NOTFOUND,
                                     e.getStatus().getCode());
                    }
                    revision = checkRevision(revMap, false, revision,
                                             updateFailure);

                    for (PortVlan pvlan: mappings.keySet()) {
                        try {
                            revMap.activateTest(updateFailure);
                            grsc.unregisterPortMap(mgr, invalidPath, pvlan,
                                                   true);
                        } catch (VTNException e) {
                            assertEquals(StatusCode.NOTFOUND,
                                         e.getStatus().getCode());
                        }
                        revision = checkRevision(revMap, false, revision,
                                                 updateFailure);
                    }

                    try {
                        PortVlan[] pvlans = {pvnew, pv};
                        for (PortVlan pvlan: pvlans) {
                            revMap.activateTest(updateFailure);
                            grsc.unregisterPortMap(mgr, mappings.remove(pvlan),
                                                   pvlan, true);
                            revision = checkRevision(revMap, true, revision,
                                                     updateFailure);
                        }
                    } catch (Exception e) {
                        unexpected(e);
                    }
                }
            }

            updateFailure++;
        }
    }

    /**
     * Test case for
     * {@link GlobalResourceManager#getMapReference(NodeConnector, short)}.
     */
    @Test
    public void testGetMapReference() {
        TestStub stubObj = new TestStub(0);
        IClusterGlobalServices cs = (IClusterGlobalServices)stubObj;
        GlobalResourceManager grsc = setupGlobalResourceManager(stubObj);

        // Use 4 containers.
        ArrayList<VTNManagerImpl> managers = new ArrayList<VTNManagerImpl>();
        for (int i = 0; i < 4; i++) {
            String name = "container" + i;
            VTNManagerImpl mgr = setupVTNManager(grsc, stubObj, name);
            assertEquals(name, mgr.getContainerName());
            managers.add(mgr);
        }
        managers.trimToSize();

        // Create 3 nodes, and 3 node connectors per node.
        Node[] nodes = new Node[3];
        HashMap<Node, List<NodeConnector>> nodePorts =
            new HashMap<Node, List<NodeConnector>>();
        ArrayList<NodeConnector> allPorts = new ArrayList<NodeConnector>();
        nodePorts.put(null, allPorts);

        NodeConnector[][] ports = new NodeConnector[nodes.length][3];
        for (int n = 0; n < nodes.length; n++) {
            nodes[n] = NodeCreator.createOFNode(Long.valueOf((long)n));
            assertNotNull(nodes[n]);
            ArrayList<NodeConnector> portList = new ArrayList<NodeConnector>();
            nodePorts.put(nodes[n], portList);
            NodeConnector[] array = ports[n];

            for (int c = 0; c < array.length; c++) {
                NodeConnector nc = NodeConnectorCreator.
                    createOFNodeConnector(Short.valueOf((short)(c + 1)),
                                          nodes[n]);
                assertNotNull(nc);
                array[c] = nc;
                allPorts.add(nc);
                portList.add(nc);

                // getMapReference() must return null when no mapping is
                // established.
                for (short vlan = 0; vlan <= 10; vlan++) {
                    assertNull(grsc.getMapReference(nc, vlan));
                }
            }
            portList.trimToSize();
        }
        allPorts.trimToSize();

        // Define paths for vBridges and vBridge interfaces.
        ArrayList<VBridgePath> bridges = new ArrayList<VBridgePath>();
        ArrayList<VBridgeIfPath> interfaces = new ArrayList<VBridgeIfPath>();
        for (int t = 0; t < 2; t++) {
            String tname = "tenant" + t;
            for (int b = 0; b < 4; b++) {
                String bname = "bridge" + b;
                VBridgePath bpath = new VBridgePath(tname, bname);
                bridges.add(bpath);

                for (int i = 0; i < 2; i++) {
                    String iname = "if" + i;
                    interfaces.add(new VBridgeIfPath(bpath, iname));
                }
            }
        }
        bridges.trimToSize();
        interfaces.trimToSize();

        Iterator<VTNManagerImpl> mgIt = managers.iterator();
        Iterator<VBridgePath> brIt = bridges.iterator();
        Iterator<VBridgeIfPath> ifIt = interfaces.iterator();

        HashSet<Short> globalMapped = new HashSet<Short>();
        HashMap<PortVlan, MapReference> vlanMaps =
            new HashMap<PortVlan, MapReference>();
        HashMap<PortVlan, MapReference> portMaps =
            new HashMap<PortVlan, MapReference>();

        VTNManagerImpl mgr;

        try {
            // Establish VLAN mappings without specifying node.

            // Map VLAN:0.
            mgr = mgIt.next();
            MapReference vmap0 = addVlanMap(grsc, mgr, vlanMaps, brIt, null,
                                            allPorts, (short)0);
            assertNotNull(vmap0);
            if (!mgIt.hasNext()) {
                mgIt = managers.iterator();
            }
            if (!brIt.hasNext()) {
                brIt = bridges.iterator();
            }

            // Map VLAN:2 to.
            mgr = mgIt.next();
            MapReference vmap2 = addVlanMap(grsc, mgr, vlanMaps, brIt, null,
                                            allPorts, (short)2);
            assertNotNull(vmap2);
            if (!mgIt.hasNext()) {
                mgIt = managers.iterator();
            }
            if (!brIt.hasNext()) {
                brIt = bridges.iterator();
            }

            for (NodeConnector port: allPorts) {
                assertEquals(vmap0, grsc.getMapReference(port, (short)0));
                assertNull(grsc.getMapReference(port, (short)1));
                assertEquals(vmap2, grsc.getMapReference(port, (short)2));

                for (short vlan = 3; vlan <= 10; vlan++) {
                    assertNull(grsc.getMapReference(port, vlan));
                }
            }

            // Establish VLAN mappings with specifying node.

            // Map VLAN:0 on nodes[0].
            Node node = nodes[0];
            List<NodeConnector> portList = nodePorts.get(node);
            mgr = mgIt.next();
            addVlanMap(grsc, mgr, vlanMaps, brIt, node, portList, (short)0);
            if (!mgIt.hasNext()) {
                mgIt = managers.iterator();
            }
            if (!brIt.hasNext()) {
                brIt = bridges.iterator();
            }

            // Map VLAN:0 on nodes[2].
            node = nodes[2];
            portList = nodePorts.get(node);
            mgr = mgIt.next();
            addVlanMap(grsc, mgr, vlanMaps, brIt, node, portList, (short)0);
            if (!mgIt.hasNext()) {
                mgIt = managers.iterator();
            }
            if (!brIt.hasNext()) {
                brIt = bridges.iterator();
            }

            // Map VLAN:1 on nodes[1].
            node = nodes[1];
            portList = nodePorts.get(node);
            mgr = mgIt.next();
            addVlanMap(grsc, mgr, vlanMaps, brIt, node, portList, (short)1);
            if (!mgIt.hasNext()) {
                mgIt = managers.iterator();
            }
            if (!brIt.hasNext()) {
                brIt = bridges.iterator();
            }

            // Map VLAN:1 on nodes[2].
            node = nodes[2];
            portList = nodePorts.get(node);
            mgr = mgIt.next();
            addVlanMap(grsc, mgr, vlanMaps, brIt, node, portList, (short)1);
            if (!mgIt.hasNext()) {
                mgIt = managers.iterator();
            }
            if (!brIt.hasNext()) {
                brIt = bridges.iterator();
            }

            for (short vlan = 0; vlan <= 10; vlan++) {
                for (NodeConnector port: allPorts) {
                    PortVlan pv = new PortVlan(port, vlan);
                    MapReference expected = vlanMaps.get(pv);
                    assertEquals(expected, grsc.getMapReference(port, vlan));
                }
            }

            // Establish port mappings.

            // Map VLAN:0 on ports[0][0].
            mgr = mgIt.next();
            addPortMap(grsc, mgr, portMaps, ifIt, ports[0][0], (short)0);
            if (!mgIt.hasNext()) {
                mgIt = managers.iterator();
            }
            if (!ifIt.hasNext()) {
                ifIt = interfaces.iterator();
            }

            // Map VLAN:1 on ports[0][0].
            mgr = mgIt.next();
            addPortMap(grsc, mgr, portMaps, ifIt, ports[0][0], (short)1);
            if (!mgIt.hasNext()) {
                mgIt = managers.iterator();
            }
            if (!ifIt.hasNext()) {
                ifIt = interfaces.iterator();
            }

            // Map VLAN:0 on ports[1][1].
            mgr = mgIt.next();
            addPortMap(grsc, mgr, portMaps, ifIt, ports[1][1], (short)0);
            if (!mgIt.hasNext()) {
                mgIt = managers.iterator();
            }
            if (!ifIt.hasNext()) {
                ifIt = interfaces.iterator();
            }

            // Map VLAN:1 on ports[1][1].
            mgr = mgIt.next();
            addPortMap(grsc, mgr, portMaps, ifIt, ports[1][1], (short)1);
            if (!mgIt.hasNext()) {
                mgIt = managers.iterator();
            }
            if (!ifIt.hasNext()) {
                ifIt = interfaces.iterator();
            }

            // Map VLAN:2 on ports[1][2].
            mgr = mgIt.next();
            addPortMap(grsc, mgr, portMaps, ifIt, ports[1][2], (short)2);
            if (!mgIt.hasNext()) {
                mgIt = managers.iterator();
            }
            if (!ifIt.hasNext()) {
                ifIt = interfaces.iterator();
            }

            for (short vlan = 0; vlan <= 10; vlan++) {
                for (NodeConnector port: allPorts) {
                    PortVlan pv = new PortVlan(port, vlan);
                    MapReference expected = portMaps.get(pv);
                    if (expected == null) {
                        expected = vlanMaps.get(pv);
                    }
                    assertEquals(expected, grsc.getMapReference(port, vlan));
                }
            }
        } catch (Exception e) {
            unexpected(e);
        }
    }

    /**
     * Test case for {@link GlobalResourceManager#getApiVersion()} and
     * {@link GlobalResourceManager#getBundleVersion()}.
     */
    @Test
    public void testGetVersion() {
        GlobalResourceManager grsc = new GlobalResourceManager();
        assertEquals(GlobalResourceManager.API_VERSION, grsc.getApiVersion());

        // This should return null because the VTN Manager is not loaded by
        // a OSGi bundle class loader.
        assertNull(grsc.getBundleVersion());
    }

    /**
     * Test method for
     * {@link GlobalResourceManager#cleanUp(String)}.
     */
    @Test
    public void testCleanUp() {
        List<String> containerNames = new ArrayList<String>();
        containerNames.add("default");
        containerNames.add("tenant");

        TestStub stubObj = new TestStub(0);
        IClusterGlobalServices cs = (IClusterGlobalServices)stubObj;
        GlobalResourceManager grsc = setupGlobalResourceManager(stubObj);
        String other = "other";
        VTNManagerImpl otherMgr = setupVTNManager(grsc, stubObj, other);

        Node node0 = NodeCreator.createOFNode(Long.valueOf(0L));
        Node node1 = NodeCreator.createOFNode(Long.valueOf(1L));

        ConfRevisionMap revMap = getRevisionCache(cs);
        int updateFailure = 0;

        for (String containerName : containerNames) {
            VTNManagerImpl mgr = setupVTNManager(grsc, stubObj, containerName);

            String tname = "tenant";
            String bname = "bridge";
            VBridgePath bpath = new VBridgePath(tname, bname);
            short vlan = 0;
            NodeVlan nvlan0 = new NodeVlan(null, vlan);
            String mapId0 = createVlanMapId(nvlan0);
            VlanMapPath vpath0 = new VlanMapPath(bpath, mapId0);
            MapReference ref = null;
            try {
                ref = grsc.registerVlanMap(mgr, vpath0, nvlan0, true);
            } catch (Exception e) {
                unexpected(e);
            }
            assertEquals(null, ref);
            checkMapCache(cs, MapType.VLAN, containerName, vpath0, nvlan0);

            NodeConnector nc = NodeConnectorCreator
                    .createOFNodeConnector(Short.valueOf((short) 10), node0);
            String ifname = "interface";
            VBridgeIfPath bifpath = new VBridgeIfPath(tname, bname, ifname);
            PortVlan pv = new PortVlan(nc, vlan);
            try {
                ref = grsc.registerPortMap(mgr, bifpath, pv, null, true);
            } catch (Exception e) {
                unexpected(e);
            }
            assertEquals(null, ref);
            checkMapCache(cs, MapType.PORT, containerName, bifpath, pv);

            // Register one more VLAN and port mapping to other container.
            NodeVlan nvlan1 = new NodeVlan(node1, (short)1);
            String mapId1 = createVlanMapId(nvlan1);
            VlanMapPath vpath1 = new VlanMapPath(bpath, mapId1);
            try {
                ref = grsc.registerVlanMap(otherMgr, vpath1, nvlan1, true);
            } catch (Exception e) {
                unexpected(e);
            }
            assertEquals(null, ref);
            checkMapCache(cs, MapType.VLAN, other, vpath1, nvlan1);

            NodeConnector nc1 = NodeConnectorCreator.
                createOFNodeConnector(Short.valueOf((short)11), node1);
            String ifname1 = "if1";
            VBridgeIfPath ifpath1 = new VBridgeIfPath(tname, bname, ifname1);
            PortVlan pv1 = new PortVlan(nc1, (short)1);
            try {
                ref = grsc.registerPortMap(otherMgr, ifpath1, pv1, null, true);
            } catch (Exception e) {
                unexpected(e);
            }
            assertEquals(null, ref);
            checkMapCache(cs, MapType.PORT, other, ifpath1, pv1);

            ConcurrentMap<Short, String> vmap =
                (ConcurrentMap<Short, String>)cs.getCache(CACHE_VLANMAP);
            ConcurrentMap<PortVlan, String> pmap =
                (ConcurrentMap<PortVlan, String>)cs.getCache(CACHE_PORTMAP);
            assertEquals(2, vmap.size());
            assertEquals(2, pmap.size());

            int revision = getRevision(revMap);
            revMap.activateTest(updateFailure);
            grsc.cleanUp("clean");
            revision = checkRevision(revMap, false, revision, updateFailure);
            assertEquals(2, vmap.size());
            assertEquals(2, pmap.size());

            revMap.activateTest(updateFailure);
            grsc.cleanUp(containerName);
            revision = checkRevision(revMap, true, revision, updateFailure);
            assertEquals(1, vmap.size());
            assertEquals(1, pmap.size());

            revMap.activateTest(updateFailure);
            grsc.cleanUp(other);
            revision = checkRevision(revMap, true, revision, updateFailure);
            assertEquals(0, vmap.size());
            assertEquals(0, pmap.size());
        }
    }

    /**
     * Test case for {@link GlobalResourceManager#executeAsync(Runnable)}.
     */
    @Test
    public void testExecuteAsync() {
        /**
         * task Thread class used for test.
         * this task wait until a latch is counted down to zero.
         */
        class WaitThreadTask implements Runnable {
            protected long waitTime = 0;
            protected CountDownLatch latch = null;

            WaitThreadTask(long timeout, CountDownLatch latch) {
                this.waitTime = timeout;
                this.latch = latch;
            }

            @Override
            synchronized public void run() {
                latch.countDown();
                await(waitTime, TimeUnit.SECONDS);
            }

            protected boolean await(long timeout, TimeUnit unit) {
                try {
                    return latch.await(timeout, unit);
                } catch (InterruptedException e) {
                    return false;
                }
            }
        }

        GlobalResourceManager grsc = new GlobalResourceManager();
        Hashtable<String, String> properties = new Hashtable<String, String>();
        properties.put("containerName", "default");
        ComponentImpl c = new ComponentImpl(null, null, null);
        c.setServiceProperties(properties);
        grsc.init(c);

        CountDownLatch latch = new CountDownLatch(2);

        WaitThreadTask task1 = new WaitThreadTask(1000L, latch);
        grsc.executeAsync(task1);
        assertFalse(task1.await(10L, TimeUnit.MILLISECONDS));

        WaitThreadTask task2 = new WaitThreadTask(1000L, latch);
        grsc.executeAsync(task2);
        assertTrue(task1.await(1000L, TimeUnit.MILLISECONDS));
        assertTrue(task2.await(1000L, TimeUnit.MILLISECONDS));
        grsc.destroy();
    }

    /**
     * test case for
     * {@link GlobalResourceManager#getRemoteClusterSize()} and
     * {@link GlobalResourceManager#isRemoteClusterAddress(InetAddress)}.
     */
    @Test
    public void testGetRemoteClusterSize() {
        InetAddress loopbackAddr = InetAddress.getLoopbackAddress();
        InetAddress otherAddr
            = getInetAddressFromAddress(new byte[] {(byte) 10, (byte) 0,
                                                    (byte) 1, (byte) 99});

        ComponentImpl c = new ComponentImpl(null, null, null);
        GlobalResourceManager grsc = new GlobalResourceManager();
        TestStub stubObj = new TestStub(0);

        Hashtable<String, String> properties = new Hashtable<String, String>();
        properties.put("containerName", "default");
        c.setServiceProperties(properties);

        grsc.destroy();

        // 1 controller.
        grsc.setClusterGlobalService(stubObj);
        grsc.init(c);

        int numCont = grsc.getRemoteClusterSize();
        assertEquals(0, numCont);

        assertFalse(grsc.isRemoteClusterAddress(loopbackAddr));
        assertFalse(grsc.isRemoteClusterAddress(otherAddr));

        grsc.destroy();

        // 2 controllers.
        TestStubCluster csStub = new TestStubCluster(2);
        grsc.setClusterGlobalService(csStub);
        grsc.init(c);

        numCont = grsc.getRemoteClusterSize();
        assertEquals(1, numCont);

        for (InetAddress addr : csStub.getClusteredControllers()) {
            if (csStub.getMyAddress().equals(addr)) {
                assertFalse(grsc.isRemoteClusterAddress(addr));
            } else {
                assertTrue(grsc.isRemoteClusterAddress(addr));
            }
        }
        assertFalse(grsc.isRemoteClusterAddress(loopbackAddr));
        assertFalse(grsc.isRemoteClusterAddress(otherAddr));

        grsc.destroy();

        // if cluster service return null.
        ClusterServiceStub clsnull = new ClusterServiceStub();
        grsc.setClusterGlobalService(clsnull);
        grsc.init(c);

        assertEquals(0, grsc.getRemoteClusterSize());
        assertFalse(grsc.isRemoteClusterAddress(loopbackAddr));
        assertFalse(grsc.isRemoteClusterAddress(otherAddr));


        // no cluster service.
        grsc.unsetClusterGlobalService(csStub);
        grsc.init(c);

        assertEquals(0, grsc.getRemoteClusterSize());
        assertFalse(grsc.isRemoteClusterAddress(loopbackAddr));
        assertFalse(grsc.isRemoteClusterAddress(otherAddr));

        grsc.destroy();
    }

    /**
     * Test case for {@link GlobalResourceManager#coordinatorChanged()}.
     */
    @Test
    public void testCoordinatorChanged() {
        setupStartupDir();

        ComponentImpl c = new ComponentImpl(null, null, null);
        Hashtable<String, String> properties = new Hashtable<String, String>();
        properties.put("containerName", "default");
        c.setServiceProperties(properties);

        GlobalResourceManager resMgr = new GlobalResourceManager();
        TestStub stubObj = new TestStub(0);
        resMgr.setClusterGlobalService(stubObj);
        resMgr.init(c);

        VTNManagerImpl vtnMgr = setupVTNManager(resMgr, stubObj, "default");

        // add entries.
        VTenantPath tpath = new VTenantPath("tenant");
        VBridgePath bpath = new VBridgePath(tpath.getTenantName(), "bridge");

        vtnMgr.addTenant(tpath, new VTenantConfig(null));
        vtnMgr.addBridge(bpath, new VBridgeConfig(null));

        long mac = 1L;
        MacTableEntryId eid = new MacTableEntryId(bpath, mac);

        Node node = NodeCreator.createOFNode(0L);
        NodeConnector nc = NodeConnectorCreator
                .createOFNodeConnector(Short.valueOf("10"), node);
        InetAddress loopback = InetAddress.getLoopbackAddress();
        MacTableEntry entLoopback = new MacTableEntry(eid, nc, (short) 0, loopback);

        vtnMgr.putMacTableEntry(entLoopback);

        List<MacAddressEntry> entries = getMacEntries(vtnMgr, bpath);
        assertEquals(0, entries.size());

        InetAddress ipaddr
            = getInetAddressFromAddress(new byte[] {(byte) 192, (byte) 168,
                                                    (byte) 0, (byte) 2});
        mac = 2L;
        MacTableEntryId eidRemote = new MacTableEntryId(ipaddr, 2L, bpath, mac);
        MacTableEntry entRemote = new MacTableEntry(eidRemote, nc, (short) 0, ipaddr);

        ipaddr = getInetAddressFromAddress(new byte[] { (byte) 192, (byte) 168,
                                                        (byte) 0, (byte) 3});
        mac = 3L;
        MacTableEntryId eidRemote2 = new MacTableEntryId(ipaddr, 3L, bpath, mac);
        MacTableEntry entRemote2 = new MacTableEntry(eidRemote2, nc, (short) 0,
                                                     ipaddr);

        vtnMgr.putMacTableEntry(entRemote);
        vtnMgr.putMacTableEntry(entRemote2);

        resMgr.coordinatorChanged();

        entries = getMacEntries(vtnMgr, bpath);
        assertEquals(0, entries.size());

        // add remote controller.
        TestStubCluster csStub = new TestStubCluster(2);
        vtnMgr.stop();
        resMgr.setClusterGlobalService(csStub);
        resMgr.init(c);
        vtnMgr.init(c);
        vtnMgr.clearDisabledNode();

        vtnMgr.putMacTableEntry(entRemote);
        vtnMgr.entryUpdated(eidRemote, entRemote, VTNManagerImpl.CACHE_MAC, false);
        vtnMgr.putMacTableEntry(entRemote2);
        vtnMgr.entryUpdated(eidRemote2, entRemote2, VTNManagerImpl.CACHE_MAC, false);
        flushTasks(vtnMgr);
        entries = getMacEntries(vtnMgr, bpath);
        assertEquals(2, entries.size());

        // no change
        resMgr.coordinatorChanged();

        entries = getMacEntries(vtnMgr, bpath);
        assertEquals(2, entries.size());

        // remove remote controller.
        resMgr.setClusterGlobalService(stubObj);

        resMgr.coordinatorChanged();
        flushTasks(vtnMgr);

        entries = getMacEntries(vtnMgr, bpath);
        assertEquals(1, entries.size());

        vtnMgr.stopping();
        vtnMgr.stop();
        vtnMgr.destroy();

        cleanupStartupDir();
    }

    /**
     * Get MAC address entries
     *
     * @param vtnMgr    VTNManager service.
     * @param bpath     A {@link VBridgePath}.
     * @return  A list of {@link MacAddressEntry}.
     */
    private List<MacAddressEntry> getMacEntries(VTNManagerImpl vtnMgr,
                                                VBridgePath bpath) {
        List<MacAddressEntry> entries = null;
        try {
            entries = vtnMgr.getMacEntries(bpath);
        } catch (VTNException e) {
            unexpected(e);
        }

        return entries;
    }

    /**
     * Setup {@link GlobalResourceManager}.
     * @param stubObj       A {@link TestStub} object.
     * @return {@link GlobalResourceManager}.
     */
    private GlobalResourceManager setupGlobalResourceManager(TestStub stubObj) {
        GlobalResourceManager grsc = resourceManager;
        if (grsc != null) {
            resourceManager = null;
            grsc.destroy();
        }

        ComponentImpl c = new ComponentImpl(null, null, null);
        grsc = new GlobalResourceManager();

        Hashtable<String, String> properties = new Hashtable<String, String>();
        c.setServiceProperties(properties);

        grsc.setClusterGlobalService(stubObj);
        grsc.init(c);
        resourceManager = grsc;

        return grsc;
    }

    /**
     * Set up a {@link VTNManagerImpl} object for test.
     *
     * @param resMgr         {@link IVTNResourceManager} service.
     * @param stubObj        {@link TestStub} object.
     * @param containerName  The name of the container.
     * @return  A {@link VTNManagerImpl} object.
     */
    private VTNManagerImpl setupVTNManager(IVTNResourceManager resMgr,
                                           TestStub stubObj,
                                           String containerName) {
        VTNManagerImpl mgr = new VTNManagerImpl();
        ComponentImpl c = new ComponentImpl(null, null, null);
        Hashtable<String, String> properties = new Hashtable<String, String>();
        properties.put("containerName", containerName);
        c.setServiceProperties(properties);

        mgr.setResourceManager(resMgr);
        mgr.setClusterContainerService(stubObj);
        mgr.init(c);
        mgr.clearDisabledNode();

        return mgr;
    }

    /**
     * Check cluster cache in {@link GlobalResourceManager}.
     *
     * @param cs             A {@link IClusterGlobalServices} object.
     * @param type           Type of the virtual mapping.
     * @param containerName  The name of the container.
     * @param path           Path to the virtual mapping.
     * @param key            A key of the cache.
     */
    private <T> void checkMapCache(IClusterGlobalServices cs, MapType type,
                                   String containerName, VBridgePath path,
                                   T key) {
        String cacheName = (type == MapType.VLAN)
            ? CACHE_VLANMAP : CACHE_PORTMAP;
        ConcurrentMap<T, MapReference> map =
            (ConcurrentMap<T, MapReference>)cs.getCache(cacheName);
        assertNotNull(map);

        MapReference ref = map.get(key);
        if (containerName == null) {
            assertNull(ref);
        } else {
            MapReference expected =
                new MapReference(type, containerName, path);
            assertEquals(expected, ref);
        }
    }

    /**
     * Return a cluster cache which keeps configuration revision.
     *
     * @param cs   A {@link IClusterGlobalServices} object.
     * @return     A cluster cache which keeps configuration revision.
     */
    private ConfRevisionMap getRevisionCache(IClusterGlobalServices cs) {
        ConfRevisionMap map = (ConfRevisionMap)cs.getCache(CACHE_CONFREVISION);
        assertNotNull(map);

        return map;
    }

    /**
     * Return the current revision number of the configuration.
     *
     * @param map  A cluster cache which keeps configuration revision number.
     * @return     The current revision number.
     */
    private int getRevision(ConcurrentMap<MapType, Integer> map) {
        Integer current = map.get(MapType.ALL);
        if (current == null) {
            return CONFIG_REV_INIT;
        }

        return current.intValue();
    }

    /**
     * Check the current configuration revision number.
     *
     * @param revMap    Cluster cache which keeps current revision number.
     * @param updated   {@code true} means the configuration was successfully
     *                  updated.
     * @param revision  The configuration revision number before test.
     * @param failure   The expected number of concurrent update of
     *                  configuration.
     * @return          The current configuration revision number.
     */
    private int checkRevision(ConfRevisionMap revMap, boolean updated,
                              int revision, int failure) {
        int rev = revision;
        int cnt = failure + 1;
        assertEquals(cnt, revMap.getPutIfAbsentCount());

        if (updated) {
            assertEquals(cnt, revMap.getReplaceCount());
            assertEquals(0, revMap.getGetCount());
            rev++;
        } else {
            assertEquals(0, revMap.getReplaceCount());
            assertEquals(cnt,  revMap.getGetCount());
        }
        revMap.activateTest(0);

        assertEquals(rev, getRevision(revMap));

        return rev;
    }

    /**
     * Create an identifier for the VLAN mapping which maps the specified
     * VLAN network.
     *
     * @param nvlan  A {@link NodeVlan} object which specifies the VLAN
     *               network.
     * @return  A string which identifies the VLAN mapping.
     */
    private String createVlanMapId(NodeVlan nvlan) {
        StringBuilder idBuilder = new StringBuilder();
        Node node = nvlan.getNode();
        if (node == null) {
            // Node is unspecified.
            idBuilder.append(NODEID_ANY);
        } else {
            idBuilder.append(node.getType()).append('-').
                append(node.getNodeIDString());
        }

        short vlan = nvlan.getVlan();
        idBuilder.append('.').append((int)vlan);

        return idBuilder.toString();
    }

    /**
     * Register a new port mapping.
     *
     * @param resMgr       Global resource manager service.
     * @param mgr          VTN Manager service.
     * @param established  A map to store a {@link MapReference} instance which
     *                     points to the target vBridge interface.
     * @param it           Iterator for vBridge interface paths.
     *                     The first element is used for port mapping.
     * @param port         A {@link NodeConnector} corresponding to the switch
     *                     port.
     * @param vlan         A VLAN ID.
     * @throws VTNException    An error occurred.
     * @see  #testGetMapReference()
     */
    private void addPortMap(IVTNResourceManager resMgr, VTNManagerImpl mgr,
                            Map<PortVlan, MapReference> established,
                            Iterator<VBridgeIfPath> it, NodeConnector port,
                            short vlan)
        throws VTNException {
        VBridgeIfPath path = it.next();
        PortVlan pvlan = new PortVlan(port, vlan);
        MapReference ref = resMgr.registerPortMap(mgr, path, pvlan, null, true);
        if (ref != null) {
            throw new VTNException(StatusCode.CONFLICT,
                                   "Port mapping failed: " + pvlan +
                                   ": " + ref);
        }

        String cname = mgr.getContainerName();
        established.put(pvlan, new MapReference(MapType.PORT, cname, path));
    }

    /**
     * Register a new port mapping.
     *
     * @param resMgr       Global resource manager service.
     * @param mgr          VTN Manager service.
     * @param established  A map to store a {@link MapReference} instance which
     *                     points to the target vBridge interface.
     * @param it           Iterator for vBridge paths.
     *                     The first element is used for port mapping.
     * @param node         A {@link Node} corresponding to the switch.
     * @param ports        A list of {@link NodeConnector} corresponding to all
     *                     switch ports affected by a new VLAN mapping.
     * @param vlan         A VLAN ID.
     * @return             A {@link MapReference} instance which points to the
     *                     vBridge interface to which the specified port is
     *                     returned.
     * @throws VTNException    An error occurred.
     * @see  #testGetMapReference()
     */
    private MapReference addVlanMap(IVTNResourceManager resMgr,
                                    VTNManagerImpl mgr,
                                    Map<PortVlan, MapReference> established,
                                    Iterator<VBridgePath> it, Node node,
                                    List<NodeConnector> ports, short vlan)
        throws VTNException {
        NodeVlan nvlan = new NodeVlan(node, vlan);
        String mapId = createVlanMapId(nvlan);
        VBridgePath bpath = it.next();
        VlanMapPath path = new VlanMapPath(bpath, mapId);
        MapReference ref = resMgr.registerVlanMap(mgr, path, nvlan, true);
        if (ref != null) {
            throw new VTNException(StatusCode.CONFLICT,
                                   "VLAN mapping failed: " + nvlan +
                                   ": " + ref);
        }

        String cname = mgr.getContainerName();
        ref = new MapReference(MapType.VLAN, cname, path);
        for (NodeConnector port: ports) {
            PortVlan pvlan = new PortVlan(port, vlan);
            established.put(pvlan, ref);
        }

        return ref;
    }
}
