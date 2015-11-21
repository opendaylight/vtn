/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.vnode;

import static org.opendaylight.vtn.manager.internal.util.vnode.VBridgeIdentifier.getMacTablePath;
import static org.opendaylight.vtn.manager.internal.vnode.VBridge.DEFAULT_AGE_INTERVAL;

import java.util.Objects;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.TxContext;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcOutputGenerator;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcUtils;
import org.opendaylight.vtn.manager.internal.util.vnode.VBridgeIdentifier;
import org.opendaylight.vtn.manager.internal.util.vnode.VTenantIdentifier;

import org.opendaylight.controller.md.sal.binding.api.ReadWriteTransaction;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeUpdateMode;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.mac.rev150907.vtn.mac.table.list.MacAddressTable;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.mac.rev150907.vtn.mac.table.list.MacAddressTableBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.UpdateVbridgeInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.UpdateVbridgeOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.UpdateVbridgeOutputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.VtnVbridgeConfig;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.port.mappable.bridge.BridgeStatus;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.vbridge.info.VbridgeConfig;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.vbridge.info.VbridgeConfigBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.vbridge.list.Vbridge;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.vbridge.list.VbridgeBuilder;

/**
 * {@code UpdateVbridgeTask} describes the MD-SAL datastore transaction task
 * that creates or updates the specified vBridge.
 *
 * @see  #create(UpdateVbridgeInput)
 */
public final class UpdateVbridgeTask
    extends UpdateVirtualNodeTask<Vbridge, VbridgeConfig, VBridgeIdentifier>
    implements RpcOutputGenerator<VtnUpdateType, UpdateVbridgeOutput> {
    /**
     * New configuration specified by the RPC.
     */
    private final VtnVbridgeConfig  config;

    /**
     * Construct a new task that creates or updates the specified vBridge.
     *
     * @param input  A {@link UpdateVbridgeInput} instance.
     * @return  A {@link UpdateVbridgeTask} instance associated with the task
     *          that creates or updates the specified vBridge.
     * @throws RpcException
     *     The given input contains invalid value.
     */
    public static UpdateVbridgeTask create(UpdateVbridgeInput input)
        throws RpcException {
        if (input == null) {
            throw RpcUtils.getNullInputException();
        }

        // Determine the target vBridge.
        VnodeUpdateMode mode = input.getUpdateMode();
        boolean find = (mode == VnodeUpdateMode.MODIFY);
        VBridgeIdentifier vbrId = VBridgeIdentifier.create(input, find);

        return new UpdateVbridgeTask(vbrId, input, mode);
    }

    /**
     * Construct a new instance.
     *
     * @param ident  The identifier for the target vBridge.
     * @param input  A {@link UpdateVbridgeInput} instance.
     * @param mode   A {@link VnodeUpdateMode} instance.
     * @throws RpcException
     *     The given input contains invalid value.
     */
    private UpdateVbridgeTask(VBridgeIdentifier ident,
                              UpdateVbridgeInput input, VnodeUpdateMode mode)
        throws RpcException {
        super(ident, mode, input.getOperation());
        config = input;
    }

    /**
     * Construct a new vBridge configuration.
     *
     * @param desc  A new description about the vBridge.
     * @param age   A new value for age-interval.
     * @return  A vBridge configuration.
     * @throws RpcException
     *    The given configuration is invalid.
     */
    private VbridgeConfig createConfig(String desc, Integer age)
        throws RpcException {
        VbridgeConfigBuilder cb = new VbridgeConfigBuilder().
            setDescription(desc);

        // Use default aging interval if null.
        Integer value = (age == null)
            ? Integer.valueOf(DEFAULT_AGE_INTERVAL) : age;
        try {
            cb.setAgeInterval(value);
        } catch (RuntimeException e) {
            throw RpcException.getBadArgumentException(
                "Invalid age-interval: " + value, e);
        }

        return cb.build();
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
        VBridgeIdentifier vbrId = getIdentifier();
        VTenantIdentifier vtnId = new VTenantIdentifier(vbrId.getTenantName());
        vtnId.fetch(ctx.getReadWriteTransaction());
    }

    /**
     * Create a new vBridge configuration as specified by the RPC.
     *
     * @return  A {@link VbridgeConfig} instance.
     * @throws RpcException
     *    The configuration specified by the RPC is invalid.
     */
    @Override
    protected VbridgeConfig createConfig() throws RpcException {
        return createConfig(config.getDescription(), config.getAgeInterval());
    }

    /**
     * Merge the vBridge configuration specified by the RPC with the current
     * configuration.
     *
     * @param cur  The current configuration of the target vBridge.
     * @return  A {@link VbridgeConfig} instance.
     * @throws RpcException
     *    The configuration specified by the RPC is invalid.
     */
    @Override
    protected VbridgeConfig mergeConfig(VbridgeConfig cur)
        throws RpcException {
        String desc = config.getDescription();
        if (desc == null) {
            desc = cur.getDescription();
        }

        Integer age = config.getAgeInterval();
        if (age == null) {
            age = cur.getAgeInterval();
        }

        return createConfig(desc, age);
    }

    /**
     * Return the vBridge configuration in the given vBridge.
     *
     * @param vnode  The vBridge container.
     * @return  The configuration for the given vBridge.
     */
    @Override
    protected VbridgeConfig getConfig(Vbridge vnode) {
        return vnode.getVbridgeConfig();
    }

    /**
     * Return the instance identifier for the configuration container in the
     * target vBridge.
     *
     * @param ident  The identifier for the target vBridge.
     * @return  The instance identifier for the vbridge-config.
     */
    @Override
    protected InstanceIdentifier<VbridgeConfig> getConfigPath(
        VBridgeIdentifier ident) {
        return ident.getIdentifierBuilder().child(VbridgeConfig.class).build();
    }

    /**
     * Determine whether the given two vBridge configurations are identical
     * or not.
     *
     * @param cur  The current configuration.
     * @param cfg  The configuration that is going to be applied.
     * @return  {@code true} only if the given two configurations are
     *           identical.
     */
    @Override
    protected boolean equalsConfig(VbridgeConfig cur, VbridgeConfig cfg) {
        return (Objects.equals(cur.getDescription(), cfg.getDescription()) &&
                cur.getAgeInterval().equals(cfg.getAgeInterval()));
    }

    /**
     * Create a new vBridge.
     *
     * @param ctx    A runtime context for transaction task.
     * @param ident  The identifier for the new vBridge.
     * @param cfg    The configuration for the new vBridge.
     * @throws VTNException  An error occurred.
     */
    @Override
    protected void create(TxContext ctx, VBridgeIdentifier ident,
                          VbridgeConfig cfg) throws VTNException {
        // Create initial bridge status.
        BridgeStatus bst = VirtualBridge.newBridgeStatus();

        // Create a new vBridge.
        Vbridge vbridge = new VbridgeBuilder().
            setName(ident.getBridgeName()).
            setVbridgeConfig(cfg).
            setBridgeStatus(bst).
            build();
        ReadWriteTransaction tx = ctx.getReadWriteTransaction();
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        tx.put(oper, ident.getIdentifier(), vbridge, false);

        // Create a MAC address table for a new vBridge.
        MacAddressTable mtable = new MacAddressTableBuilder().
            setName(ident.getBridgeNameString()).build();
        InstanceIdentifier<MacAddressTable> mpath = getMacTablePath(ident);
        tx.put(oper, mpath, mtable, false);
    }

    // RpcOutputGenerator

    /**
     * {@inheritDoc}
     */
    @Override
    public Class<UpdateVbridgeOutput> getOutputType() {
        return UpdateVbridgeOutput.class;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public UpdateVbridgeOutput createOutput(VtnUpdateType result) {
        return new UpdateVbridgeOutputBuilder().setStatus(result).build();
    }
}
