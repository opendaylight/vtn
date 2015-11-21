/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.vnode;

import java.util.Objects;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.TxContext;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcOutputGenerator;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcUtils;
import org.opendaylight.vtn.manager.internal.util.vnode.VTenantIdentifier;
import org.opendaylight.vtn.manager.internal.util.vnode.VTerminalIdentifier;

import org.opendaylight.controller.md.sal.binding.api.ReadWriteTransaction;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeUpdateMode;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.port.mappable.bridge.BridgeStatus;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vterminal.rev150907.UpdateVterminalInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vterminal.rev150907.UpdateVterminalOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vterminal.rev150907.UpdateVterminalOutputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vterminal.rev150907.VtnVterminalConfig;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vterminal.rev150907.vtn.vterminal.info.VterminalConfig;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vterminal.rev150907.vtn.vterminal.info.VterminalConfigBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vterminal.rev150907.vtn.vterminal.list.Vterminal;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vterminal.rev150907.vtn.vterminal.list.VterminalBuilder;

/**
 * {@code UpdateVterminalTask} describes the MD-SAL datastore transaction task
 * that creates or updates the specified vTerminal.
 *
a * @see  #create(UpdateVterminalInput)
 */
public final class UpdateVterminalTask
    extends UpdateVirtualNodeTask<Vterminal, VterminalConfig,
                                  VTerminalIdentifier>
    implements RpcOutputGenerator<VtnUpdateType, UpdateVterminalOutput> {
    /**
     * New configuration specified by the RPC.
     */
    private final VtnVterminalConfig  config;

    /**
     * Construct a new task that creates or updates the specified vTerminal.
     *
     * @param input  A {@link UpdateVterminalInput} instance.
     * @return  A {@link UpdateVterminalTask} instance associated with the task
     *          that creates or updates the specified vTerminal.
     * @throws RpcException
     *     The given input contains invalid value.
     */
    public static UpdateVterminalTask create(UpdateVterminalInput input)
        throws RpcException {
        if (input == null) {
            throw RpcUtils.getNullInputException();
        }

        // Determine the target vTerminal.
        VnodeUpdateMode mode = input.getUpdateMode();
        boolean find = (mode == VnodeUpdateMode.MODIFY);
        VTerminalIdentifier vtermId = VTerminalIdentifier.create(input, find);

        return new UpdateVterminalTask(vtermId, input, mode);
    }

    /**
     * Construct a new instance.
     *
     * @param ident  The identifier for the target vTerminal.
     * @param input  A {@link UpdateVterminalInput} instance.
     * @param mode   A {@link VnodeUpdateMode} instance.
     * @throws RpcException
     *     The given input contains invalid value.
     */
    private UpdateVterminalTask(
        VTerminalIdentifier ident, UpdateVterminalInput input,
        VnodeUpdateMode mode) throws RpcException {
        super(ident, mode, input.getOperation());
        config = input;
    }

    /**
     * Construct a new vTerminal configuration.
     *
     * @param desc  A new description about the vTerminal.
     * @return  A vTerminal configuration.
     */
    private VterminalConfig createConfig(String desc) {
        return new VterminalConfigBuilder().
            setDescription(desc).
            build();
    }

    // UpdateVirtualNodeTask

    /**
     * Ensure that the parent VTN is present.
     *
     * @param ctx  A runtime context for transaction task.
     * @throws VTNException  An error occurred.
     */
    @Override
    protected void checkParent(TxContext ctx) throws VTNException {
        VTerminalIdentifier vtermId = getIdentifier();
        VTenantIdentifier vtnId =
            new VTenantIdentifier(vtermId.getTenantName());
        vtnId.fetch(ctx.getReadWriteTransaction());
    }

    /**
     * Create a new vTerminal configuration as specified by the RPC.
     *
     * @return  A {@link VterminalConfig} instance.
     * @throws RpcException
     *    The configuration specified by the RPC is invalid.
     */
    @Override
    protected VterminalConfig createConfig() throws RpcException {
        return createConfig(config.getDescription());
    }

    /**
     * Merge the vTerminal configuration specified by the RPC with the current
     * configuration.
     *
     * @param cur  The current configuration of the target vTerminal.
     * @return  A {@link VterminalConfig} instance.
     */
    @Override
    protected VterminalConfig mergeConfig(VterminalConfig cur) {
        String desc = config.getDescription();
        if (desc == null) {
            desc = cur.getDescription();
        }

        return createConfig(desc);
    }

    /**
     * Return the vTerminal configuration in the given vTerminal.
     *
     * @param vnode  The vTerminal container.
     * @return  The configuration for the given vTerminal.
     */
    @Override
    protected VterminalConfig getConfig(Vterminal vnode) {
        return vnode.getVterminalConfig();
    }

    /**
     * Return the instance identifier for the configuration container in the
     * target vTerminal.
     *
     * @param ident  The identifier for the target vTerminal.
     * @return  The instance identifier for the vterminal-config.
     */
    @Override
    protected InstanceIdentifier<VterminalConfig> getConfigPath(
        VTerminalIdentifier ident) {
        return ident.getIdentifierBuilder().
            child(VterminalConfig.class).
            build();
    }

    /**
     * Determine whether the given two vTerminal configurations are identical
     * or not.
     *
     * @param cur  The current configuration.
     * @param cfg  The configuration that is going to be applied.
     * @return  {@code true} only if the given two configurations are
     *           identical.
     */
    @Override
    protected boolean equalsConfig(VterminalConfig cur, VterminalConfig cfg) {
        return Objects.equals(cur.getDescription(), cfg.getDescription());
    }

    /**
     * Create a new vTerminal.
     *
     * @param ctx    A runtime context for transaction task.
     * @param ident  The identifier for the new vTerminal.
     * @param cfg    The configuration for the new vTerminal.
     * @throws VTNException  An error occurred.
     */
    @Override
    protected void create(TxContext ctx, VTerminalIdentifier ident,
                          VterminalConfig cfg) throws VTNException {
        // Create initial bridge status.
        BridgeStatus bst = VirtualBridge.newBridgeStatus();

        // Create a new vTerminal.
        Vterminal vterminal = new VterminalBuilder().
            setName(ident.getBridgeName()).
            setVterminalConfig(cfg).
            setBridgeStatus(bst).
            build();
        ReadWriteTransaction tx = ctx.getReadWriteTransaction();
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        tx.put(oper, ident.getIdentifier(), vterminal, false);
    }

    // RpcOutputGenerator

    /**
     * {@inheritDoc}
     */
    @Override
    public Class<UpdateVterminalOutput> getOutputType() {
        return UpdateVterminalOutput.class;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public UpdateVterminalOutput createOutput(VtnUpdateType result) {
        return new UpdateVterminalOutputBuilder().setStatus(result).build();
    }
}
