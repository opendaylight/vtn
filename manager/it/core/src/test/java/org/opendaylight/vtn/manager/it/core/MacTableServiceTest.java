/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.core;

import static org.opendaylight.vtn.manager.it.ofmock.OfMockService.DEFAULT_TABLE;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.Map.Entry;
import java.util.Map;
import java.util.Set;

import org.opendaylight.vtn.manager.util.EtherAddress;
import org.opendaylight.vtn.manager.util.IpNetwork;

import org.opendaylight.vtn.manager.it.ofmock.OfMockFlow;
import org.opendaylight.vtn.manager.it.ofmock.OfMockService;
import org.opendaylight.vtn.manager.it.util.BridgeNetwork;
import org.opendaylight.vtn.manager.it.util.RpcErrorTag;
import org.opendaylight.vtn.manager.it.util.TestHost;
import org.opendaylight.vtn.manager.it.util.VirtualNetwork;
import org.opendaylight.vtn.manager.it.util.unicast.UnicastFlow;
import org.opendaylight.vtn.manager.it.util.vnode.VBridgeConfig;
import org.opendaylight.vtn.manager.it.util.vnode.VBridgeIdentifier;
import org.opendaylight.vtn.manager.it.util.vnode.VTNVlanMapConfig;
import org.opendaylight.vtn.manager.it.util.vnode.mac.MacEntry;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeState;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnErrorTag;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.mac.rev150907.RemoveMacEntryInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.mac.rev150907.RemoveMacEntryInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.mac.rev150907.RemoveMacEntryOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.mac.rev150907.VtnMacTableService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.mac.rev150907.remove.mac.entry.output.RemoveMacEntryResult;

import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.rev131026.flow.Match;

import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.yang.types.rev100924.MacAddress;

/**
 * Test case for {@link VtnMacTableService}.
 */
public final class MacTableServiceTest extends TestMethodBase {
    /**
     * Construct a new instance.
     *
     * @param vit  A {@link VTNManagerIT} instance.
     */
    public MacTableServiceTest(VTNManagerIT vit) {
        super(vit);
    }

    // TestMethodBase

    /**
     * Run the test.
     *
     * @throws Exception  An error occurred.
     */
    @Override
    protected void runTest() throws Exception {
        VTNManagerIT vit = getTest();
        VtnMacTableService macSrv = vit.getMacTableService();
        OfMockService ofmock = vit.getOfMockService();

        // Create one VTN and 2 vBridges.
        String tname = "vtn";
        String bname1 = "vbr1";
        String bname2 = "vbr2";
        VBridgeIdentifier vbrId1 = new VBridgeIdentifier(tname, bname1);
        VBridgeIdentifier vbrId2 = new VBridgeIdentifier(tname, bname2);
        MacEntryWaiter macWaiter1 = new MacEntryWaiter(ofmock, vbrId1);
        MacEntryWaiter macWaiter2 = new MacEntryWaiter(ofmock, vbrId2);
        macWaiter1.await();
        macWaiter2.await();

        VBridgeIdentifier[] vbrIds = {vbrId1, vbrId2};
        VirtualNetwork vnet = getVirtualNetwork().
            addBridge(vbrIds).
            apply();

        Set<MacEntry> empty = Collections.<MacEntry>emptySet();
        macWaiter1.set(empty).await();
        macWaiter2.set(empty).await();

        // Try to clearn empty MAC address table.
        List<MacAddress> emptyAddr = Collections.<MacAddress>emptyList();
        assertEquals(null, clearMacEntry(macSrv, vbrId1));
        assertEquals(null, removeMacEntry(macSrv, vbrId2, emptyAddr));

        // Map VLAN 0, 1 and 2, 3 to bpath1 and bpath2 respectively.
        int vid = 0;
        for (VBridgeIdentifier vbrId: vbrIds) {
            for (int i = 0; i < 2; i++) {
                VBridgeConfig bconf = vnet.getBridge(vbrId);
                VTNVlanMapConfig vmap = new VTNVlanMapConfig(vid);
                bconf.addVlanMap(vmap);
                vid++;
            }

            vnet.apply().verify();
            VNodeStateWaiter waiter = new VNodeStateWaiter(ofmock).
                set(vbrId, VnodeState.UP);
            waiter.await();
        }

        // Collect edge ports, and create test hosts.
        BridgeNetwork bridge1 = new BridgeNetwork(ofmock, vbrId1);
        BridgeNetwork bridge2 = new BridgeNetwork(ofmock, vbrId2);
        Set<MacEntry> ments1 = new HashSet<>();
        Set<MacEntry> ments2 = new HashSet<>();
        Set<String> islPorts = new HashSet<>();
        int idx = 1;
        final int nhosts = 2;
        for (String nid: ofmock.getNodes()) {
            for (String pid: ofmock.getPorts(nid, true)) {
                String pname = ofmock.getPortName(pid);
                for (vid = 0; vid <= 1; vid++) {
                    for (int i = 0; i < nhosts; i++) {
                        TestHost th = new TestHost(idx, pid, pname, vid);
                        bridge1.addHost(nid, th);
                        assertTrue(ments1.add(th.getMacEntry()));
                        idx++;
                    }
                }

                Set<Short> vids = new HashSet<>();
                for (vid = 2; vid <= 3; vid++) {
                    for (int i = 0; i < nhosts; i++) {
                        TestHost th = new TestHost(idx, pid, pname, vid);
                        bridge2.addHost(nid, th);
                        assertTrue(ments2.add(th.getMacEntry()));
                        idx++;
                    }
                }
            }

            for (String pid: ofmock.getPorts(nid, false)) {
                bridge1.setUnmappedPort(pid);
                bridge2.setUnmappedPort(pid);
                assertTrue(islPorts.add(pid));
            }
        }
        bridge1.verify();
        bridge2.verify();
        assertFalse(islPorts.isEmpty());

        macWaiter1.await();
        macWaiter2.await();

        // Let the vBridge at bpath1 learn MAC addresses.
        learnHosts(bridge1);
        macWaiter1.set(ments1).await();
        macWaiter2.await();

        // Let the vBridge at bpath2 learn MAC addresses.
        learnHosts(bridge2);
        macWaiter1.await();
        macWaiter2.set(ments2).await();

        // Error tests for remove-mac-entry RPC.

        // Null input.
        checkRpcError(macSrv.removeMacEntry(null),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);

        // Null tenant-name.
        RemoveMacEntryInput input = new RemoveMacEntryInputBuilder().build();
        checkRpcError(macSrv.removeMacEntry(input),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);

        input = new RemoveMacEntryInputBuilder().
            setBridgeName(bname1).
            build();
        checkRpcError(macSrv.removeMacEntry(input),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);

        // Null bridge-name.
        input = new RemoveMacEntryInputBuilder().
            setTenantName(tname).
            build();
        checkRpcError(macSrv.removeMacEntry(input),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);

        for (String name: INVALID_VNODE_NAMES) {
            // Invalid tenant-name.
            input = new RemoveMacEntryInputBuilder().
                setTenantName(name).
                setBridgeName(bname1).
                build();
            checkRpcError(macSrv.removeMacEntry(input),
                          RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);

            // Invalid bridge-name.
            input = new RemoveMacEntryInputBuilder().
                setTenantName(tname).
                setBridgeName(name).
                build();
            checkRpcError(macSrv.removeMacEntry(input),
                          RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);
        }

        // null in mac-addresses.
        EtherAddress unknown1 = new EtherAddress(0xa0b0c0d0e0f0L);
        EtherAddress unknown2 = new EtherAddress(0xfeff123456e8L);
        input = newRemoveMacEntryInput(vbrId1, unknown1, null, unknown2);
        checkRpcError(macSrv.removeMacEntry(input),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);

        // Specifying vBridge that is not present.
        String unknownName = "unknown";
        VBridgeIdentifier[] unknownVbrIds = {
            new VBridgeIdentifier(unknownName, bname1),
            new VBridgeIdentifier(unknownName, bname2),
            new VBridgeIdentifier(tname, unknownName),
        };
        for (VBridgeIdentifier vbrId: unknownVbrIds) {
            input = newRemoveMacEntryInput(vbrId, unknown1, unknown2);
            checkRpcError(macSrv.removeMacEntry(input),
                          RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);
        }

        // Errors should never affect MAC address tables.
        macWaiter1.await();
        macWaiter2.await();

        // Install unicast flow entries.
        List<UnicastFlow> flows1 = unicastTest(bridge1, islPorts, true);
        List<UnicastFlow> flows2 = unicastTest(bridge2, islPorts, true);

        // Move all hosts on untagged network to VLAN 1.
        Map<String, List<TestHost>> edgeMap1 = bridge1.getTestHosts();
        Map<String, List<TestHost>> edgeMap2 = bridge2.getTestHosts();
        Map<TestHost, TestHost> oldHosts0 = new HashMap<>();
        for (Entry<String, List<TestHost>> entry: edgeMap1.entrySet()) {
            List<TestHost> hosts = new ArrayList<>();
            for (TestHost th: entry.getValue()) {
                if (th.getVlanId() == 0) {
                    TestHost host = new TestHost(th, 1);
                    hosts.add(host);
                    assertEquals(null, oldHosts0.put(th, host));
                } else {
                    hosts.add(th);
                }
            }
            entry.setValue(hosts);
        }

        macWaiter1.await();
        macWaiter2.await();

        // Trying to remove MAC addresses that are not learned.
        Map<MacAddress, VtnUpdateType> result = new HashMap<>();
        result.put(unknown1.getMacAddress(), null);
        result.put(unknown2.getMacAddress(), null);
        for (VBridgeIdentifier vbrId: vbrIds) {
            assertEquals(result,
                         removeMacEntry(macSrv, vbrId, unknown1, unknown2));
        }
        macWaiter1.await();
        macWaiter2.await();

        // Send ICMP packet from moved hosts.
        // This invalidates old MAC address table entries.
        IpNetwork dstIp = IpNetwork.create("192.168.100.255");
        for (Entry<TestHost, TestHost> entry: oldHosts0.entrySet()) {
            TestHost oldHost = entry.getKey();
            TestHost newHost = entry.getValue();
            assertTrue(ments1.remove(oldHost.getMacEntry()));
            sendBroadcastIcmp(newHost, dstIp, bridge1.getMappedVlans());

            // IP address in ICMP packet should not be copied into MAC address
            // table entry.
            MacEntry ment = newHost.getMacEntry(false);
            assertTrue(ments1.add(ment));
            macWaiter1.add(ment).await();
        }

        // All flow entries for an entry should be uninstalled.
        final int tableId = DEFAULT_TABLE;
        for (UnicastFlow unicast: flows1) {
            for (OfMockFlow flow: unicast.getFlowList()) {
                Match match = flow.getMatch();
                if (getVlanMatch(match) == 0) {
                    String nid = flow.getNodeIdentifier();
                    int pri = flow.getPriority();
                    OfMockFlow newFlow = ofmock.
                        awaitFlow(nid, tableId, match, pri, false);
                    assertEquals(null, newFlow);
                }
            }
        }

        // bpath2 should not be affected.
        macWaiter2.await();
        UnicastFlow.verifyFlows(flows2, true, false);

        // Remove 2 MAC addresses from vbrId1.
        int removed = 0;
        for (Iterator<MacEntry> it = ments1.iterator(); it.hasNext();) {
            EtherAddress eaddr = it.next().getMacAddress();
            MacAddress mac = eaddr.getMacAddress();
            it.remove();
            result.clear();
            assertEquals(null, result.put(mac, VtnUpdateType.REMOVED));

            // Duplicate address should be ignored.
            assertEquals(result, removeMacEntry(macSrv, vbrId1, eaddr, eaddr));
            macWaiter1.remove(eaddr).await();

            removed++;
            if (removed == 2) {
                break;
            }
        }

        // Remove all the MAC addresses learned by vbrId1.
        result.clear();
        List<MacAddress> addrs = new ArrayList<>(ments1.size());
        for (MacEntry ment: ments1) {
            EtherAddress eaddr = ment.getMacAddress();
            MacAddress mac = eaddr.getMacAddress();
            addrs.add(mac);
            macWaiter1.remove(eaddr);
            assertEquals(null, result.put(mac, VtnUpdateType.REMOVED));
        }
        assertEquals(result, removeMacEntry(macSrv, vbrId1, addrs));
        macWaiter1.await();
        macWaiter2.await();

        // Purge MAC addresses learned by vbrId2.
        result.clear();
        for (MacEntry ment: ments2) {
            EtherAddress eaddr = ment.getMacAddress();
            MacAddress mac = eaddr.getMacAddress();
            macWaiter2.remove(eaddr);
            assertEquals(null, result.put(mac, VtnUpdateType.REMOVED));
        }
        assertEquals(result, clearMacEntry(macSrv, vbrId2));
        macWaiter1.await();
        macWaiter2.await();

        assertEquals(null, clearMacEntry(macSrv, vbrId1));
        assertEquals(null, clearMacEntry(macSrv, vbrId2));

        // Remove vbrId1.
        removeVbridge(vbrId1);
        macWaiter1.set(null).await();
        macWaiter2.await();
        vnet.removeBridge(vbrId1).verify();

        // Remove VTN.
        removeVtn(tname);
        macWaiter1.await();
        macWaiter2.set(null).await();
        vnet.removeTenant(tname).verify();
    }


    /**
     * Create a new input builder for remove-mac-entry RPC.
     *
     * @param vbrId  The identifier for the target vBridge.
     * @return  A {@link RemoveMacEntryInputBuilder} instance.
     */
    private RemoveMacEntryInputBuilder newRemoveMacEntryInputBuilder(
        VBridgeIdentifier vbrId) {
        return new RemoveMacEntryInputBuilder().
            setTenantName(vbrId.getTenantNameString()).
            setBridgeName(vbrId.getBridgeNameString());
    }

    /**
     * Create a new input for remove-mac-entry RPC.
     *
     * @param vbrId   The identifier for the target vBridge.
     * @param eaddrs  An array of {@link EtherAddress} instances.
     * @return  A {@link RemoveMacEntryInput} instance.
     */
    private RemoveMacEntryInput newRemoveMacEntryInput(
        VBridgeIdentifier vbrId, EtherAddress ... eaddrs) {
        List<MacAddress> addrs;
        if (eaddrs == null) {
            addrs = null;
        } else {
            addrs = new ArrayList<>(eaddrs.length);
            for (EtherAddress eaddr: eaddrs) {
                MacAddress mac = (eaddr == null)
                    ? null : eaddr.getMacAddress();
                addrs.add(mac);
            }
        }

        return newRemoveMacEntryInputBuilder(vbrId).
            setMacAddresses(addrs).
            build();
    }

    /**
     * Remove the given MAC addresses from the MAC address table entry.
     *
     * @param macSrv  The vtn-mac-table service.
     * @param vbrId   The identifier for the target vBridge.
     * @param eaddrs  An array of {@link EtherAddress} instances.
     * @return  A map that contains the output of the remove-mac-entry RPC.
     */
    private Map<MacAddress, VtnUpdateType> removeMacEntry(
        VtnMacTableService macSrv, VBridgeIdentifier vbrId,
        EtherAddress ... eaddrs) {
        RemoveMacEntryInput input = newRemoveMacEntryInput(vbrId, eaddrs);
        return removeMacEntry(macSrv, input);
    }

    /**
     * Remove the given MAC addresses from the MAC address table entry.
     *
     * @param macSrv  The vtn-mac-table service.
     * @param vbrId   The identifier for the target vBridge.
     * @param addrs   A list of {@link MacAddress} instances.
     * @return  A map that contains the output of the remove-mac-entry RPC.
     */
    private Map<MacAddress, VtnUpdateType> removeMacEntry(
        VtnMacTableService macSrv, VBridgeIdentifier vbrId,
        List<MacAddress> addrs) {
        RemoveMacEntryInput input = newRemoveMacEntryInputBuilder(vbrId).
            setMacAddresses(addrs).
            build();
        return removeMacEntry(macSrv, input);
    }

    /**
     * Remove all the MAC addresses in the MAC address table.
     *
     * @param macSrv  The vtn-mac-table service.
     * @param vbrId   The identifier for the target vBridge.
     * @return  A map that contains the output of the remove-mac-entry RPC.
     */
    private Map<MacAddress, VtnUpdateType> clearMacEntry(
        VtnMacTableService macSrv, VBridgeIdentifier vbrId) {
        RemoveMacEntryInput input = newRemoveMacEntryInputBuilder(vbrId).
            build();
        return removeMacEntry(macSrv, input);
    }

    /**
     * Remove the MAC addresses specified by the given RPC input.
     *
     * @param macSrv  The vtn-mac-table service.
     * @param input   A {@link RemoveMacEntryInput} instance.
     * @return  A map that contains the output of the remove-mac-entry RPC.
     */
    private Map<MacAddress, VtnUpdateType> removeMacEntry(
        VtnMacTableService macSrv, RemoveMacEntryInput input) {
        RemoveMacEntryOutput output =
            getRpcOutput(macSrv.removeMacEntry(input));

        Map<MacAddress, VtnUpdateType> result;
        List<RemoveMacEntryResult> res = output.getRemoveMacEntryResult();
        if (res == null) {
            result = null;
        } else {
            result = new HashMap<>();
            for (RemoveMacEntryResult rmres: res) {
                MacAddress mac = rmres.getMacAddress();
                VtnUpdateType status = rmres.getStatus();
                assertEquals(null, result.put(mac, status));
            }
        }

        return result;
    }
}
