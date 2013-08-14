/*
 * Copyright (c) 2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */
package org.opendaylight.vtn.manager.internal;


import java.net.InetAddress;
import java.net.UnknownHostException;
import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ConcurrentMap;

import org.junit.BeforeClass;
import org.junit.Test;

import org.opendaylight.controller.hosttracker.hostAware.HostNodeConnector;
import org.opendaylight.controller.sal.core.ConstructionException;
import org.opendaylight.controller.sal.core.Node;
import org.opendaylight.controller.sal.core.NodeConnector;
import org.opendaylight.controller.sal.core.Property;
import org.opendaylight.controller.sal.core.UpdateType;
import org.opendaylight.controller.sal.packet.ARP;
import org.opendaylight.controller.sal.packet.Ethernet;
import org.opendaylight.controller.sal.packet.IEEE8021Q;
import org.opendaylight.controller.sal.packet.Packet;
import org.opendaylight.controller.sal.packet.PacketResult;
import org.opendaylight.controller.sal.packet.RawPacket;
import org.opendaylight.controller.sal.packet.address.EthernetAddress;
import org.opendaylight.controller.sal.utils.EtherTypes;
import org.opendaylight.controller.sal.utils.NodeConnectorCreator;
import org.opendaylight.controller.sal.utils.NodeCreator;
import org.opendaylight.controller.sal.utils.Status;
import org.opendaylight.controller.sal.utils.StatusCode;
import org.opendaylight.controller.switchmanager.ISwitchManager;
import org.opendaylight.vtn.manager.MacAddressEntry;
import org.opendaylight.vtn.manager.PortMap;
import org.opendaylight.vtn.manager.PortMapConfig;
import org.opendaylight.vtn.manager.SwitchPort;
import org.opendaylight.vtn.manager.VBridge;
import org.opendaylight.vtn.manager.VBridgeIfPath;
import org.opendaylight.vtn.manager.VBridgePath;
import org.opendaylight.vtn.manager.VInterface;
import org.opendaylight.vtn.manager.VInterfaceConfig;
import org.opendaylight.vtn.manager.VNodeState;
import org.opendaylight.vtn.manager.VTNException;
import org.opendaylight.vtn.manager.VTenantPath;
import org.opendaylight.vtn.manager.VlanMap;
import org.opendaylight.vtn.manager.VlanMapConfig;

/**
 * JUnit test for {@link VTNManagerImplTest}.
 * This test class test at using stub environment.
 */
public class VTNManagerImplWithNodesTest extends VTNManagerImplTestCommon {

    @BeforeClass
    public static void beforeClass() {
        stubMode = 1;
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
        short[] vlans = new short[] { 0, 10, 4095 };
        String tname = "vtn";
        VTenantPath tpath = new VTenantPath(tname);
        String bname = "vbridge";
        VBridgePath bpath = new VBridgePath(tname, bname);
        String ifname = "vinterface";
        VBridgeIfPath ifp = new VBridgeIfPath(tname, bname, ifname);
        Status st = null;
        List<VBridgePath> bpathlist = new ArrayList<VBridgePath>();
        List<VBridgeIfPath> ifplist = new ArrayList<VBridgeIfPath>();
        bpathlist.add(bpath);
        ifplist.add(ifp);

        createTenantAndBridgeAndInterface(mgr, tpath, bpathlist, ifplist);

        // add a vlanmap to a vbridge
        testVlanMapSingle(mgr, bpath);

        // add multi vlanmap to a vbridge
        for (Node node : createNodes(3)) {
            for (short vlan : vlans) {
                VlanMapConfig vlconf = new VlanMapConfig(node, vlan);
                VlanMap map = null;
                try {
                    map = mgr.addVlanMap(bpath, vlconf);
                    if (node != null && node.getType() != NodeConnector.NodeConnectorIDType.OPENFLOW) {
                        fail("throwing Exception was expected.");
                    }
                } catch (VTNException e) {
                    if (node != null && node.getType() == NodeConnector.NodeConnectorIDType.OPENFLOW) {
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
            if (node == null || node.getType() == NodeConnector.NodeConnectorIDType.OPENFLOW) {
                assertTrue((node == null) ? "" : node.toString(), list.size() == vlans.length);
                VBridge brdg = null;
                try {
                    brdg = mgr.getBridge(bpath);
                } catch (VTNException e) {
                   unexpected(e);
                }
                assertEquals((node == null) ? "" : node.toString(), VNodeState.UP, brdg.getState());
            } else {
                assertTrue(list.size() == 0);
                VBridge brdg = null;
                try {
                    brdg = mgr.getBridge(bpath);
                } catch (VTNException e) {
                   unexpected(e);
                }
                assertEquals(node.toString(), VNodeState.UNKNOWN, brdg.getState());
            }

            for (VlanMap map : list) {
                st = mgr.removeVlanMap(bpath, map.getId());
                assertTrue((node == null) ? "" : node.toString(), st.isSuccess());
            }
        }

        st = mgr.removeTenant(tpath);
        assertTrue(st.isSuccess());
    }

    /**
     * Test method for
     * {@link VTNManagerImpl#getPortMap(VBridgeIfPath)} and
     * {@link VTNManagerImpl#setPortMap(VBridgeIfPath, PortMapConfig)}.
     */
    @Test
    public void testPortMap() {
        VTNManagerImpl mgr = vtnMgr;
        short[] vlans = new short[] { 0, 10, 4095 };
        String tname = "vtn";
        VTenantPath tpath = new VTenantPath(tname);
        String bname = "vbridge";
        VBridgePath bpath = new VBridgePath(tname, bname);
        String ifname = "vinterface";
        VBridgeIfPath ifp = new VBridgeIfPath(tname, bname, ifname);
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
                new SwitchPort("port-10", NodeConnector.NodeConnectorIDType.OPENFLOW, "10"),
                new SwitchPort(null, NodeConnector.NodeConnectorIDType.OPENFLOW, "11"),
                new SwitchPort("port-10", null, null),
                new SwitchPort("port-10"),
                new SwitchPort(NodeConnector.NodeConnectorIDType.OPENFLOW, "13"),
        };

        for (SwitchPort port: ports) {
            for (short vlan : vlans) {
                PortMapConfig pmconf = new PortMapConfig(node, port, (short)vlan);
                st = mgr.setPortMap(ifp, pmconf);
                assertTrue(pmconf.toString(), st.isSuccess());

                PortMap map = null;
                try {
                    map = mgr.getPortMap(ifp);
                } catch (Exception e) {
                    unexpected(e);
                }
                assertEquals(pmconf.toString(), pmconf, map.getConfig());
                assertNotNull(pmconf.toString(), map.getNodeConnector());
                if(port.getId() != null) {
                    assertEquals(pmconf.toString(),
                            Short.parseShort(port.getId()), map.getNodeConnector().getID());
                }
                if (port.getType() != null) {
                    assertEquals(pmconf.toString(),
                            port.getType(), map.getNodeConnector().getType());
                }

                checkNodeStatus(mgr, bpath, ifp, VNodeState.UP, VNodeState.UP, pmconf.toString());
            }
        }

        st = mgr.setPortMap(ifp, null);
        assertTrue(st.isSuccess());

        PortMap map = null;
        try {
            map = mgr.getPortMap(ifp);
        } catch (Exception e) {
            unexpected(e);
        }
        assertNull(map);

        st = mgr.removeTenant(tpath);
        assertTrue(st.isSuccess());

        // set mutli portmaps to a vbridge
        String bname1 = "vbridge1";
        VBridgePath bpath1 = new VBridgePath(tname, bname1);
        String bname2 = "vbridge2";
        VBridgePath bpath2 = new VBridgePath(tname, bname2);
        String ifname1 = "vinterface1";
        VBridgeIfPath ifp1 = new VBridgeIfPath(tname, bname1, ifname1);
        String ifname2 = "vinterface2";
        VBridgeIfPath ifp2 = new VBridgeIfPath(tname, bname1, ifname2);

        List<VBridgePath> mbpathlist = new ArrayList<VBridgePath>();
        List<VBridgeIfPath> mifplist = new ArrayList<VBridgeIfPath>();
        mbpathlist.add(bpath1);
        mbpathlist.add(bpath2);
        mifplist.add(ifp1);
        mifplist.add(ifp2);
        createTenantAndBridgeAndInterface(mgr, tpath, mbpathlist, mifplist);

        Node node1 = NodeCreator.createOFNode(0L);
        SwitchPort port1 = new SwitchPort(NodeConnector.NodeConnectorIDType.OPENFLOW, "10");
        PortMapConfig pmconf1 = new PortMapConfig(node1, port1, (short) 0);
        st = mgr.setPortMap(ifp1, pmconf1);
        assertTrue(st.isSuccess());

        SwitchPort port2 = new SwitchPort(NodeConnector.NodeConnectorIDType.OPENFLOW, "11");
        PortMapConfig pmconf2 = new PortMapConfig(node1, port2, (short) 0);
        st = mgr.setPortMap(ifp2, pmconf2);
        assertTrue(st.isSuccess());

        // set duplicate portmap.
        String ifname3 = "vinterface3";
        VBridgeIfPath ifp3 = new VBridgeIfPath(tname, bname2, ifname3);
        st = mgr.addBridgeInterface(ifp3, new VInterfaceConfig(null, true));
        st = mgr.setPortMap(ifp3, pmconf1);
        assertEquals(StatusCode.CONFLICT, st.getCode());

        // add a vlan map to this VTN.
        testVlanMapSingle(mgr, bpath1);

        // remove test settings.
        st = mgr.removeTenant(tpath);
        assertTrue(st.isSuccess());
    }

    /**
     * Common routine for {@code testVlanMap} and {@code testPortMap}
     *
     * @param mgr   a VTNManagerImpl.
     * @param bpath a VBridgePath.
     */
    private void testVlanMapSingle (VTNManagerImpl mgr, VBridgePath bpath) {
        short[] vlans = new short[] { 0, 10, 4095 };

        for (Node vnode : createNodes(3)) {
            for (short vlan : vlans) {
                VlanMapConfig vlconf = new VlanMapConfig(vnode, vlan);
                VlanMap vmap = null;
                try {
                    vmap = mgr.addVlanMap(bpath, vlconf);
                    if (vnode != null && vnode.getType() != NodeConnector.NodeConnectorIDType.OPENFLOW) {
                        fail("throwing Exception was expected.");
                    }
                } catch (VTNException e) {
                    if (vnode != null && vnode.getType() == NodeConnector.NodeConnectorIDType.OPENFLOW) {
                        unexpected(e);
                    } else {
                     // if node type is not OF, throwing a VTNException is expected.
                        continue;
                    }
                }

                VlanMap getmap = null;
                try {
                    getmap = mgr.getVlanMap(bpath, vmap.getId());
                } catch (VTNException e) {
                    unexpected(e);
                }
                assertEquals(vlconf.toString(), getmap.getId(), vmap.getId());
                assertEquals(vlconf.toString(), getmap.getNode(), vnode);
                assertEquals(vlconf.toString(), getmap.getVlan(), vlan);

                VBridge brdg = null;
                try {
                    brdg = mgr.getBridge(bpath);
                } catch (VTNException e) {
                   unexpected(e);
                }
                assertEquals(vlconf.toString(), VNodeState.UP, brdg.getState());

                Status st = mgr.removeVlanMap(bpath, vmap.getId());
                assertTrue(vlconf.toString(), st.isSuccess());
            }
        }
    }


    /**
     * Test method for invalid cases of
     * {@link VTNManagerImpl#getPortMap(VBridgeIfPath)} and
     * {@link VTNManagerImpl#setPortMap(VBridgeIfPath, PortMapConfig)}.
     */
    @Test
    public void testPortMapInvalidCase() {
        VTNManagerImpl mgr = vtnMgr;
        String tname = "vtn";
        VTenantPath tpath = new VTenantPath(tname);
        String bname = "vbridge";
        VBridgePath bpath = new VBridgePath(tname, bname);
        String ifname = "vinterface";
        VBridgeIfPath ifp = new VBridgeIfPath(tname, bname, ifname);

        List<VBridgePath> bpathlist = new ArrayList<VBridgePath>();
        List<VBridgeIfPath> ifplist = new ArrayList<VBridgeIfPath>();
        bpathlist.add(bpath);
        ifplist.add(ifp);
        createTenantAndBridgeAndInterface(mgr, tpath, bpathlist, ifplist);

        Node node = NodeCreator.createOFNode(0L);
        SwitchPort port = new SwitchPort("port-1", NodeConnector.NodeConnectorIDType.OPENFLOW, "1");
        PortMapConfig pmconf = new PortMapConfig(node, port, (short) 0);
        Status st = mgr.setPortMap(ifp, pmconf);
        assertTrue(st.isSuccess());

        // bad request
        VBridgeIfPath[] biflist = new VBridgeIfPath[] {
                null, new VBridgeIfPath(tname, bname, null),
                new VBridgeIfPath(tname, null, ifname),
                new VBridgeIfPath(null, bname, ifname)};
        SwitchPort[] badports = new SwitchPort[] {
                new SwitchPort("port-10", NodeConnector.NodeConnectorIDType.OPENFLOW, null),
                new SwitchPort("port-11", NodeConnector.NodeConnectorIDType.OPENFLOW, "10"),
                new SwitchPort(null, null, "16"),
                new SwitchPort(NodeConnector.NodeConnectorIDType.ONEPK, "10"),
        };
        PortMapConfig[] bpmlist = new PortMapConfig[] {
                new PortMapConfig(null, port, (short) 0),
                new PortMapConfig(node, null, (short) 0),
                new PortMapConfig(node, null, (short) 0),
                new PortMapConfig(node, port, (short)-1),
                new PortMapConfig(node, port, (short)4096)};

        for (VBridgeIfPath path: biflist) {
            st = mgr.setPortMap(path, pmconf);
            assertEquals((path == null) ? "null" : path.toString(),
                    StatusCode.BADREQUEST, st.getCode());

            try {
                mgr.getPortMap(path);
                fail("throwing exception was expected.");
            } catch (VTNException e) {
                assertEquals((path == null) ? "null" : path.toString(),
                        StatusCode.BADREQUEST, e.getStatus().getCode());
            }
        }

        for (SwitchPort sw: badports) {
            mgr.setPortMap(ifp, new PortMapConfig(node, sw, (short)0));
            assertEquals(sw.toString(), StatusCode.BADREQUEST, st.getCode());
        }

        for (PortMapConfig map: bpmlist) {
            st = mgr.setPortMap(ifp, map);
            assertEquals(map.toString(), StatusCode.BADREQUEST, st.getCode());
        }

        Node pnode = null;
        try {
            pnode = new Node(Node.NodeIDType.PRODUCTION, "Node ID: 0");
        } catch (ConstructionException e1) {
            unexpected(e1);
        }
        st = mgr.setPortMap(ifp, new PortMapConfig(pnode, new SwitchPort("port-1"), (short)10));
        assertEquals(StatusCode.BADREQUEST, st.getCode());

        // not found
        VBridgeIfPath[] niflist = new VBridgeIfPath[] {
                new VBridgeIfPath(tname, bname, "ii"),
                new VBridgeIfPath(tname, "bbb", ifname),
                new VBridgeIfPath("vv", bname, ifname)};

        for (VBridgeIfPath path: niflist) {
            st = mgr.setPortMap(path, pmconf);
            assertEquals(path.toString(), StatusCode.NOTFOUND, st.getCode());

            try {
                mgr.getPortMap(path);
                fail("throwing exception was expected.");
            } catch (VTNException e) {
                assertEquals(path.toString(), StatusCode.NOTFOUND, e.getStatus().getCode());
            }
        }

        // TODO: NOTACCEPTABLE

        st = mgr.removeTenant(tpath);
        assertTrue(st.isSuccess());
    }

    /**
     * Test method for
     * {@link VTNManagerImpl#notifyNode(Node, UpdateType, Map)},
     * {@link VTNManagerImpl#notifyNodeConnector(NodeConnector, UpdateType, Map)},
     * {@link VTNManagerImpl#edgeUpdate(java.util.List)}
     */
    @Test
    public void testNotifyNodeAndNodeConnectorWithPortMap() {
        VTNManagerImpl mgr = vtnMgr;
        short[] vlans = new short[] { 0, 10, 4095 };
        Status st = null;
        String tname = "vtn";
        VTenantPath tpath = new VTenantPath(tname);
        String bname = "vbridge";
        VBridgePath bpath = new VBridgePath(tname, bname);
        String ifname = "vinterface";
        VBridgeIfPath ifp = new VBridgeIfPath(tname, bname, ifname);

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
        Node onode = NodeCreator.createOFNode(1L);
        SwitchPort[] ports = new SwitchPort[] {
                new SwitchPort("port-10", NodeConnector.NodeConnectorIDType.OPENFLOW, "10"),
                new SwitchPort(null, NodeConnector.NodeConnectorIDType.OPENFLOW, "11"),
                new SwitchPort("port-10", null, null),
                new SwitchPort("port-10"),
                new SwitchPort(NodeConnector.NodeConnectorIDType.OPENFLOW, "13"),
        };

        NodeConnector otherNc
            = NodeConnectorCreator.createOFNodeConnector(Short.valueOf((short)10), onode);

        for (SwitchPort port: ports) {
            for (short vlan : vlans) {
                PortMapConfig pmconf = new PortMapConfig(node, port, (short)vlan);
                st = mgr.setPortMap(ifp, pmconf);
                assertTrue(pmconf.toString(), st.isSuccess());

                PortMap map = null;
                try {
                    map = mgr.getPortMap(ifp);
                } catch (Exception e) {
                    unexpected(e);
                }
                assertEquals(pmconf, map.getConfig());
                assertNotNull(map.getNodeConnector());
                if(port.getId() != null) {
                    assertEquals(pmconf.toString(),
                            Short.parseShort(port.getId()), map.getNodeConnector().getID());
                }
                if (port.getType() != null) {
                    assertEquals(pmconf.toString(),
                            port.getType(), map.getNodeConnector().getType());
                }
                checkNodeStatus(mgr, bpath, ifp, VNodeState.UP, VNodeState.UP, pmconf.toString());

                // test for nodeconnector change notify.
                Map<String, Property> propMap = null; // not used now.

                putMacTableEntry(mgr, bpath, map.getNodeConnector());

                mgr.notifyNodeConnector(map.getNodeConnector(), UpdateType.REMOVED, propMap);
                checkNodeStatus(mgr, bpath, ifp, VNodeState.DOWN, VNodeState.DOWN, pmconf.toString());

                checkMacTableEntry(mgr, bpath, true, pmconf.toString());

                mgr.notifyNodeConnector(map.getNodeConnector(), UpdateType.ADDED, propMap);
                checkNodeStatus(mgr, bpath, ifp, VNodeState.UP, VNodeState.UP, pmconf.toString());

                mgr.notifyNodeConnector(map.getNodeConnector(), UpdateType.CHANGED, propMap);
                checkNodeStatus(mgr, bpath, ifp, VNodeState.UP, VNodeState.UP, pmconf.toString());

                mgr.notifyNodeConnector(otherNc, UpdateType.REMOVED, propMap);
                checkNodeStatus(mgr, bpath, ifp, VNodeState.UP, VNodeState.UP, pmconf.toString());

                mgr.notifyNodeConnector(otherNc, UpdateType.ADDED, propMap);
                checkNodeStatus(mgr, bpath, ifp, VNodeState.UP, VNodeState.UP, pmconf.toString());

                // test for node change notify.
                putMacTableEntry(mgr, bpath, map.getNodeConnector());

                mgr.notifyNode(node, UpdateType.REMOVED, propMap);
                checkNodeStatus(mgr, bpath, ifp, VNodeState.DOWN, VNodeState.DOWN, pmconf.toString());

                checkMacTableEntry(mgr, bpath, true, pmconf.toString());

                mgr.notifyNode(node, UpdateType.ADDED, propMap);
                checkNodeStatus(mgr, bpath, ifp, VNodeState.DOWN, VNodeState.DOWN, pmconf.toString());

                mgr.notifyNodeConnector(map.getNodeConnector(), UpdateType.ADDED, propMap);
                checkNodeStatus(mgr, bpath, ifp, VNodeState.UP, VNodeState.UP, pmconf.toString());

                mgr.notifyNode(node, UpdateType.CHANGED, propMap);
                checkNodeStatus(mgr, bpath, ifp, VNodeState.UP, VNodeState.UP, pmconf.toString());

                mgr.notifyNode(onode, UpdateType.REMOVED, propMap);
                checkNodeStatus(mgr, bpath, ifp, VNodeState.UP, VNodeState.UP, pmconf.toString());

                mgr.notifyNode(onode, UpdateType.ADDED, propMap);
                checkNodeStatus(mgr, bpath, ifp, VNodeState.UP, VNodeState.UP, pmconf.toString());

                // TODO: test for edge change notify
            }
        }

        st = mgr.removeTenant(tpath);
        assertTrue(st.isSuccess());
    }

    /**
     * same test with {@code testNotifyNodeAndNodeConnectorWithPortMap}.
     * this test case do with vlan map setting.
     */
    @Test
    public void testNotifyNodeAndNodeConnectorWithVlanMap() {
        VTNManagerImpl mgr = vtnMgr;
        short[] vlans = new short[] { 0, 10, 4095 };

        String tname = "vtn";
        VTenantPath tpath = new VTenantPath(tname);
        String bname = "vbridge";
        VBridgePath bpath = new VBridgePath(tname, bname);
        String ifname = "vinterface";
        VBridgeIfPath ifp = new VBridgeIfPath(tname, bname, ifname);
        Status st = null;

        List<VBridgePath> bpathlist = new ArrayList<VBridgePath>();
        List<VBridgeIfPath> ifplist = new ArrayList<VBridgeIfPath>();
        bpathlist.add(bpath);
        ifplist.add(ifp);
        createTenantAndBridgeAndInterface(mgr, tpath, bpathlist, ifplist);

        Node cnode = NodeCreator.createOFNode(0L);
        Node onode = NodeCreator.createOFNode(1L);
        NodeConnector nc
            = NodeConnectorCreator.createOFNodeConnector(Short.valueOf((short)10), cnode);
        NodeConnector otherNc
            = NodeConnectorCreator.createOFNodeConnector(Short.valueOf((short)10), onode);

        // add a vlanmap to a vbridge
        for (Node node : createNodes(2)) {
            for (short vlan : vlans) {
                VlanMapConfig vlconf = new VlanMapConfig(node, vlan);
                VlanMap map = null;
                try {
                    map = mgr.addVlanMap(bpath, vlconf);
                    if (node != null && node.getType() != NodeConnector.NodeConnectorIDType.OPENFLOW) {
                        fail("throwing Exception was expected.");
                    }
                } catch (VTNException e) {
                    if (node != null && node.getType() == NodeConnector.NodeConnectorIDType.OPENFLOW) {
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
                assertEquals(getmap.getId(), map.getId());
                assertEquals(getmap.getNode(), node);
                assertEquals(getmap.getVlan(), vlan);
                checkNodeStatus(mgr, bpath, ifp, VNodeState.UP, VNodeState.UNKNOWN, vlconf.toString());

                // test for node connector change notify.
                Map<String, Property> propMap = null; // not used now.

                putMacTableEntry(mgr, bpath, nc);

                mgr.notifyNodeConnector(nc, UpdateType.REMOVED, propMap);
                checkNodeStatus(mgr, bpath, ifp, VNodeState.UP, VNodeState.UNKNOWN, vlconf.toString());

                checkMacTableEntry(mgr, bpath, true, vlconf.toString());

                mgr.notifyNodeConnector(nc, UpdateType.ADDED, propMap);
                checkNodeStatus(mgr, bpath, ifp, VNodeState.UP, VNodeState.UNKNOWN, vlconf.toString());

                mgr.notifyNodeConnector(nc, UpdateType.CHANGED, propMap);
                checkNodeStatus(mgr, bpath, ifp, VNodeState.UP, VNodeState.UNKNOWN, vlconf.toString());

                mgr.notifyNodeConnector(otherNc, UpdateType.REMOVED, propMap);
                checkNodeStatus(mgr, bpath, ifp, VNodeState.UP, VNodeState.UNKNOWN, vlconf.toString());

                mgr.notifyNodeConnector(otherNc, UpdateType.ADDED, propMap);
                checkNodeStatus(mgr, bpath, ifp, VNodeState.UP, VNodeState.UNKNOWN, vlconf.toString());

                // test for node change notify.
                putMacTableEntry(mgr, bpath, nc);
                mgr.notifyNode(cnode, UpdateType.REMOVED, propMap);
                checkNodeStatus(mgr, bpath, ifp, (node == null) ? VNodeState.UP : VNodeState.DOWN,
                        VNodeState.UNKNOWN, vlconf.toString());

               checkMacTableEntry(mgr, bpath, true, vlconf.toString());

                mgr.notifyNode(cnode, UpdateType.ADDED, propMap);
                checkNodeStatus(mgr, bpath, ifp, VNodeState.UP, VNodeState.UNKNOWN, vlconf.toString());

//                mgr.notifyNodeConnector(nc, UpdateType.ADDED, propMap);
//                checkNodeStatus(mgr, bpath, ifp, VNodeState.UP, VNodeState.UP, vlconf.toString());

                mgr.notifyNode(cnode, UpdateType.CHANGED, propMap);
                checkNodeStatus(mgr, bpath, ifp, VNodeState.UP, VNodeState.UNKNOWN, vlconf.toString());

                mgr.notifyNode(onode, UpdateType.REMOVED, propMap);
                checkNodeStatus(mgr, bpath, ifp, VNodeState.UP, VNodeState.UNKNOWN, vlconf.toString());

                mgr.notifyNode(onode, UpdateType.ADDED, propMap);
                checkNodeStatus(mgr, bpath, ifp, VNodeState.UP, VNodeState.UNKNOWN, vlconf.toString());

                // TODO: test for edge change notify

                st = mgr.removeVlanMap(bpath, map.getId());
                assertTrue(st.isSuccess());
            }
        }

        st = mgr.removeTenant(tpath);
        assertTrue(st.isSuccess());
    }

    /**
     * check node and interface status specified vbridge and vinterface.
     * @param mgr   VTNManagerImpl
     * @param bpath checked VBridgePath
     * @param ifp   checked VBridgeIfPath
     * @param bstate expected vbridge state
     * @param ifstate expected vinterface state
     * @param msg message strings print when assertion failed.
     */
    private void checkNodeStatus(VTNManagerImpl mgr, VBridgePath bpath, VBridgeIfPath ifp,
            VNodeState bstate, VNodeState ifstate, String msg) {

        VBridge brdg = null;
        VInterface bif = null;
        try {
            bif = mgr.getBridgeInterface(ifp);
            brdg = mgr.getBridge(bpath);
        } catch (VTNException e) {
            unexpected(e);
        }

        assertEquals(msg, bstate, brdg.getState());
        assertEquals(msg, ifstate, bif.getState());
    }

    /**
     * put a Mac Address Table Entry to Mac Address Table of specified bridge.
     *
     * @param mgr   VTNManagerImpl
     * @param bpath VBridgePath
     * @param nc    NodeConnector
     */
    private void putMacTableEntry(VTNManagerImpl mgr, VBridgePath bpath, NodeConnector nc) {
        byte[] src = new byte[] {(byte)0x00, (byte)0x01, (byte)0x01, (byte)0x01, (byte)0x01, (byte)0x01,};
        byte[] dst = new byte[] {(byte)0xFF, (byte)0xFF, (byte)0xFF, (byte)0xFF, (byte)0xFF, (byte)0xFF};
        byte[] sender = new byte[] {(byte)192, (byte)168, (byte)0, (byte)1};
        byte[] target = new byte[] {(byte)192, (byte)168, (byte)0, (byte)250};

        PacketContext pctx = createARPPacketContext(src, dst, sender ,target, (short)0, nc, ARP.REQUEST);
        MacAddressTable tbl = mgr.getMacAddressTable(bpath);
        tbl.add(mgr, pctx);
    }

    /**
     * check a Mac Address Table Entry.
     *
     * @param mgr   VTNManagerImpl
     * @param bpath VBridgePath
     * @param isFlushed if true, expected result is 0. if not 0, execpted result is more than 0.
     * @param msg
     */
    private void checkMacTableEntry(VTNManagerImpl mgr, VBridgePath bpath, boolean isFlushed, String msg) {
        MacAddressTable tbl = mgr.getMacAddressTable(bpath);

        List<MacAddressEntry> list = null;
        try {
            list = tbl.getEntries();
        } catch (VTNException e) {
           unexpected(e);
        }

        if (isFlushed) {
            assertEquals(msg, 0, list.size());
        } else {
            assertTrue(msg, list.size() > 0);
        }
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
        short[] vlans = new short[] { 0, 10, 4095 };
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
                st = mgr.addBridgeInterface(ifp, new VInterfaceConfig(null, true));
                assertTrue(ifp.toString(), st.isSuccess());
                inode++;
            }
        }

        // null case
        PacketResult result = mgr.receiveDataPacket(null);
        assertEquals(PacketResult.IGNORED, result);

        for (short vlan: vlans) {
            // add port mapping
            for (short i = 0; i < 4; i++) {
                int inode = 0;
                for (Node node : existNodes) {
                    String ifname = "vinterface" + inode + (i + 10);
                    VBridgeIfPath ifp = new VBridgeIfPath(tname, (i < 2) ? bname1 : bname2,
                                                            ifname);

                    SwitchPort port = new SwitchPort(NodeConnector.NodeConnectorIDType.OPENFLOW,
                                                    String.valueOf(i + 10));
                    PortMapConfig pmconf = new PortMapConfig(node, port, vlan);
                    st = mgr.setPortMap(ifp, pmconf);
                    assertTrue(ifp.toString() + "," + pmconf.toString(), st.isSuccess());

                    NodeConnector mapnc = NodeConnectorCreator.createOFNodeConnector(
                                                        Short.valueOf((short)(i + 10)), node);
                    Set<NodeConnector> set  = mappedConnectors.get((i < 2)? bpath1 : bpath2);
                    set.add(mapnc);
                    mappedConnectors.put((i < 2)? bpath1 : bpath2, set);
                    inode++;
                }
            }

            Set<NodeConnector> mappedThis = new HashSet<NodeConnector>(mappedConnectors.get(bpath1));
            Set<NodeConnector> noMappedThis = new HashSet<NodeConnector>(existConnectors);
            noMappedThis.removeAll(mappedConnectors.get(bpath1));

            testReceiveDataPacketBCLoop(mgr, bpath1, MapType.PORT, vlan,
                                        mappedThis, noMappedThis, stub);

            st = mgr.flushMacEntries(bpath1);
            assertTrue("vlan=" + vlan, st.isSuccess());

            testReceiveDataPacketUCLoopLearned(mgr, bpath1, MapType.PORT, vlan,
                    mappedThis, noMappedThis, stub);

            st = mgr.flushMacEntries(bpath1);
            assertTrue("vlan=" + vlan, st.isSuccess());

            testReceiveDataPacketUCLoop(mgr, bpath1, MapType.PORT, vlan,
                    mappedThis, noMappedThis, stub);

            st = mgr.flushMacEntries(bpath1);
            assertTrue("vlan=" + vlan, st.isSuccess());

            testReceiveDataPacketUCLoopLearned(mgr, bpath1, MapType.PORT, vlan,
                    mappedThis, noMappedThis, stub);


            // in case recieved a packet from controller.
            for (NodeConnector nc: mappedThis) {
                byte [] src = new byte[] {cntMac[0], cntMac[1], cntMac[2],
                                        cntMac[3], cntMac[4], cntMac[5]};
                byte [] dst = new byte[] {(byte)0xff, (byte)0xff, (byte)0xff,
                                        (byte)0xff, (byte)0xff, (byte)0xff};
                byte [] sender = new byte[] {(byte)192, (byte)168, (byte)0, (byte)1};
                byte [] target = new byte[] {(byte)192, (byte)168, (byte)0, (byte)250};

                RawPacket inPkt = createARPRawPacket(src, dst, sender, target, vlan, nc, ARP.REQUEST);
                result = mgr.receiveDataPacket(inPkt);
                assertEquals(nc.toString(), PacketResult.IGNORED, result);

                st = mgr.flushMacEntries(bpath1);
                assertTrue(nc.toString(), st.isSuccess());
            }
        }

        st = mgr.removeBridge(bpath2);
        assertTrue(st.isSuccess());

        // in case received pacekt from no mapped port.
        for (NodeConnector nc: mappedConnectors.get(bpath2)) {
            byte iphost = 1;
            for (EthernetAddress ea: createEthernetAddresses(false)) {
                byte [] bytes = ea.getValue();
                byte [] src = new byte[] {bytes[0], bytes[1], bytes[2],
                                        bytes[3], bytes[4], bytes[5]};
                byte [] dst = new byte[] {(byte)0xff, (byte)0xff, (byte)0xff,
                                        (byte)0xff, (byte)0xff, (byte)0xff};
                byte [] sender = new byte[] {(byte)192, (byte)168, (byte)0, (byte)iphost};
                byte [] target = new byte[] {(byte)192, (byte)168, (byte)0, (byte)250};
                short vlan = -1;
                RawPacket inPkt = createARPRawPacket(src, dst, sender, target, vlan, nc, ARP.REQUEST);
                Ethernet inPktDecoded = (Ethernet)stub.decodeDataPacket(inPkt);

                result = mgr.receiveDataPacket(inPkt);
                if (nc != null) {
                    assertEquals(nc.toString() + "," + ea.toString(), PacketResult.IGNORED, result);
                } else {
                    assertEquals(ea.toString(), PacketResult.IGNORED, result);
                }

                List<RawPacket> transDatas = stub.getTransmittedDataPacket();
                assertEquals(nc.toString() + "," + ea.toString(), 0, transDatas.size());
                iphost++;
            }
        }

        st = mgr.removeTenant(tpath);
        assertTrue(st.isSuccess());
    }

    /**
     * Test method for {@link VTNManagerImpl#receiveDataPacket(RawPacket)}.
     * The case VlanMap applied to vBridge.
     */
    @Test
    public void testReceiveDataPacketVlanMapped() {
        VTNManagerImpl mgr = vtnMgr;
        TestStub stub = stubObj;
        ISwitchManager swmgr = mgr.getSwitchManager();
        short[] vlans = new short[] { 0, 10, 4095 };
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

        for (short vlan : vlans) {
            VlanMapConfig vlconf = new VlanMapConfig(null, vlan);
            VlanMap map = null;
            try {
                map = mgr.addVlanMap(bpath1, vlconf);
            } catch (VTNException e) {
                unexpected(e);
            }

            Set<NodeConnector> set = new HashSet<NodeConnector>(existConnectors);
            mappedConnectors.put(bpath1, set);

            Set<NodeConnector> mappedThis = new HashSet<NodeConnector>(mappedConnectors.get(bpath1));
            Set<NodeConnector> noMappedThis = new HashSet<NodeConnector>(existConnectors);
            noMappedThis.removeAll(mappedConnectors.get(bpath1));

            testReceiveDataPacketBCLoop(mgr, bpath1, MapType.PORT, vlan,
                    mappedThis, noMappedThis, stub);

            st = mgr.flushMacEntries(bpath1);
            assertTrue(vlconf.toString(), st.isSuccess());

            testReceiveDataPacketUCLoopLearned(mgr, bpath1, MapType.PORT, vlan,
                    mappedThis, noMappedThis, stub);

            st = mgr.flushMacEntries(bpath1);
            assertTrue(vlconf.toString(), st.isSuccess());

            testReceiveDataPacketUCLoop(mgr, bpath1, MapType.PORT, vlan,
                    mappedThis, noMappedThis, stub);

            st = mgr.flushMacEntries(bpath1);
            assertTrue(vlconf.toString(), st.isSuccess());

            testReceiveDataPacketUCLoopLearned(mgr, bpath1, MapType.VLAN, vlan,
                    mappedThis, noMappedThis, stub);

            mappedConnectors.replace(bpath1, new HashSet<NodeConnector>());
            mappedConnectors.replace(bpath2, new HashSet<NodeConnector>());

            st = mgr.removeVlanMap(bpath1, map.getId());
            assertTrue(vlconf.toString(), st.isSuccess());

            st = mgr.flushMacEntries(bpath1);
            assertTrue(vlconf.toString(), st.isSuccess());
        }

        st = mgr.removeTenant(tpath);
        assertTrue(st.isSuccess());
    }

    /**
     * Test method for {@link VTNManagerImpl#receiveDataPacket(RawPacket)}.
     * The case both VlanMap and PortMap applied to vBridge.
     */
    @Test
    public void testReceiveDataPacketBothMapped() {
        // TODO:
    }

    /**
     * Test method for
     * {@link VTNManagerImpl#find(InetAddress)},
     * {@link internal.VTNManagerImpl#findHost(InetAddress, Set)},
     * {@link VTNManagerImpl#probe(HostNodeConnector)},
     * {@link VTNManagerImpl#probeHost(HostNodeConnector)}
     */
    @Test
    public void testFindProbe() {
        VTNManagerImpl mgr = vtnMgr;
        TestStub stub = stubObj;
        ISwitchManager swmgr = mgr.getSwitchManager();
        short[] vlans = new short[] { 0, 10, 4095 };
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
                st = mgr.addBridgeInterface(ifp, new VInterfaceConfig(null, true));
                assertTrue(ifp.toString(), st.isSuccess());
                inode++;
            }
        }

        InetAddress ia6 = null;
        try {
            ia6 = InetAddress.getByAddress(new byte[]{
                    (byte)0x20, (byte)0x01, (byte)0x04, (byte)0x20,
                    (byte)0x02, (byte)0x81, (byte)0x10, (byte)0x04,
                    (byte)0x0e1, (byte)0x23, (byte)0xe6, (byte)0x88,
                    (byte)0xd6, (byte)0x55, (byte)0xa1, (byte)0xb0});
        } catch (UnknownHostException e) {
            unexpected(e);
        }

        for (short vlan: vlans) {
            // add port mapping
            for (short i = 0; i < 4; i++) {
                int inode = 0;
                for (Node node : existNodes) {
                    String ifname = "vinterface" + inode + (i + 10);
                    VBridgeIfPath ifp = new VBridgeIfPath(tname, (i < 2) ? bname1 : bname2,
                                                            ifname);
                    SwitchPort port = new SwitchPort(NodeConnector.NodeConnectorIDType.OPENFLOW,
                                                    String.valueOf(i + 10));
                    PortMapConfig pmconf = new PortMapConfig(node, port, vlan);
                    st = mgr.setPortMap(ifp, pmconf);
                    assertTrue(ifp.toString() + "," + pmconf.getVlan(), st.isSuccess());

                    NodeConnector mapnc = NodeConnectorCreator.createOFNodeConnector(
                                                        Short.valueOf((short)(i + 10)), node);
                    Set<NodeConnector> set  = mappedConnectors.get((i < 2)? bpath1 : bpath2);
                    set.add(mapnc);
                    mappedConnectors.put((i < 2)? bpath1 : bpath2, set);
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

            InetAddress ia = null;
            try {
                ia = InetAddress.getByAddress(new byte[] {
                                     (byte)10, (byte)0, (byte)0, (byte)1});
            } catch (UnknownHostException e) {
                unexpected(e);
            }

            mgr.findHost(null, bpaths1);
            mgr.findHost(ia6, bpaths1);

            for (Set<VBridgePath> bpaths : bpathset) {
                int numMapped = 0;
                Set<NodeConnector> mappedThis = new HashSet<NodeConnector>();
                if (bpaths == null || bpaths.contains(bpath1)) {
                    mappedThis.addAll(mappedConnectors.get(bpath1));
                    numMapped += mappedConnectors.get(bpath1).size();
                }
                if (bpaths == null || bpaths.contains(bpath2)) {
                    mappedThis.addAll(mappedConnectors.get(bpath2));
                    numMapped += mappedConnectors.get(bpath2).size();
                }
                Set<NodeConnector> noMappedThis = new HashSet<NodeConnector>(existConnectors);
                noMappedThis.removeAll(mappedThis);

                // test find()
                if (bpaths == null) {
                    mgr.find(ia);
                } else {
                    mgr.findHost(ia, bpaths);
                }

                List<RawPacket> transDatas = stub.getTransmittedDataPacket();
                assertEquals(numMapped, transDatas.size());

                for (RawPacket raw : transDatas) {
                    Packet pkt = stub.decodeDataPacket(raw);
                    assertTrue(raw.getOutgoingNodeConnector().toString(),
                        mappedThis.contains(raw.getOutgoingNodeConnector()));
                    assertFalse(raw.getOutgoingNodeConnector().toString(),
                        noMappedThis.contains(raw.getOutgoingNodeConnector()));

                    Ethernet eth = (Ethernet)pkt;

                    checkOutEthernetPacket("vlan=" + vlan, eth, EtherTypes.ARP,
                            swmgr.getControllerMAC(), new byte[] {-1, -1, -1, -1, -1, -1},
                            vlan, EtherTypes.IPv4, ARP.REQUEST,
                            swmgr.getControllerMAC(), new byte[] {0, 0, 0, 0, 0, 0},
                            new byte[] {0, 0, 0, 0}, ia.getAddress());
                }
            }

            // probe()
            for (Node node: existNodes) {
                byte [] mac = new byte [] { 0x00, 0x00, 0x00, 0x11, 0x22, 0x33};
                NodeConnector nc = NodeConnectorCreator.createOFNodeConnector(Short.valueOf("10"), node);
                HostNodeConnector hnode = null;
                try {
                    hnode = new HostNodeConnector(mac, ia, nc, vlan);
                } catch (ConstructionException e) {
                    unexpected(e);
                }

                mgr.probe(hnode);

                List<RawPacket> transDatas = stub.getTransmittedDataPacket();
                assertEquals(node.toString(), 1, transDatas.size());

                RawPacket raw = transDatas.get(0);
                assertEquals(node.toString(), nc, raw.getOutgoingNodeConnector());

                Ethernet eth = (Ethernet)stub.decodeDataPacket(raw);
                checkOutEthernetPacket("vlan=" + vlan, eth, EtherTypes.ARP,
                        swmgr.getControllerMAC(), mac,
                        vlan, EtherTypes.IPv4, ARP.REQUEST,
                        swmgr.getControllerMAC(), mac,
                        new byte[] {0, 0, 0, 0}, ia.getAddress());
            }

            // if null case
            mgr.probe(null);
        }

        st = mgr.removeTenant(tpath);
        assertTrue(st.isSuccess());
    }


    // private methods.

    /**
     * common method for test which use BroadCast Packet (ARP Request packet).
     * this method expected that action for received packet is flooding to mapped port.
     */
    private void testReceiveDataPacketBCLoop(VTNManagerImpl mgr, VBridgePath bpath,
           MapType type, short vlan, Set<NodeConnector> mappedThis, Set<NodeConnector> noMappedThis,
           TestStub stub) {
        testReceiveDataPacketCommonLoop(mgr, bpath, MapType.PORT, vlan,
                mappedThis, noMappedThis, stub, (short)1, EtherTypes.ARP, null);
    }

    /**
     * common method for test which use UniCast Packet (IPv4 packet).
     * this method expected that action for received packet is flooding to mapped port.
     */
    private void testReceiveDataPacketUCLoop(VTNManagerImpl mgr, VBridgePath bpath,
            MapType type, short vlan, Set<NodeConnector> mappedThis, Set<NodeConnector> noMappedThis,
            TestStub stub) {
        testReceiveDataPacketCommonLoop(mgr, bpath, MapType.PORT, vlan,
                                mappedThis, noMappedThis, stub, (short)0, EtherTypes.IPv4, null);
    }

    /**
     * common method for
     * {@link testReceiveDataPacketBCLoop} and {@link testReceiveDataPacketUCLoop}.
     * this method expected that action for received packet is flooding to mapped port.
     */
    private void testReceiveDataPacketCommonLoop(VTNManagerImpl mgr, VBridgePath bpath,
            MapType type, short vlan, Set<NodeConnector> mappedThis, Set<NodeConnector> noMappedThis,
            TestStub stub, short bc, EtherTypes ethtype, NodeConnector targetnc) {
        ISwitchManager swmgr = mgr.getSwitchManager();
        byte[] cntMac = swmgr.getControllerMAC();

        for (NodeConnector nc: mappedThis) {
            if (targetnc != null && !targetnc.equals(nc)) {
                continue;
            }
            byte iphost = 1;
            for (EthernetAddress ea: createEthernetAddresses(false)) {
                byte [] bytes = ea.getValue();
                byte [] src = new byte[] {bytes[0], bytes[1], bytes[2],
                                       bytes[3], bytes[4], bytes[5]};
                byte [] dst;
                if (bc > 0) {
                    dst = new byte[] {(byte)0xff, (byte)0xff, (byte)0xff,
                                        (byte)0xff, (byte)0xff, (byte)0xff};
                } else {
                    dst = new byte[] {(byte)0x00, (byte)0x00, (byte)0x00,
                                        (byte)0xff, (byte)0xff, (byte)0x11};
                }
                byte [] sender = new byte[] {(byte)192, (byte)168, (byte)0, (byte)iphost};
                byte [] target = new byte[] {(byte)192, (byte)168, (byte)0, (byte)250};

                RawPacket inPkt = null;
                if (ethtype.shortValue() == EtherTypes.IPv4.shortValue()) {
                    inPkt = createIPv4RawPacket(src, dst, sender, target,
                                                    (vlan > 0) ? vlan : -1, nc);
                } else if (ethtype.shortValue() == EtherTypes.ARP.shortValue()){
                    inPkt = createARPRawPacket(src, dst, sender, target,
                            (vlan > 0) ? vlan : -1, nc, ARP.REQUEST);
                }
                Ethernet inPktDecoded = (Ethernet)stub.decodeDataPacket(inPkt);
                PacketResult result = mgr.receiveDataPacket(inPkt);

                if (nc != null &&
                    nc.getType() == NodeConnector.NodeConnectorIDType.OPENFLOW) {

                    assertEquals(nc.toString() + ea.toString(), PacketResult.KEEP_PROCESSING, result);

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
                        assertTrue(nc.toString() + ea.toString(),
                                entry.getInetAddresses().size() == 1);
                        assertArrayEquals(nc.toString() + "," + ea.toString(),
                                sender, entry.getInetAddresses().iterator().next().getAddress() );
                    } else {
                        assertTrue(nc.toString() + ea.toString(), entry.getInetAddresses().size() == 0);
                    }

                    List<RawPacket> transDatas = stub.getTransmittedDataPacket();

                    if (ethtype.shortValue() == EtherTypes.IPv4.shortValue()) {
                        // TODO: code is not implemented yet.
//                        assertEquals(nc.toString() + "," + ea.toString(),
//                                mappedThis.size(), transDatas.size());
                    } else {
                        assertEquals(nc.toString() + "," + ea.toString(),
                                mappedThis.size() - 1, transDatas.size());
                    }


                    for (RawPacket raw: transDatas) {
                        Packet pkt = stub.decodeDataPacket(raw);
                        assertTrue(nc.toString() + ea.toString() + raw.getOutgoingNodeConnector(),
                                mappedThis.contains(raw.getOutgoingNodeConnector()));
                        assertFalse(nc.toString() + ea.toString() + raw.getOutgoingNodeConnector(),
                                noMappedThis.contains(raw.getOutgoingNodeConnector()));
                        if (ethtype.shortValue() == EtherTypes.ARP.shortValue() ||
                            (ethtype.shortValue() == EtherTypes.IPv4.shortValue() &&
                             !raw.getOutgoingNodeConnector().equals(nc))) {
                            assertFalse(raw.getOutgoingNodeConnector().equals(nc));
                            assertEquals(nc.toString() + "," +  ea.toString(), inPktDecoded, pkt);

                            if (ethtype.shortValue() == EtherTypes.IPv4.shortValue()) {
                                checkOutEthernetPacketIPv4(nc.toString() + "," + ea.toString(),
                                        (Ethernet)pkt, EtherTypes.IPv4, src, dst, vlan);
                            } else if (ethtype.shortValue() == EtherTypes.ARP.shortValue()){
                                checkOutEthernetPacket(nc.toString() + "," + ea.toString(),
                                        (Ethernet)pkt, EtherTypes.ARP, src, dst, vlan,
                                        EtherTypes.IPv4, ARP.REQUEST, src, dst, sender, target);
                            } else {
                                fail("unexpected packet received.");
                            }
                        } else {
                            checkOutEthernetPacket(nc.toString() + "," + ea.toString(),
                                    (Ethernet)pkt, EtherTypes.ARP, cntMac, src, vlan,
                                    EtherTypes.IPv4, ARP.REQUEST, cntMac, src,
                                    new byte[] {0, 0, 0, 0}, sender);
                        }
                    }
                } else {
                    if (nc != null) {
                        assertEquals(nc.toString() + "," + ea.toString(), PacketResult.IGNORED, result);
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
           MapType type, short vlan, Set<NodeConnector> mappedThis, Set<NodeConnector> noMappedThis,
           TestStub stub) {
        testReceiveDataPacketCommonLoopLearned(mgr, bpath, MapType.PORT, vlan,
                mappedThis, noMappedThis, stub, (short)1, EtherTypes.ARP);
    }

    /**
     * common method for test which use UniCast Packet (IPv4 packet).
     * this method expected that action for received packet is output to learned port.
     */
    private void testReceiveDataPacketUCLoopLearned(VTNManagerImpl mgr, VBridgePath bpath,
            MapType type, short vlan, Set<NodeConnector> mappedThis, Set<NodeConnector> noMappedThis,
            TestStub stub) {
        testReceiveDataPacketCommonLoopLearned(mgr, bpath, MapType.PORT, vlan,
                                mappedThis, noMappedThis, stub, (short)0, EtherTypes.IPv4);
    }

    /**
     * common method for
     * {@link testReceiveDataPacketBCLoop} and {@link testReceiveDataPacketUCLoop}.
     * this method expected that action for received packet is a port host is connected to.
     *
     */
    private void testReceiveDataPacketCommonLoopLearned(VTNManagerImpl mgr, VBridgePath bpath,
            MapType type, short vlan, Set<NodeConnector> mappedThis, Set<NodeConnector> noMappedThis,
            TestStub stub, short bc, EtherTypes ethtype) {
        ISwitchManager swmgr = mgr.getSwitchManager();
        byte[] cntMac = swmgr.getControllerMAC();

        for (NodeConnector nc: mappedThis) {

            // first learned hosts to vbridge.
            testReceiveDataPacketCommonLoop(mgr, bpath, MapType.PORT, vlan,
                    mappedThis, noMappedThis, stub, (short)0, EtherTypes.IPv4, nc);

            // mac addresses have been learned at this point.
            for (NodeConnector innc: mappedThis) {
                byte tip = 1;
                boolean first = false;
                for (EthernetAddress ea: createEthernetAddresses(false)) {
                    byte [] bytes = ea.getValue();
                    byte [] src;
                    if (bc > 0) {
                        src = new byte[] {(byte)0xff, (byte)0xff, (byte)0xff,
                                            (byte)0xff, (byte)0xff, (byte)0xff};
                    } else {
                        src = new byte[] {(byte)0x00, (byte)0x00, (byte)0x00,
                                            (byte)0xff, (byte)0xff, (byte)0x11};
                    }
                    byte [] dst = new byte[] {bytes[0], bytes[1], bytes[2],
                                            bytes[3], bytes[4], bytes[5]};
                    byte [] sender = new byte[] {(byte)192, (byte)168, (byte)0, (byte)250};
                    byte [] target = new byte[] {(byte)192, (byte)168, (byte)0, (byte)tip};

                    RawPacket inPkt = null;
                    if (ethtype.shortValue() == EtherTypes.IPv4.shortValue()) {
                        inPkt = createIPv4RawPacket(src, dst, sender, target,
                                                        (vlan > 0) ? vlan : -1, innc);
                    } else if (ethtype.shortValue() == EtherTypes.ARP.shortValue()){
                        // not used case.
                    }

                    Ethernet inPktDecoded = (Ethernet)stub.decodeDataPacket(inPkt);
                    PacketResult result = mgr.receiveDataPacket(inPkt);

                    if (innc != null &&
                        innc.getType() == NodeConnector.NodeConnectorIDType.OPENFLOW) {
                        assertEquals(nc.toString() + ea.toString(), PacketResult.KEEP_PROCESSING, result);

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
                            assertTrue(nc.toString() + ea.toString(),
                                        entry.getInetAddresses().size() == 1);
                            assertArrayEquals(nc.toString() + "," + ea.toString(),
                                        sender, entry.getInetAddresses().iterator().next().getAddress() );
                        } else {
                            assertTrue(nc.toString() + ea.toString(), entry.getInetAddresses().size() == 0);
                        }

                        List<RawPacket> transDatas = stub.getTransmittedDataPacket();
                        if (!first) {
                            // TODO: code is not implemented yet.
//                            assertEquals(nc.toString() + "," + ea.toString(),
//                                    2, transDatas.size());
                            first = true;
                        } else {
                            assertEquals(nc.toString() + "," + ea.toString(),
                                    1, transDatas.size());
                        }

                        for (RawPacket raw : transDatas) {
                            raw = transDatas.get(0);
                            Packet pkt = stub.decodeDataPacket(raw);

                            Ethernet eth = (Ethernet)pkt;
                            short outEthType;
                            if (vlan > 0) {
                                IEEE8021Q vlantag = (IEEE8021Q)eth.getPayload();
                                outEthType = vlantag.getEtherType();
                            } else {
                                outEthType = eth.getEtherType();
                            }

                            if (outEthType == EtherTypes.ARP.shortValue() &&
                                    raw.getOutgoingNodeConnector().equals(innc)) {
                                // this is a packet to detect IP address of host
                                // which has no IP address.
                                checkOutEthernetPacket(nc.toString() + "," + ea.toString(),
                                    (Ethernet)pkt, EtherTypes.ARP, cntMac, src, vlan,
                                    EtherTypes.IPv4, ARP.REQUEST, cntMac, src,
                                    new byte[] {0, 0, 0, 0}, sender);
                            } else {
                                assertEquals(nc.toString() + "," +  ea.toString(),
                                        inPktDecoded, pkt);
                                checkOutEthernetPacketIPv4(nc.toString() + "," + ea.toString(),
                                    (Ethernet)pkt, EtherTypes.IPv4, src, dst, vlan);
                            }
                        }
                    } else {
                        if (innc != null) {
                            assertEquals(innc.toString() + "," + ea.toString(), PacketResult.IGNORED, result);
                        } else {
                            assertEquals(ea.toString(), PacketResult.IGNORED, result);
                        }
                    }
                    tip++;
                }
            }

            Status st = mgr.flushMacEntries(bpath);
            assertTrue(st.isSuccess());
        }
    }
}
