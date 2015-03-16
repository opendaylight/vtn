/*
 * Copyright (c) 2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.flow.cond;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Set;
import java.util.concurrent.ConcurrentSkipListSet;
import java.util.concurrent.Future;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.TxContext;
import org.opendaylight.vtn.manager.internal.TxTask;
import org.opendaylight.vtn.manager.internal.VTNManagerProvider;
import org.opendaylight.vtn.manager.internal.VTNSubSystem;
import org.opendaylight.vtn.manager.internal.util.CompositeAutoCloseable;
import org.opendaylight.vtn.manager.internal.util.DataStoreListener;
import org.opendaylight.vtn.manager.internal.util.DataStoreUtils;
import org.opendaylight.vtn.manager.internal.util.MiscUtils;
import org.opendaylight.vtn.manager.internal.util.XmlConfigFile;
import org.opendaylight.vtn.manager.internal.util.concurrent.VTNFuture;
import org.opendaylight.vtn.manager.internal.util.flow.cond.FlowCondUtils;
import org.opendaylight.vtn.manager.internal.util.flow.cond.VTNFlowCondition;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcFuture;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcUtils;
import org.opendaylight.vtn.manager.internal.util.tx.AbstractTxTask;

import org.opendaylight.controller.md.sal.binding.api.ReadWriteTransaction;
import org.opendaylight.controller.md.sal.common.api.data.AsyncDataBroker.DataChangeScope;
import org.opendaylight.controller.md.sal.common.api.data.AsyncDataChangeEvent;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;
import org.opendaylight.controller.sal.binding.api.RpcProviderRegistry;

import org.opendaylight.yangtools.yang.binding.DataObject;
import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;
import org.opendaylight.yangtools.yang.common.RpcResult;

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
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.vtn.flow.conditions.VtnFlowCondition;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;

/**
 * Flow condition manager.
 */
public final class FlowCondManager
    extends DataStoreListener<VtnFlowCondition, FlowCondChange>
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
     * A set of flow condition names loaded by {@link FlowCondLoadTask}.
     */
    private Set<String>  loadedConditions;

    /**
     * MD-SAL transaction task to load flow condition configurations.
     *
     * <p>
     *   This task returns current {@link VtnFlowConditions} instance.
     * </p>
     */
    private class FlowCondLoadTask extends AbstractTxTask<VtnFlowConditions> {
        /**
         * {@inheritDoc}
         */
        @Override
        public VtnFlowConditions execute(TxContext ctx) throws VTNException {
            loadedConditions = null;
            Set<String> loaded = new ConcurrentSkipListSet<>();

            // Load configuration from file.
            XmlConfigFile.Type ftype = XmlConfigFile.Type.FLOWCOND;
            List<VtnFlowCondition> vlist = new ArrayList<>();
            for (String key: XmlConfigFile.getKeys(ftype)) {
                VTNFlowCondition vfcond = XmlConfigFile.load(
                    ftype, key, VTNFlowCondition.class);
                if (vfcond != null) {
                    try {
                        vfcond.verify();
                        vlist.add(vfcond.toVtnFlowConditionBuilder().build());
                        loaded.add(vfcond.getIdentifier());
                    } catch (VTNException e) {
                        String msg = MiscUtils.joinColon(
                            "Ignore invalid flow condition configuration",
                            key, e.getMessage());
                        LOG.warn(msg);
                    }
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
            if (!loaded.isEmpty()) {
                loadedConditions = loaded;
            }

            return conditions;
        }

        /**
         * {@inheritDoc}
         */
        @Override
        public void onSuccess(VTNManagerProvider provider,
                              VtnFlowConditions result) {
            List<VtnFlowCondition> vlist = result.getVtnFlowCondition();
            if (vlist != null) {
                for (VtnFlowCondition vfc: vlist) {
                    String name = vfc.getName().getValue();
                    LOG.info("Flow condition was loaded: {}", name);
                }
            }
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
         * A list of {@link VtnFlowCondition} instances to be saved.
         */
        private List<VtnFlowCondition>  saveConfig = new ArrayList<>();

        /**
         * Set {@code true} if the root container has been created.
         */
        private boolean  created;

        /**
         * {@inheritDoc}
         */
        @Override
        public VtnFlowConditions execute(TxContext ctx) throws VTNException {
            saveConfig.clear();
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
            } else {
                List<VtnFlowCondition> vlist =
                    conditions.getVtnFlowCondition();
                if (vlist != null) {
                    for (VtnFlowCondition vfc: vlist) {
                        saveConfig.add(vfc);
                    }
                }
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
                    "An empty flow condition configuration has been created.");
            }

            Set<String> names = new HashSet<>();
            XmlConfigFile.Type ftype = XmlConfigFile.Type.FLOWCOND;
            for (VtnFlowCondition vfc: saveConfig) {
                // Save configuration into a file.
                VTNFlowCondition vfcond = VTNFlowCondition.create(vfc);
                if (vfcond != null) {
                    String name = vfcond.getIdentifier();
                    XmlConfigFile.save(ftype, name, vfcond);
                    names.add(name);
                }
            }

            // Remove obsolete configuration files.
            XmlConfigFile.deleteAll(ftype, names);
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
                         LogicalDatastoreType.OPERATIONAL,
                         DataChangeScope.SUBTREE);
    }

    /**
     * Invoked when a flow condition has been created or updated.
     *
     * @param ectx     A {@link FlowCondChange} instance which keeps changes to
     *                 the configuration.
     * @param path     Path to the flow condition.
     * @param vfc      A {@link VtnFlowCondition} instance.
     * @param created  {@code true} means that the given flow condition has
     *                 been newly created.
     */
    private void onUpdated(FlowCondChange ectx,
                           InstanceIdentifier<VtnFlowCondition> path,
                           VtnFlowCondition vfc, boolean created) {
        VTNFlowCondition vfcond = VTNFlowCondition.create(vfc);
        if (vfcond == null) {
            LOG.warn("Ignore broken {} event: path={}, value={}",
                     (created) ? "creation" : "update", path, vfc);
        } else {
            ectx.addUpdated(vfcond, created);
        }
    }

    // DataStoreListener

    /**
     * {@inheritDoc}
     */
    @Override
    protected FlowCondChange enterEvent(
        AsyncDataChangeEvent<InstanceIdentifier<?>, DataObject> ev) {
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
    protected void onCreated(FlowCondChange ectx,
                             InstanceIdentifier<VtnFlowCondition> key,
                             VtnFlowCondition value) {
        // Do nothing if the specified event was caused by the initial setup.
        Set<String> loaded = loadedConditions;
        if (loaded != null) {
            String name = FlowCondUtils.getName(key);
            if (name != null && loaded.remove(name)) {
                if (loaded.isEmpty()) {
                    LOG.debug("All loaded flow conditions have been notified.");
                    loadedConditions = null;
                }
                return;
            }
        }

        onUpdated(ectx, key, value, true);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void onUpdated(FlowCondChange ectx,
                             InstanceIdentifier<VtnFlowCondition> key,
                             VtnFlowCondition oldValue,
                             VtnFlowCondition newValue) {
        onUpdated(ectx, key, newValue, false);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void onRemoved(FlowCondChange ectx,
                             InstanceIdentifier<VtnFlowCondition> key,
                             VtnFlowCondition value) {
        String name = FlowCondUtils.getName(key);
        if (name == null) {
            LOG.warn("Ignore broken removal event: path={}, value={}",
                     key, value);
        } else {
            ectx.addRemoved(name);
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected InstanceIdentifier<VtnFlowCondition> getWildcardPath() {
        return InstanceIdentifier.builder(VtnFlowConditions.class).
            child(VtnFlowCondition.class).build();
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected Logger getLogger() {
        return LOG;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected Set<VtnUpdateType> getRequiredEvents() {
        return null;
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
     *     If the flow condition specifie dby the name already exists,
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
}
