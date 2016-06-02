/*
 * Copyright (c) 2015, 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.flow.cond;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Objects;
import java.util.Set;
import java.util.concurrent.Future;

import javax.annotation.Nonnull;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.TxContext;
import org.opendaylight.vtn.manager.internal.TxTask;
import org.opendaylight.vtn.manager.internal.VTNManagerProvider;
import org.opendaylight.vtn.manager.internal.VTNSubSystem;
import org.opendaylight.vtn.manager.internal.util.ChangedData;
import org.opendaylight.vtn.manager.internal.util.CompositeAutoCloseable;
import org.opendaylight.vtn.manager.internal.util.DataStoreUtils;
import org.opendaylight.vtn.manager.internal.util.IdentifiedData;
import org.opendaylight.vtn.manager.internal.util.MiscUtils;
import org.opendaylight.vtn.manager.internal.util.MultiDataStoreListener;
import org.opendaylight.vtn.manager.internal.util.XmlConfigFile;
import org.opendaylight.vtn.manager.internal.util.concurrent.VTNFuture;
import org.opendaylight.vtn.manager.internal.util.flow.cond.FlowCondUtils;
import org.opendaylight.vtn.manager.internal.util.flow.cond.VTNFlowCondition;
import org.opendaylight.vtn.manager.internal.util.flow.match.VTNMatch;
import org.opendaylight.vtn.manager.internal.util.log.VTNLogLevel;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcFuture;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcUtils;
import org.opendaylight.vtn.manager.internal.util.tx.AbstractTxTask;

import org.opendaylight.controller.md.sal.binding.api.ReadWriteTransaction;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;
import org.opendaylight.controller.sal.binding.api.RpcProviderRegistry;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;
import org.opendaylight.yangtools.yang.common.RpcResult;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.ClearFlowConditionOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.RemoveFlowConditionInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.RemoveFlowConditionMatchInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.RemoveFlowConditionMatchOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.SetFlowConditionInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.SetFlowConditionMatchInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.SetFlowConditionMatchOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.SetFlowConditionOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.VtnFlowConditionService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.VtnFlowConditions;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.VtnFlowConditionsBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.VtnMatchFields;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.vtn.flow.cond.config.VtnFlowMatch;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.vtn.flow.conditions.VtnFlowCondition;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;

/**
 * Flow condition manager.
 */
public final class FlowCondManager
    extends MultiDataStoreListener<VtnFlowCondition, FlowCondChange>
    implements VTNSubSystem, VtnFlowConditionService {
    /**
     * Logger instance.
     */
    private static final Logger  LOG =
        LoggerFactory.getLogger(FlowCondManager.class);

    /**
     * VTN Manager provider service.
     */
    private final VTNManagerProvider  vtnProvider;

    /**
     * MD-SAL transaction task to load flow condition configurations.
     *
     * <p>
     *   This task returns current {@link VtnFlowConditions} instance.
     * </p>
     */
    private static class FlowCondLoadTask
        extends AbstractTxTask<VtnFlowConditions> {
        /**
         * Resume the configuration for the given flow condition.
         *
         * @param ctx     MD-SAL datastore transaction context.
         * @param vlist   A list of {@link VtnFlowCondition} instance to store
         *                resumed configuration.
         * @param name    The name of the flow condition.
         * @param vfcond  A {@link VTNFlowCondition} instance.
         */
        private void resume(TxContext ctx, List<VtnFlowCondition> vlist,
                            String name, VTNFlowCondition vfcond) {
            try {
                vfcond.verify();
                String condName = vfcond.getIdentifier();
                if (!name.equals(condName)) {
                    String msg = new StringBuilder("Unexpected name: ").
                        append(condName).append(": expected=").append(name).
                        toString();
                    throw new IllegalArgumentException(msg);
                }
                vlist.add(vfcond.toVtnFlowConditionBuilder().build());
                ctx.log(LOG, VTNLogLevel.DEBUG,
                        "{}: Flow condition has been loaded.", name);
            } catch (RpcException | RuntimeException e) {
                ctx.log(LOG, VTNLogLevel.WARN, e,
                        "Ignore invalid flow condition configuration: %s", e);
            }
        }

        /**
         * {@inheritDoc}
         */
        @Override
        public VtnFlowConditions execute(TxContext ctx) throws VTNException {
            // Load configuration from file.
            XmlConfigFile.Type ftype = XmlConfigFile.Type.FLOWCOND;
            List<VtnFlowCondition> vlist = new ArrayList<>();
            for (String key: XmlConfigFile.getKeys(ftype)) {
                VTNFlowCondition vfcond = XmlConfigFile.
                    load(ftype, key, VTNFlowCondition.class);
                if (vfcond != null) {
                    resume(ctx, vlist, key, vfcond);
                }
            }

            VtnFlowConditionsBuilder builder = new VtnFlowConditionsBuilder();
            if (!vlist.isEmpty()) {
                builder.setVtnFlowCondition(vlist);
            }
            InstanceIdentifier<VtnFlowConditions> path =
                InstanceIdentifier.create(VtnFlowConditions.class);

            // Remove old configuration, and install loaded configuration.
            LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
            ReadWriteTransaction tx = ctx.getReadWriteTransaction();
            DataStoreUtils.delete(tx, oper, path);

            VtnFlowConditions conditions = builder.build();
            tx.put(oper, path, conditions, true);

            return conditions;
        }
    }

    /**
     * MD-SAL transaction task to save current flow condition configurations.
     *
     * <p>
     *   This task returns current {@link VtnFlowConditions} instance.
     * </p>
     */
    private static class FlowCondSaveTask
        extends AbstractTxTask<VtnFlowConditions> {
        /**
         * Set {@code true} if the root container has been created.
         */
        private boolean  created;

        /**
         * {@inheritDoc}
         */
        @Override
        public VtnFlowConditions execute(TxContext ctx) throws VTNException {
            created = false;

            // Load current configuration.
            InstanceIdentifier<VtnFlowConditions> path =
                InstanceIdentifier.create(VtnFlowConditions.class);
            LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
            ReadWriteTransaction tx = ctx.getReadWriteTransaction();
            VtnFlowConditions conditions =
                DataStoreUtils.read(tx, oper, path).orNull();
            if (conditions == null) {
                // Initialize the flow condition container.
                conditions = new VtnFlowConditionsBuilder().build();
                tx.put(oper, path, conditions, true);
                created = true;
            }

            return conditions;
        }

        /**
         * {@inheritDoc}
         */
        @Override
        public void onSuccess(VTNManagerProvider provider,
                              VtnFlowConditions result) {
            if (created) {
                LOG.info(
                    "An empty flow condition container has been created.");
            }

            XmlConfigFile.Type ftype = XmlConfigFile.Type.FLOWCOND;
            Set<String> names = new HashSet<>();
            List<VtnFlowCondition> vlist = result.getVtnFlowCondition();
            if (vlist != null) {
                for (VtnFlowCondition vfc: vlist) {
                    // Save configuration into a file.
                    VTNFlowCondition vfcond = VTNFlowCondition.create(vfc);
                    if (vfcond != null) {
                        String name = vfcond.getIdentifier();
                        XmlConfigFile.save(ftype, name, vfcond);
                        names.add(name);
                    }
                }
            }

            // Remove obsolete configuration files.
            XmlConfigFile.deleteAll(ftype, names);
        }
    }

    /**
     * Create a string that describes the specified flow match.
     *
     * @param vmatch  A {@link VtnMatchFields} instance.
     * @return  A string taht describes the specified flow match.
     */
    static String toString(VtnMatchFields vmatch) {
        try {
            VTNMatch vm = new VTNMatch();
            vm.set(vmatch);
            return vm.getConditionKey();
        } catch (RpcException | RuntimeException e) {
            LOG.error("Invalid flow match: " + vmatch, e);
            return String.valueOf(vmatch);
        }
    }

    /**
     * Construct a new instance.
     *
     * @param provider  VTN Manager provider service.
     */
    public FlowCondManager(VTNManagerProvider provider) {
        super(VtnFlowCondition.class);
        vtnProvider = provider;
        registerListener(provider.getDataBroker(),
                         LogicalDatastoreType.OPERATIONAL, true);
    }

    /**
     * Invoked when a flow condition has been created or updated.
     *
     * @param ectx     A {@link FlowCondChange} instance which keeps changes to
     *                 the configuration.
     * @param data     An {@link IdentifiedData} instance that contains a data
     *                 object.
     * @param created  {@code true} means that the given flow condition has
     *                 been newly created.
     * @return  The name of the flow condition on success.
     *          {@code null} on failure.
     */
    private String onUpdated(FlowCondChange ectx,
                             IdentifiedData<VtnFlowCondition> data,
                             boolean created) {
        VtnFlowCondition vfc = data.getValue();
        VTNFlowCondition vfcond = VTNFlowCondition.create(vfc);
        String name;
        if (vfcond == null) {
            LOG.warn("Ignore broken {} event: path={}, value={}",
                     (created) ? "creation" : "update", data.getIdentifier(),
                     vfc);
            name = null;
        } else {
            ectx.addUpdated(vfcond.getIdentifier(), vfcond, created);
            name = vfcond.getIdentifier();
        }

        return name;
    }

    /**
     * Handle creation or removal event for a flow match.
     *
     * @param ectx  A {@link FlowCondChange} instance which keeps changes to
     *              the configuration.
     * @param data  An {@link IdentifiedData} instance that contains a flow
     *              match.
     * @param type  {@link VtnUpdateType#CREATED} on added,
     *              {@link VtnUpdateType#REMOVED} on removed.
     */
    private void onMatchChanged(FlowCondChange ectx, IdentifiedData<?> data,
                                VtnUpdateType type) {
        IdentifiedData<VtnFlowMatch> mdata =
            data.checkType(VtnFlowMatch.class);
        if (mdata != null) {
            InstanceIdentifier<VtnFlowMatch> path = mdata.getIdentifier();
            String name = FlowCondUtils.getName(path);
            VtnFlowMatch vfm = mdata.getValue();
            LOG.info("{}.{}: Flow condition match has been {}: {{}}",
                     name, vfm.getIndex(), MiscUtils.toLowerCase(type),
                     toString(vfm));

            // Mark the flow condition as changed.
            ectx.setChanged(name);
        } else {
            // This should never happen.
            data.unexpected(LOG, type);
        }
    }

    /**
     * Handle update event for a flow match.
     *
     * @param ectx  A {@link FlowCondChange} instance which keeps changes to
     *              the configuration.
     * @param data  An {@link ChangedData} instance that contains a flow match.
     */
    private void onMatchChanged(FlowCondChange ectx, ChangedData<?> data) {
        ChangedData<VtnFlowMatch> mdata = data.checkType(VtnFlowMatch.class);
        if (mdata != null) {
            InstanceIdentifier<VtnFlowMatch> path = mdata.getIdentifier();
            String name = FlowCondUtils.getName(path);
            VtnFlowMatch old = mdata.getOldValue();
            VtnFlowMatch vfm = mdata.getValue();
            LOG.info("{}.{}: Flow condition match has been changed: " +
                     "{{}} -> {{}}", name, vfm.getIndex(),
                     toString(old), toString(vfm));

            // Mark the flow condition as changed.
            ectx.setChanged(name);
        } else {
            // This should never happen.
            data.unexpected(LOG, VtnUpdateType.CHANGED);
        }
    }

    // MultiDataStoreListener

    /**
     * {@inheritDoc}
     */
    @Override
    protected boolean isDepth(VtnUpdateType type) {
        // Creation events should be processed from outer to inner.
        // Other events should be processed from inner to outer.
        return (type == VtnUpdateType.CREATED);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected boolean isRequiredType(@Nonnull Class<?> type) {
        return (VtnFlowCondition.class.equals(type) ||
                VtnFlowMatch.class.equals(type));
    }

    /**
     * Determine whether the specified type of the tree node should be
     * treated as a leaf node.
     *
     * <p>
     *   This method returns {@code true} only if the specified type is
     *   {@link VtnFlowMatch}.
     * </p>
     *
     * @param type  A class that specifies the type of the tree node.
     * @return  {@code true} if the specified type of the tree node should
     *          be treated as a leaf node. {@code false} otherwise.
     */
    @Override
    protected boolean isLeafNode(@Nonnull Class<?> type) {
        return VtnFlowMatch.class.equals(type);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected boolean isUpdated(FlowCondChange ectx, ChangedData<?> data) {
        boolean changed;
        ChangedData<VtnFlowCondition> cdata =
            data.checkType(VtnFlowCondition.class);
        if (cdata != null) {
            // Return true if vtn-flow-match list is updated.
            VtnFlowCondition vfc = cdata.getValue();
            String name = MiscUtils.getValue(vfc.getName());
            changed = ectx.isChanged(name);
        } else {
            ChangedData<VtnFlowMatch> mdata =
                data.checkType(VtnFlowMatch.class);
            if (mdata != null) {
                // Return true if vtn-flow-match is updated.
                VtnFlowMatch old = mdata.getOldValue();
                VtnFlowMatch vfm = mdata.getValue();
                changed = !Objects.equals(old, vfm);
            } else {
                // This should never happen.
                data.unexpected(LOG, VtnUpdateType.CHANGED);
                changed = false;
            }
        }

        return changed;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void onCreated(FlowCondChange ectx, IdentifiedData<?> data) {
        IdentifiedData<VtnFlowCondition> cdata =
            data.checkType(VtnFlowCondition.class);
        if (cdata != null) {
            String name = onUpdated(ectx, cdata, true);
            if (name != null) {
                LOG.info("Flow condition has been created: name={}", name);
            }
        } else {
            onMatchChanged(ectx, data, VtnUpdateType.CREATED);
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void onUpdated(FlowCondChange ectx, ChangedData<?> data) {
        ChangedData<VtnFlowCondition> cdata =
            data.checkType(VtnFlowCondition.class);
        if (cdata != null) {
            onUpdated(ectx, cdata, false);
        } else {
            onMatchChanged(ectx, data);
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void onRemoved(FlowCondChange ectx, IdentifiedData<?> data) {
        IdentifiedData<VtnFlowCondition> cdata =
            data.checkType(VtnFlowCondition.class);
        if (cdata != null) {
            VtnFlowCondition vfc = cdata.getValue();
            String name = MiscUtils.getValue(vfc.getName());
            if (name == null) {
                LOG.warn("Ignore broken removal event: path={}, value={}",
                         cdata.getIdentifier(), vfc);
            } else {
                ectx.addRemoved(name);
                LOG.info("Flow condition has been removed: name={}", name);
            }
        } else {
            onMatchChanged(ectx, data, VtnUpdateType.REMOVED);
        }
    }

    // AbstractDataChangeListener

    /**
     * {@inheritDoc}
     */
    @Override
    protected FlowCondChange enterEvent() {
        return new FlowCondChange();
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void exitEvent(FlowCondChange ectx) {
        ectx.apply(LOG);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected InstanceIdentifier<VtnFlowCondition> getWildcardPath() {
        return InstanceIdentifier.builder(VtnFlowConditions.class).
            child(VtnFlowCondition.class).build();
    }

    // CloseableContainer

    /**
     * {@inheritDoc}
     */
    @Override
    protected Logger getLogger() {
        return LOG;
    }

    // VTNSubSystem

    /**
     * {@inheritDoc}
     */
    @Override
    public VTNFuture<?> initConfig(boolean master) {
        TxTask<?> task = (master)
            ? new FlowCondLoadTask() : new FlowCondSaveTask();
        return vtnProvider.post(task);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void initRpcServices(RpcProviderRegistry rpcReg,
                                CompositeAutoCloseable regs) {
        regs.add(rpcReg.
                 addRpcImplementation(VtnFlowConditionService.class, this));
    }

    // VtnFlowConditionService

    /**
     * Create or modify the flow condition.
     *
     * <ul>
     *   <li>
     *     If the flow condition specified by the name does not exist, a new
     *     flow condition will be associated with the specified name.
     *   </li>
     *   <li>
     *     If the flow condition specifie by the name already exists,
     *     it will be modified as specified the RPC input.
     *   </li>
     * </ul>
     *
     * @param input  Input of the RPC.
     * @return  A {@link Future} associated with the RPC task.
     */
    @Override
    public Future<RpcResult<SetFlowConditionOutput>> setFlowCondition(
        SetFlowConditionInput input) {
        try {
            // Create a task that updates the configuration.
            SetFlowConditionTask task = SetFlowConditionTask.create(input);
            VTNFuture<VtnUpdateType> taskFuture = vtnProvider.postSync(task);
            return new RpcFuture<VtnUpdateType, SetFlowConditionOutput>(
                taskFuture, task);
        } catch (Exception e) {
            return RpcUtils.getErrorBuilder(SetFlowConditionOutput.class, e).
                buildFuture();
        }
    }

    /**
     * Remove the flow condition specified by the name.
     *
     * @param input  Input of the RPC.
     * @return  A {@link Future} associated with the RPC task.
     */
    @Override
    public Future<RpcResult<Void>> removeFlowCondition(
        RemoveFlowConditionInput input) {
        try {
            // Create a task that removes the specified flow condition.
            RemoveFlowConditionTask task =
                RemoveFlowConditionTask.create(input);
            VTNFuture<VtnUpdateType> taskFuture = vtnProvider.postSync(task);
            return new RpcFuture<VtnUpdateType, Void>(taskFuture, task);
        } catch (Exception e) {
            return RpcUtils.getErrorBuilder(Void.class, e).buildFuture();
        }
    }

    /**
     * Configure a flow match condition into the flow condition specified
     * by the flow condition name and match index.
     *
     * @param input  Input of the RPC.
     * @return  A {@link Future} associated with the RPC task.
     */
    @Override
    public Future<RpcResult<SetFlowConditionMatchOutput>> setFlowConditionMatch(
        SetFlowConditionMatchInput input) {
        try {
            // Create a task that updates the flow matches in the given
            // flow condition.
            SetFlowMatchTask task = SetFlowMatchTask.create(input);
            VTNFuture<List<VtnUpdateType>> taskFuture =
                vtnProvider.postSync(task);
            return new RpcFuture<List<VtnUpdateType>, SetFlowConditionMatchOutput>(
                taskFuture, task);
        } catch (Exception e) {
            return RpcUtils.getErrorBuilder(
                SetFlowConditionMatchOutput.class, e).buildFuture();
        }
    }

    /**
     * Remove the flow match condition specified by the flow condition name
     * and match index.
     *
     * @param input  Input of the RPC.
     * @return  A {@link Future} associated with the RPC task.
     */
    @Override
    public Future<RpcResult<RemoveFlowConditionMatchOutput>> removeFlowConditionMatch(
        RemoveFlowConditionMatchInput input) {
        try {
            // Create a task that removes flow matches in the given
            // flow condition.
            RemoveFlowMatchTask task = RemoveFlowMatchTask.create(input);
            VTNFuture<List<VtnUpdateType>> taskFuture =
                vtnProvider.postSync(task);
            return new RpcFuture<List<VtnUpdateType>, RemoveFlowConditionMatchOutput>(
                taskFuture, task);
        } catch (Exception e) {
            return RpcUtils.getErrorBuilder(
                RemoveFlowConditionMatchOutput.class, e).buildFuture();
        }
    }

    /**
     * Remove all the flow conditions.
     *
     * @return  A {@link Future} associated with the RPC task.
     */
    @Override
    public Future<RpcResult<ClearFlowConditionOutput>> clearFlowCondition() {
        // Create a task that removes all the flow conditions.
        ClearFlowConditionTask task = new ClearFlowConditionTask();
        VTNFuture<VtnUpdateType> taskFuture = vtnProvider.postSync(task);
        return new RpcFuture<VtnUpdateType, ClearFlowConditionOutput>(
            taskFuture, task);
    }
}
