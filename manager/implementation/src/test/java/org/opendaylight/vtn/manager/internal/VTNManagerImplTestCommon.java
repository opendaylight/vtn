/*
 * Copyright (c) 2013-2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal;

import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.concurrent.ConcurrentMap;

import org.opendaylight.vtn.manager.IVTNManager;
import org.opendaylight.vtn.manager.MacAddressEntry;
import org.opendaylight.vtn.manager.MacMap;
import org.opendaylight.vtn.manager.MacMapConfig;
import org.opendaylight.vtn.manager.PortMap;
import org.opendaylight.vtn.manager.PortMapConfig;
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
import org.opendaylight.vtn.manager.internal.cluster.FlowGroupId;
import org.opendaylight.vtn.manager.internal.cluster.FlowModResult;
import org.opendaylight.vtn.manager.internal.cluster.PortVlan;
import org.opendaylight.vtn.manager.internal.cluster.VTNFlow;
import org.opendaylight.vtn.manager.internal.cluster.VlanMapPath;

import org.opendaylight.controller.forwardingrulesmanager.FlowEntry;
import org.opendaylight.controller.sal.action.Action;
import org.opendaylight.controller.sal.action.Output;
import org.opendaylight.controller.sal.action.SetVlanId;
import org.opendaylight.controller.sal.core.Node;
import org.opendaylight.controller.sal.core.NodeConnector;
import org.opendaylight.controller.sal.match.Match;
import org.opendaylight.controller.sal.match.MatchField;
import org.opendaylight.controller.sal.match.MatchType;
import org.opendaylight.controller.sal.packet.ARP;
import org.opendaylight.controller.sal.utils.NodeConnectorCreator;
import org.opendaylight.controller.sal.utils.Status;
import org.opendaylight.controller.sal.utils.StatusCode;

/**
 * Common class for tests of {@link VTNManagerImpl}.
 */
public class VTNManagerImplTestCommon extends TestUseVTNManagerBase {
    /**
     * Construct a new instance.
     *
     * @param stub  An integer value to be passed to {@link TestStub}.
     */
    protected VTNManagerImplTestCommon(int stub) {
        super(stub);
    }

    /**
     * Construct a new instance.
     *
     * @param stub  An integer value to be passed to {@link TestStub}.
     * @param ht    If {@code true}, use host tracker emulator.
     */
    protected VTNManagerImplTestCommon(int stub, boolean ht) {
        super(stub, ht);
    }

    /**
     * method for setup the environment.
     * create 1 Tenant and bridges
     *
     * @param mgr       VTNManager Service.
     * @param tpath     A {@link VTenantPath} to be created.
     * @param bpaths    A list of {@link VBridgePath} to be created.
     */
    protected void createTenantAndBridge(IVTNManager mgr, VTenantPath tpath,
            List<VBridgePath> bpaths) {

        Status st = mgr.addTenant(tpath, new VTenantConfig(null));
        assertEquals(StatusCode.SUCCESS, st.getCode());
        assertTrue(mgr.isActive());

        for (VBridgePath bpath : bpaths) {
            st = mgr.addBridge(bpath, new VBridgeConfig(null));
            assertEquals("(VBridgePath)" + bpath.toString(), StatusCode.SUCCESS, st.getCode());
        }
    }

    /**
     * method for setup the environment.
     * create 1 Tenant and bridges and vInterfaces
     *
     * @param mgr       VTNManager Service.
     * @param tpath     A {@link VTenantPath} to be created.
     * @param bpaths    A list of {@link VBridgePath} to be created.
     * @param ifpaths   A list of {@link VBridgeIfPath} to be created.
     */
    protected void createTenantAndBridgeAndInterface(IVTNManager mgr, VTenantPath tpath,
            List<VBridgePath> bpaths, List<VBridgeIfPath> ifpaths) {

        Status st = mgr.addTenant(tpath, new VTenantConfig(null));
        assertEquals(StatusCode.SUCCESS, st.getCode());
        assertTrue(mgr.isActive());

        for (VBridgePath bpath : bpaths) {
            st = mgr.addBridge(bpath, new VBridgeConfig(null));
            assertEquals("(VBridgePath)" + bpath.toString(), StatusCode.SUCCESS, st.getCode());
        }

        for (VBridgeIfPath ifpath : ifpaths) {
            VInterfaceConfig ifconf = new VInterfaceConfig(null, null);
            st = mgr.addBridgeInterface(ifpath, ifconf);
            assertEquals("(VBridgeIfPath)" + ifpath.toString(), StatusCode.SUCCESS, st.getCode());
        }
    }

    /**
     * check VTN configuration.
     * note: this don't support a configuration which VBridge &gt; 1
     *
     * @param tpath     A {@link VTenantPath} expected to exist.
     * @param bpathlist A set of {@link VBridgePath} expected to exist.
     * @param pmaps     A map between {@link VBridgeIfPath} expected to exist
     *                  and {@link PortMapConfig}.
     * @param vmaps     A map between {@link VlanMap} expected to exist and
     *                  {@link VlanMapConfig}.
     * @param mcconf    A {@link MacMapConfig} instance expected to be
     *                  configured in the specified vBridge.
     */
    protected void checkVTNconfig(VTNManagerImpl mgr, VTenantPath tpath,
                                  List<VBridgePath> bpathlist,
                                  Map<VBridgeIfPath, PortMapConfig> pmaps,
                                  Map<VlanMap, VlanMapConfig> vmaps,
                                  MacMapConfig mcconf) {
        VBridgePath bpath = bpathlist.get(0);

        List<VTenant> tlist = null;
        List<VBridge> blist = null;
        List<VInterface> iflist = null;
        try {
            tlist = mgr.getTenants();
            blist = mgr.getBridges(tpath);
            iflist = mgr.getBridgeInterfaces(bpath);
        } catch (VTNException e) {
            unexpected(e);
        }

        assertNotNull(tlist);
        assertNotNull(blist);
        assertEquals(1, tlist.size());
        assertEquals(tpath.getTenantName(), tlist.get(0).getName());
        assertEquals(1, blist.size());
        assertEquals(bpath.getBridgeName(), blist.get(0).getName());
        assertEquals(pmaps.size(), iflist.size());

        for (VInterface vif : iflist) {
            VBridgeIfPath bif = new VBridgeIfPath(tpath.getTenantName(),
                    bpath.getBridgeName(), vif.getName());
            assertTrue(bif.toString(), pmaps.containsKey(bif));
        }

        if (pmaps != null) {
            for (Map.Entry<VBridgeIfPath, PortMapConfig> ent : pmaps.entrySet()) {
                PortMap pmap = null;
                try {
                    pmap = mgr.getPortMap(ent.getKey());
                } catch (VTNException e) {
                    unexpected(e);
                }
                if (ent.getValue() != null) {
                    assertEquals(ent.getValue(), pmap.getConfig());
                }
            }
        }

        if (vmaps != null) {
            for (Map.Entry<VlanMap, VlanMapConfig> ent : vmaps.entrySet()) {
                VlanMap vmap = null;
                try {
                    vmap = mgr.getVlanMap(bpath, ent.getKey().getId());
                } catch (VTNException e) {
                    unexpected(e);
                }
                if (ent.getValue() != null) {
                    assertEquals(ent.getValue().getVlan(), vmap.getVlan());
                    assertEquals(ent.getValue().getNode(), vmap.getNode());
                }
            }
        }

        try {
            MacMap mcmap = mgr.getMacMap(bpath);
            if (mcconf == null) {
                assertNull(mcmap);
            } else if (!mcconf.equals(mcmap)) {
                fail("Unexpected MAC mapping: mcconf=" + mcconf +
                     ", mcmap=" + mcmap);
            }
        } catch (VTNException e) {
            unexpected(e);
        }
    }

    /**
     * Check flow entries.
     *
     * @param learnedPv     Learned {@link PortVlan} of target host.
     * @param inPortVlan    Ingress {@link PortVlan}.
     * @param flowEntries   A Set of {@link FlowEntry} which was installed.
     * @param registeredFlows   Set of string of {@link FlowEntry} which is
     *                          already registered before.
     *                          Registered flows are added to this.
     * @param src           A source MAC address.
     * @param dst           A destination MAC address.
     * @param tenant        A {@link VTenant} installed flows.
     * @param emsg          Error message.
     */
    protected void checkFlowEntries(PortVlan learnedPv, PortVlan inPortVlan,
            Set<FlowEntry> flowEntries, Set<String> registeredFlows,
            byte[] src, byte[] dst, VTenant tenant,
            String emsg) {
        NodeConnector learnedNc = learnedPv.getNodeConnector();
        Node learnedNode = learnedNc.getNode();
        Node inNode = inPortVlan.getNodeConnector().getNode();

        NodeConnector edgePort = NodeConnectorCreator.
            createOFNodeConnector(Short.valueOf((short)15), inNode);
        for (FlowEntry ent : flowEntries) {
            if (!registeredFlows.add(ent.getFlowName())) {
                continue;
            }

            // check Actions.
            if (ent.getFlowName().endsWith("-0")) {
                assertEquals(inNode, ent.getNode());
                for (Action act : ent.getFlow().getActions()) {
                    switch (act.getType()) {
                    case OUTPUT:
                        Output out = (Output)act;
                        if (learnedNode.equals(inNode)) {
                            assertEquals(learnedNc, out.getPort());
                        } else {
                            assertEquals(emsg, edgePort, out.getPort());
                        }
                        break;
                    case SET_VLAN_ID:
                        if (learnedNode.equals(inNode)) {
                            SetVlanId setVlan = (SetVlanId)act;
                            assertEquals(setVlan.getVlanId(), learnedPv.getVlan());
                        } else {
                            fail("in this case SET_VLAN_ID is not set.");
                        }
                        break;
                    case POP_VLAN:
                        if (learnedNode.equals(inNode)) {
                            assertTrue(learnedPv.getVlan() <= 0);
                        } else {
                            fail("in this case POP_VLAN is not set.");
                        }
                        break;
                    default :
                        fail("unexpected action type." + act.toString());
                        break;
                    }
                }
            } else if (ent.getFlowName().endsWith("-1")) {
                assertEquals(learnedNode, ent.getNode());
                for (Action act : ent.getFlow().getActions()) {
                    switch (act.getType()) {
                    case OUTPUT:
                        Output out = (Output)act;
                        assertEquals(learnedNc, out.getPort());
                        break;
                    case SET_VLAN_ID:
                        SetVlanId setVlan = (SetVlanId)act;
                        assertEquals(setVlan.getVlanId(), learnedPv.getVlan());
                        break;
                    case POP_VLAN:
                        assertTrue(learnedPv.getVlan() <= 0);
                        break;
                    default :
                        fail("unexpected action type." + act.toString());
                        break;
                    }
                }
            } else {
                fail("not supported case.");
            }

            NodeConnector inEdgePort = NodeConnectorCreator.
                createOFNodeConnector(Short.valueOf((short)15), learnedNode);

            // check Match.
            Match match = ent.getFlow().getMatch();
            for (MatchType mtype : match.getMatchesList()) {
                MatchField field = match.getField(mtype);
                switch (mtype) {
                case DL_SRC:
                    assertArrayEquals(src, (byte[])field.getValue());
                    break;
                case DL_DST:
                    assertArrayEquals(dst, (byte[])field.getValue());
                    break;
                case DL_VLAN:
                    assertEquals(inPortVlan.getVlan(),
                                 field.getValue());
                    break;
                case IN_PORT:
                    if (ent.getFlowName().endsWith("-0")) {
                        assertEquals(inPortVlan.getNodeConnector(),
                                     (NodeConnector)field.getValue());
                    } else {
                        assertEquals(inEdgePort,
                                     (NodeConnector)field.getValue());
                    }
                    break;
                default:
                    fail("unexpected match type." + match.toString());
                    break;
                }
            }

            // check priority.
            int pri = vtnMgr.getVTNConfig().getL2FlowPriority();
            assertEquals(pri, ent.getFlow().getPriority());

            // check setting of timeout.
            if (ent.getFlowName().endsWith("-0")) {
                assertEquals(tenant.getHardTimeout(), ent.getFlow().getHardTimeout());
                assertEquals(tenant.getIdleTimeout(), ent.getFlow().getIdleTimeout());
            } else {
                assertEquals(0, ent.getFlow().getHardTimeout());
                assertEquals(0, ent.getFlow().getIdleTimeout());
            }
        }
    }

    /**
     * Expire flow entries.
     *
     * @param mgr    VTN Manager service.
     * @param stub   Stub for OSGi service.
     */
    protected void expireFlows(VTNManagerImpl mgr, TestStub stub) {
        // Wait for all flow modifications to complete.
        NopFlowTask task = new NopFlowTask(mgr);
        mgr.postFlowTask(task);
        assertSame(FlowModResult.SUCCEEDED, task.getResult(3000));

        Set<FlowEntry> flows = stub.getFlowEntries();
        if (!flows.isEmpty()) {
            for (FlowEntry fent : flows) {
                String flowName = fent.getFlowName();
                if (flowName.endsWith("-0")) {
                    Status status = stub.uninstallFlowEntry(fent);
                    assertEquals("(FlowEntry)" + fent.toString(), StatusCode.SUCCESS,
                            status.getCode());
                    mgr.flowRemoved(fent.getNode(), fent.getFlow());
                }
            }

            // Wait for all flow entries to be removed.
            task = new NopFlowTask(mgr);
            mgr.postFlowTask(task);
            assertSame(FlowModResult.SUCCEEDED, task.getResult(3000));
            assertSame(0, stub.getFlowEntries().size());
        }
    }

    /**
     * Check node and interface status specified vbridge and vinterface.
     * @param mgr       VTN Manager service.
     * @param bpath     A {@link VBridgePath} which is checked.
     * @param ifp       A {@link VBridgeIfPath} which is checked.
     * @param bstate    A state of {@link VBridge} which is expected.
     * @param ifstate   A state of {@link VInterface} which is expected.
     * @param msg       message strings print when assertion failed.
     */
    protected void checkNodeStatus(VTNManagerImpl mgr, VBridgePath bpath, VBridgeIfPath ifp,
            VNodeState bstate, VNodeState ifstate, String msg) {

        VBridge brdg = null;
        VInterface bif = null;
        try {
            if (ifp != null) {
                bif = mgr.getBridgeInterface(ifp);
            }
            if (bpath != null) {
                brdg = mgr.getBridge(bpath);
            }
        } catch (VTNException e) {
            unexpected(e);
        }

        if (ifp != null) {
            assertEquals("VBridgeInterface status: " + msg, ifstate, bif.getState());
        }
        if (bpath != null) {
            assertEquals("VBridge status: " + msg, bstate, brdg.getState());
        }
    }

    /**
     * Put a MAC Address Table Entry to MAC Address Table of specified bridge.
     *
     * @param mgr   VTN Manager service.
     * @param path  A path to the virtual network mapping which maps the
     *              MAC address table entry.
     * @param nc    Incoming NodeConnector.
     */
    protected void putMacTableEntry(VTNManagerImpl mgr, VBridgePath path,
                                    NodeConnector nc) {
        byte[] src = new byte[] {(byte)0x00, (byte)0x01, (byte)0x01,
                                 (byte)0x01, (byte)0x01, (byte)0x01};
        byte[] dst = new byte[] {(byte)0xFF, (byte)0xFF, (byte)0xFF,
                                 (byte)0xFF, (byte)0xFF, (byte)0xFF};
        byte[] sender = new byte[] {(byte)192, (byte)168, (byte)0, (byte)1};
        byte[] target = new byte[] {(byte)192, (byte)168, (byte)0, (byte)250};

        PacketContext pctx = createARPPacketContext(src, dst, sender, target,
                                                    (short)-1, nc, ARP.REQUEST);
        MacAddressTable tbl = mgr.getMacAddressTable(path);
        TestBridgeNode bnode = new TestBridgeNode(path);
        tbl.add(pctx, bnode);
    }

    /**
     * Check a MAC Address Table Entry.
     *
     * @param mgr       VTN Manager service.
     * @param bpath     {@link VBridgePath} of checked {@link VBridge}.
     * @param isFlushed If {@code true}, a expected result is 0.
     *                  If {@code false}, expected result is more than 0.
     * @param msg       error message.
     */
    protected void checkMacTableEntry(VTNManagerImpl mgr, VBridgePath bpath,
                                      boolean isFlushed, String msg) {
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
     * Flush MAC Table
     * @param mgr   VTN Manager service.
     * @param bpath {@link VBridgePath} of {@link VBridge} which is flushed.
     */
    protected void flushMacTableEntry(VTNManagerImpl mgr, VBridgePath bpath) {
        mgr.flushMacEntries(bpath);
    }

    /**
     * Put flow entry
     */
    protected VTNFlow putFlowEntry(VTNManagerImpl mgr, String tenantName,
                              NodeConnector innc, NodeConnector outnc) {
        VTNFlowDatabase fdb = mgr.getTenantFlowDB(tenantName);
        VTNFlow flow = fdb.create(mgr);

        flow = addFlowEntry(vtnMgr, flow, innc, (short)1, outnc, 1);
        fdb.install(mgr, flow);
        flushFlowTasks();
        return flow;
    }

    /**
     * Check flow entry.
     */
    protected boolean checkFlowEntry(VTNManagerImpl mgr, VTNFlow flow) {
        ConcurrentMap<FlowGroupId, VTNFlow> flowDB = mgr.getFlowDB();
        return flowDB.containsValue(flow);
    }

    /**
     * Create a {@link VlanMapPath} instance from the VLAN mapping
     * configuration.
     *
     * @param path    A path to the vBridge.
     * @param vlconf  VLAN mapping configuration.
     */
    protected VlanMapPath createVlanMapPath(VBridgePath path,
                                            VlanMapConfig vlconf) {
        Node node = vlconf.getNode();
        short vlan = vlconf.getVlan();
        StringBuilder builder = new StringBuilder();
        if (node == null) {
            builder.append("ANY");
        } else {
            builder.append(node.getType()).append('-').
                append(node.getNodeIDString());
        }
        builder.append('.').append((int)vlan);

        return new VlanMapPath(path, builder.toString());
    }

    /**
     * Create a {@link VBridgePath} which points to the specified virtual
     * mapping.
     *
     * @param path    A path to the vBridge.
     * @param ifpath  A path to the virtual interface which has the port
     *                mapping configuration.
     * @param pmconf  Port mapping configuration.
     * @param vlconf  VLAN mapping configuration.
     */
    protected VBridgePath createMapPath(VBridgePath path, VBridgeIfPath ifpath,
                                        PortMapConfig pmconf,
                                        VlanMapConfig vlconf) {
        if (pmconf != null) {
            return ifpath;
        } else if (vlconf != null) {
            return createVlanMapPath(path, vlconf);
        }

        return path;
    }
}
