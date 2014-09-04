/*
 * Copyright (c) 2013-2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal;

import java.io.File;
import java.io.IOException;
import java.net.InetAddress;
import java.net.UnknownHostException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Hashtable;
import java.util.Iterator;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ConcurrentMap;

import org.apache.felix.dm.impl.ComponentImpl;

import org.junit.Test;
import org.junit.experimental.categories.Category;

import org.opendaylight.vtn.manager.DataLinkHost;
import org.opendaylight.vtn.manager.EthernetHost;
import org.opendaylight.vtn.manager.MacAddressEntry;
import org.opendaylight.vtn.manager.MacMap;
import org.opendaylight.vtn.manager.MacMapAclType;
import org.opendaylight.vtn.manager.MacMapConfig;
import org.opendaylight.vtn.manager.PortMap;
import org.opendaylight.vtn.manager.PortMapConfig;
import org.opendaylight.vtn.manager.SwitchPort;
import org.opendaylight.vtn.manager.UpdateOperation;
import org.opendaylight.vtn.manager.VBridge;
import org.opendaylight.vtn.manager.VBridgeConfig;
import org.opendaylight.vtn.manager.VBridgeIfPath;
import org.opendaylight.vtn.manager.VBridgePath;
import org.opendaylight.vtn.manager.VInterface;
import org.opendaylight.vtn.manager.VInterfaceConfig;
import org.opendaylight.vtn.manager.VNodeState;
import org.opendaylight.vtn.manager.VTNException;
import org.opendaylight.vtn.manager.VTenant;
import org.opendaylight.vtn.manager.VTenantPath;
import org.opendaylight.vtn.manager.VTerminalIfPath;
import org.opendaylight.vtn.manager.VTerminalPath;
import org.opendaylight.vtn.manager.VlanMap;
import org.opendaylight.vtn.manager.VlanMapConfig;
import org.opendaylight.vtn.manager.internal.cluster.FlowGroupId;
import org.opendaylight.vtn.manager.internal.cluster.MacMapPath;
import org.opendaylight.vtn.manager.internal.cluster.MacTableEntry;
import org.opendaylight.vtn.manager.internal.cluster.MacVlan;
import org.opendaylight.vtn.manager.internal.cluster.MapReference;
import org.opendaylight.vtn.manager.internal.cluster.MapType;
import org.opendaylight.vtn.manager.internal.cluster.ObjectPair;
import org.opendaylight.vtn.manager.internal.cluster.PortProperty;
import org.opendaylight.vtn.manager.internal.cluster.PortVlan;
import org.opendaylight.vtn.manager.internal.cluster.VTNFlow;
import org.opendaylight.vtn.manager.internal.cluster.VlanMapPath;

import org.opendaylight.controller.clustering.services.IClusterContainerServices;
import org.opendaylight.controller.forwardingrulesmanager.FlowEntry;
import org.opendaylight.controller.hosttracker.hostAware.HostNodeConnector;
import org.opendaylight.controller.sal.core.Config;
import org.opendaylight.controller.sal.core.ConstructionException;
import org.opendaylight.controller.sal.core.Edge;
import org.opendaylight.controller.sal.core.Name;
import org.opendaylight.controller.sal.core.Node;
import org.opendaylight.controller.sal.core.NodeConnector;
import org.opendaylight.controller.sal.core.NodeConnector.NodeConnectorIDType;
import org.opendaylight.controller.sal.core.Property;
import org.opendaylight.controller.sal.core.State;
import org.opendaylight.controller.sal.core.UpdateType;
import org.opendaylight.controller.sal.packet.ARP;
import org.opendaylight.controller.sal.packet.Ethernet;
import org.opendaylight.controller.sal.packet.IEEE8021Q;
import org.opendaylight.controller.sal.packet.Packet;
import org.opendaylight.controller.sal.packet.PacketResult;
import org.opendaylight.controller.sal.packet.RawPacket;
import org.opendaylight.controller.sal.packet.address.EthernetAddress;
import org.opendaylight.controller.sal.topology.TopoEdgeUpdate;
import org.opendaylight.controller.sal.utils.EtherTypes;
import org.opendaylight.controller.sal.utils.NetUtils;
import org.opendaylight.controller.sal.utils.NodeConnectorCreator;
import org.opendaylight.controller.sal.utils.NodeCreator;
import org.opendaylight.controller.sal.utils.Status;
import org.opendaylight.controller.sal.utils.StatusCode;
import org.opendaylight.controller.switchmanager.ISwitchManager;
import org.opendaylight.controller.topologymanager.ITopologyManager;

/**
 * JUnit test for {@link VTNManagerImplTest}.
 *
 * <p>
 * This test class test with using stub which simulate working
 * on the environment some nodes exist.
 * Tests of {@link org.opendaylight.vtn.manager.internal.cluster.VTenantImpl},
 * {@link org.opendaylight.vtn.manager.internal.cluster.VBridgeImpl},
 * {@link org.opendaylight.vtn.manager.internal.cluster.VBridgeIfImpl},
 * {@link org.opendaylight.vtn.manager.internal.cluster.VlanMapImpl} are also
 * implemented in this class.
 * </p>
 */
@Category(SlowTest.class)
public class VTNManagerImplWithNodesTest extends VTNManagerImplTestCommon {
    /**
     * Construct a new instance.
     */
    public VTNManagerImplWithNodesTest() {
        super(2, true);
    }

    /**
     * Test method for
     * {@link VTNManagerImpl#init(org.apache.felix.dm.Component)},
     * {@link VTNManagerImpl#destroy()}.
     * {@code resume()} method of some modules are also tested.
     */
    @Test
    public void testInitDestory() {
        ComponentImpl c = new ComponentImpl(null, null, null);
        Hashtable<String, String> properties = new Hashtable<String, String>();
        String containerName = "default";
        properties.put("containerName", containerName);
        c.setServiceProperties(properties);

        // create dummy file in startup directory.
        File dir = getTenantConfigDir(containerName);
        String suffix = ".conf";
        String[] dummyFileNames = new String[] {
            suffix,
            "vtn" + suffix,
            "vtn.config",
            "config",
            "notexist" + suffix
        };
        for (String fileName : dummyFileNames) {
            File file = new File(dir, fileName);
            try {
                assertTrue(file.createNewFile());
            } catch (IOException e) {
                unexpected(e);
            }
        }

        Map<VBridgeIfPath, PortMapConfig> pmaps = new HashMap<VBridgeIfPath, PortMapConfig>();
        Map<VlanMap, VlanMapConfig> vmaps = new HashMap<VlanMap, VlanMapConfig>();
        Map<VBridgePath, VNodeState> brstates = new HashMap<VBridgePath, VNodeState>();
        Map<VBridgeIfPath, VNodeState> ifstates = new HashMap<VBridgeIfPath, VNodeState>();

        // add VTN config
        String tname = "vtn";
        VTenantPath tpath = new VTenantPath(tname);
        String bname = "vbridge";
        VBridgePath bpath = new VBridgePath(tname, bname);
        VBridgeIfPath ifpath1 = new VBridgeIfPath(tname, bname, "vif1");
        VBridgeIfPath ifpath2 = new VBridgeIfPath(tname, bname, "vif2");
        VBridgeIfPath ifpath3 = new VBridgeIfPath(tname, bname, "vif3");

        List<VBridgePath> bpathlist = new ArrayList<VBridgePath>();
        List<VBridgeIfPath> ifpathlist = new ArrayList<VBridgeIfPath>();
        bpathlist.add(bpath);
        ifpathlist.add(ifpath1);
        ifpathlist.add(ifpath3);
        createTenantAndBridgeAndInterface(vtnMgr, tpath, bpathlist, ifpathlist);

        Status st = vtnMgr.addInterface(
            ifpath2, new VInterfaceConfig(null, Boolean.FALSE));
        assertEquals(StatusCode.SUCCESS, st.getCode());
        ifpathlist.add(ifpath2);

        // restart VTNManager and check the configuration after restart.
        restartVTNManager(c);

        pmaps.put(ifpath1, null);
        pmaps.put(ifpath2, null);
        pmaps.put(ifpath3, null);
        checkVTNconfig(vtnMgr, tpath, bpathlist, pmaps, null, null);

        // add mapping
        Node node = NodeCreator.createOFNode(0L);
        SwitchPort port = new SwitchPort(NodeConnectorIDType.OPENFLOW,
                                         String.valueOf(10));
        PortMapConfig pmconf = new PortMapConfig(node, port, (short)0);
        st = vtnMgr.setPortMap(ifpath1, pmconf);
        assertEquals(StatusCode.SUCCESS, st.getCode());
        pmaps.put(ifpath1, pmconf);
        pmaps.put(ifpath2, null);

        VlanMapConfig vlconf = new VlanMapConfig(null, (short)1);
        VlanMap map = null;
        try {
            map = vtnMgr.addVlanMap(bpath, vlconf);
        } catch (VTNException e) {
            unexpected(e);
        }
        vmaps.put(map, vlconf);

        vlconf = new VlanMapConfig(null, (short)4095);
        map = null;
        try {
            map = vtnMgr.addVlanMap(bpath, vlconf);
        } catch (VTNException e) {
            unexpected(e);
        }
        vmaps.put(map, vlconf);

        Set<DataLinkHost> allow = new HashSet<DataLinkHost>();
        MacVlan mvlan = null;
        for (int i = 0; i < 10; i++) {
            long mac = 0x123456789aL + i;
            DataLinkHost dlh = createEthernetHost(mac, (short)i);
            assertTrue(allow.add(dlh));
            if (mvlan == null) {
                try {
                    mvlan = new MacVlan(dlh);
                } catch (Exception e) {
                    unexpected(e);
                }
            }
        }
        assertNotNull(mvlan);

        Set<DataLinkHost> deny = new HashSet<DataLinkHost>();
        for (int i = 0; i < 5; i++) {
            long mac = 0xaabbccddeeL + i;
            DataLinkHost dlh = createEthernetHost(mac, (short)(i >> 1));
            assertTrue(deny.add(dlh));
        }

        NodeConnector edgePort = null;
        ISwitchManager swMgr = vtnMgr.getSwitchManager();
        ITopologyManager topoMgr = vtnMgr.getTopologyManager();
        for (NodeConnector nc: swMgr.getUpNodeConnectors(node)) {
            if (!(topoMgr.isInternal(nc) || swMgr.isSpecial(nc))) {
                edgePort = nc;
                break;
            }
        }
        assertNotNull(edgePort);

        MacMapConfig mcconf = new MacMapConfig(allow, deny);
        try {
            assertEquals(UpdateType.ADDED,
                         vtnMgr.setMacMap(bpath, UpdateOperation.ADD, mcconf));
        } catch (Exception e) {
            unexpected(e);
        }

        checkVTNconfig(vtnMgr, tpath, bpathlist, pmaps, vmaps, mcconf);

        // Activate one MAC mapping by sending a packet.
        short vlan = mvlan.getVlan();
        byte[] src = NetUtils.longToByteArray6(mvlan.getMacAddress());
        byte[] dst = NetUtils.getBroadcastMACAddr();
        byte[] srcIp = {(byte)192, (byte)168, (byte)100, (byte)1};
        byte[] dstIp = {(byte)192, (byte)168, (byte)100, (byte)200};
        RawPacket pkt = createARPRawPacket(src, dst, srcIp, dstIp,
                                           (vlan > 0) ? vlan : -1, edgePort,
                                           ARP.REQUEST);
        assertEquals(PacketResult.KEEP_PROCESSING,
                     vtnMgr.receiveDataPacket(pkt));

        brstates.put(bpath, VNodeState.UP);
        ifstates.put(ifpath1, VNodeState.UP);
        // ipath2 was configured but disabled and don't map.
        ifstates.put(ifpath2, VNodeState.DOWN);
        // ifpath3 was configured but don't map.
        ifstates.put(ifpath3, VNodeState.UNKNOWN);
        checkNodeState(tpath, brstates, ifstates);

        // restart VTNManager and check configuration after restart.
        restartVTNManager(c);

        checkVTNconfig(vtnMgr, tpath, bpathlist, pmaps, vmaps, mcconf);
        checkNodeState(tpath, brstates, ifstates);

        // add vlanmap to not existing node.
        node = NodeCreator.createOFNode(Long.valueOf((long)100));
        vlconf = new VlanMapConfig(node, (short)100);
        map = null;
        try {
            map = vtnMgr.addVlanMap(bpath, vlconf);
        } catch (VTNException e) {
            unexpected(e);
        }
        vmaps.put(map, vlconf);
        checkVTNconfig(vtnMgr, tpath, bpathlist, pmaps, vmaps, mcconf);

        brstates.put(bpath, VNodeState.DOWN);
        checkNodeState(tpath, brstates, ifstates);

        // start with having a configuration.
        // this case, startup with cache.
        restartVTNManager(c);
        checkVTNconfig(vtnMgr, tpath, bpathlist, pmaps, vmaps, mcconf);
        checkNodeState(tpath, brstates, ifstates);

        // start after configuration files is removed.
        // because caches remain, setting is taken over.
        stopVTNManager(false);
        ContainerConfig cfg = new ContainerConfig(containerName);
        cfg.cleanUp();
        assertFalse(dir.exists());
        startVTNManager(c);
        checkVTNconfig(vtnMgr, tpath, bpathlist, pmaps, vmaps, mcconf);
        checkNodeState(tpath, brstates, ifstates);

        // start after cache is cleared.
        // this case load from configuration files.
        stopVTNManager(true);
        startVTNManager(c);
        checkVTNconfig(vtnMgr, tpath, bpathlist, pmaps, vmaps, mcconf);

        // start with no cluster service.
        stopVTNManager(true);
        vtnMgr.unsetClusterContainerService(stubObj);
        startVTNManager(c);
        checkVTNconfig(vtnMgr, tpath, bpathlist, pmaps, vmaps, mcconf);

        // remove configuration files.
        stopVTNManager(true);
        vtnMgr.setClusterContainerService(stubObj);
        cfg.cleanUp();
        startVTNManager(c);

        // after cache is cleared, there is no configuration.
        List<VTenant> tlist = null;
        try {
            tlist = vtnMgr.getTenants();
        } catch (VTNException e) {
            unexpected(e);
        }
        assertNotNull(tlist);
        assertEquals(0, tlist.size());


        stopVTNManager(true);
        cfg.cleanUp();

        c = new ComponentImpl(null, null, null);
        c.setServiceProperties(null);
        startVTNManager(c);
        assertFalse(vtnMgr.isAvailable());

        stopVTNManager(true);
        cfg.cleanUp();

        // in case not "default" container
        c = new ComponentImpl(null, null, null);
        properties = new Hashtable<String, String>();
        String containerNameTest = "test";
        properties.put("containerName", containerNameTest);
        c.setServiceProperties(properties);
        startVTNManager(c);

        createTenantAndBridgeAndInterface(vtnMgr, tpath, bpathlist, ifpathlist);

        stopVTNManager(true);
        ContainerConfig cfgTest = new ContainerConfig(containerNameTest);
        cfgTest.cleanUp();
    }

    /**
     * Test method for
     * {@link VTNManagerImpl#init(org.apache.felix.dm.Component)},
     * {@link VTNManagerImpl#destroy()}.
     * {@code resume()} method of some modules are also tested.
     *
     * This test cases resume VTNManager after invalid mapping configuration
     * was saved.
     */
    @Test
    public void testInitDestoryAfterInvalidMap() {
        ComponentImpl c = new ComponentImpl(null, null, null);
        Hashtable<String, String> properties = new Hashtable<String, String>();
        String containerName = "default";
        properties.put("containerName", containerName);
        c.setServiceProperties(properties);

        Map<VBridgePath, VNodeState> brstates = new HashMap<VBridgePath, VNodeState>();
        Map<VBridgeIfPath, VNodeState> ifstates = new HashMap<VBridgeIfPath, VNodeState>();

        stopVTNManager(false);
        TestStub stub = new TestStub(0);
        vtnMgr.setSwitchManager(stub);
        vtnMgr.setTopologyManager(stub);
        startVTNManager(c);

        // add VTN configuration
        String tname = "vtn";
        VTenantPath tpath = new VTenantPath(tname);
        String bname = "vbridge";
        VBridgePath bpath = new VBridgePath(tname, bname);
        VBridgeIfPath ifpath1 = new VBridgeIfPath(tname, bname, "vif1");
        VBridgeIfPath ifpath2 = new VBridgeIfPath(tname, bname, "vif2");

        List<VBridgePath> bpathlist = new ArrayList<VBridgePath>();
        List<VBridgeIfPath> ifpathlist = new ArrayList<VBridgeIfPath>();
        bpathlist.add(bpath);
        ifpathlist.add(ifpath1);
        ifpathlist.add(ifpath2);
        createTenantAndBridgeAndInterface(vtnMgr, tpath, bpathlist, ifpathlist);

        Node node = NodeCreator.createOFNode(0L);
        SwitchPort port = new SwitchPort(NodeConnectorIDType.OPENFLOW,
                                         String.valueOf(10));
        PortMapConfig pmconf = new PortMapConfig(node, port, (short)0);

        // apply a conflict PortMap configuration to 2 interfaces.
        Status st = vtnMgr.setPortMap(ifpath1, pmconf);
        assertEquals(StatusCode.SUCCESS, st.getCode());

        st = vtnMgr.setPortMap(ifpath2, pmconf);
        assertEquals(StatusCode.SUCCESS, st.getCode());

        stopVTNManager(false);
        vtnMgr.setSwitchManager(stubObj);
        vtnMgr.setTopologyManager(stubObj);
        startVTNManager(c);

        brstates.put(bpath, VNodeState.DOWN);
        ifstates.put(ifpath1, VNodeState.UP);
        ifstates.put(ifpath2, VNodeState.DOWN);
        checkNodeState(tpath, brstates, ifstates);
    }

    /**
     * Check {@link VBridge} and {@link VInterface} state,
     * used in testInitDestory().
     *
     * @param tpath     A {@link VTenantPath}.
     * @param bpaths    A map between {@link VBridgePath} and {@link VNodeState}
     *                  of VBridge.
     * @param ifpaths   A map between {@link VBridgeIfPath} and {@link VNodeState}
     *                  of {@link VInterface}.
     */
    private void checkNodeState(VTenantPath tpath, Map<VBridgePath, VNodeState> bpaths,
            Map<VBridgeIfPath, VNodeState> ifpaths) {
        List<VBridge> blist = null;
        List<VInterface> iflist = null;

        if (bpaths != null) {
            // check vbridge state.
            try {
                blist = vtnMgr.getBridges(tpath);
            } catch (VTNException e) {
                unexpected(e);
            }

            VBridge vbr = blist.get(0);
            for (Map.Entry<VBridgePath, VNodeState> ent : bpaths.entrySet()) {
                assertEquals(ent.getKey().toString(),
                        ent.getKey().getBridgeName(), vbr.getName());
                assertEquals(ent.getKey().toString(), ent.getValue(), vbr.getState());
            }
        }

        if (ifpaths != null) {
            // check vInterface state.
            for (VBridgePath bpath : bpaths.keySet()) {
                try {
                    iflist = vtnMgr.getInterfaces(bpath);
                } catch (VTNException e) {
                    unexpected(e);
                }

                assertEquals(ifpaths.size(), iflist.size());
                for (VInterface vif : iflist) {
                    for (Map.Entry<VBridgeIfPath, VNodeState> ent : ifpaths.entrySet()) {
                        if (vif.getName().equals(ent.getKey().getInterfaceName())) {
                            assertEquals(ent.getValue(), vif.getState());
                        }
                    }
                }
            }
        }
    }

    /**
     * Test method for
     * {@link VTNManagerImpl#addVlanMap(VBridgePath, VlanMapConfig)},
     * {@link VTNManagerImpl#removeVlanMap(VBridgePath, java.lang.String)},
     * {@link VTNManagerImpl#getVlanMap(VBridgePath, java.lang.String)},
     * {@link VTNManagerImpl#getVlanMaps(VBridgePath)}.
     */
    @Test
    public void testVlanMap() {
        VTNManagerImpl mgr = vtnMgr;
        short[] vlans = new short[] {0, 10, 4095};
        String tname = "vtn";
        VTenantPath tpath = new VTenantPath(tname);
        String bname = "vbridge";
        VBridgePath bpath = new VBridgePath(tname, bname);
        VBridgeIfPath ifp = new VBridgeIfPath(tname, bname, "vinterface");
        Status st = null;
        List<VBridgePath> bpathlist = new ArrayList<VBridgePath>();
        List<VBridgeIfPath> ifplist = new ArrayList<VBridgeIfPath>();
        bpathlist.add(bpath);
        ifplist.add(ifp);

        createTenantAndBridgeAndInterface(mgr, tpath, bpathlist, ifplist);

        // add a vlanmap to a vbridge
        testVlanMapCheck(mgr, bpath);

        // add multi vlanmap to a vbridge
        for (Node node : createNodes(3)) {
            for (short vlan : vlans) {
                VlanMapConfig vlconf = new VlanMapConfig(node, vlan);
                VlanMap map = null;
                try {
                    map = mgr.addVlanMap(bpath, vlconf);
                    if (node != null
                            && node.getType() != NodeConnectorIDType.OPENFLOW) {
                        fail("throwing Exception was expected.");
                    }
                } catch (VTNException e) {
                    if (node != null &&
                        node.getType() == NodeConnectorIDType.OPENFLOW) {
                        unexpected(e);
                    } else {
                        // if node type is not OF, throwing a VTNException is expected.
                        continue;
                    }
                }
            }

            List<VlanMap> list = null;
            try {
                list = mgr.getVlanMaps(bpath);
            } catch (Exception e) {
                unexpected(e);
            }
            if (node == null ||
                node.getType() == NodeConnectorIDType.OPENFLOW) {
                assertEquals((node == null) ? "" : node.toString(),
                             vlans.length, list.size());
                VBridge brdg = null;
                try {
                    brdg = mgr.getBridge(bpath);
                } catch (VTNException e) {
                    unexpected(e);
                }
                assertEquals((node == null) ? "" : node.toString(),
                             VNodeState.UP, brdg.getState());
            } else {
                assertEquals(0, list.size());
                VBridge brdg = null;
                try {
                    brdg = mgr.getBridge(bpath);
                } catch (VTNException e) {
                    unexpected(e);
                }
                assertEquals(node.toString(), VNodeState.UNKNOWN,
                             brdg.getState());
            }

            for (VlanMap map : list) {
                st = mgr.removeVlanMap(bpath, map.getId());
                assertEquals((node == null) ? "" : node.toString(),
                             StatusCode.SUCCESS, st.getCode());
            }
        }

        st = mgr.removeTenant(tpath);
        assertEquals(StatusCode.SUCCESS, st.getCode());
    }

    /**
     * Test method for
     * {@link VTNManagerImpl#getPortMap(VBridgeIfPath)} and
     * {@link VTNManagerImpl#setPortMap(VBridgeIfPath, PortMapConfig)}.
     */
    @Test
    public void testPortMap() {
        VTNManagerImpl mgr = vtnMgr;
        short[] vlans = new short[] {0, 10, 4095};
        String tname = "vtn";
        VTenantPath tpath = new VTenantPath(tname);
        String bname = "vbridge";
        VBridgePath bpath = new VBridgePath(tname, bname);
        VBridgeIfPath ifp = new VBridgeIfPath(tname, bname, "vinterface");
        Status st = null;
        List<VBridgePath> bpathlist = new ArrayList<VBridgePath>();
        List<VBridgeIfPath> ifplist = new ArrayList<VBridgeIfPath>();
        bpathlist.add(bpath);
        ifplist.add(ifp);

        createTenantAndBridgeAndInterface(mgr, tpath, bpathlist, ifplist);

        PortMap pmap = null;
        try {
            pmap = mgr.getPortMap(ifp);
        } catch (Exception e) {
            unexpected(e);
        }
        assertNull(pmap);

        Node node = NodeCreator.createOFNode(0L);
        SwitchPort[] ports = new SwitchPort[] {
            new SwitchPort("port-10", NodeConnectorIDType.OPENFLOW, "10"),
            new SwitchPort(null, NodeConnectorIDType.OPENFLOW, "11"),
            new SwitchPort("port-10", null, null),
            new SwitchPort("port-10"),
            new SwitchPort(NodeConnectorIDType.OPENFLOW, "13"),
        };

        for (SwitchPort port: ports) {
            for (short vlan : vlans) {
                PortMapConfig pmconf = new PortMapConfig(node, port,
                                                         (short)vlan);
                String emsg = "(PortMapConfig)" + pmconf.toString();

                st = mgr.setPortMap(ifp, pmconf);
                assertEquals(emsg, StatusCode.SUCCESS, st.getCode());

                PortMap map = null;
                try {
                    map = mgr.getPortMap(ifp);
                } catch (Exception e) {
                    unexpected(e);
                }
                assertEquals(emsg, pmconf, map.getConfig());
                assertNotNull(emsg, map.getNodeConnector());
                if (port.getId() != null) {
                    assertEquals(emsg, Short.parseShort(port.getId()),
                                 map.getNodeConnector().getID());
                }
                if (port.getType() != null) {
                    assertEquals(emsg, port.getType(),
                                 map.getNodeConnector().getType());
                }

                checkNodeStatus(mgr, bpath, ifp, VNodeState.UP, VNodeState.UP,
                                pmconf.toString());
            }
        }

        st = mgr.setPortMap(ifp, null);
        assertEquals(StatusCode.SUCCESS, st.getCode());

        PortMap map = null;
        try {
            map = mgr.getPortMap(ifp);
        } catch (Exception e) {
            unexpected(e);
        }
        assertNull(map);

        st = mgr.removeTenant(tpath);
        assertEquals(StatusCode.SUCCESS, st.getCode());

        // set multi portmaps to a vbridge.
        String bname1 = "vbridge1";
        VBridgePath bpath1 = new VBridgePath(tname, bname1);
        String bname2 = "vbridge2";
        VBridgePath bpath2 = new VBridgePath(tname, bname2);
        VBridgeIfPath ifp1 = new VBridgeIfPath(tname, bname1, "vinterface1");
        VBridgeIfPath ifp2 = new VBridgeIfPath(tname, bname1, "vinterface2");

        List<VBridgePath> mbpathlist = new ArrayList<VBridgePath>();
        List<VBridgeIfPath> mifplist = new ArrayList<VBridgeIfPath>();
        mbpathlist.add(bpath1);
        mbpathlist.add(bpath2);
        mifplist.add(ifp1);
        mifplist.add(ifp2);
        createTenantAndBridgeAndInterface(mgr, tpath, mbpathlist, mifplist);

        node = NodeCreator.createOFNode(0L);
        SwitchPort port = new SwitchPort(NodeConnectorIDType.OPENFLOW, "10");
        PortMapConfig pmconf = new PortMapConfig(node, port, (short)0);
        st = mgr.setPortMap(ifp1, pmconf);
        assertEquals(StatusCode.SUCCESS, st.getCode());
        checkNodeStatus(mgr, bpath1, ifp1, VNodeState.UP, VNodeState.UP,
                        pmconf.toString());

        port = new SwitchPort(NodeConnectorIDType.OPENFLOW, "11");
        pmconf = new PortMapConfig(node, port, (short)0);
        st = mgr.setPortMap(ifp2, pmconf);
        assertEquals(StatusCode.SUCCESS, st.getCode());
        checkNodeStatus(mgr, bpath1, ifp2, VNodeState.UP, VNodeState.UP,
                        pmconf.toString());

        // in case same node connector which have different definition is mapped
        node = NodeCreator.createOFNode(0L);
        port = new SwitchPort("port-10");
        pmconf = new PortMapConfig(node, port, (short)0);
        st = mgr.setPortMap(ifp1, pmconf);
        assertEquals(StatusCode.SUCCESS, st.getCode());
        checkNodeStatus(mgr, bpath1, ifp1, VNodeState.UP, VNodeState.UP,
                        pmconf.toString());

        // add mapping to disabled interface.
        mgr.setPortMap(ifp1, null);
        checkNodeStatus(mgr, bpath1, ifp1, VNodeState.UP, VNodeState.UNKNOWN,
                        pmconf.toString());

        mgr.modifyInterface(
            ifp1, new VInterfaceConfig(null, Boolean.FALSE), true);
        port = new SwitchPort(NodeConnectorIDType.OPENFLOW, "10");
        pmconf = new PortMapConfig(node, port, (short)0);
        st = mgr.setPortMap(ifp1, pmconf);
        assertEquals(StatusCode.SUCCESS, st.getCode());
        checkNodeStatus(mgr, bpath1, ifp1, VNodeState.UP, VNodeState.DOWN,
                        pmconf.toString());

        mgr.modifyInterface(
            ifp1, new VInterfaceConfig(null, Boolean.TRUE), true);
        checkNodeStatus(mgr, bpath1, ifp1, VNodeState.UP, VNodeState.UP,
                        pmconf.toString());

        // map a internal port to interface.
        SwitchPort portIn =
            new SwitchPort(NodeConnectorIDType.OPENFLOW, "15");
        PortMapConfig pmconfIn = new PortMapConfig(node, portIn, (short)0);
        st = mgr.setPortMap(ifp2, pmconfIn);
        assertEquals(StatusCode.SUCCESS, st.getCode());
        checkNodeStatus(mgr, bpath1, ifp2, VNodeState.DOWN, VNodeState.DOWN,
                        pmconfIn.toString());

        st = mgr.removeInterface(ifp2);
        assertEquals(StatusCode.SUCCESS, st.getCode());
        checkNodeStatus(mgr, bpath1, null, VNodeState.UP, VNodeState.UP, "");

        // set duplicate portmap.
        VBridgeIfPath ifp3 = new VBridgeIfPath(tname, bname2, "vinterface3");
        port = new SwitchPort(NodeConnectorIDType.OPENFLOW, "10");
        pmconf = new PortMapConfig(node, port, (short)0);
        st = mgr.addInterface(
            ifp3, new VInterfaceConfig(null, Boolean.TRUE));
        st = mgr.setPortMap(ifp3, pmconf);
        assertEquals(StatusCode.CONFLICT, st.getCode());

        VTerminalIfPath vifpath = new VTerminalIfPath(tname, "vterm", "if_1");
        putInterface(mgr, vifpath);
        st = mgr.setPortMap(vifpath, pmconf);
        assertEquals(StatusCode.CONFLICT, st.getCode());

        // remove twice
        st = mgr.setPortMap(ifp1, null);
        assertEquals(StatusCode.SUCCESS, st.getCode());
        st = mgr.setPortMap(ifp1, null);
        assertEquals(StatusCode.SUCCESS, st.getCode());

        // add a vlan map to this VTN.
        testVlanMapCheck(mgr, bpath1);

        // remove test settings.
        st = mgr.removeTenant(tpath);
        assertEquals(StatusCode.SUCCESS, st.getCode());
    }

    /**
     * Test method for vTerminal interface port mapping APIs.
     *
     * <ul>
     *   <li>{@link VTNManagerImpl#getPortMap(VTerminalIfPath)}</li>
     *   <li>{@link VTNManagerImpl#setPortMap(VTerminalIfPath, PortMapConfig)}</li>
     * </ul>
     */
    @Test
    public void testTerminalPortMap() {
        VTNManagerImpl mgr = vtnMgr;
        short[] vlans = new short[] {0, 10, 4095};
        String tname = "vtn";
        String vtname = "vterm";
        String ifname = "if_1";
        VTenantPath tpath = new VTenantPath(tname);
        VTerminalPath vtpath = new VTerminalPath(tname, vtname);
        VTerminalIfPath ipath = new VTerminalIfPath(vtpath, ifname);
        putInterface(mgr, ipath);

        PortMap pmap = null;
        try {
            pmap = mgr.getPortMap(ipath);
        } catch (Exception e) {
            unexpected(e);
        }
        assertNull(pmap);

        Node node = NodeCreator.createOFNode(0L);
        SwitchPort[] ports = new SwitchPort[] {
            new SwitchPort("port-10", NodeConnectorIDType.OPENFLOW, "10"),
            new SwitchPort(null, NodeConnectorIDType.OPENFLOW, "11"),
            new SwitchPort("port-10", null, null),
            new SwitchPort("port-10"),
            new SwitchPort(NodeConnectorIDType.OPENFLOW, "13"),
        };

        for (SwitchPort port: ports) {
            for (short vlan: vlans) {
                PortMapConfig pmconf = new PortMapConfig(node, port,
                                                         (short)vlan);
                String emsg = "pmconf=" + pmconf.toString();
                Status st = mgr.setPortMap(ipath, pmconf);
                assertEquals(emsg, StatusCode.SUCCESS, st.getCode());

                PortMap map = null;
                try {
                    map = mgr.getPortMap(ipath);
                } catch (Exception e) {
                    unexpected(e);
                }
                assertEquals(emsg, pmconf, map.getConfig());
                assertNotNull(emsg, map.getNodeConnector());
                if (port.getId() != null) {
                    assertEquals(emsg, Short.parseShort(port.getId()),
                                 map.getNodeConnector().getID());
                }
                if (port.getType() != null) {
                    assertEquals(emsg, port.getType(),
                                 map.getNodeConnector().getType());
                }

                checkNodeStatus(mgr, vtpath, ipath, VNodeState.UP,
                                VNodeState.UP, pmconf.toString());
            }
        }

        Status st = mgr.setPortMap(ipath, null);
        assertEquals(StatusCode.SUCCESS, st.getCode());

        PortMap map = null;
        try {
            map = mgr.getPortMap(ipath);
        } catch (Exception e) {
            unexpected(e);
        }
        assertNull(map);

        st = mgr.removeTenant(tpath);
        assertEquals(StatusCode.SUCCESS, st.getCode());

        // set multi portmaps to a vbridge.
        VTerminalPath vtpath1 = new VTerminalPath(tname, "vterm_1");
        VTerminalPath vtpath2 = new VTerminalPath(tname, "vterm_2");
        VTerminalIfPath ipath1 = new VTerminalIfPath(vtpath1, "if");
        VTerminalIfPath ipath2 = new VTerminalIfPath(vtpath2, "if");
        putInterface(mgr, ipath1);
        putInterface(mgr, ipath2);

        SwitchPort port = new SwitchPort(NodeConnectorIDType.OPENFLOW, "10");
        PortMapConfig pmconf = new PortMapConfig(node, port, (short)0);
        st = mgr.setPortMap(ipath1, pmconf);
        assertEquals(StatusCode.SUCCESS, st.getCode());
        checkNodeStatus(mgr, vtpath1, ipath1, VNodeState.UP, VNodeState.UP,
                        pmconf.toString());

        port = new SwitchPort(NodeConnectorIDType.OPENFLOW, "11");
        pmconf = new PortMapConfig(node, port, (short)0);
        st = mgr.setPortMap(ipath2, pmconf);
        assertEquals(StatusCode.SUCCESS, st.getCode());
        checkNodeStatus(mgr, vtpath2, ipath2, VNodeState.UP, VNodeState.UP,
                        pmconf.toString());

        // Specify switch port by name.
        port = new SwitchPort("port-10");
        pmconf = new PortMapConfig(node, port, (short)0);
        st = mgr.setPortMap(ipath1, pmconf);
        assertEquals(StatusCode.SUCCESS, st.getCode());
        checkNodeStatus(mgr, vtpath1, ipath1, VNodeState.UP, VNodeState.UP,
                        pmconf.toString());

        // Clear port mapping at ipath1.
        mgr.setPortMap(ipath1, null);
        checkNodeStatus(mgr, vtpath1, ipath1, VNodeState.UNKNOWN,
                        VNodeState.UNKNOWN, pmconf.toString());

        // Set port mapping to disabled interface.
        VInterfaceConfig iconf = new VInterfaceConfig(null, Boolean.FALSE);
        st = mgr.modifyInterface(ipath1, iconf, true);
        assertEquals(StatusCode.SUCCESS, st.getCode());
        port = new SwitchPort(NodeConnectorIDType.OPENFLOW, "10");
        pmconf = new PortMapConfig(node, port, (short)0);
        st = mgr.setPortMap(ipath1, pmconf);
        assertEquals(StatusCode.SUCCESS, st.getCode());
        checkNodeStatus(mgr, vtpath1, ipath1, VNodeState.UNKNOWN,
                        VNodeState.DOWN, pmconf.toString());

        iconf = new VInterfaceConfig(null, Boolean.TRUE);
        st = mgr.modifyInterface(ipath1, iconf, true);
        assertEquals(StatusCode.SUCCESS, st.getCode());
        checkNodeStatus(mgr, vtpath1, ipath1, VNodeState.UP, VNodeState.UP,
                        ipath1.toString());

        // Try to map an internal port to interface.
        SwitchPort portIn =
            new SwitchPort(NodeConnectorIDType.OPENFLOW, "15");
        PortMapConfig pmconfIn = new PortMapConfig(node, portIn, (short)0);
        st = mgr.setPortMap(ipath2, pmconfIn);
        assertEquals(StatusCode.SUCCESS, st.getCode());
        checkNodeStatus(mgr, vtpath2, ipath2, VNodeState.DOWN, VNodeState.DOWN,
                        pmconfIn.toString());

        st = mgr.removeInterface(ipath2);
        assertEquals(StatusCode.SUCCESS, st.getCode());
        checkNodeStatus(mgr, vtpath2, null, VNodeState.UNKNOWN, null, "");

        // Try to duplicate port mapping.
        putInterface(mgr, ipath2);
        checkNodeStatus(mgr, vtpath2, ipath2, VNodeState.UNKNOWN,
                        VNodeState.UNKNOWN, ipath2.toString());
        port = new SwitchPort(NodeConnectorIDType.OPENFLOW, "10");
        pmconf = new PortMapConfig(node, port, (short)0);
        st = mgr.setPortMap(ipath2, pmconf);
        assertEquals(StatusCode.CONFLICT, st.getCode());
        checkNodeStatus(mgr, vtpath2, ipath2, VNodeState.UNKNOWN,
                        VNodeState.UNKNOWN, ipath2.toString());

        VBridgeIfPath bifpath = new VBridgeIfPath(tname, "bridge", "if");
        putInterface(mgr, bifpath);
        st = mgr.setPortMap(bifpath, pmconf);
        assertEquals(StatusCode.CONFLICT, st.getCode());

        // Remove port mapping twice.
        st = mgr.setPortMap(ipath1, null);
        assertEquals(StatusCode.SUCCESS, st.getCode());
        st = mgr.setPortMap(ipath1, null);
        assertEquals(StatusCode.SUCCESS, st.getCode());

        st = mgr.removeTenant(tpath);
        assertEquals(StatusCode.SUCCESS, st.getCode());
    }

    private static final String CACHE_NODES = "vtn.nodes";
    private static final String CACHE_PORTS = "vtn.ports";
    private static final String CACHE_ISL = "vtn.isl";

    /**
     * Test method for {@code portDB}, {@code islDB},
     * and {@code nodeDB}.
     * This tests case when {@link NodeConnector} status is changed.
     */
    @Test
    public void testNodeDBAndPortDBChangeNodeConnector() {
        VTNManagerImpl mgr = vtnMgr;
        ISwitchManager swMgr = mgr.getSwitchManager();
        IClusterContainerServices cs = mgr.getClusterContainerService();

        // initialize
        mgr.initInventory();
        mgr.initISL();

        Set<NodeConnector> existNodeConnector = new HashSet<NodeConnector>();
        Set<NodeConnector> existISL = new HashSet<NodeConnector>();
        List<TopoEdgeUpdate> addTopoList = new ArrayList<TopoEdgeUpdate>();
        getInventoryNodeAndPortData(mgr, existNodeConnector, existISL,
                                    addTopoList);

        checkNodeDBAndPortDBAndIslDB(cs, swMgr.getNodes().size(),
                existNodeConnector.size(), existISL.size());

        // invoke notifyNodeConnector().
        for (NodeConnector nc : existNodeConnector) {
            Map<String, Property> propMap = swMgr.getNodeConnectorProps(nc);

            mgr.notifyNodeConnector(nc, UpdateType.REMOVED, propMap);
            int expectedNumISL = existISL.contains(nc)
                ? existISL.size() - 1 : existISL.size();
            checkNodeDBAndPortDBAndIslDB(cs, swMgr.getNodes().size(),
                    existNodeConnector.size() - 1, expectedNumISL);

            // remove same port again. status is not changed.
            mgr.notifyNodeConnector(nc, UpdateType.REMOVED, propMap);
            checkNodeDBAndPortDBAndIslDB(cs, swMgr.getNodes().size(),
                    existNodeConnector.size() - 1, expectedNumISL);

            mgr.notifyNodeConnector(nc, UpdateType.ADDED, propMap);
            checkNodeDBAndPortDBAndIslDB(cs, swMgr.getNodes().size(),
                    existNodeConnector.size(), expectedNumISL);

            if (existISL.contains(nc)) {
                // edge information isn't updated by invoking notifyNodeConnector()
                // until edgeUpdate() is invoked.
                mgr.edgeUpdate(addTopoList);
                checkNodeDBAndPortDBAndIslDB(cs, swMgr.getNodes().size(),
                        existNodeConnector.size(), existISL.size());
            }

            mgr.notifyNodeConnector(nc, UpdateType.CHANGED, propMap);
            checkNodeDBAndPortDBAndIslDB(cs, swMgr.getNodes().size(),
                    existNodeConnector.size(), existISL.size());

            // add same port again. status is not changed.
            mgr.notifyNodeConnector(nc, UpdateType.ADDED, propMap);
            checkNodeDBAndPortDBAndIslDB(cs, swMgr.getNodes().size(),
                    existNodeConnector.size(), existISL.size());
        }

        // add and remove not existing node connector.
        // If added existing node connector , it is added to DB.
        Node node10 = NodeCreator.createOFNode(Long.valueOf("10"));
        NodeConnector nc = NodeConnectorCreator
                .createOFNodeConnector(Short.valueOf("99"), node10);
        Map<String, Property> propMap = new HashMap<String, Property>();

        mgr.notifyNodeConnector(nc, UpdateType.ADDED, propMap);
        checkNodeDBAndPortDBAndIslDB(cs, swMgr.getNodes().size() + 1,
                existNodeConnector.size() + 1, existISL.size());

        mgr.notifyNodeConnector(nc, UpdateType.CHANGED, propMap);
        checkNodeDBAndPortDBAndIslDB(cs, swMgr.getNodes().size() + 1,
                existNodeConnector.size() + 1, existISL.size());

        // nodeDB isn't updated when notifyNodeConnector() invoked with
        // UpdateType.REMOVED.
        mgr.notifyNodeConnector(nc, UpdateType.REMOVED, propMap);
        checkNodeDBAndPortDBAndIslDB(cs, swMgr.getNodes().size() + 1,
                existNodeConnector.size(), existISL.size());

        // nodeDB isn't updated until notifyNode() received().
        mgr.notifyNode(node10, UpdateType.REMOVED, propMap);
        checkNodeDBAndPortDBAndIslDB(cs, swMgr.getNodes().size(),
                existNodeConnector.size(), existISL.size());

        // in case propMap == null
        mgr.notifyNodeConnector(nc, UpdateType.ADDED, null);
        checkNodeDBAndPortDBAndIslDB(cs, swMgr.getNodes().size() + 1,
                existNodeConnector.size() + 1, existISL.size());

        mgr.notifyNodeConnector(nc, UpdateType.REMOVED, null);
        checkNodeDBAndPortDBAndIslDB(cs, swMgr.getNodes().size() + 1,
                existNodeConnector.size(), existISL.size());

        mgr.notifyNode(node10, UpdateType.REMOVED, null);
        checkNodeDBAndPortDBAndIslDB(cs, swMgr.getNodes().size(),
                existNodeConnector.size(), existISL.size());


        // add and remove invalid type node connector and
        // special type node connector.
        Node node = null;
        nc = null;
        NodeConnector snc = null;
        try {
            node = new Node(Node.NodeIDType.PRODUCTION, "Node ID: 0");
            nc = new NodeConnector(NodeConnectorIDType.PRODUCTION,
                                   "Node Connector ID: 0", node);
            snc = new NodeConnector(NodeConnectorIDType.CONTROLLER,
                                    NodeConnector.SPECIALNODECONNECTORID, node);
        } catch (ConstructionException e) {
            unexpected(e);
        }
        Set<NodeConnector> nodeSet = new HashSet<NodeConnector>();
        nodeSet.add(nc);
        nodeSet.add(snc);

        for (NodeConnector invalidNc : nodeSet) {
            mgr.notifyNodeConnector(invalidNc, UpdateType.ADDED, propMap);
            checkNodeDBAndPortDBAndIslDB(cs, swMgr.getNodes().size(),
                    existNodeConnector.size(), existISL.size());

            mgr.notifyNodeConnector(invalidNc, UpdateType.CHANGED, propMap);
            checkNodeDBAndPortDBAndIslDB(cs, swMgr.getNodes().size(),
                    existNodeConnector.size(), existISL.size());

            mgr.notifyNodeConnector(invalidNc, UpdateType.REMOVED, propMap);
            checkNodeDBAndPortDBAndIslDB(cs, swMgr.getNodes().size(),
                    existNodeConnector.size(), existISL.size());
        }

        // re-init. expect not to change DB.
        mgr.initInventory();
        mgr.initISL();
        checkNodeDBAndPortDBAndIslDB(cs, swMgr.getNodes().size(),
                existNodeConnector.size(), existISL.size());

        // change to stub which have 3 nodes.
        TestStub stubNew = new TestStub(3);
        mgr.setSwitchManager(stubNew);
        mgr.setTopologyManager(stubNew);
        swMgr = mgr.getSwitchManager();

        mgr.initInventory();
        mgr.initISL();

        existNodeConnector.clear();
        existISL.clear();
        addTopoList.clear();
        getInventoryNodeAndPortData(mgr, existNodeConnector, existISL,
                                    addTopoList);

        checkNodeDBAndPortDBAndIslDB(cs, swMgr.getNodes().size(),
                existNodeConnector.size(), existISL.size());

        // change topologyManager.
        // some edges are deleted by changing it.
        stubNew = new TestStub(2);
        mgr.setTopologyManager(stubNew);

        mgr.initInventory();
        mgr.initISL();

        existNodeConnector.clear();
        existISL.clear();
        addTopoList.clear();
        getInventoryNodeAndPortData(mgr, existNodeConnector, existISL,
                                    addTopoList);

        checkNodeDBAndPortDBAndIslDB(cs, swMgr.getNodes().size(),
                existNodeConnector.size(), existISL.size());

        // change to stub which have no nodes.
        stubNew = new TestStub(0);
        mgr.setSwitchManager(stubNew);
        mgr.setTopologyManager(stubNew);
        swMgr = mgr.getSwitchManager();

        mgr.initInventory();
        mgr.initISL();
        checkNodeDBAndPortDBAndIslDB(cs, 0, 0, 0);
    }

    /**
     * Test method for {@code portDB}, {@code islDB},
     * and {@code nodeDB}.
     * This tests case when Node status is changed.
     */
    @Test
    public void testNodeDBAndPortDBChangeNode() {
        VTNManagerImpl mgr = vtnMgr;
        ISwitchManager swMgr = mgr.getSwitchManager();
        IClusterContainerServices cs = mgr.getClusterContainerService();

        // initialize.
        mgr.initInventory();
        mgr.initISL();

        Set<NodeConnector> existNodeConnector = new HashSet<NodeConnector>();
        Set<NodeConnector> existISL = new HashSet<NodeConnector>();
        List<TopoEdgeUpdate> addTopoList = new ArrayList<TopoEdgeUpdate>();
        getInventoryNodeAndPortData(mgr, existNodeConnector, existISL,
                                    addTopoList);

        checkNodeDBAndPortDBAndIslDB(cs, swMgr.getNodes().size(),
                existNodeConnector.size(), existISL.size());

        int[] modes = new int[] {2, 3};
        Node node0 = NodeCreator.createOFNode(Long.valueOf(0L));

        for (int mode : modes) {
            TestStub stubNew = new TestStub(mode);
            mgr.setSwitchManager(stubNew);
            mgr.setTopologyManager(stubNew);
            swMgr = mgr.getSwitchManager();

            existNodeConnector.clear();
            existISL.clear();
            addTopoList.clear();
            getInventoryNodeAndPortData(mgr, existNodeConnector, existISL,
                                        addTopoList);

            for (Node node : swMgr.getNodes()) {
                mgr.initInventory();
                mgr.initISL();

                int expectedNumISL = 1;
                if (mode == 3) {
                    expectedNumISL = (node.equals(node0)) ? 2 : 3;
                }

                Map<String, Property> propMap = swMgr.getNodeProps(node);
                mgr.notifyNode(node, UpdateType.REMOVED, propMap);
                checkNodeDBAndPortDBAndIslDB(cs, swMgr.getNodes().size() - 1,
                        existNodeConnector.size() - swMgr.getNodeConnectors(node).size(),
                        expectedNumISL);

                // remove same node again. status is not changed.
                mgr.notifyNode(node, UpdateType.REMOVED, propMap);
                checkNodeDBAndPortDBAndIslDB(cs, swMgr.getNodes().size() - 1,
                        existNodeConnector.size() - swMgr.getNodeConnectors(node).size(),
                        expectedNumISL);

                mgr.notifyNode(node, UpdateType.CHANGED, propMap);
                checkNodeDBAndPortDBAndIslDB(cs, swMgr.getNodes().size() - 1,
                        existNodeConnector.size() - swMgr.getNodeConnectors(node).size(),
                        expectedNumISL);

                // node connectors associated with specified node
                // are removed by invoking notifyNode().
                // But these aren't added by invoking notifyNode().
                mgr.notifyNode(node, UpdateType.ADDED, propMap);
                checkNodeDBAndPortDBAndIslDB(cs, swMgr.getNodes().size(),
                        existNodeConnector.size() - swMgr.getNodeConnectors(node).size(),
                        expectedNumISL);

                // add same node again. status is not changed.
                mgr.notifyNode(node, UpdateType.ADDED, propMap);
                checkNodeDBAndPortDBAndIslDB(cs, swMgr.getNodes().size(),
                        existNodeConnector.size() - swMgr.getNodeConnectors(node).size(),
                        expectedNumISL);
            }
        }

        // reset switchManager and topologyManager.
        TestStub stubNew = new TestStub(3);
        mgr.setSwitchManager(stubNew);
        mgr.setTopologyManager(stubNew);
        swMgr = mgr.getSwitchManager();

        existNodeConnector.clear();
        existISL.clear();
        addTopoList.clear();
        getInventoryNodeAndPortData(mgr, existNodeConnector, existISL,
                                    addTopoList);

        // not existing node.
        Node node10 = NodeCreator.createOFNode(Long.valueOf(10L));

        mgr.initInventory();
        mgr.initISL();

        Map<String, Property> propMap = new HashMap<String, Property>();

        // When add and remove not existing node,
        // a size of nodeDB changes because existence of node isn't checked.
        mgr.notifyNode(node10, UpdateType.ADDED, propMap);
        checkNodeDBAndPortDBAndIslDB(cs, swMgr.getNodes().size() + 1,
                existNodeConnector.size(), 4);

        mgr.notifyNode(node10, UpdateType.CHANGED, propMap);
        checkNodeDBAndPortDBAndIslDB(cs, swMgr.getNodes().size() + 1,
                existNodeConnector.size(), 4);

        mgr.notifyNode(node10, UpdateType.REMOVED, propMap);
        checkNodeDBAndPortDBAndIslDB(cs, swMgr.getNodes().size(),
                existNodeConnector.size(), 4);

        // add and remove invalid type node.
        Node node = null;
        try {
            node = new Node(Node.NodeIDType.ONEPK, "Node ID: 0");
        } catch (ConstructionException e) {
            unexpected(e);
        }
        mgr.notifyNode(node, UpdateType.ADDED, propMap);
        checkNodeDBAndPortDBAndIslDB(cs, swMgr.getNodes().size(),
                existNodeConnector.size(), 4);

        mgr.notifyNode(node, UpdateType.CHANGED, propMap);
        checkNodeDBAndPortDBAndIslDB(cs, swMgr.getNodes().size(),
                existNodeConnector.size(), 4);

        mgr.notifyNode(node, UpdateType.REMOVED, propMap);
        checkNodeDBAndPortDBAndIslDB(cs, swMgr.getNodes().size(),
                existNodeConnector.size(), 4);
    }

    /**
     * Test method for {@code portDB}, {@code islDB},
     * and {@code nodeDB}.
     * This tests case when {@link Edge} status is changed.
     */
    @Test
    public void testNodeDBAndPortDBChangeEdge() {
        VTNManagerImpl mgr = vtnMgr;
        ISwitchManager swMgr = mgr.getSwitchManager();
        ITopologyManager topoMgr = mgr.getTopologyManager();
        IClusterContainerServices cs = mgr.getClusterContainerService();

        // initialize
        mgr.initInventory();
        mgr.initISL();

        Set<NodeConnector> existNodeConnector = new HashSet<NodeConnector>();
        Set<NodeConnector> existISL = new HashSet<NodeConnector>();
        getInventoryNodeAndPortData(mgr, existNodeConnector, existISL, null);

        checkNodeDBAndPortDBAndIslDB(cs, swMgr.getNodes().size(), existNodeConnector.size(),
                existISL.size());

        int[] modes = new int[] {2, 3};

        List<TopoEdgeUpdate> addTopoList = new ArrayList<TopoEdgeUpdate>();
        List<TopoEdgeUpdate> removeTopoList = new ArrayList<TopoEdgeUpdate>();
        List<TopoEdgeUpdate> changeTopoList = new ArrayList<TopoEdgeUpdate>();
        for (int mode : modes) {
            TestStub stubNew = new TestStub(mode);
            mgr.setSwitchManager(stubNew);
            mgr.setTopologyManager(stubNew);
            swMgr = mgr.getSwitchManager();
            topoMgr = mgr.getTopologyManager();

            existNodeConnector.clear();
            existISL.clear();
            addTopoList.clear();

            mgr.initInventory();
            mgr.initISL();
            getInventoryNodeAndPortData(mgr, existNodeConnector, existISL,
                                        addTopoList);

            addTopoList.clear();
            removeTopoList.clear();
            changeTopoList.clear();

            for (Edge edge : topoMgr.getEdges().keySet()) {
                TopoEdgeUpdate update = new TopoEdgeUpdate(edge, null,
                                                           UpdateType.REMOVED);
                removeTopoList.add(update);
                mgr.edgeUpdate(removeTopoList);
                checkNodeDBAndPortDBAndIslDB(cs, swMgr.getNodes().size(),
                        existNodeConnector.size(),
                        existISL.size() - 2 * removeTopoList.size());

                mgr.edgeUpdate(removeTopoList);
                checkNodeDBAndPortDBAndIslDB(cs, swMgr.getNodes().size(),
                        existNodeConnector.size(),
                        existISL.size() - 2 * removeTopoList.size());

                update = new TopoEdgeUpdate(edge, null, UpdateType.ADDED);
                addTopoList.add(update);
                mgr.edgeUpdate(addTopoList);
                checkNodeDBAndPortDBAndIslDB(cs, swMgr.getNodes().size(),
                        existNodeConnector.size(), existISL.size());

                mgr.edgeUpdate(addTopoList);
                checkNodeDBAndPortDBAndIslDB(cs, swMgr.getNodes().size(),
                        existNodeConnector.size(), existISL.size());

                update = new TopoEdgeUpdate(edge, null, UpdateType.CHANGED);
                changeTopoList.add(update);
                mgr.edgeUpdate(changeTopoList);
                checkNodeDBAndPortDBAndIslDB(cs, swMgr.getNodes().size(),
                        existNodeConnector.size(), existISL.size());
            }
        }

        // reset switchManager and topologyManager
        TestStub stubNew = new TestStub(3);
        mgr.setSwitchManager(stubNew);
        mgr.setTopologyManager(stubNew);
        swMgr = mgr.getSwitchManager();
        topoMgr = mgr.getTopologyManager();

        existNodeConnector.clear();
        existISL.clear();
        addTopoList.clear();
        getInventoryNodeAndPortData(mgr, existNodeConnector, existISL,
                                    addTopoList);

        // add, remove, change not existing edge.
        // in this case DB isn't updated.
        Node node0 = NodeCreator.createOFNode(Long.valueOf(0L));
        Node node100 = NodeCreator.createOFNode(Long.valueOf(100L));

        Set<Node> nodeSet = new HashSet<Node>();
        nodeSet.add(node0);
        nodeSet.add(node100);

        short[] ports = new short[] {(short)10, (short)99};
        for (Node nodeTail : nodeSet) {
            for (Node nodeHead : nodeSet) {
                if (nodeTail.equals(nodeHead)) {
                    continue;
                }
                for (short portTail : ports) {
                    for (short portHead : ports) {
                        NodeConnector tail = NodeConnectorCreator
                                .createOFNodeConnector(Short.valueOf(portTail),
                                                       nodeTail);
                        NodeConnector head = NodeConnectorCreator
                                .createOFNodeConnector(Short.valueOf(portHead),
                                                       nodeHead);
                        Edge newEdge = null;
                        try {
                            newEdge = new Edge(tail, head);
                        } catch (ConstructionException e) {
                            unexpected(e);
                        }

                        TopoEdgeUpdate update = new TopoEdgeUpdate(newEdge, null,
                                                                   UpdateType.ADDED);
                        addTopoList.clear();
                        addTopoList.add(update);

                        mgr.edgeUpdate(addTopoList);
                        if (nodeTail.equals(node100) || nodeHead.equals(node100)
                                || portTail == 99 || portHead == 99) {
                            // if tail or head is not existing NodeConnector
                            // islDB isn't updated.
                            checkNodeDBAndPortDBAndIslDB(cs,
                                    swMgr.getNodes().size(),
                                    existNodeConnector.size(),
                                    existISL.size());
                        } else {
                            checkNodeDBAndPortDBAndIslDB(cs,
                                    swMgr.getNodes().size(),
                                    existNodeConnector.size(),
                                    existISL.size() + 2);
                        }

                        update = new TopoEdgeUpdate(newEdge, null,
                                                    UpdateType.REMOVED);
                        removeTopoList.clear();
                        removeTopoList.add(update);

                        mgr.edgeUpdate(removeTopoList);
                        checkNodeDBAndPortDBAndIslDB(cs, swMgr.getNodes().size(),
                                existNodeConnector.size(), existISL.size());
                    }
                }
            }
        }

        NodeConnector tail = NodeConnectorCreator
                .createOFNodeConnector(Short.valueOf("99"), node0);
        NodeConnector head = NodeConnectorCreator
                .createOFNodeConnector(Short.valueOf("99"), node100);
        Edge newEdge = null;
        try {
            newEdge = new Edge(tail, head);
        } catch (ConstructionException e) {
            unexpected(e);
        }
        TopoEdgeUpdate update = new TopoEdgeUpdate(newEdge, null,
                                                   UpdateType.ADDED);
        addTopoList.clear();
        addTopoList.add(update);

        mgr.edgeUpdate(addTopoList);
        checkNodeDBAndPortDBAndIslDB(cs, swMgr.getNodes().size(),
                                     existNodeConnector.size(),
                                     existISL.size());

        update = new TopoEdgeUpdate(newEdge, null, UpdateType.REMOVED);
        removeTopoList.clear();
        removeTopoList.add(update);

        mgr.edgeUpdate(removeTopoList);
        checkNodeDBAndPortDBAndIslDB(cs, swMgr.getNodes().size(),
                                     existNodeConnector.size(),
                                     existISL.size());

        // start with invalid topology.
        // (topologyManager has edges associated with not existing Node.)
        ComponentImpl c = new ComponentImpl(null, null, null);
        Hashtable<String, String> properties = new Hashtable<String, String>();
        String containerName = "default";
        properties.put("containerName", containerName);
        c.setServiceProperties(properties);

        stubNew = new TestStub(3);
        stopVTNManager(true);
        vtnMgr.setSwitchManager(stubObj);
        vtnMgr.setTopologyManager(stubNew);
        startVTNManager(c);
        assertEquals(2, cs.getCache(CACHE_ISL).size());
    }

    /**
     * get Lists of existing {@link NodeConnector} and ISL and Edge.
     * This method set results to Lists specified by argument
     * ({@code existNodeConnector}, {@code existISL}, {@code addTopoList}).
     *
     * @param mgr                   VTNManager service.
     * @param existNodeConnector    A List of {@link NodeConnector} which is set
     *                              existing Nodeconnectors.
     * @param existISL              A List of {@link NodeConnector} which is set
     *                              existing ISLs.
     * @param addTopoList           A List of {@link TopoEdgeUpdate} which is set
     *                              {@link TopoEdgeUpdate} of existing Edges.
     */
    private void getInventoryNodeAndPortData(VTNManagerImpl mgr,
                                             Set<NodeConnector> existNodeConnector,
                                             Set<NodeConnector> existISL,
                                             List<TopoEdgeUpdate> addTopoList) {
        ISwitchManager swMgr = mgr.getSwitchManager();
        ITopologyManager topoMgr = mgr.getTopologyManager();
        for (Node node : swMgr.getNodes()) {
            existNodeConnector.addAll(swMgr.getPhysicalNodeConnectors(node));
        }

        for (Edge edge : topoMgr.getEdges().keySet()) {
            existISL.add(edge.getHeadNodeConnector());
            existISL.add(edge.getTailNodeConnector());

            if (addTopoList != null) {
                TopoEdgeUpdate update = new TopoEdgeUpdate(edge, null,
                                                           UpdateType.ADDED);
                addTopoList.add(update);
            }
        }
    }

    /**
     * Check a size of {@code nodeDB} and {@code portDB} and {@code islDB}.
     *
     * @param cs            Cluster service.
     * @param nodeDBSize    A expected size of {@code nodeDB}.
     * @param portDBSize    A expected size of {@code portDB}.
     * @param islDBSize     A expected size of {@code islDB}.
     */
    private void checkNodeDBAndPortDBAndIslDB(IClusterContainerServices cs,
                                              int nodeDBSize,
                                              int portDBSize,
                                              int islDBSize) {
        ConcurrentMap<Node, VNodeState> nodeMap =
            (ConcurrentMap<Node, VNodeState>)cs.getCache(CACHE_NODES);
        List<Node> nodeMapList = new ArrayList<Node>(nodeMap.keySet());
        ConcurrentMap<NodeConnector, PortProperty> portMap =
            (ConcurrentMap<NodeConnector, PortProperty>)
            cs.getCache(CACHE_PORTS);
        List<NodeConnector> portMapList
            = new ArrayList<NodeConnector>(portMap.keySet());
        ConcurrentMap<NodeConnector, VNodeState> islMap =
            (ConcurrentMap<NodeConnector, VNodeState>)cs.getCache(CACHE_ISL);
        List<NodeConnector> islMapList =
            new ArrayList<NodeConnector>(islMap.keySet());

        if (nodeDBSize >= 0) {
            assertEquals(nodeDBSize, nodeMapList.size());
        }
        if (portDBSize >= 0) {
            assertEquals(portDBSize, portMapList.size());
        }
        if (islDBSize >= 0) {
            assertEquals(islDBSize, islMapList.size());
        }
    }

    /**
     * Common routine for {@code testVlanMap} and {@code testPortMap}
     *
     * @param mgr   VTN Manager service.
     * @param bpath a {@link VBridgePath}.
     */
    private void testVlanMapCheck(VTNManagerImpl mgr, VBridgePath bpath) {
        short[] vlans = new short[] {0, 10, 4095};

        for (Node vnode : createNodes(3)) {
            for (short vlan : vlans) {
                VlanMapConfig vlconf = new VlanMapConfig(vnode, vlan);
                String emsg = "(VlanMapConfig)" + vlconf.toString();
                VlanMap vmap = null;
                try {
                    vmap = mgr.addVlanMap(bpath, vlconf);
                    if (vnode != null
                            && vnode.getType() != NodeConnectorIDType.OPENFLOW) {
                        fail("throwing Exception was expected.");
                    }
                } catch (VTNException e) {
                    if (vnode != null
                            && vnode.getType() == NodeConnectorIDType.OPENFLOW) {
                        unexpected(e);
                    } else {
                        // if node type is not OF,
                        // throwing a VTNException is expected.
                        continue;
                    }
                }

                VlanMap getmap = null;
                try {
                    getmap = mgr.getVlanMap(bpath, vmap.getId());
                } catch (VTNException e) {
                    unexpected(e);
                }
                assertEquals(emsg, getmap.getId(), vmap.getId());
                assertEquals(emsg, getmap.getNode(), vnode);
                assertEquals(emsg, getmap.getVlan(), vlan);

                VBridge brdg = null;
                try {
                    brdg = mgr.getBridge(bpath);
                } catch (VTNException e) {
                    unexpected(e);
                }
                assertEquals(emsg, VNodeState.UP, brdg.getState());

                Status st = mgr.removeVlanMap(bpath, vmap.getId());
                assertEquals(emsg, StatusCode.SUCCESS, st.getCode());
            }
        }
    }

    /**
     * Test method for
     * {@link VTNManagerImpl#notifyNode(Node, UpdateType, Map)},
     * {@link VTNManagerImpl#notifyNodeConnector(NodeConnector, UpdateType, Map)},
     * {@link VTNManagerImpl#edgeUpdate(java.util.List)}
     */
    @Test
    public void testNotificationWithPortMap() {
        VTNManagerImpl mgr = vtnMgr;
        ITopologyManager topoMgr = mgr.getTopologyManager();
        short[] vlans = new short[] {0, 10, 4095};
        Status st = null;
        String tname = "vtn";
        VTenantPath tpath = new VTenantPath(tname);
        String bname1 = "vbridge";
        VBridgePath bpath = new VBridgePath(tname, bname1);
        VBridgeIfPath ifp = new VBridgeIfPath(tname, bname1, "vinterface");
        VBridgeIfPath difp = new VBridgeIfPath(tname, bname1, "vinterface_disabled");
        VBridgeIfPath inifp = new VBridgeIfPath(tname, bname1, "vinterfaceIn");

        List<VBridgePath> bpathlist = new ArrayList<VBridgePath>();
        List<VBridgeIfPath> ifplist = new ArrayList<VBridgeIfPath>();
        bpathlist.add(bpath);
        ifplist.add(ifp);
        createTenantAndBridgeAndInterface(mgr, tpath, bpathlist, ifplist);

        st = mgr.addInterface(difp, new VInterfaceConfig(null, Boolean.FALSE));
        assertEquals(StatusCode.SUCCESS, st.getCode());

        PortMap pmap = null;
        try {
            pmap = mgr.getPortMap(ifp);
        } catch (Exception e) {
            unexpected(e);
        }
        assertNull(pmap);

        Set<Node> nodeSet = new HashSet<Node>();
        Node cnode = NodeCreator.createOFNode(0L);
        Node onode = NodeCreator.createOFNode(1L);
        nodeSet.add(cnode);
        nodeSet.add(onode);

        SwitchPort[] ports = new SwitchPort[] {
            new SwitchPort("port-10", NodeConnectorIDType.OPENFLOW, "10"),
            new SwitchPort(null, NodeConnectorIDType.OPENFLOW, "11"),
            new SwitchPort("port-10", null, null),
            new SwitchPort("port-10"),
            new SwitchPort(NodeConnectorIDType.OPENFLOW, "13"),
        };

        SwitchPort portIn = new SwitchPort(NodeConnectorIDType.OPENFLOW, "15");
        PortMapConfig pmconfIn = new PortMapConfig(cnode, portIn, (short)0);
        NodeConnector ncIn = NodeConnectorCreator.
            createOFNodeConnector(Short.valueOf((short)15), cnode);

        for (Node node : nodeSet) {
            for (SwitchPort port : ports) {
                for (short vlan : vlans) {
                    PortMapConfig pmconf =
                        new PortMapConfig(node, port, (short)vlan);
                    String emsg = "(Node)" + node.toString()
                            + ",(PortMapConfig)" + pmconf.toString();

                    st = mgr.setPortMap(ifp, pmconf);
                    assertEquals(emsg, StatusCode.SUCCESS, st.getCode());

                    PortMap map = null;
                    try {
                        map = mgr.getPortMap(ifp);
                    } catch (Exception e) {
                        unexpected(e);
                    }
                    assertEquals(emsg, pmconf, map.getConfig());
                    assertNotNull(emsg, map.getNodeConnector());
                    if (port.getId() != null) {
                        assertEquals(emsg,
                                     Short.parseShort(port.getId()),
                                     map.getNodeConnector().getID());
                    }
                    if (port.getType() != null) {
                        assertEquals(emsg,
                                port.getType(), map.getNodeConnector().getType());
                    }
                    checkNodeStatus(mgr, bpath, ifp, VNodeState.UP, VNodeState.UP,
                                    pmconf.toString());

                    // test each Notification APIs.
                    checkNotification(mgr, bpath, ifp, map.getNodeConnector(),
                        cnode, pmconf, null, MapType.PORT, pmconf.toString());

                    // test for a edge changed notification
                    // when port map is assigned to a internal node connector.
                    Map<Edge, Set<Property>> edges = topoMgr.getEdges();
                    for (Edge edge: edges.keySet()) {
                        List<TopoEdgeUpdate> addTopoList = new ArrayList<TopoEdgeUpdate>();
                        List<TopoEdgeUpdate> rmTopoList = new ArrayList<TopoEdgeUpdate>();
                        String emsgEdge = emsg + ",(Edge)" + edge.toString()
                                          + ",(pmconfIn)" + pmconfIn.toString();

                        TopoEdgeUpdate update = new TopoEdgeUpdate(edge, null,
                                                                   UpdateType.ADDED);
                        addTopoList.add(update);

                        Edge reverseEdge = null;
                        try {
                            reverseEdge = new Edge(edge.getHeadNodeConnector(),
                                                    edge.getTailNodeConnector());
                        } catch (ConstructionException e) {
                            unexpected(e);
                        }
                        update = new TopoEdgeUpdate(reverseEdge, null, UpdateType.ADDED);
                        addTopoList.add(update);
                        update = new TopoEdgeUpdate(edge, null, UpdateType.REMOVED);
                        rmTopoList.add(update);
                        update = new TopoEdgeUpdate(reverseEdge, null, UpdateType.REMOVED);
                        rmTopoList.add(update);

                        st = mgr.addInterface(
                            inifp, new VInterfaceConfig(null, null));
                        assertEquals(emsgEdge, StatusCode.SUCCESS, st.getCode());

                        st = mgr.setPortMap(inifp, pmconfIn);
                        assertEquals(emsgEdge, StatusCode.SUCCESS, st.getCode());

                        if (ncIn.equals(edge.getHeadNodeConnector()) ||
                                ncIn.equals(edge.getTailNodeConnector())) {
                            checkNodeStatus(mgr, bpath, inifp, VNodeState.DOWN,
                                            VNodeState.DOWN, emsgEdge);
                            checkNodeStatus(mgr, bpath, ifp, VNodeState.DOWN, VNodeState.UP,
                                            emsgEdge);
                        } else {
                            checkNodeStatus(mgr, bpath, inifp, VNodeState.UP, VNodeState.UP,
                                            emsgEdge);
                            checkNodeStatus(mgr, bpath, ifp, VNodeState.UP, VNodeState.UP,
                                            emsgEdge);
                        }

                        stubObj.deleteEdge(edge);
                        stubObj.deleteEdge(reverseEdge);

                        mgr.edgeUpdate(rmTopoList);
                        checkNodeStatus(mgr, bpath, inifp, VNodeState.UP, VNodeState.UP,
                                        emsgEdge);

                        stubObj.addEdge(edge);
                        stubObj.addEdge(reverseEdge);
                        mgr.edgeUpdate(addTopoList);
                        if (ncIn.equals(edge.getHeadNodeConnector()) ||
                                ncIn.equals(edge.getTailNodeConnector())) {
                            checkNodeStatus(mgr, bpath, inifp, VNodeState.DOWN,
                                    VNodeState.DOWN, emsgEdge);
                            checkNodeStatus(mgr, bpath, ifp, VNodeState.DOWN,
                                    VNodeState.UP, emsgEdge);
                        } else {
                            checkNodeStatus(mgr, bpath, inifp, VNodeState.UP, VNodeState.UP,
                                            emsgEdge);
                            checkNodeStatus(mgr, bpath, ifp, VNodeState.UP, VNodeState.UP,
                                            emsgEdge);
                        }

                        st = mgr.removeInterface(inifp);
                        assertEquals(emsgEdge, StatusCode.SUCCESS, st.getCode());

                        checkNodeStatus(mgr, bpath, ifp, VNodeState.UP, VNodeState.UP,
                                        emsgEdge);

                        VInterface bif = null;
                        try {
                            bif = mgr.getInterface(difp);
                        } catch (VTNException e) {
                            unexpected(e);
                        }

                        assertEquals(emsgEdge, false, bif.getEnabled());
                        assertEquals(emsgEdge, VNodeState.DOWN, bif.getState());
                    }
                }
            }
        }

        // in case notified node connector is already mapped another if.
        ISwitchManager swMgr = mgr.getSwitchManager();
        String bname2 = "vbridge2";
        VBridgePath bpath2 = new VBridgePath(tname, bname2);
        VBridgeIfPath ifp2 = new VBridgeIfPath(tname, bname2, "vinterface");
        SwitchPort sp = new SwitchPort("port-10",
                                       NodeConnectorIDType.OPENFLOW, "10");
        NodeConnector nc
            = NodeConnectorCreator.createOFNodeConnector(Short.valueOf((short)10), cnode);
        Map<String, Property> propMap = swMgr.getNodeConnectorProps(nc);
        PortMapConfig pmconf = new PortMapConfig(cnode, sp, (short)1);

        st = mgr.setPortMap(ifp, pmconf);
        assertEquals(StatusCode.SUCCESS, st.getCode());
        checkNodeStatus(mgr, bpath, ifp, VNodeState.UP, VNodeState.UP, pmconf.toString());

        mgr.notifyNodeConnector(nc, UpdateType.REMOVED, propMap);

        st = mgr.addBridge(bpath2, new VBridgeConfig(null));
        assertEquals(StatusCode.SUCCESS, st.getCode());
        st = mgr.addInterface(ifp2, new VInterfaceConfig(null, null));
        assertEquals(StatusCode.SUCCESS, st.getCode());

        // add same PortMap with ifp1 to ifp2
        st = mgr.setPortMap(ifp2, new PortMapConfig(cnode, sp, (short)1));
        assertEquals(StatusCode.SUCCESS, st.getCode());
        checkNodeStatus(mgr, bpath2, ifp2, VNodeState.DOWN, VNodeState.DOWN,
                        pmconf.toString());

        mgr.notifyNodeConnector(nc, UpdateType.ADDED, propMap);
        mgr.notifyNodeConnector(nc, UpdateType.REMOVED, propMap);
        mgr.notifyNodeConnector(nc, UpdateType.ADDED, propMap);
        mgr.initISL();

        checkNodeStatus(mgr, bpath, ifp, VNodeState.UP, VNodeState.UP, pmconf.toString());
        checkNodeStatus(mgr, bpath2, ifp2, VNodeState.DOWN, VNodeState.DOWN,
                        pmconf.toString());

        // notify disabled node
        st = mgr.modifyInterface(
            ifp2, new VInterfaceConfig(null, Boolean.FALSE), true);
        assertEquals(StatusCode.SUCCESS, st.getCode());

        mgr.notifyNodeConnector(nc, UpdateType.REMOVED, propMap);
        mgr.notifyNodeConnector(nc, UpdateType.ADDED, propMap);
        mgr.initISL();

        checkNodeStatus(mgr, bpath2, ifp2, VNodeState.UNKNOWN, VNodeState.DOWN,
                        pmconf.toString());

        // if mapped port is null.
        sp = new SwitchPort("port-16", NodeConnectorIDType.OPENFLOW, "16");
        pmconf = new PortMapConfig(cnode, sp, (short)0);
        st = mgr.setPortMap(ifp, pmconf);
        propMap = null;
        mgr.notifyNode(cnode, UpdateType.REMOVED, propMap);
        checkNodeStatus(mgr, bpath, ifp, VNodeState.DOWN, VNodeState.DOWN,
                pmconf.toString());

        nc = NodeConnectorCreator.createOFNodeConnector((short)16, cnode);
        propMap = swMgr.getNodeConnectorProps(nc);
        mgr.notifyNodeConnector(nc, UpdateType.CHANGED, propMap);
        checkNodeStatus(mgr, bpath, ifp, VNodeState.DOWN, VNodeState.DOWN,
                pmconf.toString());

        mgr.edgeUpdate(new ArrayList<TopoEdgeUpdate>(0));
        checkNodeStatus(mgr, bpath, ifp, VNodeState.DOWN, VNodeState.DOWN,
                pmconf.toString());

        st = mgr.removeTenant(tpath);
        assertEquals(StatusCode.SUCCESS, st.getCode());
    }

    /**
     * Same test with {@code testNotifyNodeAndNodeConnectorWithPortMap}.
     * this test case do with VLAN map setting.
     */
    @Test
    public void testNotificationWithVlanMap() {
        VTNManagerImpl mgr = vtnMgr;
        Set<Node> nodeSet = new HashSet<Node>();
        short[] vlans = new short[] {0, 10, 4095};

        String tname = "vtn";
        VTenantPath tpath = new VTenantPath(tname);
        String bname = "vbridge";
        VBridgePath bpath = new VBridgePath(tname, bname);
        VBridgeIfPath ifp = new VBridgeIfPath(tname, bname, "vinterface");
        Status st = null;

        List<VBridgePath> bpathlist = new ArrayList<VBridgePath>();
        List<VBridgeIfPath> ifplist = new ArrayList<VBridgeIfPath>();
        bpathlist.add(bpath);
        ifplist.add(ifp);
        createTenantAndBridgeAndInterface(mgr, tpath, bpathlist, ifplist);

        Node cnode = NodeCreator.createOFNode(0L);
        Node onode = NodeCreator.createOFNode(1L);
        nodeSet.add(null);
        nodeSet.add(cnode);
        nodeSet.add(onode);

        NodeConnector nc
            = NodeConnectorCreator.createOFNodeConnector(Short.valueOf((short)10), cnode);

        // add a vlanmap to a vbridge
        for (Node node : nodeSet) {
            for (short vlan : vlans) {
                VlanMapConfig vlconf = new VlanMapConfig(node, vlan);
                String emsg = "(VlanMapConfig)" + vlconf.toString();
                VlanMap map = null;
                try {
                    map = mgr.addVlanMap(bpath, vlconf);
                    if (node != null
                            && node.getType() != NodeConnectorIDType.OPENFLOW) {
                        fail("throwing Exception was expected.");
                    }
                } catch (VTNException e) {
                    if (node != null
                            && node.getType() == NodeConnectorIDType.OPENFLOW) {
                        unexpected(e);
                    } else {
                        continue;
                    }
                }

                VlanMap getmap = null;
                try {
                    getmap = mgr.getVlanMap(bpath, map.getId());
                } catch (VTNException e) {
                    unexpected(e);
                }
                assertEquals(emsg, getmap.getId(), map.getId());
                assertEquals(emsg, getmap.getNode(), node);
                assertEquals(emsg, getmap.getVlan(), vlan);
                checkNodeStatus(mgr, bpath, ifp, VNodeState.UP,
                                VNodeState.UNKNOWN, emsg);

                // test for notification APIs.
                checkNotification(mgr, bpath, ifp, nc, cnode, null, vlconf,
                                  MapType.VLAN, emsg);

                st = mgr.removeVlanMap(bpath, map.getId());
                assertEquals(emsg, StatusCode.SUCCESS, st.getCode());
            }
        }

        st = mgr.removeTenant(tpath);
        assertEquals(StatusCode.SUCCESS, st.getCode());
    }

    /**
     * same test with {@code testNotifyNodeAndNodeConnectorWithPortMap}.
     * this test case do with both port map and vlan map set.
     */
    @Test
    public void  testNotificationWithBothMapped() {
        VTNManagerImpl mgr = vtnMgr;
        Set<Node> nodeSet = new HashSet<Node>();

        short[] vlans = new short[] {0, 10, 4095};

        String tname = "vtn";
        VTenantPath tpath = new VTenantPath(tname);
        String bname = "vbridge";
        VBridgePath bpath = new VBridgePath(tname, bname);
        VBridgeIfPath ifp = new VBridgeIfPath(tname, bname, "vinterface");
        Status st = null;

        List<VBridgePath> bpathlist = new ArrayList<VBridgePath>();
        List<VBridgeIfPath> ifplist = new ArrayList<VBridgeIfPath>();
        bpathlist.add(bpath);
        ifplist.add(ifp);
        createTenantAndBridgeAndInterface(mgr, tpath, bpathlist, ifplist);

        PortMap pmap = null;
        try {
            pmap = mgr.getPortMap(ifp);
        } catch (Exception e) {
            unexpected(e);
        }
        assertNull(pmap);

        Node pnode = NodeCreator.createOFNode(0L);
        Node onode = NodeCreator.createOFNode(1L);
        nodeSet.add(null);
        nodeSet.add(pnode);
        nodeSet.add(onode);

        SwitchPort[] ports = new SwitchPort[] {
            new SwitchPort("port-10", NodeConnectorIDType.OPENFLOW, "10"),
            new SwitchPort(null, NodeConnectorIDType.OPENFLOW, "11"),
            new SwitchPort("port-10", null, null),
            new SwitchPort("port-10"),
            new SwitchPort(NodeConnectorIDType.OPENFLOW, "13"),
        };

        for (Node cnode : nodeSet) {
            if (cnode == null) {
                continue;
            }
            NodeConnector cnc = NodeConnectorCreator.
                createOFNodeConnector(Short.valueOf((short)10), cnode);

            for (Node pmapNode : nodeSet) {
                if (pmapNode == null) {
                    continue;
                }
                for (SwitchPort port : ports) {
                    for (short vlan : vlans) {
                        // add port map
                        PortMapConfig pmconf = new PortMapConfig(pmapNode, port,
                                                                 (short)vlan);
                        String emsg = "(PortMapConfig)" + pmconf.toString();

                        st = mgr.setPortMap(ifp, pmconf);
                        assertEquals(emsg, StatusCode.SUCCESS, st.getCode());

                        PortMap map = null;
                        try {
                            map = mgr.getPortMap(ifp);
                        } catch (Exception e) {
                            unexpected(e);
                        }
                        assertEquals(emsg, pmconf, map.getConfig());
                        assertNotNull(emsg, map.getNodeConnector());
                        if (port.getId() != null) {
                            assertEquals(emsg,
                                         Short.parseShort(port.getId()),
                                         map.getNodeConnector().getID());
                        }
                        if (port.getType() != null) {
                            assertEquals(emsg,
                                         port.getType(),
                                         map.getNodeConnector().getType());
                        }
                        checkNodeStatus(mgr, bpath, ifp, VNodeState.UP,
                                        VNodeState.UP, emsg);

                        // add a vlanmap to a vbridge
                        for (Node vmapNode : nodeSet) {
                            for (short vvlan : vlans) {
                                VlanMapConfig vlconf = new VlanMapConfig(vmapNode,
                                                                         vvlan);
                                String emsgVmap = "(VlanMapConfig)"
                                        + vlconf.toString() + "," + emsg;
                                VlanMap vmap = null;
                                try {
                                    vmap = mgr.addVlanMap(bpath, vlconf);
                                    if (vmapNode != null
                                            && vmapNode.getType()
                                                != NodeConnectorIDType.OPENFLOW) {
                                        fail("throwing Exception was expected.");
                                    }
                                } catch (VTNException e) {
                                    if (vmapNode != null
                                            && vmapNode.getType()
                                                == NodeConnectorIDType.OPENFLOW) {
                                        unexpected(e);
                                    } else {
                                        continue;
                                    }
                                }

                                VlanMap getmap = null;
                                try {
                                    getmap = mgr.getVlanMap(bpath, vmap.getId());
                                } catch (VTNException e) {
                                    unexpected(e);
                                }
                                assertEquals(emsgVmap, getmap.getId(), vmap.getId());
                                assertEquals(emsgVmap, getmap.getNode(), vmapNode);
                                assertEquals(emsgVmap, getmap.getVlan(), vvlan);
                                checkNodeStatus(mgr, bpath, ifp, VNodeState.UP,
                                                VNodeState.UP, emsgVmap);

                                // test for notification APIs.
                                checkNotification(mgr, bpath, ifp, cnc, cnode,
                                                  pmconf, vlconf, MapType.ALL,
                                                  emsgVmap);

                                st = mgr.removeVlanMap(bpath, vmap.getId());
                                assertEquals(emsgVmap, StatusCode.SUCCESS,
                                             st.getCode());
                            }
                        }
                    }
                }
            }
        }

        st = mgr.removeTenant(tpath);
        assertEquals(StatusCode.SUCCESS, st.getCode());
    }

    /**
     * do test of Notification APIs.
     *
     * @param mgr       VTN Manager service.
     * @param bpath     A {@link VBridgePath} which is checked.
     * @param ifp       A {@link VBridgeIfPath} which is checked.
     * @param chgNc     A {@link NodeConnector} which is changed status
     * @param chgNode   A {@link Node} which is changed status
     * @param pmconf    A {@link PortMapConfig}. If there are no portmap,
     *                  specify null.
     * @param vlconf    A {@link VlanMapConfig}. If there are no vlanmap,
     *                  specify null.
     * @param mapType   {@link MapType}.
     * @param msg       A string output when assertion is failed.
     */

    private void checkNotification(VTNManagerImpl mgr, VBridgePath bpath,
                                   VBridgeIfPath ifp, NodeConnector chgNc,
                                   Node chgNode, PortMapConfig pmconf,
                                   VlanMapConfig vlconf, MapType mapType,
                                   String msg) {
        ISwitchManager swMgr = mgr.getSwitchManager();
        ITopologyManager topoMgr = mgr.getTopologyManager();
        VBridgePath mapPath = createMapPath(bpath, ifp, pmconf, vlconf);

        NodeConnector mapNc = null;
        Node portMapNode = null;
        Node vlanMapNode = null;
        String portName = null;
        if (pmconf != null) {
            portName = pmconf.getPort().getName();
            Short s;
            if (pmconf.getPort().getId() == null) {
                String [] tkn = portName.split("-");
                s = Short.valueOf(tkn[1]);
            } else {
                s = Short.valueOf(pmconf.getPort().getId());
            }
            mapNc = NodeConnectorCreator.createOFNodeConnector(s, pmconf.getNode());
            portMapNode = mapNc.getNode();
        }
        if (vlconf != null) {
            vlanMapNode = vlconf.getNode();
        }

        // test for nodeconnector change notify.
        Map<String, Property> propMap = null;
        checkMacTableEntry(mgr, bpath, true, msg);
        putMacTableEntry(mgr, mapPath, chgNc);
        mgr.notifyNodeConnector(chgNc, UpdateType.REMOVED, propMap);
        if (mapType.equals(MapType.PORT) || mapType.equals(MapType.ALL)) {
            if (chgNc.equals(mapNc)) {
                checkNodeStatus(mgr, bpath, ifp, VNodeState.DOWN, VNodeState.DOWN, msg);
            } else {
                checkNodeStatus(mgr, bpath, ifp, VNodeState.UP, VNodeState.UP, msg);
            }
        } else {
            checkNodeStatus(mgr, bpath, ifp, VNodeState.UP, VNodeState.UNKNOWN, msg);
        }

        // When a NodeConnector is removed, MAC adddress table entries
        // detected by the port corresponding to removed NodeConnector are
        // always flushed.
        checkMacTableEntry(mgr, bpath, true, msg);

        propMap = swMgr.getNodeConnectorProps(chgNc);
        Name chgNcName = (Name)propMap.get(Name.NamePropName);
        mgr.notifyNodeConnector(chgNc, UpdateType.ADDED, propMap);
        mgr.initISL();
        if (mapType.equals(MapType.PORT) || mapType.equals(MapType.ALL)) {
            checkNodeStatus(mgr, bpath, ifp, VNodeState.UP, VNodeState.UP, msg);
        } else {
            checkNodeStatus(mgr, bpath, ifp, VNodeState.UP, VNodeState.UNKNOWN, msg);
        }

        // add again
        mgr.notifyNodeConnector(chgNc, UpdateType.ADDED, propMap);
        if (mapType.equals(MapType.PORT) || mapType.equals(MapType.ALL)) {
            checkNodeStatus(mgr, bpath, ifp, VNodeState.UP, VNodeState.UP, msg);
        } else {
            checkNodeStatus(mgr, bpath, ifp, VNodeState.UP, VNodeState.UNKNOWN, msg);
        }

        // Change the port name.
        Map<String, Property> newPropMap = new HashMap<String, Property>();
        newPropMap.put(Name.NamePropName, new Name(""));
        newPropMap.put(Config.ConfigPropName, new Config(Config.ADMIN_UP));
        newPropMap.put(State.StatePropName, new State(State.EDGE_UP));

        mgr.notifyNodeConnector(chgNc, UpdateType.CHANGED, newPropMap);
        if (mapType.equals(MapType.PORT) || mapType.equals(MapType.ALL)) {
            VNodeState expected = (!chgNc.equals(mapNc) || portName == null ||
                                   portName.equals(""))
                ? VNodeState.UP : VNodeState.DOWN;
            checkNodeStatus(mgr, bpath, ifp, expected, expected, msg);
        } else {
            checkNodeStatus(mgr, bpath, ifp, VNodeState.UP, VNodeState.UNKNOWN, msg);
        }
        flushMacTableEntry(mgr, bpath);

        // Restore the port name.
        newPropMap = new HashMap<String, Property>();
        if (chgNcName != null) {
            newPropMap.put(Name.NamePropName, chgNcName);
        }
        newPropMap.put(Config.ConfigPropName, new Config(Config.ADMIN_UP));
        newPropMap.put(State.StatePropName, new State(State.EDGE_UP));

        mgr.notifyNodeConnector(chgNc, UpdateType.CHANGED, newPropMap);
        if (mapType.equals(MapType.PORT) || mapType.equals(MapType.ALL)) {
            checkNodeStatus(mgr, bpath, ifp, VNodeState.UP, VNodeState.UP, msg);
        } else {
            checkNodeStatus(mgr, bpath, ifp, VNodeState.UP, VNodeState.UNKNOWN, msg);
        }
        flushMacTableEntry(mgr, bpath);

        // test for node change notify.
        putMacTableEntry(mgr, mapPath, chgNc);
        mgr.notifyNode(chgNode, UpdateType.REMOVED, propMap);

        if (mapType.equals(MapType.PORT) || mapType.equals(MapType.ALL)) {
            if (chgNode.equals(portMapNode)) {
                checkNodeStatus(mgr, bpath, ifp, VNodeState.DOWN,
                                VNodeState.DOWN, msg);
            } else if (chgNode.equals(vlanMapNode)) {
                checkNodeStatus(mgr, bpath, ifp, VNodeState.DOWN,
                                VNodeState.UP, msg);
            } else {
                checkNodeStatus(mgr, bpath, ifp, VNodeState.UP,
                                VNodeState.UP, msg);
            }
        } else {
            if (chgNode.equals(vlanMapNode)) {
                checkNodeStatus(mgr, bpath, ifp, VNodeState.DOWN,
                                VNodeState.UNKNOWN, msg);
            } else {
                checkNodeStatus(mgr, bpath, ifp, VNodeState.UP,
                                VNodeState.UNKNOWN, msg);
            }
        }

        // When a Node is removed, MAC adddress table entries detected by
        // the switch corresponding to removed Node are always flushed.
        checkMacTableEntry(mgr, bpath, chgNode.equals(chgNc.getNode()), msg);
        flushMacTableEntry(mgr, bpath);

        mgr.initInventory();
        mgr.initISL();
        mgr.getNodeDB().remove(chgNode);
        mgr.notifyNode(chgNode, UpdateType.ADDED, propMap);
        if (mapType.equals(MapType.PORT) || mapType.equals(MapType.ALL)) {
            if (chgNode.equals(portMapNode)) {
                checkNodeStatus(mgr, bpath, ifp, VNodeState.DOWN, VNodeState.DOWN, msg);
            } else {
                checkNodeStatus(mgr, bpath, ifp, VNodeState.UP, VNodeState.UP, msg);
            }
        } else {
            checkNodeStatus(mgr, bpath, ifp, VNodeState.UP, VNodeState.UNKNOWN, msg);
        }

        if (mapType.equals(MapType.PORT) || mapType.equals(MapType.ALL)) {
            mgr.initInventory();
            mgr.initISL();
            mgr.getPortDB().remove(mapNc);
            propMap = swMgr.getNodeConnectorProps(mapNc);
            mgr.notifyNodeConnector(mapNc, UpdateType.ADDED, propMap);
            checkNodeStatus(mgr, bpath, ifp, VNodeState.UP, VNodeState.UP, msg);
        }

        mgr.notifyNode(chgNode, UpdateType.CHANGED, propMap);
        if (mapType.equals(MapType.PORT) || mapType.equals(MapType.ALL)) {
            checkNodeStatus(mgr, bpath, ifp, VNodeState.UP, VNodeState.UP, msg);
        } else {
            checkNodeStatus(mgr, bpath, ifp, VNodeState.UP, VNodeState.UNKNOWN, msg);
        }

        // test for a edge changed notification
        Map<Edge, Set<Property>> edges = topoMgr.getEdges();
        for (Edge edge: edges.keySet()) {
            List<TopoEdgeUpdate> topoList = new ArrayList<TopoEdgeUpdate>();
            String emsgEdge = msg + ",(Edge)" + edge.toString();

            TopoEdgeUpdate update =
                new TopoEdgeUpdate(edge, null, UpdateType.REMOVED);
            topoList.add(update);

            Edge reverseEdge = null;
            try {
                reverseEdge = new Edge(edge.getHeadNodeConnector(),
                        edge.getTailNodeConnector());
            } catch (ConstructionException e) {
                unexpected(e);
            }
            update = new TopoEdgeUpdate(reverseEdge, null, UpdateType.REMOVED);
            topoList.add(update);

            stubObj.deleteEdge(edge);
            stubObj.deleteEdge(reverseEdge);
            mgr.edgeUpdate(topoList);
            checkNodeStatus(mgr, bpath, ifp, VNodeState.UP,
                    (mapType.equals(MapType.PORT) || mapType.equals(MapType.ALL)) ? VNodeState.UP : VNodeState.UNKNOWN,
                    emsgEdge);

            topoList.clear();
            update = new TopoEdgeUpdate(edge, null, UpdateType.ADDED);
            topoList.add(update);
            update = new TopoEdgeUpdate(reverseEdge, null, UpdateType.ADDED);
            topoList.add(update);

            stubObj.addEdge(edge);
            stubObj.addEdge(reverseEdge);
            mgr.edgeUpdate(topoList);
            checkNodeStatus(mgr, bpath, ifp, VNodeState.UP,
                    (mapType.equals(MapType.PORT) || mapType.equals(MapType.ALL)) ? VNodeState.UP : VNodeState.UNKNOWN,
                            emsgEdge);

            topoList.clear();
            update = new TopoEdgeUpdate(edge, null, UpdateType.CHANGED);
            topoList.add(update);
            update = new TopoEdgeUpdate(reverseEdge, null, UpdateType.CHANGED);
            topoList.add(update);

            mgr.edgeUpdate(topoList);
            checkNodeStatus(mgr, bpath, ifp, VNodeState.UP,
                    (mapType.equals(MapType.PORT) || mapType.equals(MapType.ALL)) ? VNodeState.UP : VNodeState.UNKNOWN,
                            emsgEdge);
        }

        // a edge consist of unmapped nodeconnector updated
        Node node = NodeCreator.createOFNode(Long.valueOf(10));
        NodeConnector head = NodeConnectorCreator.createOFNodeConnector(Short.valueOf((short)10), node);
        NodeConnector tail = NodeConnectorCreator.createOFNodeConnector(Short.valueOf((short)11), node);
        Edge edge = null;
        try {
            edge = new Edge(head, tail);
        } catch (ConstructionException e) {
            unexpected(e);
        }
        List<TopoEdgeUpdate> topoList = new ArrayList<TopoEdgeUpdate>();
        TopoEdgeUpdate update = new TopoEdgeUpdate(edge, null, UpdateType.ADDED);
        topoList.add(update);
        mgr.edgeUpdate(topoList);
        checkNodeStatus(mgr, bpath, ifp, VNodeState.UP,
                (mapType.equals(MapType.PORT) || mapType.equals(MapType.ALL)) ? VNodeState.UP : VNodeState.UNKNOWN,
                msg + "," + edge.toString());

        // Reset inventory cache.
        mgr.initInventory();
        mgr.initISL();
    }

    /**
     * Test method for
     * {@link VTNManagerImpl#recalculateDone()}
     */
    @Test
    public void testRecalculateDone() {
        VTNManagerImpl mgr = vtnMgr;
        TestStub stub0 = new TestStub(0);
        TestStub stub3 = new TestStub(3);
        VTNManagerAwareStub awareStub = new VTNManagerAwareStub();

        mgr.setSwitchManager(stub3);
        mgr.setTopologyManager(stub3);
        mgr.setRouting(stub0);
        mgr.initInventory();
        mgr.initISL();
        mgr.clearDisabledNode();

        // setup vbridge
        String tname = "vtn";
        VTenantPath tpath = new VTenantPath(tname);
        String bname = "vbridge";
        VBridgePath bpath = new VBridgePath(tname, bname);
        VBridgeIfPath ifp0 = new VBridgeIfPath(tname, bname, "vinterface0");
        VBridgeIfPath ifp1 = new VBridgeIfPath(tname, bname, "vinterface1");
        String ifname2 = "vinterface2";
        VBridgeIfPath ifp2 = new VBridgeIfPath(tname, bname, ifname2);

        List<VBridgePath> bpathlist = new ArrayList<VBridgePath>();
        List<VBridgeIfPath> ifplist = new ArrayList<VBridgeIfPath>();
        bpathlist.add(bpath);
        ifplist.add(ifp0);
        ifplist.add(ifp1);
        ifplist.add(ifp2);
        createTenantAndBridgeAndInterface(mgr, tpath, bpathlist, ifplist);
        flushTasks();

        // Append one more listener to catch port mapping change events.
        VTNManagerAwareStub pmapAware = new VTNManagerAwareStub();
        mgr.addVTNManagerAware(pmapAware);
        flushTasks();
        pmapAware.checkVtnInfo(1, tpath, UpdateType.ADDED);
        pmapAware.checkVbrInfo(1, bpath, UpdateType.ADDED);
        pmapAware.checkVIfInfo(3, ifp2, UpdateType.ADDED);

        Node node0 = NodeCreator.createOFNode(0L);
        SwitchPort port = new SwitchPort(null, NodeConnectorIDType.OPENFLOW,
                                         "10");
        PortMapConfig pmconf = new PortMapConfig(node0, port, (short)0);
        Status st = mgr.setPortMap(ifp0, pmconf);
        assertEquals(StatusCode.SUCCESS, st.getCode());
        Node node1 = NodeCreator.createOFNode(1L);
        pmconf = new PortMapConfig(node1, port, (short)0);
        st = mgr.setPortMap(ifp1, pmconf);
        assertEquals(StatusCode.SUCCESS, st.getCode());
        Node node2 = NodeCreator.createOFNode(2L);
        pmconf = new PortMapConfig(node2, port, (short)0);
        st = mgr.setPortMap(ifp2, pmconf);
        assertEquals(StatusCode.SUCCESS, st.getCode());

        // Ensure that no port mapping change event is delivered to awareStub.
        pmapAware.checkPmapInfo(3, ifp2, pmconf, UpdateType.ADDED);
        mgr.removeVTNManagerAware(pmapAware);

        flushTasks();
        mgr.addVTNManagerAware(awareStub);
        awareStub.checkVtnInfo(1, tpath, UpdateType.ADDED);
        awareStub.checkVbrInfo(1, bpath, UpdateType.ADDED);
        awareStub.checkVIfInfo(3, ifp2, UpdateType.ADDED);
        awareStub.checkPmapInfo(3, ifp2, pmconf, UpdateType.ADDED);

        // call with no fault status
        mgr.recalculateDone();

        // add fault path
        byte[] src0 = new byte[] {(byte)0x00, (byte)0x00, (byte)0x00,
                                  (byte)0x11, (byte)0x11, (byte)0x11};
        byte[] src1 = new byte[] {(byte)0x00, (byte)0x00, (byte)0x00,
                                  (byte)0x22, (byte)0x22, (byte)0x22};
        byte[] src2 = new byte[] {(byte)0x00, (byte)0x00, (byte)0x00,
                                  (byte)0x33, (byte)0x33, (byte)0x33};
        byte[] dst = new byte[] {(byte)0xff, (byte)0xff, (byte)0xff,
                                 (byte)0xff, (byte)0xff, (byte)0xff};
        byte[] sender1 = new byte[] {(byte)192, (byte)168, (byte)0, (byte)1};
        byte[] sender2 = new byte[] {(byte)192, (byte)168, (byte)0, (byte)2};
        byte[] target = new byte[] {(byte)192, (byte)168, (byte)0, (byte)250};
        short vlan = 0;

        NodeConnector nc1
            = NodeConnectorCreator.createOFNodeConnector(Short.valueOf("10"), node1);
        RawPacket inPkt = createARPRawPacket(src1, dst, sender1, target,
                                             (vlan > 0) ? vlan : -1,
                                             nc1, ARP.REQUEST);
        PacketResult result = mgr.receiveDataPacket(inPkt);
        assertEquals(PacketResult.KEEP_PROCESSING, result);

        NodeConnector nc2
            = NodeConnectorCreator.createOFNodeConnector(Short.valueOf("10"), node2);
        inPkt = createARPRawPacket(src2, dst, sender2, target,
                                   (vlan > 0) ? vlan : -1,
                                   nc2, ARP.REQUEST);
        result = mgr.receiveDataPacket(inPkt);
        assertEquals(PacketResult.KEEP_PROCESSING, result);

        NodeConnector nc0
            = NodeConnectorCreator.createOFNodeConnector(Short.valueOf("10"), node0);
        inPkt = createARPRawPacket(src0, src1, target, sender1,
                                   (vlan > 0) ? vlan : -1,
                                   nc0, ARP.REPLY);
        result = mgr.receiveDataPacket(inPkt);
        assertEquals(PacketResult.KEEP_PROCESSING, result);
        awareStub.checkVbrInfo(1, bpath, UpdateType.CHANGED);

        inPkt = createARPRawPacket(src0, src2, target, sender2,
                                   (vlan > 0) ? vlan : -1,
                                   nc0, ARP.REPLY);
        result = mgr.receiveDataPacket(inPkt);
        assertEquals(PacketResult.KEEP_PROCESSING, result);
        awareStub.checkVbrInfo(1, bpath, UpdateType.CHANGED);
        checkVBridgeStatus(mgr, bpath, 2, VNodeState.DOWN);

        // received again. expect status is kept.
        result = mgr.receiveDataPacket(inPkt);
        assertEquals(PacketResult.KEEP_PROCESSING, result);
        awareStub.checkVbrInfo(0, null, null);

        pmconf = new PortMapConfig(node2, port, (short)0);
        st = mgr.setPortMap(ifp2, pmconf);
        checkVBridgeStatus(mgr, bpath, 2, VNodeState.DOWN);

        // expected not to recover from fault state
        mgr.recalculateDone();
        checkVBridgeStatus(mgr, bpath, 2, VNodeState.DOWN);
        awareStub.checkVbrInfo(0, null, null);

        // change path settings
        mgr.setRouting(stubObj);

        // expected to 1 path recover from fault state
        mgr.recalculateDone();
        checkVBridgeStatus(mgr, bpath, 1, VNodeState.DOWN);
        awareStub.checkVbrInfo(1, bpath, UpdateType.CHANGED);

        // change path settings
        mgr.setRouting(stub3);

        // expected to recover from fault state
        mgr.recalculateDone();
        checkVBridgeStatus(mgr, bpath, 0, VNodeState.UP);
        awareStub.checkVbrInfo(1, bpath, UpdateType.CHANGED);

        mgr.setSwitchManager(stubObj);
        mgr.setTopologyManager(stubObj);
        mgr.setRouting(stubObj);
        mgr.initInventory();
        mgr.initISL();
        mgr.clearDisabledNode();

        st = mgr.removeTenant(tpath);
        assertEquals(StatusCode.SUCCESS, st.getCode());
    }

    /**
     *  check status of {@link VBridge}.
     *  (used in {@code testRecalculate()})
     *
     * @param mgr       VTN Manager service.
     * @param bpath     A {@link VBridgePath} object.
     * @param faults    number of fault path.
     * @param state     A state of {@link VBridge} expected.
     */
    private void checkVBridgeStatus(VTNManagerImpl mgr, VBridgePath bpath,
                                    int faults, VNodeState state) {
        VBridge brdg = null;
        try {
            brdg = mgr.getBridge(bpath);
        } catch (VTNException e) {
            unexpected(e);
        }
        VNodeState bstate = brdg.getState();
        assertEquals(faults, brdg.getFaults());
        assertEquals(state, bstate);
    }

    /**
     * Test method for {@link VTNManagerImpl#receiveDataPacket(RawPacket)}.
     * The case PortMap applied to vBridge.
     */
    @Test
    public void testReceiveDataPacketPortMapped() {
        VTNManagerImpl mgr = vtnMgr;
        TestStub stub = stubObj;
        ISwitchManager swmgr = mgr.getSwitchManager();
        short[] vlans = new short[] {0, 10, 4095};
        byte[] cntMac = swmgr.getControllerMAC();
        ConcurrentMap<VBridgePath, Set<NodeConnector>> mappedConnectors
            = new ConcurrentHashMap<VBridgePath, Set<NodeConnector>>();
        Status st = null;

        String tname = "vtn";
        VTenantPath tpath = new VTenantPath(tname);
        String bname1 = "vbridge1";
        VBridgePath bpath1 = new VBridgePath(tname, bname1);
        String bname2 = "vbridge2";
        VBridgePath bpath2 = new VBridgePath(tname, bname2);
        List<VBridgePath> bpathlist = new ArrayList<VBridgePath>();
        bpathlist.add(bpath1);
        bpathlist.add(bpath2);
        createTenantAndBridge(mgr, tpath, bpathlist);

        mappedConnectors.put(bpath1, new HashSet<NodeConnector>());
        mappedConnectors.put(bpath2, new HashSet<NodeConnector>());

        Set<Node> existNodes = swmgr.getNodes();
        Set<NodeConnector> existConnectors = new HashSet<NodeConnector>();
        for (Node node: existNodes) {
            existConnectors.addAll(swmgr.getNodeConnectors(node));
        }

        // add interface
        for (short i = 0; i < 4; i++) {
            int inode = 0;
            for (Node node : existNodes) {
                String ifname = "vinterface" + inode + (i + 10);
                VBridgeIfPath ifp = new VBridgeIfPath(tname, (i < 2) ? bname1 : bname2,
                                                      ifname);
                st = mgr.addInterface(ifp, new VInterfaceConfig(null, Boolean.TRUE));
                assertEquals("(VBridgeIfPath)" + ifp.toString(),
                        StatusCode.SUCCESS, st.getCode());
                inode++;
            }
        }

        // null case
        PacketResult result = mgr.receiveDataPacket(null);
        assertEquals(PacketResult.IGNORED, result);

        for (short vlan : vlans) {
            // add port mapping
            Set<PortVlan> mappedThis = new HashSet<PortVlan>();

            for (short i = 0; i < 4; i++) {
                int inode = 0;
                for (Node node : existNodes) {
                    short setvlan = (i != 1) ? vlan : 100;
                    String ifname = "vinterface" + inode + (i + 10);
                    VBridgeIfPath ifp = new VBridgeIfPath(tname, (i < 2) ? bname1 : bname2,
                                                            ifname);

                    SwitchPort port = new SwitchPort(NodeConnectorIDType.OPENFLOW,
                                                    String.valueOf(i + 10));
                    PortMapConfig pmconf = new PortMapConfig(node, port, setvlan);
                    st = mgr.setPortMap(ifp, pmconf);
                    assertEquals("(VBridgeIfPath)" + ifp.toString()
                            + ",(PortMapConfig)" + pmconf.toString(),
                            StatusCode.SUCCESS, st.getCode());

                    NodeConnector mapnc = NodeConnectorCreator.createOFNodeConnector(
                                                        Short.valueOf((short)(i + 10)), node);
                    Set<NodeConnector> set = mappedConnectors.
                        get((i < 2) ? bpath1 : bpath2);
                    set.add(mapnc);
                    mappedConnectors.put((i < 2) ? bpath1 : bpath2, set);
                    if (i < 2) {
                        mappedThis.add(new PortVlan(mapnc, setvlan));
                    }

                    inode++;
                }
            }

            testReceiveDataPacketBCLoop(mgr, bpath1, MapType.PORT, mappedThis, stub);

            st = mgr.flushMacEntries(bpath1);
            assertEquals("(vlan)" + vlan, StatusCode.SUCCESS, st.getCode());

//            testReceiveDataPacketUCLoopLearned(mgr, bpath1, MapType.PORT, mappedThis, stub);
//            st = mgr.flushMacEntries(bpath1);
//            assertTrue("vlan=" + vlan, st.isSuccess());

            testReceiveDataPacketUCLoop(mgr, bpath1, MapType.PORT, mappedThis, stub);

            st = mgr.flushMacEntries(bpath1);
            assertEquals("(vlan)" + vlan, StatusCode.SUCCESS, st.getCode());

            testReceiveDataPacketUCLoopLearned(mgr, bpath1, MapType.PORT, mappedThis, stub);


            // in case received a packet from controller.
            for (PortVlan pv : mappedThis) {
                byte [] dst = new byte[] {(byte)0xff, (byte)0xff, (byte)0xff,
                                          (byte)0xff, (byte)0xff, (byte)0xff};
                byte [] sender = new byte[] {(byte)192, (byte)168, (byte)0, (byte)1};
                byte [] target = new byte[] {(byte)192, (byte)168, (byte)0, (byte)250};
                String emsg = "(PortVlan)" + pv.toString();
                NodeConnector nc = pv.getNodeConnector();

                RawPacket inPkt = createARPRawPacket(cntMac, dst, sender, target,
                                                (pv.getVlan() > 0) ? pv.getVlan() : -1,
                                                        nc, ARP.REQUEST);
                result = mgr.receiveDataPacket(inPkt);
                assertEquals(emsg, PacketResult.IGNORED, result);

                st = mgr.flushMacEntries(bpath1);
                assertEquals(emsg, StatusCode.SUCCESS, st.getCode());
            }
        }

        st = mgr.removeBridge(bpath2);
        assertEquals(StatusCode.SUCCESS, st.getCode());

        // in case received packet from no mapped port.
        byte[] dst = new byte[] {(byte)0xff, (byte)0xff, (byte)0xff,
                                 (byte)0xff, (byte)0xff, (byte)0xff};
        byte[] sender = new byte[] {(byte)192, (byte)168, (byte)0, (byte)1};
        byte[] target = new byte[] {(byte)192, (byte)168, (byte)0, (byte)250};

        for (NodeConnector nc: mappedConnectors.get(bpath2)) {
            byte iphost = 1;
            for (EthernetAddress ea: createEthernetAddresses(false)) {
                byte [] bytes = ea.getValue();
                byte [] src = new byte[] {bytes[0], bytes[1], bytes[2],
                                          bytes[3], bytes[4], bytes[5]};
                sender[3] = iphost;
                String emsg = "(NodeConnector)" + nc.toString()
                        + ",(EthernetAddress)" + ea.toString();
                RawPacket inPkt = createARPRawPacket(src, dst, sender, target,
                                                    (short)-1, nc, ARP.REQUEST);

                result = mgr.receiveDataPacket(inPkt);
                assertEquals(emsg, PacketResult.IGNORED, result);

                List<RawPacket> dataList = stub.getTransmittedDataPacket();
                assertEquals(emsg, 0, dataList.size());
                iphost++;
            }
        }

        // in case received packet from internal port.
        byte[] src = new byte[] {(byte)0x00, (byte)0x00, (byte)0x00,
                                 (byte)0x11, (byte)0x11, (byte)0x11};
        Node node = NodeCreator.createOFNode(0L);
        NodeConnector innc = NodeConnectorCreator.createOFNodeConnector(
                Short.valueOf((short)15), node);
        RawPacket inPkt = createARPRawPacket(src, dst, sender, target, (short)-1,
                                             innc, ARP.REQUEST);

        result = mgr.receiveDataPacket(inPkt);
        assertEquals(PacketResult.IGNORED, result);
        List<RawPacket> dataList = stub.getTransmittedDataPacket();
        assertEquals(0, dataList.size());

        // not Ethernet Packet
        Ethernet inPktDecoded = (Ethernet)stub.decodeDataPacket(inPkt);
        ARP arp = (ARP)inPktDecoded.getPayload();
        RawPacket arpPkt = stub.encodeDataPacket(arp);
        NodeConnector nc = NodeConnectorCreator.createOFNodeConnector(
                Short.valueOf((short)10), node);
        arpPkt.setIncomingNodeConnector(nc);

        result = mgr.receiveDataPacket(arpPkt);
        assertEquals(PacketResult.IGNORED, result);
        dataList = stub.getTransmittedDataPacket();
        assertEquals(0, dataList.size());

        // in case out PortVlan is not match.
        byte[] src2 = new byte[] {(byte)0x00, (byte)0x00, (byte)0x00,
                                  (byte)0x11, (byte)0x11, (byte)0x12};
        MacAddressTable table = mgr.getMacAddressTable(bpath1);
        PacketContext pctx = createARPPacketContext(src, dst, sender, target,
                                                    (short)99, nc, ARP.REQUEST);
        table.flush();
        // table.add(pctx);

        VBridgeIfPath ifp1 = new VBridgeIfPath(tname, bname1, "vinterface010");
        PortMap map = null;
        try {
            map = mgr.getPortMap(ifp1);
        } catch (VTNException e) {
            unexpected(e);
        }
        short confVlan = map.getConfig().getVlan();
        inPkt = createIPv4RawPacket(src2, src, target, sender,
                    (confVlan > 0) ? confVlan : -1, map.getNodeConnector());
        result = mgr.receiveDataPacket(inPkt);
        assertEquals(PacketResult.KEEP_PROCESSING, result);
        dataList = stub.getTransmittedDataPacket();

        // receive flood + probe packet to incoming nodeconnector.
        assertEquals(mappedConnectors.get(bpath1).size(), dataList.size());
        MacTableEntry ent = table.get(MacAddressTable.getTableKey(dst));
        assertNull(ent);

        // in case outgoing interface is disabled.
        VBridgeIfPath ifp2 = new VBridgeIfPath(tname, bname1, "vinterface011");
        map = null;
        try {
            map = mgr.getPortMap(ifp2);
        } catch (VTNException e) {
            unexpected(e);
        }
        confVlan = map.getConfig().getVlan();

        st = mgr.modifyInterface(
            ifp1, new VInterfaceConfig(null, Boolean.FALSE), true);

        inPkt = createIPv4RawPacket(src, src2, sender, target,
                (confVlan > 0) ? confVlan : -1, map.getNodeConnector());
        result = mgr.receiveDataPacket(inPkt);
        assertEquals(PacketResult.KEEP_PROCESSING, result);
        dataList = stub.getTransmittedDataPacket();

        // 3 packets should be transmitted because MAC address table entries
        // relevant to the disabled interface are removed.
        //   - An ARP request to determine IP address of the source host.
        //   - Flooding to enabled interfaces except for incoming interface.
        assertEquals(3, dataList.size());

        st = mgr.modifyInterface(
            ifp1, new VInterfaceConfig(null, Boolean.TRUE), true);

        table.flush();

        // in case dst is controller
        inPkt = createARPRawPacket(src, cntMac, sender, target,
                (confVlan > 0) ? confVlan : -1, map.getNodeConnector(), ARP.REQUEST);

        result = mgr.receiveDataPacket(inPkt);
        assertEquals(PacketResult.KEEP_PROCESSING, result);
        dataList = stub.getTransmittedDataPacket();
        assertEquals(0, dataList.size());

        // in case received disable vinterface.
        st = mgr.modifyInterface(
            ifp1, new VInterfaceConfig(null, Boolean.FALSE), true);
        assertEquals(StatusCode.SUCCESS, st.getCode());
        try {
            map = mgr.getPortMap(ifp1);
        } catch (VTNException e) {
            unexpected(e);
        }
        confVlan = map.getConfig().getVlan();

        inPkt = createARPRawPacket(src, dst, sender, target,
                (confVlan > 0) ? confVlan : -1, map.getNodeConnector(), ARP.REQUEST);
        result = mgr.receiveDataPacket(inPkt);

        // incoming packet form disabled NodeConnector is discarded.
        assertEquals(PacketResult.CONSUME, result);
        dataList = stub.getTransmittedDataPacket();
        assertEquals(0, dataList.size());

        // remove tenant after test
        st = mgr.removeTenant(tpath);
        assertEquals(StatusCode.SUCCESS, st.getCode());
    }

    /**
     * Test method for {@link VTNManagerImpl#receiveDataPacket(RawPacket)}.
     * The case VlanMap applied to {@link VBridge}.
     */
    @Test
    public void testReceiveDataPacketVlanMapped() {
        VTNManagerImpl mgr = vtnMgr;
        TestStub stub = stubObj;
        ISwitchManager swmgr = mgr.getSwitchManager();
        ITopologyManager topomgr = mgr.getTopologyManager();
        short[] vlans = new short[] {0, 10, 4095};
        byte[] cntMac = swmgr.getControllerMAC();
        Status st = null;

        String tname = "vtn";
        VTenantPath tpath = new VTenantPath(tname);
        String bname1 = "vbridge1";
        VBridgePath bpath1 = new VBridgePath(tname, bname1);
        String bname2 = "vbridge2";
        VBridgePath bpath2 = new VBridgePath(tname, bname2);
        List<VBridgePath> bpathlist = new ArrayList<VBridgePath>();
        bpathlist.add(bpath1);
        bpathlist.add(bpath2);
        createTenantAndBridge(mgr, tpath, bpathlist);

        Set<Node> existNodes = swmgr.getNodes();
        Set<NodeConnector> existConnectors = new HashSet<NodeConnector>();
        for (Node node : existNodes) {
            existConnectors.addAll(swmgr.getNodeConnectors(node));
        }

        Set<Node> loopNodes = new HashSet<Node>(existNodes);
        loopNodes.add(null);
        for (Node mapNode : loopNodes) {
            for (short vlan : vlans) {
                VlanMapConfig vlconf = new VlanMapConfig(mapNode, vlan);
                String emsg = "(VLanMapConfig)" + vlconf.toString();

                VlanMap map = null;
                try {
                    map = mgr.addVlanMap(bpath1, vlconf);
                } catch (VTNException e) {
                    unexpected(e);
                }

                Set<PortVlan> mappedThis = new HashSet<PortVlan>();
                for (Node node : existNodes) {
                    if (mapNode == null || node.equals(mapNode)) {
                        for (NodeConnector nc : swmgr.getNodeConnectors(node)) {
                            if (!topomgr.isInternal(nc)) {
                                mappedThis.add(new PortVlan(nc, vlan));
                            }
                        }
                    }
                }

                testReceiveDataPacketBCLoop(mgr, bpath1, MapType.PORT, mappedThis, stub);

                st = mgr.flushMacEntries(bpath1);
                assertEquals(emsg, StatusCode.SUCCESS, st.getCode());

//                testReceiveDataPacketUCLoopLearned(mgr, bpath1, MapType.PORT, mappedThis, stub);
//                st = mgr.flushMacEntries(bpath1);
//                assertTrue(vlconf.toString(), st.isSuccess());

                testReceiveDataPacketUCLoop(mgr, bpath1, MapType.PORT, mappedThis, stub);

                st = mgr.flushMacEntries(bpath1);
                assertEquals(emsg, StatusCode.SUCCESS, st.getCode());

                testReceiveDataPacketUCLoopLearned(mgr, bpath1, MapType.VLAN, mappedThis,
                                                   stub);

                // in case received from non-mapped port.
                for (Node unmapNode : loopNodes) {
                    if (unmapNode != null && unmapNode.equals(mapNode)) {
                        byte[] src = new byte[] {(byte)0x00, (byte)0x00, (byte)0x00,
                                                 (byte)0x11, (byte)0x11, (byte)0x11};
                        byte[] dst = new byte[] {(byte)0xff, (byte)0xff, (byte)0xff,
                                                 (byte)0xff, (byte)0xff, (byte)0xff};
                        byte[] sender = new byte[] {(byte)192, (byte)168, (byte)0, (byte)1};
                        byte[] target = new byte[] {(byte)192, (byte)168, (byte)0, (byte)250};
                        NodeConnector innc = NodeConnectorCreator
                                .createOFNodeConnector(Short.valueOf((short)10), unmapNode);
                        RawPacket inPkt = createARPRawPacket(src, cntMac, sender, target,
                                (short)99, innc, ARP.REQUEST);

                        PacketResult result = mgr.receiveDataPacket(inPkt);

                        assertEquals(PacketResult.IGNORED, result);
                        List<RawPacket> dataList = stub.getTransmittedDataPacket();
                        assertEquals(emsg, 0, dataList.size());
                    }
                }

                st = mgr.removeVlanMap(bpath1, map.getId());
                assertEquals(emsg, StatusCode.SUCCESS, st.getCode());

                st = mgr.flushMacEntries(bpath1);
                assertEquals(emsg, StatusCode.SUCCESS, st.getCode());
            }
        }

        st = mgr.removeTenant(tpath);
        assertEquals(StatusCode.SUCCESS, st.getCode());
    }

    /**
     * Test method for {@link VTNManagerImpl#receiveDataPacket(RawPacket)}.
     * The case both VlanMap and PortMap applied to {@link VBridge}.
     */
    @Test
    public void testReceiveDataPacketBothMapped() {
        VTNManagerImpl mgr = vtnMgr;
        TestStub stub = stubObj;
        ISwitchManager swmgr = mgr.getSwitchManager();
        ITopologyManager topomgr = mgr.getTopologyManager();
        short[] vlans = new short[] {0, 10, 4095};
        ConcurrentMap<VBridgePath, Set<NodeConnector>> mappedConnectors
            = new ConcurrentHashMap<VBridgePath, Set<NodeConnector>>();
        Status st = null;

        String tname = "vtn";
        VTenantPath tpath = new VTenantPath(tname);
        String bname1 = "vbridge1";
        VBridgePath bpath1 = new VBridgePath(tname, bname1);
        String bname2 = "vbridge2";
        VBridgePath bpath2 = new VBridgePath(tname, bname2);
        List<VBridgePath> bpathlist = new ArrayList<VBridgePath>();
        bpathlist.add(bpath1);
        bpathlist.add(bpath2);
        createTenantAndBridge(mgr, tpath, bpathlist);

        mappedConnectors.put(bpath1, new HashSet<NodeConnector>());
        mappedConnectors.put(bpath2, new HashSet<NodeConnector>());

        Set<Node> existNodes = swmgr.getNodes();
        Set<NodeConnector> existConnectors = new HashSet<NodeConnector>();
        for (Node node: existNodes) {
            existConnectors.addAll(swmgr.getNodeConnectors(node));
        }

        Set<Node> vmapNodes = new HashSet<Node>(existNodes);
        vmapNodes.add(null);

        // add interface
        final int numInterfaces = 4;
        final int maxInterfaceBridge1 = 2;
        for (short i = 0; i < numInterfaces; i++) {
            int inode = 0;
            for (Node node : existNodes) {
                String ifname = "vinterface" + inode + (i + 10);
                String bname = (i < maxInterfaceBridge1) ? bname1 : bname2;
                VBridgeIfPath ifp
                    = new VBridgeIfPath(tname, bname, ifname);
                st = mgr.addInterface(
                    ifp, new VInterfaceConfig(null, Boolean.TRUE));
                assertEquals("(VBridgeIfPath)" + ifp.toString(),
                             StatusCode.SUCCESS, st.getCode());
                inode++;
            }
        }

        // add a disabled interface
        VBridgeIfPath difp = new VBridgeIfPath(tname, bname1, "vinterface_disabled");
        st = mgr.addInterface(difp, new VInterfaceConfig(null, Boolean.FALSE));
        assertEquals(StatusCode.SUCCESS, st.getCode());

        Node dnode = NodeCreator.createOFNode(Long.valueOf((long)2));
        SwitchPort dport = new SwitchPort(NodeConnectorIDType.OPENFLOW,
                                          String.valueOf(10));
        st = mgr.setPortMap(difp, new PortMapConfig(dnode, dport, (short)0));
        assertEquals(StatusCode.SUCCESS, st.getCode());

        // add a not mapped interface
        VBridgeIfPath nomapifp = new VBridgeIfPath(tname, bname1, "vinterface_nomap");
        st = mgr.addInterface(nomapifp, new VInterfaceConfig(null, Boolean.FALSE));
        assertEquals(StatusCode.SUCCESS, st.getCode());


        // null case
//        PacketResult result = mgr.receiveDataPacket(null);
//        assertEquals(PacketResult.IGNORED, result);

        for (short vlan : vlans) {
            // add port mapping
            Set<PortVlan> portMappedThis = new HashSet<PortVlan>();
            Set<PortVlan> portMappedOther = new HashSet<PortVlan>();

            for (short i = 0; i < numInterfaces; i++) {
                int inode = 0;
                for (Node node : existNodes) {
                    short setvlan = (i != 1) ? vlan : 100;
                    String ifname = "vinterface" + inode + (i + 10);
                    String bname = (i < maxInterfaceBridge1) ? bname1 : bname2;
                    VBridgePath bpath = (i < maxInterfaceBridge1) ? bpath1 : bpath2;
                    VBridgeIfPath ifp = new VBridgeIfPath(tname, bname, ifname);
                    SwitchPort port = new SwitchPort(NodeConnectorIDType.OPENFLOW,
                                                     String.valueOf(i + 10));
                    PortMapConfig pmconf = new PortMapConfig(node, port, setvlan);
                    st = mgr.setPortMap(ifp, pmconf);
                    assertEquals("(VBridgeIfPath)" + ifp.toString()
                            + ",(PortMapConfig)" + pmconf.toString(),
                            StatusCode.SUCCESS, st.getCode());

                    NodeConnector mapnc = NodeConnectorCreator.createOFNodeConnector(
                            Short.valueOf((short)(i + 10)), node);
                    Set<NodeConnector> set  = mappedConnectors.get(bpath);
                    set.add(mapnc);
                    mappedConnectors.put(bpath, set);
                    if (i < maxInterfaceBridge1) {
                        portMappedThis.add(new PortVlan(mapnc, setvlan));
                    } else {
                        portMappedOther.add(new PortVlan(mapnc, setvlan));
                    }

                    inode++;
                }
            }

            for (Node vlanMapNode : vmapNodes) {
                for (short vmapVlan : vlans) {
                    Set<PortVlan> mappedThis = new HashSet<PortVlan>();
                    VlanMapConfig vlconf = new VlanMapConfig(vlanMapNode, vmapVlan);
                    VlanMap map = null;
                    try {
                        map = mgr.addVlanMap(bpath1, vlconf);
                    } catch (VTNException e) {
                        unexpected(e);
                    }

                    for (Node node: existNodes) {
                        if (vlanMapNode == null || node.equals(vlanMapNode)) {
                            for (NodeConnector nc : swmgr.getNodeConnectors(node)) {
                                if (!topomgr.isInternal(nc)) {
                                    mappedThis.add(new PortVlan(nc, vmapVlan));
                                }
                            }
                        }
                    }

                    String emsg = "(vlan(pmap))" + vlan
                            + ",(VlanMapConfig)" + vlconf.toString();

                    mappedThis.addAll(portMappedThis);
                    mappedThis.removeAll(portMappedOther);

                    testReceiveDataPacketBCLoop(mgr, bpath1, MapType.PORT, mappedThis, stub);

                    st = mgr.flushMacEntries(bpath1);
                    assertEquals(emsg, StatusCode.SUCCESS, st.getCode());

//                  testReceiveDataPacketUCLoopLearned(mgr, bpath1, MapType.PORT, mappedThis, stub);

//                  st = mgr.flushMacEntries(bpath1);
//               assertTrue("vlan=" + vlan, st.isSuccess());

                    testReceiveDataPacketUCLoop(mgr, bpath1, MapType.PORT, mappedThis, stub);

                    st = mgr.flushMacEntries(bpath1);
                    assertEquals(emsg, StatusCode.SUCCESS, st.getCode());

                    testReceiveDataPacketUCLoopLearned(mgr, bpath1, MapType.PORT,
                                                       mappedThis, stub);

                    st = mgr.flushMacEntries(bpath1);
                    assertEquals(emsg, StatusCode.SUCCESS, st.getCode());

                    st = mgr.removeVlanMap(bpath1, map.getId());
                    assertEquals(emsg, StatusCode.SUCCESS, st.getCode());
                }
            }

        }

        st = mgr.removeBridge(bpath2);
        assertEquals(StatusCode.SUCCESS, st.getCode());

        // in case received packet from no mapped port.
        for (NodeConnector nc : mappedConnectors.get(bpath2)) {
            byte iphost = 1;
            for (EthernetAddress ea : createEthernetAddresses(false)) {
                byte [] bytes = ea.getValue();
                byte [] src = new byte[] {bytes[0], bytes[1], bytes[2],
                                          bytes[3], bytes[4], bytes[5]};
                byte [] dst = new byte[] {(byte)0xff, (byte)0xff, (byte)0xff,
                                          (byte)0xff, (byte)0xff, (byte)0xff};
                byte [] sender = new byte[] {(byte)192, (byte)168, (byte)0, (byte)iphost};
                byte [] target = new byte[] {(byte)192, (byte)168, (byte)0, (byte)250};
                short vlan = -1;
                String emsg = "(NodeConnector)" + nc.toString()
                        + ",(EthernetAddress)" + ea.toString();

                RawPacket inPkt = createARPRawPacket(src, dst, sender, target,
                        (vlan > 0) ? vlan : -1, nc, ARP.REQUEST);

                PacketResult result = mgr.receiveDataPacket(inPkt);
                assertEquals(emsg, PacketResult.IGNORED, result);

                List<RawPacket> dataList = stub.getTransmittedDataPacket();
                assertEquals(emsg, 0, dataList.size());
                iphost++;
            }
        }

        st = mgr.removeTenant(tpath);
        assertEquals(StatusCode.SUCCESS, st.getCode());
    }

    /**
     * Test method for
     * {@link VTNManagerImpl#find(InetAddress)},
     * {@link VTNManagerImpl#findHost(InetAddress, Set)},
     * {@link VTNManagerImpl#probe(HostNodeConnector)},
     * {@link VTNManagerImpl#probeHost(HostNodeConnector)}.
     */
    @Test
    public void testFindProbe() {
        VTNManagerImpl mgr = vtnMgr;
        TestStub stub = stubObj;
        ISwitchManager swmgr = mgr.getSwitchManager();
        short[] vlans = new short[] {0, 10, 4095};
        byte[] mac = new byte [] {0x00, 0x00, 0x00,
                                  0x11, 0x22, 0x33};
        byte[] cntMac = swmgr.getControllerMAC();
        ConcurrentMap<VBridgePath, Set<NodeConnector>> mappedConnectors
            = new ConcurrentHashMap<VBridgePath, Set<NodeConnector>>();
        Status st = null;

        String tname = "vtn";
        VTenantPath tpath = new VTenantPath(tname);
        String bname1 = "vbridge1";
        VBridgePath bpath1 = new VBridgePath(tname, bname1);
        String bname2 = "vbridge2";
        VBridgePath bpath2 = new VBridgePath(tname, bname2);
        List<VBridgePath> bpathlist = new ArrayList<VBridgePath>();
        bpathlist.add(bpath1);
        bpathlist.add(bpath2);
        createTenantAndBridge(mgr, tpath, bpathlist);

        mappedConnectors.put(bpath1, new HashSet<NodeConnector>());
        mappedConnectors.put(bpath2, new HashSet<NodeConnector>());

        Set<Node> existNodes = swmgr.getNodes();
        Set<NodeConnector> existConnectors = new HashSet<NodeConnector>();
        for (Node node: existNodes) {
            existConnectors.addAll(swmgr.getNodeConnectors(node));
        }

        // add interface
        final int numInterfaces = 4;
        final int maxInterfaceBridge1 = 2;
        for (short i = 0; i < numInterfaces; i++) {
            int inode = 0;
            for (Node node : existNodes) {
                String ifname = "vinterface" + inode + (i + 10);
                VBridgeIfPath ifp = new VBridgeIfPath(tname,
                        (i < maxInterfaceBridge1) ? bname1 : bname2, ifname);
                st = mgr.addInterface(
                    ifp, new VInterfaceConfig(null, Boolean.TRUE));
                assertEquals("(VBridgeIfPath)" + ifp.toString(),
                             StatusCode.SUCCESS, st.getCode());
                inode++;
            }
        }

        InetAddress ia = getInetAddressFromAddress(new byte[] {10, 0, 0, 1});

        InetAddress ia6 = null;
        try {
            byte[] addr = {
                (byte)0x20, (byte)0x01, (byte)0x04, (byte)0x20,
                (byte)0x02, (byte)0x81, (byte)0x10, (byte)0x04,
                (byte)0xe1, (byte)0x23, (byte)0xe6, (byte)0x88,
                (byte)0xd6, (byte)0x55, (byte)0xa1, (byte)0xb0,
            };
            ia6 = InetAddress.getByAddress(addr);
        } catch (UnknownHostException e) {
            unexpected(e);
        }

        for (short vlan: vlans) {
            // add port mapping
            for (short i = 0; i < numInterfaces; i++) {
                int inode = 0;
                for (Node node : existNodes) {
                    String bname = (i < maxInterfaceBridge1) ? bname1 : bname2;
                    String ifname = "vinterface" + inode + (i + 10);
                    VBridgePath bpath = (i < maxInterfaceBridge1) ? bpath1 : bpath2;
                    VBridgeIfPath ifp = new VBridgeIfPath(tname, bname, ifname);
                    SwitchPort port = new SwitchPort(NodeConnectorIDType.OPENFLOW,
                                                     String.valueOf(i + 10));
                    PortMapConfig pmconf = new PortMapConfig(node, port, vlan);
                    st = mgr.setPortMap(ifp, pmconf);
                    assertEquals("(VBridgeIfPath)" + ifp.toString()
                            + ",(PortMapConfig)" + pmconf.getVlan(),
                            StatusCode.SUCCESS, st.getCode());

                    NodeConnector mapnc = NodeConnectorCreator.createOFNodeConnector(
                            Short.valueOf((short)(i + 10)), node);
                    Set<NodeConnector> set  = mappedConnectors.get(bpath);
                    set.add(mapnc);
                    mappedConnectors.put(bpath, set);
                    inode++;
                }
            }

            Set<Set<VBridgePath>> bpathset = new HashSet<Set<VBridgePath>>();
            bpathset.add(null);
            Set<VBridgePath> bpaths1 = new HashSet<VBridgePath>();
            bpaths1.add(bpath1);
            bpathset.add(bpaths1);
            Set<VBridgePath> bpaths2 = new HashSet<VBridgePath>(bpaths1);
            bpaths2.add(bpath2);
            bpathset.add(bpaths2);

            mgr.findHost(null, bpaths1);
            List<RawPacket> dataList = stub.getTransmittedDataPacket();
            assertEquals(0, dataList.size());

            mgr.findHost(ia6, bpaths1);
            dataList = stub.getTransmittedDataPacket();
            assertEquals(0, dataList.size());

            for (Set<VBridgePath> bpaths : bpathset) {
                int numMapped = 0;
                Set<PortVlan> mappedThis = new HashSet<PortVlan>();
                if (bpaths == null || bpaths.contains(bpath1)) {
                    for (NodeConnector nc : mappedConnectors.get(bpath1)) {
                        mappedThis.add(new PortVlan(nc, vlan));
                        numMapped++;
                    }
                }
                if (bpaths == null || bpaths.contains(bpath2)) {
                    for (NodeConnector nc : mappedConnectors.get(bpath2)) {
                        mappedThis.add(new PortVlan(nc, vlan));
                        numMapped++;
                    }
                }

                // test find()
                if (bpaths == null) {
                    mgr.find(ia);
                } else {
                    mgr.findHost(ia, bpaths);
                }

                dataList = stub.getTransmittedDataPacket();
                assertEquals(numMapped, dataList.size());

                for (RawPacket raw : dataList) {
                    String emsg = "(vlan)" + vlan
                            + ",(Outgoing nc)" + raw.getOutgoingNodeConnector().toString();
                    Ethernet pkt = (Ethernet)stub.decodeDataPacket(raw);
                    short outVlan;
                    if (pkt.getEtherType() == EtherTypes.VLANTAGGED.shortValue()) {
                        IEEE8021Q vlantag = (IEEE8021Q)pkt.getPayload();
                        assertTrue(emsg + ",(Vid)" + vlantag.getVid(),
                                mappedThis.contains(new PortVlan(raw.getOutgoingNodeConnector(), vlantag.getVid())));
                        outVlan = vlantag.getVid();
                    } else {
                        assertTrue(emsg,
                                mappedThis.contains(new PortVlan(raw.getOutgoingNodeConnector(), (short)0)));
                        outVlan = (short)0;
                    }

                    PortVlan outPv = new PortVlan(raw.getOutgoingNodeConnector(), outVlan);
                    assertTrue(emsg, mappedThis.contains(outPv));
//                    assertFalse(emsg,
//                        noMappedThis.contains(raw.getOutgoingNodeConnector()));

                    Ethernet eth = (Ethernet)pkt;

                    checkOutEthernetPacket(emsg, eth, EtherTypes.ARP,
                            swmgr.getControllerMAC(), new byte[] {-1, -1, -1, -1, -1, -1},
                            vlan, EtherTypes.IPv4, ARP.REQUEST,
                            swmgr.getControllerMAC(), new byte[] {0, 0, 0, 0, 0, 0},
                            new byte[] {0, 0, 0, 0}, ia.getAddress());
                }

                // probe()
                for (PortVlan pv : mappedThis) {
                    String emsg = "(vlan)" + vlan + ",(PortVlan)" + pv.toString();

                    HostNodeConnector hnode = null;
                    try {
                        hnode = new HostNodeConnector(mac, ia, pv.getNodeConnector(),
                                                      pv.getVlan());
                    } catch (ConstructionException e) {
                        unexpected(e);
                    }

                    mgr.probe(hnode);

                    dataList = stub.getTransmittedDataPacket();
                    assertEquals(emsg, 1, dataList.size());

                    RawPacket raw = dataList.get(0);
                    assertEquals(emsg, pv,
                            new PortVlan(raw.getOutgoingNodeConnector(), vlan));

                    Ethernet eth = (Ethernet)stub.decodeDataPacket(raw);
                    checkOutEthernetPacket(emsg, eth, EtherTypes.ARP,
                            swmgr.getControllerMAC(), mac,
                            vlan, EtherTypes.IPv4, ARP.REQUEST,
                            swmgr.getControllerMAC(), mac,
                            new byte[] {0, 0, 0, 0}, ia.getAddress());
                }
            }
        }

        // not OPENFLOW Node
        Node n = null;
        try {
            n = new Node(Node.NodeIDType.ONEPK, "Node ID: 0");
        } catch (ConstructionException e) {
            unexpected(e);
        }
        NodeConnector nc = NodeConnectorCreator.createNodeConnector(NodeConnectorIDType.ONEPK,
                    "Port: 0", n);
        HostNodeConnector hnode = null;
        try {
            hnode = new HostNodeConnector(mac, ia, nc, (short)0);
        } catch (ConstructionException e) {
            unexpected(e);
        }
        boolean result = mgr.probeHost(hnode);
        assertFalse(result);
        List<RawPacket> dataList = stub.getTransmittedDataPacket();
        assertEquals(0, dataList.size());

        // internal port
        n = NodeCreator.createOFNode(0L);
        NodeConnector innc = NodeConnectorCreator.createOFNodeConnector(
                Short.valueOf((short)15), n);
        try {
            hnode = new HostNodeConnector(mac, ia, innc, (short)0);
        } catch (ConstructionException e) {
            unexpected(e);
        }
        result = mgr.probeHost(hnode);
        assertFalse(result);
        dataList = stub.getTransmittedDataPacket();
        assertEquals(0, dataList.size());

        // IPv6 host
        innc = NodeConnectorCreator.createOFNodeConnector(
                Short.valueOf((short)10), n);
        try {
            hnode = new HostNodeConnector(mac, ia6, innc, (short)0);
        } catch (ConstructionException e) {
            unexpected(e);
        }
        result = mgr.probeHost(hnode);
        assertFalse(result);
        dataList = stub.getTransmittedDataPacket();
        assertEquals(0, dataList.size());

        // nodeconnector is not mapped
        mgr.removeBridge(bpath2);
        nc = NodeConnectorCreator.createOFNodeConnector(
                Short.valueOf((short)14), n);
        try {
            hnode = new HostNodeConnector(mac, ia, nc, (short)0);
        } catch (ConstructionException e) {
            unexpected(e);
        }
        result = mgr.probeHost(hnode);
        assertFalse(result);
        dataList = stub.getTransmittedDataPacket();
        assertEquals(0, dataList.size());

        // in case if is disabled.
        VBridgeIfPath ifp = new VBridgeIfPath(tname, bname1, "vinterface010");
        mgr.modifyInterface(
            ifp, new VInterfaceConfig(null, Boolean.FALSE), true);

        PortMap map = null;
        try {
            map = mgr.getPortMap(ifp);
        } catch (VTNException e) {
            unexpected(e);
        }

        try {
            hnode = new HostNodeConnector(mac, ia, map.getNodeConnector(),
                    map.getConfig().getVlan());
        } catch (ConstructionException e) {
            unexpected(e);
        }

        result = mgr.probeHost(hnode);
        assertFalse(result);
        dataList = stub.getTransmittedDataPacket();
        assertEquals(0, dataList.size());


        // if null case
        mgr.probe(null);

        st = mgr.removeTenant(tpath);
        assertEquals(StatusCode.SUCCESS, st.getCode());

        mgr.stopping();
        result = mgr.probeHost(hnode);
        assertFalse(result);
    }

    /**
     * Test case for MAC mapping.
     *
     * <ul>
     *   <li>
     *     Ensure that incoming packet can be handled by the MAC mapping.
     *   </li>
     *   <li>
     *     Ensure that MAC address table is maintained correctly.
     *   </li>
     *   <li>
     *     Ensure that VTN flow database is maintained correctly.
     *   </li>
     * </ul>
     */
    @Test
    public void testMacMap() {
        // Set up test environment.
        VTNManagerImpl mgr = vtnMgr;
        String container = mgr.getContainerName();
        ISwitchManager swMgr = mgr.getSwitchManager();
        ITopologyManager topoMgr = mgr.getTopologyManager();
        List<NodeConnector> edgePorts = new ArrayList<NodeConnector>();
        List<TestHost> allHosts = new ArrayList<TestHost>();
        Map<NodeConnector, List<TestHost>> portHosts =
            new HashMap<NodeConnector, List<TestHost>>();
        long macAddr1 = 1L;
        long macAddr2 = 0xf00000000000L;
        byte[] ipAddr = {(byte)192, (byte)168, (byte)100, (byte)1};
        List<Node> nodeList = new ArrayList<Node>();

        for (Node node: swMgr.getNodes()) {
            nodeList.add(node);
            int portCount = 0;
            for (NodeConnector nc: swMgr.getUpNodeConnectors(node)) {
                if (swMgr.isSpecial(nc) || topoMgr.isInternal(nc)) {
                    continue;
                }
                List<TestHost> hostList = new ArrayList<TestHost>();
                portHosts.put(nc, hostList);
                edgePorts.add(nc);

                // Add 2 MAC addresses, and share it on VLAN 0 and 1, and
                // 1 MAC address on VLAN 2 and 3.
                for (short vlan = 0; vlan <= 3; vlan++) {
                    TestHost host = new TestHost(macAddr1, vlan, ipAddr, nc);
                    hostList.add(host);
                    allHosts.add(host);
                    ipAddr[3]++;
                    macAddr1++;

                    if (vlan < 2) {
                        host = new TestHost(macAddr2, vlan, ipAddr, nc);
                        hostList.add(host);
                        allHosts.add(host);
                        ipAddr[3]++;
                        macAddr2++;
                    }
                }

                portCount++;
                if (portCount >= 3) {
                    break;
                }
            }
        }

        // Use one more switch port on node0.
        Node node0 = nodeList.get(0);
        NodeConnector additional0 = null;
        for (NodeConnector nc: swMgr.getUpNodeConnectors(node0)) {
            if (swMgr.isSpecial(nc) || topoMgr.isInternal(nc) ||
                portHosts.containsKey(nc)) {
                continue;
            }
            additional0 = nc;
            assertNull(portHosts.put(nc, new ArrayList<TestHost>()));
            break;
        }
        assertNotNull(additional0);

        // Use one more switch port on node1.
        Node node1 = nodeList.get(1);
        NodeConnector additional1 = null;
        for (NodeConnector nc: swMgr.getUpNodeConnectors(node1)) {
            if (swMgr.isSpecial(nc) || topoMgr.isInternal(nc) ||
                portHosts.containsKey(nc)) {
                continue;
            }

            additional1 = nc;
            List<TestHost> hostList = new ArrayList<TestHost>();
            portHosts.put(nc, hostList);
            edgePorts.add(nc);

            // Add 2 more MAC addresses on each VLAN 1..3,
            for (int i = 0; i < 2; i++) {
                for (short vlan = 1; vlan <= 3; vlan++) {
                    TestHost host = new TestHost(macAddr2, vlan, ipAddr, nc);
                    hostList.add(host);
                    allHosts.add(host);
                    ipAddr[3]++;
                    macAddr2++;
                }
            }
            break;
        }
        assertNotNull(additional1);

        // Create 1 tenant, and 5 vBridges.
        String tname = "tenant";
        VTenantPath tpath = new VTenantPath(tname);
        List<VBridgePath> bridges = new ArrayList<VBridgePath>();
        for (int i = 0; i < 5; i++) {
            VBridgePath bpath = new VBridgePath(tpath, "bridge" + i);
            bridges.add(bpath);
        }
        createTenantAndBridge(mgr, tpath, bridges);

        // Configure VLAN mapping that maps VLAN 0 to bridges[0].
        VlanMapConfig vlconf = new VlanMapConfig(null, (short)0);
        VBridgePath bpath0 = bridges.get(0);
        VlanMap vmap0 = null;
        try {
            vmap0 = mgr.addVlanMap(bpath0, vlconf);
        } catch (Exception e) {
            unexpected(e);
        }
        VlanMapPath vpath0 = new VlanMapPath(bpath0, vmap0.getId());

        // Configure VLAN mapping that maps VLAN 1 on nodes[1] to bridges[1].
        VBridgePath bpath1 = bridges.get(1);
        vlconf = new VlanMapConfig(node1, (short)1);
        VlanMap vmap1 = null;
        try {
            vmap1 = mgr.addVlanMap(bpath1, vlconf);
        } catch (Exception e) {
            unexpected(e);
        }
        VlanMapPath vpath1 = new VlanMapPath(bpath1, vmap1.getId());

        flushTasks();
        VNodeState nst = VNodeState.UP;
        for (int i = 0; i < bridges.size(); i++) {
            checkVBridgeStatus(mgr, bridges.get(i), 0, nst);
            if (i == 1) {
                nst = VNodeState.UNKNOWN;
            }
        }

        for (TestHost host: allHosts) {
            short vlan = host.getVlan();
            MapReference ref;
            if (vlan == 0) {
                ref = new MapReference(MapType.VLAN, container, vpath0);
            } else if (vlan == 1 && node1.equals(host.getNode())) {
                ref = new MapReference(MapType.VLAN, container, vpath1);
            } else {
                ref = null;
            }
            host.setMapping(ref);
        }

        // Ensure that VLAN mappings are established.
        Map<FlowGroupId, VTNFlow> flowMap =
            new HashMap<FlowGroupId, VTNFlow>();
        int flowCount = 0;
        flowCount = checkMapping(bridges, allHosts, flowMap, flowCount);
        assertTrue(flowCount != 0);

        // Map VLAN 0 at edgePorts[0] and VLAN 1 at edgePorts[5] to bridges[2]
        // using port mapping.
        VBridgePath bpath2 = bridges.get(2);
        Map<VBridgeIfPath, PortMap> interfaces2 =
            new LinkedHashMap<VBridgeIfPath, PortMap>();
        Set<PortVlan> pmap0Nw = new HashSet<PortVlan>();
        int ifIdx = 0;
        for (short vlan = 0; vlan <= 1; vlan++) {
            NodeConnector port = (vlan == 0)
                ? edgePorts.get(0) : edgePorts.get(5);
            pmap0Nw.add(new PortVlan(port, vlan));

            SwitchPort swPort = new SwitchPort(NodeConnectorIDType.OPENFLOW,
                                               port.getNodeConnectorIDString());
            PortMapConfig pmconf = new PortMapConfig(port.getNode(), swPort,
                                                     vlan);
            VBridgeIfPath ifpath = new VBridgeIfPath(bpath2, "if_" + vlan);
            PortMap pmap = new PortMap(pmconf, port);
            interfaces2.put(ifpath, pmap);

            VInterfaceConfig ifconf = new VInterfaceConfig(null, Boolean.TRUE);
            Status st = mgr.addInterface(ifpath, ifconf);
            assertEquals(StatusCode.SUCCESS, st.getCode());
            st = mgr.setPortMap(ifpath, pmconf);
            assertEquals(StatusCode.SUCCESS, st.getCode());

            MapReference pref = new MapReference(MapType.PORT, container,
                                                 ifpath);
            for (TestHost host: portHosts.get(port)) {
                if (host.getVlan() == vlan) {
                    host.setMapping(pref);
                }
            }
        }
        flushTasks();
        flushFlowTasks();
        nst = VNodeState.UP;
        for (int i = 0; i < bridges.size(); i++) {
            checkVBridgeStatus(mgr, bridges.get(i), 0, nst);
            if (i == 2) {
                nst = VNodeState.UNKNOWN;
            }
        }

        // MAC address entries and flow entries superseded by a new
        // port mapping should be purged.
        for (Iterator<VTNFlow> it = flowMap.values().iterator();
             it.hasNext();) {
            VTNFlow vflow = it.next();
            ObjectPair<L2Host, L2Host> edges = vflow.getEdgeHosts();
            L2Host in = edges.getLeft();
            short ivlan = in.getHost().getVlan();
            PortVlan pvlan = new PortVlan(in.getPort(), ivlan);
            if (!pmap0Nw.contains(pvlan)) {
                L2Host out = edges.getRight();
                boolean matched;
                if (out == null) {
                    matched = true;
                } else {
                    short ovlan = out.getHost().getVlan();
                    pvlan = new PortVlan(out.getPort(), ovlan);
                    matched = pmap0Nw.contains(pvlan);
                }
                if (!matched) {
                    checkVTNFlowInstalled(tname, vflow);
                    continue;
                }
            }

            checkVTNFlowUninstalled(tname, vflow);
            flowCount -= vflow.getFlowEntries().size();
            it.remove();
        }

        for (TestHost host: allHosts) {
            MapReference ref = host.getMapping();
            if (ref != null && ref.getMapType() == MapType.PORT) {
                // MAC address entry for this host should be purged.
                host.setMapping(null);
            }
            for (VBridgePath bp: bridges) {
                host.checkLearned(mgr, bp);
            }
            host.setMapping(ref);
        }

        // Ensure that the port mapping is established.
        int fcnt = checkMapping(bridges, allHosts, flowMap, flowCount);
        assertTrue(fcnt > flowCount);
        flowCount = fcnt;

        // Configure MAC mapping into bridges[3].
        VBridgePath bpath3 = bridges.get(3);
        TestHost denied = null;
        Set<DataLinkHost> allow = new HashSet<DataLinkHost>();
        Set<DataLinkHost> deny = new HashSet<DataLinkHost>();
        List<TestHost> allowedHosts = new ArrayList<TestHost>();
        List<TestHost> deniedHosts = new ArrayList<TestHost>();
        MacMapPath mpath = new MacMapPath(bpath3);
        MapReference mref = new MapReference(MapType.MAC, container, mpath);
        Set<L2Host> mmapEdges = new HashSet<L2Host>();
        Set<L2Host> deniedEdges = new HashSet<L2Host>();

        for (TestHost host: allHosts) {
            short vlan = host.getVlan();
            if (vlan == 1) {
                MapReference ref = host.getMapping();
                if (additional1.equals(host.getPort())) {
                    // Reject hosts on `additional1' port.
                    assertTrue(deny.add(host.getDataLinkHost()));
                    deniedHosts.add(host);
                    assertTrue(deniedEdges.add(host.getL2Host()));
                    continue;
                }
                if (denied == null) {
                    if (ref != null && vpath1.equals(ref.getPath())) {
                        // Reject this host.
                        assertTrue(deny.add(host.getDataLinkHost()));
                        deniedHosts.add(host);
                        assertTrue(deniedEdges.add(host.getL2Host()));
                        denied = host;
                        continue;
                    }
                }

                // Port mapping always precedes MAC mapping.
                if (ref == null || ref.getMapType() != MapType.PORT) {
                    host.setMapping(mref);
                    assertTrue(mmapEdges.add(host.getL2Host()));
                    allowedHosts.add(host);
                }
            } else if (vlan == (short)2 && allow.size() < 5) {
                // Map 5 hosts on VLAN 2.
                assertTrue(allow.add(host.getDataLinkHost()));
                host.setMapping(mref);
                assertTrue(mmapEdges.add(host.getL2Host()));
                allowedHosts.add(host);
            }
        }
        assertEquals(3, deny.size());

        // Map all hosts on VLAN 1 except for hosts in `deny'.
        DataLinkHost mmapVlan1 = new EthernetHost((short)1);
        assertTrue(allow.add(mmapVlan1));

        MacMapConfig mcconf = new MacMapConfig(allow, deny);
        try {
            assertEquals(null, mgr.getMacMap(bpath3));
            assertEquals(UpdateType.ADDED,
                         mgr.setMacMap(bpath3, UpdateOperation.SET, mcconf));
            MacMap mcmap = mgr.getMacMap(bpath3);
            assertTrue(mcconf.equals(mcmap));
            assertTrue(mcmap.getMappedHosts().isEmpty());
        } catch (Exception e) {
            unexpected(e);
        }

        // State of bridges[3] should be DOWN because no host is activated
        // by MAC mapping.
        nst = VNodeState.UP;
        for (int i = 0; i < bridges.size(); i++) {
            checkVBridgeStatus(mgr, bridges.get(i), 0, nst);
            if (i == 2) {
                nst = VNodeState.DOWN;
            } else if (i == 3) {
                nst = VNodeState.UNKNOWN;
            }
        }

        // MAC address entries and flow entries superseded by a new MAC mapping
        // should be purged.
        Map<PortVlan, List<FlowGroupId>> vmapFlows =
            new HashMap<PortVlan, List<FlowGroupId>>();
        for (Iterator<VTNFlow> it = flowMap.values().iterator();
             it.hasNext();) {
            VTNFlow vflow = it.next();
            ObjectPair<L2Host, L2Host> edges = vflow.getEdgeHosts();
            L2Host in = edges.getLeft();
            L2Host out = edges.getRight();
            if (mmapEdges.contains(in) || mmapEdges.contains(out)) {
                checkVTNFlowUninstalled(tname, vflow);
                flowCount -= vflow.getFlowEntries().size();
                it.remove();
            } else {
                checkVTNFlowInstalled(tname, vflow);
                if (vflow.dependsOn(vpath1) &&
                    (deniedEdges.contains(in) || deniedEdges.contains(out))) {
                    // This flow will be removed when a host in incoming or
                    // outgoing network is mapped by MAC mapping.
                    FlowGroupId gid = vflow.getGroupId();
                    PortVlan[] nw = {
                        new PortVlan(in.getPort(), in.getHost().getVlan()),
                        new PortVlan(out.getPort(), out.getHost().getVlan()),
                    };
                    for (PortVlan pvlan: nw) {
                        List<FlowGroupId> groups = vmapFlows.get(pvlan);
                        if (groups == null) {
                            groups = new ArrayList<FlowGroupId>();
                            vmapFlows.put(pvlan, groups);
                        }
                        groups.add(gid);
                    }
                }
            }
        }
        stubObj.checkFlowCount(flowCount);

        for (TestHost host: allHosts) {
            MapReference ref = host.getMapping();
            if (ref != null && ref.getMapType() == MapType.MAC) {
                // MAC address entry for this host should be purged.
                host.setMapping(null);
            }
            for (VBridgePath bp: bridges) {
                host.checkLearned(mgr, bp);
            }
            host.setMapping(ref);
        }

        VTenant vtn = null;
        List<VBridge> vbridges = new ArrayList<VBridge>();
        List<VInterface> vif2 = new ArrayList<VInterface>();
        try {
            vtn = mgr.getTenant(tpath);
            for (VBridgePath bpath: bridges) {
                vbridges.add(mgr.getBridge(bpath));
            }
            for (VBridgeIfPath ipath: interfaces2.keySet()) {
                vif2.add(mgr.getInterface(ipath));
            }
        } catch (Exception e) {
            unexpected(e);
        }

        // Add an IVTNManagerAware listener. This should report current
        // configuration, including MAC mapping.
        OrderedVTNListener listener = new OrderedVTNListener();
        flushTasks();
        mgr.addVTNManagerAware(listener);
        listener.checkEvent(VTNListenerType.VTN, tpath, vtn, UpdateType.ADDED);
        for (int i = 0; i < bridges.size(); i++) {
            VBridgePath bpath = bridges.get(i);
            VBridge vbr = vbridges.get(i);
            listener.checkEvent(VTNListenerType.VBRIDGE, bpath, vbr,
                                UpdateType.ADDED);
            if (i == 0) {
                listener.checkEvent(VTNListenerType.VLANMAP, bpath, vmap0,
                                    UpdateType.ADDED);
            } else if (i == 1) {
                listener.checkEvent(VTNListenerType.VLANMAP, bpath, vmap1,
                                    UpdateType.ADDED);
            } else if (i == 2) {
                int idx = 0;
                for (Map.Entry<VBridgeIfPath, PortMap> entry:
                         interfaces2.entrySet()) {
                    VBridgeIfPath ipath = entry.getKey();
                    VInterface vif = vif2.get(idx);
                    idx++;
                    listener.checkEvent(VTNListenerType.VBRIDGE_IF, ipath,
                                        vif, UpdateType.ADDED);
                    PortMap pmap = entry.getValue();
                    if (pmap != null) {
                        listener.checkEvent(VTNListenerType.PORTMAP, ipath,
                                            pmap, UpdateType.ADDED);
                    }
                }
            } else if (i == 3) {
                listener.checkEvent(VTNListenerType.MACMAP, bpath, mcconf,
                                    UpdateType.ADDED);
            }
        }
        listener.checkEmtpy();

        // Activate MAC mapping by sending packet from mapped hosts.
        IVTNResourceManager resMgr = mgr.getResourceManager();
        assertNull(resMgr.getMacMappedNetworks(mgr, mpath));
        CounterSet<PortVlan> mmapNw = null;
        Map<MacVlan, NodeConnector> mmapHosts = null;
        Set<MacAddressEntry> macEntries = new HashSet<MacAddressEntry>();
        boolean active = false;
        for (TestHost host: allowedHosts) {
            assertEquals(mmapNw, resMgr.getMacMappedNetworks(mgr, mpath));
            assertEquals(mmapHosts, resMgr.getMacMappedHosts(mgr, mpath));
            host.setMapping(mref);
            List<FlowGroupId> flowList = vmapFlows.get(host.getPortVlan());
            List<VTNFlow> removedFlows = new ArrayList<VTNFlow>();
            if (flowList != null) {
                // VTN flows in this list should be purged when the MAC mapping
                // for this host is activated.
                for (FlowGroupId gid: flowList) {
                    VTNFlow vflow = flowMap.remove(gid);
                    if (vflow != null) {
                        removedFlows.add(vflow);
                        flowCount -= vflow.getFlowEntries().size();
                    }
                }
            }
            checkMapping(bridges, host, flowCount);
            for (VTNFlow vflow: removedFlows) {
                checkVTNFlowUninstalled(tname, vflow);
            }

            if (!active) {
                // VBridge state should be changed to UP.
                VBridge vbr = null;
                try {
                    vbr = mgr.getBridge(bpath3);
                } catch (Exception e) {
                    unexpected(e);
                }
                assertEquals(VNodeState.UP, vbr.getState());
                listener.checkEvent(VTNListenerType.VBRIDGE, bpath3, vbr,
                                    UpdateType.CHANGED);
                active = true;
            }

            PortVlan pvlan = host.getPortVlan();
            if (mmapNw == null) {
                mmapNw = new CounterSet<PortVlan>();
            }
            mmapNw.add(pvlan);
            assertEquals(mmapNw, resMgr.getMacMappedNetworks(mgr, mpath));
            assertTrue(macEntries.add(host.getMacAddressEntry()));
            if (mmapHosts == null) {
                mmapHosts = new HashMap<MacVlan, NodeConnector>();
            }
            assertNull(mmapHosts.put(host.getMacVlan(), host.getPort()));

            try {
                MacMap mcmap = mgr.getMacMap(bpath3);
                assertTrue(mcconf.equals(mcmap));
                List<MacAddressEntry> mclist = mcmap.getMappedHosts();
                assertEquals(macEntries.size(), mclist.size());
                assertTrue(macEntries.containsAll(mclist));
            } catch (Exception e) {
                unexpected(e);
            }
        }
        assertTrue(active);
        assertEquals(mmapHosts, resMgr.getMacMappedHosts(mgr, mpath));
        listener.checkEmtpy();

        // Determine mapping for hosts in deniedHosts.
        boolean portBusy = false;
        for (TestHost host: deniedHosts) {
            if (mmapNw.contains(host.getPortVlan())) {
                // This host should not be mapped by VLAN mapping because
                // VLAN network is reserved by MAC mapping.
                host.setMapping(null);
                portBusy = true;
            }
        }
        assertTrue(portBusy);
        fcnt = checkMapping(bridges, allHosts, flowMap, flowCount);
        assertTrue(fcnt > flowCount);
        flowCount = fcnt;
        listener.checkEmtpy();

        // Move 2 hosts on VLAN 2 in allowedHosts to `additional0'.
        List<TestHost> movedHosts = new ArrayList<TestHost>();
        List<MacAddressEntry> oldMacEntries = new ArrayList<MacAddressEntry>();
        List<PortVlan> oldNw = new ArrayList<PortVlan>();
        for (Iterator<TestHost> it = allowedHosts.iterator(); it.hasNext();) {
            TestHost host = it.next();
            if (host.getVlan() != 2) {
                continue;
            }

            it.remove();
            assertTrue(allHosts.remove(host));
            NodeConnector port = host.getPort();
            List<TestHost> hostList = portHosts.get(port);
            assertTrue(hostList.remove(host));
            assertTrue(oldMacEntries.add(host.getMacAddressEntry()));
            oldNw.add(host.getPortVlan());

            TestHost newHost = host.moveTo(additional0);
            assertTrue(movedHosts.add(newHost));
            assertTrue(allHosts.add(newHost));
            hostList = portHosts.get(additional0);
            assertTrue(hostList.add(newHost));
            if (movedHosts.size() >= 2) {
                break;
            }
        }
        assertTrue(allowedHosts.addAll(movedHosts));

        // Activate MAC mapping for moved hosts.
        for (int i = 0; i < movedHosts.size(); i++) {
            TestHost host = movedHosts.get(i);
            PortVlan pvlan = host.getPortVlan();
            mmapNw.add(pvlan);

            // Determine VTN flows to be purged.
            MacVlan mvlan = host.getMacVlan();
            List<VTNFlow> removedFlows = new ArrayList<VTNFlow>();
            for (Iterator<VTNFlow> it = flowMap.values().iterator();
                 it.hasNext();) {
                VTNFlow vflow = it.next();
                if (vflow.dependsOn(mpath) && vflow.dependsOn(mvlan)) {
                    it.remove();
                    removedFlows.add(vflow);
                    flowCount -= vflow.getFlowEntries().size();
                }
            }

            // Determine VLAN network to be released.
            PortVlan oldpv = oldNw.get(i);
            if (!mmapNw.remove(oldpv)) {
                oldpv = null;
            }
            checkMapping(bridges, host, flowCount, oldpv);
            for (VTNFlow vflow: removedFlows) {
                checkVTNFlowUninstalled(tname, vflow);
            }

            assertEquals(mmapNw, resMgr.getMacMappedNetworks(mgr, mpath));
            assertTrue(macEntries.add(host.getMacAddressEntry()));
            assertTrue(macEntries.remove(oldMacEntries.get(i)));
            mmapHosts.put(host.getMacVlan(), host.getPort());
            assertEquals(mmapHosts, resMgr.getMacMappedHosts(mgr, mpath));

            try {
                MacMap mcmap = mgr.getMacMap(bpath3);
                assertTrue(mcconf.equals(mcmap));
                List<MacAddressEntry> mclist = mcmap.getMappedHosts();
                assertEquals(macEntries.size(), mclist.size());
                assertTrue(macEntries.containsAll(mclist));
            } catch (Exception e) {
                unexpected(e);
            }
        }
        listener.checkEmtpy();

        // Down `additional0' port.
        Map<String, Property> propMap =
            swMgr.getNodeConnectorProps(additional0);
        propMap.put(State.StatePropName, new State(State.EDGE_DOWN));
        mgr.notifyNodeConnector(additional0, UpdateType.CHANGED, propMap);
        flushTasks();
        flushFlowTasks();
        listener.checkEmtpy();

        mmapNw.removeCounter(new PortVlan(additional0, (short)2));
        assertEquals(mmapNw, resMgr.getMacMappedNetworks(mgr, mpath));

        // MAC address entry detected on `additional0' should be purged.
        for (TestHost host: movedHosts) {
            MapReference ref = host.getMapping();
            host.setMapping(null);
            for (VBridgePath bp: bridges) {
                host.checkLearned(mgr, bp);
            }
            host.setMapping(ref);
            assertEquals(host.getPort(), mmapHosts.remove(host.getMacVlan()));

            MacAddressEntry ment = host.getMacAddressEntry();
            assertTrue(macEntries.remove(ment));
            try {
                EthernetAddress eaddr = host.getEthernetAddress();
                assertEquals(null, mgr.getMacMappedHost(bpath3, eaddr));
            } catch (Exception e) {
                unexpected(e);
            }
        }
        assertEquals(mmapHosts, resMgr.getMacMappedHosts(mgr, mpath));

        try {
            List<MacAddressEntry> mclist = mgr.getMacMappedHosts(bpath3);
            assertEquals(macEntries.size(), mclist.size());
            assertTrue(macEntries.containsAll(mclist));
        } catch (Exception e) {
            unexpected(e);
        }

        // VTN flows on `additional0' should be purged.
        for (Iterator<VTNFlow> it = flowMap.values().iterator();
             it.hasNext();) {
            VTNFlow vflow = it.next();
            ObjectPair<L2Host, L2Host> edges = vflow.getEdgeHosts();
            L2Host in = edges.getLeft();
            L2Host out = edges.getRight();
            if (additional0.equals(in.getPort()) ||
                additional0.equals(out.getPort())) {
                it.remove();
                checkVTNFlowUninstalled(tname, vflow);
                flowCount -= vflow.getFlowEntries().size();
            }
        }

        // Up `additional0' port.
        propMap.put(State.StatePropName, new State(State.EDGE_UP));
        mgr.notifyNodeConnector(additional0, UpdateType.CHANGED, propMap);
        flushTasks();
        flushFlowTasks();
        listener.checkEmtpy();

        // Activate MAC mappings on `additional0' again.
        for (int i = 0; i < movedHosts.size(); i++) {
            TestHost host = movedHosts.get(i);
            PortVlan pvlan = host.getPortVlan();
            checkMapping(bridges, host, flowCount);

            mmapNw.add(pvlan);
            mmapHosts.put(host.getMacVlan(), host.getPort());
            assertEquals(mmapHosts, resMgr.getMacMappedHosts(mgr, mpath));
            MacAddressEntry ment = host.getMacAddressEntry();
            assertTrue(macEntries.add(ment));
            try {
                EthernetAddress eaddr = host.getEthernetAddress();
                assertEquals(ment, mgr.getMacMappedHost(bpath3, eaddr));
            } catch (Exception e) {
                unexpected(e);
            }
        }
        assertEquals(mmapNw, resMgr.getMacMappedNetworks(mgr, mpath));
        listener.checkEmtpy();

        for (TestHost host: movedHosts) {
            mmapHosts.put(host.getMacVlan(), host.getPort());
        }
        assertEquals(mmapHosts, resMgr.getMacMappedHosts(mgr, mpath));

        try {
            List<MacAddressEntry> mclist = mgr.getMacMappedHosts(bpath3);
            assertEquals(macEntries.size(), mclist.size());
            assertTrue(macEntries.containsAll(mclist));
        } catch (Exception e) {
            unexpected(e);
        }

        // Add one host in `allow' to `deny'.
        TestHost tmpDenied = null;
        Set<DataLinkHost> tmpDeny = new HashSet<DataLinkHost>(deny);
        for (TestHost host: allowedHosts) {
            DataLinkHost dh = host.getDataLinkHost();
            if (allow.contains(dh)) {
                tmpDenied = host;
                assertEquals(host.getPort(),
                             mmapHosts.remove(host.getMacVlan()));
                PortVlan pvlan = host.getPortVlan();
                mmapNw.remove(pvlan);
                assertTrue(macEntries.remove(host.getMacAddressEntry()));
                assertTrue(tmpDeny.add(dh));
                break;
            }
        }
        assertNotNull(tmpDenied);
        tmpDenied.setMapping(null);

        mcconf = new MacMapConfig(allow, tmpDeny);
        try {
            assertEquals(UpdateType.CHANGED,
                         mgr.setMacMap(bpath3, UpdateOperation.SET, mcconf));
            MacMap mcmap = mgr.getMacMap(bpath3);
            assertTrue(mcconf.equals(mcmap));
            List<MacAddressEntry> mclist = mcmap.getMappedHosts();
            assertEquals(macEntries.size(), mclist.size());
            assertTrue(macEntries.containsAll(mclist));
        } catch (Exception e) {
            unexpected(e);
        }

        listener.checkEvent(VTNListenerType.MACMAP, bpath3, mcconf,
                            UpdateType.CHANGED);
        assertEquals(mmapNw, resMgr.getMacMappedNetworks(mgr, mpath));
        assertEquals(mmapHosts, resMgr.getMacMappedHosts(mgr, mpath));
        listener.checkEmtpy();

        for (VBridgePath bp: bridges) {
            tmpDenied.checkLearned(mgr, bp);
        }

        flushTasks();
        flushFlowTasks();
        for (Iterator<VTNFlow> it = flowMap.values().iterator();
             it.hasNext();) {
            VTNFlow vflow = it.next();
            MacVlan mvlan = tmpDenied.getMacVlan();
            if (vflow.dependsOn(mpath) && vflow.dependsOn(mvlan)) {
                it.remove();
                checkVTNFlowUninstalled(tname, vflow);
                flowCount -= vflow.getFlowEntries().size();
            }
        }

        // Restore MAC mapping configuration.
        mcconf = new MacMapConfig(allow, deny);
        try {
            assertEquals(UpdateType.CHANGED,
                         mgr.setMacMap(bpath3, UpdateOperation.SET, mcconf));
            MacMap mcmap = mgr.getMacMap(bpath3);
            assertTrue(mcconf.equals(mcmap));
            List<MacAddressEntry> mclist = mcmap.getMappedHosts();
            assertEquals(macEntries.size(), mclist.size());
            assertTrue(macEntries.containsAll(mclist));

            PortVlan pvlan = tmpDenied.getPortVlan();
            mmapNw.add(pvlan);
            assertTrue(macEntries.add(tmpDenied.getMacAddressEntry()));
            mmapHosts.put(tmpDenied.getMacVlan(), tmpDenied.getPort());
        } catch (Exception e) {
            unexpected(e);
        }

        listener.checkEvent(VTNListenerType.MACMAP, bpath3, mcconf,
                            UpdateType.CHANGED);

        tmpDenied.setMapping(mref);
        checkMapping(bridges, tmpDenied, flowCount);
        assertEquals(mmapNw, resMgr.getMacMappedNetworks(mgr, mpath));
        assertEquals(mmapHosts, resMgr.getMacMappedHosts(mgr, mpath));
        fcnt = checkMapping(bridges, allHosts, flowMap, flowCount);
        assertTrue(fcnt > flowCount);
        flowCount = fcnt;
        listener.checkEmtpy();

        // Map VLAN 2 at `additional0' by port mapping.
        // This should inactivate MAC mappings for hosts in `movedHosts'.
        MapReference pref2 = null;
        short vid = 2;
        String pid = additional0.getNodeConnectorIDString();
        SwitchPort swPort = new SwitchPort(NodeConnectorIDType.OPENFLOW, pid);
        PortMapConfig pmconf = new PortMapConfig(additional0.getNode(),
                                                 swPort, vid);
        VBridgeIfPath ifpath = new VBridgeIfPath(bpath2, "if_" + vid);
        PortMap pmap = new PortMap(pmconf, additional0);
        assertNull(interfaces2.put(ifpath, pmap));

        VInterfaceConfig ifconf = new VInterfaceConfig(null, Boolean.TRUE);
        Status st = mgr.addInterface(ifpath, ifconf);
        assertEquals(StatusCode.SUCCESS, st.getCode());
        VInterface vif = new VInterface(ifpath.getInterfaceName(),
                                        VNodeState.UNKNOWN, VNodeState.UNKNOWN,
                                        ifconf);
        listener.checkEvent(VTNListenerType.VBRIDGE_IF, ifpath, vif,
                            UpdateType.ADDED);

        st = mgr.setPortMap(ifpath, pmconf);
        assertEquals(StatusCode.SUCCESS, st.getCode());
        listener.checkEvent(VTNListenerType.PORTMAP, ifpath, pmap,
                            UpdateType.ADDED);
        vif = new VInterface(ifpath.getInterfaceName(), VNodeState.UP,
                             VNodeState.UP, ifconf);
        vif2.add(vif);
        listener.checkEvent(VTNListenerType.VBRIDGE_IF, ifpath, vif,
                            UpdateType.CHANGED);
        listener.checkEmtpy();
        pref2 = new MapReference(MapType.PORT, container, ifpath);

        flushTasks();
        flushFlowTasks();

        mmapNw.removeCounter(new PortVlan(additional0, (short)2));
        assertEquals(mmapNw, resMgr.getMacMappedNetworks(mgr, mpath));

        for (TestHost host: movedHosts) {
            // MAC address entry should be purged.
            host.setMapping(null);
            for (VBridgePath bp: bridges) {
                host.checkLearned(mgr, bp);
            }
            host.setMapping(pref2);

            MacAddressEntry ment = host.getMacAddressEntry();
            assertTrue(macEntries.remove(ment));
            try {
                EthernetAddress eaddr = host.getEthernetAddress();
                assertEquals(null, mgr.getMacMappedHost(bpath3, eaddr));
            } catch (Exception e) {
                unexpected(e);
            }

            // Determine VTN flows to be purged.
            for (Iterator<VTNFlow> it = flowMap.values().iterator();
                 it.hasNext();) {
                VTNFlow vflow = it.next();
                MacVlan mvlan = host.getMacVlan();
                if (vflow.dependsOn(mpath) && vflow.dependsOn(mvlan)) {
                    it.remove();
                    checkVTNFlowUninstalled(tname, vflow);
                    flowCount -= vflow.getFlowEntries().size();
                }
            }
            assertEquals(host.getPort(), mmapHosts.remove(host.getMacVlan()));
        }
        assertEquals(mmapHosts, resMgr.getMacMappedHosts(mgr, mpath));

        fcnt = checkMapping(bridges, allHosts, flowMap, flowCount);
        assertTrue(fcnt > flowCount);
        flowCount = fcnt;
        listener.checkEmtpy();

        try {
            List<MacAddressEntry> mclist = mgr.getMacMappedHosts(bpath3);
            assertEquals(macEntries.size(), mclist.size());
            assertTrue(macEntries.containsAll(mclist));
        } catch (Exception e) {
            unexpected(e);
        }

        // Map 2 hosts on VLAN 2 with specifying MAC address.
        // This should not affect flow entries.
        int mapped = 0;
        Set<DataLinkHost> allow1 = new HashSet<DataLinkHost>();
        for (TestHost host: allowedHosts) {
            short vlan = host.getVlan();
            if (vlan != 1) {
                continue;
            }
            DataLinkHost dh = host.getDataLinkHost();
            assertTrue(allow1.add(dh));
            assertTrue(allow.add(dh));
            mapped++;
            if (mapped >= 2) {
                break;
            }
        }

        try {
            MacMapConfig mc = new MacMapConfig(allow1, null);
            mcconf = new MacMapConfig(allow, deny);
            assertEquals(UpdateType.CHANGED,
                         mgr.setMacMap(bpath3, UpdateOperation.ADD, mc));
            MacMap mcmap = mgr.getMacMap(bpath3);
            assertTrue(mcconf.equals(mcmap));
            List<MacAddressEntry> mclist = mcmap.getMappedHosts();
            assertEquals(macEntries.size(), mclist.size());
            assertTrue(macEntries.containsAll(mclist));
        } catch (Exception e) {
            unexpected(e);
        }

        listener.checkEvent(VTNListenerType.MACMAP, bpath3, mcconf,
                            UpdateType.CHANGED);
        assertEquals(mmapNw, resMgr.getMacMappedNetworks(mgr, mpath));
        assertEquals(mmapHosts, resMgr.getMacMappedHosts(mgr, mpath));
        listener.checkEmtpy();
        fcnt = checkMapping(bridges, allHosts, flowMap, flowCount);
        assertEquals(fcnt, flowCount);
        listener.checkEmtpy();

        // Configure MAC mapping into bridges[4].
        VBridgePath bpath4 = bridges.get(4);
        Set<DataLinkHost> allow4 = new HashSet<DataLinkHost>();
        Set<DataLinkHost> deny4 = new HashSet<DataLinkHost>();
        List<TestHost> allowedHosts4 = new ArrayList<TestHost>();
        List<TestHost> deniedHosts4 = new ArrayList<TestHost>();
        MacMapPath mpath4 = new MacMapPath(bpath4);
        MapReference mref4 = new MapReference(MapType.MAC, container, mpath4);

        // Map hosts on VLAN 3 using wildcard MAC address.
        DataLinkHost mmapVlan3 = new EthernetHost((short)3);
        assertTrue(allow4.add(mmapVlan3));

        MacMapConfig mcconf4 = new MacMapConfig(allow4, null);
        try {
            assertEquals(null, mgr.getMacMap(bpath4));
            assertEquals(UpdateType.ADDED,
                         mgr.setMacMap(bpath4, UpdateOperation.SET,
                                       MacMapAclType.ALLOW, allow4));
            listener.checkEvent(VTNListenerType.MACMAP, bpath4, mcconf4,
                                UpdateType.ADDED);
            MacMap mcmap = mgr.getMacMap(bpath4);
            assertTrue(mcconf4.equals(mcmap));
            assertTrue(mcmap.getMappedHosts().isEmpty());

            VBridge vbr = mgr.getBridge(bpath4);
            assertEquals(VNodeState.DOWN, vbr.getState());
            listener.checkEvent(VTNListenerType.VBRIDGE, bpath4, vbr,
                                UpdateType.CHANGED);
        } catch (Exception e) {
            unexpected(e);
        }
        listener.checkEmtpy();

        // Activate MAC mapping on bridges[4].
        CounterSet<PortVlan> mmapNw4 = null;
        Map<MacVlan, NodeConnector> mmapHosts4 = null;
        Set<MacAddressEntry> macEntries4 = new HashSet<MacAddressEntry>();
        active = false;
        for (TestHost host: allHosts) {
            if (host.getVlan() != 3) {
                continue;
            }

            assertEquals(mmapNw4, resMgr.getMacMappedNetworks(mgr, mpath4));
            assertEquals(mmapHosts4, resMgr.getMacMappedHosts(mgr, mpath4));
            allowedHosts4.add(host);
            host.setMapping(mref4);
            checkMapping(bridges, host, flowCount);
            if (!active) {
                // VBridge state should be changed to UP.
                VBridge vbr = null;
                try {
                    vbr = mgr.getBridge(bpath4);
                } catch (Exception e) {
                    unexpected(e);
                }
                assertEquals(VNodeState.UP, vbr.getState());
                listener.checkEvent(VTNListenerType.VBRIDGE, bpath4, vbr,
                                    UpdateType.CHANGED);
                active = true;
            }

            PortVlan pvlan = host.getPortVlan();
            if (mmapNw4 == null) {
                mmapNw4 = new CounterSet<PortVlan>();
            }
            mmapNw4.add(pvlan);
            assertEquals(mmapNw4, resMgr.getMacMappedNetworks(mgr, mpath4));
            assertTrue(macEntries4.add(host.getMacAddressEntry()));
            if (mmapHosts4 == null) {
                mmapHosts4 = new HashMap<MacVlan, NodeConnector>();
            }
            assertNull(mmapHosts4.put(host.getMacVlan(), host.getPort()));

            try {
                MacMap mcmap = mgr.getMacMap(bpath4);
                assertTrue(mcconf4.equals(mcmap));
                List<MacAddressEntry> mclist = mcmap.getMappedHosts();
                assertEquals(macEntries4.size(), mclist.size());
                assertTrue(macEntries4.containsAll(mclist));
            } catch (Exception e) {
                unexpected(e);
            }
        }

        fcnt = checkMapping(bridges, allHosts, flowMap, flowCount);
        assertTrue(fcnt > flowCount);
        flowCount = fcnt;
        listener.checkEmtpy();

        // Map 2 hosts mapped to bridges[4] to bridges[3].
        movedHosts.clear();
        allow1.clear();
        Set<MacVlan> purgeSet = new HashSet<MacVlan>();
        for (Iterator<TestHost> it = allowedHosts4.iterator(); it.hasNext();) {
            TestHost host = it.next();
            DataLinkHost dh = host.getDataLinkHost();
            assertTrue(allow.add(dh));
            assertTrue(allow1.add(dh));
            host.setMapping(mref);
            movedHosts.add(host);
            purgeSet.add(host.getMacVlan());
            assertEquals(host.getPort(), mmapHosts4.remove(host.getMacVlan()));
            mmapNw4.remove(host.getPortVlan());
            assertTrue(macEntries4.remove(host.getMacAddressEntry()));
            it.remove();
            if (movedHosts.size() >= 2) {
                break;
            }
        }
        assertEquals(2, movedHosts.size());

        mcconf = new MacMapConfig(allow, deny);
        try {
            assertEquals(UpdateType.CHANGED,
                         mgr.setMacMap(bpath3, UpdateOperation.ADD,
                                       MacMapAclType.ALLOW, allow1));
            listener.checkEvent(VTNListenerType.MACMAP, bpath3, mcconf,
                                UpdateType.CHANGED);
            MacMap mcmap = mgr.getMacMap(bpath3);
            assertTrue(mcconf.equals(mcmap));
            List<MacAddressEntry> mclist = mcmap.getMappedHosts();
            assertEquals(macEntries.size(), mclist.size());
            assertTrue(macEntries.containsAll(mclist));

            // MAC address entries in bridges[4] should be purged.
            mclist = mgr.getMacMappedHosts(bpath4);
            assertEquals(macEntries4.size(), mclist.size());
        } catch (Exception e) {
            unexpected(e);
        }

        assertEquals(mmapNw, resMgr.getMacMappedNetworks(mgr, mpath));
        assertEquals(mmapHosts, resMgr.getMacMappedHosts(mgr, mpath));
        assertEquals(mmapNw4, resMgr.getMacMappedNetworks(mgr, mpath4));
        assertEquals(mmapHosts4, resMgr.getMacMappedHosts(mgr, mpath4));
        listener.checkEmtpy();
        flushTasks();
        flushFlowTasks();

        for (Iterator<VTNFlow> it = flowMap.values().iterator();
             it.hasNext();) {
            VTNFlow vflow = it.next();
            if (vflow.dependsOn(mpath4)) {
                ObjectPair<L2Host, L2Host> edges = vflow.getEdgeHosts();
                L2Host in = edges.getLeft();
                L2Host out = edges.getRight();
                if (purgeSet.contains(in.getHost()) ||
                    purgeSet.contains(out.getHost())) {
                    it.remove();
                    checkVTNFlowUninstalled(tname, vflow);
                    flowCount -= vflow.getFlowEntries().size();
                }
            }
        }

        // Activate MAC mapping for hosts in movedHosts.
        for (TestHost host: movedHosts) {
            assertEquals(mmapNw, resMgr.getMacMappedNetworks(mgr, mpath));
            assertEquals(mmapHosts, resMgr.getMacMappedHosts(mgr, mpath));

            checkMapping(bridges, host, flowCount);

            mmapNw.add(host.getPortVlan());
            assertNull(mmapHosts.put(host.getMacVlan(), host.getPort()));
            assertEquals(mmapNw, resMgr.getMacMappedNetworks(mgr, mpath));
            assertEquals(mmapHosts, resMgr.getMacMappedHosts(mgr, mpath));
            assertTrue(macEntries.add(host.getMacAddressEntry()));

            try {
                MacMap mcmap = mgr.getMacMap(bpath3);
                assertTrue(mcconf.equals(mcmap));
                List<MacAddressEntry> mclist = mcmap.getMappedHosts();
                assertEquals(macEntries.size(), mclist.size());
                assertTrue(macEntries.containsAll(mclist));
            } catch (Exception e) {
                unexpected(e);
            }
        }

        fcnt = checkMapping(bridges, allHosts, flowMap, flowCount);
        assertTrue(fcnt > flowCount);
        flowCount = fcnt;
        listener.checkEmtpy();

        // Map 3 hosts in allowedHosts4 explicitly.
        // This should not affect flow entries.
        for (TestHost host: allowedHosts4) {
            DataLinkHost dh = host.getDataLinkHost();
            assertTrue(allow4.add(dh));

            mcconf4 = new MacMapConfig(allow4, null);
            try {
                assertEquals(UpdateType.CHANGED,
                             mgr.setMacMap(bpath4, UpdateOperation.SET,
                                           MacMapAclType.ALLOW, allow4));
                listener.checkEvent(VTNListenerType.MACMAP, bpath4, mcconf4,
                                    UpdateType.CHANGED);
            } catch (Exception e) {
                unexpected(e);
            }

            if (allow4.size() >= 4) {
                break;
            }
        }

        assertEquals(mmapNw4, resMgr.getMacMappedNetworks(mgr, mpath4));
        assertEquals(mmapHosts4, resMgr.getMacMappedHosts(mgr, mpath4));
        fcnt = checkMapping(bridges, allHosts, flowMap, flowCount);
        assertEquals(fcnt, flowCount);
        listener.checkEmtpy();

        // Remove wildcard mapping for VLAN 3.
        purgeSet.clear();
        assertTrue(allow4.remove(mmapVlan3));
        assertEquals(3, allow4.size());
        allow1.clear();
        assertTrue(allow1.add(mmapVlan3));
        mcconf4 = new MacMapConfig(allow4, null);
        for (Iterator<TestHost> it = allowedHosts4.iterator(); it.hasNext();) {
            TestHost host = it.next();
            DataLinkHost dh = host.getDataLinkHost();
            if (!allow4.contains(dh)) {
                it.remove();
                host.setMapping(null);
                purgeSet.add(host.getMacVlan());
                assertEquals(host.getPort(),
                             mmapHosts4.remove(host.getMacVlan()));
                mmapNw4.remove(host.getPortVlan());
                assertTrue(macEntries4.remove(host.getMacAddressEntry()));
            }
        }
        assertEquals(allow4.size(), allowedHosts4.size());

        try {
            MacMapConfig mc = new MacMapConfig(allow1, null);
            assertEquals(UpdateType.CHANGED,
                         mgr.setMacMap(bpath4, UpdateOperation.REMOVE,
                                       MacMapAclType.ALLOW, allow1));
            listener.checkEvent(VTNListenerType.MACMAP, bpath4, mcconf4,
                                UpdateType.CHANGED);
            MacMap mcmap = mgr.getMacMap(bpath4);
            assertTrue(mcconf4.equals(mcmap));
        } catch (Exception e) {
            unexpected(e);
        }

        assertEquals(mmapNw4, resMgr.getMacMappedNetworks(mgr, mpath4));
        assertEquals(mmapHosts4, resMgr.getMacMappedHosts(mgr, mpath4));
        listener.checkEmtpy();
        flushTasks();
        flushFlowTasks();

        for (Iterator<VTNFlow> it = flowMap.values().iterator();
             it.hasNext();) {
            VTNFlow vflow = it.next();
            if (vflow.dependsOn(mpath4)) {
                ObjectPair<L2Host, L2Host> edges = vflow.getEdgeHosts();
                L2Host in = edges.getLeft();
                L2Host out = edges.getRight();
                if (purgeSet.contains(in.getHost()) ||
                    purgeSet.contains(out.getHost())) {
                    it.remove();
                    checkVTNFlowUninstalled(tname, vflow);
                    flowCount -= vflow.getFlowEntries().size();
                }
            }
        }

        fcnt = checkMapping(bridges, allHosts, flowMap, flowCount);
        assertEquals(flowCount, fcnt);
        flowCount = fcnt;
        listener.checkEmtpy();

        // Remove MAC mapping configured in bridges[4].
        for (Iterator<TestHost> it = allowedHosts4.iterator(); it.hasNext();) {
            TestHost host = it.next();
            DataLinkHost dh = host.getDataLinkHost();
            UpdateType utype;
            if (it.hasNext()) {
                utype = UpdateType.CHANGED;
                assertTrue(allow4.remove(dh));
                mcconf4 = new MacMapConfig(allow4, null);
            } else {
                utype = UpdateType.REMOVED;
                mcconf4 = new MacMapConfig(allow4, null);
                assertTrue(allow4.remove(dh));
            }

            allow1.clear();
            allow1.add(dh);
            host.setMapping(null);
            assertEquals(host.getPort(), mmapHosts4.remove(host.getMacVlan()));
            mmapNw4.remove(host.getPortVlan());
            assertTrue(macEntries4.remove(host.getMacAddressEntry()));

            try {
                MacMapConfig mc = new MacMapConfig(allow1, null);
                assertEquals(utype,
                             mgr.setMacMap(bpath4, UpdateOperation.REMOVE,
                                           MacMapAclType.ALLOW, allow1));
                listener.checkEvent(VTNListenerType.MACMAP, bpath4, mcconf4,
                                    utype);

                if (utype == UpdateType.REMOVED) {
                    mmapNw4 = null;
                    mmapHosts4 = null;
                    VBridge vbr = mgr.getBridge(bpath4);
                    assertEquals(VNodeState.UNKNOWN, vbr.getState());
                    listener.checkEvent(VTNListenerType.VBRIDGE, bpath4, vbr,
                                        UpdateType.CHANGED);
                    assertNull(mgr.getMacMap(bpath4));
                } else {
                    MacMap mcmap = mgr.getMacMap(bpath4);
                    assertTrue(mcconf4.equals(mcmap));
                    List<MacAddressEntry> mclist = mcmap.getMappedHosts();
                    assertEquals(macEntries4.size(), mclist.size());
                    assertTrue(macEntries4.containsAll(mclist));
                }

                assertEquals(mmapNw4,
                             resMgr.getMacMappedNetworks(mgr, mpath4));
                assertEquals(mmapHosts4,
                             resMgr.getMacMappedHosts(mgr, mpath4));
            } catch (Exception e) {
                unexpected(e);
            }
        }

        listener.checkEmtpy();
        flushTasks();
        flushFlowTasks();

        for (Iterator<VTNFlow> it = flowMap.values().iterator();
             it.hasNext();) {
            VTNFlow vflow = it.next();
            if (vflow.dependsOn(mpath4)) {
                it.remove();
                checkVTNFlowUninstalled(tname, vflow);
                flowCount -= vflow.getFlowEntries().size();
            }
        }

        fcnt = checkMapping(bridges, allHosts, flowMap, flowCount);
        assertEquals(flowCount, fcnt);
        flowCount = fcnt;
        listener.checkEmtpy();

        try {
            // Destroy all vBridges.
            for (int i = 0; i < bridges.size(); i++) {
                VBridgePath bpath = bridges.get(i);
                VBridge vbr = mgr.getBridge(bpath);
                st = mgr.removeBridge(bpath);
                assertEquals(StatusCode.SUCCESS, st.getCode());

                if (i == 0) {
                    listener.checkEvent(VTNListenerType.VLANMAP, bpath, vmap0,
                                        UpdateType.REMOVED);
                } else if (i == 1) {
                    listener.checkEvent(VTNListenerType.VLANMAP, bpath, vmap1,
                                        UpdateType.REMOVED);
                } else if (i == 2) {
                    int idx = 0;
                    for (Map.Entry<VBridgeIfPath, PortMap> entry:
                             interfaces2.entrySet()) {
                        VBridgeIfPath ipath = entry.getKey();
                        pmap = entry.getValue();
                        if (pmap != null) {
                            listener.checkEvent(VTNListenerType.PORTMAP, ipath,
                                                pmap, UpdateType.REMOVED);
                        }

                        VInterface v = vif2.get(idx);
                        idx++;
                        listener.checkEvent(VTNListenerType.VBRIDGE_IF, ipath,
                                            v, UpdateType.REMOVED);
                    }
                } else if (i == 3) {
                    listener.checkEvent(VTNListenerType.MACMAP, bpath, mcconf,
                                        UpdateType.REMOVED);
                    assertNull(resMgr.getMacMappedHosts(mgr, mpath));
                    assertNull(resMgr.getMacMappedNetworks(mgr, mpath));
                }

                listener.checkEvent(VTNListenerType.VBRIDGE, bpath, vbr,
                                    UpdateType.REMOVED);

                flushTasks();
                flushFlowTasks();
                for (Iterator<VTNFlow> it = flowMap.values().iterator();
                     it.hasNext();) {
                    VTNFlow vflow = it.next();
                    if (vflow.dependsOn(bpath)) {
                        it.remove();
                        checkVTNFlowUninstalled(tname, vflow);
                        flowCount -= vflow.getFlowEntries().size();
                    }
                }
                stubObj.checkFlowCount(flowCount);
            }
        } catch (Exception e) {
            unexpected(e);
        }

        listener.checkEmtpy();
        assertEquals(0, flowCount);
        assertTrue(flowMap.isEmpty());

        // Destroy VTN.
        try {
            st = mgr.removeTenant(tpath);
            assertEquals(StatusCode.SUCCESS, st.getCode());
            listener.checkEvent(VTNListenerType.VTN, tpath, vtn,
                                UpdateType.REMOVED);
        } catch (Exception e) {
            unexpected(e);
        }

        flushTasks();
        listener.checkEmtpy();
    }

    // private methods.

    /**
     * common method for test which use BroadCast Packet (ARP Request packet).
     * this method expected that action for received packet is flooding to mapped port.
     */
    private void testReceiveDataPacketBCLoop(VTNManagerImpl mgr, VBridgePath bpath,
           MapType type, Set<PortVlan> mappedThis, TestStub stub) {
        testReceiveDataPacketCommonLoop(mgr, bpath, MapType.PORT, mappedThis, stub,
                                        (short)1, EtherTypes.ARP, null);
    }

    /**
     * common method for test which use UniCast Packet (IPv4 packet).
     * this method expected that action for received packet is flooding to mapped port.
     */
    private void testReceiveDataPacketUCLoop(VTNManagerImpl mgr, VBridgePath bpath,
            MapType type, Set<PortVlan> mappedThis, TestStub stub) {
        testReceiveDataPacketCommonLoop(mgr, bpath, MapType.PORT, mappedThis, stub,
                                        (short)0, EtherTypes.IPv4, null);
    }

    /**
     * common method for
     * {@link #testReceiveDataPacketBCLoop(VTNManagerImpl, VBridgePath, MapType, Set, TestStub)} and
     * {@link #testReceiveDataPacketUCLoop(VTNManagerImpl, VBridgePath, MapType, Set, TestStub)}.
     * this method expected that action for received packet is flooding to mapped port.
     */
    private void testReceiveDataPacketCommonLoop(VTNManagerImpl mgr, VBridgePath bpath,
            MapType type, Set<PortVlan> mappedThis, TestStub stub, short bc,
            EtherTypes ethtype, PortVlan targetPv) {
        ISwitchManager swmgr = mgr.getSwitchManager();
        byte[] cntMac = swmgr.getControllerMAC();
        byte[] dst;
        if (bc > 0) {
            dst = new byte[] {(byte)0xff, (byte)0xff, (byte)0xff,
                              (byte)0xff, (byte)0xff, (byte)0xff};
        } else {
            dst = new byte[] {(byte)0x00, (byte)0x00, (byte)0x00,
                              (byte)0xff, (byte)0xff, (byte)0x11};
        }
        byte[] target = new byte[] {(byte)192, (byte)168, (byte)0, (byte)250};

        for (PortVlan inPv : mappedThis) {
            NodeConnector inNc = inPv.getNodeConnector();
            short inVlan = inPv.getVlan();
            if (targetPv != null && !targetPv.equals(inPv)) {
                continue;
            }
            byte iphost = 1;
            for (EthernetAddress ea : createEthernetAddresses(false)) {
                String emsg = "(input portvlan)" + inPv.toString()
                        + ",(input eth)" + ea.toString();
                byte[] bytes = ea.getValue();
                byte[] src = new byte[] {bytes[0], bytes[1], bytes[2],
                                         bytes[3], bytes[4], bytes[5]};
                byte[] sender = new byte[] {(byte)192, (byte)168, (byte)0, (byte)iphost};

                RawPacket inPkt = null;
                if (ethtype.shortValue() == EtherTypes.IPv4.shortValue()) {
                    inPkt = createIPv4RawPacket(src, dst, sender, target,
                                                (inVlan > 0) ? inVlan : -1,
                                                inNc);
                } else if (ethtype.shortValue() ==
                           EtherTypes.ARP.shortValue()) {
                    inPkt = createARPRawPacket(
                        src, dst, sender, target, (inVlan > 0) ? inVlan : -1,
                        inNc, ARP.REQUEST);
                }
                Ethernet inPktDecoded = (Ethernet)stub.decodeDataPacket(inPkt);
                PacketResult result = mgr.receiveDataPacket(inPkt);

                if (inNc != null &&
                    inNc.getType() == NodeConnector.NodeConnectorIDType.OPENFLOW) {
                    assertEquals(emsg, PacketResult.KEEP_PROCESSING, result);
                    flushFlowTasks();
                    Set<FlowEntry> flowEntries = stub.getFlowEntries();
                    expireFlows(mgr, stub);
                    assertEquals(emsg, 0, flowEntries.size());

                    MacAddressEntry entry = null;
                    try {
                        entry = mgr.getMacEntry(bpath, new EthernetAddress(src));
                    } catch (Exception e) {
                        unexpected(e);
                    }

                    Packet payload = inPktDecoded.getPayload();
                    if (inPktDecoded.getEtherType() == EtherTypes.VLANTAGGED.shortValue()) {
                        payload = payload.getPayload();
                    }
                    if (payload instanceof ARP) {
                        assertEquals(inNc.toString() + ea.toString(), 1,
                                     entry.getInetAddresses().size());
                        assertArrayEquals(emsg, sender,
                                entry.getInetAddresses().iterator().next().getAddress());
                    } else {
                        assertEquals(emsg, 0, entry.getInetAddresses().size());
                    }

                    List<RawPacket> dataList = stub.getTransmittedDataPacket();

                    if (ethtype.shortValue() == EtherTypes.IPv4.shortValue()) {
                        assertEquals(emsg, mappedThis.size(), dataList.size());
                    } else {
                        assertEquals(emsg, mappedThis.size() - 1, dataList.size());
                    }

                    for (RawPacket raw: dataList) {
                        Ethernet pkt = (Ethernet)stub.decodeDataPacket(raw);
                        String emsgPkt = emsg + ",(out packet)" + pkt.toString()
                                + ",(in nc)" + raw.getIncomingNodeConnector()
                                + ",(out nc)" + raw.getOutgoingNodeConnector();
                        short outVlan;
                        if (pkt.getEtherType() == EtherTypes.VLANTAGGED.shortValue()) {
                            IEEE8021Q vlantag = (IEEE8021Q)pkt.getPayload();
                            assertTrue(emsgPkt,
                                mappedThis.contains(new PortVlan(raw.getOutgoingNodeConnector(), vlantag.getVid())));
                            outVlan = vlantag.getVid();
                        } else {
                            assertTrue(emsgPkt,
                                    mappedThis.contains(new PortVlan(raw.getOutgoingNodeConnector(), (short)0)));
                            outVlan = (short)0;
                        }

                        PortVlan outPv = new PortVlan(raw.getOutgoingNodeConnector(),
                                                      outVlan);
                        if (ethtype.shortValue() == EtherTypes.ARP.shortValue() ||
                            (ethtype.shortValue() == EtherTypes.IPv4.shortValue() &&
                            !outPv.equals(inPv))) {

                            if (ethtype.shortValue() == EtherTypes.IPv4.shortValue()) {
                                checkOutEthernetPacketIPv4(emsgPkt,
                                        (Ethernet)pkt, EtherTypes.IPv4, src, dst, outVlan);
                            } else if (ethtype.shortValue() ==
                                       EtherTypes.ARP.shortValue()) {
                                checkOutEthernetPacket(emsgPkt,
                                        (Ethernet)pkt, EtherTypes.ARP, src, dst, outVlan,
                                        EtherTypes.IPv4, ARP.REQUEST, src, dst,
                                        sender, target);
                            } else {
                                fail("unexpected packet received.");
                            }
                        } else {
                            checkOutEthernetPacket(emsgPkt,
                                    (Ethernet)pkt, EtherTypes.ARP, cntMac, src, outVlan,
                                    EtherTypes.IPv4, ARP.REQUEST, cntMac, src,
                                    new byte[] {0, 0, 0, 0}, sender);
                        }
                    }
                } else {
                    if (inNc != null) {
                        assertEquals(inNc.toString() + "," + ea.toString(),
                                     PacketResult.IGNORED, result);
                    } else {
                        assertEquals(ea.toString(), PacketResult.IGNORED, result);
                    }
                }
                iphost++;
            }
        }
    }

    /**
     * common method for test which use BroadCast Packet (ARP Request packet).
     * this method expected that action for received packet is flooding to mapped port.
     */
    private void testReceiveDataPacketBCLoopLearned(VTNManagerImpl mgr, VBridgePath bpath,
           MapType type, Set<PortVlan> mappedThis, TestStub stub) {
        testReceiveDataPacketCommonLoopLearned(mgr, bpath, MapType.PORT, mappedThis,
                                               stub, (short)1, EtherTypes.ARP);
    }

    /**
     * common method for test which use UniCast Packet (IPv4 packet).
     * this method expected that action for received packet is output to learned port.
     */
    private void testReceiveDataPacketUCLoopLearned(VTNManagerImpl mgr, VBridgePath bpath,
            MapType type, Set<PortVlan> mappedThis, TestStub stub) {
        testReceiveDataPacketCommonLoopLearned(mgr, bpath, MapType.PORT, mappedThis,
                                               stub, (short)0, EtherTypes.IPv4);
    }

    /**
     * common method for
     * {@link #testReceiveDataPacketBCLoop(VTNManagerImpl, VBridgePath, MapType, Set, TestStub)} and
     * {@link #testReceiveDataPacketUCLoop(VTNManagerImpl, VBridgePath, MapType, Set, TestStub)}.
     * this method expected that action for received packet is a port host is connected to.
     *
     */
    private void testReceiveDataPacketCommonLoopLearned(VTNManagerImpl mgr, VBridgePath bpath,
            MapType type, Set<PortVlan> mappedThis, TestStub stub, short bc, EtherTypes ethtype) {
        ISwitchManager swmgr = mgr.getSwitchManager();
        byte[] cntMac = swmgr.getControllerMAC();
        byte[] src;
        if (bc > 0) {
            src = new byte[] {(byte)0xff, (byte)0xff, (byte)0xff,
                              (byte)0xff, (byte)0xff, (byte)0xff};
        } else {
            src = new byte[] {(byte)0x00, (byte)0x00, (byte)0x00,
                              (byte)0xff, (byte)0xff, (byte)0x11};
        }
        byte[] sender = new byte[] {(byte)192, (byte)168, (byte)0, (byte)250};

        VTenant tenant = null;
        try {
            tenant = vtnMgr.getTenant(new VTenantPath(bpath.getTenantName()));
        } catch (VTNException e) {
            unexpected(e);
        }

        List<EthernetAddress> ethers = createEthernetAddresses(false);
        for (PortVlan learnedPv : mappedThis) {
            NodeConnector learnedNc = learnedPv.getNodeConnector();
            Set<String> registeredFlows = new HashSet<String>();

            // first learned hosts to vbridge.
            testReceiveDataPacketCommonLoop(mgr, bpath, MapType.PORT, mappedThis,
                    stub, (short)0, EtherTypes.IPv4, learnedPv);

            // mac addresses have been learned at this point.
            for (PortVlan inPortVlan : mappedThis) {
                NodeConnector inNc = inPortVlan.getNodeConnector();
                short inVlan = inPortVlan.getVlan();
                byte tip = 1;
                int numExpectFlows = 0;
                for (EthernetAddress ea: ethers) {
                    String emsg = "(learned portvlan)" + learnedPv.toString()
                            + "(input portvlan)"
                            + inPortVlan.toString() + ",(input eth)" + ea.toString();

                    byte[] bytes = ea.getValue();
                    byte[] dst = new byte[] {bytes[0], bytes[1], bytes[2],
                                             bytes[3], bytes[4], bytes[5]};
                    byte[] target = new byte[] {(byte)192, (byte)168, (byte)0, (byte)tip};

                    RawPacket inPkt = null;
                    if (ethtype.shortValue() == EtherTypes.IPv4.shortValue()) {
                        inPkt = createIPv4RawPacket(src, dst, sender, target,
                                                    (inVlan > 0) ? inVlan : -1, inNc);
                    } else if (ethtype.shortValue() ==
                               EtherTypes.ARP.shortValue()) {
                        // not used case.
                    }

                    Ethernet inPktDecoded = (Ethernet)stub.decodeDataPacket(inPkt);
                    PacketResult result = mgr.receiveDataPacket(inPkt);

                    if (inNc != null &&
                        inNc.getType() == NodeConnector.NodeConnectorIDType.OPENFLOW) {
                        assertEquals(emsg, PacketResult.KEEP_PROCESSING, result);
                        flushFlowTasks();
                        Set<FlowEntry> flowEntries = stub.getFlowEntries();

                        MacAddressEntry entry = null;
                        try {
                            entry = mgr.getMacEntry(bpath, new EthernetAddress(src));
                        } catch (Exception e) {
                            unexpected(e);
                        }

                        Packet payload = inPktDecoded.getPayload();
                        if (inPktDecoded.getEtherType() == EtherTypes.VLANTAGGED.shortValue()) {
                            payload = payload.getPayload();
                        }
                        if (payload instanceof ARP) {
                            // not used this case.
                            assertEquals(emsg, 1, entry.getInetAddresses().size());
                            assertArrayEquals(emsg, sender,
                                    entry.getInetAddresses().iterator().next().getAddress());

                        } else {
                            assertEquals(emsg, 0, entry.getInetAddresses().size());
                        }

                        // Check output data packet.
                        List<RawPacket> dataList = stub.getTransmittedDataPacket();

                        // IP address probe request should be sent.
                        assertEquals(emsg, 2, dataList.size());

                        for (RawPacket raw : dataList) {
                            Packet pkt = stub.decodeDataPacket(raw);
                            Ethernet eth = (Ethernet)pkt;
                            short outEthType;
                            short outVlan;
                            String emsgr = emsg + ",(out packet)" + pkt.toString()
                                        + ",(in nc)" + raw.getIncomingNodeConnector()
                                        + ",(out nc)" + raw.getOutgoingNodeConnector();

                            if (eth.getEtherType() == EtherTypes.VLANTAGGED.shortValue()) {
                                IEEE8021Q vlantag = (IEEE8021Q)eth.getPayload();
                                assertTrue(emsgr,
                                        mappedThis.contains(new PortVlan(raw.getOutgoingNodeConnector(),
                                                                         vlantag.getVid())));
                                outEthType = vlantag.getEtherType();
                                outVlan = vlantag.getVid();
                            } else {
                                outEthType = eth.getEtherType();
                                outVlan = (short)-1;
                            }

                            if (outEthType == EtherTypes.ARP.shortValue() &&
                                    raw.getOutgoingNodeConnector().equals(inNc)) {
                                // this is a packet to detect IP address of host
                                // which has no IP address.
                                checkOutEthernetPacket(emsgr,
                                    (Ethernet)pkt, EtherTypes.ARP, cntMac, src, outVlan,
                                    EtherTypes.IPv4, ARP.REQUEST, cntMac, src,
                                    new byte[] {0, 0, 0, 0}, sender);
                            } else {
                                checkOutEthernetPacketIPv4(emsgr,
                                    (Ethernet)pkt, EtherTypes.IPv4, src, dst, outVlan);
                            }
                        }

                        // Check flow entries.
                        Node learnedNode = learnedNc.getNode();
                        Node inNode = inPortVlan.getNodeConnector().getNode();
                        if (learnedNode.equals(inNode)) {
                            numExpectFlows++;
                        } else {
                            numExpectFlows += 2;
                        }
                        assertEquals(emsg, numExpectFlows, flowEntries.size());

                        checkFlowEntries(learnedPv, inPortVlan, flowEntries,
                                         registeredFlows, src, dst, tenant, emsg);
                    } else {
                        if (inNc != null) {
                            assertEquals(emsg, PacketResult.IGNORED, result);
                        } else {
                            assertEquals(emsg, PacketResult.IGNORED, result);
                        }
                    }
                    tip++;
                }
            }
            expireFlows(mgr, stub);
            Status st = mgr.flushMacEntries(bpath);
            assertEquals(StatusCode.SUCCESS, st.getCode());
        }
    }
}
