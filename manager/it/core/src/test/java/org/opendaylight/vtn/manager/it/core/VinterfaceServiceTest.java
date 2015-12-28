/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.core;

import java.util.HashMap;
import java.util.HashSet;
import java.util.Map.Entry;
import java.util.Map;
import java.util.Random;
import java.util.Set;

import org.opendaylight.vtn.manager.it.util.RpcErrorTag;
import org.opendaylight.vtn.manager.it.util.VirtualNetwork;
import org.opendaylight.vtn.manager.it.util.vnode.VBridgeConfig;
import org.opendaylight.vtn.manager.it.util.vnode.VBridgeIdentifier;
import org.opendaylight.vtn.manager.it.util.vnode.VBridgeIfIdentifier;
import org.opendaylight.vtn.manager.it.util.vnode.VInterfaceConfig;
import org.opendaylight.vtn.manager.it.util.vnode.VInterfaceIdentifier;
import org.opendaylight.vtn.manager.it.util.vnode.VTerminalConfig;
import org.opendaylight.vtn.manager.it.util.vnode.VTerminalIdentifier;
import org.opendaylight.vtn.manager.it.util.vnode.VTerminalIfIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.virtual.route.info.VirtualNodePath;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeUpdateMode;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnErrorTag;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateOperationType;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.RemoveVinterfaceInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.RemoveVinterfaceInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.UpdateVinterfaceInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.UpdateVinterfaceInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.VtnVinterfaceService;

/**
 * Test case for {@link VtnVinterfaceService}.
 */
public final class VinterfaceServiceTest extends TestMethodBase {
    /**
     * Construct a new instance.
     *
     * @param vit  A {@link VTNManagerIT} instance.
     */
    public VinterfaceServiceTest(VTNManagerIT vit) {
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
        VtnVinterfaceService vifSrv = vit.getVinterfaceService();
        VirtualNetwork vnet = getVirtualNetwork();

        // Create 2 VTNs, 2 vBridges, and 3 vTerminals.
        String tname1 = "vtn_1";
        String tname2 = "vtn_2";
        String bname1 = "bridge_1";
        String bname2 = "bridge_2";
        VBridgeIdentifier vbrId1 = new VBridgeIdentifier(tname1, bname1);
        VBridgeIdentifier vbrId2 = new VBridgeIdentifier(tname2, bname1);
        VTerminalIdentifier vtermId1 = new VTerminalIdentifier(tname1, bname1);
        VTerminalIdentifier vtermId2 = new VTerminalIdentifier(tname2, bname1);
        VTerminalIdentifier vtermId3 = new VTerminalIdentifier(tname2, bname2);
        vnet.addBridge(vbrId1, vbrId2).
            addTerminal(vtermId1, vtermId2, vtermId3).
            apply();

        // Create 3 virtual interfaces in vbrId1.
        Map<VInterfaceIdentifier<?>, VInterfaceConfig> interfaces =
            new HashMap<>();
        Map<VBridgeIfIdentifier, VInterfaceConfig> bridgeIfs = new HashMap<>();
        String iname1 = "if_1";
        String iname2 = "1234567890123456789012345678901";
        String iname3 = "if_3";
        VBridgeConfig bconf1 = vnet.getBridge(vbrId1);
        VBridgeIfIdentifier bifId1 = vbrId1.childInterface(iname1);
        VInterfaceConfig biconf1 = new VInterfaceConfig();
        assertEquals(null, interfaces.put(bifId1, biconf1));
        assertEquals(null, bridgeIfs.put(bifId1, biconf1));
        bconf1.addInterface(bifId1.getInterfaceNameString(), biconf1);
        assertEquals(VtnUpdateType.CREATED,
                     biconf1.update(vifSrv, bifId1, null, null));
        vnet.verify();

        VBridgeIfIdentifier bifId2 = vbrId1.childInterface(iname2);
        VInterfaceConfig biconf2 = new VInterfaceConfig("vBridge-IF 2", false);
        assertEquals(null, interfaces.put(bifId2, biconf2));
        assertEquals(null, bridgeIfs.put(bifId2, biconf2));
        bconf1.addInterface(bifId2.getInterfaceNameString(), biconf2);
        assertEquals(VtnUpdateType.CREATED,
                     biconf2.update(vifSrv, bifId2, VnodeUpdateMode.CREATE,
                                    null));
        vnet.verify();

        VBridgeIfIdentifier bifId3 = vbrId1.childInterface(iname3);
        VInterfaceConfig biconf3 = new VInterfaceConfig("vBridge-IF 3", true);
        assertEquals(null, interfaces.put(bifId3, biconf3));
        assertEquals(null, bridgeIfs.put(bifId3, biconf3));
        bconf1.addInterface(bifId3.getInterfaceNameString(), biconf3);
        assertEquals(VtnUpdateType.CREATED,
                     biconf3.update(vifSrv, bifId3, VnodeUpdateMode.UPDATE,
                                    null));
        vnet.verify();

        // Create 1 virtual interfaces per vTerminal.
        Map<VTerminalIfIdentifier, VInterfaceConfig> termIfs = new HashMap<>();
        VTerminalConfig vtconf1 = vnet.getTerminal(vtermId1);
        VTerminalIfIdentifier tifId1 = vtermId1.childInterface(iname1);
        VInterfaceConfig ticonf1 = new VInterfaceConfig();
        assertEquals(null, interfaces.put(tifId1, ticonf1));
        assertEquals(null, termIfs.put(tifId1, ticonf1));
        vtconf1.addInterface(tifId1.getInterfaceNameString(), ticonf1);
        assertEquals(VtnUpdateType.CREATED,
                     ticonf1.update(vifSrv, tifId1, null, null));
        vnet.verify();

        VTerminalConfig vtconf2 = vnet.getTerminal(vtermId2);
        VTerminalIfIdentifier tifId2 = vtermId2.childInterface(iname2);
        VInterfaceConfig ticonf2 = new VInterfaceConfig(null, true);
        assertEquals(null, interfaces.put(tifId2, ticonf2));
        assertEquals(null, termIfs.put(tifId2, ticonf2));
        vtconf2.addInterface(tifId2.getInterfaceNameString(), ticonf2);
        assertEquals(VtnUpdateType.CREATED,
                     ticonf2.update(vifSrv, tifId2, VnodeUpdateMode.CREATE,
                                    null));
        vnet.verify();

        VTerminalConfig vtconf3 = vnet.getTerminal(vtermId3);
        VTerminalIfIdentifier tifId3 = vtermId3.childInterface(iname3);
        VInterfaceConfig ticonf3 =
            new VInterfaceConfig("vTerminal-IF 3", false);
        assertEquals(null, interfaces.put(tifId3, ticonf3));
        assertEquals(null, termIfs.put(tifId3, ticonf3));
        vtconf3.addInterface(tifId3.getInterfaceNameString(), ticonf3);
        assertEquals(VtnUpdateType.CREATED,
                     ticonf3.update(vifSrv, tifId3, VnodeUpdateMode.UPDATE,
                                    null));
        vnet.verify();

        // Add random configuration.
        Random rand = new Random(271828L);
        bconf1.add(rand);
        vnet.getBridge(vbrId2).add(rand);
        for (VInterfaceConfig iconf: interfaces.values()) {
            iconf.add(rand);
        }

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
        for (Entry<VInterfaceIdentifier<?>, VInterfaceConfig> entry:
                 interfaces.entrySet()) {
            VInterfaceIdentifier<?> ifId = entry.getKey();
            VInterfaceConfig iconf = entry.getValue();
            for (VnodeUpdateMode mode: modifyModes) {
                for (VtnUpdateOperationType op: modifyOperations) {
                    assertEquals(null, iconf.update(vifSrv, ifId, mode, op));
                }
            }
        }

        // Change parameters using ADD operation.
        // terminal-name should be ignored if bridge-name is specified.
        String unknownName = "unknown";
        biconf1.setDescription("vBridge interface 1");
        UpdateVinterfaceInput input = new UpdateVinterfaceInputBuilder().
            setTenantName(bifId1.getTenantNameString()).
            setBridgeName(bifId1.getBridgeNameString()).
            setTerminalName(unknownName).
            setInterfaceName(bifId1.getInterfaceNameString()).
            setDescription(biconf1.getDescription()).
            setOperation(VtnUpdateOperationType.ADD).
            setUpdateMode(VnodeUpdateMode.MODIFY).
            build();
        assertEquals(VtnUpdateType.CHANGED,
                     getRpcResult(vifSrv.updateVinterface(input)));
        vnet.verify();

        biconf2.setEnabled(true);
        input = new UpdateVinterfaceInputBuilder().
            setTenantName(bifId2.getTenantNameString()).
            setBridgeName(bifId2.getBridgeNameString()).
            setTerminalName(unknownName).
            setInterfaceName(bifId2.getInterfaceNameString()).
            setEnabled(biconf2.isEnabled()).
            setOperation(VtnUpdateOperationType.ADD).
            setUpdateMode(VnodeUpdateMode.UPDATE).
            build();
        assertEquals(VtnUpdateType.CHANGED,
                     getRpcResult(vifSrv.updateVinterface(input)));
        vnet.verify();

        biconf3.setDescription("vBridge interface 3").
            setEnabled(false);
        input = new UpdateVinterfaceInputBuilder().
            setTenantName(bifId3.getTenantNameString()).
            setBridgeName(bifId3.getBridgeNameString()).
            setTerminalName(unknownName).
            setInterfaceName(bifId3.getInterfaceNameString()).
            setDescription(biconf3.getDescription()).
            setEnabled(biconf3.isEnabled()).
            build();
        assertEquals(VtnUpdateType.CHANGED,
                     getRpcResult(vifSrv.updateVinterface(input)));
        vnet.verify();

        ticonf1.setDescription("vTerminal interface 1");
        input = new UpdateVinterfaceInputBuilder().
            setTenantName(tifId1.getTenantNameString()).
            setTerminalName(tifId1.getBridgeNameString()).
            setInterfaceName(tifId1.getInterfaceNameString()).
            setDescription(ticonf1.getDescription()).
            build();
        assertEquals(VtnUpdateType.CHANGED,
                     getRpcResult(vifSrv.updateVinterface(input)));
        vnet.verify();

        ticonf2.setEnabled(false);
        input = new UpdateVinterfaceInputBuilder().
            setTenantName(tifId2.getTenantNameString()).
            setTerminalName(tifId2.getBridgeNameString()).
            setInterfaceName(tifId2.getInterfaceNameString()).
            setEnabled(ticonf2.isEnabled()).
            setOperation(VtnUpdateOperationType.ADD).
            setUpdateMode(VnodeUpdateMode.UPDATE).
            build();
        assertEquals(VtnUpdateType.CHANGED,
                     getRpcResult(vifSrv.updateVinterface(input)));
        vnet.verify();

        ticonf3.setDescription("vTerminal interface 3").
            setEnabled(true);
        input = new UpdateVinterfaceInputBuilder().
            setTenantName(tifId3.getTenantNameString()).
            setTerminalName(tifId3.getBridgeNameString()).
            setInterfaceName(tifId3.getInterfaceNameString()).
            setDescription(ticonf3.getDescription()).
            setEnabled(ticonf3.isEnabled()).
            setOperation(VtnUpdateOperationType.ADD).
            setUpdateMode(VnodeUpdateMode.MODIFY).
            build();
        assertEquals(VtnUpdateType.CHANGED,
                     getRpcResult(vifSrv.updateVinterface(input)));
        vnet.verify();

        // Change parameters using SET operation.
        biconf1.setDescription("vBridge interface 1 (SET)").
            setEnabled(false);
        assertEquals(VtnUpdateType.CHANGED,
                     biconf1.update(vifSrv, bifId1, null,
                                    VtnUpdateOperationType.SET));
        vnet.verify();

        biconf2.setDescription("vBridge interface 2 (SET)").
            setEnabled(null);
        assertEquals(VtnUpdateType.CHANGED,
                     biconf2.update(vifSrv, bifId2, null,
                                    VtnUpdateOperationType.SET));
        vnet.verify();

        biconf3.setDescription(null).
            setEnabled(null);
        assertEquals(VtnUpdateType.CHANGED,
                     biconf3.update(vifSrv, bifId3, null,
                                    VtnUpdateOperationType.SET));
        vnet.verify();

        ticonf1.setEnabled(false);
        assertEquals(VtnUpdateType.CHANGED,
                     ticonf1.update(vifSrv, tifId1, null,
                                    VtnUpdateOperationType.SET));
        vnet.verify();

        ticonf2.setDescription(null).
            setEnabled(null);
        assertEquals(VtnUpdateType.CHANGED,
                     ticonf2.update(vifSrv, tifId2, null,
                                    VtnUpdateOperationType.SET));
        vnet.verify();

        ticonf3.setDescription("vTerminal interface 3 (SET)").
            setEnabled(false);
        assertEquals(VtnUpdateType.CHANGED,
                     ticonf3.update(vifSrv, tifId3, null,
                                    VtnUpdateOperationType.SET));
        vnet.verify();

        // Error tests.

        // Null input.
        checkRpcError(vifSrv.updateVinterface(null),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);
        checkRpcError(vifSrv.removeVinterface(null),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);

        for (VnodeUpdateMode mode: VnodeUpdateMode.values()) {
            // Null tenant-name.
            input = new UpdateVinterfaceInputBuilder().
                setUpdateMode(mode).
                build();
            checkRpcError(vifSrv.updateVinterface(input),
                          RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);

            input = new UpdateVinterfaceInputBuilder().
                setBridgeName(bname1).
                setInterfaceName(iname1).
                setUpdateMode(mode).
                build();
            checkRpcError(vifSrv.updateVinterface(input),
                          RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);

            input = new UpdateVinterfaceInputBuilder().
                setTerminalName(bname1).
                setInterfaceName(iname1).
                setUpdateMode(mode).
                build();
            checkRpcError(vifSrv.updateVinterface(input),
                          RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);

            // No bridge name.
            input = new UpdateVinterfaceInputBuilder().
                setTenantName(tname1).
                setInterfaceName(iname1).
                setUpdateMode(mode).
                build();
            checkRpcError(vifSrv.updateVinterface(input),
                          RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);

            // Null interface-name.
            input = new UpdateVinterfaceInputBuilder().
                setTenantName(tname1).
                setBridgeName(bname1).
                setUpdateMode(mode).
                build();
            checkRpcError(vifSrv.updateVinterface(input),
                          RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);

            input = new UpdateVinterfaceInputBuilder().
                setTenantName(tname1).
                setTerminalName(bname1).
                setUpdateMode(mode).
                build();
            checkRpcError(vifSrv.updateVinterface(input),
                          RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);
        }

        // Null tenant-name.
        RemoveVinterfaceInput rinput = new RemoveVinterfaceInputBuilder().
            build();
        checkRpcError(vifSrv.removeVinterface(rinput),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);

        rinput = new RemoveVinterfaceInputBuilder().
            setBridgeName(bname1).
            setInterfaceName(iname1).
            build();
        checkRpcError(vifSrv.removeVinterface(rinput),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);

        rinput = new RemoveVinterfaceInputBuilder().
            setTerminalName(bname1).
            setInterfaceName(iname1).
            build();
        checkRpcError(vifSrv.removeVinterface(rinput),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);

        // No bridge name.
        rinput = new RemoveVinterfaceInputBuilder().
            setTenantName(tname1).
            setInterfaceName(iname1).
            build();
        checkRpcError(vifSrv.removeVinterface(rinput),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);

        // Null interface-name.
        rinput = new RemoveVinterfaceInputBuilder().
            setTenantName(tname1).
            setBridgeName(bname1).
            build();
        checkRpcError(vifSrv.removeVinterface(rinput),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);

        rinput = new RemoveVinterfaceInputBuilder().
            setTenantName(tname1).
            setTerminalName(bname1).
            build();
        checkRpcError(vifSrv.removeVinterface(rinput),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);

        for (String name: INVALID_VNODE_NAMES) {
            for (VnodeUpdateMode mode: VnodeUpdateMode.values()) {
                // Invalid tenant-name.
                input = new UpdateVinterfaceInputBuilder().
                    setTenantName(name).
                    setBridgeName(bname1).
                    setInterfaceName(iname1).
                    setUpdateMode(mode).
                    build();
                checkRpcError(vifSrv.updateVinterface(input),
                              RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);

                input = new UpdateVinterfaceInputBuilder().
                    setTenantName(name).
                    setTerminalName(bname1).
                    setInterfaceName(iname1).
                    setUpdateMode(mode).
                    build();
                checkRpcError(vifSrv.updateVinterface(input),
                              RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);

                // Invalid bridge-name.
                input = new UpdateVinterfaceInputBuilder().
                    setTenantName(tname1).
                    setBridgeName(name).
                    setInterfaceName(iname1).
                    setUpdateMode(mode).
                    build();
                checkRpcError(vifSrv.updateVinterface(input),
                              RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);

                // Invalid terminal-name.
                input = new UpdateVinterfaceInputBuilder().
                    setTenantName(tname1).
                    setTerminalName(name).
                    setInterfaceName(iname1).
                    setUpdateMode(mode).
                    build();
                checkRpcError(vifSrv.updateVinterface(input),
                              RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);

                // Invalid interface-name.
                RpcErrorTag etag;
                VtnErrorTag vtag;
                if (mode == VnodeUpdateMode.MODIFY) {
                    etag = RpcErrorTag.DATA_MISSING;
                    vtag = VtnErrorTag.NOTFOUND;
                } else {
                    etag = RpcErrorTag.BAD_ELEMENT;
                    vtag = VtnErrorTag.BADREQUEST;
                }
                input = new UpdateVinterfaceInputBuilder().
                    setTenantName(tname1).
                    setBridgeName(bname1).
                    setInterfaceName(name).
                    setUpdateMode(mode).
                    build();
                checkRpcError(vifSrv.updateVinterface(input), etag, vtag);

                input = new UpdateVinterfaceInputBuilder().
                    setTenantName(tname1).
                    setTerminalName(bname1).
                    setInterfaceName(name).
                    setUpdateMode(mode).
                    build();
                checkRpcError(vifSrv.updateVinterface(input), etag, vtag);
            }

            // Invalid tenant-name.
            rinput = new RemoveVinterfaceInputBuilder().
                setTenantName(name).
                setBridgeName(bname1).
                setInterfaceName(iname1).
                build();
            checkRpcError(vifSrv.removeVinterface(rinput),
                          RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);

            rinput = new RemoveVinterfaceInputBuilder().
                setTenantName(name).
                setTerminalName(bname1).
                setInterfaceName(iname1).
                build();
            checkRpcError(vifSrv.removeVinterface(rinput),
                          RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);

            // Invalid bridge-name.
            rinput = new RemoveVinterfaceInputBuilder().
                setTenantName(tname1).
                setBridgeName(name).
                setInterfaceName(iname1).
                build();
            checkRpcError(vifSrv.removeVinterface(rinput),
                          RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);

            // Invalid terminal-name.
            rinput = new RemoveVinterfaceInputBuilder().
                setTenantName(tname1).
                setTerminalName(name).
                setInterfaceName(iname1).
                build();
            checkRpcError(vifSrv.removeVinterface(rinput),
                          RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);

            // Invalid interface-name.
            rinput = new RemoveVinterfaceInputBuilder().
                setTenantName(tname1).
                setBridgeName(bname1).
                setInterfaceName(name).
                build();
            checkRpcError(vifSrv.removeVinterface(rinput),
                          RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);

            rinput = new RemoveVinterfaceInputBuilder().
                setTenantName(tname1).
                setTerminalName(bname1).
                setInterfaceName(name).
                build();
            checkRpcError(vifSrv.removeVinterface(rinput),
                          RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);
        }

        for (VInterfaceIdentifier<?> ifId: interfaces.keySet()) {
            String tname = ifId.getTenantNameString();
            VirtualNodePath vpath = ifId.getVirtualNodePath();

            // Modifying virtual interface that is not present.
            UpdateVinterfaceInputBuilder builder =
                new UpdateVinterfaceInputBuilder();
            builder.fieldsFrom(vpath);
            input = builder.setTenantName(unknownName).
                setUpdateMode(VnodeUpdateMode.MODIFY).
                build();
            checkRpcError(vifSrv.updateVinterface(input),
                          RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);

            input = builder.setTenantName(tname).
                setBridgeName(unknownName).
                setTerminalName(null).
                build();
            checkRpcError(vifSrv.updateVinterface(input),
                          RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);

            input = builder.setBridgeName(null).
                setTerminalName(unknownName).
                build();
            checkRpcError(vifSrv.updateVinterface(input),
                          RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);

            builder.fieldsFrom(vpath);
            input = builder.setInterfaceName(unknownName).
                build();
            checkRpcError(vifSrv.updateVinterface(input),
                          RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);

            // Removing virtual interface that is not present.
            RemoveVinterfaceInputBuilder rbuilder =
                new RemoveVinterfaceInputBuilder();
            rbuilder.fieldsFrom(vpath);
            rinput = rbuilder.setTenantName(unknownName).
                build();
            checkRpcError(vifSrv.removeVinterface(rinput),
                          RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);

            rinput = rbuilder.setTenantName(tname).
                setBridgeName(unknownName).
                setTerminalName(null).
                build();
            checkRpcError(vifSrv.removeVinterface(rinput),
                          RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);

            rinput = rbuilder.setBridgeName(null).
                setTerminalName(unknownName).
                build();
            checkRpcError(vifSrv.removeVinterface(rinput),
                          RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);

            rbuilder.fieldsFrom(vpath);
            rinput = rbuilder.setInterfaceName(unknownName).
                build();
            checkRpcError(vifSrv.removeVinterface(rinput),
                          RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);

            // Name confliction.
            builder.fieldsFrom(vpath);
            input = builder.setUpdateMode(VnodeUpdateMode.CREATE).
                build();
            checkRpcError(vifSrv.updateVinterface(input),
                          RpcErrorTag.DATA_EXISTS, VtnErrorTag.CONFLICT);
        }

        // Adding more than one interfaces to vTerminal.
        for (VTerminalIfIdentifier ifId: termIfs.keySet()) {
            for (VnodeUpdateMode mode: VnodeUpdateMode.values()) {
                RpcErrorTag etag;
                VtnErrorTag vtag;
                if (mode == VnodeUpdateMode.MODIFY) {
                    etag = RpcErrorTag.DATA_MISSING;
                    vtag = VtnErrorTag.NOTFOUND;
                } else {
                    etag = RpcErrorTag.DATA_EXISTS;
                    vtag = VtnErrorTag.CONFLICT;
                }
                input = new UpdateVinterfaceInputBuilder().
                    setTenantName(ifId.getTenantNameString()).
                    setTerminalName(ifId.getBridgeNameString()).
                    setInterfaceName(unknownName).
                    setUpdateMode(mode).
                    build();
                checkRpcError(vifSrv.updateVinterface(input), etag, vtag);
            }
        }

        // Errors should never affect existing virtual interfaces.
        vnet.verify();

        // Remove bifId1, bifId3, and tifId2.
        assertEquals(biconf2, interfaces.remove(bifId2));
        assertEquals(ticonf1, interfaces.remove(tifId1));
        assertEquals(ticonf3, interfaces.remove(tifId3));
        for (VInterfaceIdentifier<?> ifId: interfaces.keySet()) {
            removeVinterface(ifId);
            vnet.removeInterface(ifId).verify();
        }

        // Remove vbrId1, vtermId1, and vtermId2.
        removeVbridge(vbrId1);
        vnet.removeBridge(vbrId1).verify();
        removeVterminal(vtermId1);
        vnet.removeTerminal(vtermId1).verify();
        removeVterminal(vtermId2);
        vnet.removeTerminal(vtermId2).verify();

        // Remove VTNs.
        Set<String> nameSet = new HashSet<>(vnet.getTenants().keySet());
        for (String tname: nameSet) {
            removeVtn(tname);
            vnet.removeTenant(tname).verify();
        }
    }
}
