/*
 * Copyright (c) 2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */
package org.opendaylight.vtn.manager.internal;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.net.InetAddress;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Hashtable;
import java.util.List;
import java.util.Map;
import java.util.Set;

import org.apache.felix.dm.impl.ComponentImpl;
import org.junit.BeforeClass;
import org.junit.Test;

import org.opendaylight.controller.clustering.services.IClusterContainerServices;
import org.opendaylight.controller.connectionmanager.IConnectionManager;
import org.opendaylight.controller.forwardingrulesmanager.IForwardingRulesManager;
import org.opendaylight.controller.hosttracker.IfHostListener;
import org.opendaylight.controller.hosttracker.IfIptoHost;
import org.opendaylight.controller.hosttracker.hostAware.HostNodeConnector;
import org.opendaylight.controller.sal.core.ConstructionException;
import org.opendaylight.controller.sal.core.Node;
import org.opendaylight.controller.sal.core.NodeConnector;
import org.opendaylight.controller.sal.core.UpdateType;
import org.opendaylight.controller.sal.packet.ARP;
import org.opendaylight.controller.sal.packet.IDataPacketService;
import org.opendaylight.controller.sal.packet.PacketResult;
import org.opendaylight.controller.sal.packet.RawPacket;
import org.opendaylight.controller.sal.packet.address.EthernetAddress;
import org.opendaylight.controller.sal.routing.IRouting;
import org.opendaylight.controller.sal.utils.GlobalConstants;
import org.opendaylight.controller.sal.utils.NodeCreator;
import org.opendaylight.controller.sal.utils.Status;
import org.opendaylight.controller.sal.utils.StatusCode;
import org.opendaylight.controller.switchmanager.ISwitchManager;
import org.opendaylight.controller.topologymanager.ITopologyManager;
import org.opendaylight.vtn.manager.IVTNManagerAware;
import org.opendaylight.vtn.manager.IVTNModeListener;
import org.opendaylight.vtn.manager.MacAddressEntry;
import org.opendaylight.vtn.manager.PortMap;
import org.opendaylight.vtn.manager.PortMapConfig;
import org.opendaylight.vtn.manager.SwitchPort;
import org.opendaylight.vtn.manager.VBridge;
import org.opendaylight.vtn.manager.VBridgeConfig;
import org.opendaylight.vtn.manager.VBridgeIfPath;
import org.opendaylight.vtn.manager.VBridgePath;
import org.opendaylight.vtn.manager.VInterface;
import org.opendaylight.vtn.manager.VInterfaceConfig;
import org.opendaylight.vtn.manager.VNodeState;
import org.opendaylight.vtn.manager.VTNException;
import org.opendaylight.vtn.manager.VTenant;
import org.opendaylight.vtn.manager.VTenantConfig;
import org.opendaylight.vtn.manager.VTenantPath;
import org.opendaylight.vtn.manager.VlanMap;
import org.opendaylight.vtn.manager.VlanMapConfig;
import org.opendaylight.vtn.manager.internal.cluster.ClusterEventId;
import org.opendaylight.vtn.manager.internal.cluster.VTenantEvent;

/**
 * JUnit test for {@link VTNManagerImplTest}.
 * This test includes tests for other modules
 * under {@code org/opendaylight/vtn/manager/internal} directory.
 */
public class VTNManagerImplTest extends VTNManagerImplTestCommon {

    @BeforeClass
    public static void beforeClass() {
        stubMode = 0;
    }

    /**
     * Test method for
     * {@link VTNManagerImpl#init()},
     * {@link VTNManagerImpl#destroy()}
     */
    @Test
    public void testInitDestory() {
        ComponentImpl c = new ComponentImpl(null, null, null);
        Hashtable<String, String> properties = new Hashtable<String, String>();
        String containerName = "default";
        properties.put("containerName", containerName);
        c.setServiceProperties(properties);
        Map<VBridgeIfPath, PortMapConfig> pmaps = new HashMap<VBridgeIfPath, PortMapConfig>();
        Map<VlanMap, VlanMapConfig> vmaps = new HashMap<VlanMap, VlanMapConfig>();

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
        createTenantAndBridgeAndInterface(vtnMgr, tpath, bpathlist, ifpathlist);

        Status st = vtnMgr.addBridgeInterface(ifpath2, new VInterfaceConfig(null, false));
        ifpathlist.add(ifpath2);
        assertTrue(st.toString(), st.isSuccess());

        restartVTNManager(c);

        pmaps.put(ifpath1, null);
        pmaps.put(ifpath2, null);
        checkVTNconfig(tpath, bpathlist, pmaps, null);

        // add mappings
        Node node = NodeCreator.createOFNode(0L);
        SwitchPort port = new SwitchPort(NodeConnector.NodeConnectorIDType.OPENFLOW,
        String.valueOf(10));
        PortMapConfig pmconf = new PortMapConfig(node, port, (short)0);
        st = vtnMgr.setPortMap(ifpath1, pmconf);

        VlanMapConfig vlconf = new VlanMapConfig(null, (short)4095);
        VlanMap map = null;
        try {
            map = vtnMgr.addVlanMap(bpath, vlconf);
        } catch (VTNException e) {
            unexpected(e);
        }
        vmaps.put(map, vlconf);

        restartVTNManager(c);

        checkVTNconfig(tpath, bpathlist, pmaps, vmaps);

        stopVTNManager(false);
        VTNManagerImpl.cleanUpConfigFile(containerName);
        startVTNManager(c);

        checkVTNconfig(tpath, bpathlist, pmaps, vmaps);

        stopVTNManager(true);
        VTNManagerImpl.cleanUpConfigFile(containerName);
        startVTNManager(c);

        List<VTenant> tlist = null;
        try {
            tlist = vtnMgr.getTenants();
        } catch (VTNException e) {
            unexpected(e);
        }
        assertNotNull(tlist);
        assertEquals(0, tlist.size());

        stopVTNManager(true);
        VTNManagerImpl.cleanUpConfigFile(containerName);

        c = new ComponentImpl(null, null, null);
        c.setServiceProperties(null);
        startVTNManager(c);
        assertFalse(vtnMgr.isAvailable());

        stopVTNManager(true);
        VTNManagerImpl.cleanUpConfigFile(containerName);

        // in case not "default" container
        c = new ComponentImpl(null, null, null);
        properties = new Hashtable<String, String>();
        String containerNameTest = "test";
        properties.put("containerName", containerNameTest);
        c.setServiceProperties(properties);
        startVTNManager(c);

        createTenantAndBridgeAndInterface(vtnMgr, tpath, bpathlist, ifpathlist);

        stopVTNManager(true);
        VTNManagerImpl.cleanUpConfigFile(containerNameTest);
    }

    /**
     * Test method for
     * {@link VTNManagerImpl#getContainerName()}
     */
    @Test
    public void testGetContainerName() {
        assertEquals("default", vtnMgr.getContainerName());
    }

    /**
     * Test method for
     * {@link VTNManagerImpl#setClusterContainerService(IClusterContainerServices)},
     * {@link VTNManagerImpl#unsetClusterContainerService(IClusterContainerServices)}
     * .
     */
    @Test
    public void testSetUnsetClusterContainerService() {
        VTNManagerImpl mgr = vtnMgr;
        TestStub stubnew = new TestStub();
        TestStub stubnew2 = new TestStub();

        // TODO: need to check
        mgr.setClusterContainerService(stubnew);
        mgr.unsetClusterContainerService(stubnew2);
        mgr.unsetClusterContainerService(stubnew);
        mgr.setClusterContainerService(stubObj);
    }

    /**
     * Test method for
     * {@link VTNManagerImpl#setSwitchManager(ISwitchManager)},
     * {@link VTNManagerImpl#unsetSwitchManager(ISwitchManager)},
     * {@link VTNManagerImpl#getSwitchManager()}
     * .
     */
    @Test
    public void testSetUnsetSwitchManager() {
        VTNManagerImpl mgr = vtnMgr;
        ISwitchManager org = mgr.getSwitchManager();
        TestStub stub = new TestStub();
        TestStub stub2 = new TestStub();

        mgr.setSwitchManager((ISwitchManager)stub);
        assertSame(stub, mgr.getSwitchManager());

        mgr.unsetSwitchManager((ISwitchManager)stub2);
        assertSame(stub, mgr.getSwitchManager());

        mgr.unsetSwitchManager((ISwitchManager)stub);
        assertNull(mgr.getSwitchManager());

        mgr.setSwitchManager((ISwitchManager)org);
        assertSame(org, mgr.getSwitchManager());
    }

    /**
     * Test method for
     * {@link VTNManagerImpl#setTopologyManager(ITopologyManager)},
     * {@link VTNManagerImpl#unsetTopologyManager(ITopologyManager)},
     * {@link VTNManagerImpl#getTopologyManager()}.
     */
    @Test
    public void testSetUnsetTopologyManager() {
        VTNManagerImpl mgr = vtnMgr;
        ITopologyManager org = mgr.getTopologyManager();
        TestStub stub = new TestStub();
        TestStub stub2 = new TestStub();

        mgr.setTopologyManager((ITopologyManager)stub);
        assertSame(stub, mgr.getTopologyManager());

        mgr.unsetTopologyManager((ITopologyManager)stub2);
        assertSame(stub, mgr.getTopologyManager());

        mgr.unsetTopologyManager((ITopologyManager)stub);
        assertNull(mgr.getTopologyManager());

        mgr.setTopologyManager((ITopologyManager)org);
        assertSame(org, mgr.getTopologyManager());
    }

    /**
     * Test method for
     * {@link VTNManagerImpl#setForwardingRuleManager(IForwardingRulesManager)},
     * {@link VTNManagerImpl#unsetForwardingRuleManager(IForwardingRulesManager)},
     * {@link VTNManagerImpl#getForwardingRuleManager()}
     */
    @Test
    public void testSetUnsetForwardingRuleManager() {
        VTNManagerImpl mgr = vtnMgr;
        IForwardingRulesManager org = mgr.getForwardingRuleManager();
        TestStub stub = new TestStub();
        TestStub stub2 = new TestStub();

        mgr.setForwardingRuleManager((IForwardingRulesManager)stub);
        assertSame(stub, mgr.getForwardingRuleManager());

        mgr.unsetForwardingRuleManager((IForwardingRulesManager)stub2);
        assertSame(stub, mgr.getForwardingRuleManager());

        mgr.unsetForwardingRuleManager((IForwardingRulesManager)stub);
        assertNull(mgr.getForwardingRuleManager());

        mgr.setForwardingRuleManager((IForwardingRulesManager)org);
        assertSame(org, mgr.getForwardingRuleManager());
    }

    /**
     * Test method for
     * {@link VTNManagerImpl#setRouting(IRouting)},
     * {@link VTNManagerImpl#unsetRouting(IRouting)},
     * {@link VTNManagerImpl#getRouting()}
     */
    @Test
    public void testSetUnsetRouting() {
        VTNManagerImpl mgr = vtnMgr;
        IRouting org = mgr.getRouting();
        TestStub stub = new TestStub();
        TestStub stub2 = new TestStub();

        mgr.setRouting((IRouting)stub);
        assertSame(stub, mgr.getRouting());

        mgr.unsetRouting((IRouting)stub2);
        assertSame(stub, mgr.getRouting());

        mgr.unsetRouting((IRouting)stub);
        assertNull(mgr.getRouting());

        mgr.setRouting((IRouting)org);
        assertSame(org, mgr.getRouting());
    }

    /**
     * Test method for
     * {@link VTNManagerImpl#setDataPacketService(IDataPacketService)},
     * {@link VTNManagerImpl#unsetDataPacketService(IDataPacketService)},
     * {@link VTNManagerImpl#getDataPacketService()}
     * .
     */
    @Test
    public void testSetUnsetDataPacketService() {
        VTNManagerImpl mgr = vtnMgr;
        IDataPacketService org = mgr.getDataPacketService();
        TestStub stub = new TestStub();
        TestStub stub2 = new TestStub();

        mgr.setDataPacketService((IDataPacketService)stub);
        assertSame(stub, mgr.getDataPacketService());

        mgr.unsetDataPacketService((IDataPacketService)stub2);
        assertSame(stub, mgr.getDataPacketService());

        mgr.unsetDataPacketService((IDataPacketService)stub);
        assertNull(mgr.getDataPacketService());

        mgr.setDataPacketService((IDataPacketService)org);
        assertSame(org, mgr.getDataPacketService());
    }

    /**
     * Test method for
     * {@link VTNManagerImpl#setHostTracker(IfIptoHost)},
     * {@link VTNManagerImpl#unsetHostTracker(IfIptoHost)},
     * {@link VTNManagerImpl#getHostTracker()}
     */
    @Test
    public void testSetUnsetHostTracker() {
        VTNManagerImpl mgr = vtnMgr;
        IfIptoHost org = mgr.getHostTracker();
        TestStub stub = new TestStub();
        TestStub stub2 = new TestStub();

        mgr.setHostTracker(stub);
        assertSame(stub, mgr.getHostTracker());

        mgr.unsetHostTracker(stub2);
        assertSame(stub, mgr.getHostTracker());

        mgr.unsetHostTracker(stub);
        assertNull(mgr.getHostTracker());

        mgr.setHostTracker(org);
        assertSame(org, mgr.getHostTracker());
    }

    /**
     * Test method for
     * {@link VTNManagerImpl#addHostListener(IfHostListener)},
     * {@link VTNManagerImpl#removeHostListener(IfHostListener)},
     * {@link VTNManagerImpl#notifyHost(HostNodeConnector)}
     */
    @Test
    public void testIfHostListener() {
       VTNManagerImpl mgr = vtnMgr;

       HostListener hl1 = new HostListener();
       HostListener hl2 = new HostListener();
       mgr.addHostListener(hl1);
       mgr.addHostListener(hl2);
       mgr.addHostListener(hl1);

       // check not removed listner when registered listener isn't match
       // a listener specified removeHostListener
       HostListener hlnew1 = new HostListener();
       HostListener hlnew2 = new HostListener();
       mgr.removeHostListener(hlnew2);
       mgr.removeHostListener(hlnew1);

       // add entry to MacAddressTable to call HostListener
       byte [] src = new byte[] {(byte)0, (byte)0, (byte)0, (byte)0, (byte)0, (byte)0x01};
       byte [] dst = new byte[] {(byte)0xff, (byte)0xff, (byte)0xff,
                                   (byte)0xff, (byte)0xff, (byte)0xff};
       byte [] sender = new byte[] {(byte)192, (byte)168, (byte)0, (byte)1};
       byte [] target = new byte[] {(byte)192, (byte)168, (byte)0, (byte)250};
       List<NodeConnector> connectors = createNodeConnectors(1, false);
       PacketContext pctx = createARPPacketContext(src, dst, sender, target,
               (short)-1, connectors.get(0), ARP.REQUEST);

       VBridgePath path = new VBridgePath("tenant1", "bridge1");
       MacAddressTable table = new MacAddressTable(mgr, path, 600);
       table.add(pctx);

       flushTasks();
       assertEquals(1, hl1.getHostListenerCalled());
       assertEquals(1, hl2.getHostListenerCalled());

       mgr.removeHostListener(hl2);
       mgr.removeHostListener(hl1);
    }

    /**
     * Test method for
     * {@link VTNManagerImpl#setConnectionManager(IConnectionManager)},
     * {@link VTNManagerImpl#unsetConnectionManager(IConnectionManager)},
     * {@link VTNManagerImpl#getConnectionManager()}.
     */
    @Test
    public void testSetUnsetConnectionManager() {
        VTNManagerImpl mgr = vtnMgr;
        IConnectionManager org = mgr.getConnectionManager();
        TestStub stub = new TestStub();

        mgr.setConnectionManager(stub);
        assertSame(stub, mgr.getConnectionManager());

        mgr.unsetConnectionManager(org);
        assertSame(stub, mgr.getConnectionManager());
        mgr.unsetConnectionManager(stub);
        assertNull(mgr.getConnectionManager());

        mgr.setConnectionManager(org);
        assertSame(org, mgr.getConnectionManager());
    }

    /**
     * Test method for
     * {@link VTNManagerImpl#setResourceManager(IVTNResourceManager)},
     * {@link VTNManagerImpl#unsetResourceManager(IVTNResourceManager)},
     * {@link VTNManagerImpl#getResourceManager()}
     */
    @Test
    public void testSetResourceManager() {
        VTNManagerImpl mgr = vtnMgr;
        IVTNResourceManager org = mgr.getResourceManager();
        GlobalResourceManager newmgr = new GlobalResourceManager();
        GlobalResourceManager newmgr2 = new GlobalResourceManager();

        mgr.setResourceManager(newmgr);
        assertSame(newmgr, mgr.getResourceManager());

        mgr.unsetResourceManager(newmgr2);
        assertSame(newmgr, mgr.getResourceManager());

        mgr.unsetResourceManager(newmgr);
        assertNull(mgr.getResourceManager());

        mgr.setResourceManager(org);
        assertSame(org, mgr.getResourceManager());
    }

    /**
     * Test method for
     * {@link VTNManagerImpl#removeVTNManagerAware(IVTNManagerAware)},
     * {@link VTNManagerImpl#addVTNManagerAware(IVTNManagerAware)}
     */
    @Test
    public void testIVTNManagerAware() {
        VTNManagerImpl mgr = vtnMgr;

        VTNManagerAwareStub stub1 = new VTNManagerAwareStub();
        VTNManagerAwareStub stub2 = new VTNManagerAwareStub();

        // add a tenant
        String tname = "tenant";
        VTenantPath tpath = new VTenantPath(tname);
        Status st = mgr.addTenant(tpath, new VTenantConfig(null));
        assertTrue(st.isSuccess());
        flushTasks();
        stub1.checkAllNull();
        stub2.checkAllNull();

        mgr.addVTNManagerAware(stub1);
        mgr.addVTNManagerAware(stub2);
        flushTasks();
        stub1.checkVtnInfo(1, tpath, tname, UpdateType.ADDED);
        stub2.checkVtnInfo(1, tpath, tname, UpdateType.ADDED);

        mgr.removeVTNManagerAware(stub2);
        mgr.addVTNManagerAware(stub2);
        flushTasks();
        stub2.checkVtnInfo(1, tpath, tname, UpdateType.ADDED);
        stub1.checkAllNull();
        stub2.checkAllNull();

        // add a vbridge
        String bname = "bridge";
        VBridgePath bpath = new VBridgePath(tname, bname);
        st = mgr.addBridge(bpath, new VBridgeConfig(null));
        assertTrue(st.isSuccess());
        stub1.checkVbrInfo(1, bpath, bname, UpdateType.ADDED);
        stub2.checkVbrInfo(1, bpath, bname, UpdateType.ADDED);

        mgr.removeVTNManagerAware(stub2);
        mgr.addVTNManagerAware(stub2);
        flushTasks();
        stub2.checkVtnInfo(1, tpath, tname, UpdateType.ADDED);
        stub2.checkVbrInfo(1, bpath, bname, UpdateType.ADDED);
        stub1.checkAllNull();
        stub2.checkAllNull();

        // add a vInterface
        String ifname = "vif";
        VBridgeIfPath ifpath = new VBridgeIfPath(tname, bname, ifname);
        VInterfaceConfig ifconf = new VInterfaceConfig(null, null);
        st = mgr.addBridgeInterface(ifpath, ifconf);
        assertTrue(st.isSuccess());
        stub1.checkVIfInfo(1, ifpath, ifname, UpdateType.ADDED);
        stub2.checkVIfInfo(1, ifpath, ifname, UpdateType.ADDED);

        mgr.removeVTNManagerAware(stub2);
        mgr.addVTNManagerAware(stub2);
        flushTasks();
        stub2.checkVtnInfo(1, tpath, tname, UpdateType.ADDED);
        stub2.checkVbrInfo(1, bpath, bname, UpdateType.ADDED);
        stub2.checkVIfInfo(1, ifpath, ifname, UpdateType.ADDED);
        stub1.checkAllNull();
        stub2.checkAllNull();

        // set a PortMap
        Node node = NodeCreator.createOFNode(0L);
        SwitchPort port = new SwitchPort(NodeConnector.NodeConnectorIDType.OPENFLOW,
        String.valueOf(10));
        PortMapConfig pmconf = new PortMapConfig(node, port, (short)0);
        st = mgr.setPortMap(ifpath, pmconf);

        // at this vbridge status and vInterface status are UNKNOWN -> DOWN.
        stub1.checkVbrInfo(1, bpath, bname, UpdateType.CHANGED);
        stub1.checkVIfInfo(1, ifpath, ifname, UpdateType.CHANGED);
        stub1.checkPmapInfo(1, ifpath, pmconf, UpdateType.ADDED);
        stub2.checkVbrInfo(1, bpath, bname, UpdateType.CHANGED);
        stub2.checkVIfInfo(1, ifpath, ifname, UpdateType.CHANGED);
        stub2.checkPmapInfo(1, ifpath, pmconf, UpdateType.ADDED);

        mgr.removeVTNManagerAware(stub2);
        mgr.addVTNManagerAware(stub2);
        flushTasks();
        stub2.checkVtnInfo(1, tpath, tname, UpdateType.ADDED);
        stub2.checkVbrInfo(1, bpath, bname, UpdateType.ADDED);
        stub2.checkVIfInfo(1, ifpath, ifname, UpdateType.ADDED);
        stub2.checkPmapInfo(1, ifpath, pmconf, UpdateType.ADDED);
        stub1.checkAllNull();
        stub2.checkAllNull();

        // set a VLanMap
        VlanMapConfig vlconf = new VlanMapConfig(null, (short)4095);
        VlanMap map = null;
        try {
            map = mgr.addVlanMap(bpath, vlconf);
        } catch (VTNException e) {
            unexpected(e);
        }
        stub1.checkVlmapInfo(1, bpath, map.getId(), UpdateType.ADDED);
        stub2.checkVlmapInfo(1, bpath, map.getId(), UpdateType.ADDED);

        mgr.removeVTNManagerAware(stub2);
        mgr.addVTNManagerAware(stub2);
        flushTasks();
        stub2.checkVtnInfo(1, tpath, tname, UpdateType.ADDED);
        stub2.checkVbrInfo(1, bpath, bname, UpdateType.ADDED);
        stub2.checkVIfInfo(1, ifpath, ifname, UpdateType.ADDED);
        stub2.checkPmapInfo(1, ifpath, pmconf, UpdateType.ADDED);
        stub2.checkVlmapInfo(1, bpath, map.getId(), UpdateType.ADDED);
        stub1.checkAllNull();
        stub2.checkAllNull();

        mgr.addVTNManagerAware(stub2);
        flushTasks();

        // modify a tenant setting
        st = mgr.modifyTenant(tpath, new VTenantConfig("desc"), false);
        assertTrue(st.isSuccess());
        stub1.checkVtnInfo(1, tpath, tname, UpdateType.CHANGED);
        stub2.checkVtnInfo(1, tpath, tname, UpdateType.CHANGED);
        stub1.checkAllNull();
        stub2.checkAllNull();

        st = mgr.modifyBridge(bpath, new VBridgeConfig("desc"), false);
        assertTrue(st.isSuccess());
        stub1.checkVbrInfo(1, bpath, bname, UpdateType.CHANGED);
        stub2.checkVbrInfo(1, bpath, bname, UpdateType.CHANGED);
        stub1.checkAllNull();
        stub2.checkAllNull();

        st = mgr.modifyBridgeInterface(ifpath, new VInterfaceConfig("interface", true), false);
        assertTrue(st.isSuccess());
        stub1.checkVIfInfo(1, ifpath, ifname, UpdateType.CHANGED);
        stub2.checkVIfInfo(1, ifpath, ifname, UpdateType.CHANGED);
        stub1.checkAllNull();
        stub2.checkAllNull();

        // change a PortMap setting
        port = new SwitchPort(NodeConnector.NodeConnectorIDType.OPENFLOW, String.valueOf(11));
        pmconf = new PortMapConfig(node, port, (short)0);
        st = mgr.setPortMap(ifpath, pmconf);
        stub1.checkPmapInfo(1, ifpath, pmconf, UpdateType.CHANGED);
        stub2.checkPmapInfo(1, ifpath, pmconf, UpdateType.CHANGED);

        // remove a VLANMap
        st = mgr.removeVlanMap(bpath, map.getId());
        assertTrue(st.isSuccess());
        stub1.checkVlmapInfo(1, bpath, map.getId(), UpdateType.REMOVED);
        stub2.checkVlmapInfo(1, bpath, map.getId(), UpdateType.REMOVED);
        stub1.checkAllNull();
        stub2.checkAllNull();

        // remove a portmap
        st = mgr.setPortMap(ifpath, null);
        assertTrue(st.isSuccess());

        // vBridge status and vInterface status is changed (DOWN->UNKNOWN)
        stub1.checkVbrInfo(1, bpath, bname, UpdateType.CHANGED);
        stub1.checkVIfInfo(1, ifpath, ifname, UpdateType.CHANGED);
        stub1.checkPmapInfo(1, ifpath, pmconf, UpdateType.REMOVED);
        stub2.checkVbrInfo(1, bpath, bname, UpdateType.CHANGED);
        stub2.checkVIfInfo(1, ifpath, ifname, UpdateType.CHANGED);
        stub2.checkPmapInfo(1, ifpath, pmconf, UpdateType.REMOVED);
        stub1.checkAllNull();
        stub2.checkAllNull();

        // remove a vbridge interface
        st = mgr.removeBridgeInterface(ifpath);
        assertTrue(st.isSuccess());
        stub1.checkVIfInfo(1, ifpath, ifname, UpdateType.REMOVED);
        stub2.checkVIfInfo(1, ifpath, ifname, UpdateType.REMOVED);
        stub1.checkAllNull();
        stub2.checkAllNull();

        // remove a vbridge
        st = mgr.removeBridge(bpath);
        assertTrue(st.isSuccess());
        stub1.checkVbrInfo(1, bpath, bname, UpdateType.REMOVED);
        stub2.checkVbrInfo(1, bpath, bname, UpdateType.REMOVED);
        stub1.checkAllNull();
        stub2.checkAllNull();

        VTNManagerAwareStub stubnew1 = new VTNManagerAwareStub();
        mgr.removeVTNManagerAware(stubnew1);

        // remove a tenant
        st = mgr.removeTenant(tpath);
        assertTrue(st.isSuccess());
        stub1.checkVtnInfo(1, tpath, tname, UpdateType.REMOVED);
        stub2.checkVtnInfo(1, tpath, tname, UpdateType.REMOVED);
        stub1.checkAllNull();
        stub2.checkAllNull();
    }

    /**
     * Test method for
     * {@link VTNManagerImpl#addVTNModeListener(IVTNModeListener)},
     * {@link VTNManagerImpl#removeVTNModeListener(IVTNModeListener)},
     * {@link VTNManagerImpl#notifyChange(boolean)},
     * {@link VTNManagerImpl#notifyChange(IVTNModeListener, boolean)}
     */
    @Test
    public void testIVTNModeListener() {
        VTNManagerImpl mgr = vtnMgr;

        VTNModeListenerStub stub1 = new VTNModeListenerStub();
        VTNModeListenerStub stub2 = new VTNModeListenerStub();

        VTenantPath tpath = new VTenantPath("tenant");
        Status st = mgr.addTenant(tpath, new VTenantConfig(null));
        assertTrue(st.isSuccess());
        flushTasks();
        stub1.checkCalledInfo(0);
        stub2.checkCalledInfo(0);

        mgr.addVTNModeListener(stub1);
        stub1.checkCalledInfo(1, Boolean.TRUE);

        mgr.addVTNModeListener(stub2);
        stub2.checkCalledInfo(1, Boolean.TRUE);
        stub1.checkCalledInfo(0);

        mgr.addVTNModeListener(stub1);
        stub1.checkCalledInfo(0);
        stub2.checkCalledInfo(0);

        // notifyChange(boolean)
        for (Boolean bool: createBooleans(false)) {
            boolean curr = bool.booleanValue();
            mgr.notifyChange(curr);
            flushTasks();

            stub1.checkCalledInfo(1, bool);
            stub2.checkCalledInfo(1, bool);
        }

        // notifyChange(IVTNModeListener, boolean)
        for (Boolean bool: createBooleans(false)) {
            boolean curr = bool.booleanValue();
            mgr.notifyChange(stub1, curr);
            stub1.checkCalledInfo(1, bool);
            stub2.checkCalledInfo(0);

            mgr.notifyChange(stub2, curr);
            stub1.checkCalledInfo(0);
            stub2.checkCalledInfo(1, bool);
        }

        VTNModeListenerStub stubnew = new VTNModeListenerStub();
        mgr.removeVTNModeListener(stubnew);

        assertTrue(mgr.isActive());

        // test for containerModeUpdated()
        mgr.containerModeUpdated(UpdateType.ADDED);
        assertFalse(mgr.isActive());
        stub1.checkCalledInfo(1, Boolean.FALSE);
        stub2.checkCalledInfo(1, Boolean.FALSE);

        mgr.containerModeUpdated(UpdateType.CHANGED);
        assertFalse(mgr.isActive());
        stub1.checkCalledInfo(0);
        stub2.checkCalledInfo(0);

        mgr.containerModeUpdated(UpdateType.REMOVED);
        assertTrue(mgr.isActive());
        stub1.checkCalledInfo(1, Boolean.TRUE);
        stub2.checkCalledInfo(1, Boolean.TRUE);

        mgr.containerModeUpdated(UpdateType.CHANGED);
        assertTrue(mgr.isActive());
        stub1.checkCalledInfo(0);
        stub2.checkCalledInfo(0);

        mgr.containerModeUpdated(UpdateType.REMOVED);
        assertTrue(mgr.isActive());
        stub1.checkCalledInfo(0);
        stub2.checkCalledInfo(0);

        // remove Tenant
        st = mgr.removeTenant(tpath);
        assertTrue(st.isSuccess());
        stub1.checkCalledInfo(1, Boolean.FALSE);
        stub2.checkCalledInfo(1, Boolean.FALSE);

        // remove stub1
        mgr.removeVTNModeListener(stub1);
        mgr.notifyChange(true);
        stub1.checkCalledInfo(0);
        stub2.checkCalledInfo(1, Boolean.TRUE);

        // Append one more listener to catch mode change event.
        VTNModeListenerStub listener = new VTNModeListenerStub();
        mgr.addVTNModeListener(listener);
        listener.checkCalledInfo(1, Boolean.FALSE);

        mgr.removeVTNModeListener(stub2);
        mgr.notifyChange(true);
        stub1.checkCalledInfo(0);
        stub2.checkCalledInfo(0);

        // Ensure that mode change event is not delivered to a new listener.
        listener.checkCalledInfo(1, Boolean.TRUE);
        mgr.removeVTNModeListener(listener);

        // add in case that there is no tenant.
        mgr.addVTNModeListener(stub1);
        stub1.checkCalledInfo(1, Boolean.FALSE);

        mgr.removeVTNModeListener(stub1);
    }

    /**
     * Test method for
     * {@link VTNManagerImpl#addMacAddressTable(VBridgePath, int)},
     * {@link VTNManagerImpl#removeMacAddressTable(VBridgePath, boolean)},
     * {@link VTNManagerImpl#getMacAddressTable(VBridgePath)}
     */
    @Test
    public void testMacAddressTable() {
        VTNManagerImpl mgr = vtnMgr;
        MacAddressTable table = null;
        VBridgePath bpath = new VBridgePath("teannt", "bridge");
        int[] ages = new int[] {10, 600, 1000000};

        for (int age: ages) {
            mgr.addMacAddressTable(bpath, age);

            table = mgr.getMacAddressTable(bpath);
            assertNotNull(table);

            mgr.removeMacAddressTable(bpath, false);
        }

        table = mgr.getMacAddressTable(bpath);
        assertNull(table);

        mgr.removeMacAddressTable(bpath, false);
    }

    /**
     * Test method for
     * {@link VTNManagerImpl#addTenant(VTenantPath, VTenantConfig)},
     * {@link VTNManagerImpl#removeTenant(VTenantPath)},
     * {@link VTNManagerImpl#getTenants()},
     * {@link VTNManagerImpl#getTenant(VTenantPath)},
     * {@link VTNManagerImpl#isActive()}
     */
    @Test
    public void testAddGetRemoveTenant() {
        VTNManagerImpl mgr = vtnMgr;
        List<String> strings = createStrings("tenant", false);
        List<String> descs = createStrings("desc");
        List<Integer> ivs = createIntegers(-1, 4);
        List<Integer> hvs = createIntegers(-1, 4);

        strings.add(new String("123456789012345678901234567890_"));
        ivs.add(new Integer(65535));
        hvs.add(new Integer(65535));

        assertFalse(mgr.isActive());

        // test for add
        for (String tname : strings) {
            if (tname.isEmpty()) {
                // empty is invalid for tenant name.
                continue;
            }
            VTenantPath tpath = new VTenantPath(tname);

            for (String desc : descs) {
                for (Integer iv : ivs) {
                    for (Integer hv : hvs) {
                        VTenantConfig tconf = createVTenantConfig(desc, iv, hv);

                        Status st = mgr.addTenant(tpath, tconf);
                        if (iv != null && hv != null && iv.intValue() > 0 && hv.intValue() > 0
                                && iv.intValue() >= hv.intValue()) {
                            assertEquals(tconf.toString(), StatusCode.BADREQUEST, st.getCode());
                            continue;
                        } else if ((iv == null || iv.intValue() < 0) && hv != null &&
                                hv.intValue() > 0 && 300 >= hv.intValue()) {
                            assertEquals(tconf.toString(), StatusCode.BADREQUEST, st.getCode());
                            continue;
                        } else {
                            assertTrue(tconf.toString(), st.isSuccess());
                            assertTrue(tconf.toString(), mgr.isActive());
                        }

                        // getTenant()
                        VTenant tenant = null;
                        try {
                            tenant = mgr.getTenant(tpath);
                        } catch (Exception e) {
                            unexpected(e);
                        }
                        assertEquals(tname, tenant.getName());
                        assertEquals(tconf.getDescription(), tenant.getDescription());
                        if (iv == null || iv.intValue() < 0) {
                            assertEquals(tconf.toString(), 300, tenant.getIdleTimeout());
                        } else {
                            assertEquals(tconf.toString(), iv.intValue(), tenant.getIdleTimeout());
                        }
                        if (hv == null || hv.intValue() < 0) {
                            assertEquals(tconf.toString(), 0, tenant.getHardTimeout());
                        } else {
                            assertEquals(tconf.toString(), hv.intValue(), tenant.getHardTimeout());
                        }

                        // removeTenant()
                        mgr.removeTenant(tpath);
                        assertTrue(tconf.toString(), st.isSuccess());
                    }
                }
            }
        }

        try {
            List<VTenant> list = mgr.getTenants();
            assertTrue(list.size() == 0);
        } catch (Exception e) {
            unexpected(e);
        }

        assertFalse(mgr.isActive());
    }

    /**
     * Test method for
     * {@link VTNManagerImpl#modifyTenant(VTenantPath, VTenantConfig, boolean)}
     */
    @Test
    public void testModifyTenant() {
        VTNManagerImpl mgr = vtnMgr;
        List<String> tnames = new ArrayList<String>();
        List<String> descs = new ArrayList<String>();
        List<Integer> ivs = createIntegers(-1, 4);
        List<Integer> hvs = createIntegers(-1, 4);

        tnames.add(new String("Vtn"));
        tnames.add(new String("123456789012345678901234567890_"));
        descs.add(null);
        descs.add(new String("desc"));
        ivs.add(new Integer(65535));
        hvs.add(new Integer(65535));

        boolean first = true;
        for (String tname : tnames) {
            VTenantPath tpath = new VTenantPath(tname);
            VTenantConfig tconf = createVTenantConfig(new String("orig"), 20, 30);
            Status st = mgr.addTenant(tpath, tconf);

            for (String desc : descs) {
                for (Integer orgiv : ivs) {
                    for (Integer orghv : hvs) {
                        // check parameters
                        if (orgiv != null && orghv != null && orgiv.intValue() > 0 && orghv.intValue() > 0
                                && orgiv.intValue() >= orghv.intValue()) {
                            continue;
                        } else if ((orgiv == null || orgiv.intValue() < 0) && orghv != null && orghv.intValue() > 0
                                && 300 >= orghv.intValue()) {
                            continue;
                        }

                        String olddesc;
                        Integer oldiv;
                        Integer oldhv;
                        for (Integer iv : ivs) {
                            for (Integer hv : hvs) {
                                VTenant tenant = null;

                                if (first) {
                                    for (String ndesc : descs) {
                                        // test for all == true. executed at
                                        // first time only.
                                        tconf = createVTenantConfig(ndesc, iv, hv);
                                        st = mgr.modifyTenant(tpath, tconf, true);

                                        if (iv != null && hv != null &&
                                            iv.intValue() > 0 && hv.intValue() > 0 &&
                                            iv.intValue() >= hv.intValue()) {
                                            assertEquals(tconf.toString(),
                                                    StatusCode.BADREQUEST, st.getCode());
                                        } else if ((iv == null || iv < 0) &&
                                                hv != null && hv > 0 && 300 >= hv) {
                                            assertEquals(tconf.toString(),
                                                    StatusCode.BADREQUEST, st.getCode());
                                        } else {
                                            assertTrue(tconf.toString(), st.isSuccess());
                                        }

                                        try {
                                            tenant = mgr.getTenant(tpath);
                                        } catch (Exception e) {
                                            unexpected(e);
                                        }

                                        if (st.isSuccess()) {
                                            assertEquals(tconf.toString(),
                                                    tname, tenant.getName());
                                            assertEquals(tconf.toString(),
                                                    ndesc, tenant.getDescription());
                                            if (iv == null || iv.intValue() < 0) {
                                                assertEquals(tconf.toString(),
                                                        300, tenant.getIdleTimeout());
                                            } else {
                                                assertEquals(tconf.toString(),
                                                        iv.intValue(), tenant.getIdleTimeout());
                                            }
                                            if (hv == null || hv.intValue() < 0) {
                                                assertEquals(tconf.toString(),
                                                        0, tenant.getHardTimeout());
                                            } else {
                                                assertEquals(tconf.toString(),
                                                        hv.intValue(), tenant.getHardTimeout());
                                            }
                                        }
                                    }
                                }

                                tconf = createVTenantConfig(desc, orgiv, orghv);
                                st = mgr.modifyTenant(tpath, tconf, true);

                                olddesc = (desc == null) ? null : new String(desc);
                                oldiv = (orgiv == null || orgiv.intValue() < 0) ?
                                            new Integer(300) : orgiv;
                                oldhv = (orghv == null || orghv.intValue() < 0) ?
                                            new Integer(0) : orghv;

                                // all == false
                                tconf = createVTenantConfig(desc, iv, hv);
                                st = mgr.modifyTenant(tpath, tconf, false);

                                if ((iv == null || iv.intValue() < 0) && (hv == null || hv.intValue() < 0)) {
                                    // both are notset
                                    // not changed.
                                } else if (iv == null || iv.intValue() < 0) {
                                    // idle_timeout is notset
                                    if (hv.intValue() > 0
                                            && (oldiv.intValue() != 0 && oldiv.intValue() >= hv.intValue())) {
                                        assertEquals(tconf.toString(),
                                                StatusCode.BADREQUEST, st.getCode());
                                    } else {
                                        assertTrue(tconf.toString(), st.isSuccess());
                                    }
                                } else if (hv == null || hv.intValue() < 0) {
                                    // hard_timeout is notset
                                    if (iv > 0 && oldhv > 0) {
                                        if (iv >= oldhv) {
                                            assertEquals(tconf.toString(),
                                                    StatusCode.BADREQUEST, st.getCode());
                                        } else {
                                            assertTrue(tconf.toString(), st.isSuccess());
                                        }
                                    } else {
                                        assertTrue(tconf.toString(), st.isSuccess());
                                    }
                                } else {
                                    // both are set
                                    if (iv.intValue() > 0 && hv.intValue() > 0) {
                                        if (iv.intValue() >= hv.intValue()) {
                                            assertEquals(tconf.toString(),
                                                    StatusCode.BADREQUEST, st.getCode());
                                        } else {
                                            assertTrue(tconf.toString(), st.isSuccess());
                                        }
                                    } else {
                                        assertTrue(tconf.toString(), st.isSuccess());
                                    }
                                }

                                tenant = null;
                                try {
                                    tenant = mgr.getTenant(tpath);
                                } catch (Exception e) {
                                    unexpected(e);
                                }

                                if (st.isSuccess()) {
                                    assertEquals(tconf.toString(), tname, tenant.getName());
                                    assertEquals(tconf.toString(), olddesc, tenant.getDescription());
                                    if (iv == null || iv.intValue() < 0) {
                                        assertEquals(tconf.toString(),
                                                oldiv.intValue(), tenant.getIdleTimeout());
                                    } else {
                                        assertEquals(tconf.toString(),
                                                iv.intValue(), tenant.getIdleTimeout());
                                    }
                                    if (hv == null || hv.intValue() < 0) {
                                        assertEquals(tconf.toString(),
                                                oldhv.intValue(), tenant.getHardTimeout());
                                    } else {
                                        assertEquals(tconf.toString(),
                                                hv.intValue(), tenant.getHardTimeout());
                                    }
                                }

                                olddesc = tenant.getDescription();
                                oldiv = tenant.getIdleTimeout();
                                oldhv = tenant.getHardTimeout();
                            }
                        }
                        first = false;
                    }
                }
            }
        }

        try {
            List<VTenant> list = mgr.getTenants();
            assertTrue(list.size() == tnames.size());
        } catch (Exception e) {
            unexpected(e);
        }

        for (String tname : tnames) {
            VTenantPath tpath = new VTenantPath(tname);
            mgr.removeTenant(tpath);
        }
    }

    /**
     * Test method for invalid cases of
     * {@link VTNManagerImpl#addTenant(VTenantPath, VTenantConfig)},
     * {@link VTNManagerImpl#modifyTenant(VTenantPath, VTenantConfig, boolean)},
     * {@link VTNManagerImpl#removeTenant(VTenantPath)},
     * {@link VTNManagerImpl#getTenants()},
     * {@link VTNManagerImpl#getTenant(VTenantPath)}
     */
    @Test
    public void testTenantInvalidCase() {
        VTNManagerImpl mgr = vtnMgr;
        String name = "Tenant";
        String desc = "Description";
        VTenantPath tpath = new VTenantPath(name);
        VTenantConfig tconf = new VTenantConfig(desc);
        Status st = null;

        // bad request
        VTenantPath[] btplist = new VTenantPath[] {null, new VTenantPath(null)};
        VTenantConfig[] bcflist = new VTenantConfig[] {
                null, new VTenantConfig(desc, 65536, 65535),
                new VTenantConfig(desc, 65535, 65536)};

        for (VTenantPath path: btplist) {
            st = mgr.addTenant(path, tconf);
            assertEquals((path == null) ? "null" : path.toString(), StatusCode.BADREQUEST, st.getCode());
        }

        for (VTenantConfig conf: bcflist) {
            st = mgr.addTenant(tpath, conf);
            assertEquals((conf == null) ? "null" : conf.toString(), StatusCode.BADREQUEST, st.getCode());
        }

        st = mgr.addTenant(new VTenantPath(""), tconf);
        assertEquals(StatusCode.BADREQUEST, st.getCode());
        st = mgr.addTenant(new VTenantPath("12345678901234567890123456789012"), tconf);
        assertEquals(StatusCode.BADREQUEST, st.getCode());
        st = mgr.addTenant(new VTenantPath("tenant-"), tconf);
        assertEquals(StatusCode.BADREQUEST, st.getCode());
        st = mgr.addTenant(new VTenantPath("tenant:"), tconf);
        assertEquals(StatusCode.BADREQUEST, st.getCode());
        st = mgr.addTenant(new VTenantPath("tenant "), tconf);
        assertEquals(StatusCode.BADREQUEST, st.getCode());
        st = mgr.addTenant(new VTenantPath("_tenant "), tconf);
        assertEquals(StatusCode.BADREQUEST, st.getCode());


        st = mgr.addTenant(tpath, tconf);
        assertTrue(st.isSuccess());

        for (VTenantPath path: btplist) {
            st = mgr.modifyTenant(path, tconf, false);
            assertEquals((path == null) ? "null" : path.toString(),
                    StatusCode.BADREQUEST, st.getCode());

            st = mgr.removeTenant(path);
            assertEquals((path == null) ? "null" : path.toString(),
                    StatusCode.BADREQUEST, st.getCode());

            try {
                mgr.getTenant(path);
                fail("Throwing exception was expected.");
            } catch (VTNException e) {
               assertEquals((path == null) ? "null" : path.toString(),
                       StatusCode.BADREQUEST,e.getStatus().getCode());
            }
        }

        for (VTenantConfig conf: bcflist) {
            st = mgr.modifyTenant(tpath, conf, true);
            assertEquals((conf == null) ? "null" : conf.toString(),
                    StatusCode.BADREQUEST, st.getCode());
        }

        // confilct
        st = mgr.addTenant(tpath, tconf);
        assertEquals(StatusCode.CONFLICT, st.getCode());

        // not found
        VTenantPath lpath = new VTenantPath("12345678901234567890123456789012");
        st = mgr.modifyTenant(lpath, tconf, true);
        assertEquals(StatusCode.NOTFOUND, st.getCode());

        st = mgr.removeTenant(lpath);
        assertEquals(StatusCode.NOTFOUND, st.getCode());

        try {
            mgr.getTenant(lpath);
            fail("Throwing exception was expected.");
        } catch (VTNException e) {
           assertEquals(StatusCode.NOTFOUND, e.getStatus().getCode());
        }

        st = mgr.removeTenant(tpath);
        assertTrue(st.isSuccess());

        st = mgr.removeTenant(tpath);
        assertEquals(StatusCode.NOTFOUND, st.getCode());

        st = mgr.modifyTenant(tpath, tconf, true);
        assertEquals(StatusCode.NOTFOUND, st.getCode());

        st = mgr.removeTenant(tpath);
        assertEquals(StatusCode.NOTFOUND, st.getCode());

        // in Container mode
        mgr.containerModeUpdated(UpdateType.ADDED);
        st = mgr.addTenant(tpath, tconf);
        assertEquals(StatusCode.NOTACCEPTABLE, st.getCode());
        st = mgr.modifyTenant(tpath, tconf, true);
        assertEquals(StatusCode.NOTACCEPTABLE, st.getCode());
        st = mgr.removeTenant(tpath);
        assertEquals(StatusCode.NOTACCEPTABLE, st.getCode());
        mgr.containerModeUpdated(UpdateType.REMOVED);

    }

    /**
     * Test method for
     * {@link VTNManagerImpl#addBridge(VBridgePath, VBridgeConfig)},
     * {@link VTNManagerImpl#removeBridge(VBridgePath)},
     * {@link VTNManagerImpl#modifyBridge(VBridgePath, VBridgeConfig, boolean)},
     * {@link VTNManagerImpl#getBridges(VTenantPath)},
     * {@link VTNManagerImpl#getBridge(VBridgePath)}.
     */
    @Test
    public void testBridge() {
        VTNManagerImpl mgr = vtnMgr;
        List<Integer> ages = new ArrayList<Integer>();
        List<String> tlist = new ArrayList<String>();
        List<String> blist = createStrings("vbr", false);

        tlist.add("vtn");
        tlist.add("123456789012345678901234567890_");
        blist.add("012345678901234567890123456789_");
        ages.add(null);
        ages.add(10);
        ages.add(600);
        ages.add(1000000);

        boolean first = true;
        for (String tname : tlist) {
            VTenantPath tpath = new VTenantPath(tname);
            Status st = mgr.addTenant(tpath, new VTenantConfig(null));
            assertTrue(st.isSuccess());

            for (String bname : blist) {
                if (bname.isEmpty()) {
                    continue; // This is a invalid condition.
                }
                VBridgePath bpath = new VBridgePath(tname, bname);
                for (String desc : createStrings("desc")) {
                    for (Integer age : ages) {
                        VBridgeConfig bconf = createVBridgeConfig(desc, age);
                        st = mgr.addBridge(bpath, bconf);
                        assertTrue(bconf.toString(), st.isSuccess());

                        VBridge brdg = null;
                        try {
                            brdg = mgr.getBridge(bpath);
                        } catch (Exception e) {
                            unexpected(e);
                        }
                        assertEquals(bconf.toString(), bname, brdg.getName());
                        assertEquals(bconf.toString(), desc, brdg.getDescription());
                        if (age == null) {
                            assertEquals(bconf.toString(), 600, brdg.getAgeInterval());
                        } else {
                            assertEquals(bconf.toString(), age.intValue(), brdg.getAgeInterval());
                        }
                        assertEquals(bconf.toString(), VNodeState.UNKNOWN, brdg.getState());

                        String olddesc = brdg.getDescription();
                        int oldage = brdg.getAgeInterval();

                        for (String newdesc : createStrings("desc")) {
                            for (Integer newage : ages) {
                                bconf = createVBridgeConfig(newdesc, newage);
                                st = mgr.modifyBridge(bpath, bconf, false);

                                brdg = null;
                                try {
                                    brdg = mgr.getBridge(bpath);
                                } catch (Exception e) {
                                    unexpected(e);
                                }
                                assertEquals(bconf.toString(), bname, brdg.getName());
                                if (newdesc == null) {
                                    assertEquals(bconf.toString(),
                                            olddesc, brdg.getDescription());
                                } else {
                                    assertEquals(bconf.toString(),
                                            newdesc, brdg.getDescription());
                                    olddesc = newdesc;
                                }
                                if (newage == null) {
                                    assertEquals(bconf.toString(), oldage, brdg.getAgeInterval());
                                } else {
                                    assertEquals(bconf.toString(),
                                            newage.intValue(), brdg.getAgeInterval());
                                    oldage = newage.intValue();
                                }
                                assertEquals(bconf.toString(),
                                        VNodeState.UNKNOWN, brdg.getState());
                            }
                        }

                        st = mgr.removeBridge(bpath);
                        assertTrue(bpath.toString(), st.isSuccess());
                    }

                    if (first) {
                        VBridgeConfig bconf = new VBridgeConfig("desc", 10);
                        st = mgr.addBridge(bpath, bconf);
                        for (String newdesc : createStrings("desc")) {
                            for (Integer newage : ages) {
                                bconf = createVBridgeConfig(newdesc, newage);
                                st = mgr.modifyBridge(bpath, bconf, true);

                                VBridge brdg = null;
                                try {
                                    brdg = mgr.getBridge(bpath);
                                } catch (Exception e) {
                                    unexpected(e);
                                }
                                assertEquals(bconf.toString(), bname, brdg.getName());
                                assertEquals(bconf.toString(), newdesc, brdg.getDescription());
                                if (newage == null) {
                                    assertEquals(bconf.toString(), 600, brdg.getAgeInterval());
                                } else {
                                    assertEquals(bconf.toString(),
                                            newage.intValue(), brdg.getAgeInterval());
                                }
                                assertEquals(bconf.toString(),
                                        VNodeState.UNKNOWN, brdg.getState());
                            }
                        }

                        st = mgr.removeBridge(bpath);
                        assertTrue(bpath.toString(), st.isSuccess());

                        first = false;
                    }
                }
            }

            try {
                List<VBridge> list = mgr.getBridges(tpath);
                assertEquals(tpath.toString(), 0, list.size());
            } catch (VTNException e) {
                unexpected(e);
            }
            st = mgr.removeTenant(tpath);
            assertTrue(tpath.toString(), st.isSuccess());
        }

        // add mulitple entry.
        for (String tname : tlist) {
            VTenantPath tpath = new VTenantPath(tname);
            Status st = mgr.addTenant(tpath, new VTenantConfig(null));
            assertTrue(tpath.toString(), st.isSuccess());

            for (String bname : blist) {
                if (bname.isEmpty()) {
                    continue; // This is a invalid condition.
                }
                VBridgePath bpath = new VBridgePath(tname, bname);
                VBridgeConfig bconf = createVBridgeConfig(null, null);

                st = mgr.addBridge(bpath, bconf);
                assertTrue(bconf.toString(), st.isSuccess());
            }
            try {
                List<VBridge> list = mgr.getBridges(tpath);
                assertEquals(tpath.toString(), blist.size() - 1, list.size());
            } catch (VTNException e) {
                unexpected(e);
            }
            st = mgr.removeTenant(tpath);
            assertTrue(tpath.toString(), st.isSuccess());
        }
    }

    /**
     * Test method for invalid cases of
     * {@link VTNManagerImpl#addBridge(VBridgePath, VBridgeConfig)},
     * {@link VTNManagerImpl#removeBridge(VBridgePath)},
     * {@link VTNManagerImpl#modifyBridge(VBridgePath, VBridgeConfig, boolean)},
     * {@link VTNManagerImpl#getBridges(VTenantPath)},
     * {@link VTNManagerImpl#getBridge(VBridgePath)}.
     */
    @Test
    public void testBridgeInvalidCase() {
        VTNManagerImpl mgr = vtnMgr;

        // invalid case for addBridge()
        String tname = "tenant";
        VTenantPath tpath = new VTenantPath(tname);
        Status st = mgr.addTenant(tpath, new VTenantConfig(null));
        assertTrue(st.isSuccess());
        String bname = "bridge";
        VBridgePath bpath = new VBridgePath(tname, bname);
        VBridgeConfig bconf = createVBridgeConfig(null, null);

        VBridgePath[] bbplist = new VBridgePath[] {null, new VBridgePath(tname, null),
                                                    new VBridgePath((String)null, bname)};
        VBridgeConfig[] bcflist = new VBridgeConfig[] {null, new VBridgeConfig("desc", 9),
                new VBridgeConfig("desc", 1000001)};

        for (VBridgePath path : bbplist) {
            st = mgr.addBridge(path, bconf);
            assertEquals((path == null) ? "null" : path.toString(),
                    StatusCode.BADREQUEST, st.getCode());
        }

        st = mgr.addBridge(new VBridgePath(tname, ""), bconf);
        assertEquals(StatusCode.BADREQUEST, st.getCode());
        st = mgr.addBridge(new VBridgePath(tname, "123456789012345678901234567890_1"), bconf);
        assertEquals(StatusCode.BADREQUEST, st.getCode());
        st = mgr.addBridge(new VBridgePath(tname, "123456789012345678901234567890:"), bconf);
        assertEquals(StatusCode.BADREQUEST, st.getCode());
        st = mgr.addBridge(new VBridgePath(tname, "123456789012345678901234567890:"), bconf);
        assertEquals(StatusCode.BADREQUEST, st.getCode());
        st = mgr.addBridge(new VBridgePath(tname, "_1234567890"), bconf);
        assertEquals(StatusCode.BADREQUEST, st.getCode());

        for (VBridgeConfig conf : bcflist) {
            st = mgr.addBridge(bpath, conf);
            assertEquals((conf == null) ? "null" :conf.toString(),
                    StatusCode.BADREQUEST, st.getCode());
        }

        // add bridge before modifyBridge() and removeBridge()
        st = mgr.addBridge(bpath, bconf);
        assertTrue(st.isSuccess());

        for (VBridgePath path : bbplist) {
            st = mgr.modifyBridge(path, bconf, true);
            assertEquals((path == null) ? "null" :path.toString(),
                    StatusCode.BADREQUEST, st.getCode());
            st = mgr.removeBridge(path);
            assertEquals((path == null) ? "null" : path.toString(),
                    StatusCode.BADREQUEST, st.getCode());

            try {
                mgr.getBridge(path);
                fail("Throwing exception was expected.");
            } catch (VTNException e) {
                assertEquals((path == null) ? "null" : path.toString(),
                        StatusCode.BADREQUEST, e.getStatus().getCode());
            }
        }
        for (VBridgeConfig conf : bcflist) {
            st = mgr.modifyBridge(bpath, conf, false);
            assertEquals((conf == null) ? "null" : conf.toString(),
                    StatusCode.BADREQUEST, st.getCode());
        }

        try {
            mgr.getBridges(null);
            fail("Throwing exception was expected.");
        } catch (VTNException e) {
            assertEquals(StatusCode.BADREQUEST, e.getStatus().getCode());
        }

        try {
            mgr.getBridges(new VTenantPath(null));
            fail("Throwing exception was expected.");
        } catch (VTNException e) {
            assertEquals(StatusCode.BADREQUEST, e.getStatus().getCode());
        }

        // conflict
        st = mgr.addBridge(bpath, bconf);
        assertEquals(StatusCode.CONFLICT, st.getCode());

        // not found
        VBridgePath[] nbplist = new VBridgePath[]{new VBridgePath(tname, "bbbbb"),
                new VBridgePath("tt", bname)};
        for (VBridgePath path: nbplist) {
            st = mgr.removeBridge(path);
            assertEquals(StatusCode.NOTFOUND, st.getCode());

            st = mgr.modifyBridge(path, bconf, true);
            assertEquals(StatusCode.NOTFOUND, st.getCode());

            try {
                mgr.getBridge(path);
                fail("Throwing exception was expected.");
            } catch (VTNException e) {
                assertEquals(StatusCode.NOTFOUND, e.getStatus().getCode());
            }
        }

        st = mgr.addBridge(new VBridgePath("tt", bname), bconf);
        assertEquals(StatusCode.NOTFOUND, st.getCode());

        st = mgr.removeTenant(tpath);
        assertTrue(st.isSuccess());

        st = mgr.removeBridge(bpath);
        assertEquals(StatusCode.NOTFOUND, st.getCode());

        st = mgr.addBridge(bpath, bconf);
        assertEquals(StatusCode.NOTFOUND, st.getCode());

        try {
            mgr.getBridge(bpath);
            fail("Throwing exception was expected.");
        } catch (VTNException e) {
            assertEquals(StatusCode.NOTFOUND, e.getStatus().getCode());
        }

        try {
            mgr.getBridges(tpath);
            fail("Throwing exception was expected.");
        } catch (VTNException e) {
            assertEquals(StatusCode.NOTFOUND, e.getStatus().getCode());
        }

        // in Container mode
        mgr.containerModeUpdated(UpdateType.ADDED);
        st = mgr.addBridge(bpath, bconf);
        assertEquals(StatusCode.NOTACCEPTABLE, st.getCode());
        st = mgr.modifyBridge(bpath, bconf, true);
        assertEquals(StatusCode.NOTACCEPTABLE, st.getCode());
        st = mgr.removeBridge(bpath);
        assertEquals(StatusCode.NOTACCEPTABLE, st.getCode());
        mgr.containerModeUpdated(UpdateType.REMOVED);
    }

    /**
     * Test method for
     * {@link VTNManagerImpl#addBridgeInterface(VBridgeIfPath, VInterfaceConfig)},
     * {@link VTNManagerImpl#modifyBridgeInterface(VBridgeIfPath, VInterfaceConfig, boolean)},
     * {@link VTNManagerImpl#removeBridgeInterface(VBridgeIfPath)},
     * {@link VTNManagerImpl#getBridgeInterfaces(VBridgePath)},
     * {@link VTNManagerImpl#getBridgeInterface(VBridgeIfPath)}.
     */
    @Test
    public void testBridgeInterface() {
        VTNManagerImpl mgr = vtnMgr;
        List<String> tlist = new ArrayList<String>();
        List<String> blist = new ArrayList<String>();
        List<String> iflist = createStrings("vinterface", false);
        List<String> descs = new ArrayList<String>();

        tlist.add("vtn");
        blist.add("vbr");
        iflist.add("abcdefghijklmnoopqrstuvwxyz1234");
        descs.add(null);
        descs.add("description");

        for (String tname : tlist) {
            VTenantPath tpath = new VTenantPath(tname);
            Status st = mgr.addTenant(tpath, new VTenantConfig(null));
            assertTrue(tpath.toString(), st.isSuccess());

            for (String bname : blist) {
                VBridgePath bpath = new VBridgePath(tname, bname);
                VBridgeConfig bconf = createVBridgeConfig(null, null);
                st = mgr.addBridge(bpath, bconf);
                assertTrue(bpath.toString(), st.isSuccess());

                List<VInterface> list = null;
                try {
                    list = mgr.getBridgeInterfaces(bpath);
                } catch (Exception e) {
                    unexpected(e);
                }
                assertTrue(list.size() == 0);

                for (String ifname : iflist) {
                    if (ifname.isEmpty()) {
                        continue; // This is a invalid condition.
                    }

                    VBridgeIfPath ifp = new VBridgeIfPath(tname, bname, ifname);
                    for (String desc : createStrings("desc")) {
                        for (Boolean enabled : createBooleans()) {
                            VInterfaceConfig ifconf = new VInterfaceConfig(desc, enabled);

                            st = mgr.addBridgeInterface(ifp, ifconf);
                            assertTrue(ifp.toString() + "," + ifconf.toString(),
                                    st.isSuccess());

                            VInterface vif = null;
                            try {
                                vif = mgr.getBridgeInterface(ifp);
                            } catch (Exception e) {
                                unexpected(e);
                            }

                            assertEquals(ifp.toString() + "," + ifconf.toString(),
                                    ifname, vif.getName());
                            assertEquals(ifp.toString() + "," + ifconf.toString(),
                                    desc, vif.getDescription());
                            if (enabled == null) {
                                assertEquals(ifp.toString() + "," + ifconf.toString(),
                                        Boolean.TRUE, vif.getEnabled());
                            } else {
                                assertEquals(ifp.toString() + "," + ifconf.toString(),
                                        enabled, vif.getEnabled());
                            }
                            if (enabled == Boolean.FALSE) {
                                assertEquals(ifp.toString() + "," + ifconf.toString(),
                                        VNodeState.DOWN, vif.getState());
                            } else {
                                assertEquals(ifp.toString() + "," + ifconf.toString(),
                                        VNodeState.UNKNOWN, vif.getState());
                            }

                            VBridge brdg = null;
                            try {
                                brdg = mgr.getBridge(bpath);
                            } catch (VTNException e) {
                               unexpected(e);
                            }
                            assertEquals(ifp.toString() + "," + ifconf.toString(),
                                    VNodeState.UNKNOWN, brdg.getState());

                            st = mgr.removeBridgeInterface(ifp);
                            assertTrue(ifp.toString() + "," + ifconf.toString(), st.isSuccess());
                        }
                    }

                    // for modify(false)
                    st = mgr.addBridgeInterface(ifp, new VInterfaceConfig("desc", Boolean.FALSE));
                    assertTrue(st.isSuccess());

                    String olddesc = new String("desc");
                    Boolean oldenabled = Boolean.FALSE;
                    for (String desc : descs) {
                        for (Boolean enabled : createBooleans()) {
                            VInterfaceConfig ifconf = new VInterfaceConfig(desc, enabled);
                            st = mgr.modifyBridgeInterface(ifp, ifconf, false);
                            assertTrue(ifp.toString() + "," + ifconf.toString(), st.isSuccess());

                            VInterface vif = null;
                            try {
                                vif = mgr.getBridgeInterface(ifp);
                            } catch (Exception e) {
                                unexpected(e);
                            }

                            assertEquals(ifp.toString() + "," + ifconf.toString(),
                                    ifname, vif.getName());

                            if (desc == null) {
                                assertEquals(ifp.toString() + "," + ifconf.toString(),
                                        olddesc, vif.getDescription());
                            } else {
                                assertEquals(ifp.toString() + "," + ifconf.toString(),
                                        desc, vif.getDescription());
                            }

                            Boolean currenabled = enabled;
                            if (enabled == null) {
                                assertEquals(ifp.toString() + "," + ifconf.toString(),
                                        oldenabled, vif.getEnabled());
                                currenabled = oldenabled;
                            } else {
                                assertEquals(ifp.toString() + "," + ifconf.toString(),
                                        enabled, vif.getEnabled());
                            }

                            if (currenabled == Boolean.FALSE) {
                                assertEquals(ifp.toString() + "," + ifconf.toString(),
                                        VNodeState.DOWN, vif.getState());
                            } else {
                                assertEquals(ifp.toString() + "," + ifconf.toString(),
                                        VNodeState.UNKNOWN, vif.getState());
                            }
                            VBridge brdg = null;
                            try {
                                brdg = mgr.getBridge(bpath);
                            } catch (VTNException e) {
                               unexpected(e);
                            }
                            assertEquals(ifp.toString() + "," + ifconf.toString(),
                                    VNodeState.UNKNOWN, brdg.getState());

                            olddesc = vif.getDescription();
                            oldenabled = vif.getEnabled();
                        }
                    }

                    // for modify(true)
                    st = mgr.modifyBridgeInterface(ifp, new VInterfaceConfig("desc", Boolean.FALSE), true);
                    assertTrue(ifp.toString(), st.isSuccess());
                    for (String desc : descs) {
                        for (Boolean enabled : createBooleans()) {
                            VInterfaceConfig ifconf = new VInterfaceConfig(desc, enabled);
                            st = mgr.modifyBridgeInterface(ifp, ifconf, true);
                            assertTrue(ifp.toString() + "," + ifconf.toString(), st.isSuccess());

                            VInterface vif = null;
                            try {
                                vif = mgr.getBridgeInterface(ifp);
                            } catch (Exception e) {
                                unexpected(e);
                            }

                            assertEquals(ifname, vif.getName());
                            assertEquals(desc, vif.getDescription());
                            if (enabled == null) {
                                assertEquals(ifp.toString() + "," + ifconf.toString(),
                                        Boolean.TRUE, vif.getEnabled());
                            } else {
                                assertEquals(ifp.toString() + "," + ifconf.toString(),
                                        enabled, vif.getEnabled());
                            }

                            if (enabled == Boolean.FALSE) {
                                assertEquals(ifp.toString() + "," + ifconf.toString(),
                                        VNodeState.DOWN, vif.getState());
                            } else {
                                assertEquals(ifp.toString() + "," + ifconf.toString(),
                                        VNodeState.UNKNOWN, vif.getState());
                            }

                            VBridge brdg = null;
                            try {
                                brdg = mgr.getBridge(bpath);
                            } catch (VTNException e) {
                               unexpected(e);
                            }
                            assertEquals(ifp.toString() + "," + ifconf.toString(),
                                    VNodeState.UNKNOWN, brdg.getState());
                        }
                    }
                }

                list = null;
                try {
                    list = mgr.getBridgeInterfaces(bpath);
                } catch (Exception e) {
                    unexpected(e);
                }
                assertTrue((iflist.size() - 1) == list.size());
            }

            st = mgr.removeTenant(tpath);
            assertTrue(tpath.toString(), st.isSuccess());
        }
    }

    /**
     * Test method for invalid cases of
     * {@link VTNManagerImpl#addBridgeInterface(VBridgeIfPath, VInterfaceConfig)},
     * {@link VTNManagerImpl#modifyBridgeInterface(VBridgeIfPath, VInterfaceConfig, boolean)},
     * {@link VTNManagerImpl#removeBridgeInterface(VBridgeIfPath)},
     * {@link VTNManagerImpl#getBridgeInterfaces(VBridgePath)},
     * {@link VTNManagerImpl#getBridgeInterface(VBridgeIfPath)}
     */
    @Test
    public void testBridgeInterfaceInvalidCase() {
        VTNManagerImpl mgr = vtnMgr;

        String tname = "tenant";
        VTenantPath tpath = new VTenantPath(tname);
        Status st = mgr.addTenant(tpath, new VTenantConfig(null));
        assertTrue(st.isSuccess());
        String bname = "bridge";
        VBridgePath bpath = new VBridgePath(tname, bname);
        st = mgr.addBridge(bpath, new VBridgeConfig(null));
        assertTrue(st.isSuccess());
        String ifname = "interface";
        VBridgeIfPath ifpath = new VBridgeIfPath(tname, bname, ifname);
        VInterfaceConfig ifconf = new VInterfaceConfig("desc", Boolean.FALSE);

        VBridgeIfPath[] ifplist = new VBridgeIfPath[] {
                null, new VBridgeIfPath(tname, bname, null),
                new VBridgeIfPath(tname, null, ifname),
                new VBridgeIfPath(null, bname, ifname),
                };
        VInterfaceConfig[] ifclist = new VInterfaceConfig[] {null};

        for (VBridgeIfPath path : ifplist) {
            st = mgr.addBridgeInterface(path, ifconf);
            assertEquals((path == null) ? "null" : path.toString(),
                    StatusCode.BADREQUEST, st.getCode());
        }

        for (VInterfaceConfig conf : ifclist) {
            st = mgr.addBridgeInterface(ifpath, conf);
            assertEquals((conf == null) ? "null" : conf.toString(),
                    StatusCode.BADREQUEST, st.getCode());
        }

        // addBridgeInterface() also return BADREQUEST in these case.
        st = mgr.addBridgeInterface(new VBridgeIfPath(tname, bname, ""), ifconf);
        assertEquals(StatusCode.BADREQUEST, st.getCode());
        st = mgr.addBridgeInterface(new VBridgeIfPath(tname, bname, "123456789012345678901234567890_1"), ifconf);
        assertEquals(StatusCode.BADREQUEST, st.getCode());
        st = mgr.addBridgeInterface(new VBridgeIfPath(tname, bname, "123456789012345678901234567890:"), ifconf);
        assertEquals(StatusCode.BADREQUEST, st.getCode());
        st = mgr.addBridgeInterface(new VBridgeIfPath(tname, bname, "123456789012345678901234567890 "), ifconf);
        assertEquals(StatusCode.BADREQUEST, st.getCode());
        st = mgr.addBridgeInterface(new VBridgeIfPath(tname, bname, "_1234567890"), ifconf);
        assertEquals(StatusCode.BADREQUEST, st.getCode());

        // add Interface before modify and remove()
        st = mgr.addBridgeInterface(ifpath, ifconf);
        assertTrue(st.isSuccess());

        for (VBridgeIfPath path : ifplist) {
            st = mgr.modifyBridgeInterface(path, ifconf, false);
            assertEquals((path == null) ? "null" : path.toString(),
                    StatusCode.BADREQUEST, st.getCode());
            st = mgr.removeBridgeInterface(path);
            assertEquals((path == null) ? "null" : path.toString(),
                    StatusCode.BADREQUEST, st.getCode());

            try {
                mgr.getBridgeInterface(path);
                fail("Throwing exception was expected.");
            } catch (VTNException e) {
                assertEquals((path == null) ? "null" : path.toString(),
                        StatusCode.BADREQUEST, e.getStatus().getCode());
            }
        }

        for (VInterfaceConfig conf : ifclist) {
            st = mgr.modifyBridgeInterface(ifpath, conf, true);
            assertEquals((conf == null) ? "null" : conf.toString(),
                    StatusCode.BADREQUEST, st.getCode());
        }

        try {
            mgr.getBridgeInterfaces(null);
            fail("Throwing exception was expected.");
        } catch (VTNException e) {
            assertEquals(StatusCode.BADREQUEST, e.getStatus().getCode());
        }

        // conflict
        st = mgr.addBridgeInterface(ifpath, ifconf);
        assertEquals(StatusCode.CONFLICT, st.getCode());

        // not found
        VBridgeIfPath[] nbplist = new VBridgeIfPath[] {
                new VBridgeIfPath(tname, bname, "i"),
                new VBridgeIfPath(tname, "bbbbb", ifname),
                new VBridgeIfPath("tt", bname, ifname)
        };

        for (VBridgeIfPath path: nbplist) {
            st = mgr.removeBridgeInterface(path);
            assertEquals((path == null) ? "null" : path.toString(),
                    StatusCode.NOTFOUND, st.getCode());

            st = mgr.modifyBridgeInterface(path, ifconf, false);
            assertEquals((path == null) ? "null" : path.toString(),
                    StatusCode.NOTFOUND, st.getCode());

            if (!(path.getTenantName().equals(tname) &&
                    path.getBridgeName().equals(bname))) {
                st = mgr.addBridgeInterface(path, ifconf);
                assertEquals((path == null) ? "null" : path.toString(),
                        StatusCode.NOTFOUND, st.getCode());
            }

            try {
                mgr.getBridgeInterface(path);
                fail("Throwing exception was expected.");
            } catch (VTNException e) {
                assertEquals((path == null) ? "null" : path.toString(),
                        StatusCode.NOTFOUND, e.getStatus().getCode());
            }
        }

        st = mgr.removeTenant(tpath);
        assertTrue(st.isSuccess());

        st = mgr.removeBridgeInterface(ifpath);
        assertEquals(StatusCode.NOTFOUND, st.getCode());

        st = mgr.addBridgeInterface(ifpath, ifconf);
        assertEquals(StatusCode.NOTFOUND, st.getCode());

        try {
            mgr.getBridgeInterface(ifpath);
            fail("Throwing exception was expected.");
        } catch (VTNException e) {
            assertEquals(StatusCode.NOTFOUND, e.getStatus().getCode());
        }

        try {
            mgr.getBridgeInterfaces(bpath);
            fail("Throwing exception was expected.");
        } catch (VTNException e) {
            assertEquals(StatusCode.NOTFOUND, e.getStatus().getCode());
        }

        // in container mode
        mgr.containerModeUpdated(UpdateType.ADDED);
        st = mgr.addBridgeInterface(ifpath, ifconf);
        assertEquals(StatusCode.NOTACCEPTABLE, st.getCode());
        st = mgr.modifyBridgeInterface(ifpath, ifconf, true);
        assertEquals(StatusCode.NOTACCEPTABLE, st.getCode());
        st = mgr.removeBridgeInterface(ifpath);
        assertEquals(StatusCode.NOTACCEPTABLE, st.getCode());
        mgr.containerModeUpdated(UpdateType.REMOVED);
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

        // add a vlanmap to a vbridge
        for (Node node : createNodes(3)) {
            for (short vlan : vlans) {
                VlanMapConfig vlconf = new VlanMapConfig(node, vlan);
                VlanMap map = null;
                try {
                    map = mgr.addVlanMap(bpath, vlconf);
                    if (node != null && node.getType() != NodeConnector.NodeConnectorIDType.OPENFLOW) {
                        fail("throwing Exception was expected." + vlconf.toString());
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
                assertEquals(vlconf.toString(), getmap.getId(), map.getId());
                assertEquals(vlconf.toString(), getmap.getNode(), node);
                assertEquals(vlconf.toString(), getmap.getVlan(), vlan);

                VBridge brdg = null;
                try {
                    brdg = mgr.getBridge(bpath);
                } catch (VTNException e) {
                   unexpected(e);
                }
                assertEquals((node == null) ? VNodeState.UP : VNodeState.DOWN, brdg.getState());

                st = mgr.removeVlanMap(bpath, map.getId());
                assertTrue(vlconf.toString(), st.isSuccess());
            }
        }

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
                        continue;
                    }
                } catch (Exception e) {
                    unexpected(e);
                }
            }

            List<VlanMap> list = null;
            try {
                list = mgr.getVlanMaps(bpath);
            } catch (Exception e) {
                unexpected(e);
            }
            if (node == null || node.getType() == NodeConnector.NodeConnectorIDType.OPENFLOW) {
                assertTrue((node == null) ? "null" : node.toString(),
                        list.size() == vlans.length);
                VBridge brdg = null;
                try {
                    brdg = mgr.getBridge(bpath);
                } catch (VTNException e) {
                   unexpected(e);
                }
                if (node == null) {
                    assertEquals(VNodeState.UP, brdg.getState());
                } else {
                    assertEquals(node.toString(), VNodeState.DOWN, brdg.getState());
                }
            } else {
                assertTrue(node.toString(), list.size() == 0);
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
                assertTrue((node == null) ? "null" : node.toString() + "," + map.toString(),
                        st.isSuccess());
            }
        }

        st = mgr.removeTenant(tpath);
        assertTrue(st.isSuccess());
    }


    /**
     * Test method for invalid cases of
     * {@link VTNManagerImpl#addVlanMap(VBridgePath, VlanMapConfig)},
     * {@link VTNManagerImpl#removeVlanMap(VBridgePath, java.lang.String)},
     * {@link VTNManagerImpl#getVlanMap(VBridgePath, java.lang.String)},
     * {@link VTNManagerImpl#getVlanMaps(VBridgePath)}.
     */
    @Test
    public void testVlanMapInvalidCase() {
        VTNManagerImpl mgr = vtnMgr;
        Status st = null;
        String tname = "vtn";
        VTenantPath tpath = new VTenantPath(tname);
        String bname = "vbridge";
        VBridgePath bpath = new VBridgePath(tname, bname);
        String bname2 = "vbridge2";
        VBridgePath bpath2 = new VBridgePath(tname, bname2);

        String ifname = "vinterface";
        VBridgeIfPath ifp = new VBridgeIfPath(tname, bname, ifname);

        List<VBridgePath> bpathlist = new ArrayList<VBridgePath>();
        List<VBridgeIfPath> ifplist = new ArrayList<VBridgeIfPath>();
        bpathlist.add(bpath);
        bpathlist.add(bpath2);
        ifplist.add(ifp);
        createTenantAndBridgeAndInterface(mgr, tpath, bpathlist, ifplist);

        Node node = NodeCreator.createOFNode(0L);
        Node node1 = NodeCreator.createOFNode(1L);
        Node node2 = NodeCreator.createOFNode(2L);

        VlanMap map = null;

        // conflict case
        try {
            map = mgr.addVlanMap(bpath, new VlanMapConfig(null, (short) 0));
        } catch (Exception e) {
            unexpected(e);
        }

        map = null;
        try {
            map = mgr.addVlanMap(bpath, new VlanMapConfig(null, (short) 0));
            fail("Expected to throw Exception.");
        } catch (VTNException e) {
            assertEquals(StatusCode.CONFLICT, e.getStatus().getCode());
        }

        try {
            map = mgr.addVlanMap(bpath2, new VlanMapConfig(null, (short) 0));
            fail("Expected to throw Exception.");
        } catch (VTNException e) {
            assertEquals(StatusCode.CONFLICT, e.getStatus().getCode());
        }

        map = null;
        try {
            map = mgr.addVlanMap(bpath, new VlanMapConfig(node, (short) 0));
            fail("Expected to throw Exception.");
        } catch (VTNException e) {
            assertEquals(StatusCode.CONFLICT, e.getStatus().getCode());
        }

        try {
            map = mgr.addVlanMap(bpath2, new VlanMapConfig(node, (short) 0));
            fail("Expected to throw Exception.");
        } catch (VTNException e) {
            assertEquals(StatusCode.CONFLICT, e.getStatus().getCode());
        }

        map = null;
        try {
            map = mgr.addVlanMap(bpath, new VlanMapConfig(node, (short) 1));
        } catch (Exception e) {
            unexpected(e);
        }

        // if mapped node is not exist, duplicate vlanmap success.
        try {
            map = mgr.addVlanMap(bpath, new VlanMapConfig(node1, (short) 1));
            map = mgr.addVlanMap(bpath, new VlanMapConfig(node2, (short) 1));
        } catch (Exception e) {
            unexpected(e);
        }
        st = mgr.removeVlanMap(bpath, map.getId());
        assertTrue("status=" + st.toString(), st.isSuccess());

        map = null;
        try {
            map = mgr.addVlanMap(bpath, new VlanMapConfig(null, (short) 1));
            fail("Expected to throw Exception.");
        } catch (VTNException e) {
            assertEquals(StatusCode.CONFLICT, e.getStatus().getCode());
        }

        // invalid case
        VBridgeConfig bconf = new VBridgeConfig(null);
        st = mgr.removeBridge(bpath);
        assertTrue(st.isSuccess());
        st = mgr.addBridge(bpath, bconf);
        assertTrue(st.isSuccess());
        node = NodeCreator.createOFNode(0L);

        // bad request
        VlanMapConfig vlconf = new VlanMapConfig(null, (short) 0);
        VBridgePath[] bbplist = new VBridgePath[] {null,
                new VBridgePath(tname, null), new VBridgePath((String) null, bname)};
        VlanMapConfig[] bvclist = new VlanMapConfig[] {
                null, new VlanMapConfig(node, (short) -1), new VlanMapConfig(node, (short) 4096)};

        for (VBridgePath path: bbplist) {
            try {
                map = mgr.addVlanMap(path, vlconf);
                fail("Throwing Exception was expected.");
            } catch (VTNException e) {
                assertEquals((path == null) ? "null" : path.toString(),
                        StatusCode.BADREQUEST, e.getStatus().getCode());
            }
            try {
                map = mgr.addVlanMap(path, new VlanMapConfig(null, (short) 0));
                fail("Throwing Exception was expected.");
            } catch (VTNException e) {
                assertEquals((path == null) ? "null" : path.toString(),
                        StatusCode.BADREQUEST, e.getStatus().getCode());
            }
        }

        for (VlanMapConfig conf: bvclist) {
            try {
                map = mgr.addVlanMap(bpath, conf);
                fail("Throwing Exception was expected." + conf.toString());
            } catch (VTNException e) {
                assertEquals((conf == null) ? "null" : conf.toString(),
                        StatusCode.BADREQUEST, e.getStatus().getCode());
            }
        }

        // add to execute getVlanMap() and removeVlanMap() test.
        try {
            map = mgr.addVlanMap(bpath, new VlanMapConfig(node, (short) 4095));
        } catch (Exception e) {
            unexpected(e);
        }

        try {
            map = mgr.getVlanMap(bpath, null);
            fail("Throwing Exception was expected.");
        } catch (VTNException e) {
            assertEquals(StatusCode.BADREQUEST, e.getStatus().getCode());
        }

        st = mgr.removeVlanMap(bpath, null);
        assertEquals(StatusCode.BADREQUEST, st.getCode());

        List<VlanMap> list = null;
        for (VBridgePath path: bbplist) {
            try {
                map = mgr.getVlanMap(path, map.getId());
                fail("Throwing Exception was expected.");
            } catch (VTNException e) {
                assertEquals((path == null) ? "null" : path.toString(),
                        StatusCode.BADREQUEST, e.getStatus().getCode());
            }

            try {
                list = mgr.getVlanMaps(path);
                fail("Throwing Exception was expected.");
            } catch (VTNException e) {
                assertEquals((path == null) ? "null" : path.toString(),
                        StatusCode.BADREQUEST, e.getStatus().getCode());
            }

            st = mgr.removeVlanMap(path, map.getId());
            assertEquals((path == null) ? "null" : path.toString(),
                    StatusCode.BADREQUEST, st.getCode());
        }

        // case Node == null is executed in {@link testVlanMap()}.

        // not found
        VBridgePath[] nbplist = new VBridgePath[] {
                new VBridgePath(tname, "vbridg"), new VBridgePath("vtn0", bname)};

        for (VBridgePath path: nbplist) {
            try {
                map = mgr.addVlanMap(path, new VlanMapConfig(node, (short) 0));
                fail("Throwing Exception was expected.");
            } catch (VTNException e) {
                assertEquals((path == null) ? "null" : path.toString(),
                        StatusCode.NOTFOUND, e.getStatus().getCode());
            }

            try {
                map = mgr.getVlanMap(path, map.getId());
                fail("Throwing Exception was expected.");
            } catch (VTNException e) {
                assertEquals((path == null) ? "null" : path.toString(),
                        StatusCode.NOTFOUND, e.getStatus().getCode());
            }

            try {
                list = mgr.getVlanMaps(path);
                fail("Throwing Exception was expected.");
            } catch (VTNException e) {
                assertEquals((path == null) ? "null" : path.toString(),
                        StatusCode.NOTFOUND, e.getStatus().getCode());
            }

            st = mgr.removeVlanMap(path, map.getId());
            assertEquals((path == null) ? "null" : path.toString(),
                    StatusCode.NOTFOUND, st.getCode());
        }

        st = mgr.removeVlanMap(bpath, map.getId());
        assertEquals(StatusCode.SUCCESS, st.getCode());

        st = mgr.removeVlanMap(bpath, map.getId());
        assertEquals(StatusCode.NOTFOUND, st.getCode());

        try {
            map = mgr.getVlanMap(bpath, map.getId());
            fail("Throwing Exception was expected.");
        } catch (VTNException e) {
            assertEquals(StatusCode.NOTFOUND, e.getStatus().getCode());
        }

        // in container mode
        mgr.containerModeUpdated(UpdateType.ADDED);
        try {
            map = mgr.addVlanMap(bpath, vlconf);
            fail("Throwing Exception was expected.");
        } catch (VTNException e) {
            assertEquals(StatusCode.NOTACCEPTABLE, e.getStatus().getCode());
        }
        st = mgr.removeVlanMap(bpath, "");
        assertEquals(StatusCode.NOTACCEPTABLE, st.getCode());
        mgr.containerModeUpdated(UpdateType.REMOVED);

        st = mgr.removeTenant(tpath);
        assertTrue(st.isSuccess());
    }

    /**
     * Test method for {@link VTNManagerImpl#getPortMap(VBridgeIfPath)} and
     * {@link VTNManagerImpl#setPortMap(VBridgeIfPath, PortMapConfig)}.
     */
    @Test
    public void testPortMap() {
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
                assertEquals(pmconf, map.getConfig());
                assertNull(pmconf.toString(), map.getNodeConnector());

                VInterface bif = null;
                try {
                    bif = mgr.getBridgeInterface(ifp);
                } catch (VTNException e) {
                    unexpected(e);
                }
                assertEquals(pmconf.toString(), VNodeState.DOWN, bif.getState());

                VBridge brdg = null;
                try {
                    brdg = mgr.getBridge(bpath);
                } catch (VTNException e) {
                   unexpected(e);
                }
                assertEquals(pmconf.toString(), VNodeState.DOWN, brdg.getState());
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

        // assign mutli portmaps to a vbridge
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

        SwitchPort port2 = new SwitchPort( NodeConnector.NodeConnectorIDType.OPENFLOW, "11");
        PortMapConfig pmconf2 = new PortMapConfig(node1, port2, (short) 0);
        st = mgr.setPortMap(ifp2, pmconf2);
        assertTrue(st.isSuccess());


        // if specified port is not exist, duplicate portmap success.
        String ifname3 = "vinterface3";
        VBridgeIfPath ifp3 = new VBridgeIfPath(tname, bname2, ifname3);
        st = mgr.addBridgeInterface(ifp3, new VInterfaceConfig(null, true));
        st = mgr.setPortMap(ifp3, pmconf1);
        assertTrue(st.isSuccess());

        st = mgr.removeTenant(tpath);
        assertTrue(st.isSuccess());
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
                new SwitchPort("port-2", NodeConnector.NodeConnectorIDType.OPENFLOW, null),
                new SwitchPort(null, null, "16"),
                new SwitchPort(NodeConnector.NodeConnectorIDType.ONEPK, "10"),
        };
        PortMapConfig[] bpmlist = new PortMapConfig[] {
                new PortMapConfig(null, port, (short) 0),
                new PortMapConfig(node, null, (short) 0),
                new PortMapConfig(node, null, (short) 0),
                new PortMapConfig(node, port, (short)-1)};

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
        st = mgr.setPortMap(ifp, new PortMapConfig(pnode, new SwitchPort("port-1"), (short) 0));
        assertEquals(StatusCode.BADREQUEST, st.getCode());

        // not found
        VBridgeIfPath[] niflist = new VBridgeIfPath[] {
                new VBridgeIfPath(tname, bname, "ii"),
                new VBridgeIfPath(tname, "bbb", ifname),
                new VBridgeIfPath("vv", bname, ifname)};

        for (VBridgeIfPath path: niflist) {
            st = mgr.setPortMap(path, pmconf);
            assertEquals(StatusCode.NOTFOUND, st.getCode());
            try {
                mgr.getPortMap(path);
                fail("throwing exception was expected.");
            } catch (VTNException e) {
                assertEquals(path.toString(), StatusCode.NOTFOUND, e.getStatus().getCode());
            }
        }

        // in container mode
        mgr.containerModeUpdated(UpdateType.ADDED);
        st = mgr.setPortMap(ifp, pmconf);
        assertEquals(StatusCode.NOTACCEPTABLE, st.getCode());
        mgr.containerModeUpdated(UpdateType.REMOVED);

        st = mgr.removeTenant(tpath);
        assertTrue(st.isSuccess());
    }

    /**
     * Test method for
     * {@link VTNManagerImpl#getMacEntries(VBridgePath)},
     * {@link VTNManagerImpl#getMacEntries(VBridgePath)},
     * {@link VTNManagerImpl#getMacEntry(VBridgePath, DataLinkAddress)},
     * {@link VTNManagerImpl#removeMacEntry(VBridgePath, DataLinkAddress)},
     * {@link VTNManagerImpl#flushMacEntries(VBridgePath)}
     */
    @Test
    public void testMacEntry() {
        VTNManagerImpl mgr = vtnMgr;
        short[] vlans = new short[] { 0, 10, 4095 };
        String tname = "vtn";
        VTenantPath tpath = new VTenantPath(tname);
        String bname = "vbridge";
        VBridgePath bpath = new VBridgePath(tname, bname);
        String bname2 = "vbridge2";
        VBridgePath bpath2 = new VBridgePath(tname, bname2);
        List<VBridgePath> bpathlist = new ArrayList<VBridgePath>();
        bpathlist.add(bpath);
        bpathlist.add(bpath2);

        createTenantAndBridge(mgr, tpath, bpathlist);

        MacAddressTable tbl = mgr.getMacAddressTable(bpath);
        MacAddressTable tbl2 = mgr.getMacAddressTable(bpath2);

        List<EthernetAddress> ethers = createEthernetAddresses(false);
        List<NodeConnector> connectors = createNodeConnectors(3, false);
        NodeConnector nc = connectors.get(0);
        byte iphost = 1;
        short vlan = -1;
        short vlan2 = 4095;

        for (EthernetAddress ea: ethers) {
            byte[] bytes = ea.getValue();
            byte[] src = new byte[] {bytes[0], bytes[1], bytes[2], bytes[3], bytes[4], bytes[5]};
            byte[] dst = new byte[] {(byte)0xFF, (byte)0xFF, (byte)0xFF, (byte)0xFF, (byte)0xFF, (byte)0xFF};
            byte[] sender = new byte[] {(byte)192, (byte)168, (byte)0, (byte)iphost};
            byte[] target = new byte[] {(byte)192, (byte)168, (byte)0, (byte)250};
            byte[] src2 = new byte[] {00, bytes[4], bytes[3], bytes[2], bytes[1], bytes[0]};
            byte[] dst2 = new byte[] {(byte)0xFF, (byte)0xFF, (byte)0xFF, (byte)0xFF, (byte)0xFF, (byte)0xFF};
            byte[] sender2 = new byte[] {(byte)192, (byte)1, (byte)0, (byte)iphost};
            byte[] target2 = new byte[] {(byte)192, (byte)1, (byte)0, (byte)250};
            EthernetAddress ea2 = null;
            try {
                 ea2 = new EthernetAddress(src2);
            } catch (ConstructionException e1) {
                unexpected(e1);
            }

            PacketContext pctx = createARPPacketContext(src, dst, sender, target,
                                    (vlan > 0) ? vlan : -1, connectors.get(0), ARP.REQUEST);
            PacketContext pctx2 = createARPPacketContext(src2, dst2, sender2, target2,
                                    (vlan2 > 0) ? vlan2 : -1, connectors.get(0), ARP.REQUEST);

            tbl.add(pctx);
            tbl2.add(pctx2);

            MacAddressEntry entry = null;
            try {
                entry = mgr.getMacEntry(bpath, ea);
            } catch (VTNException e) {
                unexpected(e);
            }
            assertEquals(ea.toString(), ea, entry.getAddress());
            assertEquals(ea.toString(), ((vlan < 0) ? (short)0 : vlan), entry.getVlan());
            assertEquals(ea.toString(), nc, entry.getNodeConnector());

            Set<InetAddress> ips = entry.getInetAddresses();
            assertArrayEquals(ea.toString(), sender, ips.iterator().next().getAddress());

            try {
                entry = mgr.getMacEntry(bpath, ea2);
            } catch (VTNException e) {
                unexpected(e);
            }
            assertNull(ea.toString(), entry);

            try {
                entry = mgr.getMacEntry(bpath2, ea2);
            } catch (VTNException e) {
                unexpected(e);
            }
            assertEquals(ea.toString(), ea2, entry.getAddress());
            assertEquals(ea.toString(), ((vlan2 < 0) ? (short)0 : vlan2), entry.getVlan());
            assertEquals(ea.toString(), nc, entry.getNodeConnector());

            ips = entry.getInetAddresses();
            assertArrayEquals(ea.toString(), sender2, ips.iterator().next().getAddress());

            try {
                mgr.removeMacEntry(bpath, ea);
            } catch (VTNException e) {
                unexpected(e);
            }

            iphost++;
        }

        // test flush
        for (EthernetAddress ea: ethers) {
            byte[] bytes = ea.getValue();
            byte[] src = new byte[] {bytes[0], bytes[1], bytes[2], bytes[3], bytes[4], bytes[5]};
            byte[] dst = new byte[] {(byte)0xFF, (byte)0xFF, (byte)0xFF, (byte)0xFF, (byte)0xFF, (byte)0xFF};
            byte[] sender = new byte[] {(byte)192, (byte)168, (byte)0, (byte)iphost};
            byte[] target = new byte[] {(byte)192, (byte)168, (byte)0, (byte)250};

            PacketContext pctx = createARPPacketContext(src, dst, sender, target, (short)-1, connectors.get(0), ARP.REQUEST);

            tbl.add(pctx);
        }

        List<MacAddressEntry> list = null;
        try {
            list = mgr.getMacEntries(bpath);
        } catch (VTNException e) {
            unexpected(e);
        }
        assertNotNull(list);
        assertTrue(list.size() == ethers.size());

        Status st = mgr.flushMacEntries(bpath);
        assertEquals(StatusCode.SUCCESS, st.getCode());

        try {
            list = mgr.getMacEntries(bpath);
        } catch (VTNException e) {
            unexpected(e);
        }
        assertNotNull(list);
        assertTrue(list.size() == 0);

        try {
            list = mgr.getMacEntries(bpath2);
        } catch (VTNException e) {
            unexpected(e);
        }
        assertNotNull(list);
        assertTrue(list.size() == ethers.size());

        st = mgr.flushMacEntries(bpath2);
        assertEquals(StatusCode.SUCCESS, st.getCode());

        try {
            list = mgr.getMacEntries(bpath2);
        } catch (VTNException e) {
            unexpected(e);
        }
        assertNotNull(list);
        assertTrue(list.size() == 0);

        st = mgr.removeTenant(tpath);
        assertTrue(st.isSuccess());
    }

    /**
     * Test method for invalid cases of
     * {@link VTNManagerImpl#getMacEntries(VBridgePath)},
     * {@link VTNManagerImpl#getMacEntries(VBridgePath)},
     * {@link VTNManagerImpl#getMacEntry(VBridgePath, DataLinkAddress)},
     * {@link VTNManagerImpl#removeMacEntry(VBridgePath, DataLinkAddress)},
     * {@link VTNManagerImpl#flushMacEntries(VBridgePath)}
     */
    @Test
    public void testMacEntryInvalidCase() {
        VTNManagerImpl mgr = vtnMgr;
        short[] vlans = new short[] { 0, 10, 4095 };
        String tname = "vtn";
        VTenantPath tpath = new VTenantPath(tname);
        String bname = "vbridge";
        VBridgePath bpath = new VBridgePath(tname, bname);
        Status st = null;
        List<VBridgePath> bpathlist = new ArrayList<VBridgePath>();
        bpathlist.add(bpath);

        createTenantAndBridge(mgr, tpath, bpathlist);

        MacAddressTable tbl = mgr.getMacAddressTable(bpath);

        List<NodeConnector> connectors = createNodeConnectors(1, false);
        NodeConnector nc = connectors.get(0);
        short vlan = -1;

        byte[] src = new byte[] {(byte)0x00, (byte)0x00, (byte)0x00, (byte)0x00, (byte)0x00, (byte)0x01};
        byte[] dst = new byte[] {(byte)0xFF, (byte)0xFF, (byte)0xFF, (byte)0xFF, (byte)0xFF, (byte)0xFF};
        byte[] sender = new byte[] {(byte)192, (byte)168, (byte)0, (byte)1};
        byte[] target = new byte[] {(byte)192, (byte)168, (byte)0, (byte)250};

        PacketContext pctx = createARPPacketContext(src, dst, sender, target,
                                        (vlan > 0) ? vlan : -1, nc, ARP.REQUEST);
        EthernetAddress ea = null;
        try {
            ea = new EthernetAddress(src);
        } catch (ConstructionException e) {
            unexpected(e);
        }

        tbl.add(pctx);

        VBridgePath[] badplist = new VBridgePath[] {null,
                new VBridgePath(tname, null),
                new VBridgePath((String)null, bname)};
        EthernetAddress[] badelist = new EthernetAddress[] {null};

        // bad request
        for (VBridgePath path: badplist) {
            try {
                mgr.getMacEntry(path, ea);
                fail("Throwing exception was expected.");
            } catch (VTNException e) {
                assertEquals((path == null) ? "null" : path.toString(),
                        StatusCode.BADREQUEST, e.getStatus().getCode());
            }

            try {
                mgr.getMacEntries(path);
                fail("Throwing exception was expected.");
            } catch (VTNException e) {
                assertEquals((path == null) ? "null" : path.toString(),
                        StatusCode.BADREQUEST, e.getStatus().getCode());
            }

            try {
                mgr.removeMacEntry(path, ea);
                fail("Throwing exception was expected.");
            } catch (VTNException e) {
                assertEquals((path == null) ? "null" : path.toString(),
                        StatusCode.BADREQUEST, e.getStatus().getCode());
            }

            st = mgr.flushMacEntries(path);
            assertEquals((path == null) ? "null" : path.toString(),
                    StatusCode.BADREQUEST, st.getCode());
        }

        for (EthernetAddress eth: badelist) {
            try {
                mgr.getMacEntry(bpath, eth);
                fail("Throwing exception was expected.");
            } catch (VTNException e) {
                assertEquals(StatusCode.BADREQUEST, e.getStatus().getCode());
            }
        }

        // not found
        VBridgePath[] nfplist = new VBridgePath[] {
                new VBridgePath("vvvvvv", bname),
                new VBridgePath(tname, "bbbbbb")};
        EthernetAddress[] nfelist = new EthernetAddress[] {null};

        // bad request
        for (VBridgePath path: nfplist) {
            try {
                mgr.getMacEntry(path, ea);
                fail("Throwing exception was expected.");
            } catch (VTNException e) {
                assertEquals((path == null) ? "null" : path.toString(),
                        StatusCode.NOTFOUND, e.getStatus().getCode());
            }

            try {
                mgr.getMacEntries(path);
                fail("Throwing exception was expected.");
            } catch (VTNException e) {
                assertEquals((path == null) ? "null" : path.toString(),
                        StatusCode.NOTFOUND, e.getStatus().getCode());
            }

            try {
                mgr.removeMacEntry(path, ea);
                fail("Throwing exception was expected.");
            } catch (VTNException e) {
                assertEquals((path == null) ? "null" : path.toString(),
                        StatusCode.NOTFOUND, e.getStatus().getCode());
            }

            st = mgr.flushMacEntries(path);
            assertEquals((path == null) ? "null" : path.toString(),
                    StatusCode.NOTFOUND, st.getCode());
        }

        st = mgr.removeTenant(tpath);
        assertTrue(st.isSuccess());
    }

    /**
     * Test method for {@link VTNManagerImpl#readObject(ObjectInputStream))}
     */
    @Test
    public void testReadObject() {
        Object o = new VTenantPath("tenant");
        byte[] bytes = null;
        try {
            ByteArrayOutputStream bout = new ByteArrayOutputStream();
            ObjectOutputStream out = new ObjectOutputStream(bout);

            out.writeObject(o);
            out.close();
            bytes = bout.toByteArray();
        } catch (Exception e) {
            unexpected(e);
        }
        assertTrue(bytes.length != 0);

        // Deserialize the object.
        Object newobj = null;
        try {
            ByteArrayInputStream bin = new ByteArrayInputStream(bytes);
            ObjectInputStream in = new ObjectInputStream(bin);
            newobj = vtnMgr.readObject(in);
            in.close();
        } catch (Exception e) {
            unexpected(e);
        }

        assertNotSame(o, newobj);
        assertEquals(o, newobj);
    }

    /**
     * Test method for {@link VTNManagerImpl#entryUpdated}.
     */
    @Test
    public void testCacheEntryChange() {
        VTNManagerImpl mgr = vtnMgr;
        String root = GlobalConstants.STARTUPHOME.toString();
        String tenantListFileName = root + "vtn-default-tenant-names.conf";
        String configFileName100 =
            root + "vtn-" + "default" + "-" + "tenant100" + ".conf";
        String configFileName =
            root + "vtn-" + "default" + "-" + "tenant" + ".conf";

        // create tenant
        File tenantList = new File(tenantListFileName);
        tenantList.delete();

        File configFile = new File(configFileName);
        File configFile100 = new File(configFileName100);
        configFile.delete();
        configFile100.delete();

        InetAddress ipaddr = null;
        try {
            ipaddr = InetAddress.getByName("0.0.0.0");
        } catch (Exception e) {
            unexpected(e);
        }
        ClusterEventId evid = new ClusterEventId(ipaddr, 0);
        VTenantPath tpath = new VTenantPath("tenant");
        VTenantConfig tconf = new VTenantConfig(null);
        VTenant vtenant = new VTenant("tenant", tconf);
        mgr.addTenant(tpath, tconf);
        tenantList.delete();
        configFile.delete();
        configFile100.delete();

        VTenantEvent ev = new VTenantEvent(tpath, vtenant, UpdateType.ADDED);
        mgr.entryUpdated(evid, ev, VTNManagerImpl.CACHE_EVENT, true);
        flushTasks();
        assertFalse(tenantList.exists());
        assertFalse(configFile.exists());
        assertFalse(configFile100.exists());

        mgr.entryUpdated(evid, ev, VTNManagerImpl.CACHE_EVENT, false);
        flushTasks();
        assertTrue(tenantList.exists());
        assertTrue(configFile.exists());
        assertFalse(configFile100.exists());

        VTenantPath tpath100 = new VTenantPath("tenant100");
        VTenantConfig tconf100 = new VTenantConfig("tenant 100");
        VTenant vtenant100 = new VTenant("tenant100", tconf100);
        mgr.addTenant(tpath100, tconf100);
        flushTasks();
        tenantList.delete();
        configFile.delete();
        configFile100.delete();

        ev = new VTenantEvent(tpath100, vtenant100, UpdateType.ADDED);
        mgr.entryUpdated(evid, ev, VTNManagerImpl.CACHE_EVENT, true);
        flushTasks();
        assertFalse(tenantList.exists());
        assertFalse(configFile.exists());
        assertFalse(configFile100.exists());

        mgr.entryUpdated(evid, ev, VTNManagerImpl.CACHE_EVENT, false);
        flushTasks();
        assertTrue(tenantList.exists());
        assertFalse(configFile.exists());
        assertTrue(configFile100.exists());

        tenantList.delete();
        configFile.delete();
        configFile100.delete();

        // update
        ev = new VTenantEvent(tpath, vtenant, UpdateType.CHANGED);
        mgr.entryUpdated(evid, ev, VTNManagerImpl.CACHE_EVENT, true);
        flushTasks();
        checkFileExists(configFile, false, true);
        checkFileExists(configFile100, false, true);
        checkFileExists(tenantList, false, true);

        mgr.entryUpdated(evid, ev, VTNManagerImpl.CACHE_EVENT, false);
        flushTasks();
        checkFileExists(configFile, true, true);
        checkFileExists(configFile100, false, true);
        checkFileExists(tenantList, false, true);

        ev = new VTenantEvent(tpath100, vtenant100, UpdateType.CHANGED);
        mgr.entryUpdated(evid, ev, VTNManagerImpl.CACHE_EVENT, true);
        flushTasks();
        checkFileExists(configFile, false, true);
        checkFileExists(configFile100, false, true);
        checkFileExists(tenantList, false, true);

        mgr.entryUpdated(evid, ev, VTNManagerImpl.CACHE_EVENT, false);
        flushTasks();
        checkFileExists(configFile, false, true);
        checkFileExists(configFile100, true, true);
        checkFileExists(tenantList, false, true);

        mgr.removeTenant(tpath);
        mgr.removeTenant(tpath100);
        flushTasks();

        // delete
        tenantList.delete();
        try {
            configFile.createNewFile();
            configFile100.createNewFile();
        } catch (IOException e) {
            unexpected(e);
        }

        ev = new VTenantEvent(tpath, vtenant, UpdateType.REMOVED);
        mgr.entryUpdated(evid, ev, VTNManagerImpl.CACHE_EVENT, true);
        flushTasks();
        checkFileExists(configFile, true, false);
        checkFileExists(configFile100, true, false);
        checkFileExists(tenantList, false, true);

        mgr.entryUpdated(evid, ev, VTNManagerImpl.CACHE_EVENT, false);
        flushTasks();
        checkFileExists(configFile, false, false);
        checkFileExists(configFile100, true, false);
        checkFileExists(tenantList, true, true);

        ev = new VTenantEvent(tpath100, vtenant100, UpdateType.REMOVED);
        mgr.entryUpdated(evid, ev, VTNManagerImpl.CACHE_EVENT, true);
        flushTasks();
        checkFileExists(configFile, true, false);
        checkFileExists(configFile100, true, false);
        checkFileExists(tenantList, false, true);

        mgr.entryUpdated(evid, ev, VTNManagerImpl.CACHE_EVENT, false);
        flushTasks();
        checkFileExists(configFile, true, true);
        checkFileExists(configFile100, false, true);
        checkFileExists(tenantList, true, true);
    }

    private void checkFileExists(File file, boolean result, boolean remove) {
        boolean exists = file.exists();
        assertEquals(result, exists);
        if (remove) {
            if (exists) {
                file.delete();
            }
        } else {
            try {
                file.createNewFile();
            } catch (IOException e) {
               unexpected(e);
            }
        }
    };


    /**
     * Test method for
     * {@link VTNManagerImpl#saveConfiguration()}
     */
    @Test
    public void testSaveConfiguration() {
        VTNManagerImpl mgr = vtnMgr;
        String tname = "vtn";
        VTenantPath tpath = new VTenantPath(tname);
        Status st = mgr.addTenant(tpath, new VTenantConfig(null));
        assertTrue(st.isSuccess());
        assertTrue(mgr.isActive());

        st = mgr.saveConfiguration();
        assertTrue(st.isSuccess());

        mgr.removeTenant(tpath);
        assertTrue(st.isSuccess());

        st = mgr.saveConfiguration();
        assertTrue(st.isSuccess());
    }

    /**
     * Test method for
     * {@link VTNManagerImpl#receiveDataPacket(RawPacket)}
     */
    @Test
    public void testReceiveDataPacket() {
        VTNManagerImpl mgr = vtnMgr;
        short[] vlans = new short[] { 0, 10, 4095 };
        ISwitchManager swmgr = mgr.getSwitchManager();
        byte[] cntMac = swmgr.getControllerMAC();

        String tname = "vtn";
        VTenantPath tpath = new VTenantPath(tname);
        Status st = mgr.addTenant(tpath, new VTenantConfig(null));
        assertTrue(st.isSuccess());
        assertTrue(mgr.isActive());

        PacketResult result = mgr.receiveDataPacket(null);
        assertEquals(PacketResult.IGNORED, result);

        byte [] srcCnt = new byte[] {cntMac[0], cntMac[1], cntMac[2],
                                     cntMac[3], cntMac[4], cntMac[5]};
        byte [] dst = new byte[] {(byte)0xff, (byte)0xff, (byte)0xff,
                                  (byte)0xff, (byte)0xff, (byte)0xff};
        byte [] target = new byte[] {(byte)192, (byte)168, (byte)0, (byte)250};
        byte [] sender = new byte[] {(byte)192, (byte)168, (byte)0, (byte)1};

        List<NodeConnector> connectors = createNodeConnectors(4);
        for (NodeConnector nc: connectors) {
            byte iphost = 1;
            for (EthernetAddress ea: createEthernetAddresses(false)) {
                byte [] bytes = ea.getValue();
                byte [] src = new byte[] {bytes[0], bytes[1], bytes[2],
                                        bytes[3], bytes[4], bytes[5]};
                sender[3] = (byte)iphost;
                RawPacket inPkt = createARPRawPacket(src, dst, sender, target, (short)-1, nc, ARP.REQUEST);
                result = mgr.receiveDataPacket(inPkt);

                // because there are no topology, in this case always ignored.
                assertEquals(PacketResult.IGNORED, result);

                iphost++;
            }

            // packet from controller.
            RawPacket inPkt = createARPRawPacket(srcCnt, dst, sender, target, (short)-1, nc, ARP.REQUEST);
            result = mgr.receiveDataPacket(inPkt);

            assertEquals(PacketResult.IGNORED, result);
        }

        st = mgr.removeTenant(tpath);
        assertTrue(st.isSuccess());
    }
}
