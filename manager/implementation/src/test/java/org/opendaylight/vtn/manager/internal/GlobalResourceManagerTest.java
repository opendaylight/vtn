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
import org.junit.AfterClass;
import org.junit.BeforeClass;
import org.junit.Test;

import org.opendaylight.vtn.manager.MacAddressEntry;
import org.opendaylight.vtn.manager.VBridgeConfig;
import org.opendaylight.vtn.manager.VBridgeIfPath;
import org.opendaylight.vtn.manager.VBridgePath;
import org.opendaylight.vtn.manager.VTNException;
import org.opendaylight.vtn.manager.VTenantConfig;
import org.opendaylight.vtn.manager.VTenantPath;
import org.opendaylight.vtn.manager.internal.cluster.ClusterEventId;
import org.opendaylight.vtn.manager.internal.cluster.MacMapPath;
import org.opendaylight.vtn.manager.internal.cluster.MacMapState;
import org.opendaylight.vtn.manager.internal.cluster.MacTableEntry;
import org.opendaylight.vtn.manager.internal.cluster.MacTableEntryId;
import org.opendaylight.vtn.manager.internal.cluster.MacVlan;
import org.opendaylight.vtn.manager.internal.cluster.MapReference;
import org.opendaylight.vtn.manager.internal.cluster.MapType;
import org.opendaylight.vtn.manager.internal.cluster.NodeVlan;
import org.opendaylight.vtn.manager.internal.cluster.PortVlan;
import org.opendaylight.vtn.manager.internal.cluster.VlanMapPath;

import org.opendaylight.controller.clustering.services.CacheConfigException;
import org.opendaylight.controller.clustering.services.CacheExistException;
import org.opendaylight.controller.clustering.services.IClusterGlobalServices;
import org.opendaylight.controller.clustering.services.IClusterServices.cacheMode;
import org.opendaylight.controller.sal.core.Node;
import org.opendaylight.controller.sal.core.NodeConnector;
import org.opendaylight.controller.sal.utils.NetUtils;
import org.opendaylight.controller.sal.utils.NodeConnectorCreator;
import org.opendaylight.controller.sal.utils.NodeCreator;
import org.opendaylight.controller.sal.utils.Status;
import org.opendaylight.controller.sal.utils.StatusCode;

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
     * The name of the cluster cache which keeps MAC mappings.
     */
    private static final String  CACHE_MACMAP = "vtn.macmap";

    /**
     * The name of the cluster cache which keeps VLAN networks to be eliminated
     * from MAC mapping.
     */
    private static final String  CACHE_MACMAP_DENY = "vtn.macmap.deny";

    /**
     * The name of the cluster cache which keeps runtime state of
     * MAC mapping.
     */
    private static final String  CACHE_MACMAP_STATE = "vtn.macmap.state";

    /**
     * A {@link GlobalResourceManager} instance.
     */
    private GlobalResourceManager  resourceManager;

    /**
     * A set of {@link VTNManagerImpl} instances.
     */
    private final Set<VTNManagerImpl> vtnManagers =
        new HashSet<VTNManagerImpl>();

    /**
     * Prepare the test environment.
     */
    @BeforeClass
    public static void prepare() {
        setupStartupDir();
    }

    /**
     * Clean up the test environment.
     */
    @AfterClass()
    public static void cleanUp() {
        cleanupStartupDir();
    }

    /**
     * Stub class of {@link IClusterGlobalServices}.
     * Some methods in this class always return null.
     */
    private class ClusterServiceStub implements IClusterGlobalServices {
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
     * An instance of this class is used to change MAC mapping configuration.
     */
    private class MacMapConf implements Cloneable {
        /**
         * Path to the MAC mapping.
         */
        private final MacMapPath  mapPath;

        /**
         * A set of hosts to be mapped by this MAC mapping.
         */
        private Set<MacVlan>  allowedHosts = new HashSet<MacVlan>();

        /**
         * A set of hosts not to be mapped by this MAC mapping.
         */
        private Set<MacVlan>  deniedHosts = new HashSet<MacVlan>();

        /**
         * A set of hosts to be added to {@link #allowedHosts}.
         */
        private Set<MacVlan>  allowAdded = new HashSet<MacVlan>();

        /**
         * A set of hosts to be removed from {@link #allowedHosts}.
         */
        private Set<MacVlan>  allowRemoved = new HashSet<MacVlan>();

        /**
         * A set of hosts to be added to {@link #deniedHosts}.
         */
        private Set<MacVlan>  denyAdded = new HashSet<MacVlan>();

        /**
         * A set of hosts to be removed from {@link #deniedHosts}.
         */
        private Set<MacVlan>  denyRemoved = new HashSet<MacVlan>();

        /**
         * A map which keeps activated hosts.
         */
        private Map<MacVlan, NodeConnector>  activeHosts =
            new HashMap<MacVlan, NodeConnector>();

        /**
         * Construct a new instance.
         *
         * @param path  A path to the target vBridge.
         */
        private MacMapConf(VBridgePath path) {
            mapPath = new MacMapPath(path);
        }

        /**
         * Return a path to the MAC mapping.
         *
         * @return  A path to the MAC mapping.
         */
        private MacMapPath getPath() {
            return mapPath;
        }

        /**
         * Return a set of hosts allowed to be mapped.
         *
         * @return  A set of {@link MacVlan} instances which represents
         *          allowed hosts.
         */
        private Set<MacVlan> getAllowedHosts() {
            return allowedHosts;
        }

        /**
         * Return a set of hosts denied to be mapped.
         *
         * @return  A set of {@link MacVlan} instances which represents
         *          denied hosts.
         */
        private Set<MacVlan> getDeniedHosts() {
            return deniedHosts;
        }

        /**
         * Add the specified host to the allowed host set.
         *
         * @param mvlan  A {@link MacVlan} instance.
         * @return  This instance.
         */
        private MacMapConf addAllowed(MacVlan mvlan) {
            if (allowedHosts.add(mvlan)) {
                allowAdded.add(mvlan);
            }
            return this;
        }

        /**
         * Remove the specified host from the allowed host set.
         *
         * @param mvlan  A {@link MacVlan} instance.
         * @return  This instance.
         */
        private MacMapConf removeAllowed(MacVlan mvlan) {
            if (allowedHosts.remove(mvlan)) {
                allowRemoved.add(mvlan);
            }
            return this;
        }

        /**
         * Add the specified host to the denied host set.
         *
         * @param mvlan  A {@link MacVlan} instance.
         * @return  This instance.
         */
        private MacMapConf addDenied(MacVlan mvlan) {
            if (deniedHosts.add(mvlan)) {
                denyAdded.add(mvlan);
            }
            return this;
        }

        /**
         * Remove the specified host from the denied host set.
         *
         * @param mvlan  A {@link MacVlan} instance.
         * @return  This instance.
         */
        private MacMapConf removeDenied(MacVlan mvlan) {
            if (deniedHosts.remove(mvlan)) {
                denyRemoved.add(mvlan);
            }
            return this;
        }

        /**
         * Configure the MAC mapping to be removed.
         */
        private void remove() {
            allowRemoved.addAll(allowedHosts);
            denyRemoved.addAll(deniedHosts);
            allowedHosts.clear();
            deniedHosts.clear();
        }

        /**
         * Clear configuration.
         */
        private void clear() {
            allowedHosts.clear();
            deniedHosts.clear();
            allowAdded.clear();
            allowRemoved.clear();
            denyAdded.clear();
            denyRemoved.clear();
            activeHosts.clear();
        }

        /**
         * Determine whether the specified host will be mapped by this
         * MAC mapping or not.
         *
         * @param mvlan  A {@link MacVlan} instance.
         * @return  {@code true} only if the specified host will be mapped.
         */
        private boolean isMapped(MacVlan mvlan) {
            if (deniedHosts.contains(mvlan)) {
                return false;
            }
            if (allowedHosts.contains(mvlan)) {
                return true;
            }

            MacVlan wc = new MacVlan(MacVlan.UNDEFINED, mvlan.getVlan());
            return allowedHosts.contains(wc);
        }

        /**
         * Change the MAC mapping configuration.
         *
         * @param mgr  VTN Manager service.
         * @throws VTNException    An error occurred.
         */
        private void change(VTNManagerImpl mgr)
            throws VTNException {
            IVTNResourceManager resMgr = mgr.getResourceManager();
            int flags = (allowedHosts.isEmpty() && deniedHosts.isEmpty())
                ? MacMapChange.REMOVING : 0;
            assertFalse(allowAdded.isEmpty() && allowRemoved.isEmpty() &&
                        denyAdded.isEmpty() && denyRemoved.isEmpty());
            MacMapChange ch =
                new MacMapChange(allowAdded, allowRemoved, denyAdded,
                                 denyRemoved, flags);
            resMgr.registerMacMap(mgr, mapPath, ch);

            for (MacVlan mvlan: allowRemoved) {
                activeHosts.remove(mvlan);
            }
            for (MacVlan mvlan: denyAdded) {
                activeHosts.remove(mvlan);
            }

            allowAdded.clear();
            allowRemoved.clear();
            denyAdded.clear();
            denyRemoved.clear();
        }

        /**
         * Activate the MAC mapping for the specified host.
         *
         * @param mgr    VTN Manager service.
         * @param mvlan  A {@link MacVlan} instance which represents the host
         *               to be activated.
         * @param port   A {@link NodeConnector} instance corresponding to
         *               a switch port where the host was detected.
         * @throws VTNException    An error occurred.
         */
        private void activate(VTNManagerImpl mgr, MacVlan mvlan,
                              NodeConnector port)
            throws VTNException {
            IVTNResourceManager resMgr = mgr.getResourceManager();
            boolean expected = activeHosts.isEmpty();
            assertEquals(expected,
                         resMgr.activateMacMap(mgr, mapPath, mvlan, port));
            activeHosts.put(mvlan, port);
        }

        /**
         * Inactivate MAC mappings on the specified switch port.
         *
         * @param mgr   VTN Manager service.
         * @param port  A {@link NodeConnector} corresponding to a switch port.
         * @throws VTNException    An error occurred.
         */
        private void inactivate(VTNManagerImpl mgr, NodeConnector port)
            throws VTNException {
            IVTNResourceManager resMgr = mgr.getResourceManager();
            Map<MacVlan, NodeConnector> newHosts =
                new HashMap<MacVlan, NodeConnector>(activeHosts);
            for (Iterator<NodeConnector> it = newHosts.values().iterator();
                 it.hasNext();) {
                NodeConnector nc = it.next();
                if (nc.equals(port)) {
                    it.remove();
                }
            }
            boolean expected = !newHosts.isEmpty();
            PortFilter filter = new SpecificPortFilter(port);
            assertEquals(expected,
                         resMgr.inactivateMacMap(mgr, mapPath, filter));
            activeHosts = newHosts;
        }

        /**
         * Return a reference to the MAC mapping.
         *
         * @param mgr     VTN Manager service.
         * @return  A reference to the MAC mapping.
         */
        private MapReference getMapReference(VTNManagerImpl mgr) {
            MapReference ref = mgr.getMapReference(mapPath);
            assertEquals(MapType.MAC, ref.getMapType());
            assertEquals(mgr.getContainerName(), ref.getContainerName());
            assertEquals(mapPath, ref.getPath());

            return ref;
        }

        /**
         * Check cluster caches for this MAC mapping configuration.
         *
         * @param cs             Cluster service.
         * @param containerName  The name of the container.
         */
        private void checkCache(IClusterGlobalServices cs,
                                String containerName) {
            boolean exists = !(allowedHosts.isEmpty() &&
                               deniedHosts.isEmpty());
            ConcurrentMap<MacVlan, MapReference> macAllowed =
                (ConcurrentMap<MacVlan, MapReference>)
                cs.getCache(CACHE_MACMAP);
            ConcurrentMap<MacVlan, Set<MapReference>> macDenied =
                (ConcurrentMap<MacVlan, Set<MapReference>>)
                cs.getCache(CACHE_MACMAP_DENY);
            ConcurrentMap<MapReference, MacMapState> macState =
                (ConcurrentMap<MapReference, MacMapState>)
                cs.getCache(CACHE_MACMAP_STATE);
            MapReference ref = new MapReference(MapType.MAC, containerName,
                                                mapPath);
            if (allowedHosts.isEmpty() && deniedHosts.isEmpty()) {
                // MAC mapping configuration should not exist.
                for (MacVlan allowed: allowedHosts) {
                    MapReference r = macAllowed.get(allowed);
                    if (r != null) {
                        assertFalse("allowed=" + allowed + ", ref=" + r,
                                    r.equals(ref));
                    }
                }
                for (MacVlan denied: deniedHosts) {
                    Set<MapReference> rset = macDenied.get(denied);
                    if (rset != null) {
                        assertFalse(rset.isEmpty());
                        assertFalse(rset.contains(ref));
                    }
                }
                assertFalse(macState.containsKey(ref));
            } else {
                for (MacVlan allowed: allowedHosts) {
                    assertEquals("allowed=" + allowed,
                                 ref, macAllowed.get(allowed));
                }
                for (MacVlan denied: deniedHosts) {
                    Set<MapReference> rset = macDenied.get(denied);
                    assertTrue(rset.contains(ref));
                }
                MacMapState mst = macState.get(ref);
                Map<MacVlan, NodeConnector> hosts = activeHosts;
                if (mst == null) {
                    assertTrue(hosts.isEmpty());
                } else {
                    if (hosts.isEmpty()) {
                        hosts = null;
                    }
                    assertEquals(hosts, mst.getActiveHosts());
                }
            }
        }

        /**
         * Check MAC mappin state for the specified host.
         *
         * @param mgr    VTN Manager service.
         * @param mvlan  A {@link MacVlan} instance to be tested.
         */
        private void checkState(VTNManagerImpl mgr, MacVlan mvlan) {
            IVTNResourceManager resMgr = mgr.getResourceManager();
            if (activeHosts.isEmpty()) {
                assertNull(resMgr.getMacMappedNetworks(mgr, mapPath));
                assertNull(resMgr.getMacMappedHosts(mgr, mapPath));
                assertFalse(resMgr.hasMacMappedHost(mgr, mapPath));
            } else {
                assertEquals(activeHosts,
                             resMgr.getMacMappedHosts(mgr, mapPath));

                Set<PortVlan> nwSet = new HashSet<PortVlan>();
                for (Map.Entry<MacVlan, NodeConnector> entry:
                         activeHosts.entrySet()) {
                    MacVlan mv = entry.getKey();
                    NodeConnector nc = entry.getValue();
                    PortVlan pv = new PortVlan(nc, mv.getVlan());
                    nwSet.add(pv);
                }
                assertEquals(nwSet, resMgr.getMacMappedNetworks(mgr, mapPath));
                assertTrue(resMgr.hasMacMappedHost(mgr, mapPath));
            }

            NodeConnector port = activeHosts.get(mvlan);
            assertEquals(port, resMgr.getMacMappedPort(mgr, mapPath, mvlan));
            long mac = mvlan.getMacAddress();
            if (port == null) {
                assertNull(resMgr.getMacMappedNetwork(mgr, mapPath, mac));
            } else {
                PortVlan pv = new PortVlan(port, mvlan.getVlan());
                assertEquals(pv,
                             resMgr.getMacMappedNetwork(mgr, mapPath, mac));
            }
        }

        // Cloneable

        /**
         * Return a shallow copy of this instance.
         *
         * @return  A copy of this instance.
         */
        @Override
        public MacMapConf clone() {
            try {
                MacMapConf mc = (MacMapConf)super.clone();
                mc.allowedHosts = (Set<MacVlan>)
                    ((HashSet<MacVlan>)allowedHosts).clone();
                mc.deniedHosts = (Set<MacVlan>)
                    ((HashSet<MacVlan>)deniedHosts).clone();
                mc.allowAdded = (Set<MacVlan>)
                    ((HashSet<MacVlan>)allowAdded).clone();
                mc.allowRemoved = (Set<MacVlan>)
                    ((HashSet<MacVlan>)allowRemoved).clone();
                mc.denyAdded = (Set<MacVlan>)
                    ((HashSet<MacVlan>)denyAdded).clone();
                mc.denyRemoved = (Set<MacVlan>)
                    ((HashSet<MacVlan>)denyRemoved).clone();
                mc.activeHosts = (Map<MacVlan, NodeConnector>)
                    ((HashMap<MacVlan, NodeConnector>)activeHosts).clone();

                return mc;
            } catch (CloneNotSupportedException e) {
                throw new InternalError();
            }
        }
    }

    /**
     * Tear down the test environment.
     */
    @After
    public void tearDown() {
        for (VTNManagerImpl mgr: vtnManagers) {
            mgr.stopping();
            mgr.stop();
            mgr.destroy();
        }

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
        IClusterGlobalServices cs = (IClusterGlobalServices)stubObj;

        Hashtable<String, String> properties = new Hashtable<String, String>();
        properties.put("containerName", "default");
        c.setServiceProperties(properties);

        grsc.destroy();
        grsc.init(c);

        ConcurrentMap<Short, String> vmap = (ConcurrentMap<Short, String>)
            cs.getCache(CACHE_VLANMAP);
        assertNull(vmap);

        ConcurrentMap<Short, String> pmap = (ConcurrentMap<Short, String>)
            cs.getCache(CACHE_PORTMAP);
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

            vmap = (ConcurrentMap<Short, String>)cs.getCache(CACHE_VLANMAP);
            assertNotNull(vmap);
            assertEquals(0, vmap.size());

            pmap = (ConcurrentMap<Short, String>)cs.getCache(CACHE_PORTMAP);
            assertNotNull(pmap);
            assertEquals(0, pmap.size());

            evid = new  ClusterEventId();
            if (service.getMyAddress() != null &&
                !service.getMyAddress().equals(loopback)) {
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
            NodeVlan nvlan0Node0 = new NodeVlan(node0, vlan);
            String mapId0Node0 = createVlanMapId(nvlan0Node0);
            VlanMapPath vpath0Node0 = new VlanMapPath(bpathnew, mapId0Node0);
            try {
                revMap.activateTest(updateFailure);
                ref = grsc.registerVlanMap(otherMgr, vpath0Node0, nvlan0Node0,
                                           true);
                mappings.put(nvlan0Node0, vpath0Node0);
            } catch (Exception e) {
                unexpected(e);
            }
            revision = checkRevision(revMap, true, revision, updateFailure);
            assertEquals(null, ref);
            checkMapCache(cs, MapType.VLAN, containerName, vpath, nvlan0);
            checkMapCache(cs, MapType.VLAN, other, vpath0Node0, nvlan0Node0);

            // add with different VLAN ID
            NodeVlan nvlan4095Node0 = new NodeVlan(node0, (short)4095);
            String mapId4095Node0 = createVlanMapId(nvlan4095Node0);
            VlanMapPath vpath4095Node0 =
                new VlanMapPath(bpathnew, mapId4095Node0);
            try {
                revMap.activateTest(updateFailure);
                ref = grsc.registerVlanMap(mgr, vpath4095Node0, nvlan4095Node0,
                                           true);
                mappings.put(nvlan4095Node0, vpath4095Node0);
            } catch (Exception e) {
                unexpected(e);
            }
            revision = checkRevision(revMap, true, revision, updateFailure);
            assertEquals(null, ref);
            checkMapCache(cs, MapType.VLAN, containerName, vpath4095Node0,
                          nvlan4095Node0);

            try {
                revMap.activateTest(updateFailure);
                grsc.unregisterVlanMap(mgr, mappings.remove(nvlan0), nvlan0,
                                       true);
            } catch (Exception e) {
                unexpected(e);
            }
            revision = checkRevision(revMap, true, revision, updateFailure);
            checkMapCache(cs, MapType.VLAN, null, null, nvlan0);
            checkMapCache(cs, MapType.VLAN, other, vpath0Node0, nvlan0Node0);

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
            checkMapCache(cs, MapType.VLAN, containerName, vpath4095Node0,
                          nvlan4095Node0);

            // Try to register VLAN mapping with specifying VLAN:0 again.
            try {
                VlanMapPath p = new VlanMapPath(bpath, mapId0Node0);
                revMap.activateTest(updateFailure);
                ref = grsc.registerVlanMap(mgr, p, nvlan0Node0, true);
            } catch (Exception e) {
                unexpected(e);
            }
            revision = checkRevision(revMap, false, revision, updateFailure);
            required = new MapReference(MapType.VLAN, other, vpath0Node0);
            assertEquals(required, ref);
            checkMapCache(cs, MapType.VLAN, null, null, nvlan0);
            checkMapCache(cs, MapType.VLAN, other, vpath0Node0, nvlan0Node0);

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
            checkMapCache(cs, MapType.VLAN, other, vpath0Node0, nvlan0Node0);

            // Try to map VLAN:4095 on Node:1 to bpath.
            NodeVlan nvlan4095Node1 = new NodeVlan(node1, (short)4095);
            String mapId4095Node1 = createVlanMapId(nvlan4095Node1);
            VlanMapPath vpath4095Node1 = new VlanMapPath(bpath, mapId4095Node1);
            try {
                revMap.activateTest(updateFailure);
                ref = grsc.registerVlanMap(mgr, vpath4095Node1, nvlan4095Node1,
                                           true);
                mappings.put(nvlan4095Node1, vpath4095Node1);
            } catch (Exception e) {
                unexpected(e);
            }
            revision = checkRevision(revMap, true, revision, updateFailure);
            assertEquals(null, ref);
            checkMapCache(cs, MapType.VLAN, containerName, vpath4095,
                          nvlan4095);
            checkMapCache(cs, MapType.VLAN, containerName, vpath4095Node0,
                          nvlan4095Node0);
            checkMapCache(cs, MapType.VLAN, containerName, vpath4095Node1,
                          nvlan4095Node1);

            // Unregister VLAN:0 on Node:0.
            try {
                revMap.activateTest(updateFailure);
                grsc.unregisterVlanMap(otherMgr, vpath0Node0, nvlan0Node0,
                                       true);
                mappings.put(nvlan0Node0, vpath0Node0);
            } catch (Exception e) {
                unexpected(e);
            }
            revision = checkRevision(revMap, true, revision, updateFailure);
            checkMapCache(cs, MapType.VLAN, containerName, vpath, nvlan0);
            checkMapCache(cs, MapType.VLAN, null, null, nvlan0Node0);

            // Try to unregister VLAN mapping which does not exist.
            NodeVlan[] invalid = {
                new NodeVlan(null, (short)1),
                new NodeVlan(node0, (short)1),
                nvlan0Node0,
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
                    nvlan0, nvlan4095, nvlan4095Node0, nvlan4095Node1
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
        short[] vlans = {0, 10, 4095};
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
     * Test case for MAC mapping.
     *
     * <ul>
     *   <li>{@link GlobalResourceManager#registerMacMap(VTNManagerImpl, MacMapPath, MacMapChange)}</li>
     *   <li>{@link GlobalResourceManager#getMapReference(MacVlan)}</li>
     *   <li>{@link GlobalResourceManager#getMacMappedNetworks(VTNManagerImpl, MacMapPath)}</li>
     *   <li>{@link GlobalResourceManager#getMacMappedNetwork(VTNManagerImpl, MacMapPath, long)}</li>
     *   <li>{@link GlobalResourceManager#getMacMappedHosts(VTNManagerImpl, MacMapPath)}</li>
     *   <li>{@link GlobalResourceManager#getMacMappedPort(VTNManagerImpl, MacMapPath, MacVlan)}</li>
     *   <li>{@link GlobalResourceManager#hasMacMappedHost(VTNManagerImpl, MacMapPath)}</li>
     *   <li>{@link GlobalResourceManager#activateMacMap(VTNManagerImpl, MacMapPath, MacVlan, NodeConnector)}</li>
     *   <li>{@link GlobalResourceManager#inactivateMacMap(VTNManagerImpl, MacMapPath, PortFilter)}</li>
     * </ul>
     */
    @Test
    public void testMacMap() {
        TestStub stubObj = new TestStub(0);
        IClusterGlobalServices cs = (IClusterGlobalServices)stubObj;
        GlobalResourceManager grsc = setupGlobalResourceManager(stubObj);
        ConfRevisionMap revMap = getRevisionCache(cs);

        // Create one container, one node, and 4 ports.
        String container = "default";
        VTNManagerImpl mgr = setupVTNManager(grsc, stubObj, container);
        Node node = NodeCreator.createOFNode(Long.valueOf(0L));
        ArrayList<NodeConnector> allPorts = new ArrayList<NodeConnector>();
        for (int i = 0; i < 4; i++) {
            NodeConnector port = NodeConnectorCreator.
                createOFNodeConnector((short)(i + 1), node);
            assertNotNull(port);
            allPorts.add(port);
        }
        allPorts.trimToSize();

        // Create 4 MAC addresses per port.
        // Each MAC address is on VLAN 0, 1, 2, and 3.
        Map<NodeConnector, List<MacVlan>> portHosts =
            new HashMap<NodeConnector, List<MacVlan>>();
        Map<MacVlan, NodeConnector> allHosts =
            new HashMap<MacVlan, NodeConnector>();
        long macAddr = 1L;
        for (NodeConnector port: allPorts) {
            ArrayList<MacVlan> hostList = new ArrayList<MacVlan>();
            assertNull(portHosts.put(port, hostList));
            for (int i = 0; i < 4; i++) {
                for (short vlan = 0; vlan <= 3; vlan++) {
                    MacVlan mvlan = new MacVlan(macAddr, vlan);
                    hostList.add(mvlan);
                    assertNull(allHosts.put(mvlan, port));
                }
                macAddr++;
            }
            hostList.trimToSize();
        }

        // Use 2 vBridges.
        VBridgePath bpath0 = new VBridgePath("tenant", "bridge0");
        VBridgePath bpath1 = new VBridgePath("tenant", "bridge1");

        for (int updateFailure = 0; updateFailure <= 2; updateFailure += 2) {
            MacMapConf mc0 = new MacMapConf(bpath0);
            MacMapConf mc1 = new MacMapConf(bpath1);
            MapReference ref0 = mc0.getMapReference(mgr);
            MapReference ref1 = mc1.getMapReference(mgr);

            int revision = getRevision(revMap);

            // Map VLAN 1 to mc0.
            try {
                revMap.activateTest(updateFailure);
                mc0.addAllowed(new MacVlan(MacVlan.UNDEFINED, (short)1));
                mc0.change(mgr);
            } catch (Exception e) {
                unexpected(e);
            }
            revision = checkRevision(revMap, true, revision, updateFailure);
            mc0.checkCache(cs, container);
            mc1.checkCache(cs, container);
            for (int i = 0; i < 0x20; i++) {
                long mac = 0xf00000000000L + i;
                MacVlan mvlan = new MacVlan(mac, (short)1);
                assertEquals(ref0, grsc.getMapReference(mvlan));
                mc0.checkState(mgr, mvlan);
                mc1.checkState(mgr, mvlan);
            }

            // Activate all hosts on VLAN 1.
            for (Map.Entry<MacVlan, NodeConnector> entry: allHosts.entrySet()) {
                MacVlan mvlan = entry.getKey();
                if (mvlan.getVlan() == 1) {
                    NodeConnector port = entry.getValue();
                    try {
                        revMap.activateTest(updateFailure);
                        mc1.activate(mgr, mvlan, port);
                        fail("An exception must be thrown.");
                    } catch (MacMapGoneException e) {
                        assertEquals(mvlan, e.getHost());
                        assertEquals(ref1, e.getMapReference());
                    } catch (Exception e) {
                        unexpected(e);
                    }
                    revision = checkRevision(revMap, false, revision,
                                             updateFailure);
                    mc0.checkState(mgr, mvlan);
                    mc1.checkState(mgr, mvlan);

                    try {
                        revMap.activateTest(updateFailure);
                        mc0.activate(mgr, mvlan, port);
                    } catch (Exception e) {
                        unexpected(e);
                    }
                    revision = checkRevision(revMap, true, revision,
                                             updateFailure);
                    mc0.checkState(mgr, mvlan);
                    mc1.checkState(mgr, mvlan);
                }
            }

            // Iterate switch ports, and map one host on VLAN 0 to mc0.
            MacVlan host0 = null;
            for (NodeConnector port: allPorts) {
                for (MacVlan mvlan: portHosts.get(port)) {
                    if (mvlan.getVlan() == 0) {
                        try {
                            revMap.activateTest(updateFailure);
                            mc0.addAllowed(mvlan);
                            mc0.change(mgr);
                        } catch (Exception e) {
                            unexpected(e);
                        }
                        revision = checkRevision(revMap, true, revision,
                                                 updateFailure);
                        assertEquals(ref0, grsc.getMapReference(mvlan));
                        mc0.checkCache(cs, container);
                        mc1.checkCache(cs, container);

                        // This host can not be mapped because the same
                        // MAC address on VLAN 1 is already mapped.
                        try {
                            revMap.activateTest(updateFailure);
                            mc0.activate(mgr, mvlan, port);
                            fail("An exception must be thrown.");
                        } catch (MacMapDuplicateException e) {
                            MacVlan mv = new MacVlan(mvlan.getMacAddress(),
                                                     (short)1);
                            assertEquals(mv, e.getDuplicate());
                            assertEquals(mvlan, e.getHost());
                            assertEquals(ref0, e.getMapReference());
                        } catch (Exception e) {
                            unexpected(e);
                        }
                        revision = checkRevision(revMap, false, revision,
                                                 updateFailure);
                        mc0.checkCache(cs, container);
                        mc1.checkCache(cs, container);
                        host0 = mvlan;
                        break;
                    }
                }
            }
            assertNotNull(host0);

            // Add a pair of MAC address in host0 and VLAN ID 1 to denied host
            // set.
            try {
                MacVlan mv = new MacVlan(host0.getMacAddress(), (short)1);
                assertEquals(ref0, grsc.getMapReference(mv));
                revMap.activateTest(updateFailure);
                mc0.addDenied(mv);
                mc0.change(mgr);
                revision = checkRevision(revMap, true, revision,
                                         updateFailure);
                assertEquals(null, grsc.getMapReference(mv));

                // Now host0 can be mapped.
                revMap.activateTest(updateFailure);
                mc0.activate(mgr, host0, allHosts.get(host0));
                revision = checkRevision(revMap, true, revision,
                                         updateFailure);
            } catch (Exception e) {
                unexpected(e);
            }

            // Inactivate all MAC mappings in mc0.
            for (NodeConnector port: allPorts) {
                try {
                    revMap.activateTest(updateFailure);
                    mc0.inactivate(mgr, port);
                } catch (Exception e) {
                    unexpected(e);
                }
                revision = checkRevision(revMap, true, revision,
                                         updateFailure);
                mc0.checkCache(cs, container);
                mc1.checkCache(cs, container);
            }
            assertFalse(grsc.hasMacMappedHost(mgr, mc0.getPath()));

            // Activate MAC mappings for hosts on VLAN 0.
            for (MacVlan mvlan: mc0.getAllowedHosts()) {
                if (mvlan.getMacAddress() != MacVlan.UNDEFINED) {
                    NodeConnector port = allHosts.get(mvlan);
                    try {
                        revMap.activateTest(updateFailure);
                        mc0.activate(mgr, mvlan, port);
                    } catch (Exception e) {
                        unexpected(e);
                    }
                    revision = checkRevision(revMap, true, revision,
                                             updateFailure);
                    mc0.checkCache(cs, container);
                    mc1.checkCache(cs, container);
                }
            }

            // Map hosts on switch ports reserved by mc0 to mc1, but they can
            // not be activated.
            Set<PortVlan> nwSet = grsc.getMacMappedNetworks(mgr, mc0.getPath());
            assertNotNull(nwSet);
            assertFalse(nwSet.isEmpty());
            Set<MacVlan> hset = new HashSet<MacVlan>();
            for (PortVlan pvlan: nwSet) {
                NodeConnector port = pvlan.getNodeConnector();
                short vlan = pvlan.getVlan();
                for (MacVlan mvlan: portHosts.get(port)) {
                    if (mvlan.getVlan() == vlan && !mc0.isMapped(mvlan)) {
                        mc1.addAllowed(mvlan);
                        hset.add(mvlan);
                    }
                }
            }
            try {
                revMap.activateTest(updateFailure);
                mc1.change(mgr);
            } catch (Exception e) {
                unexpected(e);
            }
            revision = checkRevision(revMap, true, revision, updateFailure);
            mc0.checkCache(cs, container);
            mc1.checkCache(cs, container);

            for (MacVlan mvlan: hset) {
                assertEquals(ref1, grsc.getMapReference(mvlan));
                try {
                    revMap.activateTest(updateFailure);
                    mc1.activate(mgr, mvlan, allHosts.get(mvlan));
                    fail("An exception must be thrown.");
                } catch (MacMapPortBusyException e) {
                    assertEquals(ref0, e.getAnotherMapping());
                    assertEquals(mvlan, e.getHost());
                    assertEquals(ref1, e.getMapReference());
                } catch (Exception e) {
                    unexpected(e);
                }
                revision = checkRevision(revMap, false, revision,
                                         updateFailure);
            }

            // Ensure that hosts already mapped to mc0 can not map to mc1.
            for (MacVlan mvlan: mc0.getAllowedHosts()) {
                MacMapConf mc = mc1.clone();
                mc.addAllowed(mvlan);
                try {
                    revMap.activateTest(updateFailure);
                    mc.change(mgr);
                    fail("An exception must be thrown.");
                } catch (MacMapConflictException e) {
                    assertEquals(mvlan, e.getHost());
                    assertEquals(ref0, e.getMapReference());
                } catch (Exception e) {
                    unexpected(e);
                }
                revision = checkRevision(revMap, false, revision,
                                         updateFailure);
                mc0.checkCache(cs, container);
                mc1.checkCache(cs, container);
                assertEquals(ref0, grsc.getMapReference(mvlan));
            }

            // Iterate switch ports, and map one host on VLAN 2 to mc1.
            for (NodeConnector port: allPorts) {
                for (MacVlan mvlan: portHosts.get(port)) {
                    if (mvlan.getVlan() == 2) {
                        try {
                            mc1.addAllowed(mvlan);
                            revMap.activateTest(updateFailure);
                            mc1.change(mgr);
                            revision = checkRevision(revMap, true, revision,
                                                     updateFailure);
                            assertEquals(ref1, grsc.getMapReference(mvlan));

                            revMap.activateTest(updateFailure);
                            mc1.activate(mgr, mvlan, port);
                            revision = checkRevision(revMap, true, revision,
                                                     updateFailure);
                        } catch (Exception e) {
                            unexpected(e);
                        }
                        mc0.checkCache(cs, container);
                        mc1.checkCache(cs, container);
                        break;
                    }
                }
            }

            // Ensure that MacMapConflictException does not affect to the
            // MAC mapping configuration.
            for (MacVlan mvlan: mc0.getAllowedHosts()) {
                MacMapConf mc = mc1.clone();
                mc.addAllowed(mvlan).
                    addAllowed(new MacVlan(0xffffffffL, (short)0)).
                    addDenied(new MacVlan(0x12345L, (short)4095));
                try {
                    revMap.activateTest(updateFailure);
                    mc.change(mgr);
                    fail("An exception must be thrown.");
                } catch (MacMapConflictException e) {
                    assertEquals(mvlan, e.getHost());
                    assertEquals(ref0, e.getMapReference());
                } catch (Exception e) {
                    unexpected(e);
                }
                revision = checkRevision(revMap, false, revision,
                                         updateFailure);
                mc0.checkCache(cs, container);
                mc1.checkCache(cs, container);
                assertEquals(ref0, grsc.getMapReference(mvlan));
            }

            // Remove 2 hosts from allowed set in mc1.
            int count = 0;
            hset.clear();
            for (MacVlan mvlan: new HashSet<MacVlan>(mc1.getAllowedHosts())) {
                mc1.removeAllowed(mvlan);
                hset.add(mvlan);
                count++;
                if (count >= 2) {
                    break;
                }
            }
            try {
                revMap.activateTest(updateFailure);
                mc1.change(mgr);
            } catch (Exception e) {
                unexpected(e);
            }
            revision = checkRevision(revMap, true, revision, updateFailure);
            mc0.checkCache(cs, container);
            mc1.checkCache(cs, container);
            for (MacVlan mvlan: hset) {
                assertEquals(null, grsc.getMapReference(mvlan));
            }

            // Add arbitrary hosts to denied host set.
            Set<MacVlan> hset0 = new HashSet<MacVlan>();
            hset0.add(new MacVlan(0x12345678L, (short)100));
            hset0.add(new MacVlan(0x12345678L, (short)101));
            Set<MacVlan> hset1 = new HashSet<MacVlan>(hset0);
            hset0.add(new MacVlan(0xf00000000000L, (short)0));
            hset1.add(new MacVlan(0x100000000000L, (short)999));

            for (MacVlan mvlan: hset0) {
                mc0.addDenied(mvlan);
                try {
                    revMap.activateTest(updateFailure);
                    mc0.change(mgr);
                } catch (Exception e) {
                    unexpected(e);
                }
                revision = checkRevision(revMap, true, revision,
                                         updateFailure);
                mc0.checkCache(cs, container);
                mc1.checkCache(cs, container);
            }

            for (MacVlan mvlan: hset1) {
                mc1.addDenied(mvlan);
            }
            try {
                revMap.activateTest(updateFailure);
                mc1.change(mgr);
            } catch (Exception e) {
                unexpected(e);
            }
            revision = checkRevision(revMap, true, revision, updateFailure);
            mc0.checkCache(cs, container);
            mc1.checkCache(cs, container);

            // Try to add hosts that are already configured.
            Set<MacVlan> emptySet = new HashSet<MacVlan>();
            MacMapChange ch = new MacMapChange(mc0.getAllowedHosts(), emptySet,
                                               mc0.getDeniedHosts(), emptySet,
                                               0);
            try {
                revMap.activateTest(updateFailure);
                grsc.registerMacMap(mgr, mc0.getPath(), ch);
            } catch (Exception e) {
                unexpected(e);
            }
            revision = checkRevision(revMap, true, revision, updateFailure);

            // Try to remove unexpected host.
            MacVlan unexpectedHost = new MacVlan(0x7777777777L, (short)1000);
            hset.clear();
            hset.add(unexpectedHost);
            try {
                revMap.activateTest(updateFailure);
                ch = new MacMapChange(emptySet, hset, emptySet, emptySet, 0);
                grsc.registerMacMap(mgr, mc0.getPath(), ch);
                fail("An exception must be thrown.");
            } catch (VTNException e) {
                Status st = e.getStatus();
                assertEquals(StatusCode.NOTFOUND, st.getCode());
            }
            revision = checkRevision(revMap, false, revision, updateFailure);

            try {
                revMap.activateTest(updateFailure);
                ch = new MacMapChange(emptySet, emptySet, emptySet, hset, 0);
                grsc.registerMacMap(mgr, mc0.getPath(), ch);
                fail("An exception must be thrown.");
            } catch (VTNException e) {
                Status st = e.getStatus();
                assertEquals(StatusCode.NOTFOUND, st.getCode());
            }
            revision = checkRevision(revMap, false, revision, updateFailure);

            // Remove hosts from denied from set in mc0.
            for (MacVlan mvlan: hset0) {
                mc0.removeDenied(mvlan);
            }
            try {
                revMap.activateTest(updateFailure);
                mc0.change(mgr);
            } catch (Exception e) {
                unexpected(e);
            }
            revision = checkRevision(revMap, true, revision, updateFailure);
            mc0.checkCache(cs, container);
            mc1.checkCache(cs, container);

            // Destory MAC mappings.
            mc1.remove();
            try {
                revMap.activateTest(updateFailure);
                mc1.change(mgr);
            } catch (Exception e) {
                unexpected(e);
            }
            revision = checkRevision(revMap, true, revision, updateFailure);
            mc0.checkCache(cs, container);
            mc1.checkCache(cs, container);

            mc0.remove();
            try {
                revMap.activateTest(updateFailure);
                mc0.change(mgr);
            } catch (Exception e) {
                unexpected(e);
            }
            revision = checkRevision(revMap, true, revision, updateFailure);
            mc0.checkCache(cs, container);
            mc1.checkCache(cs, container);
        }
    }

    /**
     * Test case for
     * {@link GlobalResourceManager#getMapReference(byte[], NodeConnector, short)}.
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
        ArrayList<Node> nodes = new ArrayList<Node>();
        HashMap<Node, List<NodeConnector>> nodePorts =
            new HashMap<Node, List<NodeConnector>>();
        ArrayList<NodeConnector> allPorts = new ArrayList<NodeConnector>();

        for (long dpid = 0; dpid < 3; dpid++) {
            Node node = NodeCreator.createOFNode(Long.valueOf(dpid));
            assertNotNull(node);
            nodes.add(node);

            ArrayList<NodeConnector> portList = new ArrayList<NodeConnector>();
            assertNull(nodePorts.put(node, portList));
            for (short id = 1; id <= 3; id++) {
                NodeConnector port = NodeConnectorCreator.
                    createOFNodeConnector(Short.valueOf(id), node);
                assertNotNull(port);
                allPorts.add(port);
                portList.add(port);
            }
            portList.trimToSize();
        }
        allPorts.trimToSize();

        // Create 2 MAC addresses per port.
        // Each MAC address is on VLAN 0, 1, and 2.
        Map<NodeConnector, List<MacVlan>> portHosts =
            new HashMap<NodeConnector, List<MacVlan>>();
        Map<MacVlan, NodeConnector> allHosts =
            new HashMap<MacVlan, NodeConnector>();
        Map<MacVlan, NodeConnector> unmappedHosts =
            new HashMap<MacVlan, NodeConnector>();

        long macAddr = 1L;
        for (NodeConnector port: allPorts) {
            ArrayList<MacVlan> hostList = new ArrayList<MacVlan>();
            assertNull(portHosts.put(port, hostList));
            for (int i = 0; i < 2; i++) {
                for (short vlan = 0; vlan <= 2; vlan++) {
                    MacVlan mvlan = new MacVlan(macAddr, vlan);
                    hostList.add(mvlan);
                    assertNull(allHosts.put(mvlan, port));
                }
                macAddr++;
            }

            // Create hosts that are never mapped to vBridge.
            for (short vlan = 4094; vlan <= 4095; vlan++) {
                MacVlan mvlan = new MacVlan(macAddr, vlan);
                assertNull(unmappedHosts.put(mvlan, port));
            }

            macAddr++;
            hostList.trimToSize();
        }

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

        Map<MacVlan, MapReference> expected =
            new HashMap<MacVlan, MapReference>();

        try {
            // Establish VLAN mappings.

            // Map all VLAN networks to bridges[0] in container[0].
            VTNManagerImpl mgr = managers.get(0);
            VBridgePath bpath = bridges.get(0);
            for (short vlan = 0; vlan <= 2; vlan++) {
                MapReference vmap = addVlanMap(grsc, mgr, bpath, null, vlan);

                // All hosts should be mapped by VLAN mapping.
                for (MacVlan mvlan: allHosts.keySet()) {
                    if (mvlan.getVlan() == vlan) {
                        expected.put(mvlan, vmap);
                    }
                }
                checkMapReference(grsc, allHosts, unmappedHosts, expected);
            }

            // Map VLAN 1 on node[2] to bridge[4] in container[2].
            mgr = managers.get(0);
            bpath = bridges.get(4);
            Node node = nodes.get(2);
            MapReference vmap = addVlanMap(grsc, mgr, bpath, node, (short)1);
            for (NodeConnector nc: nodePorts.get(node)) {
                for (MacVlan mvlan: portHosts.get(nc)) {
                    if (mvlan.getVlan() == 1) {
                        expected.put(mvlan, vmap);
                    }
                }
            }
            checkMapReference(grsc, allHosts, unmappedHosts, expected);

            // Map VLAN 0 on node[0] to bridge[6] in container[3].
            mgr = managers.get(3);
            bpath = bridges.get(6);
            node = nodes.get(0);
            MapReference vmap0Node0 = addVlanMap(grsc, mgr, bpath, node,
                                                 (short)0);
            for (NodeConnector nc: nodePorts.get(node)) {
                for (MacVlan mvlan: portHosts.get(nc)) {
                    if (mvlan.getVlan() == 0) {
                        expected.put(mvlan, vmap0Node0);
                    }
                }
            }
            checkMapReference(grsc, allHosts, unmappedHosts, expected);

            // Establish port mappings.

            // Map VLAN 0 on port[4] to interface[1] in container[1].
            mgr = managers.get(1);
            VBridgeIfPath ipath = interfaces.get(1);
            NodeConnector port = allPorts.get(4);
            MapReference pmap0Port4 = addPortMap(grsc, mgr, ipath, port,
                                                 (short)0);
            for (MacVlan mvlan: portHosts.get(port)) {
                if (mvlan.getVlan() == 0) {
                    expected.put(mvlan, pmap0Port4);
                }
            }
            checkMapReference(grsc, allHosts, unmappedHosts, expected);

            // Map VLAN 2 on port[4] to interface[4] in container[0].
            mgr = managers.get(0);
            ipath = interfaces.get(4);
            MapReference pmap2Port4 = addPortMap(grsc, mgr, ipath, port,
                                                 (short)2);
            for (MacVlan mvlan: portHosts.get(port)) {
                if (mvlan.getVlan() == 2) {
                    expected.put(mvlan, pmap2Port4);
                }
            }
            checkMapReference(grsc, allHosts, unmappedHosts, expected);

            // Establish MAC mapping.

            // Map all hosts on VLAN 2 to bridge[2] in container[2] except for
            // hosts on port[7].
            mgr = managers.get(2);
            bpath = bridges.get(2);
            port = allPorts.get(7);
            Map<MacVlan, MapReference> save =
                new HashMap<MacVlan, MapReference>(expected);
            MacMapConf mc = new MacMapConf(bpath);
            Set<MacVlan> mmapped2 = new HashSet<MacVlan>();
            MapReference mmap = mc.getMapReference(mgr);
            mc.addAllowed(new MacVlan(MacVlan.UNDEFINED, (short)2));
            for (Map.Entry<MacVlan, NodeConnector> entry:
                     allHosts.entrySet()) {
                MacVlan mvlan = entry.getKey();
                if (mvlan.getVlan() != 2) {
                    continue;
                }

                NodeConnector nc = entry.getValue();
                if (nc.equals(port)) {
                    mc.addDenied(mvlan);
                    continue;
                }

                // Port mapping should precede MAC mapping.
                MapReference ref = expected.get(mvlan);
                if (ref.getMapType() != MapType.PORT) {
                    assertTrue(mmapped2.add(mvlan));
                    expected.put(mvlan, mmap);
                }
            }
            assertFalse(mmapped2.isEmpty());
            mc.change(mgr);
            checkMapReference(grsc, allHosts, unmappedHosts, expected);

            // Map hosts on VLAN 0 at port[1] to the same vBridge.
            Set<MacVlan> mmapped0 = new HashSet<MacVlan>();
            port = allPorts.get(1);
            for (MacVlan mvlan: portHosts.get(port)) {
                if (mvlan.getVlan() == 0) {
                    mc.addAllowed(mvlan);
                    assertTrue(mmapped0.add(mvlan));
                    expected.put(mvlan, mmap);
                }
            }
            assertTrue(mmapped0.size() > 1);
            mc.change(mgr);
            checkMapReference(grsc, allHosts, unmappedHosts, expected);

            // Map hosts on VLAN 1 at port[5] to the same vBridge.
            Set<MacVlan> mmapped1 = new HashSet<MacVlan>();
            port = allPorts.get(5);
            for (MacVlan mvlan: portHosts.get(port)) {
                if (mvlan.getVlan() == 1) {
                    mc.addAllowed(mvlan);
                    assertTrue(mmapped1.add(mvlan));
                    expected.put(mvlan, mmap);
                }
            }
            assertFalse(mmapped1.isEmpty());
            mc.change(mgr);
            checkMapReference(grsc, allHosts, unmappedHosts, expected);

            // Activate MAC mappings.
            //   - Host on VLAN 0 at port[1].
            //   - Host on VLAN 2.
            Map<Long, MacVlan> mmappedAddr = new HashMap<Long, MacVlan>();
            MacMapPath mpath = mc.getPath();
            MacVlan rmhost = null;
            for (MacVlan mvlan: mmapped0) {
                if (rmhost == null) {
                    rmhost = mvlan;
                }
                assertNull(mmappedAddr.put(mvlan.getMacAddress(), mvlan));
                NodeConnector nc = allHosts.get(mvlan);
                mc.activate(mgr, mvlan, nc);
            }

            Set<PortVlan> reserved = new HashSet<PortVlan>();
            for (MacVlan mvlan: mmapped2) {
                NodeConnector nc = allHosts.get(mvlan);
                Long key = Long.valueOf(mvlan.getMacAddress());
                try {
                    mc.activate(mgr, mvlan, nc);
                    assertNull(mmappedAddr.put(key, mvlan));
                    assertEquals(nc,
                                 grsc.getMacMappedPort(mgr, mpath, mvlan));
                    reserved.add(new PortVlan(nc, mvlan.getVlan()));
                } catch (MacMapDuplicateException e) {
                    // This host should not be mapped by this MAC mapping
                    // because the same MAC address is already activated.
                    assertEquals(mmappedAddr.get(key), e.getDuplicate());
                    assertEquals(mvlan, e.getHost());
                    assertEquals(mmap, e.getMapReference());
                    assertNull(grsc.getMacMappedPort(mgr, mpath, mvlan));
                    expected.put(mvlan, save.get(mvlan));
                }
            }
            for (MacVlan mvlan: mmapped1) {
                Long key = Long.valueOf(mvlan.getMacAddress());
                if (mmappedAddr.containsKey(key)) {
                    // This host should not be mapped by this MAC mapping
                    // because the same MAC address is already activated.
                    expected.put(mvlan, save.get(mvlan));
                }
            }
            checkMapReference(grsc, allHosts, unmappedHosts, expected);

            // Unmap rmhost, and map to bridge[5] in container[3], but it
            // should not be mapped by any mapping because VLAN on a switch
            // port is reserved by the MAC mapping previously configured.
            port = allHosts.get(rmhost);
            assertEquals(port,
                         grsc.getMacMappedPort(mgr, mpath, rmhost));
            mc.removeAllowed(rmhost);
            expected.put(rmhost, null);

            // A host that has the same MAC address as rmhost and is on VLAN 2
            // will be mapped by wildcard MAC mapping.
            expected.put(new MacVlan(rmhost.getMacAddress(), (short)2), mmap);

            mc.change(mgr);
            checkMapReference(grsc, allHosts, unmappedHosts, expected);
            assertNull(grsc.getMacMappedPort(mgr, mpath, rmhost));
            PortVlan pvlan = new PortVlan(port, rmhost.getVlan());
            assertEquals(mmap, grsc.getMapReference(pvlan));

            VTNManagerImpl mgr3 = managers.get(3);
            bpath = bridges.get(5);
            port = allPorts.get(3);
            MacMapConf mc1 = new MacMapConf(bpath);
            MapReference mmap1 = mc1.getMapReference(mgr3);
            MacMapPath mpath1 = mc1.getPath();
            mc1.addAllowed(rmhost);
            mc1.change(mgr3);
            checkMapReference(grsc, allHosts, unmappedHosts, expected);
            assertNull(grsc.getMacMappedPort(mgr3, mpath1, rmhost));

            // Ensure that rmhost can not be activated.
            try {
                mc1.activate(mgr3, rmhost, allHosts.get(rmhost));
                fail("An exception must be thrown.");
            } catch (MacMapPortBusyException e) {
                assertEquals(mmap, e.getAnotherMapping());
                assertEquals(rmhost, e.getHost());
                assertEquals(mmap1, e.getMapReference());
            }

            // Map hosts on VLAN 2 at port[3] to bridge[5] in container[3].
            pvlan = new PortVlan(port, (short)2);
            assertEquals(mmap, grsc.getMapReference(pvlan));
            for (MacVlan mvlan: portHosts.get(port)) {
                if (mvlan.getVlan() == 2) {
                    assertEquals(port,
                                 grsc.getMacMappedPort(mgr, mpath, mvlan));
                    mc1.addAllowed(mvlan);
                    expected.put(mvlan, mmap1);
                }
            }
            mc1.change(mgr3);

            // Specific MAC mapping should supersede wildcard mapping.
            assertEquals(null, grsc.getMapReference(pvlan));
            for (MacVlan mvlan: portHosts.get(port)) {
                if (mvlan.getVlan() == 2) {
                    assertNull(grsc.getMacMappedPort(mgr, mpath, mvlan));
                }
            }
            checkMapReference(grsc, allHosts, unmappedHosts, expected);

            // Map VLAN 0 on port[1] to interface[13] in container[1].
            // This will inactivate MAC mappings on port[1].
            VTNManagerImpl mgr1 = managers.get(1);
            ipath = interfaces.get(13);
            port = allPorts.get(1);
            pvlan = new PortVlan(port, (short)0);
            assertEquals(port, allHosts.get(rmhost));
            assertEquals(mmap, grsc.getMapReference(pvlan));
            for (MacVlan mvlan: portHosts.get(port)) {
                if (mvlan.getVlan() == 0) {
                    if (mvlan.equals(rmhost)) {
                        assertEquals(mmap1, grsc.getMapReference(mvlan));
                        assertNull(grsc.getMacMappedPort(mgr, mpath, mvlan));
                        assertNull(grsc.getMacMappedPort(mgr3, mpath1, mvlan));
                    } else {
                        assertEquals(mmap, grsc.getMapReference(mvlan));
                        assertEquals(port,
                                     grsc.getMacMappedPort(mgr, mpath, mvlan));
                    }
                }
            }
            MapReference pmap0If13 = addPortMap(grsc, mgr1, ipath, port,
                                                (short)0);
            for (MacVlan mvlan: portHosts.get(port)) {
                if (mvlan.getVlan() == 0) {
                    expected.put(mvlan, pmap0If13);
                    assertNull(grsc.getMacMappedPort(mgr, mpath, mvlan));
                }
            }

            // Hosts on VLAN 2 at port[1] will be mapped by MAC mapping because
            // hosts which have the same MAC address is no longer mapped.
            for (MacVlan mvlan: mmapped2) {
                if (!mc1.isMapped(mvlan)) {
                    expected.put(mvlan, mmap);
                }
            }

            checkMapReference(grsc, allHosts, unmappedHosts, expected);
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
            VBridgePath bpath1 = new VBridgePath(tname, bname + "1");
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

            NodeConnector nc = NodeConnectorCreator.
                createOFNodeConnector(Short.valueOf((short)10), node0);
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

            MacMapConf mc0 = new MacMapConf(bpath);
            MacMapPath mpath0 = mc0.getPath();
            MacVlan allowed0 = new MacVlan(1L, (short)4095);
            MacVlan denied0 = new MacVlan(2L, (short)4094);
            MacVlan denied1 = new MacVlan(4L, (short)4092);
            mc0.addAllowed(allowed0).addDenied(denied0).addDenied(denied1);
            try {
                mc0.change(mgr);
                mc0.activate(mgr, allowed0, nc);
            } catch (Exception e) {
                unexpected(e);
            }
            mc0.checkCache(cs, containerName);

            // Register one more VLAN, port, and MAC mapping into other
            // container.
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

            MacMapConf mc1 = new MacMapConf(bpath1);
            MacMapPath mpath1 = mc1.getPath();
            MacVlan allowed1 = new MacVlan(3L, (short)4093);
            mc1.addAllowed(allowed1).addDenied(denied1);
            try {
                mc1.change(otherMgr);
                mc1.activate(otherMgr, allowed1, nc);
            } catch (Exception e) {
                unexpected(e);
            }
            mc1.checkCache(cs, other);

            ConcurrentMap<Short, MapReference> vmap =
                (ConcurrentMap<Short, MapReference>)cs.getCache(CACHE_VLANMAP);
            ConcurrentMap<PortVlan, MapReference> pmap =
                (ConcurrentMap<PortVlan, MapReference>)
                cs.getCache(CACHE_PORTMAP);
            ConcurrentMap<MacVlan, MapReference> macAllowed =
                (ConcurrentMap<MacVlan, MapReference>)cs.getCache(CACHE_MACMAP);
            ConcurrentMap<MacVlan, Set<MapReference>> macDenied =
                (ConcurrentMap<MacVlan, Set<MapReference>>)
                cs.getCache(CACHE_MACMAP_DENY);
            ConcurrentMap<MapReference, MacMapState> macState =
                (ConcurrentMap<MapReference, MacMapState>)
                cs.getCache(CACHE_MACMAP_STATE);
            assertEquals(2, vmap.size());
            assertEquals(4, pmap.size());
            assertEquals(2, macAllowed.size());
            assertEquals(2, macDenied.size());
            assertEquals(2, macState.size());

            int revision = getRevision(revMap);
            revMap.activateTest(updateFailure);
            grsc.cleanUp("clean");
            revision = checkRevision(revMap, false, revision, updateFailure);
            assertEquals(2, vmap.size());
            assertEquals(4, pmap.size());
            assertEquals(2, macAllowed.size());
            assertEquals(2, macDenied.size());
            assertEquals(2, macState.size());
            mc0.checkCache(cs, containerName);
            mc1.checkCache(cs, other);

            revMap.activateTest(updateFailure);
            grsc.cleanUp(containerName);
            revision = checkRevision(revMap, true, revision, updateFailure);
            assertEquals(1, vmap.size());
            assertEquals(2, pmap.size());
            assertEquals(1, macAllowed.size());
            assertEquals(1, macDenied.size());
            assertEquals(1, macState.size());
            mc0.clear();
            mc0.checkCache(cs, containerName);
            mc1.checkCache(cs, other);

            revMap.activateTest(updateFailure);
            grsc.cleanUp(other);
            revision = checkRevision(revMap, true, revision, updateFailure);
            assertEquals(0, vmap.size());
            assertEquals(0, pmap.size());
            assertEquals(0, macAllowed.size());
            assertEquals(0, macDenied.size());
            assertEquals(0, macState.size());
            mc1.clear();
            mc0.checkCache(cs, containerName);
            mc1.checkCache(cs, other);
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
            public synchronized void run() {
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
        InetAddress otherAddr = getInetAddressFromAddress(
            new byte[] {(byte)10, (byte)0, (byte)1, (byte)99});

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
        MacTableEntry entLoopback = new MacTableEntry(eid, nc, (short)0,
                                                      loopback);

        vtnMgr.putMacTableEntry(entLoopback);

        List<MacAddressEntry> entries = getMacEntries(vtnMgr, bpath);
        assertEquals(0, entries.size());

        InetAddress ipaddr
            = getInetAddressFromAddress(new byte[] {(byte)192, (byte)168,
                                                    (byte)0, (byte)2});
        mac = 2L;
        MacTableEntryId eidRemote = new MacTableEntryId(ipaddr, 2L, bpath, mac);
        MacTableEntry entRemote = new MacTableEntry(eidRemote, nc, (short)0,
                                                    ipaddr);

        ipaddr = getInetAddressFromAddress(new byte[] {(byte)192, (byte)168,
                                                       (byte)0, (byte)3});
        mac = 3L;
        MacTableEntryId eidRemote2 = new MacTableEntryId(ipaddr, 3L, bpath,
                                                         mac);
        MacTableEntry entRemote2 = new MacTableEntry(eidRemote2, nc, (short)0,
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
        mgr.setRouting(stubObj);
        mgr.init(c);
        mgr.clearDisabledNode();
        vtnManagers.add(mgr);

        return mgr;
    }

    /**
     * Check cluster cache in {@link GlobalResourceManager}.
     *
     * @param cs             A {@link IClusterGlobalServices} object.
     * @param type           Type of the virtual mapping.
     * @param containerName  The name of the container.
     *                       If {@code null} is specified, this method ensures
     *                       that the specified key does not exist in the
     *                       cache.
     * @param path           Path to the virtual mapping.
     * @param key            A key of the cache.
     * @param <T>  The type of the key for cluster cache.
     */
    private <T> void checkMapCache(IClusterGlobalServices cs, MapType type,
                                   String containerName, VBridgePath path,
                                   T key) {
        String cacheName;
        if (type == MapType.VLAN) {
            cacheName = CACHE_VLANMAP;
        } else {
            assertEquals(MapType.PORT, type);
            cacheName = CACHE_PORTMAP;
        }

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
            assertEquals(cnt, revMap.getGetCount());
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
     * @param resMgr  Global resource manager service.
     * @param mgr     VTN Manager service.
     * @param ipath   A path to the target vBridge interface.
     * @param port    A {@link NodeConnector} corresponding to the switch port.
     * @param vlan    A VLAN ID.
     * @return        A {@link MapReference} instance which points to a new
     *                port maping is returned.
     * @throws VTNException    An error occurred.
     * @see  #testGetMapReference()
     */
    private MapReference addPortMap(IVTNResourceManager resMgr,
                                    VTNManagerImpl mgr, VBridgeIfPath ipath,
                                    NodeConnector port, short vlan)
        throws VTNException {
        PortVlan pvlan = new PortVlan(port, vlan);
        assertNull(resMgr.registerPortMap(mgr, ipath, pvlan, null, true));

        MapReference ref = mgr.getMapReference(ipath);
        assertEquals(MapType.PORT, ref.getMapType());
        assertEquals(mgr.getContainerName(), ref.getContainerName());
        assertEquals(ipath, ref.getPath());

        return ref;
    }

    /**
     * Register a new VLAN mapping.
     *
     * @param resMgr  Global resource manager service.
     * @param mgr     VTN Manager service.
     * @param bpath   A path to the target vBridge.
     * @param node    A {@link Node} corresponding to the switch.
     * @param vlan    A VLAN ID.
     * @return        A {@link MapReference} instance which points to a new
     *                VLAN mapping is returned.
     * @throws VTNException    An error occurred.
     * @see  #testGetMapReference()
     */
    private MapReference addVlanMap(IVTNResourceManager resMgr,
                                    VTNManagerImpl mgr, VBridgePath bpath,
                                    Node node, short vlan)
        throws VTNException {
        NodeVlan nvlan = new NodeVlan(node, vlan);
        String mapId = createVlanMapId(nvlan);
        VlanMapPath vpath = new VlanMapPath(bpath, mapId);
        assertNull(resMgr.registerVlanMap(mgr, vpath, nvlan, true));

        MapReference ref = mgr.getMapReference(vpath);
        assertEquals(MapType.VLAN, ref.getMapType());
        assertEquals(mgr.getContainerName(), ref.getContainerName());
        assertEquals(vpath, ref.getPath());

        return ref;
    }

    /**
     * Ensure that the incoming packet is routed as appropriate.
     *
     * @param resMgr    Global resource manager service.
     * @param hosts     A map which keeps pairs of hosts and node connector.
     * @param unmapped  A map which keeps host information that are not mapped
     *                  to any vBridge.
     * @param expected  A map which keeps hosts to be tested and virtual
     *                  mapping which maps the host.
     * @see  #testGetMapReference()
     */
    private void checkMapReference(IVTNResourceManager resMgr,
                                   Map<MacVlan, NodeConnector> hosts,
                                   Map<MacVlan, NodeConnector> unmapped,
                                   Map<MacVlan, MapReference> expected) {
        for (Map.Entry<MacVlan, NodeConnector> entry: hosts.entrySet()) {
            MacVlan mvlan = entry.getKey();
            NodeConnector port = entry.getValue();
            byte[] addr = NetUtils.longToByteArray6(mvlan.getMacAddress());
            short vlan = mvlan.getVlan();
            MapReference ref = expected.get(mvlan);
            assertEquals("mvlan=" + mvlan + ", port=" + port,
                         ref, resMgr.getMapReference(addr, port, vlan));
        }

        for (Map.Entry<MacVlan, NodeConnector> entry: unmapped.entrySet()) {
            MacVlan mvlan = entry.getKey();
            NodeConnector port = entry.getValue();
            byte[] addr = NetUtils.longToByteArray6(mvlan.getMacAddress());
            short vlan = mvlan.getVlan();
            assertEquals(null, resMgr.getMapReference(addr, port, vlan));
        }
    }
}
