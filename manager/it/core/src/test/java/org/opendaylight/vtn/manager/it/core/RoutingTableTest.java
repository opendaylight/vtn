/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.core;

import java.util.Collections;
import java.util.HashSet;
import java.util.List;
import java.util.Map.Entry;
import java.util.Map;
import java.util.Set;

import org.opendaylight.vtn.manager.it.ofmock.OfMockLink;
import org.opendaylight.vtn.manager.it.ofmock.OfMockService;
import org.opendaylight.vtn.manager.it.util.BridgeNetwork;
import org.opendaylight.vtn.manager.it.util.TestHost;
import org.opendaylight.vtn.manager.it.util.VirtualNetwork;
import org.opendaylight.vtn.manager.it.util.unicast.ArpFlowFactory;
import org.opendaylight.vtn.manager.it.util.unicast.UnicastFlow;
import org.opendaylight.vtn.manager.it.util.vnode.VBridgeConfig;
import org.opendaylight.vtn.manager.it.util.vnode.VBridgeIdentifier;
import org.opendaylight.vtn.manager.it.util.vnode.VTNVlanMapConfig;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.mac.rev150907.VtnMacMapService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.port.rev150907.VtnPortMapService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeState;

/**
 * Test case for the packet routing table maintenance.
 */
public final class RoutingTableTest extends TestMethodBase {
    /**
     * Construct a new instance.
     *
     * @param vit  A {@link VTNManagerIT} instance.
     */
    public RoutingTableTest(VTNManagerIT vit) {
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
        OfMockService ofmock = vit.getOfMockService();
        VtnPortMapService pmapSrv = vit.getPortMapService();
        VtnMacMapService mmapSrv = vit.getMacMapService();

        // Create a VTN and a vBridge for test.
        String tname = "vtn";
        VBridgeIdentifier bpath = new VBridgeIdentifier(tname, "vbr");
        VirtualNetwork vnet = getVirtualNetwork().
            addBridge(bpath).
            apply().
            verify();

        // Collect edge ports per node, and all ISL ports.
        BridgeNetwork bridge = new BridgeNetwork(ofmock, bpath);
        Set<String> islPorts = new HashSet<>();
        int idx = 1;
        int vid = 0;
        for (String nid: ofmock.getNodes()) {
            for (String pid: ofmock.getPorts(nid, true)) {
                String pname = ofmock.getPortName(pid);
                bridge.addHost(nid, new TestHost(idx, pid, pname, vid));
                idx++;
            }

            for (String pid: ofmock.getPorts(nid, false)) {
                bridge.setUnmappedPort(pid);
                assertTrue(islPorts.add(pid));
            }
        }
        bridge.verify();
        assertFalse(islPorts.isEmpty());

        // Map untagged frame using VLAN mapping.
        VTNVlanMapConfig vmap0 = new VTNVlanMapConfig();
        VBridgeConfig bconf = vnet.getBridge(bpath).
            addVlanMap(vmap0);
        vnet.apply().verify();
        VNodeStateWaiter waiter = new VNodeStateWaiter(ofmock).
            set(bpath, VnodeState.UP);

        // Let the test vBridge learn MAC addresses.
        learnHosts(bridge);

        // Flow entry should not be present.
        new FlowCounter(ofmock).verify();

        // Cause path fault.
        unicastTest(bridge, islPorts, false);

        // Packet processing for previous test may not complete.
        // So we need to install a valid flow entry here in order to
        // synchronize packet processing.
        Map<String, List<TestHost>> hostMap = bridge.getTestHosts();
        Set<String> edgeNodes = new HashSet<>();
        for (Entry<String, List<TestHost>> entry: hostMap.entrySet()) {
            List<TestHost> hosts = entry.getValue();
            if (hosts.size() >= 2) {
                edgeNodes.add(entry.getKey());
                TestHost src = hosts.get(0);
                TestHost dst = hosts.get(1);
                ArpFlowFactory factory = new ArpFlowFactory(ofmock);
                List<OfMockLink> route = Collections.<OfMockLink>emptyList();
                UnicastFlow unicast = factory.create(src, dst, route);
                unicast.runTest();
                assertEquals(1, unicast.getFlowCount());
            }
        }
        assertFalse(edgeNodes.isEmpty());

        for (String nid: ofmock.getNodes()) {
            int expected = (edgeNodes.contains(nid)) ? 1 : 0;
            assertEquals(expected, ofmock.getFlowCount(nid));
        }

        // Resolve path fault.
        unicastTest(bridge, islPorts, true);

        vnet.verify();
        removeVtn(tname);
        vnet.removeTenant(tname).verify();
    }
}
