/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.vnode;

import java.util.HashSet;
import java.util.List;
import java.util.Set;

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
import org.opendaylight.vtn.manager.internal.util.IdentifierTargetComparator;
import org.opendaylight.vtn.manager.internal.util.MiscUtils;
import org.opendaylight.vtn.manager.internal.util.MultiDataStoreListener;
import org.opendaylight.vtn.manager.internal.util.XmlConfigFile;
import org.opendaylight.vtn.manager.internal.util.concurrent.VTNFuture;
import org.opendaylight.vtn.manager.internal.util.tx.AbstractTxTask;
import org.opendaylight.vtn.manager.internal.util.vnode.VTenantUtils;
import org.opendaylight.vtn.manager.internal.vnode.xml.XmlVTenant;

import org.opendaylight.controller.md.sal.binding.api.ReadWriteTransaction;
import org.opendaylight.controller.md.sal.common.api.data.AsyncDataBroker.DataChangeScope;
import org.opendaylight.controller.md.sal.common.api.data.AsyncDataChangeEvent;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;
import org.opendaylight.controller.sal.binding.api.RpcProviderRegistry;

import org.opendaylight.yangtools.yang.binding.DataObject;
import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathmap.rev150328.vtn.path.map.list.VtnPathMap;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.Vtns;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.VtnsBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.vtns.Vtn;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;

/**
 * Virtual tenant manager.
 */
public final class VTenantManager
    extends MultiDataStoreListener<Vtn, VTenantChange>
    implements VTNSubSystem {
    /**
     * Logger instance.
     */
    private static final Logger  LOG =
        LoggerFactory.getLogger(VTenantManager.class);

    /**
     * Comparator for the target type of instance identifier that specifies
     * the order of data change event processing.
     */
    private static final IdentifierTargetComparator  PATH_COMPARATOR;

    /**
     * VTN Manager provider service.
     */
    private final VTNManagerProvider  vtnProvider;

    /**
     * A set of paths to the loaded objects.
     */
    private Set<InstanceIdentifier<?>>  loadedPaths;

    /**
     * Initialize static fields.
     */
    static {
        // Create a path comparator that assigns higher priority to inner data.
        int order = 0;
        PATH_COMPARATOR = new IdentifierTargetComparator().
            setOrder(VtnPathMap.class, ++order).
            setOrder(Vtn.class, ++order);
    }

    /**
     * {@code VTenantSaveTask} describes a MD-SAL datastore transaction task
     * that saves the VTN configurations.
     *
     * <p>
     *   This task returns the root container of all VTNs.
     * </p>
     */
    private static final class VTenantSaveTask extends AbstractTxTask<Vtns> {
        /**
         * Set {@code true} if the root container has been created.
         */
        private boolean  created;

        // AbstractTxTask

        /**
         * {@inheritDoc}
         */
        @Override
        public Vtns execute(TxContext ctx) throws VTNException {
            created = false;

            // Load current configuration.
            InstanceIdentifier<Vtns> path =
                InstanceIdentifier.create(Vtns.class);
            LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
            ReadWriteTransaction tx = ctx.getReadWriteTransaction();
            Vtns root = DataStoreUtils.read(tx, oper, path).orNull();
            if (root == null) {
                // Initialize the VTN container.
                root = new VtnsBuilder().build();
                tx.put(oper, path, root, true);
                created = true;
            }

            return root;
        }

        /**
         * {@inheritDoc}
         */
        @Override
        public void onSuccess(VTNManagerProvider provider, Vtns result) {
            if (created) {
                LOG.info("An empty VTN container has been created.");
            }

            XmlConfigFile.Type ftype = XmlConfigFile.Type.VTN;
            Set<String> names = new HashSet<>();
            List<Vtn> vlist = result.getVtn();
            if (vlist != null) {
                for (Vtn vtn: vlist) {
                    // Save configuration into a file.
                    try {
                        XmlVTenant xvtn = new XmlVTenant(vtn);
                        String name = xvtn.getName();
                        XmlConfigFile.save(ftype, name, xvtn);
                        names.add(name);
                    } catch (Exception e) {
                        LOG.warn("Ignore broken VTN: " + vtn, e);
                    }
                }

                // Remove obsolete configuration files.
                XmlConfigFile.deleteAll(ftype, names);
            }
        }
    }

    /**
     * Construct a new instance.
     *
     * @param provider  A VTN Manager provider service.
     */
    public VTenantManager(VTNManagerProvider provider) {
        super(Vtn.class);
        vtnProvider = provider;
        registerListener(provider.getDataBroker(),
                         LogicalDatastoreType.OPERATIONAL,
                         DataChangeScope.SUBTREE);
    }

    /**
     * Set paths to the objects loaded from the configuration file.
     *
     * @param paths  A set of object paths.
     */
    void setLoadedPaths(Set<InstanceIdentifier<?>> paths) {
        loadedPaths = paths;
    }

    /**
     * Invoked when a VTN has been created or updated.
     *
     * @param ectx     A {@link VTenantChange} instance.
     * @param data     A {@link IdentifiedData} instance.
     * @param created  {@code true} means that the given VTN has been newly
     *                 created.
     */
    private void onVtnChanged(VTenantChange ectx, IdentifiedData<Vtn> data,
                              boolean created) {
        Vtn vtn = data.getValue();
        String name = vtn.getName().getValue();
        ectx.addUpdatedVtn(name, vtn, created);
    }

    /**
     * Invoked when a VTN path map has been created or updated.
     *
     * @param ectx  A {@link VTenantChange} instance.
     * @param name  The name of the VTN.
     * @param vpm   A {@link VtnPathMap} instance.
     * @param type  A {@link VtnUpdateType} instance which indicates the event
     *              type.
     */
    private void onPathMapChanged(VTenantChange ectx, String name,
                                  VtnPathMap vpm, VtnUpdateType type) {
        LOG.info("{}.{}: VTN path map has been {}.", name, vpm.getIndex(),
                 MiscUtils.toLowerCase(type.name()));
        ectx.addTargetVtn(name);
    }

    // VTNSubSystem

    /**
     * {@inheritDoc}
     */
    @Override
    public VTNFuture<?> initConfig(boolean master) {
        TxTask<?> task = (master)
            ? new VTenantLoadTask(this) : new VTenantSaveTask();
        return vtnProvider.post(task);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void initRpcServices(RpcProviderRegistry rpcReg,
                                CompositeAutoCloseable regs) {
    }

    // MultiDataStoreListener

    /**
     * {@inheritDoc}
     */
    @Override
    protected IdentifierTargetComparator getComparator() {
        return PATH_COMPARATOR;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected boolean getOrder(VtnUpdateType type) {
        // Creation events should be processed from outer to inner.
        // Other events should be processed from inner to outer.
        return (type != VtnUpdateType.CREATED);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected VTenantChange enterEvent(
        AsyncDataChangeEvent<InstanceIdentifier<?>, DataObject> ev) {
        return new VTenantChange(vtnProvider);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void exitEvent(VTenantChange ectx) {
        ectx.apply(LOG);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void onCreated(VTenantChange ectx, IdentifiedData<?> data) {
        // Do nothing if the specified event was caused by the initial setup.
        Set<InstanceIdentifier<?>> loaded = loadedPaths;
        if (loaded != null && loaded.remove(data.getIdentifier())) {
            if (loaded.isEmpty()) {
                LOG.debug("All loaded VTNs have been notified.");
                loadedPaths = null;
            }
            return;
        }

        IdentifiedData<Vtn> vtnData = data.checkType(Vtn.class);
        if (vtnData != null) {
            onVtnChanged(ectx, vtnData, true);
            return;
        }

        String vtnName = VTenantUtils.getName(data.getIdentifier());
        IdentifiedData<VtnPathMap> pmapData = data.checkType(VtnPathMap.class);
        if (pmapData != null) {
            onPathMapChanged(ectx, vtnName, pmapData.getValue(),
                             VtnUpdateType.CREATED);
            return;
        }

        // This should never happen.
        LOG.warn("CREATED: Unexpected event: path={}, data={}",
                 data.getIdentifier(), data.getValue());
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void onUpdated(VTenantChange ectx, ChangedData<?> data) {
        IdentifiedData<Vtn> vtnData = data.checkType(Vtn.class);
        if (vtnData != null) {
            onVtnChanged(ectx, vtnData, false);
            return;
        }

        String vtnName = VTenantUtils.getName(data.getIdentifier());
        IdentifiedData<VtnPathMap> pmapData = data.checkType(VtnPathMap.class);
        if (pmapData != null) {
            onPathMapChanged(ectx, vtnName, pmapData.getValue(),
                             VtnUpdateType.CHANGED);
            return;
        }

        // This should never happen.
        LOG.warn("CHANGED: Unexpected event: path={}, old={}, new={}",
                 data.getIdentifier(), data.getOldValue(), data.getValue());
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void onRemoved(VTenantChange ectx, IdentifiedData<?> data) {
        IdentifiedData<Vtn> vtnData = data.checkType(Vtn.class);
        if (vtnData != null) {
            Vtn vtn = vtnData.getValue();
            ectx.addRemoved(vtn.getName().getValue());
            return;
        }

        String vtnName = VTenantUtils.getName(data.getIdentifier());
        IdentifiedData<VtnPathMap> pmapData = data.checkType(VtnPathMap.class);
        if (pmapData != null) {
            VtnPathMap vpm = pmapData.getValue();
            LOG.info("{}.{}: VTN path map has been removed.", vtnName,
                     vpm.getIndex());
            ectx.addTargetVtn(vtnName);
            return;
        }

        // This should never happen.
        LOG.warn("REMOVED: Unexpected event: path={}, data={}",
                 data.getIdentifier(), data.getValue());
    }

    // AbstractDataChangeListener

    /**
     * {@inheritDoc}
     */
    @Override
    protected InstanceIdentifier<Vtn> getWildcardPath() {
        return InstanceIdentifier.builder(Vtns.class).
            child(Vtn.class).build();
    }

    // CloseableContainer

    /**
     * {@inheritDoc}
     */
    @Override
    protected Logger getLogger() {
        return LOG;
    }
}
