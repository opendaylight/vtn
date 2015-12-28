/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.core;

import java.util.List;

import org.opendaylight.vtn.manager.it.ofmock.OfMockService;
import org.opendaylight.vtn.manager.it.ofmock.OfMockUtils;
import org.opendaylight.vtn.manager.it.util.VirtualNetwork;
import org.opendaylight.vtn.manager.it.util.vnode.VBridgeConfig;
import org.opendaylight.vtn.manager.it.util.vnode.VBridgeIdentifier;
import org.opendaylight.vtn.manager.it.util.vnode.VTNVlanMapConfig;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.vlan.rev150907.AddVlanMapInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.vlan.rev150907.AddVlanMapOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.vlan.rev150907.VtnVlanMapService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeState;

/**
 * Test case for {@link VtnVlanMapService} that depends on inventory
 * information.
 */
public final class VlanMapStatusTest extends TestMethodBase {
    /**
     * Construct a new instance.
     *
     * @param vit  A {@link VTNManagerIT} instance.
     */
    public VlanMapStatusTest(VTNManagerIT vit) {
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
        VtnVlanMapService vmapSrv = vit.getVlanMapService();
        OfMockService ofmock = vit.getOfMockService();

        // Determine nodes that has at least one edge port.
        String edgeNode = null;
        String internalNode = null;
        List<String> edgePorts = null;
        for (String nid: ofmock.getNodes()) {
            List<String> edges = ofmock.getPorts(nid, true);
            if (edges.isEmpty()) {
                internalNode = nid;
            } else {
                edgeNode = nid;
                edgePorts = edges;
            }

            if (edgeNode != null && internalNode != null) {
                break;
            }
        }

        assertNotNull(edgeNode);
        assertNotNull(internalNode);

        // Determine one port in internalNode that is not connected to
        // edgeNode.
        String internalPort = null;
        for (String pid: ofmock.getPorts(internalNode, false)) {
            String peer = ofmock.getPeerIdentifier(pid);
            if (!internalNode.equals(OfMockUtils.getNodeIdentifier(peer))) {
                internalPort = pid;
            }
        }
        assertNotNull(internalPort);

        // Create 2 vBridges.
        VirtualNetwork vnet = getVirtualNetwork();
        String tname1 = "vtn_1";
        String bname1 = "bridge_1";
        String bname2 = "bridge_2";

        VBridgeIdentifier vbrId1 = new VBridgeIdentifier(tname1, bname1);
        VBridgeIdentifier vbrId2 = new VBridgeIdentifier(tname1, bname2);
        vnet.addBridge(vbrId1, vbrId2).apply();
        VBridgeConfig bconf1 = vnet.getBridge(vbrId1);
        VBridgeConfig bconf2 = vnet.getBridge(vbrId2);

        // Map VLAN 1 on edgeNode to vbrId1.
        VTNVlanMapConfig vmap1 = new VTNVlanMapConfig(edgeNode, 1).
            setActive(Boolean.TRUE);
        bconf1.addVlanMap(vmap1);
        AddVlanMapInput input = vmap1.newInputBuilder(vbrId1).build();
        AddVlanMapOutput output = getRpcOutput(vmapSrv.addVlanMap(input));
        assertEquals(vmap1.getMapId(), output.getMapId());
        assertEquals(Boolean.TRUE, output.isActive());
        vnet.verify();

        // Map VLAN 1 on internalNode to vbrId2.
        VTNVlanMapConfig vmap2 = new VTNVlanMapConfig(internalNode, 1).
            setActive(Boolean.FALSE);
        bconf2.addVlanMap(vmap2);
        input = vmap2.newInputBuilder(vbrId2).build();
        output = getRpcOutput(vmapSrv.addVlanMap(input));
        assertEquals(vmap2.getMapId(), output.getMapId());
        assertEquals(Boolean.FALSE, output.isActive());
        vnet.verify();

        // Down all the edge ports in edgeNode.
        // Then the state of vbrId1 should be changed to DOWN.
        for (String pid: edgePorts) {
            assertEquals(true, ofmock.setPortState(pid, false, false));
        }

        VNodeStateWaiter waiter = new VNodeStateWaiter(ofmock).
            set(vbrId1, VnodeState.DOWN).
            await();
        vmap1.setActive(Boolean.FALSE);
        vnet.verify();

        // Remove inter-switch link on internalPort.
        // Then the state of vbrId2 should be changed to UP.
        addStaticEdgePort(vit, internalPort);
        waiter.set(vbrId2, VnodeState.UP).await();
        vmap2.setActive(Boolean.TRUE);
        vnet.verify();

        // Up all the edge ports in edgeNode.
        // Then the state of vbrId1 should be changed to UP.
        for (String pid: edgePorts) {
            assertEquals(true, ofmock.setPortState(pid, true, false));
            waiter.set(vbrId1, VnodeState.UP).await();
            vmap1.setActive(Boolean.TRUE);
            vnet.verify();
        }

        // Remove static topology configuration.
        // Then the state of vbrId2 should be changed to DOWN.
        removeVtnStaticTopology(vit);
        waiter.set(vbrId2, VnodeState.DOWN).await();
        vmap2.setActive(Boolean.FALSE);
        vnet.verify();

        // Remove VTN.
        removeVtn(tname1);
        vnet.removeTenant(tname1).verify();
    }
}
