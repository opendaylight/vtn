/*
 * Copyright (c) 2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.net.InetAddress;
import java.util.ArrayList;
import java.util.HashSet;
import java.util.Hashtable;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ConcurrentMap;

import org.apache.felix.dm.impl.ComponentImpl;

import org.junit.Test;
import org.opendaylight.controller.sal.core.Node;
import org.opendaylight.controller.sal.core.NodeConnector;
import org.opendaylight.controller.sal.core.Property;
import org.opendaylight.controller.sal.core.UpdateType;
import org.opendaylight.controller.sal.packet.ARP;
import org.opendaylight.controller.sal.packet.Ethernet;
import org.opendaylight.controller.sal.packet.IEEE8021Q;
import org.opendaylight.controller.sal.packet.PacketResult;
import org.opendaylight.controller.sal.packet.RawPacket;
import org.opendaylight.controller.sal.utils.EtherTypes;
import org.opendaylight.controller.sal.utils.GlobalConstants;
import org.opendaylight.controller.sal.utils.NodeConnectorCreator;
import org.opendaylight.controller.sal.utils.NodeCreator;
import org.opendaylight.controller.sal.utils.Status;
import org.opendaylight.controller.switchmanager.ISwitchManager;
import org.opendaylight.controller.topologymanager.ITopologyManager;
import org.opendaylight.vtn.manager.IVTNManager;
import org.opendaylight.vtn.manager.PortMapConfig;
import org.opendaylight.vtn.manager.SwitchPort;
import org.opendaylight.vtn.manager.VBridgeConfig;
import org.opendaylight.vtn.manager.VBridgeIfPath;
import org.opendaylight.vtn.manager.VBridgePath;
import org.opendaylight.vtn.manager.VInterfaceConfig;
import org.opendaylight.vtn.manager.VTNException;
import org.opendaylight.vtn.manager.VTenantConfig;
import org.opendaylight.vtn.manager.VTenantPath;
import org.opendaylight.vtn.manager.VlanMap;
import org.opendaylight.vtn.manager.VlanMapConfig;
import org.opendaylight.vtn.manager.internal.cluster.ClusterEventId;
import org.opendaylight.vtn.manager.internal.cluster.PortVlan;
import org.opendaylight.vtn.manager.internal.cluster.RawPacketEvent;

/**
 * JUnit test for {@link VTNManagerImplTest}.
 *
 * <p>
 * Test a function disable some service for a newly detected node.
 * </p>
 */
public class VTNManagerImplDisableNodesTest extends TestBase {
    private VTNManagerImpl vtnMgr = null;
    private TestStub stubObj = null;
    private int stubMode = 2;

    /**
     * test case receive packet with existing entry in {@code disabledNode}.
     */
    @Test
    public void testWithHavingDisabledNode() {
        // set a value of EdgeTimeout to 600 sec.
        setupVTNManager(600000);

        VTNManagerImpl mgr = vtnMgr;
        ISwitchManager swMgr = mgr.getSwitchManager();
        ITopologyManager topoMgr = mgr.getTopologyManager();
        TestStub stub = stubObj;
        short[] vlans = new short[] { 0, 10, 4095 };
        Set<Node> nodeSet = new HashSet<Node>();

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

        ConcurrentMap<VBridgePath, Set<NodeConnector>> mappedConnectors
                = new ConcurrentHashMap<VBridgePath, Set<NodeConnector>>();
        mappedConnectors.put(bpath, new HashSet<NodeConnector>());

        Set<Node> existNodes = swMgr.getNodes();
        Set<NodeConnector> existConnectors = new HashSet<NodeConnector>();
        for (Node node : existNodes) {
            existConnectors.addAll(swMgr.getNodeConnectors(node));
        }

        Set<Node> vmapNodes = new HashSet<Node>(existNodes);
        vmapNodes.add(null);

        byte[] src = new byte[] { (byte) 0x00, (byte) 0x00, (byte) 0x00,
                                  (byte) 0x00, (byte) 0x00, (byte) 0x11 };
        byte[] dst = new byte[] { (byte) 0xff, (byte) 0xff, (byte) 0xff,
                                  (byte) 0xff, (byte) 0xff, (byte) 0xff };
        byte[] sender = new byte[] { (byte) 192, (byte) 168, (byte) 0, (byte) 1 };
        byte[] target
                = new byte[] { (byte) 192, (byte) 168, (byte) 0, (byte) 250 };

        Node pnode = NodeCreator.createOFNode(0L);
        SwitchPort port = new SwitchPort(
                NodeConnector.NodeConnectorIDType.OPENFLOW, String.valueOf(10));
        NodeConnector pmapnc = NodeConnectorCreator.createOFNodeConnector(
                 Short.valueOf((short) 10), pnode);
        Node disableNode = NodeCreator.createOFNode(0L);

        for (short vlan : vlans) {
            // add port mapping
            Set<PortVlan> portMappedThis = new HashSet<PortVlan>();
            Set<PortVlan> portMappedOther = new HashSet<PortVlan>();
            short pmapvlan = vlan;

            PortMapConfig pmconf = new PortMapConfig(pnode, port, pmapvlan);
            st = mgr.setPortMap(ifp, pmconf);
            assertTrue(st.isSuccess());

            Set<NodeConnector> set = mappedConnectors.get(bpath);
            set.add(pmapnc);
            mappedConnectors.put(bpath, set);
            portMappedThis.add(new PortVlan(pmapnc, pmapvlan));

            for (Node vlanMapNode : vmapNodes) {
                for (short vmapVlan : vlans) {
                    Set<PortVlan> mappedThis = new HashSet<PortVlan>();
                    Set<PortVlan> disabledPV = new HashSet<PortVlan>();

                    // add vlan mapping
                    VlanMapConfig vlconf = new VlanMapConfig(vlanMapNode,
                            vmapVlan);
                    VlanMap map = null;
                    try {
                        map = mgr.addVlanMap(bpath, vlconf);
                    } catch (VTNException e) {
                        unexpected(e);
                    }

                    for (Node node : existNodes) {
                        if (vlanMapNode == null || node.equals(vlanMapNode)) {
                            for (NodeConnector nc :
                                    swMgr.getNodeConnectors(node)) {
                                if (!topoMgr.isInternal(nc)) {
                                    mappedThis.add(new PortVlan(nc, vmapVlan));
                                    if (node.equals(disableNode)) {
                                        disabledPV.add(new PortVlan(nc,
                                                vmapVlan));
                                    }
                                }
                            }
                        }
                    }

                    mappedThis.addAll(portMappedThis);
                    mappedThis.removeAll(portMappedOther);

                    if (pmconf.getNode().equals(disableNode)) {
                        disabledPV.add(new PortVlan(pmapnc, pmapvlan));
                    }

                    // a result of adding Node, specified Node set to
                    // disableNodes.
                    addNode(mgr, swMgr, disableNode);

                    // EdgeTimeout is not infinity, but time is long enough and
                    // expected to return result before timeout.
                    for (PortVlan pv : mappedThis) {
                        String emsg = "vlan=" + vlan + "," + vlconf.toString()
                                        + "," + pv.toString();

                        RawPacket inPkt = createARPRawPacket(src, dst, sender,
                                target, (pv.getVlan() > 0) ? pv.getVlan() : -1,
                                pv.getNodeConnector(), ARP.REQUEST);
                        PacketResult result = mgr.receiveDataPacket(inPkt);
                        List<RawPacket> transDatas
                                = stub.getTransmittedDataPacket();

                        if (pv.getNodeConnector().getNode().equals(disableNode)) {
                            assertEquals(emsg, PacketResult.IGNORED, result);
                            assertEquals(emsg, 0, transDatas.size());
                        } else {
                            assertEquals(emsg, PacketResult.KEEP_PROCESSING,
                                         result);
                            assertEquals(emsg,
                                    mappedThis.size() - disabledPV.size() - 1,
                                    transDatas.size());
                        }

                        for (RawPacket raw : transDatas) {
                            Ethernet pkt
                                    = (Ethernet) stub.decodeDataPacket(raw);
                            String emsgr = emsg + ",(out packet)"
                                    + pkt.toString() + ",(in nc)"
                                    + raw.getIncomingNodeConnector()
                                    + ",(out nc)"
                                    + raw.getOutgoingNodeConnector();

                            short outVlan;
                            if (pkt.getEtherType()
                                    == EtherTypes.VLANTAGGED.shortValue()) {
                                IEEE8021Q vlantag = (IEEE8021Q) pkt.getPayload();
                                outVlan = vlantag.getVid();
                            } else {
                                outVlan = (short) 0;
                            }
                            PortVlan outPv = new PortVlan(raw.getOutgoingNodeConnector(),
                                                          outVlan);
                            assertTrue(emsgr, mappedThis.contains(outPv));

                            checkOutEthernetPacket(emsgr, (Ethernet) pkt,
                                    EtherTypes.ARP, src, dst, outVlan,
                                    EtherTypes.IPv4, ARP.REQUEST, src, dst,
                                    sender, target);
                        }
                    }

                    mgr.clearDisabledNode();

                    for (PortVlan pv : mappedThis) {
                        String emsg = "(vlan)" + vlan
                                + ",(VlanMapConfig)" + vlconf.toString()
                                + ",(PortVlan)" + pv.toString();
                        RawPacket inPkt = createARPRawPacket(src, dst, sender,
                                target, (pv.getVlan() > 0) ? pv.getVlan() : -1,
                                pv.getNodeConnector(), ARP.REQUEST);
                        PacketResult result = mgr.receiveDataPacket(inPkt);
                        List<RawPacket> transDatas = stub.getTransmittedDataPacket();

                        assertEquals(emsg, PacketResult.KEEP_PROCESSING, result);
                        assertEquals(emsg, mappedThis.size() - 1, transDatas.size());
                    }

                    st = mgr.removeVlanMap(bpath, map.getId());
                    assertTrue("(vlconf)" + vlconf.toString()
                            + ",(status)" + st,
                            st.isSuccess());
                }
            }
        }

        stopVTNManager();
    }

    /**
     * test case receive a packet after disabledNode timer is expired.
     */
    @Test
    public void testDisabledNodeAfterTimeout() {
        // set a value of EdgeTimeout to 1 msec.
        // expect expire before receive a packet.
        setupVTNManager(1);

        VTNManagerImpl mgr = vtnMgr;
        ISwitchManager swMgr = mgr.getSwitchManager();
        ITopologyManager topoMgr = mgr.getTopologyManager();
        TestStub stub = stubObj;
        short[] vlans = new short[] { 0, 10, 4095 };
        Set<Node> nodeSet = new HashSet<Node>();

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

        ConcurrentMap<VBridgePath, Set<NodeConnector>> mappedConnectors
                = new ConcurrentHashMap<VBridgePath, Set<NodeConnector>>();
        mappedConnectors.put(bpath, new HashSet<NodeConnector>());

        Set<Node> existNodes = swMgr.getNodes();
        Set<NodeConnector> existConnectors = new HashSet<NodeConnector>();
        for (Node node : existNodes) {
            existConnectors.addAll(swMgr.getNodeConnectors(node));
        }

        Set<Node> vmapNodes = new HashSet<Node>(existNodes);
        vmapNodes.add(null);

        byte[] src = new byte[] { (byte) 0x00, (byte) 0x00, (byte) 0x00,
                                  (byte) 0x00, (byte) 0x00, (byte) 0x11 };
        byte[] dst = new byte[] { (byte) 0xff, (byte) 0xff, (byte) 0xff,
                                  (byte) 0xff, (byte) 0xff, (byte) 0xff };
        byte[] sender = new byte[] { (byte) 192, (byte) 168, (byte) 0, (byte) 1 };
        byte[] target = new byte[] { (byte) 192, (byte) 168, (byte) 0, (byte) 250 };

        Node pnode = NodeCreator.createOFNode(0L);
        SwitchPort port = new SwitchPort(
                NodeConnector.NodeConnectorIDType.OPENFLOW, String.valueOf(10));
        NodeConnector pmapnc
            = NodeConnectorCreator.createOFNodeConnector(Short.valueOf((short) 10),
                                                         pnode);
        Node disableNode = NodeCreator.createOFNode(0L);

        for (short vlan : vlans) {
            // add port mapping
            Set<PortVlan> portMappedThis = new HashSet<PortVlan>();
            Set<PortVlan> portMappedOther = new HashSet<PortVlan>();
            short pmapvlan = vlan;

            PortMapConfig pmconf = new PortMapConfig(pnode, port, pmapvlan);
            st = mgr.setPortMap(ifp, pmconf);
            assertTrue(st.isSuccess());

            Set<NodeConnector> set = mappedConnectors.get(bpath);
            set.add(pmapnc);
            mappedConnectors.put(bpath, set);
            portMappedThis.add(new PortVlan(pmapnc, pmapvlan));

            for (Node vlanMapNode : vmapNodes) {
                for (short vmapVlan : vlans) {
                    Set<PortVlan> mappedThis = new HashSet<PortVlan>();
                    Set<PortVlan> disabledPV = new HashSet<PortVlan>();

                    // add vlan mapping
                    VlanMapConfig vlconf = new VlanMapConfig(vlanMapNode,
                                                             vmapVlan);
                    VlanMap map = null;
                    try {
                        map = mgr.addVlanMap(bpath, vlconf);
                    } catch (VTNException e) {
                        unexpected(e);
                    }

                    for (Node node : existNodes) {
                        if (vlanMapNode == null || node.equals(vlanMapNode)) {
                            for (NodeConnector nc :
                                    swMgr.getNodeConnectors(node)) {
                                if (!topoMgr.isInternal(nc)) {
                                    mappedThis.add(new PortVlan(nc, vmapVlan));
                                    if (node.equals(disableNode)) {
                                        disabledPV.add(new PortVlan(nc,
                                                       vmapVlan));
                                    }
                                }
                            }
                        }
                    }

                    mappedThis.addAll(portMappedThis);
                    mappedThis.removeAll(portMappedOther);

                    if (pmconf.getNode().equals(disableNode)) {
                        disabledPV.add(new PortVlan(pmapnc, pmapvlan));
                    }

                    // a result of adding Node, specified Node set to
                    // disableNodes.
                    addNode(mgr, swMgr, disableNode);

                    // sleep until disabled Node timer timeout
                    try {
                        Thread.sleep(10);
                    } catch (InterruptedException e) {
                        unexpected(e);
                    }

                    // EdgeTimeout is not infinity, but time is long enough and
                    // expected to return result before timeout.
                    for (PortVlan pv : mappedThis) {
                        String emsg = "(vlan)" + vlan
                                + ",(vlanmap conf)" + vlconf.toString()
                                + ",(portvlan)" + pv.toString();
                        RawPacket inPkt = createARPRawPacket(src, dst, sender,
                                target, (pv.getVlan() > 0) ? pv.getVlan() : -1,
                                pv.getNodeConnector(), ARP.REQUEST);
                        PacketResult result = mgr.receiveDataPacket(inPkt);
                        List<RawPacket> transDatas = stub.getTransmittedDataPacket();
                        assertEquals(emsg, PacketResult.KEEP_PROCESSING, result);
                        assertEquals(emsg, mappedThis.size() - 1,
                                transDatas.size());

                        for (RawPacket raw : transDatas) {
                            Ethernet pkt = (Ethernet) stub.decodeDataPacket(raw);
                            String emsgr = emsg + ",(out packet)"
                                            + pkt.toString() + ",(in nc)"
                                            + raw.getIncomingNodeConnector()
                                            + ",(out nc)"
                                            + raw.getOutgoingNodeConnector();
                            short outVlan;
                            if (pkt.getEtherType()
                                    == EtherTypes.VLANTAGGED.shortValue()) {
                                IEEE8021Q vlantag = (IEEE8021Q) pkt.getPayload();
                                outVlan = vlantag.getVid();
                            } else {
                                outVlan = (short) 0;
                            }
                            PortVlan outPv = new PortVlan(raw.getOutgoingNodeConnector(),
                                    outVlan);
                            assertTrue(emsgr, mappedThis.contains(outPv));

                            checkOutEthernetPacket(emsgr, (Ethernet) pkt,
                                    EtherTypes.ARP, src, dst, outVlan,
                                    EtherTypes.IPv4, ARP.REQUEST, src, dst,
                                    sender, target);
                        }
                    }

                    st = mgr.removeVlanMap(bpath, map.getId());
                    assertTrue("(vlconf)" + vlconf.toString() + ",(status)" + st,
                                st.isSuccess());
                }
            }
        }

        stopVTNManager();
    }

    /**
     * Test method for {@link VTNManagerImpl#entryUpdated}.
     * This tests {@link RawPacketEvent}.
     */
    @Test
    public void testCacheEntryChangeRawPacketEvent() {
        // set a value of EdgeTimeout to 600 sec.
        setupVTNManager(600000);

        VTNManagerImpl mgr = vtnMgr;
        ISwitchManager swMgr = mgr.getSwitchManager();

        byte[] src = new byte[] {0x00, 0x00, 0x00, 0x00, 0x00, 0x01};
        byte[] dst = new byte[] {(byte) 0xff, (byte) 0xff, (byte) 0xff,
                                 (byte) 0xff, (byte) 0xff, (byte) 0xff};
        byte[] sender = new byte[] {(byte) 192, (byte) 168, (byte) 0, (byte) 1};
        byte[] target = new byte[] {(byte) 192, (byte) 168, (byte) 0, (byte) 10};
        Node node0 = NodeCreator.createOFNode(Long.valueOf(0L));
        NodeConnector innc = NodeConnectorCreator.createOFNodeConnector(Short.valueOf((short) 10), node0);
        NodeConnector outnc = NodeConnectorCreator.createOFNodeConnector(Short.valueOf((short) 11), node0);
        RawPacket pkt = createARPRawPacket(src, dst, sender, target, (short) 0, innc, ARP.REQUEST);

        RawPacketEvent ev = new RawPacketEvent(pkt, outnc);

        InetAddress ipaddr = getInetAddressFromAddress(new byte[] {0, 0, 0, 0});
        ClusterEventId evidRemote = new ClusterEventId(ipaddr, 0);
        ClusterEventId evidLocal = new ClusterEventId();

        Set<ClusterEventId> evIdSet = new HashSet<ClusterEventId>();
        evIdSet.add(evidLocal);
        evIdSet.add(evidRemote);

        for (Node disableNode : swMgr.getNodes()) {
            for (ClusterEventId evid : evIdSet) {
                mgr.clearDisabledNode();
                addNode(mgr, swMgr, disableNode);

                String emsg = evid.toString();
                // in case entry created, no operation is executed.
                mgr.entryCreated(evid, VTNManagerImpl.CACHE_EVENT, true);
                flushTasks(mgr);
                assertEquals(emsg, 0, stubObj.getTransmittedDataPacket().size());

                mgr.entryCreated(evid, VTNManagerImpl.CACHE_EVENT, false);
                flushTasks(mgr);
                assertEquals(emsg, 0, stubObj.getTransmittedDataPacket().size());

                // update event.
                mgr.entryUpdated(evid, ev, VTNManagerImpl.CACHE_EVENT, true);
                flushTasks(mgr);
                assertEquals(emsg, 0, stubObj.getTransmittedDataPacket().size());

                mgr.entryUpdated(evid, ev, VTNManagerImpl.CACHE_EVENT, false);
                flushTasks(mgr);
                if (evid == evidRemote && !disableNode.equals(node0)) {
                    assertEquals(emsg, 1,
                                 stubObj.getTransmittedDataPacket().size());
                } else {
                    assertEquals(emsg, 0,
                                 stubObj.getTransmittedDataPacket().size());
                }

                // in case entry deleted, no operation is executed.
                mgr.entryDeleted(evid, VTNManagerImpl.CACHE_EVENT, true);
                flushTasks(mgr);
                assertEquals(emsg, 0,
                             stubObj.getTransmittedDataPacket().size());

                mgr.entryDeleted(evid, VTNManagerImpl.CACHE_EVENT, false);
                flushTasks(mgr);
                assertEquals(emsg, 0,
                             stubObj.getTransmittedDataPacket().size());
            }
        }

        stopVTNManager();
    }


    // private methods

    /**
     * setup initfile
     *
     * @param containerName a container name.
     * @param val {@code nodeEdgeWait} time (in milliseconds)
     */
    private void setupInifile(String containerName, int val) {
        String dir = GlobalConstants.STARTUPHOME.toString();
        String filename = "vtnmanager-" + containerName + ".ini";

        FileWriter writer;
        File inifile = new File(dir, filename);
        String prop = "nodeEdgeWait=" + val;
        try {
            writer = new FileWriter(inifile);
            writer.write(prop);
            writer.close();
        } catch (IOException e) {
            unexpected(e);
        }
    }

    /**
     * setup VTNManager.
     *
     * @param edgeTimeoutVal    a time wait after new node detect in millisecond.
     */
    private void setupVTNManager(int edgeTimeoutVal) {
        File confdir = new File(GlobalConstants.STARTUPHOME.toString());
        boolean result = confdir.exists();
        if (!result) {
            result = confdir.mkdirs();
        } else {
            File[] list = confdir.listFiles();
            for (File f : list) {
                f.delete();
            }
        }

        setupInifile("default", edgeTimeoutVal);

        vtnMgr = new VTNManagerImpl();
        ComponentImpl c = new ComponentImpl(null, null, null);
        GlobalResourceManager grsc = new GlobalResourceManager();
        stubObj = new TestStub(stubMode);

        Hashtable<String, String> properties = new Hashtable<String, String>();
        properties.put("containerName", "default");
        c.setServiceProperties(properties);

        grsc.setClusterGlobalService(stubObj);
        grsc.init(c);
        vtnMgr.setResourceManager(grsc);
        vtnMgr.setClusterContainerService(stubObj);
        vtnMgr.setSwitchManager(stubObj);
        vtnMgr.setTopologyManager(stubObj);
        vtnMgr.setDataPacketService(stubObj);
        vtnMgr.setRouting(stubObj);
        vtnMgr.setHostTracker(stubObj);
        vtnMgr.setForwardingRuleManager(stubObj);
        vtnMgr.setConnectionManager(stubObj);
        vtnMgr.init(c);
        vtnMgr.clearDisabledNode();
    }

    /**
     * stop VTNManager.
     */
    private void stopVTNManager() {
        vtnMgr.destroy();

        String currdir = new File(".").getAbsoluteFile().getParent();
        File confdir = new File(GlobalConstants.STARTUPHOME.toString());
        if (confdir.exists()) {
            File[] list = confdir.listFiles();
            for (File f : list) {
                f.delete();
            }

            while (confdir != null && confdir.getAbsolutePath() != currdir) {
                confdir.delete();
                String pname = confdir.getParent();
                if (pname == null) {
                    break;
                }
                confdir = new File(pname);
            }
        }
    }

    /**
     * method for setup a environment. create 1 Tenant and bridges and vinterfaces.
     */
    private void createTenantAndBridgeAndInterface(IVTNManager mgr,
            VTenantPath tpath, List<VBridgePath> bpaths,
            List<VBridgeIfPath> ifpaths) {

        Status st = mgr.addTenant(tpath, new VTenantConfig(null));
        assertTrue(st.isSuccess());
        assertTrue(mgr.isActive());

        for (VBridgePath bpath : bpaths) {
            st = mgr.addBridge(bpath, new VBridgeConfig(null));
            assertTrue(st.isSuccess());
        }

        for (VBridgeIfPath ifpath : ifpaths) {
            VInterfaceConfig ifconf = new VInterfaceConfig(null, null);
            st = mgr.addBridgeInterface(ifpath, ifconf);
            assertTrue(st.isSuccess());
        }
    }

    /**
     * remove and add specified node to add to {@code disabledNode}.
     *
     * @param mgr   a {@link VTNManagerImpl} object.
     * @param swmgr a {@link ISwitchManager} object.
     * @param dnode a {@link Node} disabled.
     */
    private void addNode(VTNManagerImpl mgr, ISwitchManager swmgr, Node dnode) {
        Map<String, Property> propMap = null;
        mgr.notifyNode(dnode, UpdateType.REMOVED, propMap);
        mgr.notifyNode(dnode, UpdateType.ADDED, propMap);

        Set<NodeConnector> ncs = swmgr.getNodeConnectors(dnode);
        for (NodeConnector nc : ncs) {
            propMap = swmgr.getNodeConnectorProps(nc);
            mgr.notifyNodeConnector(nc, UpdateType.ADDED, propMap);
        }
        mgr.initISL();
    }
}
