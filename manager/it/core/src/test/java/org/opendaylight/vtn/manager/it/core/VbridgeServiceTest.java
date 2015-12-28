/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.core;

import static org.opendaylight.vtn.manager.it.ofmock.OfMockService.ID_OPENFLOW;

import java.util.HashMap;
import java.util.HashSet;
import java.util.Map.Entry;
import java.util.Map;
import java.util.Random;
import java.util.Set;

import org.opendaylight.vtn.manager.it.util.RpcErrorTag;
import org.opendaylight.vtn.manager.it.util.VirtualNetwork;
import org.opendaylight.vtn.manager.it.util.inventory.MacVlan;
import org.opendaylight.vtn.manager.it.util.vnode.VBridgeConfig;
import org.opendaylight.vtn.manager.it.util.vnode.VBridgeIdentifier;
import org.opendaylight.vtn.manager.it.util.vnode.VTNMacMapConfig;
import org.opendaylight.vtn.manager.it.util.vnode.VTNVlanMapConfig;
import org.opendaylight.vtn.manager.it.util.vnode.VTenantConfig;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeUpdateMode;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnErrorTag;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateOperationType;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.RemoveVbridgeInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.RemoveVbridgeInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.UpdateVbridgeInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.UpdateVbridgeInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.VtnVbridgeService;

/**
 * Test case for {@link VtnVbridgeService}.
 */
public final class VbridgeServiceTest extends TestMethodBase {
    /**
     * Construct a new instance.
     *
     * @param vit  A {@link VTNManagerIT} instance.
     */
    public VbridgeServiceTest(VTNManagerIT vit) {
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
        VtnVbridgeService vbrSrv = vit.getVbridgeService();
        VirtualNetwork vnet = getVirtualNetwork();

        // Create 2 VTNs.
        String tname1 = "vtn_1";
        String tname2 = "vtn_2";
        VTenantConfig tconf1 = new VTenantConfig();
        vnet.addTenant(tname1, tconf1).
            addTenant(tname2, new VTenantConfig()).
            apply();

        // Create 3 vBridges in vtn_1.
        Map<VBridgeIdentifier, VBridgeConfig> bridges = new HashMap<>();
        VBridgeIdentifier vbrId1 = new VBridgeIdentifier(tname1, "vbr_1");
        VBridgeConfig bconf1 = new VBridgeConfig();
        assertEquals(null, bridges.put(vbrId1, bconf1));
        tconf1.addBridge(vbrId1.getBridgeNameString(), bconf1);
        assertEquals(VtnUpdateType.CREATED,
                     bconf1.update(vbrSrv, vbrId1, null, null));
        vnet.verify();

        VBridgeIdentifier vbrId2 = new VBridgeIdentifier(tname1, "vbr_2");
        VBridgeConfig bconf2 = new VBridgeConfig("vBridge 2", 10);
        assertEquals(null, bridges.put(vbrId2, bconf2));
        tconf1.addBridge(vbrId2.getBridgeNameString(), bconf2);
        assertEquals(VtnUpdateType.CREATED,
                     bconf2.update(vbrSrv, vbrId2, VnodeUpdateMode.CREATE,
                                   null));
        vnet.verify();

        VBridgeIdentifier vbrId3 =
            new VBridgeIdentifier(tname1, "0123456789012345678901234567890");
        VBridgeConfig bconf3 = new VBridgeConfig("Test vBridge 3", 1000000);
        assertEquals(null, bridges.put(vbrId3, bconf3));
        tconf1.addBridge(vbrId3.getBridgeNameString(), bconf3);
        assertEquals(VtnUpdateType.CREATED,
                     bconf3.update(vbrSrv, vbrId3, VnodeUpdateMode.UPDATE,
                                   null));
        vnet.verify();

        // Add random configuration.
        Random rand = new Random(77777777L);
        bconf1.add(rand).
            addVlanMap(new VTNVlanMapConfig());
        bconf2.add(rand).
            addVlanMap(new VTNVlanMapConfig(12));
        VTNMacMapConfig macMap = new VTNMacMapConfig().
            addAllowed(new MacVlan(MacVlan.UNDEFINED, 0),
                       new MacVlan(0x1234567L, 32));
        bconf3.add(rand).
            addVlanMap(new VTNVlanMapConfig(ID_OPENFLOW + "123", 4095)).
            setMacMap(macMap);

        vnet.apply().verify();

        // Try to update with the same parameter.
        VnodeUpdateMode[] modifyModes = {
            null,
            VnodeUpdateMode.UPDATE,
            VnodeUpdateMode.MODIFY,
        };
        VtnUpdateOperationType[] modifyOperations = {
            null,
            VtnUpdateOperationType.SET,
            VtnUpdateOperationType.ADD,
        };
        for (Entry<VBridgeIdentifier, VBridgeConfig> entry:
                 bridges.entrySet()) {
            VBridgeIdentifier vbrId = entry.getKey();
            VBridgeConfig bconf = entry.getValue();
            for (VnodeUpdateMode mode: modifyModes) {
                for (VtnUpdateOperationType op: modifyOperations) {
                    assertEquals(null, bconf.update(vbrSrv, vbrId, mode, op));
                }
            }
        }

        // Change parameters using ADD operation.
        bconf1.setAgeInterval(123456);
        UpdateVbridgeInput input = new UpdateVbridgeInputBuilder().
            setTenantName(vbrId1.getTenantNameString()).
            setBridgeName(vbrId1.getBridgeNameString()).
            setAgeInterval(bconf1.getAgeInterval()).
            build();
        assertEquals(VtnUpdateType.CHANGED,
                     getRpcResult(vbrSrv.updateVbridge(input)));
        vnet.verify();

        bconf2.setDescription("Virtual bridge 2");
        input = new UpdateVbridgeInputBuilder().
            setTenantName(vbrId2.getTenantNameString()).
            setBridgeName(vbrId2.getBridgeNameString()).
            setDescription(bconf2.getDescription()).
            setOperation(VtnUpdateOperationType.ADD).
            setUpdateMode(VnodeUpdateMode.MODIFY).
            build();
        assertEquals(VtnUpdateType.CHANGED,
                     getRpcResult(vbrSrv.updateVbridge(input)));
        vnet.verify();

        bconf3.setDescription("Virtual bridge 3").
            setAgeInterval(9876);
        input = new UpdateVbridgeInputBuilder().
            setTenantName(vbrId3.getTenantNameString()).
            setBridgeName(vbrId3.getBridgeNameString()).
            setDescription(bconf3.getDescription()).
            setAgeInterval(bconf3.getAgeInterval()).
            setOperation(VtnUpdateOperationType.ADD).
            setUpdateMode(VnodeUpdateMode.UPDATE).
            build();
        assertEquals(VtnUpdateType.CHANGED,
                     getRpcResult(vbrSrv.updateVbridge(input)));
        vnet.verify();

        // Change parameters using SET operation.
        bconf1.setDescription("Test vBridge 1 (SET)");
        assertEquals(VtnUpdateType.CHANGED,
                     bconf1.update(vbrSrv, vbrId1, null,
                                   VtnUpdateOperationType.SET));
        vnet.verify();

        bconf2.setDescription(null).
            setAgeInterval(314159);
        assertEquals(VtnUpdateType.CHANGED,
                     bconf2.update(vbrSrv, vbrId2, VnodeUpdateMode.UPDATE,
                                   VtnUpdateOperationType.SET));
        vnet.verify();

        bconf3.setDescription(null).
            setAgeInterval(null);
        assertEquals(VtnUpdateType.CHANGED,
                     bconf3.update(vbrSrv, vbrId3, VnodeUpdateMode.MODIFY,
                                   VtnUpdateOperationType.SET));
        vnet.verify();

        // Error tests.

        // Null input.
        checkRpcError(vbrSrv.updateVbridge(null),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);
        checkRpcError(vbrSrv.removeVbridge(null),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);

        for (VnodeUpdateMode mode: VnodeUpdateMode.values()) {
            // Null tenant-name.
            input = new UpdateVbridgeInputBuilder().
                setUpdateMode(mode).
                build();
            checkRpcError(vbrSrv.updateVbridge(input),
                          RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);

            input = new UpdateVbridgeInputBuilder().
                setBridgeName(vbrId1.getBridgeNameString()).
                setUpdateMode(mode).
                build();
            checkRpcError(vbrSrv.updateVbridge(input),
                          RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);

            // Null bridge-name.
            input = new UpdateVbridgeInputBuilder().
                setTenantName(tname1).
                setUpdateMode(mode).
                build();
            checkRpcError(vbrSrv.updateVbridge(input),
                          RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);
        }

        // Null tenant-name.
        RemoveVbridgeInput rinput = new RemoveVbridgeInputBuilder().build();
        checkRpcError(vbrSrv.removeVbridge(rinput),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);

        rinput = new RemoveVbridgeInputBuilder().
            setBridgeName(vbrId1.getBridgeNameString()).
            build();
        checkRpcError(vbrSrv.removeVbridge(rinput),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);

        // Null bridge-name.
        rinput = new RemoveVbridgeInputBuilder().
            setTenantName(tname1).
            build();
        checkRpcError(vbrSrv.removeVbridge(rinput),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);

        for (String name: INVALID_VNODE_NAMES) {
            for (VnodeUpdateMode mode: VnodeUpdateMode.values()) {
                // Invalid tenant-name.
                input = new UpdateVbridgeInputBuilder().
                    setTenantName(name).
                    setBridgeName(vbrId1.getBridgeNameString()).
                    setUpdateMode(mode).
                    build();
                checkRpcError(vbrSrv.updateVbridge(input),
                              RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);

                // Invalid bridge-name.
                RpcErrorTag etag;
                VtnErrorTag vtag;
                if (mode == VnodeUpdateMode.MODIFY) {
                    etag = RpcErrorTag.DATA_MISSING;
                    vtag = VtnErrorTag.NOTFOUND;
                } else {
                    etag = RpcErrorTag.BAD_ELEMENT;
                    vtag = VtnErrorTag.BADREQUEST;
                }
                input = new UpdateVbridgeInputBuilder().
                    setTenantName(tname1).
                    setBridgeName(name).
                    setUpdateMode(mode).
                    build();
                checkRpcError(vbrSrv.updateVbridge(input), etag, vtag);
            }

            // Invalid tenant-name.
            rinput = new RemoveVbridgeInputBuilder().
                setTenantName(name).
                setBridgeName(vbrId1.getBridgeNameString()).
                build();
            checkRpcError(vbrSrv.removeVbridge(rinput),
                          RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);

            // Invalid bridge-name.
            rinput = new RemoveVbridgeInputBuilder().
                setTenantName(tname1).
                setBridgeName(name).
                build();
            checkRpcError(vbrSrv.removeVbridge(rinput),
                          RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);
        }

        // Modifying vBridge that is not present.
        String unknownName = "unknown";
        input = new UpdateVbridgeInputBuilder().
            setTenantName(unknownName).
            setBridgeName(vbrId1.getBridgeNameString()).
            setUpdateMode(VnodeUpdateMode.MODIFY).
            build();
        checkRpcError(vbrSrv.updateVbridge(input),
                      RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);

        input = new UpdateVbridgeInputBuilder().
            setTenantName(tname1).
            setBridgeName(unknownName).
            setUpdateMode(VnodeUpdateMode.MODIFY).
            build();
        checkRpcError(vbrSrv.updateVbridge(input),
                      RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);

        // Removing vBridge that is not present.
        rinput = new RemoveVbridgeInputBuilder().
            setTenantName(unknownName).
            setBridgeName(vbrId1.getBridgeNameString()).
            build();
        checkRpcError(vbrSrv.removeVbridge(rinput),
                      RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);

        rinput = new RemoveVbridgeInputBuilder().
            setTenantName(tname1).
            setBridgeName(unknownName).
            build();
        checkRpcError(vbrSrv.removeVbridge(rinput),
                      RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);

        // Invalid update operation.
        input = new UpdateVbridgeInputBuilder().
            setTenantName(vbrId1.getTenantNameString()).
            setBridgeName(vbrId1.getBridgeNameString()).
            setOperation(VtnUpdateOperationType.REMOVE).
            build();
        checkRpcError(vbrSrv.updateVbridge(input),
                      RpcErrorTag.BAD_ELEMENT, VtnErrorTag.BADREQUEST);

        // Removing vBridge that is not present.
        String[] tnames = {unknownName, tname2};
        String[] bnames = {
            unknownName,
            vbrId1.getBridgeNameString(),
            vbrId2.getBridgeNameString(),
            vbrId3.getBridgeNameString(),
        };
        for (String tname: tnames) {
            for (String bname: bnames) {
                rinput = new RemoveVbridgeInputBuilder().
                    setTenantName(tname).
                    setBridgeName(bname).
                    build();
                checkRpcError(vbrSrv.removeVbridge(rinput),
                              RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);
            }
        }

        // Name confliction.
        for (VBridgeIdentifier vbrId: bridges.keySet()) {
            input = new UpdateVbridgeInputBuilder().
                setTenantName(vbrId.getTenantNameString()).
                setBridgeName(vbrId.getBridgeNameString()).
                setUpdateMode(VnodeUpdateMode.CREATE).
                build();
            checkRpcError(vbrSrv.updateVbridge(input),
                          RpcErrorTag.DATA_EXISTS, VtnErrorTag.CONFLICT);
        }

        // Errors should never affect existing vBridges.
        vnet.verify();

        // Remove vbrId1 and vbrId3.
        assertEquals(bconf2, bridges.remove(vbrId2));
        for (VBridgeIdentifier vbrId: bridges.keySet()) {
            removeVbridge(vbrId);
            tconf1.removeBridge(vbrId.getBridgeNameString());
            vnet.verify();
        }

        // Remove VTNs.
        Set<String> nameSet = new HashSet<>(vnet.getTenants().keySet());
        for (String tname: nameSet) {
            removeVtn(tname);
            vnet.removeTenant(tname).verify();
        }
    }
}
