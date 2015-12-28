/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.core;

import static org.opendaylight.vtn.manager.it.ofmock.OfMockService.ID_OPENFLOW;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import org.opendaylight.vtn.manager.it.util.RpcErrorTag;
import org.opendaylight.vtn.manager.it.util.VirtualNetwork;
import org.opendaylight.vtn.manager.it.util.vnode.VBridgeConfig;
import org.opendaylight.vtn.manager.it.util.vnode.VBridgeIdentifier;
import org.opendaylight.vtn.manager.it.util.vnode.VTNVlanMapConfig;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.vlan.rev150907.AddVlanMapInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.vlan.rev150907.AddVlanMapInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.vlan.rev150907.AddVlanMapOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.vlan.rev150907.GetVlanMapInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.vlan.rev150907.GetVlanMapInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.vlan.rev150907.GetVlanMapOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.vlan.rev150907.RemoveVlanMapInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.vlan.rev150907.RemoveVlanMapInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.vlan.rev150907.VtnVlanMapService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnErrorTag;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;

/**
 * Test case for {@link VtnVlanMapService}.
 */
public final class VlanMapServiceTest extends TestMethodBase {
    /**
     * Construct a new instance.
     *
     * @param vit  A {@link VTNManagerIT} instance.
     */
    public VlanMapServiceTest(VTNManagerIT vit) {
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
        VirtualNetwork vnet = getVirtualNetwork();

        // Create 3 vBridges.
        String tname1 = "vtn_1";
        String bname1 = "bridge_1";
        String bname2 = "bridge_2";
        String bname3 = "bridge_3";

        VBridgeIdentifier vbrId1 = new VBridgeIdentifier(tname1, bname1);
        VBridgeIdentifier vbrId2 = new VBridgeIdentifier(tname1, bname2);
        VBridgeIdentifier vbrId3 = new VBridgeIdentifier(tname1, bname3);
        vnet.addBridge(vbrId1, vbrId2, vbrId3).apply();

        // Map VLAN 0 to vbrId1.
        List<VTNVlanMapConfig> vmaps1 = new ArrayList<>();
        VBridgeConfig bconf1 = vnet.getBridge(vbrId1);
        VTNVlanMapConfig vmap11 = new VTNVlanMapConfig();
        vmaps1.add(vmap11);
        bconf1.addVlanMap(vmap11);
        AddVlanMapInput input = vmap11.newInputBuilder(vbrId1).build();
        AddVlanMapOutput output = getRpcOutput(vmapSrv.addVlanMap(input));
        assertEquals(vmap11.getMapId(), output.getMapId());
        assertEquals(Boolean.TRUE, output.isActive());
        vnet.verify();

        // Map VLAN 4095 to vbrId1.
        VTNVlanMapConfig vmap12 = new VTNVlanMapConfig(4095);
        vmaps1.add(vmap12);
        bconf1.addVlanMap(vmap12);
        input = vmap12.newInputBuilder(vbrId1).build();
        output = getRpcOutput(vmapSrv.addVlanMap(input));
        assertEquals(vmap12.getMapId(), output.getMapId());
        assertEquals(Boolean.TRUE, output.isActive());
        vnet.verify();

        // Map VLAN 10 on node1 (should not be present) to vbrId1.
        String node1 = ID_OPENFLOW + "1";
        VTNVlanMapConfig vmap13 = new VTNVlanMapConfig(node1, 10).
            setActive(false);
        vmaps1.add(vmap13);
        bconf1.addVlanMap(vmap13);
        input = vmap13.newInputBuilder(vbrId1).build();
        output = getRpcOutput(vmapSrv.addVlanMap(input));
        assertEquals(vmap13.getMapId(), output.getMapId());
        assertEquals(Boolean.FALSE, output.isActive());
        vnet.verify();

        // Map VLAN 0 on node1 to vbrId2.
        List<VTNVlanMapConfig> vmaps2 = new ArrayList<>();
        VBridgeConfig bconf2 = vnet.getBridge(vbrId2);
        VTNVlanMapConfig vmap21 = new VTNVlanMapConfig(node1, null).
            setActive(false);
        vmaps2.add(vmap21);
        bconf2.addVlanMap(vmap21);
        input = vmap21.newInputBuilder(vbrId2).build();
        output = getRpcOutput(vmapSrv.addVlanMap(input));
        assertEquals(vmap21.getMapId(), output.getMapId());
        assertEquals(Boolean.FALSE, output.isActive());
        vnet.verify();

        // Map VLAN 1 to vbrId2.
        VTNVlanMapConfig vmap22 = new VTNVlanMapConfig(1);
        vmaps2.add(vmap22);
        bconf2.addVlanMap(vmap22);
        input = vmap22.newInputBuilder(vbrId2).build();
        output = getRpcOutput(vmapSrv.addVlanMap(input));
        assertEquals(vmap22.getMapId(), output.getMapId());
        assertEquals(Boolean.TRUE, output.isActive());
        vnet.verify();

        // Map VLAN 30 on node2 (should not be present) to vbrId2.
        String node2 = ID_OPENFLOW + "18446744073709551615";
        VTNVlanMapConfig vmap23 = new VTNVlanMapConfig(node2, 30).
            setActive(false);
        vmaps2.add(vmap23);
        bconf2.addVlanMap(vmap23);
        input = vmap23.newInputBuilder(vbrId2).build();
        output = getRpcOutput(vmapSrv.addVlanMap(input));
        assertEquals(vmap23.getMapId(), output.getMapId());
        assertEquals(Boolean.FALSE, output.isActive());
        vnet.verify();

        // get-vlan-id test.
        VBridgeIdentifier[] vbrIds = {vbrId2, vbrId3};
        for (VTNVlanMapConfig vmap: vmaps1) {
            GetVlanMapInput ginput = vmap.newGetInputBuilder(vbrId1).build();
            GetVlanMapOutput goutput =
                getRpcOutput(vmapSrv.getVlanMap(ginput));
            assertEquals(vmap.getMapId(), goutput.getMapId());
            Boolean active = Boolean.valueOf(vmap.getNode() == null);
            assertEquals(active, goutput.isActive());

            VTNVlanMapConfig copy = vmap.complete();
            ginput = copy.newGetInputBuilder(vbrId1).build();
            goutput = getRpcOutput(vmapSrv.getVlanMap(ginput));
            assertEquals(vmap.getMapId(), goutput.getMapId());
            assertEquals(active, goutput.isActive());

            VTNVlanMapConfig[] vmaps = {vmap, copy};
            for (VBridgeIdentifier vbrId: vbrIds) {
                for (VTNVlanMapConfig vmc: vmaps) {
                    ginput = vmc.newGetInputBuilder(vbrId).build();
                    goutput = getRpcOutput(vmapSrv.getVlanMap(ginput));
                    assertEquals(null, goutput.getMapId());
                    assertEquals(null, goutput.isActive());
                }
            }
        }

        vbrIds = new VBridgeIdentifier[]{vbrId1, vbrId3};
        for (VTNVlanMapConfig vmap: vmaps2) {
            GetVlanMapInput ginput = vmap.newGetInputBuilder(vbrId2).build();
            GetVlanMapOutput goutput =
                getRpcOutput(vmapSrv.getVlanMap(ginput));
            assertEquals(vmap.getMapId(), goutput.getMapId());
            Boolean active = Boolean.valueOf(vmap.getNode() == null);
            assertEquals(active, goutput.isActive());

            VTNVlanMapConfig copy = vmap.complete();
            ginput = copy.newGetInputBuilder(vbrId2).build();
            goutput = getRpcOutput(vmapSrv.getVlanMap(ginput));
            assertEquals(vmap.getMapId(), goutput.getMapId());
            assertEquals(active, goutput.isActive());

            VTNVlanMapConfig[] vmaps = {vmap, copy};
            for (VBridgeIdentifier vbrId: vbrIds) {
                for (VTNVlanMapConfig vmc: vmaps) {
                    ginput = vmc.newGetInputBuilder(vbrId).build();
                    goutput = getRpcOutput(vmapSrv.getVlanMap(ginput));
                    assertEquals(null, goutput.getMapId());
                    assertEquals(null, goutput.isActive());
                }
            }
        }

        // Error tests.

        // Null input.
        checkRpcError(vmapSrv.addVlanMap(null),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);
        checkRpcError(vmapSrv.removeVlanMap(null),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);
        checkRpcError(vmapSrv.getVlanMap(null),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);

        // Null tenant-name.
        input = new AddVlanMapInputBuilder().build();
        checkRpcError(vmapSrv.addVlanMap(input),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);

        input = new AddVlanMapInputBuilder().
            setBridgeName(bname1).
            build();
        checkRpcError(vmapSrv.addVlanMap(input),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);

        RemoveVlanMapInput rinput = new RemoveVlanMapInputBuilder().build();
        checkRpcError(vmapSrv.removeVlanMap(rinput),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);

        rinput = new RemoveVlanMapInputBuilder().
            setBridgeName(bname1).
            build();
        checkRpcError(vmapSrv.removeVlanMap(rinput),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);

        GetVlanMapInput ginput = new GetVlanMapInputBuilder().build();
        checkRpcError(vmapSrv.getVlanMap(ginput),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);

        ginput = new GetVlanMapInputBuilder().
            setBridgeName(bname1).
            build();
        checkRpcError(vmapSrv.getVlanMap(ginput),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);

        // Null bridge-name.
        input = new AddVlanMapInputBuilder().
            setTenantName(tname1).
            build();
        checkRpcError(vmapSrv.addVlanMap(input),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);

        rinput = new RemoveVlanMapInputBuilder().
            setTenantName(tname1).
            build();
        checkRpcError(vmapSrv.removeVlanMap(rinput),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);

        ginput = new GetVlanMapInputBuilder().
            setTenantName(tname1).
            build();
        checkRpcError(vmapSrv.getVlanMap(ginput),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);

        for (String name: INVALID_VNODE_NAMES) {
            // Invalid tenant-name.
            input = new AddVlanMapInputBuilder().
                setTenantName(name).
                setBridgeName(bname1).
                build();
            checkRpcError(vmapSrv.addVlanMap(input),
                          RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);

            rinput = new RemoveVlanMapInputBuilder().
                setTenantName(name).
                setBridgeName(bname1).
                build();
            checkRpcError(vmapSrv.removeVlanMap(rinput),
                          RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);

            ginput = new GetVlanMapInputBuilder().
                setTenantName(name).
                setBridgeName(bname1).
                build();
            checkRpcError(vmapSrv.getVlanMap(ginput),
                          RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);

            // Invalid bridge-name.
            input = new AddVlanMapInputBuilder().
                setTenantName(tname1).
                setBridgeName(name).
                build();
            checkRpcError(vmapSrv.addVlanMap(input),
                          RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);

            rinput = new RemoveVlanMapInputBuilder().
                setTenantName(tname1).
                setBridgeName(name).
                build();
            checkRpcError(vmapSrv.removeVlanMap(rinput),
                          RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);

            ginput = new GetVlanMapInputBuilder().
                setTenantName(tname1).
                setBridgeName(name).
                build();
            checkRpcError(vmapSrv.getVlanMap(ginput),
                          RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);
        }

        // Specifying vBridge that is not present.
        String unknownName = "unknown";
        input = new AddVlanMapInputBuilder().
            setTenantName(unknownName).
            setBridgeName(bname1).
            build();
        checkRpcError(vmapSrv.addVlanMap(input),
                      RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);

        input = new AddVlanMapInputBuilder().
            setTenantName(tname1).
            setBridgeName(unknownName).
            build();
        checkRpcError(vmapSrv.addVlanMap(input),
                      RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);

        rinput = new RemoveVlanMapInputBuilder().
            setTenantName(unknownName).
            setBridgeName(bname1).
            build();
        checkRpcError(vmapSrv.removeVlanMap(rinput),
                      RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);

        rinput = new RemoveVlanMapInputBuilder().
            setTenantName(tname1).
            setBridgeName(unknownName).
            build();
        checkRpcError(vmapSrv.removeVlanMap(rinput),
                      RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);

        ginput = new GetVlanMapInputBuilder().
            setTenantName(unknownName).
            setBridgeName(bname1).
            build();
        checkRpcError(vmapSrv.getVlanMap(ginput),
                      RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);

        ginput = new GetVlanMapInputBuilder().
            setTenantName(tname1).
            setBridgeName(unknownName).
            build();
        checkRpcError(vmapSrv.getVlanMap(ginput),
                      RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);

        // Invalid node ID.
        for (String node: INVALID_NODE_IDS) {
            VTNVlanMapConfig vmap = new VTNVlanMapConfig(node, 0);
            input = vmap.newInputBuilder(vbrId1).build();
            checkRpcError(vmapSrv.addVlanMap(input),
                          RpcErrorTag.BAD_ELEMENT, VtnErrorTag.BADREQUEST);

            ginput = vmap.newGetInputBuilder(vbrId1).build();
            GetVlanMapOutput goutput =
                getRpcOutput(vmapSrv.getVlanMap(ginput));
            assertEquals(null, goutput.getMapId());
            assertEquals(null, goutput.isActive());
        }

        // Already mapped.
        List<VTNVlanMapConfig> allMaps = new ArrayList<>(vmaps1);
        allMaps.addAll(vmaps2);
        vbrIds = new VBridgeIdentifier[]{vbrId1, vbrId2, vbrId3};
        for (VBridgeIdentifier vbrId: vbrIds) {
            for (VTNVlanMapConfig vmap: allMaps) {
                input = vmap.newInputBuilder(vbrId).build();
                checkRpcError(vmapSrv.addVlanMap(input),
                              RpcErrorTag.DATA_EXISTS, VtnErrorTag.CONFLICT);
            }
        }

        // Errors should never affect existing VLAN mappings.
        vnet.verify();

        // Remove all VLAN mappings.
        for (VTNVlanMapConfig vmap: vmaps1) {
            String mapId = vmap.getMapId();
            Map<String, VtnUpdateType> expected = new HashMap<>();
            expected.put(mapId, VtnUpdateType.REMOVED);
            List<String> mapIds = Collections.singletonList(mapId);
            Map<String, VtnUpdateType> result = removeVlanMap(vbrId1, mapIds);
            assertEquals(expected, result);
            bconf1.removeVlanMap(mapId);
            vnet.verify();
        }

        Map<String, VtnUpdateType> expected = new HashMap<>();
        List<String> mapIds = new ArrayList<>();
        for (VTNVlanMapConfig vmap: vmaps1) {
            String mapId = vmap.getMapId();
            expected.put(mapId, null);
            mapIds.add(mapId);
        }

        for (VTNVlanMapConfig vmap: vmaps2) {
            String mapId = vmap.getMapId();
            expected.put(mapId, VtnUpdateType.REMOVED);
            mapIds.add(mapId);
        }

        Map<String, VtnUpdateType> result = removeVlanMap(vbrId2, mapIds);
        assertEquals(expected, result);
        bconf2.clearVlanMap();
        vnet.verify();

        // Restore all the VLAN mappigs into vbrId3.
        VBridgeConfig bconf3 = vnet.getBridge(vbrId3);
        for (VTNVlanMapConfig vmap: allMaps) {
            bconf3.addVlanMap(vmap);
            input = vmap.newInputBuilder(vbrId3).build();
            output = getRpcOutput(vmapSrv.addVlanMap(input));
            assertEquals(vmap.getMapId(), output.getMapId());
            Boolean active = Boolean.valueOf(vmap.getNode() == null);
            assertEquals(active, output.isActive());
            vnet.verify();
        }

        // Remove vbrId3.
        removeVbridge(vbrId3);
        vnet.removeBridge(vbrId3).verify();

        // Restore all the VLAN mappings into another VTN.
        String tname2 = "vtn_2";
        VBridgeIdentifier vbrId4 = new VBridgeIdentifier(tname2, bname1);
        vnet.addBridge(vbrId4).apply();
        VBridgeConfig bconf4 = vnet.getBridge(vbrId4);
        for (VTNVlanMapConfig vmap: allMaps) {
            bconf4.addVlanMap(vmap);
            input = vmap.newInputBuilder(vbrId4).build();
            output = getRpcOutput(vmapSrv.addVlanMap(input));
            assertEquals(vmap.getMapId(), output.getMapId());
            Boolean active = Boolean.valueOf(vmap.getNode() == null);
            assertEquals(active, output.isActive());
            vnet.verify();
        }

        // Remove vtn_2.
        removeVtn(tname2);
        vnet.removeTenant(tname2).verify();

        // Restore all the VLAN mappings into vbrId1.
        for (VTNVlanMapConfig vmap: allMaps) {
            bconf1.addVlanMap(vmap);
            input = vmap.newInputBuilder(vbrId1).build();
            output = getRpcOutput(vmapSrv.addVlanMap(input));
            assertEquals(vmap.getMapId(), output.getMapId());
            Boolean active = Boolean.valueOf(vmap.getNode() == null);
            assertEquals(active, output.isActive());
            vnet.verify();
        }

        // Remove vtn_1.
        removeVtn(tname1);
        vnet.removeTenant(tname1).verify();
    }
}
