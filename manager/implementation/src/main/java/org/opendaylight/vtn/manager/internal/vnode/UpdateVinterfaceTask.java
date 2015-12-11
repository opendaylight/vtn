/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.vnode;

import java.util.Collections;
import java.util.Objects;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.TxContext;
import org.opendaylight.vtn.manager.internal.util.MiscUtils;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcOutputGenerator;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcUtils;
import org.opendaylight.vtn.manager.internal.util.vnode.BridgeIdentifier;
import org.opendaylight.vtn.manager.internal.util.vnode.VBridgeIfIdentifier;
import org.opendaylight.vtn.manager.internal.util.vnode.VInterfaceIdentifier;
import org.opendaylight.vtn.manager.internal.util.vnode.VTerminalIfIdentifier;

import org.opendaylight.controller.md.sal.binding.api.ReadWriteTransaction;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeState;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeUpdateMode;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.VtnPortMappableBridge;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.port.mappable.bridge.BridgeStatus;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.port.mappable.bridge.BridgeStatusBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.vbridge.list.Vbridge;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.UpdateVinterfaceInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.UpdateVinterfaceOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.UpdateVinterfaceOutputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.VtnVinterfaceConfig;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.vtn.mappable.vinterface.VinterfaceStatus;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.vtn.mappable.vinterface.VinterfaceStatusBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.vtn.mappable.vinterface.list.Vinterface;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.vtn.mappable.vinterface.list.VinterfaceBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.vtn.vinterface.info.VinterfaceConfig;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.vtn.vinterface.info.VinterfaceConfigBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vterminal.rev150907.vtn.vterminal.list.Vterminal;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vterminal.rev150907.vtn.vterminal.list.VterminalBuilder;

/**
 * {@code UpdateVinterfaceTask} describes the MD-SAL datastore transaction task
 * that creates or updates the specified virtual interface.
 *
 * @param <B>  The type of the virtual bridge that contains the target
 *             virtual interface.
 * @see  #create(UpdateVinterfaceInput)
 */
public abstract class UpdateVinterfaceTask<B extends VtnPortMappableBridge>
    extends UpdateVirtualNodeTask
            <Vinterface, VinterfaceConfig, VInterfaceIdentifier<B>>
    implements RpcOutputGenerator<VtnUpdateType, UpdateVinterfaceOutput> {
    /**
     * New configuration specified by the RPC.
     */
    private final VtnVinterfaceConfig  config;

    /**
     * The identifier for the parent virtual bridge.
     */
    private final BridgeIdentifier<B>  parentIdentifier;

    /**
     * The parent virtual bridge.
     */
    private B  parentBridge;

    /**
     * The enabled status to be applied.
     *
     * <ul>
     *   <li>This field is used only for updating existing interface.</li>
     *   <li>{@code null} implies the enabled status is unchanged.</li>
     * </ul>
     */
    private Boolean  newEnabled;

    /**
     * Construct a new task that creates or updates the specified virtual
     * interface.
     *
     * @param input  A {@link UpdateVinterfaceInput} instance.
     * @return  A {@link UpdateVinterfaceTask} instance associated with the
     *          task that creates or updates the specified virtual interface.
     * @throws RpcException
     *     The given input contains invalid value.
     */
    public static UpdateVinterfaceTask<?> create(UpdateVinterfaceInput input)
        throws RpcException {
        if (input == null) {
            throw RpcUtils.getNullInputException();
        }

        VnodeUpdateMode mode = input.getUpdateMode();
        boolean find = (mode == VnodeUpdateMode.MODIFY);

        // Determine the target virtual interface.
        String tname = input.getTenantName();
        String bname = input.getBridgeName();
        String iname = input.getInterfaceName();
        UpdateVinterfaceTask<?> task;
        if (bname == null) {
            String tmname = input.getTerminalName();
            VTerminalIfIdentifier ifId = VTerminalIfIdentifier.
                create(tname, tmname, iname, find);
            task = new VTerminalTask(ifId, input, mode);
        } else {
            VBridgeIfIdentifier ifId = VBridgeIfIdentifier.
                create(tname, bname, iname, find);
            task = new VBridgeTask(ifId, input, mode);
        }

        return task;
    }

    /**
     * {@code VBridgeTask} describes the MD-SAL datastore transaction task
     * that creates or updates the specified virtual interface inside the
     * vBridge.
     *
     * @see  UpdateVinterfaceTask#create(UpdateVinterfaceInput)
     */
    private static final class VBridgeTask
        extends UpdateVinterfaceTask<Vbridge> {
        /**
         * Construct a new instance.
         *
         * @param ident  The identifier for the target virtual interface.
         * @param input  A {@link UpdateVinterfaceInput} instance.
         * @param mode   A {@link VnodeUpdateMode} instance.
         * @throws RpcException
         *     The given input contains invalid value.
         */
        private VBridgeTask(VBridgeIfIdentifier ident,
                            UpdateVinterfaceInput input,
                            VnodeUpdateMode mode) throws RpcException {
            super(ident, input, mode);
        }

        /**
         * Add a new virtual interface in the vBridge.
         *
         * @param ctx    A runtime context for transaction task.
         * @param ident  The identifier for the new virtual interface.
         * @param vintf  A new virtual interface to be added.
         * @throws VTNException  An error occurred.
         */
        @Override
        protected void addInterface(
            TxContext ctx, VInterfaceIdentifier<Vbridge> ident,
            Vinterface vintf) throws VTNException {
            // Put the specified vBridge interface.
            ReadWriteTransaction tx = ctx.getReadWriteTransaction();
            LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
            tx.put(oper, ident.getIdentifier(), vintf, false);

            // Update the status of the parent vBridge.
            BridgeIdentifier<Vbridge> parentId = getParentIdentifier();
            VBridge vbr = new VBridge(parentId, parentId.fetch(tx));
            vbr.putState(ctx);
        }

        /**
         * Construct a new cache object for the parent vBridge.
         *
         * @param ident   The identifier for the parent vBridge.
         * @param parent  The parent vBridge.
         * @return  A new cache object for the parent vBridge.
         */
        @Override
        protected VBridge newBridge(BridgeIdentifier<Vbridge> ident,
                                    Vbridge parent) {
            return new VBridge(ident, parent);
        }
    }

    /**
     * {@code VTerminalTask} describes the MD-SAL datastore transaction task
     * that creates or updates the specified virtual interface inside the
     * vTerminal.
     *
     * @see  UpdateVinterfaceTask#create(UpdateVinterfaceInput)
     */
    private static final class VTerminalTask
        extends UpdateVinterfaceTask<Vterminal> {
        /**
         * Construct a new instance.
         *
         * @param ident  The identifier for the target virtual interface.
         * @param input  A {@link UpdateVinterfaceInput} instance.
         * @param mode   A {@link VnodeUpdateMode} instance.
         * @throws RpcException
         *     The given input contains invalid value.
         */
        private VTerminalTask(VTerminalIfIdentifier ident,
                              UpdateVinterfaceInput input,
                              VnodeUpdateMode mode) throws RpcException {
            super(ident, input, mode);
        }

        /**
         * Add a new virtual interface in the vTerminal.
         *
         * @param ctx    A runtime context for transaction task.
         * @param ident  The identifier for the new virtual interface.
         * @param vintf  A new virtual interface to be added.
         * @throws VTNException  An error occurred.
         */
        @Override
        protected void addInterface(
            TxContext ctx, VInterfaceIdentifier<Vterminal> ident,
            Vinterface vintf) throws VTNException {
            // We need to update the vterminal container at once in order to
            // prevent from creating more than one interfaces.
            VterminalBuilder builder = new VterminalBuilder();
            builder.fieldsFrom(getParentBridge());
            if (!MiscUtils.isEmpty(builder.getVinterface())) {
                throw RpcException.getDataExistsException(
                    "vTerminal can have only one interface");
            }

            // Update the vTerminal status.
            // The given interface state can be used as the vTerminal state
            // as long as the interface is enabled because vTerminal can
            // contains only one interface.
            VinterfaceConfig iconf = vintf.getVinterfaceConfig();
            VnodeState bstate = (iconf.isEnabled().booleanValue())
                ? vintf.getVinterfaceStatus().getState()
                : VnodeState.UNKNOWN;
            BridgeStatus bst = builder.getBridgeStatus();
            bst = new BridgeStatusBuilder(bst).setState(bstate).build();

            // Update the vTerminal.
            Vterminal vterminal = builder.
                setBridgeStatus(bst).
                setVinterface(Collections.singletonList(vintf)).
                build();
            BridgeIdentifier<Vterminal> parentId = getParentIdentifier();
            ReadWriteTransaction tx = ctx.getReadWriteTransaction();
            LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
            tx.put(oper, parentId.getIdentifier(), vterminal, false);
        }

        /**
         * Construct a new cache object for the parent vTerminal.
         *
         * @param ident   The identifier for the parent vTerminal.
         * @param parent  The parent vTerminal.
         * @return  A new cache object for the parent vTerminal.
         */
        @Override
        protected VTerminal newBridge(BridgeIdentifier<Vterminal> ident,
                                      Vterminal parent) {
            return new VTerminal(ident, parent);
        }
    }

    /**
     * Construct a new instance.
     *
     * @param ident  The identifier for the target virtual interface.
     * @param input  A {@link UpdateVinterfaceInput} instance.
     * @param mode   A {@link VnodeUpdateMode} instance.
     * @throws RpcException
     *     The given input contains invalid value.
     */
    private UpdateVinterfaceTask(
        VInterfaceIdentifier<B> ident, UpdateVinterfaceInput input,
        VnodeUpdateMode mode) throws RpcException {
        super(ident, mode, input.getOperation());
        config = input;
        parentIdentifier = ident.getBridgeIdentifier();
    }

    /**
     * Return the identifier for the parent bridge.
     *
     * @return  The identifier for the parent bridge.
     */
    protected final BridgeIdentifier<B> getParentIdentifier() {
        return parentIdentifier;
    }

    /**
     * Return the parent virtual bridge fetched by the call of
     * {@link #checkParent(TxContext)}.
     *
     * @return  The parent virtual bridge.
     */
    protected final B getParentBridge() {
        return parentBridge;
    }

    /**
     * Construct a new virtual interface configuration.
     *
     * @param desc  A new description about the virtual interface.
     * @param en    A boolean value which determines the virtual interface
     *              to be enabled or not.
     * @return  A virtual interface configuration.
     */
    private VinterfaceConfig createConfig(String desc, Boolean en) {
        // Enable the interface by default.
        Boolean e = en;
        if (e == null) {
            e = Boolean.TRUE;
        }
        return new VinterfaceConfigBuilder().
            setDescription(desc).
            setEnabled(e).
            build();
    }

    /**
     * Add a new virtual interface.
     *
     * @param ctx    A runtime context for transaction task.
     * @param ident  The identifier for the new virtual interface.
     * @param vintf  A new virtual interface to be added.
     * @throws VTNException  An error occurred.
     */
    protected abstract void addInterface(
        TxContext ctx, VInterfaceIdentifier<B> ident, Vinterface vintf)
        throws VTNException;

    /**
     * Construct a new cache object for the parent virtual bridge.
     *
     * @param ident   The identifier for the parent virtual bridge.
     * @param parent  The parent virtual bridge.
     * @return  A new cache object for the parent virtual bridge.
     */
    protected abstract VirtualBridge<B> newBridge(BridgeIdentifier<B> ident,
                                                  B parent);

    // UpdateVirtualNodeTask

    /**
     * Invoked when the configuration for the target virtual interface has been
     * changed.
     *
     * @param ctx  A runtime context for transaction task.
     * @throws VTNException  An error occurred.
     */
    @Override
    protected void onUpdated(TxContext ctx) throws VTNException {
        if (newEnabled != null) {
            // Read the parent virtual bridge.
            // This should contain the updated virtual interface.
            ReadWriteTransaction tx = ctx.getReadWriteTransaction();
            B parent = parentIdentifier.fetch(tx);
            VirtualBridge<B> bridge = newBridge(parentIdentifier, parent);

            // Change the status of the target virtual interface, and
            // update the status of the parent bridge.
            boolean en = newEnabled.booleanValue();
            bridge.setInterfaceEnabled(ctx, getIdentifier(), en);
        }
    }

    /**
     * Ensure that the parent virtual bridge is present.
     *
     * @param ctx  A runtime context for transaction task.
     * @throws VTNException  An error occurred.
     */
    @Override
    protected final void checkParent(TxContext ctx) throws VTNException {
        parentBridge = parentIdentifier.fetch(ctx.getReadWriteTransaction());
    }

    /**
     * Create a new virtual interface configuration as specified by the RPC.
     *
     * @return  A {@link VinterfaceConfig} instance.
     */
    @Override
    protected final VinterfaceConfig createConfig() {
        return createConfig(config.getDescription(), config.isEnabled());
    }

    /**
     * Merge the virtual interface configuration specified by the RPC with
     * the current configuration.
     *
     * @param cur  The current configuration of the target virtual interface.
     * @return  A {@link VinterfaceConfig} instance.
     */
    @Override
    protected final VinterfaceConfig mergeConfig(VinterfaceConfig cur)
        throws RpcException {
        String desc = config.getDescription();
        if (desc == null) {
            desc = cur.getDescription();
        }

        Boolean en = config.isEnabled();
        if (en == null) {
            en = cur.isEnabled();
        }

        return createConfig(desc, en);
    }

    /**
     * Return the virtual interface configuration in the given virtual
     * interface.
     *
     * @param vnode  The virtual interface container.
     * @return  The configuration for the given virtual interface.
     */
    @Override
    protected final VinterfaceConfig getConfig(Vinterface vnode) {
        return vnode.getVinterfaceConfig();
    }

    /**
     * Return the instance identifier for the configuration container in the
     * target virtual interface.
     *
     * @param ident  The identifier for the target virtual interface.
     * @return  The instance identifier for the vinterface-config.
     */
    @Override
    protected final InstanceIdentifier<VinterfaceConfig> getConfigPath(
        VInterfaceIdentifier<B> ident) {
        return ident.getIdentifierBuilder().
            child(VinterfaceConfig.class).
            build();
    }

    /**
     * Determine whether the given two virtual interface configurations are
     * identical or not.
     *
     * @param cur  The current configuration.
     * @param cfg  The configuration that is going to be applied.
     * @return  {@code true} only if the given two configurations are
     *           identical.
     */
    @Override
    protected final boolean equalsConfig(VinterfaceConfig cur,
                                         VinterfaceConfig cfg) {
        Boolean curEn = cur.isEnabled();
        Boolean newEn = cfg.isEnabled();
        boolean ret = (curEn.booleanValue() == newEn.booleanValue());
        if (ret) {
            ret = Objects.equals(cur.getDescription(),
                                 cfg.getDescription());
            newEnabled = null;
        } else {
            newEnabled = newEn;
        }

        return ret;
    }

    /**
     * Create a new virtual interface.
     *
     * @param ctx    A runtime context for transaction task.
     * @param ident  The identifier for the new virtual interface.
     * @param cfg    The configuration for the new virtual interface.
     * @throws VTNException  An error occurred.
     */
    @Override
    protected final void create(TxContext ctx, VInterfaceIdentifier<B> ident,
                                VinterfaceConfig cfg) throws VTNException {
        // Create initial virtual interface status.
        VnodeState state = (cfg.isEnabled().booleanValue())
            ? VnodeState.UNKNOWN
            : VnodeState.DOWN;
        VinterfaceStatus ist = new VinterfaceStatusBuilder().
            setState(state).
            setEntityState(VnodeState.UNKNOWN).
            build();

        // Create a new virtual interface.
        Vinterface vintf = new VinterfaceBuilder().
            setName(ident.getInterfaceName()).
            setVinterfaceConfig(cfg).
            setVinterfaceStatus(ist).
            build();
        addInterface(ctx, ident, vintf);
    }

    // RpcOutputGenerator

    /**
     * {@inheritDoc}
     */
    @Override
    public final Class<UpdateVinterfaceOutput> getOutputType() {
        return UpdateVinterfaceOutput.class;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public final UpdateVinterfaceOutput createOutput(VtnUpdateType result) {
        return new UpdateVinterfaceOutputBuilder().setStatus(result).build();
    }
}
