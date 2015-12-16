/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.util.vnode;

import static org.junit.Assert.assertEquals;

import static org.opendaylight.vtn.manager.it.util.ModelDrivenTestBase.getRpcOutput;
import static org.opendaylight.vtn.manager.it.util.ModelDrivenTestBase.getRpcResult;
import static org.opendaylight.vtn.manager.it.util.TestBase.createVnodeName;

import java.util.Random;

import org.opendaylight.vtn.manager.it.util.VTNServices;

import org.opendaylight.controller.md.sal.binding.api.ReadTransaction;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeState;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeUpdateMode;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateOperationType;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vterminal.rev150907.RemoveVterminalInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vterminal.rev150907.RemoveVterminalInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vterminal.rev150907.UpdateVterminalInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vterminal.rev150907.UpdateVterminalInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vterminal.rev150907.VtnVterminalService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vterminal.rev150907.vtn.vterminal.info.VterminalConfig;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vterminal.rev150907.vtn.vterminal.list.Vterminal;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vterminal.rev150907.vtn.vterminal.list.VterminalBuilder;

/**
 * {@code VTerminalConfig} describes the configuration of a vTerminal.
 */
public final class VTerminalConfig
    extends BridgeConfig<VTerminalConfig, Vterminal> {
    /**
     * Remove the specified vTerminal.
     *
     * @param service  The vtn-vterminal RPC service.
     * @param ident    The identifier for the vTerminal.
     */
    public static void removeVterminal(
        VtnVterminalService service, VTerminalIdentifier ident) {
        RemoveVterminalInput input = new RemoveVterminalInputBuilder().
            setTenantName(ident.getTenantNameString()).
            setTerminalName(ident.getBridgeNameString()).
            build();

        assertEquals(null, getRpcOutput(service.removeVterminal(input), true));
    }

    /**
     * Construct a new instance with default values.
     */
    public VTerminalConfig() {
    }

    /**
     * Construct a new instance.
     *
     * @param desc  The description about the vTerminal.
     */
    public VTerminalConfig(String desc) {
        super(desc);
    }

    /**
     * Create a new input builder for update-vterminal RPC.
     *
     * @return  An {@link UpdateVterminalInputBuilder} instance.
     */
    public UpdateVterminalInputBuilder newInputBuilder() {
        return new UpdateVterminalInputBuilder().
            setDescription(getDescription());
    }

    /**
     * Add random configuration to this vTerminal configuration using the given
     * random generator.
     *
     * @param rand  A pseudo random generator.
     * @return  This instance.
     */
    public VTerminalConfig add(Random rand) {
        if (getInterfaceCount() == 0) {
            addInterface(createVnodeName("if_", rand), new VInterfaceConfig());
        }

        return this;
    }

    /**
     * Update the specified vTerminal.
     *
     * @param service  The vtn-vterminal service.
     * @param ident    The identifier for the vTerminal.
     * @param mode     A {@link VnodeUpdateMode} instance.
     * @param op       A {@link VtnUpdateOperationType} instance.
     * @return  A {@link VtnUpdateType} instance returned by the RPC.
     */
    public VtnUpdateType update(VtnVterminalService service,
                                VTerminalIdentifier ident,
                                VnodeUpdateMode mode,
                                VtnUpdateOperationType op) {
        UpdateVterminalInput input = newInputBuilder().
            setTenantName(ident.getTenantNameString()).
            setTerminalName(ident.getBridgeNameString()).
            setUpdateMode(mode).
            setOperation(op).
            build();
        return getRpcResult(service.updateVterminal(input));
    }

    /**
     * Verify the given vTerminal.
     *
     * @param rtx    A read-only MD-SAL datastore transaction.
     * @param ident  The identifier for the vTerminal.
     * @param vterm  The vTerminal to be verified.
     */
    public void verify(ReadTransaction rtx, VTerminalIdentifier ident,
                       Vterminal vterm) {
        // Verify the configuration.
        VterminalConfig vtermc = vterm.getVterminalConfig();
        assertEquals(getDescription(), vtermc.getDescription());

        // Verify the virtual interfaces and the bridge status.
        verify(rtx, ident, vterm, VnodeState.UNKNOWN);
    }

    /**
     * Apply all the configurations in this instance.
     *
     * @param service  A {@link VTNServices} instance.
     * @param ident    The identifier for the vTerminal.
     * @param current  The current vTerminal.
     */
    public void apply(VTNServices service, VTerminalIdentifier ident,
                      Vterminal current) {
        // Create the vTerminal if not present.
        UpdateVterminalInput input = newInputBuilder().
            setTenantName(ident.getTenantNameString()).
            setTerminalName(ident.getBridgeNameString()).
            setUpdateMode(VnodeUpdateMode.UPDATE).
            setOperation(VtnUpdateOperationType.SET).
            build();
        getRpcOutput(service.getVterminalService().updateVterminal(input));

        Vterminal vterm = (current == null)
            ? new VterminalBuilder().build()
            : current;

        // Apply the virtual interface configuration.
        applyInterfaces(service, ident, vterm);
    }
}
