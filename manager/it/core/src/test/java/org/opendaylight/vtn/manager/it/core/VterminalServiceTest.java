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
import org.opendaylight.vtn.manager.it.util.vnode.VTenantConfig;
import org.opendaylight.vtn.manager.it.util.vnode.VTerminalConfig;
import org.opendaylight.vtn.manager.it.util.vnode.VTerminalIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeUpdateMode;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnErrorTag;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateOperationType;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vterminal.rev150907.RemoveVterminalInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vterminal.rev150907.RemoveVterminalInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vterminal.rev150907.UpdateVterminalInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vterminal.rev150907.UpdateVterminalInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vterminal.rev150907.VtnVterminalService;

/**
 * Test case for {@link VtnVterminalService}.
 */
public final class VterminalServiceTest extends TestMethodBase {
    /**
     * Construct a new instance.
     *
     * @param vit  A {@link VTNManagerIT} instance.
     */
    public VterminalServiceTest(VTNManagerIT vit) {
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
        VtnVterminalService vtermSrv = vit.getVterminalService();
        VirtualNetwork vnet = getVirtualNetwork();

        // Create 2 VTNs.
        String tname1 = "vtn_1";
        String tname2 = "vtn_2";
        VTenantConfig tconf1 = new VTenantConfig();
        vnet.addTenant(tname1, tconf1).
            addTenant(tname2, new VTenantConfig());
        vnet.apply();

        // Create 3 vTerminals in vtn_1.
        Map<VTerminalIdentifier, VTerminalConfig> terminals = new HashMap<>();
        VTerminalIdentifier vtermId1 =
            new VTerminalIdentifier(tname1, "vterm_1");
        VTerminalConfig vtconf1 = new VTerminalConfig();
        assertEquals(null, terminals.put(vtermId1, vtconf1));
        tconf1.addTerminal(vtermId1.getBridgeNameString(), vtconf1);
        assertEquals(VtnUpdateType.CREATED,
                     vtconf1.update(vtermSrv, vtermId1, null, null));
        vnet.verify();

        VTerminalIdentifier vtermId2 =
            new VTerminalIdentifier(tname1, "vterm_2");
        VTerminalConfig vtconf2 = new VTerminalConfig("vTerminal 2");
        assertEquals(null, terminals.put(vtermId2, vtconf2));
        tconf1.addTerminal(vtermId2.getBridgeNameString(), vtconf2);
        assertEquals(VtnUpdateType.CREATED,
                     vtconf2.update(vtermSrv, vtermId2, VnodeUpdateMode.CREATE,
                                    null));
        vnet.verify();

        VTerminalIdentifier vtermId3 =
            new VTerminalIdentifier(tname1, "1234567890123456789012345678901");
        VTerminalConfig vtconf3 = new VTerminalConfig("Test vTerminal 3");
        assertEquals(null, terminals.put(vtermId3, vtconf3));
        tconf1.addTerminal(vtermId3.getBridgeNameString(), vtconf3);
        assertEquals(VtnUpdateType.CREATED,
                     vtconf3.update(vtermSrv, vtermId3, VnodeUpdateMode.UPDATE,
                                    null));
        vnet.verify();

        // Add random configuration.
        Random rand = new Random(0xabcdef123L);
        for (VTerminalConfig vtconf: terminals.values()) {
            vtconf.add(rand);
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
        for (Entry<VTerminalIdentifier, VTerminalConfig> entry:
                 terminals.entrySet()) {
            VTerminalIdentifier vtermId = entry.getKey();
            VTerminalConfig vtconf = entry.getValue();
            for (VnodeUpdateMode mode: modifyModes) {
                for (VtnUpdateOperationType op: modifyOperations) {
                    assertEquals(null,
                                 vtconf.update(vtermSrv, vtermId, mode, op));
                }
            }
        }

        // Change parameters using ADD operation.
        vtconf1.setDescription("Virtual terminal 1");
        UpdateVterminalInput input = new UpdateVterminalInputBuilder().
            setTenantName(vtermId1.getTenantNameString()).
            setTerminalName(vtermId1.getBridgeNameString()).
            setDescription(vtconf1.getDescription()).
            build();
        assertEquals(VtnUpdateType.CHANGED,
                     getRpcResult(vtermSrv.updateVterminal(input)));
        vnet.verify();

        vtconf2.setDescription("Virtual terminal 2");
        input = new UpdateVterminalInputBuilder().
            setTenantName(vtermId2.getTenantNameString()).
            setTerminalName(vtermId2.getBridgeNameString()).
            setDescription(vtconf2.getDescription()).
            setOperation(VtnUpdateOperationType.ADD).
            setUpdateMode(VnodeUpdateMode.UPDATE).
            build();
        assertEquals(VtnUpdateType.CHANGED,
                     getRpcResult(vtermSrv.updateVterminal(input)));
        vnet.verify();

        vtconf3.setDescription("vTerminal 3");
        input = new UpdateVterminalInputBuilder().
            setTenantName(vtermId3.getTenantNameString()).
            setTerminalName(vtermId3.getBridgeNameString()).
            setDescription(vtconf3.getDescription()).
            setOperation(VtnUpdateOperationType.ADD).
            setUpdateMode(VnodeUpdateMode.MODIFY).
            build();
        assertEquals(VtnUpdateType.CHANGED,
                     getRpcResult(vtermSrv.updateVterminal(input)));
        vnet.verify();

        // Change parameters using SET operation.
        vtconf1.setDescription("vTerminal 1 (SET)");
        assertEquals(VtnUpdateType.CHANGED,
                     vtconf1.update(vtermSrv, vtermId1, null,
                                    VtnUpdateOperationType.SET));
        vnet.verify();

        vtconf2.setDescription("vTerminal 2 (SET)");
        assertEquals(VtnUpdateType.CHANGED,
                     vtconf2.update(vtermSrv, vtermId2, VnodeUpdateMode.UPDATE,
                                    VtnUpdateOperationType.SET));
        vnet.verify();

        vtconf3.setDescription("vTerminal 3 (SET)");
        assertEquals(VtnUpdateType.CHANGED,
                     vtconf3.update(vtermSrv, vtermId3, VnodeUpdateMode.MODIFY,
                                    VtnUpdateOperationType.SET));
        vnet.verify();

        // Error tests.

        // Null input.
        checkRpcError(vtermSrv.updateVterminal(null),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);
        checkRpcError(vtermSrv.removeVterminal(null),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);

        for (VnodeUpdateMode mode: VnodeUpdateMode.values()) {
            // Null tenant-name.
            input = new UpdateVterminalInputBuilder().
                setUpdateMode(mode).
                build();
            checkRpcError(vtermSrv.updateVterminal(input),
                          RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);

            input = new UpdateVterminalInputBuilder().
                setTerminalName(vtermId1.getBridgeNameString()).
                setUpdateMode(mode).
                build();
            checkRpcError(vtermSrv.updateVterminal(input),
                          RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);

            // Null terminal-name.
            input = new UpdateVterminalInputBuilder().
                setTenantName(tname1).
                setUpdateMode(mode).
                build();
            checkRpcError(vtermSrv.updateVterminal(input),
                          RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);
        }

        // Null tenant-name.
        RemoveVterminalInput rinput = new RemoveVterminalInputBuilder().
            build();
        checkRpcError(vtermSrv.removeVterminal(rinput),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);

        rinput = new RemoveVterminalInputBuilder().
            setTerminalName(vtermId1.getBridgeNameString()).
            build();
        checkRpcError(vtermSrv.removeVterminal(rinput),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);

        // Null terminal-name.
        rinput = new RemoveVterminalInputBuilder().
            setTenantName(tname1).
            build();
        checkRpcError(vtermSrv.removeVterminal(rinput),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);

        for (String name: INVALID_VNODE_NAMES) {
            for (VnodeUpdateMode mode: VnodeUpdateMode.values()) {
                // Invalid tenant-name.
                input = new UpdateVterminalInputBuilder().
                    setTenantName(name).
                    setTerminalName(vtermId1.getBridgeNameString()).
                    setUpdateMode(mode).
                    build();
                checkRpcError(vtermSrv.updateVterminal(input),
                              RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);

                // Invalid terminal-name.
                RpcErrorTag etag;
                VtnErrorTag vtag;
                if (mode == VnodeUpdateMode.MODIFY) {
                    etag = RpcErrorTag.DATA_MISSING;
                    vtag = VtnErrorTag.NOTFOUND;
                } else {
                    etag = RpcErrorTag.BAD_ELEMENT;
                    vtag = VtnErrorTag.BADREQUEST;
                }
                input = new UpdateVterminalInputBuilder().
                    setTenantName(tname1).
                    setTerminalName(name).
                    setUpdateMode(mode).
                    build();
                checkRpcError(vtermSrv.updateVterminal(input), etag, vtag);
            }

            // Invalid tenant-name.
            rinput = new RemoveVterminalInputBuilder().
                setTenantName(name).
                setTerminalName(vtermId1.getBridgeNameString()).
                build();
            checkRpcError(vtermSrv.removeVterminal(rinput),
                          RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);

            // Invalid terminal-name.
            rinput = new RemoveVterminalInputBuilder().
                setTenantName(tname1).
                setTerminalName(name).
                build();
            checkRpcError(vtermSrv.removeVterminal(rinput),
                          RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);
        }

        // Modifying vTerminal that is not present.
        String unknownName = "unknown";
        input = new UpdateVterminalInputBuilder().
            setTenantName(unknownName).
            setTerminalName(vtermId1.getBridgeNameString()).
            setUpdateMode(VnodeUpdateMode.MODIFY).
            build();
        checkRpcError(vtermSrv.updateVterminal(input),
                      RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);

        input = new UpdateVterminalInputBuilder().
            setTenantName(tname1).
            setTerminalName(unknownName).
            setUpdateMode(VnodeUpdateMode.MODIFY).
            build();
        checkRpcError(vtermSrv.updateVterminal(input),
                      RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);

        // Removing vTerminal that is not present.
        rinput = new RemoveVterminalInputBuilder().
            setTenantName(unknownName).
            setTerminalName(vtermId1.getBridgeNameString()).
            build();
        checkRpcError(vtermSrv.removeVterminal(rinput),
                      RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);

        rinput = new RemoveVterminalInputBuilder().
            setTenantName(tname1).
            setTerminalName(unknownName).
            build();
        checkRpcError(vtermSrv.removeVterminal(rinput),
                      RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);

        // Invalid update operation.
        input = new UpdateVterminalInputBuilder().
            setTenantName(vtermId1.getTenantNameString()).
            setTerminalName(vtermId1.getBridgeNameString()).
            setOperation(VtnUpdateOperationType.REMOVE).
            build();
        checkRpcError(vtermSrv.updateVterminal(input),
                      RpcErrorTag.BAD_ELEMENT, VtnErrorTag.BADREQUEST);

        // Removing vTerminal that is not present.
        String[] tnames = {unknownName, tname2};
        String[] bnames = {
            unknownName,
            vtermId1.getBridgeNameString(),
            vtermId2.getBridgeNameString(),
            vtermId3.getBridgeNameString(),
        };
        for (String tname: tnames) {
            for (String bname: bnames) {
                rinput = new RemoveVterminalInputBuilder().
                    setTenantName(tname).
                    setTerminalName(bname).
                    build();
                checkRpcError(vtermSrv.removeVterminal(rinput),
                              RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);
            }
        }

        // Name confliction.
        for (VTerminalIdentifier vtermId: terminals.keySet()) {
            input = new UpdateVterminalInputBuilder().
                setTenantName(vtermId.getTenantNameString()).
                setTerminalName(vtermId.getBridgeNameString()).
                setUpdateMode(VnodeUpdateMode.CREATE).
                build();
            checkRpcError(vtermSrv.updateVterminal(input),
                          RpcErrorTag.DATA_EXISTS, VtnErrorTag.CONFLICT);
        }

        // Errors should never affect existing vTerminals.
        vnet.verify();

        // Remove vtermId2 and vtermId3.
        assertEquals(vtconf1, terminals.remove(vtermId1));
        for (VTerminalIdentifier vtermId: terminals.keySet()) {
            removeVterminal(vtermId);
            tconf1.removeTerminal(vtermId.getBridgeNameString());
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
