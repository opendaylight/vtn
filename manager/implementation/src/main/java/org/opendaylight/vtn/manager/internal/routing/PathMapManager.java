/*
 * Copyright (c) 2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.routing;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Set;
import java.util.concurrent.ConcurrentSkipListSet;
import java.util.concurrent.Future;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import org.opendaylight.vtn.manager.PathMap;
import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.TxContext;
import org.opendaylight.vtn.manager.internal.TxTask;
import org.opendaylight.vtn.manager.internal.VTNManagerProvider;
import org.opendaylight.vtn.manager.internal.VTNSubSystem;
import org.opendaylight.vtn.manager.internal.util.ChangedData;
import org.opendaylight.vtn.manager.internal.util.CompositeAutoCloseable;
import org.opendaylight.vtn.manager.internal.util.DataStoreListener;
import org.opendaylight.vtn.manager.internal.util.DataStoreUtils;
import org.opendaylight.vtn.manager.internal.util.FixedLogger;
import org.opendaylight.vtn.manager.internal.util.IdentifiedData;
import org.opendaylight.vtn.manager.internal.util.MiscUtils;
import org.opendaylight.vtn.manager.internal.util.XmlConfigFile;
import org.opendaylight.vtn.manager.internal.util.concurrent.VTNFuture;
import org.opendaylight.vtn.manager.internal.util.pathmap.PathMapUtils;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;
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

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathmap.rev150328.ClearPathMapInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathmap.rev150328.ClearPathMapOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathmap.rev150328.GlobalPathMaps;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathmap.rev150328.GlobalPathMapsBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathmap.rev150328.RemovePathMapInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathmap.rev150328.RemovePathMapOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathmap.rev150328.SetPathMapInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathmap.rev150328.SetPathMapOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathmap.rev150328.VtnPathMapService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathmap.rev150328.vtn.path.map.list.VtnPathMap;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;

/**
 * Path map manager.
 */
public final class PathMapManager
    extends DataStoreListener<VtnPathMap, GlobalPathMapChange>
    implements VTNSubSystem, VtnPathMapService {
    /**
     * Logger instance.
     */
    private static final Logger  LOG =
        LoggerFactory.getLogger(PathMapManager.class);

    /**
     * VTN Manager provider service.
     */
    private final VTNManagerProvider  vtnProvider;

    /**
     * A set of global path map identifiers loaded by {@link PathMapLoadTask}.
     */
    private Set<Integer>  loadedPathMaps;

    /**
     * MD-SAL transaction task to load global path map configurations.
     *
     * <p>
     *   This task returns current {@link GlobalPathMaps} instance.
     * </p>
     */
    private class PathMapLoadTask extends AbstractTxTask<GlobalPathMaps> {
        /**
         * Resume the configuration for the given path map.
         *
         * @param vlist   A list of {@link VtnPathMap} instance to store
         *                resumed configuration.
         * @param loaded  A set of loaded path path map indices.
         * @param key     A string representation of the map index.
         * @param pmap    A {@link PathMap} instance.
         */
        private void resume(List<VtnPathMap> vlist, Set<Integer> loaded,
                            String key, PathMap pmap) {
            Integer index = pmap.getIndex();
            try {
                if (!key.equals(String.valueOf(index))) {
                    String msg = new StringBuilder("Unexpected index: ").
                        append(index).append(": expected=").append(key).
                        toString();
                    throw new IllegalArgumentException(msg);
                }
                VtnPathMap vpm = PathMapUtils.toVtnPathMapBuilder(pmap).
                    build();
                vlist.add(vpm);
                loaded.add(index);
            } catch (RpcException | RuntimeException e) {
                String msg = MiscUtils.joinColon(
                    "Ignore invalid path map configuration",
                    pmap, e.getMessage());
                LOG.warn(msg, e);
            }
        }

        /**
         * {@inheritDoc}
         */
        @Override
        public GlobalPathMaps execute(TxContext ctx) throws VTNException {
            loadedPathMaps = null;
            Set<Integer> loaded = new ConcurrentSkipListSet<>();

            // Load configuration from file.
            XmlConfigFile.Type ftype = XmlConfigFile.Type.PATHMAP;
            List<VtnPathMap> vlist = new ArrayList<>();
            for (String key: XmlConfigFile.getKeys(ftype)) {
                PathMap pmap = XmlConfigFile.load(ftype, key, PathMap.class);
                if (pmap != null) {
                    resume(vlist, loaded, key, pmap);
                }
            }

            GlobalPathMapsBuilder builder = new GlobalPathMapsBuilder();
            if (!vlist.isEmpty()) {
                builder.setVtnPathMap(vlist);
            }
            InstanceIdentifier<GlobalPathMaps> path =
                InstanceIdentifier.create(GlobalPathMaps.class);

            // Remove old configuration, and install loaded configuration.
            LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
            ReadWriteTransaction tx = ctx.getReadWriteTransaction();
            DataStoreUtils.delete(tx, oper, path);

            GlobalPathMaps maps = builder.build();
            tx.put(oper, path, maps, true);
            if (!loaded.isEmpty()) {
                loadedPathMaps = loaded;
            }

            return maps;
        }

        /**
         * {@inheritDoc}
         */
        @Override
        public void onSuccess(VTNManagerProvider provider,
                              GlobalPathMaps result) {
            List<VtnPathMap> vlist = result.getVtnPathMap();
            if (vlist != null) {
                for (VtnPathMap vpm: vlist) {
                    LOG.info("{}: Global path map has been loaded: " +
                             "cond={}, policy={}, idle={}, hard={}",
                             vpm.getIndex(), vpm.getCondition().getValue(),
                             vpm.getPolicy(), vpm.getIdleTimeout(),
                             vpm.getHardTimeout());
                }
            }
        }
    }

    /**
     * MD-SAL transaction task to save current global path map configurations.
     *
     * <p>
     *   This task returns current {@link GlobalPathMaps} instance.
     * </p>
     */
    private static class PathMapSaveTask
        extends AbstractTxTask<GlobalPathMaps> {
        /**
         * Set {@code true} if the root container has been created.
         */
        private boolean  created;

        /**
         * {@inheritDoc}
         */
        @Override
        public GlobalPathMaps execute(TxContext ctx) throws VTNException {
            created = false;

            // Load current configuration.
            InstanceIdentifier<GlobalPathMaps> path =
                InstanceIdentifier.create(GlobalPathMaps.class);
            LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
            ReadWriteTransaction tx = ctx.getReadWriteTransaction();
            GlobalPathMaps maps = DataStoreUtils.read(tx, oper, path).orNull();
            if (maps == null) {
                // Initialize the global path map container.
                maps = new GlobalPathMapsBuilder().build();
                tx.put(oper, path, maps, true);
                created = true;
            }

            return maps;
        }

        /**
         * {@inheritDoc}
         */
        @Override
        public void onSuccess(VTNManagerProvider provider,
                              GlobalPathMaps result) {
            if (created) {
                LOG.info(
                    "An empty global path map container has been created.");
            }

            XmlConfigFile.Type ftype = XmlConfigFile.Type.PATHMAP;
            Set<String> indices = new HashSet<>();
            List<VtnPathMap> vlist = result.getVtnPathMap();
            if (vlist != null) {
                for (VtnPathMap vpm: vlist) {
                    // Save configuration into a file.
                    try {
                        PathMap pmap = PathMapUtils.toPathMap(vpm);
                        String index = pmap.getIndex().toString();
                        XmlConfigFile.save(ftype, index, pmap);
                        indices.add(index);
                    } catch (Exception e) {
                        LOG.warn("Ignore broken path map: " + vpm, e);
                    }
                }
            }

            // Remove obsolete configuration files.
            XmlConfigFile.deleteAll(ftype, indices);
        }
    }

    /**
     * Construct a new instance.
     *
     * @param provider  VTN Manager provider service.
     */
    public PathMapManager(VTNManagerProvider provider) {
        super(VtnPathMap.class);
        vtnProvider = provider;
        registerListener(provider.getDataBroker(),
                         LogicalDatastoreType.OPERATIONAL,
                         DataChangeScope.SUBTREE);
    }

    /**
     * Invoked when a global path map has been created or updated.
     *
     * @param ectx     A {@link GlobalPathMapChange} instance which keeps
     *                 changes to the configuration.
     * @param data     An {@link IdentifiedData} instance that contains a data
     *                 object.
     * @param created  {@code true} means that the given path map has been
     *                 newly created.
     */
    private void onUpdated(GlobalPathMapChange ectx,
                           IdentifiedData<VtnPathMap> data, boolean created) {
        VtnPathMap vpm = data.getValue();
        try {
            PathMap pmap = PathMapUtils.toPathMap(vpm);
            ectx.addUpdated(pmap.getIndex(), pmap, created);
        } catch (Exception e) {
            FixedLogger logger = new FixedLogger.Warn(LOG);
            logger.log(e, "Ignore broken %s event: path=%s, value=%s",
                       (created) ? "creation" : "update",
                       data.getIdentifier(), vpm);
        }
    }

    // DataStoreListener

    /**
     * {@inheritDoc}
     */
    @Override
    protected GlobalPathMapChange enterEvent(
        AsyncDataChangeEvent<InstanceIdentifier<?>, DataObject> ev) {
        return new GlobalPathMapChange();
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void exitEvent(GlobalPathMapChange ectx) {
        ectx.apply(LOG);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void onCreated(GlobalPathMapChange ectx,
                             IdentifiedData<VtnPathMap> data) {
        // Do nothing if the specified event was caused by the initial setup.
        Set<Integer> loaded = loadedPathMaps;
        if (loaded != null) {
            Integer index = PathMapUtils.getIndex(data.getIdentifier());
            if (index != null && loaded.remove(index)) {
                if (loaded.isEmpty()) {
                    LOG.debug(
                        "All loaded global path maps have been notified.");
                    loadedPathMaps = null;
                }
                return;
            }
        }

        onUpdated(ectx, data, true);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void onUpdated(GlobalPathMapChange ectx,
                             ChangedData<VtnPathMap> data) {
        onUpdated(ectx, data, false);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void onRemoved(GlobalPathMapChange ectx,
                             IdentifiedData<VtnPathMap> data) {
        InstanceIdentifier<VtnPathMap> path = data.getIdentifier();
        Integer index = PathMapUtils.getIndex(path);
        if (index == null) {
            LOG.warn("Ignore broken removal event: path={}, value={}",
                     path, data.getValue());
        } else {
            ectx.addRemoved(index);
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected InstanceIdentifier<VtnPathMap> getWildcardPath() {
        return InstanceIdentifier.builder(GlobalPathMaps.class).
            child(VtnPathMap.class).build();
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
            ? new PathMapLoadTask() : new PathMapSaveTask();
        return vtnProvider.post(task);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void initRpcServices(RpcProviderRegistry rpcReg,
                                CompositeAutoCloseable regs) {
        regs.add(rpcReg.
                 addRpcImplementation(VtnPathMapService.class, this));
    }

    // VtnPathMapService

    /**
     * Create or modify the global or VTN path map.
     *
     * <p>
     *   If the name of the VTN is specified in the RPC input, this operation
     *   creates or modifies VTN path map that affect flows in the specified
     *   VTN. Otherwise this operation creates or modifies global path map
     *   that affect flows in all the VTNs.
     * </p>
     * <ul>
     *   <li>
     *     If the path map specified by the VTN name and map index does not
     *     exist, a new path map will be associated with the specified index.
     *   </li>
     *   <li>
     *     If the path map specifie by the VTN name and map index already
     *     exists, it will be modified as specified the RPC input.
     *   </li>
     * </ul>
     *
     * @param input  Input of the RPC.
     * @return  A {@link Future} associated with the RPC task.
     */
    @Override
    public Future<RpcResult<SetPathMapOutput>> setPathMap(
        SetPathMapInput input) {
        try {
            // Create a task that updates the path maps.
            SetPathMapTask task = SetPathMapTask.create(input);
            VTNFuture<List<VtnUpdateType>> taskFuture =
                vtnProvider.postSync(task);
            return new RpcFuture<List<VtnUpdateType>, SetPathMapOutput>(
                taskFuture, task);
        } catch (Exception e) {
            return RpcUtils.getErrorBuilder(SetPathMapOutput.class, e).
                buildFuture();
        }
    }

    /**
     * Remove the path map configuration in the global or VTN path map.
     *
     * <p>
     *   If the name of the VTN is specified in the RPC input, this operation
     *   removes VTN path map. Otherwise this operation removes global path
     *   map.
     * </p>
     *
     * @param input  Input of the RPC.
     * @return  A {@link Future} associated with the RPC task.
     */
    @Override
    public Future<RpcResult<RemovePathMapOutput>> removePathMap(
        RemovePathMapInput input) {
        try {
            // Create a task that removes the specified path maps.
            RemovePathMapTask task = RemovePathMapTask.create(input);
            VTNFuture<List<VtnUpdateType>> taskFuture =
                vtnProvider.postSync(task);
            return new RpcFuture<List<VtnUpdateType>, RemovePathMapOutput>(
                taskFuture, task);
        } catch (Exception e) {
            return RpcUtils.getErrorBuilder(RemovePathMapOutput.class, e).
                buildFuture();
        }
    }

    /**
     * Remove all the path map configurations present in the global or VTN
     * path map container.
     *
     * @param input  Input of the RPC.
     * @return  A {@link Future} associated with the RPC task.
     */
    @Override
    public Future<RpcResult<ClearPathMapOutput>> clearPathMap(
        ClearPathMapInput input) {
        try {
            // Create a task that clears the specified path map container.
            ClearPathMapTask task = ClearPathMapTask.create(input);
            VTNFuture<VtnUpdateType> taskFuture = vtnProvider.postSync(task);
            return new RpcFuture<VtnUpdateType, ClearPathMapOutput>(
                taskFuture, task);
        } catch (Exception e) {
            return RpcUtils.getErrorBuilder(ClearPathMapOutput.class, e).
                buildFuture();
        }
    }
}
