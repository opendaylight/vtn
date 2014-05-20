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
import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Hashtable;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ConcurrentMap;

import org.apache.felix.dm.Component;
import org.apache.felix.dm.impl.ComponentImpl;

import org.junit.Before;
import org.junit.Test;
import org.junit.experimental.categories.Category;

import org.opendaylight.controller.forwardingrulesmanager.FlowEntry;
import org.opendaylight.controller.sal.connection.ConnectionLocality;
import org.opendaylight.controller.sal.core.Config;
import org.opendaylight.controller.sal.core.ConstructionException;
import org.opendaylight.controller.sal.core.Edge;
import org.opendaylight.controller.sal.core.Name;
import org.opendaylight.controller.sal.core.Node;
import org.opendaylight.controller.sal.core.NodeConnector;
import org.opendaylight.controller.sal.core.Property;
import org.opendaylight.controller.sal.core.State;
import org.opendaylight.controller.sal.core.UpdateType;
import org.opendaylight.controller.sal.match.Match;
import org.opendaylight.controller.sal.match.MatchType;
import org.opendaylight.controller.sal.packet.ARP;
import org.opendaylight.controller.sal.packet.Ethernet;
import org.opendaylight.controller.sal.packet.IEEE8021Q;
import org.opendaylight.controller.sal.packet.Packet;
import org.opendaylight.controller.sal.packet.PacketResult;
import org.opendaylight.controller.sal.packet.RawPacket;
import org.opendaylight.controller.sal.packet.address.EthernetAddress;
import org.opendaylight.controller.sal.topology.TopoEdgeUpdate;
import org.opendaylight.controller.sal.utils.EtherTypes;
import org.opendaylight.controller.sal.utils.GlobalConstants;
import org.opendaylight.controller.sal.utils.NodeConnectorCreator;
import org.opendaylight.controller.sal.utils.NodeCreator;
import org.opendaylight.controller.sal.utils.Status;
import org.opendaylight.controller.sal.utils.StatusCode;
import org.opendaylight.controller.switchmanager.ISwitchManager;
import org.opendaylight.controller.topologymanager.ITopologyManager;
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
import org.opendaylight.vtn.manager.internal.cluster.ClusterEvent;
import org.opendaylight.vtn.manager.internal.cluster.ClusterEventId;
import org.opendaylight.vtn.manager.internal.cluster.FlowGroupId;
import org.opendaylight.vtn.manager.internal.cluster.FlowModResult;
import org.opendaylight.vtn.manager.internal.cluster.FlowModResultEvent;
import org.opendaylight.vtn.manager.internal.cluster.MacTableEntry;
import org.opendaylight.vtn.manager.internal.cluster.MacTableEntryId;
import org.opendaylight.vtn.manager.internal.cluster.ObjectPair;
import org.opendaylight.vtn.manager.internal.cluster.PortMapEvent;
import org.opendaylight.vtn.manager.internal.cluster.PortVlan;
import org.opendaylight.vtn.manager.internal.cluster.RawPacketEvent;
import org.opendaylight.vtn.manager.internal.cluster.VBridgeEvent;
import org.opendaylight.vtn.manager.internal.cluster.VBridgeIfEvent;
import org.opendaylight.vtn.manager.internal.cluster.VNodeEvent;
import org.opendaylight.vtn.manager.internal.cluster.VTNFlow;
import org.opendaylight.vtn.manager.internal.cluster.VTenantEvent;
import org.opendaylight.vtn.manager.internal.cluster.VTenantImpl;
import org.opendaylight.vtn.manager.internal.cluster.VlanMapEvent;

/**
 * JUnit test for {@link VTNManagerImplTest}.
 *
 * <p>
 * This test class test with using stub which simulate working
 * on the environment some nodes exist in cluster mode.
 * </p>
 */
@Category(SlowTest.class)
public class VTNManagerImplClusterTest extends VTNManagerImplTestCommon {
    /**
     * Construct a new instance.
     */
    public VTNManagerImplClusterTest() {
        super(3);
    }

    @Before
    @Override
    public void before() {
        setupStartupDir();

        vtnMgr = new VTNManagerImpl();
        resMgr = new GlobalResourceManager();
        ComponentImpl c = new ComponentImpl(null, null, null);
        stubObj = new TestStubCluster(stubMode);

        Hashtable<String, String> properties = new Hashtable<String, String>();
        properties.put("containerName", "default");
        c.setServiceProperties(properties);

        resMgr.setClusterGlobalService(stubObj);
        resMgr.init(c);
        vtnMgr.setResourceManager(resMgr);
        vtnMgr.setClusterContainerService(stubObj);
        vtnMgr.setSwitchManager(stubObj);
        vtnMgr.setTopologyManager(stubObj);
        vtnMgr.setDataPacketService(stubObj);
        vtnMgr.setRouting(stubObj);
        vtnMgr.setHostTracker(stubObj);
        vtnMgr.setForwardingRuleManager(stubObj);
        vtnMgr.setConnectionManager(stubObj);
        startVTNManager(c);
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
                file.createNewFile();
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
        checkVTNconfig(vtnMgr, tpath, bpathlist, pmaps, null);

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
        checkVTNconfig(vtnMgr, tpath, bpathlist, pmaps, vmaps);

        // start after configuration files is removed.
        // because caches remain, setting is taken over.
        stopVTNManager(false);
        VTNManagerImpl.cleanUpConfigFile(containerName);
        startVTNManager(c);
        checkVTNconfig(vtnMgr, tpath, bpathlist, pmaps, vmaps);

        // start after cache is cleared
        // this case load from configuration files.
        stopVTNManager(true);
        startVTNManager(c);
        checkVTNconfig(vtnMgr, tpath, bpathlist, pmaps, vmaps);

        // in case another node is loading configuration and
        // waiting process in self node timeout.
        stopVTNManager(true);
        createClusterCache(stubObj, true);

        ConcurrentMap<VTenantPath, Object> stateDB
            = (ConcurrentMap<VTenantPath, Object>) stubObj.getCache(VTNManagerImpl.CACHE_STATE);

        VTenantPath tpathNull = new VTenantPath(null);
        InetAddress ia
            = getInetAddressFromAddress(new byte[] {(byte) 192, (byte) 168,
                                                    (byte) 0, (byte) 2});
        ObjectPair<InetAddress, Boolean> obj
            = new ObjectPair<InetAddress, Boolean>(ia, Boolean.FALSE);
        stateDB.put(tpathNull, obj);

        startVTNManager(c);

        // loaded from configuration file.
        checkVTNconfig(vtnMgr, tpath, bpathlist, pmaps, vmaps);


        // in case another node start loading configuration
        // but loading doesn't finish and stop.
        stopVTNManager(true);
        createClusterCache(stubObj, true);
        stateDB
            = (ConcurrentMap<VTenantPath, Object>) stubObj.getCache(VTNManagerImpl.CACHE_STATE);

        ia = getInetAddressFromAddress(new byte[] {(byte) 192, (byte) 168,
                                                   (byte) 0, (byte) 99});
        obj = new ObjectPair<InetAddress, Boolean>(ia, Boolean.FALSE);
        assertNull(stateDB.put(tpathNull, obj));

        startVTNManager(c);

        // check whether configuration loaded from configuration file.
        checkVTNconfig(vtnMgr, tpath, bpathlist, pmaps, vmaps);


        // start with no cluster service.
        stopVTNManager(true);
        vtnMgr.unsetClusterContainerService(stubObj);
        startVTNManager(c);
        checkVTNconfig(vtnMgr, tpath, bpathlist, pmaps, vmaps);

        stopVTNManager(true);
        vtnMgr.setClusterContainerService(stubObj);

        // remove configuration files.
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

        // check if dummy files exist.
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
     * In case that a cache is remained when cache is created in cluster mode.
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

        stopVTNManager(false);
        VTNManagerImpl.cleanUpConfigFile(containerName);

        ClusterEventMap clsmap = (ClusterEventMap)
            stubObj.getCache(VTNManagerImpl.CACHE_EVENT);
        List<ClusterEvent> posted = clsmap.getPostedEvents();
        assertEquals(1 + bpathlist.size() + ifpathlist.size(), posted.size());
        assertTrue(posted.get(0) instanceof VTenantEvent);
        int index = 1;
        for (VBridgePath path: bpathlist) {
            assertTrue(posted.get(index) instanceof VBridgeEvent);
            index++;
        }
        for (VBridgePath path: ifpathlist) {
            assertTrue(posted.get(index) instanceof VBridgeIfEvent);
            index++;
        }

        posted.clear();
        for (int i = 0; i < 10; i++) {
            FlowGroupId gid = new FlowGroupId("test_tenant");
            VTNFlow flow = new VTNFlow(gid);
            FlowModResultEvent ev =
                new FlowModResultEvent("test", FlowModResult.SUCCEEDED);
            clsmap.put(gid, ev);
            posted.add(ev);
        }

        startVTNManager(c);
        assertTrue(clsmap.isEmpty());
        assertEquals(posted, clsmap.getPostedEvents());

        // in case flow cache remain.
        VTNFlowDatabase fdb = vtnMgr.getTenantFlowDB(tpath.getTenantName());
        VTNFlow flow = fdb.create(vtnMgr);
        Node node = NodeCreator.createOFNode(Long.valueOf(0L));
        NodeConnector innc = NodeConnectorCreator
                .createOFNodeConnector(Short.valueOf("1"), node);
        NodeConnector outnc = NodeConnectorCreator
                .createOFNodeConnector(Short.valueOf("2"), node);
        Match match = new Match();
        match.setField(MatchType.IN_PORT, innc);
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
        FlowGroupId gid = new FlowGroupId("test_tenant");
        flow = new VTNFlow(gid);
        flowMap.put(gid, flow);

        // A entry not associated with existing tenant correctly is
        // removed in initialization process.
        startVTNManager(c);
        assertEquals(1, vtnMgr.getFlowDB().size());

        // in case MAC Address Table cache remain.
        InetAddress ipaddr = getInetAddressFromAddress(new byte[] {10, 0, 0, 1});
        MacTableEntry tent = new MacTableEntry(bpath, 1L, innc,
                                               (short) 0, ipaddr);
        vtnMgr.putMacTableEntry(tent);
        flushTasks();
        List<MacAddressEntry> entries = getMacAddressEntries(tent.getEntryId());
        assertEquals(0, entries.size());
        ConcurrentMap<MacTableEntryId, MacTableEntry> map = vtnMgr.getMacAddressDB();
        assertEquals(1, map.size());

        stopVTNManager(false);
        startVTNManager(c);

        // when entry which is local entry is removed.
        entries = getMacAddressEntries(tent.getEntryId());
        assertEquals(0, entries.size());
        assertEquals(0, map.size());

        // in case remote entry is remained.
        InetAddress ipaddrRemote = getInetAddressFromAddress(new byte[] {0, 0, 0, 0});
        MacTableEntryId evidRemote = new MacTableEntryId(ipaddrRemote, 1L, bpath, 1L);

        tent = new MacTableEntry(evidRemote, innc, (short) 0, ipaddr);
        vtnMgr.putMacTableEntry(tent);
        flushTasks();
        entries = getMacAddressEntries(tent.getEntryId());
        assertEquals(0, entries.size());
        assertEquals(1, map.size());
        stopVTNManager(false);
        startVTNManager(c);

        entries = getMacAddressEntries(tent.getEntryId());
        assertEquals(1, entries.size());
        assertEquals(1, map.size());
    }

    /**
     * Get mac address entries.
     *
     * @param id    A {@link MacTableEntryId}.
     * @return  List of {@link MacAddressEntry}.
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
     * test method for {@link VNodeEvent}.
     * This test event class which inherit {@link VNodeEvent}.
     */
    @Test
    public void testClusterEvent() {
        VTNManagerImpl mgr = vtnMgr;

        VTenantPath tpath = new VTenantPath("tenant");
        VBridgePath bpath = new VBridgePath(tpath.getTenantName(), "bridge");
        VBridgeIfPath ifpath = new VBridgeIfPath(tpath.getTenantName(),
                                                 bpath.getBridgeName(),
                                                 "interface");
        ClusterEventMap clsEvents = (ClusterEventMap)
            stubObj.getCache(VTNManagerImpl.CACHE_EVENT);
        assertTrue(clsEvents.getPostedEvents().isEmpty());

        VTNManagerAwareStub listener = new VTNManagerAwareStub();
        mgr.addVTNManagerAware(listener);
        flushTasks();

        // add vTenant.
        VTenantConfig tconf = new VTenantConfig(null, 300, 0);
        Status st = mgr.addTenant(tpath, tconf);
        assertEquals(StatusCode.SUCCESS, st.getCode());
        List<ClusterEvent> events = clsEvents.getPostedEvents();
        checkVTenantEvent(events, 1, tpath, tconf, UpdateType.ADDED);
        VTenantEvent tev = (VTenantEvent) events.get(0);

        listener.checkVtnInfo(1, tpath, UpdateType.ADDED);
        executeVTenantEvent(mgr, tpath, tev);
        listener.checkVtnInfo(1, tpath, UpdateType.ADDED);

        // change vTenant.
        tconf = new VTenantConfig("desc", 100, 200);
        st = mgr.modifyTenant(tpath, tconf, true);
        assertEquals(StatusCode.SUCCESS, st.getCode());
        events = clsEvents.getPostedEvents();
        checkVTenantEvent(events, 1, tpath, tconf, UpdateType.CHANGED);
        tev = (VTenantEvent) events.get(0);

        listener.checkVtnInfo(1, tpath, UpdateType.CHANGED);
        executeVTenantEvent(mgr, tpath, tev);
        listener.checkVtnInfo(1, tpath, UpdateType.CHANGED);

        // remove vTenant.
        st = mgr.removeTenant(tpath);
        assertEquals(StatusCode.SUCCESS, st.getCode());
        events = clsEvents.getPostedEvents();
        checkVTenantEvent(events, 1, tpath, tconf, UpdateType.REMOVED);
        tev = (VTenantEvent) events.get(0);

        listener.checkVtnInfo(1, tpath, UpdateType.REMOVED);
        executeVTenantEvent(mgr, tpath, tev);
        listener.checkVtnInfo(1, tpath, UpdateType.REMOVED);

        // add vTenant again for following tests.
        tconf = new VTenantConfig(null, 300, 0);
        st = mgr.addTenant(tpath, tconf);
        assertEquals(StatusCode.SUCCESS, st.getCode());

        listener.checkVtnInfo(1, tpath, UpdateType.ADDED);
        events = clsEvents.getPostedEvents();
        checkVTenantEvent(events, 1, tpath, tconf, UpdateType.ADDED);

        // add vBridge.
        VBridgeConfig bconf = new VBridgeConfig(null, 600);
        st = mgr.addBridge(bpath, bconf);
        assertEquals(StatusCode.SUCCESS, st.getCode());
        events = clsEvents.getPostedEvents();
        checkVBridgeEvent(events, 1, bpath, bconf, UpdateType.ADDED);
        VBridgeEvent bev = (VBridgeEvent) events.get(0);

        listener.checkVbrInfo(1, bpath, UpdateType.ADDED);
        executeVBridgeEvent(mgr, bpath, bev, true);
        listener.checkVbrInfo(1, bpath, UpdateType.ADDED);

        // change vBridge.
        bconf = new VBridgeConfig("desc", 600);
        st = mgr.modifyBridge(bpath, bconf, true);
        assertEquals(StatusCode.SUCCESS, st.getCode());
        events = clsEvents.getPostedEvents();
        checkVBridgeEvent(events, 1, bpath, bconf, UpdateType.CHANGED);
        bev = (VBridgeEvent) events.get(0);

        listener.checkVbrInfo(1, bpath, UpdateType.CHANGED);
        executeVBridgeEvent(mgr, bpath, bev, true);
        listener.checkVbrInfo(1, bpath, UpdateType.CHANGED);

        // remove vBridge.
        st = mgr.removeBridge(bpath);
        assertEquals(StatusCode.SUCCESS, st.getCode());
        events = clsEvents.getPostedEvents();
        checkVBridgeEvent(events, 1, bpath, bconf, UpdateType.REMOVED);
        bev = (VBridgeEvent) events.get(0);

        listener.checkVbrInfo(1, bpath, UpdateType.REMOVED);
        executeVBridgeEvent(mgr, bpath, bev, true);
        listener.checkVbrInfo(1, bpath, UpdateType.REMOVED);

        // add vBridge for following tests.
        bconf = new VBridgeConfig(null, 600);
        st = mgr.addBridge(bpath, bconf);
        assertEquals(StatusCode.SUCCESS, st.getCode());

        listener.checkVbrInfo(1, bpath, UpdateType.ADDED);
        events = clsEvents.getPostedEvents();
        checkVBridgeEvent(events, 1, bpath, bconf, UpdateType.ADDED);

        // add vBridgeInterface.
        VInterfaceConfig ifconf = new VInterfaceConfig(null, Boolean.TRUE);
        st = mgr.addBridgeInterface(ifpath, ifconf);
        assertEquals(StatusCode.SUCCESS, st.getCode());
        events = clsEvents.getPostedEvents();
        checkVBridgeIfEvent(events, 1, ifpath, ifconf, UpdateType.ADDED);
        VBridgeIfEvent ifev = (VBridgeIfEvent) events.get(0);

        listener.checkVIfInfo(1, ifpath, UpdateType.ADDED);
        executeClusterEvent(mgr, ifpath, ifev, true);
        listener.checkVIfInfo(1, ifpath, UpdateType.ADDED);

        // change vBridgeInterface.
        ifconf = new VInterfaceConfig("description", Boolean.TRUE);
        st = mgr.modifyBridgeInterface(ifpath, ifconf, true);
        assertEquals(StatusCode.SUCCESS, st.getCode());
        events = clsEvents.getPostedEvents();
        checkVBridgeIfEvent(events, 1, ifpath, ifconf, UpdateType.CHANGED);
        ifev = (VBridgeIfEvent) events.get(0);

        listener.checkVIfInfo(1, ifpath, UpdateType.CHANGED);
        executeClusterEvent(mgr, ifpath, ifev, true);
        listener.checkVIfInfo(1, ifpath, UpdateType.CHANGED);

        // remove vBridgeInterface.
        st = mgr.removeBridgeInterface(ifpath);
        assertEquals(StatusCode.SUCCESS, st.getCode());
        events = clsEvents.getPostedEvents();
        checkVBridgeIfEvent(events, 1, ifpath, ifconf, UpdateType.REMOVED);
        ifev = (VBridgeIfEvent) events.get(0);

        listener.checkVIfInfo(1, ifpath, UpdateType.REMOVED);
        executeClusterEvent(mgr, ifpath, ifev, true);
        listener.checkVIfInfo(1, ifpath, UpdateType.REMOVED);

        // add vBridgeInterface for following tests.
        ifconf = new VInterfaceConfig(null, Boolean.TRUE);
        st = mgr.addBridgeInterface(ifpath, ifconf);
        assertEquals(StatusCode.SUCCESS, st.getCode());

        listener.checkVIfInfo(1, ifpath, UpdateType.ADDED);
        events = clsEvents.getPostedEvents();
        checkVBridgeIfEvent(events, 1, ifpath, ifconf, UpdateType.ADDED);

        // add PortMap. When PortMap status is changed,
        // VBridge and VBridgeInterface status is also changed.
        Node node0 = NodeCreator.createOFNode(Long.valueOf("0"));
        SwitchPort port = new SwitchPort(node0.getType(), "10");
        short vlan = 0;
        PortMapConfig pmconf = new PortMapConfig(node0, port, vlan);
        st = mgr.setPortMap(ifpath, pmconf);
        assertEquals(StatusCode.SUCCESS, st.getCode());
        events = clsEvents.getPostedEvents();
        assertEquals(3, events.size());

        bev = null;
        ifev = null;
        PortMapEvent pmev = null;
        for (ClusterEvent ev : events) {
            if (ev instanceof VBridgeEvent) {
                if (bev != null) {
                    fail("unexpected event was posted.(" + ev.toString() + ")");
                }
                bev = (VBridgeEvent) ev;
                checkVBridgeEvent(bev, bpath, bconf,
                                  VNodeState.UP, UpdateType.CHANGED);
                listener.checkVbrInfo(1, bpath, UpdateType.CHANGED);
                executeVBridgeEvent(mgr, bpath, bev, false);
                listener.checkVbrInfo(1, bpath, UpdateType.CHANGED);
            } else if (ev instanceof VBridgeIfEvent) {
                if (ifev != null) {
                    fail("unexpected event was posted.(" + ev.toString() + ")");
                }
                ifev = (VBridgeIfEvent) ev;
                checkVBridgeIfEvent(ifev, ifpath, ifconf,
                                    VNodeState.UP, VNodeState.UP,
                                    UpdateType.CHANGED);
                listener.checkVIfInfo(1, ifpath, UpdateType.CHANGED);
                executeClusterEvent(mgr, ifpath, ifev, false);
                listener.checkVIfInfo(1, ifpath, UpdateType.CHANGED);
            } else if (ev instanceof PortMapEvent) {
                if (pmev != null) {
                    fail("unexpected event was posted.(" + ev.toString() + ")");
                }
                pmev = (PortMapEvent) ev;
                checkPortMapEvent(pmev, ifpath, pmconf, UpdateType.ADDED);
                listener.checkPmapInfo(1, ifpath, pmconf, UpdateType.ADDED);
                executeClusterEvent(mgr, ifpath, pmev, true);
                listener.checkPmapInfo(1, ifpath, pmconf, UpdateType.ADDED);
            }
        }

        // change PortMap
        pmconf = new PortMapConfig(node0, port, (short) 1);
        st = vtnMgr.setPortMap(ifpath, pmconf);
        assertEquals(StatusCode.SUCCESS, st.getCode());
        events = clsEvents.getPostedEvents();
        assertEquals(1, events.size());
        checkPortMapEvent(events, 1, ifpath, pmconf, UpdateType.CHANGED);
        pmev = (PortMapEvent) events.get(0);

        listener.checkPmapInfo(1, ifpath, pmconf, UpdateType.CHANGED);
        executeClusterEvent(mgr, ifpath, pmev, true);
        listener.checkPmapInfo(1, ifpath, pmconf, UpdateType.CHANGED);

        // clear PortMap. When portmap is removed,
        // VBridge and VBridgeInterface status is also changed.
        st = mgr.setPortMap(ifpath, null);
        assertEquals(StatusCode.SUCCESS, st.getCode());
        events = clsEvents.getPostedEvents();
        assertEquals(3, events.size());
        flushTasks();
        bev = null;
        ifev = null;
        pmev = null;
        for (ClusterEvent ev : events) {
            if (ev instanceof VBridgeEvent) {
                if (bev != null) {
                    fail("unexpected event was posted.(" + ev.toString() + ")");
                }
                bev = (VBridgeEvent) ev;
                checkVBridgeEvent(bev, bpath, bconf,
                                  VNodeState.UNKNOWN, UpdateType.CHANGED);
                listener.checkVbrInfo(1, bpath, UpdateType.CHANGED);
                executeVBridgeEvent(mgr, bpath, bev, false);
                listener.checkVbrInfo(1, bpath, UpdateType.CHANGED);
            } else if (ev instanceof VBridgeIfEvent) {
                if (ifev != null) {
                    fail("unexpected event was posted.(" + ev.toString() + ")");
                }
                ifev = (VBridgeIfEvent) ev;
                checkVBridgeIfEvent(ifev, ifpath, ifconf,
                                    VNodeState.UNKNOWN, VNodeState.UNKNOWN,
                                    UpdateType.CHANGED);
                listener.checkVIfInfo(1, ifpath, UpdateType.CHANGED);
                executeClusterEvent(mgr, ifpath, ifev, false);
                listener.checkVIfInfo(1, ifpath, UpdateType.CHANGED);
            } else if (ev instanceof PortMapEvent) {
                if (pmev != null) {
                    fail("unexpected event was posted.(" + ev.toString() + ")");
                }
                pmev = (PortMapEvent) ev;
                checkPortMapEvent(pmev, ifpath, pmconf, UpdateType.REMOVED);
                listener.checkPmapInfo(1, ifpath, pmconf, UpdateType.REMOVED);
                executeClusterEvent(mgr, ifpath, pmev, true);
                listener.checkPmapInfo(1, ifpath, pmconf, UpdateType.REMOVED);
            }
        }

        // add VlanMap.
        // VBridge status is also changed.
        VlanMapConfig vlconf = new VlanMapConfig(node0, (short) 0);
        VlanMap map = null;
        try {
            map = vtnMgr.addVlanMap(bpath, vlconf);
        } catch (VTNException e) {
            unexpected(e);
        }
        assertNotNull(map);
        events = clsEvents.getPostedEvents();
        assertEquals(2, events.size());

        bev = null;
        VlanMapEvent vlev = null;
        for (ClusterEvent ev : events) {
            if (ev instanceof VBridgeEvent) {
                if (bev != null) {
                    fail("unexpected event was posted.(" + ev.toString() + ")");
                }
                bev = (VBridgeEvent) ev;
                checkVBridgeEvent(bev, bpath, bconf,
                                  VNodeState.UP, UpdateType.CHANGED);
                listener.checkVbrInfo(1, bpath, UpdateType.CHANGED);
                executeVBridgeEvent(mgr, bpath, bev, false);
                listener.checkVbrInfo(1, bpath, UpdateType.CHANGED);
            } else if (ev instanceof VlanMapEvent) {
                if (vlev != null) {
                    fail("unexpected event was posted.(" + ev.toString() + ")");
                }
                vlev = (VlanMapEvent) ev;
                checkVlanMapEvent(vlev, bpath, map, UpdateType.ADDED);
                listener.checkVlmapInfo(1, bpath, map.getId(), UpdateType.ADDED);
                executeClusterEvent(mgr, ifpath, vlev, true);
                listener.checkVlmapInfo(1, bpath, map.getId(), UpdateType.ADDED);
            }
        }

        // remove VlanMap.
        // VBridge status is also changed.
        st = mgr.removeVlanMap(bpath, map.getId());
        assertEquals(StatusCode.SUCCESS, st.getCode());
        events = clsEvents.getPostedEvents();
        assertEquals(2, events.size());

        bev = null;
        vlev = null;
        for (ClusterEvent ev : events) {
            if (ev instanceof VBridgeEvent) {
                if (bev != null) {
                    fail("unexpected event was posted.(" + ev.toString() + ")");
                }
                bev = (VBridgeEvent) ev;
                checkVBridgeEvent(bev, bpath, bconf,
                                  VNodeState.UNKNOWN, UpdateType.CHANGED);
                listener.checkVbrInfo(1, bpath, UpdateType.CHANGED);
                executeVBridgeEvent(mgr, bpath, bev, false);
                listener.checkVbrInfo(1, bpath, UpdateType.CHANGED);
            } else if (ev instanceof VlanMapEvent) {
                if (vlev != null) {
                    fail("unexpected event was posted.(" + ev.toString() + ")");
                }
                vlev = (VlanMapEvent) ev;
                checkVlanMapEvent(vlev, bpath, map, UpdateType.REMOVED);
                listener.checkVlmapInfo(1, bpath, map.getId(), UpdateType.REMOVED);
                executeClusterEvent(mgr, ifpath, vlev, true);
                listener.checkVlmapInfo(1, bpath, map.getId(), UpdateType.REMOVED);
            }
        }
        flushTasks();
        listener.checkAllNull();

        // add port map and vlan map.
        pmconf = new PortMapConfig(node0, port, (short) 0);
        st = mgr.setPortMap(ifpath, pmconf);
        assertEquals(StatusCode.SUCCESS, st.getCode());

        events = clsEvents.getPostedEvents();
        assertEquals(3, events.size());
        checkPortMapEvent((PortMapEvent)events.get(0), ifpath, pmconf,
                          UpdateType.ADDED);
        checkVBridgeIfEvent((VBridgeIfEvent)events.get(1), ifpath, ifconf,
                            VNodeState.UP, VNodeState.UP, UpdateType.CHANGED);
        checkVBridgeEvent((VBridgeEvent)events.get(2), bpath, bconf,
                          VNodeState.UP, UpdateType.CHANGED);

        listener.checkVbrInfo(1, bpath, UpdateType.CHANGED);
        listener.checkVIfInfo(1, ifpath, UpdateType.CHANGED);
        listener.checkPmapInfo(1, ifpath, pmconf, UpdateType.ADDED);

        vlconf = new VlanMapConfig(node0, (short) 4095);
        try {
            map = vtnMgr.addVlanMap(bpath, vlconf);
        } catch (VTNException e) {
            unexpected(e);
        }
        assertNotNull(map);

        events = clsEvents.getPostedEvents();
        assertEquals(1, events.size());
        checkVlanMapEvent((VlanMapEvent)events.get(0), bpath, map,
                          UpdateType.ADDED);

        listener.checkVlmapInfo(1, bpath, map.getId(), UpdateType.ADDED);

        // remove tenant.
        // VTenantEvent, VBridgeEvent, VBridgeIfEvent, PortMapEvent and
        // VlanMapEvent are posted.
        st = mgr.removeTenant(tpath);
        assertEquals(StatusCode.SUCCESS, st.getCode());
        events = clsEvents.getPostedEvents();
        assertEquals(5, events.size());

        listener.checkVtnInfo(1, tpath, UpdateType.REMOVED);
        listener.checkVbrInfo(1, bpath, UpdateType.REMOVED);
        listener.checkVIfInfo(1, ifpath, UpdateType.REMOVED);
        listener.checkPmapInfo(1, ifpath, pmconf, UpdateType.REMOVED);
        listener.checkVlmapInfo(1, bpath, map.getId(), UpdateType.REMOVED);

        flushTasks();
        listener.checkAllNull();

        checkEventCleared(clsEvents);
    }

    /**
     * Check {@link VTenantEvent} object.
     *
     * @param events    A list of {@link ClusterEvent}.
     * @param numEvents A number of events in {@code clsEvents}.
     * @param tpath     A {@link VTenantPath}.
     * @param tconf     A {@link VTenantConfig}.
     * @param utype     A {@link UpdateType}.
     */
    private void checkVTenantEvent(List<ClusterEvent> events,
                                   int numEvents, VTenantPath tpath,
                                   VTenantConfig tconf, UpdateType utype) {
        assertEquals(numEvents, events.size());
        for (ClusterEvent ev : events) {
            if (ev instanceof VTenantEvent) {
                VTenantEvent tev = (VTenantEvent)ev;
                assertEquals(tpath, tev.getPath());
                assertEquals(new VTenant(tpath.getTenantName(), tconf),
                             tev.getVTenant());
                assertEquals(utype, tev.getUpdateType());
                assertEquals("virtual tenant", tev.getTypeName());
            } else {
                fail("unexpected event was posted.");
            }
        }
    }

    /**
     * Check {@link VBridgeEvent}.
     *
     * @param events    A list of {@link ClusterEvent}.
     * @param numEvents A number of events in {@code clsEvents}.
     * @param bpath     A {@link VBridgePath}.
     * @param bconf     A {@link VBridgeConfig}.
     * @param utype     A {@link UpdateType}.
     */
    private void checkVBridgeEvent(List<ClusterEvent> events,
                                   int numEvents, VBridgePath bpath,
                                   VBridgeConfig bconf, UpdateType utype) {
        assertEquals(numEvents, events.size());
        for (ClusterEvent ev : events) {
            if (ev instanceof VBridgeEvent) {
                VBridgeEvent bev = (VBridgeEvent)ev;
                checkVBridgeEvent(bev, bpath, bconf, VNodeState.UNKNOWN, utype);
            } else {
                fail("unexpected event was posted.");
            }
        }
    }

    /**
     * Check {@link VBridgeEvent}.
     *
     * @param bev   A {@link VBridgeEvent}.
     * @param bpath A {@link VBridgePath}.
     * @param bconf A {@link VBridgeConfig}.
     * @param state A state of {@link VBridge}.
     * @param utype A {@link UpdateType}.
     */
    private void checkVBridgeEvent(VBridgeEvent bev, VBridgePath bpath,
                                   VBridgeConfig bconf, VNodeState state,
                                   UpdateType utype) {
        assertEquals(bpath, bev.getPath());
        VBridge brdg = new VBridge(bpath.getBridgeName(), state,
                                   0, bconf);
        assertEquals(brdg, bev.getVBridge());
        assertEquals(utype, bev.getUpdateType());
        assertEquals("virtual bridge", bev.getTypeName());
    }

    /**
     * Check {@link VBridgeIfEvent}.
     *
     * @param events    A list of {@link ClusterEvent}.
     * @param numEvents A number of events in {@code clsEvents}.
     * @param ifpath    A {@link VBridgeIfPath}.
     * @param ifconf    A {@link VInterfaceConfig}.
     * @param utype     A {@link UpdateType}.
     */
    private void checkVBridgeIfEvent(List<ClusterEvent> events,
                                     int numEvents, VBridgeIfPath ifpath,
                                     VInterfaceConfig ifconf,
                                     UpdateType utype) {
        assertEquals(numEvents, events.size());
        for (ClusterEvent ev : events) {
            if (ev instanceof VBridgeIfEvent) {
                VBridgeIfEvent ifev = (VBridgeIfEvent)ev;
                checkVBridgeIfEvent(ifev, ifpath, ifconf,
                                    VNodeState.UNKNOWN, VNodeState.UNKNOWN,
                                    utype);
            } else {
                fail("unexpected event was posted.");
            }
        }
    }

    /**
     * Check {@link VBridgeIfEvent}.
     *
     * @param ifev      A {@link VBridgeIfEvent}.
     * @param ifpath    A {@link VBridgeIfPath}.
     * @param ifconf    A {@link VInterfaceConfig}.
     * @param state     A state of {@link VInterface}.
     * @param estate    A state of port mapped to.
     * @param utype     A {@link UpdateType}.
     */
    private void checkVBridgeIfEvent(VBridgeIfEvent ifev, VBridgeIfPath ifpath,
                                     VInterfaceConfig ifconf,
                                     VNodeState state, VNodeState estate,
                                     UpdateType utype) {
        assertEquals(ifpath, ifev.getPath());
        VInterface bif = new VInterface(ifpath.getInterfaceName(),
                                        state, estate, ifconf);
        assertEquals(bif, ifev.getVInterface());
        assertEquals(utype, ifev.getUpdateType());
        assertEquals("virtual bridge interface", ifev.getTypeName());
    }

    /**
     * Check {@link PortMapEvent}.
     *
     * @param events    A list of {@link ClusterEvent}.
     * @param numEvents A number of events in {@code clsEvents}.
     * @param ifpath    A {@link VBridgeIfPath}.
     * @param pmconf    A {@link PortMapConfig}.
     * @param utype     A {@link UpdateType}.
     */
    private void checkPortMapEvent(List<ClusterEvent> events,
                                   int numEvents, VBridgeIfPath ifpath,
                                   PortMapConfig pmconf, UpdateType utype) {

        assertEquals(numEvents, events.size());
        for (ClusterEvent ev : events) {
            if (ev instanceof PortMapEvent) {
                PortMapEvent pmev = (PortMapEvent) ev;
                checkPortMapEvent(pmev, ifpath, pmconf, utype);
            } else {
                fail("unexpected event was posted.");
            }
        }
    }

    /**
     * Check {@link PortMapEvent}.
     *
     * @param ev        A {@link PortMapEvent}.
     * @param ifpath    A {@link VBridgeIfPath}.
     * @param pmconf    A {@link PortMapConfig}.
     * @param utype     A {@link UpdateType}.
     */
    private void checkPortMapEvent(PortMapEvent ev,
                                   VBridgeIfPath ifpath, PortMapConfig pmconf,
                                   UpdateType utype) {
        assertEquals(ifpath, ev.getPath());
        assertEquals(pmconf, ev.getPortMap().getConfig());
        NodeConnector nc = NodeConnectorCreator
                .createOFNodeConnector(Short.valueOf(pmconf.getPort().getId()),
                                       pmconf.getNode());
        PortMap pmap = new PortMap(pmconf, nc);
        assertEquals(pmap, ev.getPortMap());
        assertEquals(utype, ev.getUpdateType());
        assertEquals("port mapping", ev.getTypeName());
    }

    /**
     * Check {@link VlanMapEvent}.
     *
     * @param ev    A {@link VlanMapEvent}.
     * @param bpath A {@link VBridgePath}.
     * @param map   A {@link VlanMap}.
     * @param utype A {@link UpdateType}.
     */
    private void checkVlanMapEvent(VlanMapEvent ev,
                                   VBridgePath bpath, VlanMap map,
                                   UpdateType utype) {
        assertEquals(bpath, ev.getPath());
        assertEquals(map, ev.getVlanMap());
        assertEquals(utype, ev.getUpdateType());
        assertEquals("VLAN mapping", ev.getTypeName());
    }

    /**
     * Execute {@link VTenantEvent} and check result.
     *
     * @param mgr       VTNManager service.
     * @param path      A {@link VTenantPath}.
     * @param event     A {@link VTenantEvent}.
     */
    private void executeVTenantEvent(VTNManagerImpl mgr, VTenantPath path,
                                     VTenantEvent event) {
        String tname = path.getTenantName();
        String root = GlobalConstants.STARTUPHOME.toString();
        String tenantListFileName = root + "vtn-default-tenant-names.conf";
        String configFileName =
            root + "vtn-" + "default" + "-" +  tname + ".conf";
        File tenantList = new File(tenantListFileName);
        File configFile = new File(configFileName);

        Set<ClusterEventId> evIdSet = new HashSet<ClusterEventId>();
        InetAddress ipaddr = getInetAddressFromAddress(new byte[] {0, 0, 0, 0});
        ClusterEventId evidRemote = new ClusterEventId(ipaddr, 0);
        ClusterEventId evidLocal = new ClusterEventId();
        evIdSet.add(evidLocal);
        evIdSet.add(evidRemote);

        for (ClusterEventId evId : evIdSet) {
            if (event.getUpdateType() == UpdateType.ADDED
                || event.getUpdateType() == UpdateType.CHANGED) {
                tenantList.delete();
                configFile.delete();
                if (event.getUpdateType() == UpdateType.ADDED) {
                    mgr.removeTenantFlowDB(tname);
                }
            } else {
                if (!tenantList.exists()) {
                    try {
                        tenantList.createNewFile();
                    } catch (IOException e) {
                        unexpected(e);
                    }
                }
                if (!configFile.exists()) {
                    try {
                        configFile.createNewFile();
                    } catch (IOException e) {
                        unexpected(e);
                    }
                }
                if (mgr.getTenantFlowDB(tname) == null) {
                    mgr.createTenantFlowDB(tname);
                }
            }

            mgr.entryUpdated(evId, event, VTNManagerImpl.CACHE_EVENT, true);
            flushTasks();
            if (event.getUpdateType() == UpdateType.ADDED
                    || event.getUpdateType() == UpdateType.CHANGED) {
                assertFalse(tenantList.exists());
                assertFalse(configFile.exists());
                assertNull(mgr.getTenantFlowDB(tname));
            } else {
                assertTrue(tenantList.exists());
                assertTrue(configFile.exists());
                assertNotNull(mgr.getTenantFlowDB(tname));
            }

            mgr.entryUpdated(evId, event, VTNManagerImpl.CACHE_EVENT, false);
            flushTasks();
            switch (event.getUpdateType()) {
            case ADDED:
                if (evId == evidRemote) {
                    assertTrue(tenantList.exists());
                    assertTrue(configFile.exists());
                    assertNotNull(mgr.getTenantFlowDB(tname));
                } else {
                    assertFalse(tenantList.exists());
                    assertFalse(configFile.exists());
                    assertNull(mgr.getTenantFlowDB(tname));
                }
                break;
            case CHANGED:
                if (evId == evidRemote) {
                    assertFalse(tenantList.exists());
                    assertTrue(configFile.exists());
                } else {
                    assertFalse(tenantList.exists());
                    assertFalse(configFile.exists());
                }
                break;
            case REMOVED:
                if (evId == evidRemote) {
                    assertTrue(tenantList.exists());
                    assertFalse(configFile.exists());
                    assertNull(mgr.getTenantFlowDB(tname));
                } else {
                    assertTrue(tenantList.exists());
                    assertTrue(configFile.exists());
                    assertNotNull(mgr.getTenantFlowDB(tname));
                }
                break;
            default:
                fail("Unexpected case.");
                break;
            }
        }
    }

    /**
     * Execute {@link VBridgeEvent} and check result.
     *
     * @param mgr       VTNManager service.
     * @param path      A {@link VBridgePath}.
     * @param event     A {@link VBridgeEvent}
     * @param saveConfig    if {@code true} expect that configuration file is saved.
     *                      if {@code false} expect that it isn't saved.
     */
    private void executeVBridgeEvent(VTNManagerImpl mgr, VBridgePath path,
                                     VBridgeEvent event, boolean saveConfig) {
        String tname = path.getTenantName();
        String root = GlobalConstants.STARTUPHOME.toString();
        String tenantListFileName = root + "vtn-default-tenant-names.conf";
        String configFileName =
            root + "vtn-" + "default" + "-" +  tname + ".conf";
        File tenantList = new File(tenantListFileName);
        File configFile = new File(configFileName);

        Set<ClusterEventId> evIdSet = new HashSet<ClusterEventId>();
        InetAddress ipaddr = getInetAddressFromAddress(new byte[] {0, 0, 0, 0});
        ClusterEventId evIdRemote = new ClusterEventId(ipaddr, 0);
        ClusterEventId evIdLocal = new ClusterEventId();
        evIdSet.add(evIdLocal);
        evIdSet.add(evIdRemote);

        for (ClusterEventId evId : evIdSet) {
            configFile.delete();
            if (mgr.getMacAddressTable(path) == null) {
                mgr.addMacAddressTable(path, 600);
            }

            mgr.entryUpdated(evId, event, VTNManagerImpl.CACHE_EVENT, true);
            flushTasks();
            assertFalse(configFile.exists());
            assertNotNull(mgr.getMacAddressTable(path));

            mgr.entryUpdated(evId, event, VTNManagerImpl.CACHE_EVENT, false);
            flushTasks();

            if (evId == evIdRemote && saveConfig) {
                assertTrue(configFile.exists());
            } else {
                assertFalse(configFile.exists());
            }

            if (evId == evIdRemote
                    && event.getUpdateType() == UpdateType.REMOVED) {
                assertNull(mgr.getMacAddressTable(path));

            } else {
                assertNotNull(mgr.getMacAddressTable(path));
            }
            assertTrue(tenantList.exists());
        }
    }

    /**
     * Execute {@link ClusterEvent} and check result.
     *
     * @param mgr       VTNManager service.
     * @param path      A {@link VTenantPath} or object inherit it.
     * @param event     A {@link ClusterEvent} or object inherit it.
     * @param saveConfig    if {@code true} expect that configuration file is saved.
     *                      if {@code false} expect that it isn't saved.
     */
    private void executeClusterEvent(VTNManagerImpl mgr, VTenantPath path,
                                       ClusterEvent event, boolean saveConfig) {
        String tname = path.getTenantName();
        String root = GlobalConstants.STARTUPHOME.toString();
        String tenantListFileName = root + "vtn-default-tenant-names.conf";
        String configFileName =
            root + "vtn-" + "default" + "-" +  tname + ".conf";
        File tenantList = new File(tenantListFileName);
        File configFile = new File(configFileName);

        Set<ClusterEventId> evIdSet = new HashSet<ClusterEventId>();
        InetAddress ipaddr = getInetAddressFromAddress(new byte[] {0, 0, 0, 0});
        ClusterEventId evIdRemote = new ClusterEventId(ipaddr, 0);
        ClusterEventId evIdLocal = new ClusterEventId();
        evIdSet.add(evIdLocal);
        evIdSet.add(evIdRemote);

        for (ClusterEventId evId : evIdSet) {
            configFile.delete();

            mgr.entryUpdated(evId, event, VTNManagerImpl.CACHE_EVENT, true);
            flushTasks();
            assertFalse(configFile.exists());

            mgr.entryUpdated(evId, event, VTNManagerImpl.CACHE_EVENT, false);
            flushTasks();

            if (evId == evIdRemote && saveConfig) {
                assertTrue(configFile.exists());
            } else {
                assertFalse(configFile.exists());
            }
            assertTrue(tenantList.exists());
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
        Node node0 = NodeCreator.createOFNode(Long.valueOf(0L));
        Node node1 = NodeCreator.createOFNode(Long.valueOf(1L));
        Set<Node> nodeSet = new HashSet<Node>();
        nodeSet.add(node0);
        nodeSet.add(node1);

        InetAddress ipaddr = getInetAddressFromAddress(new byte[] {0, 0, 0, 0});
        ClusterEventId evidRemote = new ClusterEventId(ipaddr, 0);
        ClusterEventId evidLocal = new ClusterEventId();
        Set<ClusterEventId> evIdSet = new HashSet<ClusterEventId>();
        evIdSet.add(evidLocal);
        evIdSet.add(evidRemote);

        for (Node node : nodeSet) {
            NodeConnector innc = NodeConnectorCreator
                    .createOFNodeConnector(Short.valueOf((short) 10), node);
            NodeConnector outnc = NodeConnectorCreator
                    .createOFNodeConnector(Short.valueOf((short) 11), node);
            RawPacket pkt = createARPRawPacket(src, dst, sender, target,
                                               (short) 0, innc, ARP.REQUEST);
            RawPacketEvent ev = new RawPacketEvent(pkt, outnc);

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
                if (evid == evidRemote && node.equals(node0)) {
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
    }

    /**
     * Test method for
     * {@link VTNManagerImpl#notifyNode(Node, UpdateType, Map)},
     * {@link VTNManagerImpl#notifyNodeConnector(NodeConnector, UpdateType, Map)},
     * {@link VTNManagerImpl#edgeUpdate(java.util.List)}.
     *
     * <p>
     * test with both port map and vlan map set in cluster mode.
     * </p>
     */
    @Test
    public void  testNotificationWithBothMapped() {
        VTNManagerImpl mgr = vtnMgr;
        Set<Node> nodeSet = new HashSet<Node>();

        short[] vlans = new short[] { 0, 10, 4095 };

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
                new SwitchPort("port-10", NodeConnector.NodeConnectorIDType.OPENFLOW, "10"),
                new SwitchPort(null, NodeConnector.NodeConnectorIDType.OPENFLOW, "11"),
                new SwitchPort("port-10", null, null),
                new SwitchPort("port-10"),
                new SwitchPort(NodeConnector.NodeConnectorIDType.OPENFLOW, "13"),
        };

        for (Node cnode : nodeSet) {
            if (cnode == null) {
                continue;
            }
            NodeConnector cnc
                = NodeConnectorCreator
                    .createOFNodeConnector(Short.valueOf((short)10), cnode);

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
                        if(port.getId() != null) {
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
                                            && vmapNode.getType() != NodeConnector.NodeConnectorIDType.OPENFLOW) {
                                        fail("throwing Exception was expected.");
                                    }
                                } catch (VTNException e) {
                                    if (vmapNode != null
                                            && vmapNode.getType() == NodeConnector.NodeConnectorIDType.OPENFLOW) {
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
                                checkNotification (mgr, bpath, ifp, cnc,
                                                   cnode, pmconf, vlconf,
                                                   MapType.ALL, emsgVmap);

                                st = mgr.removeVlanMap(bpath, vmap.getId());
                                assertEquals(emsgVmap, StatusCode.SUCCESS, st.getCode());
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
     * Test Notification APIs.
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
    protected void checkNotification(VTNManagerImpl mgr, VBridgePath bpath,
                                     VBridgeIfPath ifp, NodeConnector chgNc,
                                     Node chgNode, PortMapConfig pmconf,
                                     VlanMapConfig vlconf, MapType mapType,
                                     String msg) {
        ISwitchManager swMgr = mgr.getSwitchManager();
        ITopologyManager topoMgr = mgr.getTopologyManager();
        VTenantPath tpath = new VTenantPath(bpath.getTenantName());

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
        Map<String, Property> propMap = null; // not used now.
        checkMacTableEntry(mgr, bpath, true, msg);
        putMacTableEntry(mgr, bpath, chgNc);
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
        if (chgNc.equals(mapNc) || vlanMapNode == null || chgNode.equals(vlanMapNode)) {
            checkMacTableEntry(mgr, bpath, true, msg);
        } else {
            checkMacTableEntry(mgr, bpath, false, msg);
        }

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

        // test for node change notify
        Node node0 = NodeCreator.createOFNode(Long.valueOf("0"));
        NodeConnector innc = NodeConnectorCreator
                .createOFNodeConnector(Short.valueOf("10"), node0);
        NodeConnector outnc = NodeConnectorCreator
                .createOFNodeConnector(Short.valueOf("11"), node0);

        VTNFlow flow = putFlowEntry(mgr, tpath.getTenantName(), innc, outnc);
        putMacTableEntry(mgr, bpath, chgNc);
        mgr.notifyNode(chgNode, UpdateType.REMOVED, propMap);
        if (mapType.equals(MapType.PORT) || mapType.equals(MapType.ALL)) {
            if (chgNode.equals(portMapNode)) {
                checkNodeStatus(mgr, bpath, ifp, VNodeState.DOWN, VNodeState.DOWN, msg);
                checkMacTableEntry(mgr, bpath, true, msg);
            } else if (chgNode.equals(vlanMapNode)) {
                checkNodeStatus(mgr, bpath, ifp, VNodeState.DOWN, VNodeState.UP, msg);
                checkMacTableEntry(mgr, bpath, true, msg);
            } else {
                checkNodeStatus(mgr, bpath, ifp, VNodeState.UP, VNodeState.UP, msg);
                checkMacTableEntry(mgr, bpath,
                        (mapType.equals(MapType.ALL) && vlanMapNode == null) ? true : false,
                        msg);
            }
        } else {
            if (chgNode.equals(vlanMapNode)) {
                checkNodeStatus(mgr, bpath, ifp, VNodeState.DOWN, VNodeState.UNKNOWN,msg);
                checkMacTableEntry(mgr, bpath, true, msg);
            } else {
                checkNodeStatus(mgr, bpath, ifp, VNodeState.UP, VNodeState.UNKNOWN,msg);
                checkMacTableEntry(mgr, bpath, (vlanMapNode == null) ? true : false, msg);
            }
        }

        flushFlowTasks();
        if (chgNc.getNode().equals(node0)) {
            assertFalse(checkFlowEntry(mgr, flow));
            for (FlowEntry ent : flow.getFlowEntries()) {
                stubObj.uninstallFlowEntry(ent);
            }
        } else {
            assertTrue(checkFlowEntry(mgr, flow));
        }
        mgr.removeAllFlows(tpath);
        flushMacTableEntry(mgr, bpath);

        mgr.initInventory();
        mgr.initISL();
        flow = putFlowEntry(mgr, tpath.getTenantName(), innc, outnc);
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

        flushFlowTasks();
        if (chgNc.getNode().equals(node0)) {
            assertFalse(checkFlowEntry(mgr, flow));
            for (FlowEntry ent : flow.getFlowEntries()) {
                stubObj.uninstallFlowEntry(ent);
            }
        } else {
            assertTrue(checkFlowEntry(mgr, flow));
        }
        mgr.removeAllFlows(tpath);

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
     * Test method for {@link VTNManagerImpl#receiveDataPacket(RawPacket)}.
     * The case both VlanMap and PortMap applied to vBridge.
     *
     * This tests with connection manager have not_local status node.
     */
    @Test
    public void testReceiveDataPacketBothMapped() {
        VTNManagerImpl mgr = vtnMgr;
        TestStubCluster stub = (TestStubCluster) stubObj;
        ISwitchManager swmgr = mgr.getSwitchManager();
        ITopologyManager topomgr = mgr.getTopologyManager();
        short[] vlans = new short[] { 0, 10, 4095 };
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
        final int NUM_INTERFACES = 2;
        final int MAX_INTERFACE_VBRIDGE1 = 1;

        for (short i = 0; i < NUM_INTERFACES; i++) {
            int inode = 0;
            for (Node node : existNodes) {
                String bname = (i < MAX_INTERFACE_VBRIDGE1) ? bname1 : bname2;
                String ifname = "vinterface" + inode + (i + 10);
                VBridgeIfPath ifp = new VBridgeIfPath(tname, bname, ifname);
                st = mgr.addBridgeInterface(ifp, new VInterfaceConfig(null, Boolean.TRUE));
                assertEquals("(VBridgeIfPath)" + ifp.toString(),
                             StatusCode.SUCCESS, st.getCode());
                inode++;
            }
        }

        for (short vlan : vlans) {
            // add port mapping
            Set<PortVlan> portMappedThis = new HashSet<PortVlan>();
            Set<PortVlan> portMappedOther = new HashSet<PortVlan>();

            for (short i = 0; i < NUM_INTERFACES; i++) {
                int inode = 0;
                for (Node node : existNodes) {
                    short setvlan = (i != 1) ? vlan : 100;
                    String bname = (i < MAX_INTERFACE_VBRIDGE1) ? bname1 : bname2;
                    String ifname = "vinterface" + inode + (i + 10);
                    VBridgePath bpath = (i < MAX_INTERFACE_VBRIDGE1)? bpath1 : bpath2;
                    VBridgeIfPath ifp = new VBridgeIfPath(tname, bname, ifname);
                    SwitchPort port = new SwitchPort(NodeConnector.NodeConnectorIDType.OPENFLOW,
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
                    if (i < MAX_INTERFACE_VBRIDGE1) {
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
                    Set<PortVlan> transmitOtherNode = new HashSet<PortVlan>();
                    Set<PortVlan> notTransmit = new HashSet<PortVlan>();
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

                    for (PortVlan pv : mappedThis) {
                        if (stubObj.getLocalityStatus(pv.getNodeConnector().getNode())
                                ==  ConnectionLocality.NOT_LOCAL) {
                            transmitOtherNode.add(pv);
                        } else if (stubObj.getLocalityStatus(pv.getNodeConnector().getNode())
                                !=  ConnectionLocality.LOCAL){
                            notTransmit.add(pv);
                        }
                    }

                    mappedThis.removeAll(transmitOtherNode);
                    mappedThis.removeAll(notTransmit);

                    testReceiveDataPacketBCLoop(mgr, bpath1, MapType.PORT,
                            mappedThis, transmitOtherNode, stub);

                    st = mgr.flushMacEntries(bpath1);
                    assertEquals(emsg, StatusCode.SUCCESS, st.getCode());

                    testReceiveDataPacketUCLoop(mgr, bpath1, MapType.PORT,
                            mappedThis, transmitOtherNode, stub);

                    st = mgr.flushMacEntries(bpath1);
                    assertEquals(emsg, StatusCode.SUCCESS, st.getCode());

                    testReceiveDataPacketUCLoopLearned(mgr, bpath1, MapType.PORT,
                            mappedThis, transmitOtherNode, stub);

                    st = mgr.flushMacEntries(bpath1);
                    assertEquals(emsg, StatusCode.SUCCESS, st.getCode());

                    st = mgr.removeVlanMap(bpath1, map.getId());
                    assertEquals(emsg, StatusCode.SUCCESS, st.getCode());
                }
            }
        }

        // in case packet received from ISL port.
        byte[] dst = new byte[] {(byte)0xff, (byte)0xff, (byte)0xff,
                                 (byte)0xff, (byte)0xff, (byte)0xff};
        byte[] sender = new byte[] {(byte)192, (byte)168, (byte)0, (byte)1};
        byte[] target = new byte[] {(byte)192, (byte)168, (byte)0, (byte)250};
        for (NodeConnector innc : existConnectors) {
            if (!vtnMgr.isEdgePort(innc)) {
                byte[] src = new byte[] {(byte)0x00, (byte)0x00, (byte)0x00,
                                         (byte)0x11, (byte)0x11, (byte)0x11};

                RawPacket inPkt = createARPRawPacket(src, dst, sender, target,
                                                     (short)-1, innc,
                                                     ARP.REQUEST);

                PacketResult result = mgr.receiveDataPacket(inPkt);
                assertEquals(PacketResult.IGNORED, result);
                List<RawPacket> dataList = stub.getTransmittedDataPacket();
                assertEquals(0, dataList.size());
            }
        }

        ClusterEventMap clsEvents = (ClusterEventMap)
            stubObj.getCache(VTNManagerImpl.CACHE_EVENT);
        checkEventCleared(clsEvents);
    }

    /**
     * common method for test which use BroadCast Packet (ARP Request packet).
     * this method expected that action for received packet is flooding to mapped port.
     */
    private void testReceiveDataPacketBCLoop(VTNManagerImpl mgr, VBridgePath bpath,
           MapType type, Set<PortVlan> mappedThis, Set<PortVlan> transmitOtherNode,
           TestStub stub) {
        testReceiveDataPacketCommonLoop(mgr, bpath, MapType.PORT, mappedThis,
                                        transmitOtherNode, stub,
                                        (short)1, EtherTypes.ARP, null);
    }

    /**
     * common method for test which use UniCast Packet (IPv4 packet).
     * this method expected that action for received packet is flooding to mapped port.
     */
    private void testReceiveDataPacketUCLoop(VTNManagerImpl mgr, VBridgePath bpath,
            MapType type, Set<PortVlan> mappedThis, Set<PortVlan> transmitOtherNode,
            TestStub stub) {
        testReceiveDataPacketCommonLoop(mgr, bpath, MapType.PORT, mappedThis,
                                        transmitOtherNode, stub,
                                        (short)0, EtherTypes.IPv4, null);
    }

    /**
     * common method for
     * {@link #testReceiveDataPacketBCLoop(VTNManagerImpl, VBridgePath, MapType, Set, Set, TestStub)} and
     * {@link #testReceiveDataPacketUCLoop(VTNManagerImpl, VBridgePath, MapType, Set, Set, TestStub)}.
     * this method expected that action for received packet is flooding to mapped port.
     */
    private void testReceiveDataPacketCommonLoop(VTNManagerImpl mgr, VBridgePath bpath,
            MapType type, Set<PortVlan> mappedThis, Set<PortVlan> transmitOtherNode,
            TestStub stub, short bc, EtherTypes ethtype, PortVlan targetPv) {
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

        ClusterEventMap clsEvents = (ClusterEventMap)
            stubObj.getCache(VTNManagerImpl.CACHE_EVENT);

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

                clsEvents.getPostedEvents();

                RawPacket inPkt = null;
                if (ethtype.shortValue() == EtherTypes.IPv4.shortValue()) {
                    inPkt = createIPv4RawPacket(src, dst, sender, target,
                                                    (inVlan > 0) ? inVlan : -1, inNc);
                } else if (ethtype.shortValue() == EtherTypes.ARP.shortValue()){
                    inPkt = createARPRawPacket(src, dst, sender, target,
                            (inVlan > 0) ? inVlan : -1, inNc, ARP.REQUEST);
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
                            } else if (ethtype.shortValue() == EtherTypes.ARP.shortValue()){
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

                    List<ClusterEvent> posted = clsEvents.getPostedEvents();
                    assertEquals(transmitOtherNode.size(), posted.size());
                    for (ClusterEvent ev : posted) {
                        assertTrue(ev instanceof RawPacketEvent);
                    }

                } else {
                    if (inNc != null) {
                        assertEquals(inNc.toString() + "," + ea.toString(),
                                     PacketResult.IGNORED, result);
                    } else {
                        assertEquals(ea.toString(), PacketResult.IGNORED, result);
                    }

                    assertEquals(0, clsEvents.getPostedEvents().size());
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
           MapType type, Set<PortVlan> mappedThis, Set<PortVlan> transmitOtherNode,
           TestStub stub) {
        testReceiveDataPacketCommonLoopLearned(mgr, bpath, MapType.PORT, mappedThis,
                                               transmitOtherNode, stub, (short)1,
                                               EtherTypes.ARP);
    }

    /**
     * common method for test which use UniCast Packet (IPv4 packet).
     * this method expected that action for received packet is output to learned port.
     */
    private void testReceiveDataPacketUCLoopLearned(VTNManagerImpl mgr, VBridgePath bpath,
            MapType type, Set<PortVlan> mappedThis, Set<PortVlan> transmitOtherNode,
            TestStub stub) {
        testReceiveDataPacketCommonLoopLearned(mgr, bpath, MapType.PORT, mappedThis,
                                               transmitOtherNode, stub, (short)0,
                                               EtherTypes.IPv4);
    }

    /**
     * common method for
     * {@link #testReceiveDataPacketBCLoop(VTNManagerImpl, VBridgePath, MapType, Set, Set, TestStub)} and
     * {@link #testReceiveDataPacketUCLoop(VTNManagerImpl, VBridgePath, MapType, Set, Set, TestStub)}.
     * this method expected that action for received packet is a port host is connected to.
     *
     */
    private void testReceiveDataPacketCommonLoopLearned(VTNManagerImpl mgr, VBridgePath bpath,
            MapType type, Set<PortVlan> mappedThis, Set<PortVlan> transmitOtherNode,
            TestStub stub, short bc, EtherTypes ethtype) {
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
                    transmitOtherNode, stub, (short)0, EtherTypes.IPv4, learnedPv);

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
                    } else if (ethtype.shortValue() == EtherTypes.ARP.shortValue()){
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

    /**
     * Ensure that all events were removed by timer thread.
     *
     * @param map  A cluster cache which keeps cluster events.
     */
    private void checkEventCleared(ClusterEventMap map) {
        try {
            map.waitForCleared(10000);
        } catch (Exception e) {
            unexpected(e);
        }
    }
}
