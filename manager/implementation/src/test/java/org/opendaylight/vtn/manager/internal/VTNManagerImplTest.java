/*
 * Copyright (c) 2013-2014 NEC Corporation
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
import java.util.HashSet;
import java.util.Hashtable;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.Timer;
import java.util.TimerTask;
import java.util.concurrent.ConcurrentMap;

import org.apache.felix.dm.Component;
import org.apache.felix.dm.impl.ComponentImpl;

import org.junit.Test;
import org.junit.experimental.categories.Category;

import org.opendaylight.vtn.manager.DataLinkHost;
import org.opendaylight.vtn.manager.EthernetHost;
import org.opendaylight.vtn.manager.IVTNManagerAware;
import org.opendaylight.vtn.manager.IVTNModeListener;
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
import org.opendaylight.vtn.manager.VTenantConfig;
import org.opendaylight.vtn.manager.VTenantPath;
import org.opendaylight.vtn.manager.VlanMap;
import org.opendaylight.vtn.manager.VlanMapConfig;
import org.opendaylight.vtn.manager.internal.cluster.ClusterEvent;
import org.opendaylight.vtn.manager.internal.cluster.ClusterEventId;
import org.opendaylight.vtn.manager.internal.cluster.FlowAddEvent;
import org.opendaylight.vtn.manager.internal.cluster.FlowGroupId;
import org.opendaylight.vtn.manager.internal.cluster.FlowModResult;
import org.opendaylight.vtn.manager.internal.cluster.FlowModResultEvent;
import org.opendaylight.vtn.manager.internal.cluster.FlowRemoveEvent;
import org.opendaylight.vtn.manager.internal.cluster.MacMapEvent;
import org.opendaylight.vtn.manager.internal.cluster.MacMapPath;
import org.opendaylight.vtn.manager.internal.cluster.MacTableEntry;
import org.opendaylight.vtn.manager.internal.cluster.MacTableEntryId;
import org.opendaylight.vtn.manager.internal.cluster.MacVlan;
import org.opendaylight.vtn.manager.internal.cluster.MapReference;
import org.opendaylight.vtn.manager.internal.cluster.MapType;
import org.opendaylight.vtn.manager.internal.cluster.PortVlan;
import org.opendaylight.vtn.manager.internal.cluster.RawPacketEvent;
import org.opendaylight.vtn.manager.internal.cluster.VBridgeIfImpl;
import org.opendaylight.vtn.manager.internal.cluster.VBridgeImpl;
import org.opendaylight.vtn.manager.internal.cluster.VTNFlow;
import org.opendaylight.vtn.manager.internal.cluster.VTenantEvent;
import org.opendaylight.vtn.manager.internal.cluster.VTenantImpl;
import org.opendaylight.vtn.manager.internal.cluster.VlanMapImpl;
import org.opendaylight.vtn.manager.internal.cluster.VlanMapPath;

import org.opendaylight.controller.clustering.services.IClusterContainerServices;
import org.opendaylight.controller.connectionmanager.IConnectionManager;
import org.opendaylight.controller.forwardingrulesmanager.FlowEntry;
import org.opendaylight.controller.forwardingrulesmanager.IForwardingRulesManager;
import org.opendaylight.controller.hosttracker.IfHostListener;
import org.opendaylight.controller.hosttracker.IfIptoHost;
import org.opendaylight.controller.hosttracker.hostAware.HostNodeConnector;
import org.opendaylight.controller.sal.core.ConstructionException;
import org.opendaylight.controller.sal.core.Node;
import org.opendaylight.controller.sal.core.NodeConnector;
import org.opendaylight.controller.sal.core.UpdateType;
import org.opendaylight.controller.sal.flowprogrammer.Flow;
import org.opendaylight.controller.sal.match.Match;
import org.opendaylight.controller.sal.match.MatchType;
import org.opendaylight.controller.sal.packet.ARP;
import org.opendaylight.controller.sal.packet.IDataPacketService;
import org.opendaylight.controller.sal.packet.PacketResult;
import org.opendaylight.controller.sal.packet.RawPacket;
import org.opendaylight.controller.sal.packet.address.DataLinkAddress;
import org.opendaylight.controller.sal.packet.address.EthernetAddress;
import org.opendaylight.controller.sal.routing.IRouting;
import org.opendaylight.controller.sal.utils.GlobalConstants;
import org.opendaylight.controller.sal.utils.NetUtils;
import org.opendaylight.controller.sal.utils.NodeConnectorCreator;
import org.opendaylight.controller.sal.utils.NodeCreator;
import org.opendaylight.controller.sal.utils.Status;
import org.opendaylight.controller.sal.utils.StatusCode;
import org.opendaylight.controller.switchmanager.ISwitchManager;
import org.opendaylight.controller.topologymanager.ITopologyManager;

/**
 * JUnit test for {@link VTNManagerImpl}.
 *
 * <p>
 * Tests of {@link VTenantImpl}, {@link VBridgeImpl}, {@link VBridgeIfImpl},
 * {@link VlanMapImpl} are also implemented in this class.
 * </p>
 */
@Category(SlowTest.class)
public class VTNManagerImplTest extends VTNManagerImplTestCommon {
    /**
     * Construct a new instance.
     */
    public VTNManagerImplTest() {
        super(0);
    }

    /**
     * Test method for
     * {@link VTNManagerImpl#init(Component)},
     * {@link VTNManagerImpl#destroy()}.
     */
    @Test
    public void testInitDestory() {
        ComponentImpl c = new ComponentImpl(null, null, null);
        Hashtable<String, String> properties = new Hashtable<String, String>();
        String containerName = "default";
        properties.put("containerName", containerName);
        c.setServiceProperties(properties);
        Map<VBridgeIfPath, PortMapConfig> pmaps
                = new HashMap<VBridgeIfPath, PortMapConfig>();
        Map<VlanMap, VlanMapConfig> vmaps = new HashMap<VlanMap, VlanMapConfig>();

        // create dummy file in startup directory.
        String dir = GlobalConstants.STARTUPHOME.toString();
        String prefix = VTenantImpl.CONFIG_FILE_PREFIX;
        String suffix = VTenantImpl.CONFIG_FILE_SUFFIX;
        String[] dummyFileNames = new String[] {
                prefix + containerName + "-" + suffix,
                "vtn" + suffix,
                prefix + containerName + "-" + "config",
                "config",
                prefix + containerName + "-" + "notexist" + suffix
        };
        Set<File> fileSet = new HashSet<File>();
        for (String fileName : dummyFileNames) {
            File file = new File(dir, fileName);
            try {
                assertTrue(file.createNewFile());
                fileSet.add(file);
            } catch (IOException e) {
                unexpected(e);
            }
        }

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

        Status st = vtnMgr.addBridgeInterface(ifpath2,
                new VInterfaceConfig(null, Boolean.FALSE));
        ifpathlist.add(ifpath2);
        assertEquals(StatusCode.SUCCESS, st.getCode());

        restartVTNManager(c);

        pmaps.put(ifpath1, null);
        pmaps.put(ifpath2, null);
        checkVTNconfig(vtnMgr, tpath, bpathlist, pmaps, null, null);

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

        // start with having a configuration.
        // this case, startup with cache.
        restartVTNManager(c);
        checkVTNconfig(vtnMgr, tpath, bpathlist, pmaps, vmaps, null);

        // Configure MAC mapping.
        Set<DataLinkHost> allow = new HashSet<DataLinkHost>();
        for (int i = 0; i < 10; i++) {
            long mac = 0x123456789aL + i;
            DataLinkHost dlh = createEthernetHost(mac, (short)i);
            assertTrue(allow.add(dlh));
        }

        Set<DataLinkHost> deny = new HashSet<DataLinkHost>();
        for (int i = 0; i < 5; i++) {
            long mac = 0xaabbccddeeL + i;
            DataLinkHost dlh = createEthernetHost(mac, (short)(i >> 1));
            assertTrue(deny.add(dlh));
        }

        MacMapConfig mcconf = new MacMapConfig(allow, deny);
        try {
            assertEquals(UpdateType.ADDED,
                         vtnMgr.setMacMap(bpath, UpdateOperation.SET, mcconf));
            assertNull(vtnMgr.setMacMap(bpath, UpdateOperation.SET, mcconf));
        } catch (Exception e) {
            unexpected(e);
        }

        checkVTNconfig(vtnMgr, tpath, bpathlist, pmaps, vmaps, mcconf);
        restartVTNManager(c);
        checkVTNconfig(vtnMgr, tpath, bpathlist, pmaps, vmaps, mcconf);

        // start after configuration files is removed.
        // because caches remain, setting is taken over.
        stopVTNManager(false);
        VTNManagerImpl.cleanUpConfigFile(containerName);
        startVTNManager(c);
        checkVTNconfig(vtnMgr, tpath, bpathlist, pmaps, vmaps, mcconf);

        // start after cache is cleared
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
        VTNManagerImpl.cleanUpConfigFile(containerName);
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

        // check dummy files exist.
        for (File file : fileSet) {
            if (file.getName()
                    .equals(prefix + containerName + "-" + "notexist" + suffix)) {
                assertFalse(file.toString(), file.exists());
            } else {
                assertTrue(file.toString(), file.exists());
                file.delete();
            }
        }
    }

    /**
     * Test method for
     * {@link VTNManagerImpl#init(Component)},
     * {@link VTNManagerImpl#destroy()}.
     *
     * <p>
     * In case that a cache is remained when cache is created.
     * </p>
     */
    @Test
    public void testInitEventAndFlowAndMacCacheRemainedCase() {
        ComponentImpl c = new ComponentImpl(null, null, null);
        Hashtable<String, String> properties = new Hashtable<String, String>();
        String containerName = "default";
        properties.put("containerName", containerName);
        c.setServiceProperties(properties);

        String tname = "vtn";
        VTenantPath tpath = new VTenantPath(tname);
        String bname = "vbridge";
        VBridgePath bpath = new VBridgePath(tname, bname);
        VBridgeIfPath ifpath1 = new VBridgeIfPath(tname, bname, "vif1");

        List<VBridgePath> bpathlist = new ArrayList<VBridgePath>();
        List<VBridgeIfPath> ifpathlist = new ArrayList<VBridgeIfPath>();
        bpathlist.add(bpath);
        ifpathlist.add(ifpath1);
        createTenantAndBridgeAndInterface(vtnMgr, tpath, bpathlist, ifpathlist);

        // in case event cache remain.
        stopVTNManager(false);
        VTNManagerImpl.cleanUpConfigFile(containerName);

        ConcurrentMap<ClusterEventId, ClusterEvent> clsmap
            = (ConcurrentMap<ClusterEventId, ClusterEvent>) stubObj
                .getCache(VTNManagerImpl.CACHE_EVENT);

        FlowGroupId gid = new FlowGroupId("test_tenant");
        InetAddress ipaddr = getInetAddressFromAddress(new byte[] {0, 0, 0, 0});
        FlowGroupId gidRemote = new FlowGroupId(ipaddr, 0L, "test_tenant2");

        VTNFlow flow = new VTNFlow(gid);
        FlowModResultEvent ev = new FlowModResultEvent("test",
                                                       FlowModResult.SUCCEEDED);
        clsmap.put(gid, ev);

        VTNFlow flowRemote = new VTNFlow(gid);
        FlowModResultEvent evRemote = new FlowModResultEvent("remote",
                                                       FlowModResult.SUCCEEDED);
        clsmap.put(gidRemote, evRemote);

        startVTNManager(c);
        assertNull(clsmap.get(gid));
        assertEquals(evRemote, clsmap.get(gidRemote));

        // in case flow cache remain.
        VTNFlowDatabase fdb = vtnMgr.getTenantFlowDB(tpath.getTenantName());
        flow = fdb.create(vtnMgr);
        Node node = NodeCreator.createOFNode(Long.valueOf(0L));
        NodeConnector innc = NodeConnectorCreator.createOFNodeConnector(Short.valueOf("1"), node);
        NodeConnector outnc = NodeConnectorCreator.createOFNodeConnector(Short.valueOf("2"), node);
        byte[] src = {
            (byte)0x00, (byte)0x11, (byte)0x22,
            (byte)0x33, (byte)0x44, (byte)0x55,
        };
        byte[] dst = {
            (byte)0xf0, (byte)0xfa, (byte)0xfb,
            (byte)0xfc, (byte)0xfd, (byte)0xfe,
        };
        Match match = new Match();
        match.setField(MatchType.IN_PORT, innc);
        match.setField(MatchType.DL_VLAN, (short)1);
        match.setField(MatchType.DL_SRC, src);
        match.setField(MatchType.DL_DST, dst);
        ActionList actions = new ActionList(node);
        actions.addOutput(outnc);
        flow.addFlow(vtnMgr, match, actions, 1);

        fdb.install(vtnMgr, flow);
        flushFlowTasks();
        assertEquals(1, vtnMgr.getFlowDB().size());

        stopVTNManager(false);

        ConcurrentMap<FlowGroupId, VTNFlow> flowMap
            = (ConcurrentMap<FlowGroupId, VTNFlow>) stubObj
                .getCache(VTNManagerImpl.CACHE_FLOWS);
        gid = new FlowGroupId("test_tenant");
        flow = new VTNFlow(gid);
        flowMap.put(gid, flow);

        // A entry not associated with existing tenant correctly is
        // removed in initialization process.
        startVTNManager(c);
        assertEquals(1, vtnMgr.getFlowDB().size());

        // in case MAC Address Table cache remain.
        ipaddr = getInetAddressFromAddress(new byte[] {10, 0, 0, 1});
        MacTableEntry tent = new MacTableEntry(bpath, 1L, innc,
                (short) 0, ipaddr);
        vtnMgr.putMacTableEntry(tent);
        flushTasks();
        List<MacAddressEntry> entries = getMacAddressEntries(tent.getEntryId());
        assertEquals(0, entries.size());

        stopVTNManager(false);
        startVTNManager(c);

        // when entry which is local entry is removed.
        entries = getMacAddressEntries(tent.getEntryId());
        assertEquals(0, entries.size());
    }

    /**
     * Get MAC address entries.
     * @param id    A {@link MacTableEntryId}.
     * @return  A List of {@link MacAddressEntry}.
     */
    private List<MacAddressEntry> getMacAddressEntries(MacTableEntryId id) {
        MacAddressTable table = vtnMgr.getMacAddressTable(id);
        List<MacAddressEntry> entries = null;
        try {
            entries = table.getEntries();
        } catch (VTNException e) {
            unexpected(e);
        }
        return entries;
    }

    /**
     * Test method for
     * {@link VTNManagerImpl#getContainerName()}
     */
    @Test
    public void testGetContainerName() {
        // "default" is set as a container name in before class.
        assertEquals("default", vtnMgr.getContainerName());

        // in case not "default" container
        ComponentImpl c = new ComponentImpl(null, null, null);
        Hashtable<String, String> properties = new Hashtable<String, String>();
        String containerNameTest = "test";
        properties.put("containerName", containerNameTest);
        c.setServiceProperties(properties);
        startVTNManager(c);
        assertEquals(containerNameTest, vtnMgr.getContainerName());
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

       // in case IHostListener specified with argument of removeHostListener()
       // don't match already registered
       HostListener hlnew1 = new HostListener();
       HostListener hlnew2 = new HostListener();
       mgr.removeHostListener(hlnew2);
       mgr.removeHostListener(hlnew1);

       // add entry to MacAddressTable to call HostListener
       byte [] src = new byte[] {(byte)0, (byte)0, (byte)0,
                                 (byte)0, (byte)0, (byte)0x01};
       byte [] dst = new byte[] {(byte)0xff, (byte)0xff, (byte)0xff,
                                 (byte)0xff, (byte)0xff, (byte)0xff};
       byte [] sender = new byte[] {(byte)192, (byte)168, (byte)0, (byte)1};
       byte [] target = new byte[] {(byte)192, (byte)168, (byte)0, (byte)250};
       List<NodeConnector> connectors = createNodeConnectors(1, false);
       PacketContext pctx = createARPPacketContext(src, dst, sender, target,
               (short)-1, connectors.get(0), ARP.REQUEST);

       VBridgePath path = new VBridgePath("tenant1", "bridge1");
       VBridgeIfPath ipath = new VBridgeIfPath(path, "if1");
       TestBridgeNode bnode = new TestBridgeNode(ipath);
       MacAddressTable table = new MacAddressTable(mgr, path, 600);
       table.add(pctx, bnode);

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
        assertEquals(StatusCode.SUCCESS, st.getCode());
        flushTasks();
        stub1.checkAllNull();
        stub2.checkAllNull();

        mgr.addVTNManagerAware(stub1);
        mgr.addVTNManagerAware(stub2);
        flushTasks();
        stub1.checkVtnInfo(1, tpath, UpdateType.ADDED);
        stub2.checkVtnInfo(1, tpath, UpdateType.ADDED);

        mgr.removeVTNManagerAware(stub2);
        mgr.addVTNManagerAware(stub2);
        flushTasks();
        stub2.checkVtnInfo(1, tpath, UpdateType.ADDED);
        stub1.checkAllNull();
        stub2.checkAllNull();

        // add a vbridge
        String bname = "bridge";
        VBridgePath bpath = new VBridgePath(tname, bname);
        st = mgr.addBridge(bpath, new VBridgeConfig(null));
        assertEquals(StatusCode.SUCCESS, st.getCode());
        stub1.checkVbrInfo(1, bpath, UpdateType.ADDED);
        stub2.checkVbrInfo(1, bpath, UpdateType.ADDED);

        mgr.removeVTNManagerAware(stub2);
        mgr.addVTNManagerAware(stub2);
        flushTasks();
        stub2.checkVtnInfo(1, tpath, UpdateType.ADDED);
        stub2.checkVbrInfo(1, bpath, UpdateType.ADDED);
        stub1.checkAllNull();
        stub2.checkAllNull();

        // add a vInterface
        String ifname = "vif";
        VBridgeIfPath ifpath = new VBridgeIfPath(tname, bname, ifname);
        VInterfaceConfig ifconf = new VInterfaceConfig(null, null);
        st = mgr.addBridgeInterface(ifpath, ifconf);
        assertEquals(StatusCode.SUCCESS, st.getCode());
        stub1.checkVIfInfo(1, ifpath, UpdateType.ADDED);
        stub2.checkVIfInfo(1, ifpath, UpdateType.ADDED);

        mgr.removeVTNManagerAware(stub2);
        mgr.addVTNManagerAware(stub2);
        flushTasks();
        stub2.checkVtnInfo(1, tpath, UpdateType.ADDED);
        stub2.checkVbrInfo(1, bpath, UpdateType.ADDED);
        stub2.checkVIfInfo(1, ifpath, UpdateType.ADDED);
        stub1.checkAllNull();
        stub2.checkAllNull();

        // set a PortMap
        Node node = NodeCreator.createOFNode(0L);
        SwitchPort port = new SwitchPort(NodeConnector.NodeConnectorIDType.OPENFLOW,
        String.valueOf(10));
        PortMapConfig pmconf = new PortMapConfig(node, port, (short)0);
        st = mgr.setPortMap(ifpath, pmconf);

        // after set this mapping
        // vbridge status and vInterface status are UNKNOWN -> DOWN.
        stub1.checkVbrInfo(1, bpath, UpdateType.CHANGED);
        stub1.checkVIfInfo(1, ifpath, UpdateType.CHANGED);
        stub1.checkPmapInfo(1, ifpath, pmconf, UpdateType.ADDED);
        stub2.checkVbrInfo(1, bpath, UpdateType.CHANGED);
        stub2.checkVIfInfo(1, ifpath, UpdateType.CHANGED);
        stub2.checkPmapInfo(1, ifpath, pmconf, UpdateType.ADDED);

        mgr.removeVTNManagerAware(stub2);
        mgr.addVTNManagerAware(stub2);
        flushTasks();
        stub2.checkVtnInfo(1, tpath, UpdateType.ADDED);
        stub2.checkVbrInfo(1, bpath, UpdateType.ADDED);
        stub2.checkVIfInfo(1, ifpath, UpdateType.ADDED);
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
        stub2.checkVtnInfo(1, tpath, UpdateType.ADDED);
        stub2.checkVbrInfo(1, bpath, UpdateType.ADDED);
        stub2.checkVIfInfo(1, ifpath, UpdateType.ADDED);
        stub2.checkPmapInfo(1, ifpath, pmconf, UpdateType.ADDED);
        stub2.checkVlmapInfo(1, bpath, map.getId(), UpdateType.ADDED);
        stub1.checkAllNull();
        stub2.checkAllNull();

        mgr.addVTNManagerAware(stub2);
        flushTasks();

        // modify a tenant setting
        st = mgr.modifyTenant(tpath, new VTenantConfig("desc"), false);
        assertEquals(StatusCode.SUCCESS, st.getCode());
        stub1.checkVtnInfo(1, tpath, UpdateType.CHANGED);
        stub2.checkVtnInfo(1, tpath, UpdateType.CHANGED);

        flushTasks();
        stub1.checkAllNull();
        stub2.checkAllNull();

        st = mgr.modifyBridge(bpath, new VBridgeConfig("desc"), false);
        assertEquals(StatusCode.SUCCESS, st.getCode());
        stub1.checkVbrInfo(1, bpath, UpdateType.CHANGED);
        stub2.checkVbrInfo(1, bpath, UpdateType.CHANGED);

        flushTasks();
        stub1.checkAllNull();
        stub2.checkAllNull();

        // modify a vbridge setting
        st = mgr.modifyBridgeInterface(ifpath,
                new VInterfaceConfig("interface", Boolean.TRUE), false);
        assertEquals(StatusCode.SUCCESS, st.getCode());
        stub1.checkVIfInfo(1, ifpath, UpdateType.CHANGED);
        stub2.checkVIfInfo(1, ifpath, UpdateType.CHANGED);

        flushTasks();
        stub1.checkAllNull();
        stub2.checkAllNull();

        // change a PortMap setting
        port = new SwitchPort(NodeConnector.NodeConnectorIDType.OPENFLOW, String.valueOf(11));
        pmconf = new PortMapConfig(node, port, (short)0);
        st = mgr.setPortMap(ifpath, pmconf);
        assertEquals(StatusCode.SUCCESS, st.getCode());
        stub1.checkPmapInfo(1, ifpath, pmconf, UpdateType.CHANGED);
        stub2.checkPmapInfo(1, ifpath, pmconf, UpdateType.CHANGED);

        // remove a VLANMap setting
        st = mgr.removeVlanMap(bpath, map.getId());
        assertEquals(StatusCode.SUCCESS, st.getCode());
        stub1.checkVlmapInfo(1, bpath, map.getId(), UpdateType.REMOVED);
        stub2.checkVlmapInfo(1, bpath, map.getId(), UpdateType.REMOVED);

        flushTasks();
        stub1.checkAllNull();
        stub2.checkAllNull();

        // remove a portmap
        st = mgr.setPortMap(ifpath, null);
        assertEquals(StatusCode.SUCCESS, st.getCode());

        // vBridge status and vInterface status is changed (DOWN->UNKNOWN)
        stub1.checkVbrInfo(1, bpath, UpdateType.CHANGED);
        stub1.checkVIfInfo(1, ifpath, UpdateType.CHANGED);
        stub1.checkPmapInfo(1, ifpath, pmconf, UpdateType.REMOVED);
        stub2.checkVbrInfo(1, bpath, UpdateType.CHANGED);
        stub2.checkVIfInfo(1, ifpath, UpdateType.CHANGED);
        stub2.checkPmapInfo(1, ifpath, pmconf, UpdateType.REMOVED);

        flushTasks();
        stub1.checkAllNull();
        stub2.checkAllNull();

        // remove a vbridge interface
        st = mgr.removeBridgeInterface(ifpath);
        assertEquals(StatusCode.SUCCESS, st.getCode());
        stub1.checkVIfInfo(1, ifpath, UpdateType.REMOVED);
        stub2.checkVIfInfo(1, ifpath, UpdateType.REMOVED);

        flushTasks();
        stub1.checkAllNull();
        stub2.checkAllNull();

        // remove a vbridge
        st = mgr.removeBridge(bpath);
        assertEquals(StatusCode.SUCCESS, st.getCode());
        stub1.checkVbrInfo(1, bpath, UpdateType.REMOVED);
        stub2.checkVbrInfo(1, bpath, UpdateType.REMOVED);

        flushTasks();
        stub1.checkAllNull();
        stub2.checkAllNull();

        VTNManagerAwareStub stubnew1 = new VTNManagerAwareStub();
        mgr.removeVTNManagerAware(stubnew1);

        // remove a tenant
        st = mgr.removeTenant(tpath);
        assertEquals(StatusCode.SUCCESS, st.getCode());
        stub1.checkVtnInfo(1, tpath, UpdateType.REMOVED);
        stub2.checkVtnInfo(1, tpath, UpdateType.REMOVED);

        flushTasks();
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
        assertEquals(StatusCode.SUCCESS, st.getCode());
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
        for (Boolean bool : createBooleans(false)) {
            boolean curr = bool.booleanValue();
            mgr.notifyChange(curr);
            flushTasks();

            stub1.checkCalledInfo(1, bool);
            stub2.checkCalledInfo(1, bool);
        }

        // notifyChange(IVTNModeListener, boolean)
        for (Boolean bool : createBooleans(false)) {
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
        assertEquals(StatusCode.SUCCESS, st.getCode());
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

        // in case received event from other node.
        InetAddress ipaddr = null;
        try {
            ipaddr = InetAddress.getByName("0.0.0.0");
        } catch (Exception e) {
            unexpected(e);
        }
        ClusterEventId evid = new ClusterEventId(ipaddr, 0);
        VTenantConfig tconf = new VTenantConfig(null);
        VTenant vtenant = new VTenant("tenant", tconf);
        IClusterContainerServices cs = mgr.getClusterContainerService();
        ConcurrentMap<String, VTenantImpl> tenantDB
            = (ConcurrentMap<String, VTenantImpl>)cs.getCache("vtn.tenant");
        VTenantImpl imp = null;
        try {
            imp = new VTenantImpl("default", tpath.getTenantName(), tconf);
        } catch (VTNException e) {
            unexpected(e);
        }
        tenantDB.put(tpath.getTenantName(), imp);

        VTenantEvent ev = new VTenantEvent(tpath, vtenant, UpdateType.ADDED);
        mgr.entryUpdated(evid, ev, VTNManagerImpl.CACHE_EVENT, false);
        flushTasks();
        stub1.checkCalledInfo(1, Boolean.TRUE);

        mgr.removeVTNModeListener(stub1);
    }

    /**
     * Test method for
     * {@link VTNManagerImpl#addMacAddressTable(VBridgePath, int)},
     * {@link VTNManagerImpl#removeMacAddressTable(VBridgePath, boolean)},
     * {@link VTNManagerImpl#getMacAddressTable(VBridgePath)}.
     */
    @Test
    public void testMacAddressTable() {
        VTNManagerImpl mgr = vtnMgr;
        MacAddressTable table = null;
        VBridgePath bpath = new VBridgePath("teannt", "bridge");
        VBridgeIfPath ipath = new VBridgeIfPath(bpath, "if");
        VlanMapPath vpath = new VlanMapPath(bpath, "ANY.0");
        int[] ages = new int[] {10, 600, 1000000};

        for (int age : ages) {
            mgr.addMacAddressTable(bpath, age);

            table = mgr.getMacAddressTable(bpath);
            assertNotNull(table);

            assertEquals(table, mgr.getMacAddressTable(ipath));
            assertEquals(table, mgr.getMacAddressTable(vpath));

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
                        String emsg = "(name)" + tname + "(desc)"+ desc
                                + ",(iv)" + ((iv == null) ? "null" : iv.intValue())
                                + ",(hv)" + ((hv == null) ? "null" : hv.intValue());

                        // test addTenant()
                        Status st = mgr.addTenant(tpath, tconf);
                        if (iv != null && hv != null
                                && iv.intValue() > 0 && hv.intValue() > 0
                                && iv.intValue() >= hv.intValue()) {
                            assertEquals(emsg, StatusCode.BADREQUEST, st.getCode());
                            continue;
                        } else if ((iv == null || iv.intValue() < 0) && hv != null &&
                                hv.intValue() > 0 && 300 >= hv.intValue()) {
                            assertEquals(emsg, StatusCode.BADREQUEST, st.getCode());
                            continue;
                        } else {
                            assertEquals("(VTenantConfig)" + tconf.toString(),
                                    StatusCode.SUCCESS, st.getCode());
                            assertTrue(emsg, mgr.isActive());
                        }

                        // test getTenant() and test getter methods
                        VTenant tenant = null;
                        try {
                            tenant = mgr.getTenant(tpath);
                        } catch (Exception e) {
                            unexpected(e);
                        }
                        assertEquals(tname, tenant.getName());
                        assertEquals(tconf.getDescription(), tenant.getDescription());
                        if (iv == null || iv.intValue() < 0) {
                            assertEquals(emsg, 300, tenant.getIdleTimeout());
                        } else {
                            assertEquals(emsg, iv.intValue(), tenant.getIdleTimeout());
                        }
                        if (hv == null || hv.intValue() < 0) {
                            assertEquals(emsg, 0, tenant.getHardTimeout());
                        } else {
                            assertEquals(emsg, hv.intValue(), tenant.getHardTimeout());
                        }

                        // test removeTenant()
                        mgr.removeTenant(tpath);
                        assertEquals(emsg, StatusCode.SUCCESS, st.getCode());
                    }
                }
            }
        }

        try {
            List<VTenant> list = mgr.getTenants();
            assertEquals(0, list.size());
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
                        // reject cases which have invalid combination of parameters.
                        // origxx is used for initial settings before modify,
                        // then need to reject invalid parameter.
                        if (orgiv != null && orghv != null
                                && orgiv.intValue() > 0 && orghv.intValue() > 0
                                && orgiv.intValue() >= orghv.intValue()) {
                            continue;
                        } else if ((orgiv == null || orgiv.intValue() < 0)
                                && orghv != null && orghv.intValue() > 0
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
                                        String emsg = "(name)" + tname + "(ndesc)"+ ndesc
                                                + ",(iv)" + ((iv == null) ? "null" : iv.intValue())
                                                + ",(hv)" + ((hv == null) ? "null" : hv.intValue());

                                        tconf = createVTenantConfig(ndesc, iv, hv);
                                        st = mgr.modifyTenant(tpath, tconf, true);

                                        if (iv != null && hv != null &&
                                            iv.intValue() > 0 && hv.intValue() > 0 &&
                                            iv.intValue() >= hv.intValue()) {
                                            assertEquals(emsg,
                                                    StatusCode.BADREQUEST, st.getCode());
                                        } else if ((iv == null || iv < 0) &&
                                                hv != null && hv > 0 && 300 >= hv) {
                                            assertEquals(emsg,
                                                    StatusCode.BADREQUEST, st.getCode());
                                        } else {
                                            assertEquals(emsg,
                                                    StatusCode.SUCCESS, st.getCode());
                                        }

                                        try {
                                            tenant = mgr.getTenant(tpath);
                                        } catch (Exception e) {
                                            unexpected(e);
                                        }

                                        if (st.isSuccess()) {
                                            assertEquals(emsg,
                                                    tname, tenant.getName());
                                            assertEquals(emsg,
                                                    ndesc, tenant.getDescription());
                                            if (iv == null || iv.intValue() < 0) {
                                                assertEquals(emsg,
                                                        300, tenant.getIdleTimeout());
                                            } else {
                                                assertEquals(emsg,
                                                        iv.intValue(),
                                                        tenant.getIdleTimeout());
                                            }
                                            if (hv == null || hv.intValue() < 0) {
                                                assertEquals(tconf.toString(),
                                                        0, tenant.getHardTimeout());
                                            } else {
                                                assertEquals(tconf.toString(),
                                                        hv.intValue(),
                                                        tenant.getHardTimeout());
                                            }
                                        }
                                    }
                                }

                                // set initial parameter before test.
                                VTenantConfig tconfOrg = createVTenantConfig(desc,
                                        orgiv, orghv);
                                st = mgr.modifyTenant(tpath, tconfOrg, true);

                                olddesc = (desc == null) ? null : new String(desc);
                                oldiv = (orgiv == null || orgiv.intValue() < 0) ?
                                            new Integer(300) : orgiv;
                                oldhv = (orghv == null || orghv.intValue() < 0) ?
                                            new Integer(0) : orghv;

                                // in case all == false
                                tconf = createVTenantConfig(desc, iv, hv);
                                st = mgr.modifyTenant(tpath, tconf, false);
                                String emsg = "(VTenangConfig(orig))"  + tconfOrg.toString()
                                        + "(name)" + tname + "(ndesc)"+ desc
                                        + ",(iv)" + ((iv == null) ? "null" : iv.intValue())
                                        + ",(hv)" + ((hv == null) ? "null" : hv.intValue());

                                if ((iv == null || iv.intValue() < 0)
                                        && (hv == null || hv.intValue() < 0)) {
                                    // both are not set
                                    // not changed.
                                } else if (iv == null || iv.intValue() < 0) {
                                    // idle_timeout is not set
                                    if (hv.intValue() > 0 && (oldiv.intValue() != 0
                                            && oldiv.intValue() >= hv.intValue())) {
                                        assertEquals(emsg,
                                                StatusCode.BADREQUEST, st.getCode());
                                    } else {
                                        assertEquals(emsg,
                                                StatusCode.SUCCESS, st.getCode());
                                    }
                                } else if (hv == null || hv.intValue() < 0) {
                                    // hard_timeout is not set
                                    if (iv > 0 && oldhv > 0) {
                                        if (iv >= oldhv) {
                                            assertEquals(emsg,
                                                    StatusCode.BADREQUEST, st.getCode());
                                        } else {
                                            assertEquals(emsg,
                                                    StatusCode.SUCCESS, st.getCode());
                                        }
                                    } else {
                                        assertEquals(emsg,
                                                StatusCode.SUCCESS, st.getCode());
                                    }
                                } else {
                                    // both are set
                                    if (iv.intValue() > 0 && hv.intValue() > 0) {
                                        if (iv.intValue() >= hv.intValue()) {
                                            assertEquals(emsg,
                                                    StatusCode.BADREQUEST, st.getCode());
                                        } else {
                                            assertEquals(emsg,
                                                    StatusCode.SUCCESS, st.getCode());
                                        }
                                    } else {
                                        assertEquals(emsg,
                                                StatusCode.SUCCESS, st.getCode());
                                    }
                                }

                                // check parameters configured.
                                tenant = null;
                                try {
                                    tenant = mgr.getTenant(tpath);
                                } catch (Exception e) {
                                    unexpected(e);
                                }

                                if (st.isSuccess()) {
                                    assertEquals(emsg, tname, tenant.getName());
                                    assertEquals(emsg, olddesc, tenant.getDescription());
                                    if (iv == null || iv.intValue() < 0) {
                                        assertEquals(emsg,
                                                oldiv.intValue(), tenant.getIdleTimeout());
                                    } else {
                                        assertEquals(emsg,
                                                iv.intValue(), tenant.getIdleTimeout());
                                    }
                                    if (hv == null || hv.intValue() < 0) {
                                        assertEquals(emsg,
                                                oldhv.intValue(), tenant.getHardTimeout());
                                    } else {
                                        assertEquals(emsg,
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
            assertEquals(tnames.size(), list.size());
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
        VTenantPath[] btplist = new VTenantPath[] {
                null,
                new VTenantPath(null)
        };
        VTenantConfig[] bcflist = new VTenantConfig[] {
                null,
                new VTenantConfig(desc, 65536, 65535),
                new VTenantConfig(desc, 65535, 65536)
        };

        // BADREQUEST cases
        for (VTenantPath path: btplist) {
            st = mgr.addTenant(path, tconf);
            assertEquals((path == null) ? "null" : path.toString(),
                    StatusCode.BADREQUEST, st.getCode());
        }

        for (VTenantConfig conf : bcflist) {
            st = mgr.addTenant(tpath, conf);
            assertEquals((conf == null) ? "null" : conf.toString(),
                    StatusCode.BADREQUEST, st.getCode());
        }

        st = mgr.addTenant(new VTenantPath(""), tconf);
        assertEquals(StatusCode.BADREQUEST, st.getCode());
        st = mgr.addTenant(new VTenantPath("12345678901234567890123456789012"),
                                           tconf);
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
        assertEquals(StatusCode.SUCCESS, st.getCode());

        for (VTenantPath path : btplist) {
            String emsg = (path == null) ? "null" : path.toString();
            st = mgr.modifyTenant(path, tconf, false);
            assertEquals(emsg, StatusCode.BADREQUEST, st.getCode());

            st = mgr.removeTenant(path);
            assertEquals(emsg, StatusCode.BADREQUEST, st.getCode());

            try {
                mgr.getTenant(path);
                fail("Throwing exception was expected.");
            } catch (VTNException e) {
               assertEquals(emsg,
                       StatusCode.BADREQUEST,e.getStatus().getCode());
            }
        }

        for (VTenantConfig conf : bcflist) {
            st = mgr.modifyTenant(tpath, conf, true);
            assertEquals((conf == null) ? "null" : conf.toString(),
                    StatusCode.BADREQUEST, st.getCode());
        }

        // conflict cases
        st = mgr.addTenant(tpath, tconf);
        assertEquals(StatusCode.CONFLICT, st.getCode());

        // not found cases
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
        assertEquals(StatusCode.SUCCESS, st.getCode());

        // remove and modify tenant which don't exist.
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
            assertEquals("(VTenantPath)" + tpath.toString(),
                    StatusCode.SUCCESS, st.getCode());

            for (String bname : blist) {
                if (bname.isEmpty()) {
                    continue; // This is a invalid condition.
                }
                VBridgePath bpath = new VBridgePath(tname, bname);
                for (String desc : createStrings("desc")) {
                    for (Integer age : ages) {
                        VBridgeConfig bconf = createVBridgeConfig(desc, age);
                        String emsg = "(VBridgePath)" + bpath.toString()
                                + "(VBridgeConfig)" + bconf.toString()
                                + "(age)" + ((age == null) ? "null" : age.intValue());

                        st = mgr.addBridge(bpath, bconf);
                        assertEquals(emsg, StatusCode.SUCCESS, st.getCode());

                        VBridge brdg = null;
                        try {
                            brdg = mgr.getBridge(bpath);
                        } catch (Exception e) {
                            unexpected(e);
                        }
                        assertEquals(emsg, bname, brdg.getName());
                        assertEquals(emsg, desc, brdg.getDescription());
                        if (age == null) {
                            assertEquals(emsg, 600, brdg.getAgeInterval());
                        } else {
                            assertEquals(emsg, age.intValue(), brdg.getAgeInterval());
                        }
                        assertEquals(emsg, VNodeState.UNKNOWN, brdg.getState());

                        String olddesc = brdg.getDescription();
                        int oldage = brdg.getAgeInterval();

                        // test modifyBridge()
                        for (String newdesc : createStrings("desc")) {
                            for (Integer newage : ages) {
                                bconf = createVBridgeConfig(newdesc, newage);
                                st = mgr.modifyBridge(bpath, bconf, false);

                                String emsgMod = emsg
                                        + "(VBridgeConfig(new))" + bconf.toString()
                                        + "(age(new))"
                                        + ((newage == null) ? "null" : newage.intValue());

                                brdg = null;
                                try {
                                    brdg = mgr.getBridge(bpath);
                                } catch (Exception e) {
                                    unexpected(e);
                                }
                                assertEquals(emsgMod, bname, brdg.getName());
                                if (newdesc == null) {
                                    assertEquals(emsgMod,
                                            olddesc, brdg.getDescription());
                                } else {
                                    assertEquals(emsgMod,
                                            newdesc, brdg.getDescription());
                                    olddesc = newdesc;
                                }
                                if (newage == null) {
                                    assertEquals(emsgMod,
                                            oldage, brdg.getAgeInterval());
                                } else {
                                    assertEquals(emsgMod,
                                            newage.intValue(), brdg.getAgeInterval());
                                    oldage = newage.intValue();
                                }
                                assertEquals(emsgMod,
                                        VNodeState.UNKNOWN, brdg.getState());
                            }
                        }

                        st = mgr.removeBridge(bpath);
                        assertEquals(emsg, StatusCode.SUCCESS, st.getCode());
                    }

                    // in case all == true, test at first time only.
                    if (first) {
                        VBridgeConfig bconf = new VBridgeConfig("desc", 10);
                        st = mgr.addBridge(bpath, bconf);
                        for (String newdesc : createStrings("desc")) {
                            for (Integer newage : ages) {
                                bconf = createVBridgeConfig(newdesc, newage);
                                st = mgr.modifyBridge(bpath, bconf, true);

                                String emsgMod = "(VBridgePath)" + bpath.toString()
                                        + "(VBridgeConfig)" + bconf.toString();
                                VBridge brdg = null;
                                try {
                                    brdg = mgr.getBridge(bpath);
                                } catch (Exception e) {
                                    unexpected(e);
                                }
                                assertEquals(emsgMod, bname, brdg.getName());
                                assertEquals(emsgMod, newdesc, brdg.getDescription());
                                if (newage == null) {
                                    assertEquals(emsgMod, 600, brdg.getAgeInterval());
                                } else {
                                    assertEquals(emsgMod,
                                            newage.intValue(), brdg.getAgeInterval());
                                }
                                assertEquals(emsgMod,
                                        VNodeState.UNKNOWN, brdg.getState());
                            }
                        }

                        st = mgr.removeBridge(bpath);
                        assertEquals("(VBridgePath)" + bpath.toString(),
                                StatusCode.SUCCESS, st.getCode());

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
            assertEquals("(VTenantPath)" + tpath.toString(),
                    StatusCode.SUCCESS, st.getCode());
        }

        // add multiple entries.
        for (String tname : tlist) {
            VTenantPath tpath = new VTenantPath(tname);
            Status st = mgr.addTenant(tpath, new VTenantConfig(null));
            assertEquals("(VTenantPath)" + tpath.toString(),
                    StatusCode.SUCCESS, st.getCode());

            for (String bname : blist) {
                if (bname.isEmpty()) {
                    continue; // This is a invalid condition.
                }
                VBridgePath bpath = new VBridgePath(tname, bname);
                VBridgeConfig bconf = createVBridgeConfig(null, null);

                st = mgr.addBridge(bpath, bconf);
                assertEquals("(VBridgePath)" + bpath.toString()
                        + "(VBridgeConfig)" + bconf.toString(),
                        StatusCode.SUCCESS, st.getCode());
            }
            try {
                List<VBridge> list = mgr.getBridges(tpath);
                assertEquals(tpath.toString(), blist.size() - 1, list.size());
            } catch (VTNException e) {
                unexpected(e);
            }
            st = mgr.removeTenant(tpath);
            assertEquals("(VTenantPath)" + tpath.toString(),
                    StatusCode.SUCCESS, st.getCode());
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
        assertEquals(StatusCode.SUCCESS, st.getCode());
        String bname = "bridge";
        VBridgePath bpath = new VBridgePath(tname, bname);
        VBridgeConfig bconf = createVBridgeConfig(null, null);

        VBridgePath[] bbplist = new VBridgePath[] {
                null,
                new VBridgePath(tname, null),
                new VBridgePath((String)null, bname)
        };
        VBridgeConfig[] bcflist = new VBridgeConfig[] {
                null,
                new VBridgeConfig("desc", 9),
                new VBridgeConfig("desc", 1000001)
        };

        for (VBridgePath path : bbplist) {
            st = mgr.addBridge(path, bconf);
            assertEquals((path == null) ? "null" : path.toString(),
                    StatusCode.BADREQUEST, st.getCode());
        }

        st = mgr.addBridge(new VBridgePath(tname, ""), bconf);
        assertEquals(StatusCode.BADREQUEST, st.getCode());
        st = mgr.addBridge(new VBridgePath(tname, "123456789012345678901234567890_1"),
                                           bconf);
        assertEquals(StatusCode.BADREQUEST, st.getCode());
        st = mgr.addBridge(new VBridgePath(tname, "123456789012345678901234567890:"),
                                           bconf);
        assertEquals(StatusCode.BADREQUEST, st.getCode());
        st = mgr.addBridge(new VBridgePath(tname, "123456789012345678901234567890:"),
                                           bconf);
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
        assertEquals(StatusCode.SUCCESS, st.getCode());

        for (VBridgePath path : bbplist) {
            String emsg = (path == null) ? "null" : path.toString();
            st = mgr.modifyBridge(path, bconf, true);
            assertEquals(emsg, StatusCode.BADREQUEST, st.getCode());
            st = mgr.removeBridge(path);
            assertEquals(emsg, StatusCode.BADREQUEST, st.getCode());

            try {
                mgr.getBridge(path);
                fail("Throwing exception was expected.");
            } catch (VTNException e) {
                assertEquals(emsg,
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
        for (VBridgePath path : nbplist) {
            String emsg = path.toString();
            st = mgr.removeBridge(path);
            assertEquals(emsg, StatusCode.NOTFOUND, st.getCode());

            st = mgr.modifyBridge(path, bconf, true);
            assertEquals(emsg, StatusCode.NOTFOUND, st.getCode());

            try {
                mgr.getBridge(path);
                fail("Throwing exception was expected.");
            } catch (VTNException e) {
                assertEquals(emsg, StatusCode.NOTFOUND, e.getStatus().getCode());
            }
        }

        st = mgr.addBridge(new VBridgePath("tt", bname), bconf);
        assertEquals(StatusCode.NOTFOUND, st.getCode());

        st = mgr.removeTenant(tpath);
        assertEquals(StatusCode.SUCCESS, st.getCode());

        // try to remove and add not existing tenant.
        st = mgr.removeBridge(bpath);
        assertEquals(StatusCode.NOTFOUND, st.getCode());

        st = mgr.addBridge(bpath, bconf);
        assertEquals(StatusCode.NOTFOUND, st.getCode());

        // try to get Bridge by invalid VTenantPath or VBridgePath
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
            assertEquals("(VTenantPath)" + tpath.toString(),
                    StatusCode.SUCCESS, st.getCode());

            for (String bname : blist) {
                VBridgePath bpath = new VBridgePath(tname, bname);
                VBridgeConfig bconf = createVBridgeConfig(null, null);
                st = mgr.addBridge(bpath, bconf);
                assertEquals(bpath.toString(), StatusCode.SUCCESS, st.getCode());

                List<VInterface> list = null;
                try {
                    list = mgr.getBridgeInterfaces(bpath);
                } catch (Exception e) {
                    unexpected(e);
                }
                assertEquals(bpath.toString(), 0, list.size());

                for (String ifname : iflist) {
                    if (ifname.isEmpty()) {
                        continue; // This is a invalid condition.
                    }

                    VBridgeIfPath ifp = new VBridgeIfPath(tname, bname, ifname);

                    // test addBridgeInterface() and getBridgeInterface()
                    for (String desc : createStrings("desc")) {
                        for (Boolean enabled : createBooleans()) {
                            VInterfaceConfig ifconf = new VInterfaceConfig(desc, enabled);

                            st = mgr.addBridgeInterface(ifp, ifconf);
                            String emsg = "(VBridgeIfPath)" + ifp.toString()
                                    + ",(VInterfaceConfig)" + ifconf.toString();
                            assertEquals(emsg, StatusCode.SUCCESS, st.getCode());

                            VInterface vif = null;
                            try {
                                vif = mgr.getBridgeInterface(ifp);
                            } catch (Exception e) {
                                unexpected(e);
                            }

                            assertEquals(emsg, ifname, vif.getName());
                            assertEquals(emsg, desc, vif.getDescription());
                            if (enabled == null) {
                                assertEquals(emsg, Boolean.TRUE, vif.getEnabled());
                            } else {
                                assertEquals(emsg, enabled, vif.getEnabled());
                            }
                            if (enabled == Boolean.FALSE) {
                                assertEquals(emsg, VNodeState.DOWN, vif.getState());
                            } else {
                                assertEquals(emsg, VNodeState.UNKNOWN, vif.getState());
                            }

                            VBridge brdg = null;
                            try {
                                brdg = mgr.getBridge(bpath);
                            } catch (VTNException e) {
                               unexpected(e);
                            }
                            assertEquals(emsg, VNodeState.UNKNOWN, brdg.getState());

                            st = mgr.removeBridgeInterface(ifp);
                            assertEquals(emsg, StatusCode.SUCCESS, st.getCode());
                        }
                    }

                    // for modifyBridgeInterface (all == false)
                    st = mgr.addBridgeInterface(ifp,
                            new VInterfaceConfig("desc", Boolean.FALSE));
                    assertEquals(StatusCode.SUCCESS, st.getCode());

                    String olddesc = new String("desc");
                    Boolean oldenabled = Boolean.FALSE;
                    for (String desc : descs) {
                        for (Boolean enabled : createBooleans()) {
                            VInterfaceConfig ifconf = new VInterfaceConfig(desc, enabled);
                            st = mgr.modifyBridgeInterface(ifp, ifconf, false);
                            String emsg = "(VBridgeIfPath)" + ifp.toString()
                                    + "(VInterfaceConfig)" + ifconf.toString();
                            assertEquals(emsg, StatusCode.SUCCESS, st.getCode());

                            VInterface vif = null;
                            try {
                                vif = mgr.getBridgeInterface(ifp);
                            } catch (Exception e) {
                                unexpected(e);
                            }

                            assertEquals(emsg, ifname, vif.getName());

                            if (desc == null) {
                                assertEquals(emsg, olddesc, vif.getDescription());
                            } else {
                                assertEquals(emsg, desc, vif.getDescription());
                            }

                            Boolean currenabled = enabled;
                            if (enabled == null) {
                                assertEquals(emsg, oldenabled, vif.getEnabled());
                                currenabled = oldenabled;
                            } else {
                                assertEquals(emsg, enabled, vif.getEnabled());
                            }

                            if (currenabled == Boolean.FALSE) {
                                assertEquals(emsg, VNodeState.DOWN, vif.getState());
                            } else {
                                assertEquals(emsg, VNodeState.UNKNOWN, vif.getState());
                            }
                            VBridge brdg = null;
                            try {
                                brdg = mgr.getBridge(bpath);
                            } catch (VTNException e) {
                               unexpected(e);
                            }
                            assertEquals(emsg, VNodeState.UNKNOWN, brdg.getState());

                            olddesc = vif.getDescription();
                            oldenabled = vif.getEnabled();
                        }
                    }

                    // for modifyBridgeInterface (all == true)
                    st = mgr.modifyBridgeInterface(ifp,
                            new VInterfaceConfig("desc", Boolean.FALSE), true);
                    assertEquals("(VBridgeIfPath)" + ifp.toString(),
                            StatusCode.SUCCESS, st.getCode());
                    for (String desc : descs) {
                        for (Boolean enabled : createBooleans()) {
                            VInterfaceConfig ifconf = new VInterfaceConfig(desc, enabled);
                            st = mgr.modifyBridgeInterface(ifp, ifconf, true);

                            String emsg = "(VBridgeIfPath)" + ifp.toString()
                                    + "(VInterfaceConfig)" + ifconf.toString();
                            assertEquals(emsg, StatusCode.SUCCESS, st.getCode());

                            VInterface vif = null;
                            try {
                                vif = mgr.getBridgeInterface(ifp);
                            } catch (Exception e) {
                                unexpected(e);
                            }

                            assertEquals(emsg, ifname, vif.getName());
                            assertEquals(emsg, desc, vif.getDescription());
                            if (enabled == null) {
                                assertEquals(emsg, Boolean.TRUE, vif.getEnabled());
                            } else {
                                assertEquals(emsg, enabled, vif.getEnabled());
                            }

                            if (enabled == Boolean.FALSE) {
                                assertEquals(emsg, VNodeState.DOWN, vif.getState());
                            } else {
                                assertEquals(emsg, VNodeState.UNKNOWN, vif.getState());
                            }

                            VBridge brdg = null;
                            try {
                                brdg = mgr.getBridge(bpath);
                            } catch (VTNException e) {
                               unexpected(e);
                            }
                            assertEquals(emsg, VNodeState.UNKNOWN, brdg.getState());
                        }
                    }
                }

                list = null;
                try {
                    list = mgr.getBridgeInterfaces(bpath);
                } catch (Exception e) {
                    unexpected(e);
                }
                assertEquals("(VBridgePath)" + bpath.toString(), (iflist.size() - 1), list.size());
            }

            st = mgr.removeTenant(tpath);
            assertEquals("(VTenantPath)" + tpath.toString(),
                    StatusCode.SUCCESS, st.getCode());
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
        assertEquals(StatusCode.SUCCESS, st.getCode());
        String bname = "bridge";
        VBridgePath bpath = new VBridgePath(tname, bname);
        st = mgr.addBridge(bpath, new VBridgeConfig(null));
        assertEquals(StatusCode.SUCCESS, st.getCode());
        String ifname = "interface";
        VBridgeIfPath ifpath = new VBridgeIfPath(tname, bname, ifname);
        VInterfaceConfig ifconf = new VInterfaceConfig("desc", Boolean.FALSE);

        VBridgeIfPath[] ifplist = new VBridgeIfPath[] {
                null,
                new VBridgeIfPath(tname, bname, null),
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
        st = mgr.addBridgeInterface(new VBridgeIfPath(tname, bname,
                        "123456789012345678901234567890_1"), ifconf);
        assertEquals(StatusCode.BADREQUEST, st.getCode());
        st = mgr.addBridgeInterface(new VBridgeIfPath(tname, bname,
                        "123456789012345678901234567890:"), ifconf);
        assertEquals(StatusCode.BADREQUEST, st.getCode());
        st = mgr.addBridgeInterface(new VBridgeIfPath(tname, bname,
                "123456789012345678901234567890 "), ifconf);
        assertEquals(StatusCode.BADREQUEST, st.getCode());
        st = mgr.addBridgeInterface(new VBridgeIfPath(tname, bname,
                "_1234567890"), ifconf);
        assertEquals(StatusCode.BADREQUEST, st.getCode());

        // add Interface before modify and remove()
        st = mgr.addBridgeInterface(ifpath, ifconf);
        assertEquals(StatusCode.SUCCESS, st.getCode());

        for (VBridgeIfPath path : ifplist) {
            String emsg = (path == null) ? "null" : path.toString();
            st = mgr.modifyBridgeInterface(path, ifconf, false);
            assertEquals(emsg, StatusCode.BADREQUEST, st.getCode());
            st = mgr.removeBridgeInterface(path);
            assertEquals(emsg, StatusCode.BADREQUEST, st.getCode());

            try {
                mgr.getBridgeInterface(path);
                fail("Throwing exception was expected.");
            } catch (VTNException e) {
                assertEquals(emsg,
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

        for (VBridgeIfPath path : nbplist) {
            String emsg = (path == null) ? "null" : path.toString();
            st = mgr.removeBridgeInterface(path);
            assertEquals(emsg, StatusCode.NOTFOUND, st.getCode());

            st = mgr.modifyBridgeInterface(path, ifconf, false);
            assertEquals(emsg, StatusCode.NOTFOUND, st.getCode());

            if (!(path.getTenantName().equals(tname) &&
                    path.getBridgeName().equals(bname))) {
                st = mgr.addBridgeInterface(path, ifconf);
                assertEquals(emsg, StatusCode.NOTFOUND, st.getCode());
            }

            try {
                mgr.getBridgeInterface(path);
                fail("Throwing exception was expected.");
            } catch (VTNException e) {
                assertEquals(emsg,
                        StatusCode.NOTFOUND, e.getStatus().getCode());
            }
        }

        st = mgr.removeTenant(tpath);
        assertEquals(StatusCode.SUCCESS, st.getCode());

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
     * {@link VTNManagerImpl#getVlanMap(VBridgePath, VlanMapConfig)},
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
        VBridgeIfPath ifp = new VBridgeIfPath(tname, bname, "vinterface");
        List<VBridgePath> bpathlist = new ArrayList<VBridgePath>();
        List<VBridgeIfPath> ifplist = new ArrayList<VBridgeIfPath>();
        bpathlist.add(bpath);
        ifplist.add(ifp);

        createTenantAndBridgeAndInterface(mgr, tpath, bpathlist, ifplist);

        // add a vlanmap to a vbridge
        for (Node node : createNodes(3)) {
            for (short vlan : vlans) {
                VlanMapConfig vlconf = new VlanMapConfig(node, vlan);
                String emsg = "(VlanMapConfig)" + vlconf.toString();
                VlanMap map = null;
                try {
                    map = mgr.addVlanMap(bpath, vlconf);
                    if (node != null && node.getType() != NodeConnector.NodeConnectorIDType.OPENFLOW) {
                        fail("throwing Exception was expected." + emsg);
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
                    assertEquals(getmap, mgr.getVlanMap(bpath, vlconf));
                } catch (VTNException e) {
                    unexpected(e);
                }
                assertEquals(emsg, getmap.getId(), map.getId());
                assertEquals(emsg, getmap.getNode(), node);
                assertEquals(emsg, getmap.getVlan(), vlan);

                VBridge brdg = null;
                try {
                    brdg = mgr.getBridge(bpath);
                } catch (VTNException e) {
                   unexpected(e);
                }
                assertEquals(emsg,
                        (node == null) ? VNodeState.UP : VNodeState.DOWN, brdg.getState());

                st = mgr.removeVlanMap(bpath, map.getId());
                assertEquals(emsg, StatusCode.SUCCESS, st.getCode());
            }
        }

        // add multi vlanmap to a vbridge
        for (Node node : createNodes(3)) {
            for (short vlan : vlans) {
                VlanMapConfig vlconf = new VlanMapConfig(node, vlan);
                VlanMap map = null;
                try {
                    map = mgr.addVlanMap(bpath, vlconf);
                    if (node != null
                            && node.getType() != NodeConnector.NodeConnectorIDType.OPENFLOW) {
                        fail("throwing Exception was expected.");
                    }
                } catch (VTNException e) {
                    if (node != null
                            && node.getType() == NodeConnector.NodeConnectorIDType.OPENFLOW) {
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
            if (node == null
                    || node.getType() == NodeConnector.NodeConnectorIDType.OPENFLOW) {
                assertEquals((node == null) ? "null" : node.toString(),
                        vlans.length, list.size());
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
                assertEquals(node.toString(), 0, list.size());
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
                assertEquals("(VlanMap)" + map.toString(),
                        StatusCode.SUCCESS, st.getCode());
            }
        }

        st = mgr.removeTenant(tpath);
        assertEquals(StatusCode.SUCCESS, st.getCode());
    }

    /**
     * Test method for invalid cases of
     * {@link VTNManagerImpl#addVlanMap(VBridgePath, VlanMapConfig)},
     * {@link VTNManagerImpl#removeVlanMap(VBridgePath, java.lang.String)},
     * {@link VTNManagerImpl#getVlanMap(VBridgePath, java.lang.String)},
     * {@link VTNManagerImpl#getVlanMap(VBridgePath, VlanMapConfig)},
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
        VBridgeIfPath ifp = new VBridgeIfPath(tname, bname, "vinterface");

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

        // try to add vlan map conflict previous one.
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

        // The same VLAN ID can be mapped as long as node differs.
        map = null;
        try {
            map = mgr.addVlanMap(bpath, new VlanMapConfig(node, (short) 0));
        } catch (VTNException e) {
            unexpected(e);
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

        // if mapped node specified with VlanMapConfig don't exist,
        // duplicate Vlanmap succeed.
        try {
            map = mgr.addVlanMap(bpath, new VlanMapConfig(node1, (short) 1));
            map = mgr.addVlanMap(bpath, new VlanMapConfig(node2, (short) 1));
        } catch (Exception e) {
            unexpected(e);
        }
        st = mgr.removeVlanMap(bpath, map.getId());
        assertEquals(StatusCode.SUCCESS, st.getCode());

        // Although VLAN mappings which maps VLAN:1 on specific node exist,
        // VLAN:1 on any switch can be mapped.
        map = null;
        try {
            map = mgr.addVlanMap(bpath, new VlanMapConfig(null, (short) 1));
        } catch (VTNException e) {
            unexpected(e);
        }

        // invalid case
        VBridgeConfig bconf = new VBridgeConfig(null);
        st = mgr.removeBridge(bpath);
        assertEquals(StatusCode.SUCCESS, st.getCode());
        st = mgr.addBridge(bpath, bconf);
        assertEquals(StatusCode.SUCCESS, st.getCode());
        node = NodeCreator.createOFNode(0L);

        // bad request
        VlanMapConfig vlconf = new VlanMapConfig(null, (short) 0);
        VBridgePath[] bbplist = new VBridgePath[] {
                null,
                new VBridgePath(tname, null),
                new VBridgePath((String) null, bname)
        };
        VlanMapConfig[] bvclist = new VlanMapConfig[] {
                null,
                new VlanMapConfig(node, (short) -1),
                new VlanMapConfig(node, (short) 4096)
        };

        for (VBridgePath path : bbplist) {
            String emsg = (path == null) ? "null" : path.toString();
            try {
                map = mgr.addVlanMap(path, vlconf);
                fail("Throwing Exception was expected.");
            } catch (VTNException e) {
                assertEquals(emsg,
                        StatusCode.BADREQUEST, e.getStatus().getCode());
            }
            try {
                map = mgr.addVlanMap(path, new VlanMapConfig(null, (short) 0));
                fail("Throwing Exception was expected.");
            } catch (VTNException e) {
                assertEquals(emsg,
                        StatusCode.BADREQUEST, e.getStatus().getCode());
            }
        }

        for (VlanMapConfig conf : bvclist) {
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
            map = mgr.getVlanMap(bpath, (String)null);
            fail("Throwing Exception was expected.");
        } catch (VTNException e) {
            assertEquals(StatusCode.BADREQUEST, e.getStatus().getCode());
        }

        try {
            map = mgr.getVlanMap(bpath, (VlanMapConfig)null);
            fail("Throwing Exception was expected.");
        } catch (VTNException e) {
            assertEquals(StatusCode.BADREQUEST, e.getStatus().getCode());
        }

        st = mgr.removeVlanMap(bpath, null);
        assertEquals(StatusCode.BADREQUEST, st.getCode());

        List<VlanMap> list = null;
        for (VBridgePath path : bbplist) {
            String emsg = (path == null) ? "null" : path.toString();
            try {
                map = mgr.getVlanMap(path, map.getId());
                fail("Throwing Exception was expected.");
            } catch (VTNException e) {
                assertEquals(emsg,
                        StatusCode.BADREQUEST, e.getStatus().getCode());
            }

            try {
                list = mgr.getVlanMaps(path);
                fail("Throwing Exception was expected.");
            } catch (VTNException e) {
                assertEquals(emsg,
                        StatusCode.BADREQUEST, e.getStatus().getCode());
            }

            st = mgr.removeVlanMap(path, map.getId());
            assertEquals(emsg,
                    StatusCode.BADREQUEST, st.getCode());
        }

        // case Node == null is executed in {@link testVlanMap()}.

        // not found
        VBridgePath[] nbplist = new VBridgePath[] {
            new VBridgePath(tname, "vbridg"),
            new VBridgePath("vtn0", bname)
        };

        for (VBridgePath path : nbplist) {
            String emsg = (path == null) ? "null" : path.toString();
            VlanMapConfig vlc = new VlanMapConfig(node, (short)0);
            try {
                map = mgr.addVlanMap(path, vlc);
                fail("Throwing Exception was expected.");
            } catch (VTNException e) {
                assertEquals(emsg,
                        StatusCode.NOTFOUND, e.getStatus().getCode());
            }

            try {
                map = mgr.getVlanMap(path, map.getId());
                fail("Throwing Exception was expected.");
            } catch (VTNException e) {
                assertEquals(emsg,
                        StatusCode.NOTFOUND, e.getStatus().getCode());
            }

            try {
                map = mgr.getVlanMap(path, vlc);
                fail("Throwing Exception was expected.");
            } catch (VTNException e) {
                assertEquals(emsg, StatusCode.NOTFOUND,
                             e.getStatus().getCode());
            }

            try {
                list = mgr.getVlanMaps(path);
                fail("Throwing Exception was expected.");
            } catch (VTNException e) {
                assertEquals(emsg,
                        StatusCode.NOTFOUND, e.getStatus().getCode());
            }

            st = mgr.removeVlanMap(path, map.getId());
            assertEquals(emsg, StatusCode.NOTFOUND, st.getCode());
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

        try {
            map = mgr.getVlanMap(bpath, map);
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
        assertEquals(StatusCode.SUCCESS, st.getCode());
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
        VBridgeIfPath ifp = new VBridgeIfPath(tname, bname, "vinterface");
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

        for (SwitchPort port : ports) {
            for (short vlan : vlans) {
                PortMapConfig pmconf = new PortMapConfig(node, port, (short)vlan);
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
                assertNull(emsg, map.getNodeConnector());

                VInterface bif = null;
                try {
                    bif = mgr.getBridgeInterface(ifp);
                } catch (VTNException e) {
                    unexpected(e);
                }
                assertEquals(emsg, VNodeState.DOWN, bif.getState());

                VBridge brdg = null;
                try {
                    brdg = mgr.getBridge(bpath);
                } catch (VTNException e) {
                   unexpected(e);
                }
                assertEquals(emsg, VNodeState.DOWN, brdg.getState());
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

        // assign multi portmaps to a vbridge
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

        Node node1 = NodeCreator.createOFNode(0L);
        SwitchPort port1
            = new SwitchPort(NodeConnector.NodeConnectorIDType.OPENFLOW, "10");
        PortMapConfig pmconf1 = new PortMapConfig(node1, port1, (short) 0);
        st = mgr.setPortMap(ifp1, pmconf1);
        assertEquals(StatusCode.SUCCESS, st.getCode());

        SwitchPort port2
            = new SwitchPort( NodeConnector.NodeConnectorIDType.OPENFLOW, "11");
        PortMapConfig pmconf2 = new PortMapConfig(node1, port2, (short) 0);
        st = mgr.setPortMap(ifp2, pmconf2);
        assertEquals(StatusCode.SUCCESS, st.getCode());

        // if specified port is not exist, duplicate portmap success.
        VBridgeIfPath ifp3 = new VBridgeIfPath(tname, bname2, "vinterface3");
        st = mgr.addBridgeInterface(ifp3, new VInterfaceConfig(null, Boolean.TRUE));
        st = mgr.setPortMap(ifp3, pmconf1);
        assertEquals(StatusCode.SUCCESS, st.getCode());

        st = mgr.removeTenant(tpath);
        assertEquals(StatusCode.SUCCESS, st.getCode());
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
        SwitchPort port
            = new SwitchPort("port-1", NodeConnector.NodeConnectorIDType.OPENFLOW, "1");
        PortMapConfig pmconf = new PortMapConfig(node, port, (short) 0);
        Status st = mgr.setPortMap(ifp, pmconf);
        assertEquals(StatusCode.SUCCESS, st.getCode());

        // bad request
        VBridgeIfPath[] biflist = new VBridgeIfPath[] {
                null, new VBridgeIfPath(tname, bname, null),
                new VBridgeIfPath(tname, null, ifname),
                new VBridgeIfPath(null, bname, ifname)
        };
        SwitchPort[] badports = new SwitchPort[] {
                new SwitchPort("port-10", NodeConnector.NodeConnectorIDType.OPENFLOW, null),
                new SwitchPort("port-11", NodeConnector.NodeConnectorIDType.OPENFLOW, "10"),
                new SwitchPort(null, null, "16"),
                new SwitchPort(null, null, null),
                new SwitchPort("", NodeConnector.NodeConnectorIDType.OPENFLOW, null),
                new SwitchPort(NodeConnector.NodeConnectorIDType.ONEPK, "10"),
        };
        PortMapConfig[] bpmlist = new PortMapConfig[] {
                new PortMapConfig(null, port, (short) 0),
                new PortMapConfig(node, null, (short) 0),
                new PortMapConfig(node, null, (short) 0),
                new PortMapConfig(node, port, (short)-1),
                new PortMapConfig(node, port, (short)4096)
        };

        for (VBridgeIfPath path : biflist) {
            String emsg = (path == null) ? "null" : path.toString();
            st = mgr.setPortMap(path, pmconf);
            assertEquals(emsg, StatusCode.BADREQUEST, st.getCode());

            try {
                mgr.getPortMap(path);
                fail("throwing exception was expected.");
            } catch (VTNException e) {
                assertEquals(emsg,
                        StatusCode.BADREQUEST, e.getStatus().getCode());
            }
        }

        for (SwitchPort sw : badports) {
            mgr.setPortMap(ifp, new PortMapConfig(node, sw, (short)0));
            assertEquals(sw.toString(), StatusCode.BADREQUEST, st.getCode());
        }

        for (PortMapConfig map : bpmlist) {
            st = mgr.setPortMap(ifp, map);
            assertEquals(map.toString(), StatusCode.BADREQUEST, st.getCode());
        }

        Node pnode = null;
        try {
            pnode = new Node(Node.NodeIDType.PRODUCTION, "Node ID: 0");
        } catch (ConstructionException e1) {
            unexpected(e1);
        }
        st = mgr.setPortMap(ifp, new PortMapConfig(pnode, new SwitchPort("port-1"),
                            (short) 0));
        assertEquals(StatusCode.BADREQUEST, st.getCode());

        // not found
        VBridgeIfPath[] niflist = new VBridgeIfPath[] {
                new VBridgeIfPath(tname, bname, "ii"),
                new VBridgeIfPath(tname, "bbb", ifname),
                new VBridgeIfPath("vv", bname, ifname)
        };

        for (VBridgeIfPath path : niflist) {
            st = mgr.setPortMap(path, pmconf);
            assertEquals(StatusCode.NOTFOUND, st.getCode());
            try {
                mgr.getPortMap(path);
                fail("throwing exception was expected.");
            } catch (VTNException e) {
                assertEquals(path.toString(),
                        StatusCode.NOTFOUND, e.getStatus().getCode());
            }
        }

        // in container mode
        mgr.containerModeUpdated(UpdateType.ADDED);
        st = mgr.setPortMap(ifp, pmconf);
        assertEquals(StatusCode.NOTACCEPTABLE, st.getCode());
        mgr.containerModeUpdated(UpdateType.REMOVED);

        st = mgr.removeTenant(tpath);
        assertEquals(StatusCode.SUCCESS, st.getCode());
    }

    /**
     * Test case for MAC mapping.
     *
     * <ul>
     *   <li>{@link VTNManagerImpl#getMacMap(VBridgePath)}</li>
     *   <li>{@link VTNManagerImpl#getMacMapConfig(VBridgePath, MacMapAclType)}</li>
     *   <li>{@link VTNManagerImpl#getMacMappedHosts(VBridgePath)}</li>
     *   <li>{@link VTNManagerImpl#getMacMappedHost(VBridgePath, DataLinkAddress)}</li>
     *   <li>{@link VTNManagerImpl#setMacMap(VBridgePath, UpdateOperation, MacMapConfig)}</li>
     *   <li>{@link VTNManagerImpl#setMacMap(VBridgePath, UpdateOperation, MacMapAclType, Set)}</li>
     * </ul>
     * <p>
     *   This method also has test cases for {@link GlobalResourceManager}
     *   class related to MAC mapping.
     * </p>
     */
    @Test
    public void testMacMap() {
        final int NUM_PORTS = 5;
        final int MAX_PORT_IDX = NUM_PORTS - 1;

        /**
         * Test environment for MAC mapping.
         */
        class TestEnv {
            private final List<NodeConnector> portList;

            private final Map<Short, NodeConnector> mappedPorts =
                new HashMap<Short, NodeConnector>();
            private final Map<PortVlan, Integer>  mappedNetworks =
                new HashMap<PortVlan, Integer>();

            private Map<Long, MacAddressEntry>  mappedHosts =
                new HashMap<Long, MacAddressEntry>();
            private final Set<Short>  anyMap = new HashSet<Short>();

            private final VBridgePath  bridgePath;
            private MacMapConfig  config = new MacMapConfig(null, null);
            private boolean  activated;

            private final byte[]  dstAddr = {
                (byte)192, (byte)168, (byte)20, (byte)254,
            };

            private TestEnv(VBridgePath path, int index,
                            List<NodeConnector> ports) {
                bridgePath = path;
                portList = ports;
            }

            private void reset() {
                mappedNetworks.clear();
                mappedHosts.clear();
                anyMap.clear();
                activated = false;
            }

            private MacMapConfig update(Set<DataLinkHost> allow,
                                        Set<DataLinkHost> deny) {
                config = new MacMapConfig(allow, deny);
                return config;
            }

            private void assign(int portIndex, short vlan) {
                NodeConnector port = portList.get(portIndex);
                assertNull(mappedPorts.put(vlan, port));
            }

            private NodeConnector getPort(short vlan) {
                NodeConnector port = mappedPorts.get(vlan);
                assertNotNull(port);
                return port;
            }

            private void activate(byte[] src)
                throws VTNException {
                for (DataLinkHost dlh: config.getAllowedHosts()) {
                    src[3]++;
                    InetAddress ipaddr = createInetAddress(src);
                    EthernetHost eh = (EthernetHost)dlh;
                    EthernetAddress eaddr = eh.getAddress();
                    short vlan = eh.getVlan();
                    if (eaddr == null) {
                        // Wildcard mapping will be tested later.
                        assertTrue(anyMap.add(vlan));
                    } else {
                        activate(eaddr, vlan, ipaddr);
                    }
                }
            }

            private void activate(EthernetAddress eaddr, short vlan,
                                  InetAddress ipaddr) throws VTNException {
                byte[] srcMac = eaddr.getValue();
                byte[] dstMac = NetUtils.getBroadcastMACAddr();
                byte[] srcAddr = ipaddr.getAddress();
                String emsg = "eaddr = " + eaddr + ", vlan = " + vlan;
                NodeConnector port = mappedPorts.get(vlan);
                assertNotNull(emsg, port);
                short v = (vlan == 0) ? -1 : vlan;

                // The specified source host must be mapped by the
                // MAC mapping.
                IVTNResourceManager resMgr = vtnMgr.getResourceManager();
                MacMapPath mpath = new MacMapPath(bridgePath);
                MacVlan mvlan = new MacVlan(srcMac, vlan);
                String cname = vtnMgr.getContainerName();
                MapReference ref = vtnMgr.getMapReference(mpath);
                assertEquals(emsg, ref, resMgr.getMapReference(mvlan));
                assertNull(emsg, vtnMgr.getMacMappedHost(bridgePath, eaddr));

                // Ensure that the MAC mapping is not established.
                PortVlan pvlan = new PortVlan(port, vlan);
                checkNotMapped(ref, mvlan, pvlan);

                // Send an ARP packet.
                RawPacket pkt = createARPRawPacket(srcMac, dstMac, srcAddr,
                                                   dstAddr, v, port,
                                                   ARP.REQUEST);
                assertEquals(emsg, PacketResult.KEEP_PROCESSING,
                             vtnMgr.receiveDataPacket(pkt));
                flushTasks();

                MacAddressEntry ment =
                    vtnMgr.getMacMappedHost(bridgePath, eaddr);
                assertNotNull(emsg, ment);
                assertEquals(emsg, eaddr, ment.getAddress());
                assertEquals(emsg, vlan, ment.getVlan());
                assertEquals(emsg, ment,
                             vtnMgr.getMacEntry(bridgePath, eaddr));
                Set<InetAddress> iset = new HashSet<InetAddress>();
                assertTrue(emsg, iset.add(ipaddr));
                assertEquals(emsg, iset, ment.getInetAddresses());
                long mac = mvlan.getMacAddress();
                assertNull(emsg, mappedHosts.put(mac, ment));

                activated = true;
                assertTrue(resMgr.hasMacMappedHost(vtnMgr, mpath));
                assertEquals(port,
                             resMgr.getMacMappedPort(vtnMgr, mpath, mvlan));
                assertEquals(pvlan,
                             resMgr.getMacMappedNetwork(vtnMgr, mpath, mac));
                Integer cnt = mappedNetworks.get(pvlan);
                if (cnt == null) {
                    cnt = Integer.valueOf(1);
                } else {
                    cnt = Integer.valueOf(cnt.intValue() + 1);
                }
                mappedNetworks.put(pvlan, cnt);
                assertEquals(mappedNetworks.keySet(),
                             resMgr.getMacMappedNetworks(vtnMgr, mpath));
                checkState();

                // Send ARP packet again.
                assertEquals(emsg, PacketResult.KEEP_PROCESSING,
                             vtnMgr.receiveDataPacket(pkt));
                flushTasks();
                checkState();

                // Specify unknown address to getMacMappedHost().
                assertNull(vtnMgr.getMacMappedHost(bridgePath,
                                                   new TestDataLink("addr")));
                assertNull(vtnMgr.getMacMappedHost(bridgePath, null));
            }

            private long activateAny(long mac, byte[] src)
                throws VTNException {
                for (short vlan: anyMap) {
                    EthernetAddress eaddr = createEthernetAddresses(mac);
                    InetAddress ipaddr = createInetAddress(src);
                    activate(eaddr, vlan, ipaddr);
                    mac++;
                }

                return mac;
            }

            private boolean inactivate(short vlan) throws VTNException {
                NodeConnector port = mappedPorts.get(vlan);
                assertNotNull(port);

                SpecificPortFilter filter = new SpecificPortFilter(port);
                IVTNResourceManager resMgr = vtnMgr.getResourceManager();
                MacMapPath mpath = new MacMapPath(bridgePath);
                return resMgr.inactivateMacMap(vtnMgr, mpath, filter);
            }

            private void checkIgnored(EthernetHost host)
                throws VTNException {
                checkIgnored(host, null, null);
            }

            private void checkIgnored(EthernetHost host, NodeConnector port,
                                      MapReference resv)
                throws VTNException {
                byte[] src = {
                    (byte)192, (byte)168, (byte)50, (byte)1,
                };
                byte[] dst = {
                    (byte)192, (byte)168, (byte)50, (byte)99,
                };

                EthernetAddress eaddr = host.getAddress();
                short vlan = host.getVlan();
                String emsg = "eaddr = " + eaddr + ", vlan = " + vlan;
                if (port == null) {
                    port = mappedPorts.get(vlan);
                    if (port == null) {
                        port = portList.get(0);
                    }
                }
                short v = (vlan == 0) ? -1 : vlan;

                // Send an ARP packet.
                byte[] srcMac = eaddr.getValue();
                byte[] dstMac = NetUtils.getBroadcastMACAddr();
                RawPacket pkt = createARPRawPacket(srcMac, dstMac, src, dst,
                                                   v, port, ARP.REQUEST);
                assertEquals(emsg, PacketResult.IGNORED,
                             vtnMgr.receiveDataPacket(pkt));
                flushTasks();

                MacMapPath mpath = new MacMapPath(bridgePath);
                MapReference ref = vtnMgr.getMapReference(mpath);
                MacVlan mvlan = new MacVlan(eaddr.getValue(), vlan);
                PortVlan pvlan = new PortVlan(port, vlan);
                checkNotMapped(ref, mvlan, pvlan, resv);
                checkState();

                // Send ARP packet again.
                assertEquals(emsg, PacketResult.IGNORED,
                             vtnMgr.receiveDataPacket(pkt));
                flushTasks();
                checkNotMapped(ref, mvlan, pvlan, resv);
                checkState();
            }

            private void checkDuplicate(EthernetAddress eaddr, short vlan)
                throws VTNException {
                String emsg = "eaddr = " + eaddr + ", vlan = " + vlan;
                byte[] srcMac = eaddr.getValue();
                MacVlan mvlan = new MacVlan(srcMac, vlan);
                long mac = mvlan.getMacAddress();
                MacAddressEntry ment = mappedHosts.get(mac);
                assertNotNull(emsg, ment);

                Set<InetAddress> iset = ment.getInetAddresses();
                InetAddress ipaddr = iset.iterator().next();

                // Send ARP packet.
                byte[] dstMac = NetUtils.getBroadcastMACAddr();
                byte[] srcAddr = ipaddr.getAddress();
                NodeConnector port = ment.getNodeConnector();
                short v = (vlan == 0) ? -1 : vlan;
                RawPacket pkt = createARPRawPacket(srcMac, dstMac, srcAddr,
                                                   dstAddr, v, port,
                                                   ARP.REQUEST);
                for (int i = 0; i < 2; i++) {
                    assertEquals(emsg, PacketResult.IGNORED,
                                 vtnMgr.receiveDataPacket(pkt));
                    flushTasks();
                    checkState();
                }
            }

            private void checkState() throws VTNException {
                MacMap mcmap = vtnMgr.getMacMap(bridgePath);
                assertNotNull(mcmap);
                assertTrue(config.equals(mcmap));
                List<MacAddressEntry> mlist = mcmap.getMappedHosts();
                assertEquals(mappedHosts.size(), mlist.size());
                HashSet<MacAddressEntry> mset =
                    new HashSet<MacAddressEntry>(mappedHosts.values());
                assertTrue(mset.containsAll(mlist));

                mlist = vtnMgr.getMacEntries(bridgePath);
                assertEquals(mappedHosts.size(), mlist.size());
                assertTrue(mset.containsAll(mlist));
            }

            private void checkNotMapped(MapReference ref, MacVlan mvlan,
                                        PortVlan pvlan)
                throws VTNException {
                checkNotMapped(ref, mvlan, pvlan, null);
            }

            private void checkNotMapped(MapReference ref, MacVlan mvlan,
                                        PortVlan pvlan, MapReference resv)
                throws VTNException {
                IVTNResourceManager resMgr = vtnMgr.getResourceManager();
                MacMapPath mpath = (MacMapPath)ref.getPath();
                long mac = mvlan.getMacAddress();
                assertEquals(activated,
                             resMgr.hasMacMappedHost(vtnMgr, mpath));
                assertNull(resMgr.getMacMappedPort(vtnMgr, mpath, mvlan));
                assertNull(resMgr.getMacMappedNetwork(vtnMgr, mpath, mac));
                Set<PortVlan> nw = (activated)
                    ? mappedNetworks.keySet() : null;
                assertEquals(nw, resMgr.getMacMappedNetworks(vtnMgr, mpath));

                MapReference exref;
                if (resv != null) {
                    // The specified VLAN on a port should be reserved by
                    // another mapping.
                    exref = resv;
                } else if (nw != null && nw.contains(pvlan)) {
                    // The specified VLAN on a port should be reserved by
                    // this MAC mapping.
                    exref = ref;
                } else {
                    // The specified VLAN on a port should not be reserved by
                    // any mapping.
                    exref = null;
                }
                assertEquals(exref, resMgr.getMapReference(pvlan));

                EthernetAddress eaddr = createEthernetAddresses(mac);
                assertNull(vtnMgr.getMacEntry(bridgePath, eaddr));
            }

            private void checkInactivated(Set<DataLinkHost> hset)
                throws VTNException {
                List<MacVlan> removed = new ArrayList<MacVlan>();
                Set<Short> unmapped = new HashSet<Short>();
                for (DataLinkHost host: hset) {
                    EthernetHost eh = (EthernetHost)host;
                    short vlan = eh.getVlan();
                    EthernetAddress eaddr = eh.getAddress();
                    if (eaddr != null) {
                        removed.add(new MacVlan(eaddr.getValue(), vlan));
                    } else {
                        // Wildcard mapping is inactivated.
                        unmapped.add(vlan);
                    }
                }

                Set<DataLinkHost> allow = config.getAllowedHosts();
                for (MacAddressEntry ment: mappedHosts.values()) {
                    short vlan = ment.getVlan();
                    EthernetAddress eaddr = (EthernetAddress)ment.getAddress();
                    DataLinkHost h = new EthernetHost(eaddr, vlan);
                    if (unmapped.contains(vlan) && !allow.contains(h)) {
                        removed.add(new MacVlan(eaddr.getValue(), vlan));
                    }
                }

                for (MacVlan mvlan: removed) {
                    long mac = mvlan.getMacAddress();
                    short vlan = mvlan.getVlan();
                    NodeConnector port = mappedPorts.get(vlan);
                    assertNotNull(port);
                    PortVlan pvlan = new PortVlan(port, vlan);
                    MacAddressEntry ment = mappedHosts.remove(mac);
                    if (ment != null) {
                        assertEquals(port, ment.getNodeConnector());
                        Integer cnt = mappedNetworks.remove(pvlan);
                        assertNotNull(cnt);
                        int c = cnt.intValue() - 1;
                        if (c > 0) {
                            mappedNetworks.put(pvlan, Integer.valueOf(c));
                        }

                        if (mappedHosts.isEmpty()) {
                            activated = false;
                        }
                    }
                }

                MacMapPath mpath = new MacMapPath(bridgePath);
                MapReference ref = vtnMgr.getMapReference(mpath);

                for (MacVlan mvlan: removed) {
                    short vlan = mvlan.getVlan();
                    NodeConnector port = mappedPorts.get(vlan);
                    assertNotNull(port);
                    PortVlan pvlan = new PortVlan(port, vlan);
                    checkNotMapped(ref, mvlan, pvlan);
                }
                checkState();
            }

            private void checkHostMoving(NodeConnector testPort)
                throws VTNException {
                for (MacAddressEntry ment: mappedHosts.values()) {
                    checkHostMoving(ment, testPort);
                }
            }

            private void checkHostMoving(MacAddressEntry ment,
                                         NodeConnector testPort)
                throws VTNException {
                // Ensure that the specified MAC address entry is mapped by
                // the MAC mapping.
                String emsg = "ment = " + ment;
                IVTNResourceManager resMgr = vtnMgr.getResourceManager();
                MacMapPath mpath = new MacMapPath(bridgePath);
                MapReference ref = vtnMgr.getMapReference(mpath);
                EthernetAddress eaddr = (EthernetAddress)ment.getAddress();
                short vlan = ment.getVlan();
                byte[] srcMac = eaddr.getValue();
                MacVlan mvlan = new MacVlan(srcMac, vlan);
                NodeConnector port = ment.getNodeConnector();
                assertEquals(emsg, port,
                             resMgr.getMacMappedPort(vtnMgr, mpath, mvlan));
                assertEquals(emsg, ment,
                             vtnMgr.getMacMappedHost(bridgePath, eaddr));

                // Send an ARP packet with changing incoming port.
                Set<InetAddress> iset = ment.getInetAddresses();
                InetAddress ipaddr = iset.iterator().next();
                byte[] dstMac = NetUtils.getBroadcastMACAddr();
                byte[] srcAddr = ipaddr.getAddress();
                short v = (vlan == 0) ? -1 : vlan;
                RawPacket pkt = createARPRawPacket(srcMac, dstMac, srcAddr,
                                                   dstAddr, v, testPort,
                                                   ARP.REQUEST);
                assertEquals(emsg, PacketResult.KEEP_PROCESSING,
                             vtnMgr.receiveDataPacket(pkt));
                flushTasks();

                // Ensure that the location of the host was changed.
                MacAddressEntry newent =
                    vtnMgr.getMacMappedHost(bridgePath, eaddr);
                assertNotNull(emsg, newent);
                assertEquals(emsg, eaddr, newent.getAddress());
                assertEquals(emsg, vlan, newent.getVlan());
                assertEquals(emsg, iset, newent.getInetAddresses());
                assertEquals(emsg, testPort, newent.getNodeConnector());
                assertEquals(emsg, newent,
                             vtnMgr.getMacEntry(bridgePath, eaddr));
                assertEquals(emsg, testPort,
                             resMgr.getMacMappedPort(vtnMgr, mpath, mvlan));

                long mac = mvlan.getMacAddress();
                PortVlan pvlan = new PortVlan(testPort, vlan);
                assertEquals(emsg, pvlan,
                             resMgr.getMacMappedNetwork(vtnMgr, mpath, mac));
                assertEquals(emsg, ref, resMgr.getMapReference(pvlan));

                // Restore the location of the host.
                pkt = createARPRawPacket(srcMac, dstMac, srcAddr, dstAddr, v,
                                         port, ARP.REQUEST);
                assertEquals(emsg, PacketResult.KEEP_PROCESSING,
                             vtnMgr.receiveDataPacket(pkt));
                flushTasks();

                assertNull(emsg, resMgr.getMapReference(pvlan));
                assertEquals(emsg, ment,
                             vtnMgr.getMacMappedHost(bridgePath, eaddr));
                assertEquals(emsg, ment,
                             vtnMgr.getMacEntry(bridgePath, eaddr));
                assertEquals(emsg, port,
                             resMgr.getMacMappedPort(vtnMgr, mpath, mvlan));
                pvlan = new PortVlan(port, vlan);
                assertEquals(pvlan,
                             resMgr.getMacMappedNetwork(vtnMgr, mpath, mac));
                assertEquals(emsg, ref, resMgr.getMapReference(pvlan));
                checkState();
            }
        }

        VTNManagerImpl mgr = vtnMgr;
        VTenantPath tpath = new VTenantPath("tenant");
        VBridgePath bpath1 = new VBridgePath(tpath, "bridge_1");
        VBridgePath bpath2 = new VBridgePath(tpath, "bridge_2");
        List<VBridgePath> bpaths = new ArrayList<VBridgePath>();
        bpaths.add(bpath1);
        bpaths.add(bpath2);
        createTenantAndBridge(mgr, tpath, bpaths);

        long untestedMac = 0x007777777777L;
        DataLinkHost untested = createEthernetHost(untestedMac, (short)0);
        short untestedVlan = 4094;
        Set<DataLinkHost> hset = new HashSet<DataLinkHost>();
        assertTrue(hset.add(untested));
        MacMapConfig mcconf = new MacMapConfig(hset, null);

        //
        // Test cases for illegal arguments.
        //

        // Pass illegal vBridge path.
        bpaths.clear();
        bpaths.add(null);
        bpaths.add(new VBridgePath(tpath, null));
        bpaths.add(new VBridgePath((String)null, "bridge"));
        bpaths.add(new VBridgePath("tenant", null));
        bpaths.add(new VBridgePath(tpath, "bridge_3"));
        for (VBridgePath path: bpaths) {
            StatusCode code =
                (path != null && path.getTenantName() != null &&
                 path.getBridgeName() != null)
                ? StatusCode.NOTFOUND
                : StatusCode.BADREQUEST;

            errorGetMacMap(path, code);
            errorGetMacMapConfig(path, MacMapAclType.ALLOW, code);
            errorGetMacMappedHosts(path, code);
            errorGetMacMappedHost(path, untested.getAddress(), code);
            errorSetMacMap(path, UpdateOperation.SET, mcconf, code);
            errorSetMacMap(path, UpdateOperation.SET, MacMapAclType.ALLOW, hset,
                           code);
        }

        // Pass null operation.
        errorSetMacMap(bpath1, null, mcconf, StatusCode.BADREQUEST);
        errorSetMacMap(bpath1, null, MacMapAclType.ALLOW, hset,
                       StatusCode.BADREQUEST);

        // Pass null ACL type.
        errorSetMacMap(bpath1, UpdateOperation.SET, null, hset,
                       StatusCode.BADREQUEST);

        // Pass a set of DataLinkHost which contains null.
        {
            assertTrue(hset.add(null));
            MacMapConfig cf1 = new MacMapConfig(hset, null);
            MacMapConfig cf2 = new MacMapConfig(null, hset);
            for (UpdateOperation op: UpdateOperation.values()) {
                errorSetMacMap(bpath1, op, cf1, StatusCode.BADREQUEST);
                errorSetMacMap(bpath1, op, cf2, StatusCode.BADREQUEST);
                errorSetMacMap(bpath1, op, MacMapAclType.ALLOW, hset,
                               StatusCode.BADREQUEST);
                errorSetMacMap(bpath1, op, MacMapAclType.DENY, hset,
                               StatusCode.BADREQUEST);
            }
            assertTrue(hset.remove(null));
        }

        // Pass unsupported host information.
        {
            TestDataLink dladdr = new TestDataLink("addr");
            TestDataLinkHost dh = new TestDataLinkHost(dladdr);
            assertTrue(hset.add(dh));

            MacMapConfig cf1 = new MacMapConfig(hset, null);
            MacMapConfig cf2 = new MacMapConfig(null, hset);
            for (UpdateOperation op: UpdateOperation.values()) {
                errorSetMacMap(bpath1, op, cf1, StatusCode.BADREQUEST);
                errorSetMacMap(bpath1, op, cf2, StatusCode.BADREQUEST);
                errorSetMacMap(bpath1, op, MacMapAclType.ALLOW, hset,
                               StatusCode.BADREQUEST);
                errorSetMacMap(bpath1, op, MacMapAclType.DENY, hset,
                               StatusCode.BADREQUEST);
            }
            assertTrue(hset.remove(dh));
        }

        MacMapConfig[] emptyCf = {
            new MacMapConfig(null, null),
            null,
        };
        Set<Set<DataLinkHost>> emptySet = new HashSet<Set<DataLinkHost>>();
        emptySet.add(new HashSet<DataLinkHost>());
        emptySet.add(null);

        try {
            // Getter methods should return null if the MAC mapping is not
            // configured.
            assertNull(mgr.getMacMap(bpath1));
            assertNull(mgr.getMacMappedHosts(bpath1));
            assertNull(mgr.getMacMappedHost(bpath1, untested.getAddress()));
            assertNull(mgr.getMacMapConfig(bpath1, MacMapAclType.ALLOW));
            assertNull(mgr.getMacMapConfig(bpath1, MacMapAclType.DENY));

            // Empty data should be ignored.
            hset.clear();
            for (UpdateOperation op: UpdateOperation.values()) {
                for (MacMapConfig cf: emptyCf) {
                    assertNull(mgr.setMacMap(bpath1, op, cf));
                }
                for (MacMapAclType acl: MacMapAclType.values()) {
                    for (Set<DataLinkHost> s: emptySet) {
                        assertNull(mgr.setMacMap(bpath1, op, acl, s));
                    }
                }
            }
        } catch (Exception e) {
            unexpected(e);
        }

        // Create ports.
        Node node = NodeCreator.createOFNode(0L);
        List<NodeConnector> portList = new ArrayList<NodeConnector>();
        for (short idx = 1; idx <= NUM_PORTS; idx++) {
            NodeConnector port = NodeConnectorCreator.
                createOFNodeConnector(Short.valueOf(idx), node);
            portList.add(port);
        }

        // Create one more port for test case which changes the location of
        // the host.
        NodeConnector movePort = NodeConnectorCreator.
            createOFNodeConnector(Short.valueOf((short)100), node);

        // Determine location of MAC mapped hosts.
        short anyVlan1 = 1000;
        short anyVlan2 = 2000;

        // bridge 1:
        //   port 0, VLAN 0
        //   port 1, VLAN 1
        //   port 2, VLAN 2, VLAN 2000
        //   port 3, VLAN 3
        //   port 4, VLAN 1000
        TestEnv env1 = new TestEnv(bpath1, 1, portList);
        for (int i = 0; i < MAX_PORT_IDX; i++) {
            env1.assign(i, (short)i);
        }
        env1.assign(MAX_PORT_IDX, anyVlan1);
        env1.assign(2, anyVlan2);

        // bridge 2:
        //  port 0, VLAN 2000
        //  port 1, VLAN 2
        //  port 2, VLAN 3, VLAN 1000
        //  port 3, VLAN 1
        //  port 4, VLAN 0
        TestEnv env2 = new TestEnv(bpath2, 2, portList);
        env2.assign(0, anyVlan2);
        env2.assign(1, (short)2);
        env2.assign(2, (short)3);
        env2.assign(3, (short)1);
        env2.assign(3, anyVlan1);
        env2.assign(4, (short)0);

        // Construct a valid MAC mapping configuration.
        Set<DataLinkHost> allow = new HashSet<DataLinkHost>();
        assertTrue(allow.add(new EthernetHost(null, anyVlan1)));
        for (int i = 0; i < 10; i++) {
            long mac = 0x123456789aL + i;
            short vlan = (short)(i % NUM_PORTS);
            if (vlan == MAX_PORT_IDX) {
                vlan = anyVlan2;
            }
            DataLinkHost dlh = createEthernetHost(mac, vlan);
            assertTrue(allow.add(dlh));
        }

        // Add 3 mappings that supersede wildcard mapping.
        assertTrue(allow.add(createEthernetHost(0x11L, anyVlan1)));
        assertTrue(allow.add(createEthernetHost(0x22L, anyVlan1)));
        assertTrue(allow.add(createEthernetHost(0xf0ffffffffffL, anyVlan1)));

        Set<DataLinkHost> deny = new HashSet<DataLinkHost>();
        for (int i = 0; i < 5; i++) {
            long mac = 0xaabbccddeeL + i;
            DataLinkHost dlh = createEthernetHost(mac, anyVlan1);
            assertTrue(deny.add(dlh));

            // The same MAC address can be configured to the denied host set.
            dlh = createEthernetHost(mac, untestedVlan);
            assertTrue(deny.add(dlh));
        }

        // Configure MAC mapping to the bridge 1 at once.
        mcconf = new MacMapConfig(allow, deny);
        try {
            assertNull(mgr.getMacMap(bpath1));
            assertNull(mgr.getMacMap(bpath2));
            assertEquals(UpdateType.ADDED,
                         mgr.setMacMap(bpath1, UpdateOperation.SET, mcconf));
            assertNull(mgr.setMacMap(bpath1, UpdateOperation.SET, mcconf));

            MacMap mcmap = mgr.getMacMap(bpath1);
            assertNotNull(mcmap);
            assertTrue(mcconf.equals(mcmap));
            assertTrue(mcmap.getMappedHosts().isEmpty());
            assertNull(mgr.getMacMap(bpath2));
            mcconf = env1.update(allow, deny);
        } catch (Exception e) {
            unexpected(e);
        }

        UpdateOperation[] ops = {
            UpdateOperation.ADD, UpdateOperation.SET,
        };
        try {
            // Try to append the same hosts to the MAC mapping.
            for (UpdateOperation op: ops) {
                assertNull(mgr.setMacMap(bpath1, op, mcconf));
                assertNull(mgr.setMacMap(bpath1, op, MacMapAclType.ALLOW,
                                         allow));
                assertNull(mgr.setMacMap(bpath1, op, MacMapAclType.DENY,
                                         deny));
            }
            for (DataLinkHost dlh: allow) {
                Set<DataLinkHost> set = new HashSet<DataLinkHost>();
                assertTrue(set.add(dlh));
                assertNull(mgr.setMacMap(bpath1, UpdateOperation.ADD,
                                         MacMapAclType.ALLOW, set));
            }
            for (DataLinkHost dlh: deny) {
                Set<DataLinkHost> set = new HashSet<DataLinkHost>();
                assertTrue(set.add(dlh));
                assertNull(mgr.setMacMap(bpath1, UpdateOperation.ADD,
                                         MacMapAclType.DENY, set));
            }

            // ADD and REMOVE operation should ignore empty data.
            UpdateOperation[] ops2 = {
                UpdateOperation.ADD, UpdateOperation.REMOVE,
            };
            hset.clear();
            for (UpdateOperation op: ops2) {
                for (MacMapConfig cf: emptyCf) {
                    assertNull(mgr.setMacMap(bpath1, op, cf));
                }
                for (MacMapAclType acl: MacMapAclType.values()) {
                    for (Set<DataLinkHost> s: emptySet) {
                        assertNull(mgr.setMacMap(bpath1, op, acl, s));
                    }
                }
            }
        } catch (Exception e) {
            unexpected(e);
        }

        // Try to map the same MAC address with specifying different
        // VLAN ID.
        for (DataLinkHost dlh: allow) {
            EthernetAddress eaddr = (EthernetAddress)dlh.getAddress();
            if (eaddr == null) {
                continue;
            }
            DataLinkHost host = new EthernetHost(eaddr, (short)4095);
            checkMacMapError(bpath1, mcconf, UpdateOperation.ADD,
                             MacMapAclType.ALLOW, host, StatusCode.CONFLICT);
        }

        long[] badMacs = {
            // Broadcast address
            0xffffffffffffL,

            // Multicast address
            0x010000000001L,
            0x0faabbccddeeL,

            // Zero
            0x000000000000L,
        };
        for (UpdateOperation op: ops) {
            // Try to set bad MAC address to the MAC mapping configuration.
            for (long mac: badMacs) {
                DataLinkHost host = createEthernetHost(mac, (short)4093);
                checkMacMapError(bpath1, mcconf, op, MacMapAclType.ALLOW,
                                 host, StatusCode.BADREQUEST);
                checkMacMapError(bpath1, mcconf, op, MacMapAclType.DENY,
                                 host, StatusCode.BADREQUEST);
            }

            // Try to set NULL host to the denied host set.
            for (short vlan = 0; vlan < 10; vlan++) {
                DataLinkHost h = new EthernetHost(null, vlan);
                checkMacMapError(bpath1, mcconf, op, MacMapAclType.DENY,
                                 h, StatusCode.BADREQUEST);
            }
        }

        // Configure MAC mapping to bridge 2.
        UpdateType mmresult = UpdateType.ADDED;
        Set<DataLinkHost> allow2 = new HashSet<DataLinkHost>();
        Set<DataLinkHost> deny2 = new HashSet<DataLinkHost>();
        MacMapConfig mcconf2 = null;
        try {
            Set<DataLinkHost> set = new HashSet<DataLinkHost>();
            UpdateType r;
            long mac = 0xaaaaaaaaL;
            for (DataLinkHost dlh: deny) {
                set.clear();
                assertTrue(set.add(dlh));
                assertTrue(deny2.add(dlh));
                DataLinkHost h = createEthernetHost(mac, anyVlan1);
                mac++;
                assertTrue(set.add(h));
                assertTrue(deny2.add(h));
                r = mgr.setMacMap(bpath2, UpdateOperation.ADD,
                                  MacMapAclType.DENY, set);
                assertEquals(mmresult, r);
                mcconf2 = env2.update(null, deny2);
                MacMap mcmap = mgr.getMacMap(bpath2);
                assertNotNull(mcmap);
                assertTrue(mcconf2.equals(mcmap));
                assertTrue(mcmap.getMappedHosts().isEmpty());
                mmresult = UpdateType.CHANGED;
            }

            for (int i = 0; i < 10; i++) {
                set.clear();
                if (i == 0) {
                    DataLinkHost h = new EthernetHost(null, anyVlan2);
                    assertTrue(set.add(h));
                    assertTrue(allow2.add(h));
                }

                mac = 0xbaddcafeL + i;
                short vlan = (short)(i % NUM_PORTS);
                if (vlan == MAX_PORT_IDX) {
                    vlan = anyVlan1;
                }
                DataLinkHost dlh = createEthernetHost(mac, vlan);
                assertTrue(allow2.add(dlh));
                assertTrue(set.add(dlh));

                mac = 0xdeadbeefL + i;
                DataLinkHost h = createEthernetHost(mac, vlan);
                assertTrue(allow2.add(h));
                assertTrue(set.add(h));
                r = mgr.setMacMap(bpath2, UpdateOperation.ADD,
                                  MacMapAclType.ALLOW, set);
                assertEquals(mmresult, r);
                mcconf2 = env2.update(allow2, deny2);
                MacMap mcmap = mgr.getMacMap(bpath2);
                assertNotNull(mcmap);
                assertTrue(mcconf2.equals(mcmap));
                assertTrue(mcmap.getMappedHosts().isEmpty());
            }
        } catch (Exception e) {
            unexpected(e);
        }

        // Try to map MAC addresses mapped to another vBridge.
        for (UpdateOperation op: ops) {
            for (DataLinkHost dlh: allow) {
                Set<DataLinkHost> set = new HashSet<DataLinkHost>();
                assertTrue(set.add(dlh));
                errorSetMacMap(bpath2, op, MacMapAclType.ALLOW, set,
                               StatusCode.CONFLICT);
            }
            errorSetMacMap(bpath2, op, MacMapAclType.ALLOW, allow,
                           StatusCode.CONFLICT);
            MacMapConfig cf = new MacMapConfig(allow, null);
            errorSetMacMap(bpath2, op, cf, StatusCode.CONFLICT);
        }

        Set<DataLinkHost> allow1 = new HashSet<DataLinkHost>(allow);
        Set<DataLinkHost> deny1 = new HashSet<DataLinkHost>(deny);

        // Ensure that the MAC mapping configurations are not changed.
        try {
            MacMap mcmap = mgr.getMacMap(bpath1);
            assertNotNull(mcmap);
            assertTrue(mcconf.equals(mcmap));
            assertTrue(mcmap.getMappedHosts().isEmpty());

            mcmap = mgr.getMacMap(bpath2);
            assertNotNull(mcmap);
            assertTrue(mcconf2.equals(mcmap));
            assertTrue(mcmap.getMappedHosts().isEmpty());
        } catch (Exception e) {
            unexpected(e);
        }

        byte[] src1 = {(byte)192, (byte)168, (byte)100, (byte)0};
        byte[] src2 = {(byte)192, (byte)168, (byte)200, (byte)0};
        try {
            // Activate MAC mapped hosts by sending ARP packet.
            env1.activate(src1);
            env2.activate(src2);

            for (short vlan = 0; vlan < 30; vlan++) {
                // Unconfigured MAC address should be ignored.
                EthernetHost h = createEthernetHost(untestedMac, vlan);
                env1.checkIgnored(h);
                env2.checkIgnored(h);
            }

            // Test case for MAC addresses mapped by wildcard mapping.
            long mac = 0x9988776655L;
            for (int i = 0; i < 10; i++) {
                mac = env1.activateAny(mac, src1);
                mac = env2.activateAny(mac, src2);
            }

            // Test case for denied host set.
            // All hosts denied by bridge 1 must be ignored because they are
            // also denied by bridge 2.
            for (DataLinkHost dlh: deny) {
                env1.checkIgnored((EthernetHost)dlh);
            }

            for (DataLinkHost dlh: deny2) {
                EthernetHost eh = (EthernetHost)dlh;
                short vlan = eh.getVlan();
                if (vlan == anyVlan1 && !deny.contains(dlh)) {
                    // This should be mapped to bridge 1.
                    EthernetAddress eaddr = eh.getAddress();
                    src1[3]++;
                    InetAddress ipaddr = createInetAddress(src1);
                    env1.activate(eaddr, vlan, ipaddr);
                } else {
                    // This should be ignored.
                    env1.checkIgnored((EthernetHost)dlh);
                }
            }

            // Ensure that a host found on a switch port reserved by another
            // MAC mapping is ignored.
            NodeConnector port = env2.getPort(anyVlan1);
            MapReference resv = mgr.getMapReference(new MacMapPath(bpath2));
            mac = 0x005555555555L;
            for (int i = 0; i < 10; i++) {
                EthernetHost h = createEthernetHost(mac, anyVlan1);
                env1.checkIgnored(h, port, resv);
                mac++;
            }

            port = env1.getPort(anyVlan2);
            resv = mgr.getMapReference(new MacMapPath(bpath1));
            for (int i = 0; i < 10; i++) {
                EthernetHost h = createEthernetHost(mac, anyVlan2);
                env2.checkIgnored(h, port, resv);
                mac++;
            }

            // Ensure that change of host location can be detected.
            env1.checkHostMoving(movePort);
            env2.checkHostMoving(movePort);

            {
                // Remove 1 MAC mapped host from bridge 1 which supersedes
                // wildcard mapping for anyVlan1.
                DataLinkHost any = new EthernetHost(null, anyVlan1);
                assertTrue(allow.remove(any));
                hset.clear();
                for (Iterator<DataLinkHost> it = allow.iterator();
                     it.hasNext();) {
                    DataLinkHost h = it.next();
                    if (((EthernetHost)h).getVlan() == anyVlan1) {
                        hset.add(h);
                        it.remove();
                        break;
                    }
                }
                assertTrue(allow.add(any));

                assertEquals(UpdateType.CHANGED,
                             mgr.setMacMap(bpath1, UpdateOperation.SET,
                                           MacMapAclType.ALLOW, allow));
                mcconf = env1.update(allow, deny);
                env1.checkInactivated(hset);

                // Remove 2 MAC mapped hosts from bridge 1 by MacMapConfig
                // REMOVE operation. This operation will remove hosts mapped
                // by wildcard mapping for anyVlan1.
                hset.clear();
                hset.add(any);
                assertTrue(allow.remove(any));
                for (Iterator<DataLinkHost> it = allow.iterator();
                     it.hasNext();) {
                    DataLinkHost h = it.next();
                    if (((EthernetHost)h).getVlan() != anyVlan1) {
                        hset.add(h);
                        it.remove();
                        break;
                    }
                }
                MacMapConfig cf = new MacMapConfig(hset, null);
                assertEquals(UpdateType.CHANGED,
                             mgr.setMacMap(bpath1, UpdateOperation.REMOVE,
                                           cf));
                mcconf = env1.update(allow, deny);
                env1.checkInactivated(hset);
            }

            // Remove more 2 MAC mapped hosts from bridge 1 by host set
            // REMOVE operation.
            {
                hset.clear();
                Iterator<DataLinkHost> it = allow.iterator();
                for (int i = 0; i < 2; i++) {
                    hset.add(it.next());
                    it.remove();
                }
                assertEquals(UpdateType.CHANGED,
                             mgr.setMacMap(bpath1, UpdateOperation.REMOVE,
                                           MacMapAclType.ALLOW, hset));
                mcconf = env1.update(allow, deny);
                env1.checkInactivated(hset);
            }

            // Remove more 2 MAC mapped hosts from bridge 1 by MacMapConfig
            // SET operation.
            {
                hset.clear();
                Iterator<DataLinkHost> it = allow.iterator();
                for (int i = 0; i < 2; i++) {
                    hset.add(it.next());
                    it.remove();
                }
                mcconf = env1.update(allow, deny);
                assertEquals(UpdateType.CHANGED,
                             mgr.setMacMap(bpath1, UpdateOperation.SET,
                                           mcconf));
                env1.checkInactivated(hset);
            }

            // Remove more 2 MAC mapped hosts from bridge 1 by host set
            // SET operation.
            {
                hset.clear();
                Iterator<DataLinkHost> it = allow.iterator();
                for (int i = 0; i < 2; i++) {
                    hset.add(it.next());
                    it.remove();
                }
                mcconf = env1.update(allow, deny);
                assertEquals(UpdateType.CHANGED,
                             mgr.setMacMap(bpath1, UpdateOperation.SET,
                                           MacMapAclType.ALLOW, allow));
                env1.checkInactivated(hset);
            }

            // Remove all mapped hosts.
            mcconf = env1.update(null, deny);
            assertEquals(UpdateType.CHANGED,
                         mgr.setMacMap(bpath1, UpdateOperation.SET,
                                       MacMapAclType.ALLOW, null));
            env1.checkInactivated(allow);
            assertTrue(mgr.getMacEntries(bpath1).isEmpty());

            // Remove MAC mappig on bridge 1 by REMOVE operation.
            mmresult = UpdateType.CHANGED;
            boolean loop = true;
            for (Iterator<DataLinkHost> it = deny.iterator(); loop;) {
                DataLinkHost h = it.next();
                loop = it.hasNext();
                hset.clear();
                hset.add(h);
                if (!loop) {
                    mmresult = UpdateType.REMOVED;
                }

                MacMapConfig cf = new MacMapConfig(null, hset);
                assertEquals(mmresult,
                             mgr.setMacMap(bpath1, UpdateOperation.REMOVE,
                                           cf));
            }
            assertNull(mgr.getMacMap(bpath1));
            assertEquals(UpdateType.ADDED,
                         mgr.setMacMap(bpath1, UpdateOperation.SET,
                                       MacMapAclType.DENY, deny));
            mmresult = UpdateType.CHANGED;
            loop = true;
            for (Iterator<DataLinkHost> it = deny.iterator(); loop;) {
                DataLinkHost h = it.next();
                loop = it.hasNext();
                hset.clear();
                hset.add(h);
                if (!loop) {
                    mmresult = UpdateType.REMOVED;
                }

                assertEquals(mmresult,
                             mgr.setMacMap(bpath1, UpdateOperation.REMOVE,
                                           MacMapAclType.DENY, hset));
            }
            assertEquals(null, mgr.getMacMap(bpath1));

            assertEquals(UpdateType.ADDED,
                         mgr.setMacMap(bpath1, UpdateOperation.SET,
                                       MacMapAclType.ALLOW, allow));
            mmresult = UpdateType.CHANGED;
            loop = true;
            for (Iterator<DataLinkHost> it = allow.iterator(); loop;) {
                DataLinkHost h = it.next();
                loop = it.hasNext();
                hset.clear();
                hset.add(h);
                if (!loop) {
                    mmresult = UpdateType.REMOVED;
                }

                MacMapConfig cf = new MacMapConfig(hset, null);
                assertEquals(mmresult,
                             mgr.setMacMap(bpath1, UpdateOperation.REMOVE,
                                           cf));
            }
            assertNull(mgr.getMacMap(bpath1));
            assertEquals(UpdateType.ADDED,
                         mgr.setMacMap(bpath1, UpdateOperation.SET,
                                       MacMapAclType.ALLOW, allow));
            mmresult = UpdateType.CHANGED;
            loop = true;
            for (Iterator<DataLinkHost> it = allow.iterator(); loop;) {
                DataLinkHost h = it.next();
                loop = it.hasNext();
                hset.clear();
                hset.add(h);
                if (!loop) {
                    mmresult = UpdateType.REMOVED;
                }

                assertEquals(mmresult,
                             mgr.setMacMap(bpath1, UpdateOperation.REMOVE,
                                           MacMapAclType.ALLOW, hset));
            }
            assertEquals(null, mgr.getMacMap(bpath1));

            // Remove MAC mapping by SET operation.
            mcconf = new MacMapConfig(allow, deny);
            for (MacMapConfig cf: emptyCf) {
                assertEquals(UpdateType.ADDED,
                             mgr.setMacMap(bpath1, UpdateOperation.SET,
                                           mcconf));
                assertEquals(UpdateType.REMOVED,
                             mgr.setMacMap(bpath1, UpdateOperation.SET, cf));
                assertEquals(null, mgr.getMacMap(bpath1));
            }

            for (Set<DataLinkHost> s: emptySet) {
                assertEquals(UpdateType.ADDED,
                             mgr.setMacMap(bpath1, UpdateOperation.SET,
                                           MacMapAclType.DENY, deny));
                assertEquals(UpdateType.REMOVED,
                             mgr.setMacMap(bpath1, UpdateOperation.SET,
                                           MacMapAclType.DENY, s));
                assertNull(mgr.getMacMap(bpath1));

                assertEquals(UpdateType.ADDED,
                             mgr.setMacMap(bpath1, UpdateOperation.SET,
                                           MacMapAclType.ALLOW, allow));
                assertEquals(UpdateType.REMOVED,
                             mgr.setMacMap(bpath1, UpdateOperation.SET,
                                           MacMapAclType.ALLOW, s));
            }

            // Ensure that the same MAC address is not mapped to the vBridge.
            hset.clear();
            short targetVlan = 0;
            Map<EthernetAddress, MacAddressEntry> entMap =
                new HashMap<EthernetAddress, MacAddressEntry>();
            for (DataLinkHost dlh: allow2) {
                EthernetHost eh = (EthernetHost)dlh;
                if (eh.getVlan() == targetVlan) {
                    EthernetAddress eaddr = eh.getAddress();
                    MacAddressEntry ment = vtnMgr.getMacEntry(bpath2, eaddr);
                    assertNotNull(ment);
                    assertNull(entMap.put(eaddr, ment));
                    env2.checkDuplicate(eaddr, anyVlan2);
                    assertTrue(hset.add(dlh));
                }
            }

            // MAC address should be mapped if it is inactivated.
            assertTrue(env2.inactivate(targetVlan));

            // Above operation never purges network caches.
            // So MAC address table entries need to be purged explicitly.
            for (Map.Entry<EthernetAddress, MacAddressEntry> entry:
                     entMap.entrySet()) {
                EthernetAddress eaddr = entry.getKey();
                MacAddressEntry ment = entry.getValue();
                assertEquals(ment, vtnMgr.removeMacEntry(bpath2, eaddr));
            }

            env2.checkInactivated(hset);
            for (DataLinkHost dlh: hset) {
                EthernetAddress eaddr = (EthernetAddress)dlh.getAddress();
                src2[3]++;
                InetAddress ipaddr = createInetAddress(src2);
                env2.activate(eaddr, anyVlan2, ipaddr);
            }

            // Remove MAC mapping on bridge 2.
            assertEquals(UpdateType.REMOVED,
                         mgr.setMacMap(bpath2, UpdateOperation.SET, null));
            assertTrue(mgr.getMacEntries(bpath2).isEmpty());
            for (MacMapConfig cf: emptyCf) {
                assertNull(mgr.setMacMap(bpath2, UpdateOperation.SET, cf));
            }

            // Ensure that we can reproduce MAC mappings.
            env1.reset();
            env2.reset();
            mcconf = env1.update(allow1, deny1);
            mcconf2 = env2.update(allow2, deny2);
            assertEquals(UpdateType.ADDED,
                         mgr.setMacMap(bpath1, UpdateOperation.SET, mcconf));
            assertEquals(UpdateType.ADDED,
                         mgr.setMacMap(bpath2, UpdateOperation.ADD, mcconf2));
            env1.checkState();
            env2.checkState();

            src1[3] = 0;
            src2[3] = 0;
            env1.activate(src1);
            env2.activate(src2);

            // Remove bridge 1.
            mgr.removeBridge(bpath1);

            // Remove tenant.
            mgr.removeTenant(tpath);
        } catch (Exception e) {
            unexpected(e);
        }
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

        byte[] dst = new byte[] {(byte)0xFF, (byte)0xFF, (byte)0xFF,
                                 (byte)0xFF, (byte)0xFF, (byte)0xFF};
        byte[] target = new byte[] {(byte)192, (byte)168, (byte)0, (byte)250};
        byte[] dst2 = new byte[] {(byte)0xFF, (byte)0xFF, (byte)0xFF,
                                  (byte)0xFF, (byte)0xFF, (byte)0xFF};
        byte[] target2 = new byte[] {(byte)192, (byte)1, (byte)0, (byte)250};

        TestBridgeNode bnode = new TestBridgeNode();
        int nignored = 0;

        for (EthernetAddress ea : ethers) {
            byte[] bytes = ea.getValue();
            byte[] src = new byte[] {bytes[0], bytes[1], bytes[2],
                                     bytes[3], bytes[4], bytes[5]};
            byte[] sender = new byte[] {(byte)192, (byte)168, (byte)0, (byte)iphost};
            byte[] src2 = new byte[] {(byte)0xf0, bytes[4], bytes[3],
                                      bytes[2], bytes[1], bytes[0]};
            byte[] sender2 = new byte[] {(byte)192, (byte)1, (byte)0, (byte)iphost};
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

            VBridgePath bp = new VBridgeIfPath(bpath, "if");
            bnode.setPath(bp);
            tbl.add(pctx, bnode);

            bp = new VlanMapPath(bpath2, "ANY.0");
            bnode.setPath(bp);
            tbl2.add(pctx2, bnode);

            String emsg = ea.toString();
            MacAddressEntry entry = null;
            try {
                entry = mgr.getMacEntry(bpath, ea);
            } catch (VTNException e) {
                unexpected(e);
            }
            assertEquals(emsg, ea, entry.getAddress());
            assertEquals(emsg, ((vlan < 0) ? (short)0 : vlan), entry.getVlan());
            assertEquals(emsg, nc, entry.getNodeConnector());

            Set<InetAddress> ips = entry.getInetAddresses();
            assertArrayEquals(emsg, sender, ips.iterator().next().getAddress());

            try {
                entry = mgr.getMacEntry(bpath, ea2);
            } catch (VTNException e) {
                unexpected(e);
            }
            assertNull(emsg, entry);

            try {
                entry = mgr.getMacEntry(bpath2, ea2);
            } catch (VTNException e) {
                unexpected(e);
            }

            // The VTN Manager never learns zero MAC address.
            if (NetUtils.byteArray6ToLong(src2) == 0L) {
                nignored++;
                assertNull(entry);
            } else {
                assertEquals(emsg, ea2, entry.getAddress());
                assertEquals(emsg, ((vlan2 < 0) ? (short)0 : vlan2),
                             entry.getVlan());
                assertEquals(emsg, nc, entry.getNodeConnector());

                ips = entry.getInetAddresses();
                assertArrayEquals(emsg, sender2,
                                  ips.iterator().next().getAddress());

                try {
                    mgr.removeMacEntry(bpath, ea);
                } catch (VTNException e) {
                    unexpected(e);
                }
            }

            iphost++;
        }

        // test flush.
        // at first add entry before test.
        dst = new byte[] {(byte)0xFF, (byte)0xFF, (byte)0xFF,
                          (byte)0xFF, (byte)0xFF, (byte)0xFF};
        target = new byte[] {(byte)192, (byte)168, (byte)0, (byte)250};
        for (EthernetAddress ea : ethers) {
            byte[] bytes = ea.getValue();
            byte[] src = new byte[] {bytes[0], bytes[1], bytes[2],
                                     bytes[3], bytes[4], bytes[5]};
            byte[] sender = new byte[] {(byte)192, (byte)168, (byte)0, (byte)iphost};

            PacketContext pctx = createARPPacketContext(src, dst, sender, target,
                    (short)-1, connectors.get(0), ARP.REQUEST);

            VBridgePath bp = new VBridgeIfPath(bpath, "if");
            bnode.setPath(bp);
            tbl.add(pctx, bnode);
        }

        // check entry
        List<MacAddressEntry> list = null;
        try {
            list = mgr.getMacEntries(bpath);
        } catch (VTNException e) {
            unexpected(e);
        }
        assertNotNull(list);
        assertEquals(ethers.size(), list.size());

        // test flush
        Status st = mgr.flushMacEntries(bpath);
        assertEquals(StatusCode.SUCCESS, st.getCode());

        try {
            list = mgr.getMacEntries(bpath);
        } catch (VTNException e) {
            unexpected(e);
        }
        assertNotNull(list);
        assertEquals(0, list.size());

        try {
            list = mgr.getMacEntries(bpath2);
        } catch (VTNException e) {
            unexpected(e);
        }
        assertNotNull(list);
        assertEquals(ethers.size() - nignored, list.size());

        st = mgr.flushMacEntries(bpath2);
        assertEquals(StatusCode.SUCCESS, st.getCode());

        try {
            list = mgr.getMacEntries(bpath2);
        } catch (VTNException e) {
            unexpected(e);
        }
        assertNotNull(list);
        assertEquals(0, list.size());

        st = mgr.removeTenant(tpath);
        assertEquals(StatusCode.SUCCESS, st.getCode());
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

        byte[] src = new byte[] {(byte)0x00, (byte)0x00, (byte)0x00,
                                 (byte)0x00, (byte)0x00, (byte)0x01};
        byte[] dst = new byte[] {(byte)0xFF, (byte)0xFF, (byte)0xFF,
                                 (byte)0xFF, (byte)0xFF, (byte)0xFF};
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

        VlanMapPath vpath = new VlanMapPath(bpath, "id");
        TestBridgeNode bnode = new TestBridgeNode(vpath);
        tbl.add(pctx, bnode);

        VBridgePath[] badplist = new VBridgePath[] {null,
                new VBridgePath(tname, null),
                new VBridgePath((String)null, bname)
        };
        EthernetAddress[] badelist = new EthernetAddress[] {null};

        // bad request
        for (VBridgePath path : badplist) {
            String emsg = (path == null) ? "null" : path.toString();
            try {
                mgr.getMacEntry(path, ea);
                fail("Throwing exception was expected.");
            } catch (VTNException e) {
                assertEquals(emsg,
                        StatusCode.BADREQUEST, e.getStatus().getCode());
            }

            try {
                mgr.getMacEntries(path);
                fail("Throwing exception was expected.");
            } catch (VTNException e) {
                assertEquals(emsg,
                        StatusCode.BADREQUEST, e.getStatus().getCode());
            }

            try {
                mgr.removeMacEntry(path, ea);
                fail("Throwing exception was expected.");
            } catch (VTNException e) {
                assertEquals(emsg,
                        StatusCode.BADREQUEST, e.getStatus().getCode());
            }

            st = mgr.flushMacEntries(path);
            assertEquals(emsg, StatusCode.BADREQUEST, st.getCode());
        }

        for (EthernetAddress eth : badelist) {
            try {
                mgr.getMacEntry(bpath, eth);
                fail("Throwing exception was expected.");
            } catch (VTNException e) {
                assertEquals((eth != null) ? eth.toString() : "null",
                        StatusCode.BADREQUEST, e.getStatus().getCode());
            }
        }

        // list for not found case
        VBridgePath[] nfplist = new VBridgePath[] {
                new VBridgePath("vvvvvv", bname),
                new VBridgePath(tname, "bbbbbb")
        };
        EthernetAddress[] nfelist = new EthernetAddress[] {null};

        // bad request
        for (VBridgePath path : nfplist) {
            String emsg = (path == null) ? "null" : path.toString();
            try {
                mgr.getMacEntry(path, ea);
                fail("Throwing exception was expected.");
            } catch (VTNException e) {
                assertEquals(emsg,
                        StatusCode.NOTFOUND, e.getStatus().getCode());
            }

            try {
                mgr.getMacEntries(path);
                fail("Throwing exception was expected.");
            } catch (VTNException e) {
                assertEquals(emsg,
                        StatusCode.NOTFOUND, e.getStatus().getCode());
            }

            try {
                mgr.removeMacEntry(path, ea);
                fail("Throwing exception was expected.");
            } catch (VTNException e) {
                assertEquals(emsg,
                        StatusCode.NOTFOUND, e.getStatus().getCode());
            }

            st = mgr.flushMacEntries(path);
            assertEquals(emsg, StatusCode.NOTFOUND, st.getCode());
        }

        st = mgr.removeTenant(tpath);
        assertEquals(StatusCode.SUCCESS, st.getCode());
    }

    /**
     * Test method for {@link VTNManagerImpl#readObject(ObjectInputStream)}.
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
     * Test method for {@link VTNManagerImpl#entryUpdated(ClusterEventId, Object, String, boolean)}.
     *
     * This tests {@link VTenantEvent}.
     */
    @Test
    public void testCacheEntryChangeTenantEvent() {
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

    /**
     * Check if a specified file exists..
     *
     * @param file      A checked file.
     * @param result    A expected result.
     * @param remove    If {@code true} specified file is removed after check.
     *                  If {@code false} specified file is created after check.
     */
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
    }

    /**
     * Test method for {@link VTNManagerImpl#entryUpdated}.
     * This tests {@link RawPacketEvent}.
     */
    @Test
    public void testCacheEntryChangeRawPacketEvent() {
        VTNManagerImpl mgr = vtnMgr;

        byte[] src = new byte[] {0x00, 0x00, 0x00, 0x00, 0x00, 0x01};
        byte[] dst = new byte[] {(byte) 0xff, (byte) 0xff, (byte) 0xff,
                                 (byte) 0xff, (byte) 0xff, (byte) 0xff};
        byte[] sender = new byte[] {(byte) 192, (byte) 168, (byte) 0, (byte) 1};
        byte[] target = new byte[] {(byte) 192, (byte) 168, (byte) 0, (byte) 10};
        Node node = NodeCreator.createOFNode(Long.valueOf(0L));
        NodeConnector innc = NodeConnectorCreator.createOFNodeConnector(Short.valueOf((short) 10), node);
        NodeConnector outnc = NodeConnectorCreator.createOFNodeConnector(Short.valueOf((short) 11), node);
        RawPacket pkt = createARPRawPacket(src, dst, sender, target, (short) 0, innc, ARP.REQUEST);

        RawPacketEvent ev = new RawPacketEvent(pkt, outnc);

        InetAddress ipaddr = getInetAddressFromAddress(new byte[] {0, 0, 0, 0});
        ClusterEventId evidRemote = new ClusterEventId(ipaddr, 0);
        ClusterEventId evidLocal = new ClusterEventId();

        Set<ClusterEventId> evIdSet = new HashSet<ClusterEventId>();
        evIdSet.add(evidLocal);
        evIdSet.add(evidRemote);

        for (ClusterEventId evid : evIdSet) {
            String emsg = evid.toString();
            // in case entry created, no operation is executed.
            mgr.entryCreated(evid, VTNManagerImpl.CACHE_EVENT, true);
            flushTasks();
            assertEquals(emsg, 0, stubObj.getTransmittedDataPacket().size());

            mgr.entryCreated(evid, VTNManagerImpl.CACHE_EVENT, false);
            flushTasks();
            assertEquals(emsg, 0, stubObj.getTransmittedDataPacket().size());

            // update event.
            mgr.entryUpdated(evid, ev, VTNManagerImpl.CACHE_EVENT, true);
            flushTasks();
            assertEquals(emsg, 0, stubObj.getTransmittedDataPacket().size());

            mgr.entryUpdated(evid, ev, VTNManagerImpl.CACHE_EVENT, false);
            flushTasks();
            if (evid == evidRemote) {
                assertEquals(emsg, 1,
                             stubObj.getTransmittedDataPacket().size());
            } else {
                assertEquals(emsg, 0,
                             stubObj.getTransmittedDataPacket().size());
            }

            // in case entry deleted, no operation is executed.
            mgr.entryDeleted(evid, VTNManagerImpl.CACHE_EVENT, true);
            flushTasks();
            assertEquals(emsg, 0,
                         stubObj.getTransmittedDataPacket().size());

            mgr.entryDeleted(evid, VTNManagerImpl.CACHE_EVENT, false);
            flushTasks();
            assertEquals(emsg, 0,
                         stubObj.getTransmittedDataPacket().size());
        }
    }

    /**
     * Test method for {@link VTNManagerImpl#entryUpdated}.
     * This tests {@link FlowAddEvent} and {@link FlowRemoveEvent}.
     */
    @Test
    public void testCacheEntryChangeFlowEvent() {
        long remoteTimeout = vtnMgr.getVTNConfig().getRemoteFlowModTimeout();
        VTNManagerImpl mgr = vtnMgr;

        // create tenant.
        VTenantPath path = new VTenantPath("tenant");
        Status st = vtnMgr.addTenant(path, new VTenantConfig(""));
        assertEquals(StatusCode.SUCCESS, st.getCode());

        VTNFlowDatabase fdb = vtnMgr.getTenantFlowDB(path.getTenantName());
        VTNFlow flow = fdb.create(vtnMgr);

        // ingress
        Node node0 = NodeCreator.createOFNode(Long.valueOf(0L));
        NodeConnector innc
            = NodeConnectorCreator.createOFNodeConnector(Short.valueOf("10"),
                                                         node0);
        NodeConnector outnc
            = NodeConnectorCreator.createOFNodeConnector(Short.valueOf("11"),
                                                         node0);
        Match match = new Match();
        match.setField(MatchType.IN_PORT, innc);
        match.setField(MatchType.DL_VLAN, (short) 1);
        ActionList actions = new ActionList(outnc.getNode());
        actions.addOutput(outnc);
        int pri = 1;
        flow.addFlow(vtnMgr, match, actions, pri);

        // + local entry.
        innc = NodeConnectorCreator.createOFNodeConnector(Short.valueOf("12"),
                                                          node0);
        outnc = NodeConnectorCreator.createOFNodeConnector(Short.valueOf("11"),
                                                           node0);
        flow = addFlowEntry(vtnMgr, flow, innc, (short) 1, outnc, pri);

        FlowAddEvent addEvent = new FlowAddEvent(flow.getFlowEntries());
        FlowRemoveEvent removeEvent = new FlowRemoveEvent(flow.getFlowEntries());

        Set<ClusterEventId> evIdSet = new HashSet<ClusterEventId>();
        InetAddress ipaddr = getInetAddressFromAddress(new byte[] {0, 0, 0, 0});
        ClusterEventId evIdRemote = new ClusterEventId(ipaddr, 0);
        evIdSet.add(evIdRemote);

        ClusterEventId evIdLocal = new ClusterEventId();
        evIdSet.add(evIdLocal);

        for (ClusterEventId evid : evIdSet) {
            String emsg = evid.toString();

            // in case entry created, no operation is executed.
            mgr.entryCreated(evid, VTNManagerImpl.CACHE_EVENT, true);
            flushAsyncTask(remoteTimeout);
            assertEquals(emsg, 0, stubObj.getFlowEntries().size());

            mgr.entryCreated(evid, VTNManagerImpl.CACHE_EVENT, false);
            flushAsyncTask(remoteTimeout);
            assertEquals(emsg, 0, stubObj.getFlowEntries().size());

            // apply add event
            mgr.entryUpdated(evid, addEvent, VTNManagerImpl.CACHE_EVENT, true);
            flushAsyncTask(remoteTimeout);
            assertEquals(emsg, 0, stubObj.getFlowEntries().size());

            mgr.entryUpdated(evid, addEvent, VTNManagerImpl.CACHE_EVENT, false);
            flushAsyncTask(remoteTimeout);
            assertEquals(emsg, (evid == evIdRemote) ? 2 : 0,
                         stubObj.getFlowEntries().size());

            // in case entry deleted, no operation is executed.
            mgr.entryDeleted(evid, VTNManagerImpl.CACHE_EVENT, true);
            flushAsyncTask(remoteTimeout);
            assertEquals(emsg, (evid == evIdRemote) ? 2 : 0,
                         stubObj.getFlowEntries().size());

            mgr.entryDeleted(evid, VTNManagerImpl.CACHE_EVENT, false);
            flushAsyncTask(remoteTimeout);
            assertEquals(emsg, (evid == evIdRemote) ? 2 : 0,
                         stubObj.getFlowEntries().size());

            // apply remove event
            mgr.entryUpdated(evid, removeEvent, VTNManagerImpl.CACHE_EVENT, true);
            flushAsyncTask(remoteTimeout);
            assertEquals(emsg, (evid == evIdRemote) ? 2 : 0,
                         stubObj.getFlowEntries().size());

            mgr.entryUpdated(evid, removeEvent, VTNManagerImpl.CACHE_EVENT, false);
            flushAsyncTask(remoteTimeout);
            assertEquals(emsg, 0, stubObj.getFlowEntries().size());
        }

        // specify a name except for CACHE_EVENT as cacheName.
        mgr.entryUpdated(evIdRemote, addEvent, VTNManagerImpl.CACHE_FLOWS, false);
        flushAsyncTask(remoteTimeout);
        assertEquals(0, stubObj.getFlowEntries().size());

        mgr.entryUpdated(evIdRemote, addEvent, VTNManagerImpl.CACHE_MAC, false);
        flushAsyncTask(remoteTimeout);
        assertEquals(0, stubObj.getFlowEntries().size());

        mgr.entryUpdated(evIdRemote, addEvent, VTNManagerImpl.CACHE_TENANT, false);
        flushAsyncTask(remoteTimeout);
        assertEquals(0, stubObj.getFlowEntries().size());
    }

    /**
     * Test case for {@link VTNManagerImpl#entryUpdated}.
     * This tests {@link FlowModResultEvent}.
     */
    @Test
    public void testCacheFlowModResult() {

        /**
         * TimerTask used to invoke {@code entryUpdated()}.
         */
        class ResultTimerTask extends TimerTask {
            private VTNManagerImpl vtnManager = null;
            private ClusterEventId evid = null;
            private String flowName;
            private FlowModResult result = null;
            private boolean isLocal = false;

            public ResultTimerTask(VTNManagerImpl mgr, ClusterEventId evid,
                    String flowName, FlowModResult res, boolean local) {
                vtnManager = mgr;
                this.evid = evid;
                this.flowName = flowName;
                result = res;
                isLocal = local;
            }

            @Override
            public void run() {
                FlowModResultEvent re
                    = new FlowModResultEvent(flowName, result);
                vtnManager.entryUpdated(evid, re, VTNManagerImpl.CACHE_EVENT, isLocal);
            }
        }

        setupVTNManagerForRemoteTaskTest(1000L, 1000L);

        long timeout = vtnMgr.getVTNConfig().getFlowModTimeout();
        long remoteTimeout = vtnMgr.getVTNConfig().getRemoteFlowModTimeout();

        // set IClusterGlobalService to stub which work
        // as have multiple cluster nodes.
        TestStub stubNew = new TestStub(2);
        TestStubCluster cm = new TestStubCluster(2);

        ComponentImpl c = new ComponentImpl(null, null, null);
        Hashtable<String, String> properties = new Hashtable<String, String>();
        properties.put("containerName", "default");
        c.setServiceProperties(properties);
        stopVTNManager(true);

        resMgr.setClusterGlobalService(cm);
        resMgr.init(c);
        vtnMgr.setResourceManager((IVTNResourceManager) resMgr);
        vtnMgr.setConnectionManager(cm);
        startVTNManager(c);

        // create tenant.
        VTenantPath path = new VTenantPath("tenant");
        Status st = vtnMgr.addTenant(path, new VTenantConfig(""));
        assertEquals(StatusCode.SUCCESS, st.getCode());

        VTNFlowDatabase fdb = vtnMgr.getTenantFlowDB(path.getTenantName());
        VTNFlow flow = fdb.create(vtnMgr);

        Node node0 = NodeCreator.createOFNode(Long.valueOf(0L));
        Node node1 = NodeCreator.createOFNode(Long.valueOf(1L));
        ConcurrentMap<Node, VNodeState> nodeDB = vtnMgr.getNodeDB();
        nodeDB.put(node0, VNodeState.UP);
        nodeDB.put(node1, VNodeState.UP);

        // ingress
        NodeConnector innc
            = NodeConnectorCreator.createOFNodeConnector(Short.valueOf("10"),
                                                         node0);
        NodeConnector outnc
            = NodeConnectorCreator.createOFNodeConnector(Short.valueOf("11"),
                                                         node0);
        Match match = new Match();
        byte[] src = {
            (byte)0x00, (byte)0x11, (byte)0x22,
            (byte)0x33, (byte)0x44, (byte)0x55,
        };
        byte[] dst = {
            (byte)0xf0, (byte)0xfa, (byte)0xfb,
            (byte)0xfc, (byte)0xfd, (byte)0xfe,
        };
        match.setField(MatchType.IN_PORT, innc);
        match.setField(MatchType.DL_VLAN, (short) 1);
        match.setField(MatchType.DL_SRC, src);
        match.setField(MatchType.DL_DST, dst);
        ActionList actions = new ActionList(outnc.getNode());
        actions.addOutput(outnc);
        int pri = 1;
        flow.addFlow(vtnMgr, match, actions, pri);

        // + local entry.
        innc = NodeConnectorCreator.createOFNodeConnector(Short.valueOf("12"),
                                                          node0);
        outnc = NodeConnectorCreator.createOFNodeConnector(Short.valueOf("11"),
                                                           node0);
        flow = addFlowEntry(vtnMgr, flow, innc, (short) 1, outnc, pri);

        // + remote entry.
        innc = NodeConnectorCreator.createOFNodeConnector(Short.valueOf("10"),
                                                          node1);
        outnc = NodeConnectorCreator.createOFNodeConnector(Short.valueOf("11"),
                                                           node1);
        flow = addFlowEntry(vtnMgr, flow, innc, (short) 1, outnc, pri);

        FlowEntry rent = null;
        Iterator<FlowEntry> it = flow.getFlowEntries().iterator();
        while (it.hasNext()) {
            rent = it.next();
            if (rent.getNode().equals(node1)) {
                break;
            }
            rent = null;
        }

        InetAddress ipaddr
            = getInetAddressFromAddress(new byte[] {0, 0, 0, 0});
        ClusterEventId evidRemote = new ClusterEventId(ipaddr, 0);
        ClusterEventId evidLocal = new ClusterEventId();

        for (FlowModResult result : FlowModResult.values()) {
            for (Boolean local : createBooleans(false)) {
                String emsg = "(FlowModResult)" + result.toString()
                        + ",(local)" + local.toString();

                // FlowModResultEvent is called from this timerTask.
                TimerTask timerTask = new ResultTimerTask(vtnMgr, evidRemote,
                                                          rent.getFlowName(),
                                                          result,
                                                          local.booleanValue());
                Timer timer = new Timer();

                timer.schedule(timerTask, 100L);
                fdb.install(vtnMgr, flow);
                flushFlowTasks(remoteTimeout * 3);
                timerTask.cancel();

                if (result == FlowModResult.SUCCEEDED && local == Boolean.FALSE) {
                    checkRegisteredFlowEntry(vtnMgr, 1, flow, flow, 2, emsg);

                    timerTask = new ResultTimerTask(vtnMgr, evidRemote,
                                                    rent.getFlowName(),
                                                    FlowModResult.SUCCEEDED,
                                                    false);
                    timer.schedule(timerTask, 100L);
                    vtnMgr.removeAllFlows(path);
                    timer.cancel();
                } else {
                    checkRegisteredFlowEntry(vtnMgr, 0, flow, null, 0, emsg);
                    vtnMgr.removeAllFlows(path);
                    flushFlowTasks(timeout);
                }

                fdb.clear(vtnMgr);
                flushFlowTasks(remoteTimeout * 3);
                checkRegisteredFlowEntry(vtnMgr, 0, flow, null, 0, emsg);
            }
        }

        // in case local event.
        TimerTask timerTask = new ResultTimerTask(vtnMgr, evidLocal,
                                                  rent.getFlowName(),
                                                  FlowModResult.SUCCEEDED,
                                                  false);
        Timer timer = new Timer();

        timer.schedule(timerTask, 100L);
        fdb.install(vtnMgr, flow);
        flushFlowTasks(remoteTimeout * 3);
        timerTask.cancel();
        checkRegisteredFlowEntry(vtnMgr, 0, flow, null, 0, "");

        fdb.clear(vtnMgr);
        flushFlowTasks(timeout);

        cleanupSetupFile();
    }

    /**
     * Test method for {@link VTNManagerImpl#entryUpdated}.
     * This tests Flow cache.
     */
    @Test
    public void testCacheEntryChangeFlowCache() {
        VTNManagerImpl mgr = vtnMgr;

        // create tenant.
        VTenantPath path = new VTenantPath("tenant");
        Status st = vtnMgr.addTenant(path, new VTenantConfig(""));
        assertEquals(StatusCode.SUCCESS, st.getCode());

        Set<VTenantPath> pathSet = new HashSet<VTenantPath>();
        pathSet.add(path);

        VTNFlowDatabase fdb = vtnMgr.getTenantFlowDB(path.getTenantName());

        // create FlowGroupId.
        InetAddress ipaddr = getInetAddressFromAddress(new byte[] {0, 0, 0, 0});
        FlowGroupId evIdRemote = new FlowGroupId(ipaddr, 0L, "tenant");
        FlowGroupId evIdLocal = new FlowGroupId(InetAddress.getLoopbackAddress(),
                                                0L, "tenant");
        Set<FlowGroupId> evIdSet = new HashSet<FlowGroupId>();
        evIdSet.add(evIdRemote);
        evIdSet.add(evIdLocal);

        Node node0 = NodeCreator.createOFNode(Long.valueOf(0L));
        byte[] src = {
            (byte)0x00, (byte)0x11, (byte)0x22,
            (byte)0x33, (byte)0x44, (byte)0x55,
        };
        byte[] dst = {
            (byte)0xf0, (byte)0xfa, (byte)0xfb,
            (byte)0xfc, (byte)0xfd, (byte)0xfe,
        };
        for (FlowGroupId evid : evIdSet) {
            String emsg = evid.toString();
            VTNFlow flow = new VTNFlow(evid);

            // ingress
            NodeConnector innc
                = NodeConnectorCreator.createOFNodeConnector(Short.valueOf("10"),
                                                             node0);
            NodeConnector outnc
                = NodeConnectorCreator.createOFNodeConnector(Short.valueOf("11"),
                                                             node0);
            Match match = new Match();
            match.setField(MatchType.IN_PORT, innc);
            match.setField(MatchType.DL_VLAN, (short) 1);
            match.setField(MatchType.DL_SRC, src);
            match.setField(MatchType.DL_DST, dst);
            ActionList actions = new ActionList(outnc.getNode());
            actions.addOutput(outnc);
            int pri = 1;
            flow.addFlow(vtnMgr, match, actions, pri);

            // + local entry.
            innc = NodeConnectorCreator.createOFNodeConnector(Short.valueOf("12"),
                                                              node0);
            outnc = NodeConnectorCreator.createOFNodeConnector(Short.valueOf("11"),
                                                               node0);
            flow = addFlowEntry(vtnMgr, flow, innc, (short) 1, outnc, pri);

            flow.addDependency(pathSet);

            FlowEntry ingress = flow.getFlowEntries().get(0);

            // entry created.
            mgr.entryCreated(evid, VTNManagerImpl.CACHE_FLOWS, true);
            assertFalse(emsg, fdb.containsIngressFlow(ingress));

            mgr.entryCreated(evid, VTNManagerImpl.CACHE_FLOWS, false);
            assertFalse(emsg, fdb.containsIngressFlow(ingress));

            // entry updated
            mgr.entryUpdated(evid, flow, VTNManagerImpl.CACHE_FLOWS, true);
            assertFalse(emsg, fdb.containsIngressFlow(ingress));

            mgr.entryUpdated(evid, flow, VTNManagerImpl.CACHE_FLOWS, false);
            if (evid == evIdRemote) {
                assertTrue(emsg, fdb.containsIngressFlow(ingress));
            } else {
                assertFalse(emsg, fdb.containsIngressFlow(ingress));
            }

            // entry deleted
            mgr.entryDeleted(evid, VTNManagerImpl.CACHE_FLOWS, true);
            if (evid == evIdRemote) {
                assertTrue(emsg, fdb.containsIngressFlow(ingress));
            } else {
                assertFalse(emsg, fdb.containsIngressFlow(ingress));
            }

            mgr.entryDeleted(evid, VTNManagerImpl.CACHE_FLOWS, false);
            assertFalse(emsg, fdb.containsIngressFlow(ingress));
        }

        // specify a name except for CACHE_FLOWS as cacheName.
        VTNFlow flow = new VTNFlow(evIdRemote);
        NodeConnector innc
            = NodeConnectorCreator.createOFNodeConnector(Short.valueOf("10"),
                                                         node0);
        NodeConnector outnc
            = NodeConnectorCreator.createOFNodeConnector(Short.valueOf("11"),
                                                         node0);
        Match match = new Match();
        match.setField(MatchType.IN_PORT, innc);
        match.setField(MatchType.DL_VLAN, (short) 1);
        match.setField(MatchType.DL_SRC, src);
        match.setField(MatchType.DL_DST, dst);
        ActionList actions = new ActionList(outnc.getNode());
        actions.addOutput(outnc);
        int pri = 1;
        flow.addFlow(vtnMgr, match, actions, pri);
        FlowEntry ingress = flow.getFlowEntries().get(0);

        mgr.entryUpdated(evIdRemote, flow, VTNManagerImpl.CACHE_EVENT, false);
        assertFalse(fdb.containsIngressFlow(ingress));

        mgr.entryUpdated(evIdRemote, flow, VTNManagerImpl.CACHE_MAC, false);
        assertFalse(fdb.containsIngressFlow(ingress));

        mgr.entryUpdated(evIdRemote, flow, VTNManagerImpl.CACHE_PORTS, false);
        assertFalse(fdb.containsIngressFlow(ingress));

        // specify invalid id.
        mgr.entryDeleted(null, VTNManagerImpl.CACHE_FLOWS, false);
        assertFalse(fdb.containsIngressFlow(ingress));

        FlowGroupId gid = new FlowGroupId("badtenant");
        mgr.entryUpdated(gid, flow, VTNManagerImpl.CACHE_FLOWS, false);
        assertFalse(fdb.containsIngressFlow(ingress));

        mgr.entryDeleted(gid, VTNManagerImpl.CACHE_FLOWS, false);
        assertFalse(fdb.containsIngressFlow(ingress));

        // specify a object except for VTNFlow.
        mgr.entryUpdated(evIdRemote, flow, VTNManagerImpl.CACHE_FLOWS, false);
        assertTrue(fdb.containsIngressFlow(ingress));

        mgr.entryUpdated(evIdRemote, Integer.valueOf(0),
                VTNManagerImpl.CACHE_FLOWS, false);

        assertTrue(fdb.containsIngressFlow(ingress));
    }

    /**
     * Test method for {@link VTNManagerImpl#entryUpdated}.
     * This tests MacAddressTableEvent.
     */
    @Test
    public void testCacheEntryChangeMacTableEntry() {
        VTNManagerImpl mgr = vtnMgr;
        VTenantPath tpath = new VTenantPath("vtn");
        VBridgePath bpath = new VBridgePath(tpath.getTenantName(), "vbr");
        List<VBridgePath> bpathlist = new ArrayList<VBridgePath>();
        bpathlist.add(bpath);
        createTenantAndBridge(mgr, tpath, bpathlist);

        long mac = 1L;
        EthernetAddress ea = null;
        try {
            ea = new EthernetAddress(NetUtils.longToByteArray6(mac));
        } catch (ConstructionException e) {
            unexpected(e);
        }
        Node node = NodeCreator.createOFNode(Long.valueOf(0L));
        NodeConnector nc = NodeConnectorCreator
                .createOFNodeConnector(Short.valueOf("1"), node);
        InetAddress ipaddr = getInetAddressFromAddress(new byte[] {0, 0, 0, 0});

        MacTableEntryId evidRemote = new MacTableEntryId(ipaddr, 0L, bpath, mac);
        MacTableEntryId evidLocal = new MacTableEntryId(bpath, mac);
        Set<MacTableEntryId> evidSet = new HashSet<MacTableEntryId>();
        evidSet.add(evidRemote);
        evidSet.add(evidLocal);

        for (MacTableEntryId evid : evidSet) {
            String emsg = evid.toString();
            MacTableEntry ent = new MacTableEntry(evid, nc, (short) 0, ipaddr);

            // entryCreated
            mgr.entryCreated(evid, VTNManagerImpl.CACHE_MAC, true);
            MacAddressEntry entry = getMacAddressEntry(mgr, bpath, ea);
            assertNull(emsg, entry);

            mgr.entryCreated(evid, VTNManagerImpl.CACHE_MAC, false);
            entry = getMacAddressEntry(mgr, bpath, ea);
            assertNull(emsg, entry);

            // entryUpdates
            mgr.entryUpdated(evid, ent, VTNManagerImpl.CACHE_MAC, true);
            entry = getMacAddressEntry(mgr, bpath, ea);
            assertNull(emsg, entry);

            mgr.entryUpdated(evid, ent, VTNManagerImpl.CACHE_MAC, false);
            entry = getMacAddressEntry(mgr, bpath, ea);
            if (evid == evidRemote) {
                assertNotNull(emsg, entry);
            } else {
                assertNull(emsg, entry);
            }

            // entryDeleted.
            mgr.entryDeleted(evid, VTNManagerImpl.CACHE_MAC, true);
            entry = getMacAddressEntry(mgr, bpath, ea);
            if (evid == evidRemote) {
                assertNotNull(emsg, entry);
            } else {
                assertNull(emsg, entry);
            }

            mgr.entryDeleted(evid, VTNManagerImpl.CACHE_MAC, false);
            entry = getMacAddressEntry(mgr, bpath, ea);
            assertNull(emsg, entry);
        }

        // specify a name except for CACHE_MAC as cacheName.
        MacTableEntry ent = new MacTableEntry(evidRemote, nc, (short) 0,
                                              ipaddr);
        mgr.entryUpdated(evidRemote, ent, VTNManagerImpl.CACHE_EVENT, false);
        MacAddressEntry entry = getMacAddressEntry(mgr, bpath, ea);
        assertNull(entry);

        mgr.entryUpdated(evidRemote, ent, VTNManagerImpl.CACHE_FLOWS, false);
        entry = getMacAddressEntry(mgr, bpath, ea);
        assertNull(entry);

        mgr.entryUpdated(evidRemote, ent, VTNManagerImpl.CACHE_NODES, false);
        entry = getMacAddressEntry(mgr, bpath, ea);
        assertNull(entry);

        // specify invalid id
        List<MacAddressEntry> entries = getMacAddressEntries(evidRemote);
        int presize = entries.size();
        mgr.entryDeleted(null, VTNManagerImpl.CACHE_MAC, false);
        entry = getMacAddressEntry(mgr, bpath, ea);
        assertNull(entry);
        assertEquals(presize, getMacAddressEntries(evidRemote).size());

        VBridgePath bpathNew = new VBridgePath("tenant", "bbb");
        MacTableEntryId eid = new MacTableEntryId(ipaddr, 0L, bpathNew, mac);
        mgr.entryUpdated(eid, ent, VTNManagerImpl.CACHE_MAC, false);
        assertNull(mgr.getMacAddressTable(eid));


        mgr.entryUpdated(evidRemote, ent, VTNManagerImpl.CACHE_MAC, false);
        entry = getMacAddressEntry(mgr, bpath, ea);
        assertNotNull(entry);

        mgr.entryDeleted(eid, VTNManagerImpl.CACHE_MAC, false);
        assertNull(mgr.getMacAddressTable(eid));

        mgr.entryDeleted(evidRemote, VTNManagerImpl.CACHE_MAC, false);
        assertNull(mgr.getMacAddressTable(eid));


        // specify invalid object
        presize = entries.size();
        mgr.entryUpdated(evidRemote, Integer.valueOf(0),
                         VTNManagerImpl.CACHE_MAC, false);
        entry = getMacAddressEntry(mgr, bpath, ea);
        assertNull(entry);
        assertEquals(presize, getMacAddressEntries(evidRemote).size());
    }

    /**
     * get Mac Address table Entry.
     *
     * @param mgr       A VTNManager service.
     * @param bpath     A {@link VBridgePath}.
     * @param ea        A {@link EthernetAddress}.
     * @return  A {@link MacAddressEntry}.
     */
    private MacAddressEntry getMacAddressEntry(VTNManagerImpl mgr, VBridgePath bpath,
                                               EthernetAddress ea) {
        MacAddressEntry entry = null;
        try {
            entry = mgr.getMacEntry(bpath, ea);
        } catch (VTNException e) {
            unexpected(e);
        }
        return entry;
    }

    /**
     * Test method for {@link VTNManagerImpl#flowRemoved(Node, Flow)}.
     */
    @Test
    public void testFlowRemoved() {
        VTNManagerImpl mgr = vtnMgr;

        // create tenant.
        VTenantPath path = new VTenantPath("tenant");
        Status st = mgr.addTenant(path, new VTenantConfig(""));
        assertEquals(StatusCode.SUCCESS, st.getCode());

        VTNFlowDatabase fdb = vtnMgr.getTenantFlowDB(path.getTenantName());

        Node node0 = NodeCreator.createOFNode(Long.valueOf(0L));
        Node node1 = NodeCreator.createOFNode(Long.valueOf(1L));
        Node node10 = NodeCreator.createOFNode(Long.valueOf(10L));
        Set<Node> nodeSet = new HashSet<Node>();
        nodeSet.add(node0);
        nodeSet.add(node1);

        Set<VTNFlow> flows = new HashSet<VTNFlow>();
        for (Node node : nodeSet) {
            VTNFlow flow = fdb.create(vtnMgr);

            // ingress
            NodeConnector innc
                = NodeConnectorCreator.createOFNodeConnector(Short.valueOf("10"),
                                                             node);
            NodeConnector outnc
                = NodeConnectorCreator.createOFNodeConnector(Short.valueOf("11"),
                                                             node);
            Match match = new Match();
            byte[] src = {
                (byte)0x00, (byte)0x11, (byte)0x22,
                (byte)0x33, (byte)0x44, (byte)0x55,
            };
            byte[] dst = {
                (byte)0xf0, (byte)0xfa, (byte)0xfb,
                (byte)0xfc, (byte)0xfd, (byte)0xfe,
            };
            match.setField(MatchType.IN_PORT, innc);
            match.setField(MatchType.DL_VLAN, (short) 1);
            match.setField(MatchType.DL_SRC, src);
            match.setField(MatchType.DL_DST, dst);
            ActionList actions = new ActionList(outnc.getNode());
            actions.addOutput(outnc);
            int pri = 1;
            flow.addFlow(vtnMgr, match, actions, pri);

            // + local entry.
            innc = NodeConnectorCreator.createOFNodeConnector(Short.valueOf("12"),
                                                              node);
            outnc = NodeConnectorCreator.createOFNodeConnector(Short.valueOf("11"),
                                                               node);
            flow = addFlowEntry(vtnMgr, flow, innc, (short) 1, outnc, pri);

            fdb.install(mgr, flow);

            flows.add(flow);
        }

        NodeConnector innc
            = NodeConnectorCreator.createOFNodeConnector(Short.valueOf("10"),
                                                         node10);
        NodeConnector outnc
            = NodeConnectorCreator.createOFNodeConnector(Short.valueOf("11"),
                                                         node10);
        Match match = new Match();
        match.setField(MatchType.IN_PORT, innc);
        match.setField(MatchType.DL_VLAN, (short) 1);
        ActionList acts = new ActionList(outnc.getNode());
        acts.addOutput(outnc);

        FlowEntry badEntry = new FlowEntry("", "", new Flow(match, acts.get()),
                                           node0);

        flushFlowTasks();
        assertEquals(4, stubObj.getFlowEntries().size());
        assertEquals(2, mgr.getFlowDB().size());

        for (VTNFlow flow : flows) {
            FlowEntry entry = flow.getFlowEntries().iterator().next();
            mgr.flowRemoved(entry.getNode(), entry.getFlow());
            flushFlowTasks();
            assertEquals(3, stubObj.getFlowEntries().size());
            assertEquals(1, mgr.getFlowDB().size());

            stubObj.uninstallFlowEntry(entry);
            fdb.removeIndex(mgr, flow);
            fdb.install(mgr, flow);
            flushFlowTasks();
            assertEquals(4, stubObj.getFlowEntries().size());
            assertEquals(2, mgr.getFlowDB().size());

            mgr.flowRemoved(node10, entry.getFlow());
            flushFlowTasks();
            assertEquals(4, stubObj.getFlowEntries().size());
            assertEquals(2, mgr.getFlowDB().size());

            mgr.flowRemoved(node10, badEntry.getFlow());
            flushFlowTasks();
            assertEquals(4, stubObj.getFlowEntries().size());
            assertEquals(2, mgr.getFlowDB().size());

        }
    }

    /**
     * test method for {@link VTNManagerImpl#removeAllFlows(VTenantPath).}
     */
    @Test
    public void testRemoveAllFlows() {
        VTNManagerImpl mgr = vtnMgr;

        // create tenant.
        VTenantPath path = new VTenantPath("tenant");
        Status st = mgr.addTenant(path, new VTenantConfig(""));
        assertEquals(StatusCode.SUCCESS, st.getCode());

        VTenantPath anotherPath = new VTenantPath("another");
        st = mgr.addTenant(anotherPath, new VTenantConfig(""));
        assertEquals(StatusCode.SUCCESS, st.getCode());

        VTenantPath notExistPath = new VTenantPath("not_exist");

        // add flow.
        VTNFlowDatabase fdb = vtnMgr.getTenantFlowDB(path.getTenantName());

        Node node0 = NodeCreator.createOFNode(Long.valueOf(0L));
        ConcurrentMap<Node, VNodeState> nodeDB = vtnMgr.getNodeDB();
        nodeDB.put(node0, VNodeState.UP);

        Set<VTNFlow> flows = new HashSet<VTNFlow>();
        VTNFlow flow = fdb.create(vtnMgr);

        // ingress
        NodeConnector innc
            = NodeConnectorCreator.createOFNodeConnector(Short.valueOf("10"),
                                                         node0);
        NodeConnector outnc
            = NodeConnectorCreator.createOFNodeConnector(Short.valueOf("11"),
                                                         node0);
        Match match = new Match();
        byte[] src = {
            (byte)0x00, (byte)0x11, (byte)0x22,
            (byte)0x33, (byte)0x44, (byte)0x55,
        };
        byte[] dst = {
            (byte)0xf0, (byte)0xfa, (byte)0xfb,
            (byte)0xfc, (byte)0xfd, (byte)0xfe,
        };
        match.setField(MatchType.IN_PORT, innc);
        match.setField(MatchType.DL_VLAN, (short) 1);
        match.setField(MatchType.DL_SRC, src);
        match.setField(MatchType.DL_DST, dst);
        ActionList actions = new ActionList(outnc.getNode());
        actions.addOutput(outnc);
        int pri = 1;
        flow.addFlow(vtnMgr, match, actions, pri);

        // + local entry.
        innc = NodeConnectorCreator.createOFNodeConnector(Short.valueOf("12"),
                                                          node0);
        outnc = NodeConnectorCreator.createOFNodeConnector(Short.valueOf("11"),
                                                           node0);
        flow = addFlowEntry(vtnMgr, flow, innc, (short) 1, outnc, pri);

        fdb.install(mgr, flow);
        flows.add(flow);
        flushFlowTasks();
        assertEquals(2, stubObj.getFlowEntries().size());
        assertEquals(1, mgr.getFlowDB().size());

        // test removeAllFlows
        mgr.removeAllFlows(anotherPath);
        assertEquals(2, stubObj.getFlowEntries().size());
        assertEquals(1, mgr.getFlowDB().size());

        mgr.removeAllFlows(notExistPath);
        assertEquals(2, stubObj.getFlowEntries().size());
        assertEquals(1, mgr.getFlowDB().size());

        mgr.removeAllFlows(path);
        assertEquals(0, stubObj.getFlowEntries().size());
        assertEquals(0, mgr.getFlowDB().size());
    }

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
        assertEquals(StatusCode.SUCCESS, st.getCode());
        assertTrue(mgr.isActive());

        st = mgr.saveConfiguration();
        assertEquals(StatusCode.SUCCESS, st.getCode());

        mgr.removeTenant(tpath);
        assertEquals(StatusCode.SUCCESS, st.getCode());

        st = mgr.saveConfiguration();
        assertEquals(StatusCode.SUCCESS, st.getCode());
    }

    /**
     * Test method for
     * {@link VTNManagerImpl#receiveDataPacket(RawPacket)}
     */
    @Test
    public void testReceiveDataPacket() {
        VTNManagerImpl mgr = vtnMgr;
        ISwitchManager swmgr = mgr.getSwitchManager();
        byte[] cntMac = swmgr.getControllerMAC();

        String tname = "vtn";
        VTenantPath tpath = new VTenantPath(tname);
        Status st = mgr.addTenant(tpath, new VTenantConfig(null));
        assertEquals(StatusCode.SUCCESS, st.getCode());
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
        for (NodeConnector nc : connectors) {
            byte iphost = 1;
            for (EthernetAddress ea : createEthernetAddresses(false)) {
                byte [] bytes = ea.getValue();
                byte [] src = new byte[] {bytes[0], bytes[1], bytes[2],
                                        bytes[3], bytes[4], bytes[5]};
                sender[3] = (byte)iphost;
                String emsg = ((nc != null) ? "(Incoming nc)" + nc.toString() : "")
                        + ea.toString();

                RawPacket inPkt = createARPRawPacket(src, dst, sender, target,
                                                     (short)-1, nc, ARP.REQUEST);
                result = mgr.receiveDataPacket(inPkt);

                // because there are no topology, in this case always ignored.
                assertEquals(emsg, PacketResult.IGNORED, result);

                iphost++;
            }

            // packet from controller.
            RawPacket inPkt = createARPRawPacket(srcCnt, dst, sender, target,
                                                 (short)-1, nc, ARP.REQUEST);
            result = mgr.receiveDataPacket(inPkt);

            assertEquals(PacketResult.IGNORED, result);
        }

        st = mgr.removeTenant(tpath);
        assertEquals(StatusCode.SUCCESS, st.getCode());
    }

    /**
     * Ensure that {@link VTNManagerImpl#getMacMap(VBridgePath)} fails.
     *
     * @param path  A {@link VBridgePath} instance to be passed.
     * @param code  A {@link StatusCode} instance expected to be thrown via
     *              an {@link VTNException}.
     */
    private void errorGetMacMap(VBridgePath path, StatusCode code) {
        try {
            vtnMgr.getMacMap(path);
            fail("An exception must be thrown.");
        } catch (VTNException e) {
            Status st = e.getStatus();
            assertEquals(code, st.getCode());
        }
    }

    /**
     * Ensure that
     * {@link VTNManagerImpl#getMacMapConfig(VBridgePath, MacMapAclType)}
     * fails.
     *
     * @param path  A {@link VBridgePath} instance to be passed.
     * @param acl   A {@link MacMapAclType} instance to be passed.
     * @param code  A {@link StatusCode} instance expected to be thrown via
     *              an {@link VTNException}.
     */
    private void errorGetMacMapConfig(VBridgePath path, MacMapAclType acl,
                                      StatusCode code) {
        try {
            vtnMgr.getMacMapConfig(path, acl);
            fail("An exception must be thrown.");
        } catch (VTNException e) {
            Status st = e.getStatus();
            assertEquals(code, st.getCode());
        }
    }

    /**
     * Ensure that
     * {@link VTNManagerImpl#getMacMappedHosts(VBridgePath)}
     * fails.
     *
     * @param path  A {@link VBridgePath} instance to be passed.
     * @param code  A {@link StatusCode} instance expected to be thrown via
     *              an {@link VTNException}.
     */
    private void errorGetMacMappedHosts(VBridgePath path, StatusCode code) {
        try {
            vtnMgr.getMacMappedHosts(path);
            fail("An exception must be thrown.");
        } catch (VTNException e) {
            Status st = e.getStatus();
            assertEquals(code, st.getCode());
        }
    }

    /**
     * Ensure that
     * {@link VTNManagerImpl#getMacMappedHost(VBridgePath, DataLinkAddress)}
     * fails.
     *
     * @param path  A {@link VBridgePath} instance to be passed.
     * @param addr  A {@link DataLinkAddress} instance to be passed.
     * @param code  A {@link StatusCode} instance expected to be thrown via
     *              an {@link VTNException}.
     */
    private void errorGetMacMappedHost(VBridgePath path, DataLinkAddress addr,
                                       StatusCode code) {
        try {
            vtnMgr.getMacMappedHost(path, addr);
            fail("An exception must be thrown.");
        } catch (VTNException e) {
            Status st = e.getStatus();
            assertEquals(code, st.getCode());
        }
    }

    /**
     * Ensure that
     * {@link VTNManagerImpl#setMacMap(VBridgePath, UpdateOperation, MacMapConfig)}
     * fails.
     *
     * @param path  A {@link VBridgePath} instance to be passed.
     * @param op    A {@link UpdateOperation} instance to be passed.
     * @param cf    A {@link MacMapConfig} instance to be passed.
     * @param code  A {@link StatusCode} instance expected to be thrown via
     *              an {@link VTNException}.
     */
    private void errorSetMacMap(VBridgePath path, UpdateOperation op,
                                MacMapConfig cf, StatusCode code) {
        try {
            vtnMgr.setMacMap(path, op, cf);
            fail("An exception must be thrown.");
        } catch (VTNException e) {
            Status st = e.getStatus();
            assertEquals(code, st.getCode());
        }
    }

    /**
     * Ensure that
     * {@link VTNManagerImpl#setMacMap(VBridgePath, UpdateOperation, MacMapAclType, Set)}
     * fails.
     *
     * @param path  A {@link VBridgePath} instance to be passed.
     * @param op    A {@link UpdateOperation} instance to be passed.
     * @param acl   A {@link MacMapAclType} instance to be passed.
     * @param set   A set of {@link DataLinkHost} instances to be passed.
     * @param code  A {@link StatusCode} instance expected to be thrown via
     *              an {@link VTNException}.
     */
    private void errorSetMacMap(VBridgePath path, UpdateOperation op,
                                MacMapAclType acl, Set<DataLinkHost> set,
                                StatusCode code) {
        try {
            vtnMgr.setMacMap(path, op, acl, set);
            fail("An exception must be thrown.");
        } catch (VTNException e) {
            Status st = e.getStatus();
            assertEquals(code, st.getCode());
        }
    }

    /**
     * Verify that the specified host can not be configured to the
     * MAC mapping.
     *
     * @param path    A path to the vBridge.
     * @param mcconf  A {@link MacMapConfig} instance which keeps current
     *                MAC mapping configuration.
     * @param op      A {@link UpdateOperation} to be performed.
     * @param acl     A {@link MacMapAclType} which specifies the MAC mapping
     *                ACL to be configured.
     * @param host    A {@link DataLinkHost} instance to be tested.
     * @param code    A {@link StatusCode} instance expected to be thrown
     *                by a {@link VTNException}.
     */
    private void checkMacMapError(VBridgePath path, MacMapConfig mcconf,
                                  UpdateOperation op, MacMapAclType acl,
                                  DataLinkHost host, StatusCode code) {
        MacMapConfig mc;
        Set<DataLinkHost> set;
        if (acl == MacMapAclType.ALLOW) {
            set = mcconf.getAllowedHosts();
            assertTrue(set.add(host));
            mc = new MacMapConfig(set, mcconf.getDeniedHosts());
        } else {
            set = mcconf.getDeniedHosts();
            assertTrue(set.add(host));
            mc = new MacMapConfig(mcconf.getAllowedHosts(), set);
        }

        errorSetMacMap(path, op, mc, code);
        errorSetMacMap(path, op, acl, set, code);

        set.clear();
        assertTrue(set.add(host));
        errorSetMacMap(path, op, acl, set, code);
    }
}
