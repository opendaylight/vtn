/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.vnode;

import static org.opendaylight.vtn.manager.internal.util.vnode.VBridgeIdentifier.getTenantMacTablePath;

import java.util.Objects;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.TxContext;
import org.opendaylight.vtn.manager.internal.util.flow.FlowUtils;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcOutputGenerator;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcUtils;
import org.opendaylight.vtn.manager.internal.util.vnode.VTenantIdentifier;

import org.opendaylight.controller.md.sal.binding.api.ReadWriteTransaction;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.UpdateVtnInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.UpdateVtnOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.UpdateVtnOutputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.VtnVtenantConfig;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.vtn.info.VtenantConfig;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.vtn.info.VtenantConfigBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.vtns.Vtn;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.vtns.VtnBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeUpdateMode;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.mac.rev150907.mac.tables.TenantMacTable;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.mac.rev150907.mac.tables.TenantMacTableBuilder;

/**
 * {@code UpdateVtnTask} describes the MD-SAL datastore transaction task
 * that creates or updates the specified VTN.
 *
 * @see  #create(UpdateVtnInput)
 */
public final class UpdateVtnTask
    extends UpdateVirtualNodeTask<Vtn, VtenantConfig, VTenantIdentifier>
    implements RpcOutputGenerator<VtnUpdateType, UpdateVtnOutput> {
    /**
     * Default value for idle-timeout.
     */
    private static final int  DEFAULT_IDLE_TIMEOUT = 300;

    /**
     * Default value for hard-timeout.
     */
    private static final int  DEFAULT_HARD_TIMEOUT = 0;

    /**
     * New configuration specified by the RPC.
     */
    private final VtnVtenantConfig  config;

    /**
     * Construct a new task that creates or updates the specified VTN.
     *
     * @param input  A {@link UpdateVtnInput} instance.
     * @return  A {@link UpdateVtnTask} instance associated with the task
     *          that creates or updates the specified VTN.
     * @throws RpcException
     *     The given input contains invalid value.
     */
    public static UpdateVtnTask create(UpdateVtnInput input)
        throws RpcException {
        if (input == null) {
            throw RpcUtils.getNullInputException();
        }

        // Determine the target VTN.
        VnodeUpdateMode mode = input.getUpdateMode();
        boolean find = (mode == VnodeUpdateMode.MODIFY);
        VTenantIdentifier vtnId = VTenantIdentifier.create(
            input.getTenantName(), find);

        return new UpdateVtnTask(vtnId, input, mode);
    }

    /**
     * Construct a new instance.
     *
     * @param ident  The identifier for the target VTN.
     * @param input  A {@link UpdateVtnInput} instance.
     * @param mode   A {@link VnodeUpdateMode} instance.
     * @throws RpcException
     *     The given input contains invalid value.
     */
    private UpdateVtnTask(VTenantIdentifier ident, UpdateVtnInput input,
                          VnodeUpdateMode mode) throws RpcException {
        super(ident, mode, input.getOperation());
        config = input;
    }

    // UpdateVirtualNodeTask

    /**
     * This method does nothing because the VTN has no parent node.
     *
     * @param ctx  A runtime context for transaction task.
     */
    @Override
    protected void checkParent(TxContext ctx) {
    }

    /**
     * Create a new VTN configuration as specified by the RPC.
     *
     * @return  A {@link VtenantConfig} instance.
     * @throws RpcException
     *    The configuration specified by the RPC is invalid.
     */
    @Override
    protected VtenantConfig createConfig() throws RpcException {
        VtenantConfigBuilder cb = new VtenantConfigBuilder().
            setDescription(config.getDescription());

        Integer idle = config.getIdleTimeout();
        if (idle == null) {
            // Use default idle-timeout.
            idle = DEFAULT_IDLE_TIMEOUT;
        }

        Integer hard = config.getHardTimeout();
        if (hard == null) {
            // Use default hard-timeout.
            hard = DEFAULT_HARD_TIMEOUT;
        }

        // Verify the configuration.
        FlowUtils.verifyFlowTimeout(idle, hard, true);
        return cb.setIdleTimeout(idle).setHardTimeout(hard).build();
    }

    /**
     * Merge the VTN configuration specified by the RPC with the current
     * configuration.
     *
     * @param cur  The current configuration of the target VTN.
     * @return  A {@link VtenantConfig} instance.
     * @throws RpcException
     *    The configuration specified by the RPC is invalid.
     */
    @Override
    protected VtenantConfig mergeConfig(VtenantConfig cur) throws RpcException {
        VtenantConfigBuilder cb = new VtenantConfigBuilder(cur);
        String desc = config.getDescription();
        if (desc != null) {
            cb.setDescription(desc);
        }

        Integer idle = config.getIdleTimeout();
        if (idle == null) {
            idle = cb.getIdleTimeout();
        } else {
            cb.setIdleTimeout(idle);
        }

        Integer hard = config.getHardTimeout();
        if (hard == null) {
            hard = cb.getHardTimeout();
        } else {
            cb.setHardTimeout(hard);
        }

        // Verify the configuration.
        FlowUtils.verifyFlowTimeout(idle, hard, true);
        return cb.build();
    }

    /**
     * Return the VTN configuration in the given VTN.
     *
     * @param vnode  The VTN container.
     * @return  The configuration for the given VTN.
     */
    @Override
    protected VtenantConfig getConfig(Vtn vnode) {
        return vnode.getVtenantConfig();
    }

    /**
     * Return the instance identifier for the configuration container in the
     * target VTN.
     *
     * @param ident  The identifier for the target VTN.
     * @return  The instance identifier for the vtenant-config.
     */
    @Override
    protected InstanceIdentifier<VtenantConfig> getConfigPath(
        VTenantIdentifier ident) {
        return ident.getIdentifierBuilder().child(VtenantConfig.class).build();
    }

    /**
     * Determine whether the given two VTN configurations are identical or not.
     *
     * @param cur  The current configuration.
     * @param cfg  The configuration that is going to be applied.
     * @return  {@code true} only if the given two configurations are
     *           identical.
     */
    @Override
    protected boolean equalsConfig(VtenantConfig cur, VtenantConfig cfg) {
        return (Objects.equals(cur.getDescription(), cfg.getDescription()) &&
                cur.getIdleTimeout().equals(cfg.getIdleTimeout()) &&
                cur.getHardTimeout().equals(cfg.getHardTimeout()));
    }

    /**
     * Create a new VTN.
     *
     * @param ctx    A runtime context for transaction task.
     * @param ident  The identifier for the new VTN.
     * @param cfg    The configuration for the new VTN.
     * @throws VTNException  An error occurred.
     */
    @Override
    protected void create(TxContext ctx, VTenantIdentifier ident,
                          VtenantConfig cfg) throws VTNException {
        // Create a new VTN.
        Vtn vtn = new VtnBuilder().
            setName(ident.getTenantName()).
            setVtenantConfig(cfg).
            build();
        ReadWriteTransaction tx = ctx.getReadWriteTransaction();
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        tx.put(oper, ident.getIdentifier(), vtn, true);

        // Prepare a list of MAC address tables for a new VTN.
        InstanceIdentifier<TenantMacTable> mpath =
            getTenantMacTablePath(ident);
        TenantMacTable mtable = new TenantMacTableBuilder().
            setName(ident.getTenantNameString()).
            build();
        tx.put(oper, mpath, mtable, true);
    }

    // RpcOutputGenerator

    /**
     * {@inheritDoc}
     */
    @Override
    public Class<UpdateVtnOutput> getOutputType() {
        return UpdateVtnOutput.class;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public UpdateVtnOutput createOutput(VtnUpdateType result) {
        return new UpdateVtnOutputBuilder().setStatus(result).build();
    }
}
