/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.inventory;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.TxContext;
import org.opendaylight.vtn.manager.internal.TxQueue;
import org.opendaylight.vtn.manager.internal.TxTask;
import org.opendaylight.vtn.manager.internal.VTNManagerProvider;
import org.opendaylight.vtn.manager.internal.VTNSubSystem;
import org.opendaylight.vtn.manager.internal.inventory.xml.XmlStaticEdgePorts;
import org.opendaylight.vtn.manager.internal.inventory.xml.XmlStaticSwitchLinks;
import org.opendaylight.vtn.manager.internal.util.ChangedData;
import org.opendaylight.vtn.manager.internal.util.CompositeAutoCloseable;
import org.opendaylight.vtn.manager.internal.util.DataStoreUtils;
import org.opendaylight.vtn.manager.internal.util.IdentifiedData;
import org.opendaylight.vtn.manager.internal.util.IdentifierTargetComparator;
import org.opendaylight.vtn.manager.internal.util.MiscUtils;
import org.opendaylight.vtn.manager.internal.util.MultiDataStoreListener;
import org.opendaylight.vtn.manager.internal.util.concurrent.VTNFuture;
import org.opendaylight.vtn.manager.internal.util.inventory.InventoryUtils;
import org.opendaylight.vtn.manager.internal.util.tx.AbstractTxTask;

import org.opendaylight.controller.md.sal.binding.api.DataBroker;
import org.opendaylight.controller.md.sal.binding.api.ReadTransaction;
import org.opendaylight.controller.md.sal.binding.api.ReadWriteTransaction;
import org.opendaylight.controller.md.sal.common.api.data.AsyncDataBroker.DataChangeScope;
import org.opendaylight.controller.md.sal.common.api.data.AsyncDataChangeEvent;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;
import org.opendaylight.controller.sal.binding.api.RpcProviderRegistry;

import org.opendaylight.yangtools.yang.binding.DataObject;
import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.topology._static.rev150801.VtnStaticTopology;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.topology._static.rev150801.VtnStaticTopologyBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.topology._static.rev150801.vtn._static.topology.StaticEdgePorts;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.topology._static.rev150801.vtn._static.topology.StaticSwitchLinks;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.topology._static.rev150801.vtn._static.topology._static._switch.links.StaticSwitchLink;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.topology._static.rev150801.vtn._static.topology._static.edge.ports.StaticEdgePort;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;

/**
 * {@code StaticTopologyManager} manages the static network topology
 * configured by user.
 */
public final class StaticTopologyManager
    extends MultiDataStoreListener<VtnStaticTopology, StaticLinkUpdateTask>
    implements VTNSubSystem {
    /**
     * Logger instance.
     */
    private static final Logger  LOG =
        LoggerFactory.getLogger(StaticTopologyManager.class);

    /**
     * Comparator for the target type of instance identifier that specifies
     * the order of data change event processing.
     */
    private static final IdentifierTargetComparator  PATH_COMPARATOR;

    /**
     * Instance identifier of the root container for the static network
     * topology configuration.
     */
    static final InstanceIdentifier<VtnStaticTopology> IDENT_TOPOLOGY =
        InstanceIdentifier.create(VtnStaticTopology.class);

    /**
     * Initialize static fields.
     */
    static {
        PATH_COMPARATOR = new IdentifierTargetComparator().
            setOrder(StaticEdgePort.class, 1).
            setOrder(StaticSwitchLink.class, 2);
    }

    /**
     * VTN Manager provider service.
     */
    private final VTNManagerProvider  vtnProvider;

    /**
     * MD-SAL datastore transaction queue used to update inventory information.
     */
    private final TxQueue  inventoryQueue;

    /**
     * Update the configuration file for the static network topology.
     *
     * @param vstopo  A {@link VtnStaticTopology} instance to be saved.
     *                {@code null} means that the configuration file should be
     *                deleted.
     */
    static void saveConfig(VtnStaticTopology vstopo) {
        StaticSwitchLinks swlinks;
        StaticEdgePorts edges;
        if (vstopo == null) {
            swlinks = null;
            edges = null;
        } else {
            swlinks = vstopo.getStaticSwitchLinks();
            edges = vstopo.getStaticEdgePorts();
        }

        new XmlStaticSwitchLinks().save(swlinks);
        new XmlStaticEdgePorts().save(edges);
    }

    /**
     * MD-SAL datastore transaction task to load static network topology.
     *
     * <p>
     *   This task returns current {@link VtnStaticTopology} instance.
     * </p>
     */
    private static class LoadTask extends AbstractTxTask<VtnStaticTopology> {
        /**
         * {@inheritDoc}
         */
        @Override
        public VtnStaticTopology execute(TxContext ctx) throws VTNException {
            ReadWriteTransaction tx = ctx.getReadWriteTransaction();
            VtnStaticTopologyBuilder builder = new VtnStaticTopologyBuilder();

            // Load static inter-switch link configuration.
            new XmlStaticSwitchLinks().load(builder, tx);

            // Load static edge port configuration from file.
            new XmlStaticEdgePorts().load(builder, tx);

            return builder.build();
        }

        /**
         * {@inheritDoc}
         */
        @Override
        public void onSuccess(VTNManagerProvider provider,
                              VtnStaticTopology vstopo) {
            if (vstopo.getStaticSwitchLinks() != null) {
                LOG.info("Static inter-switch link configuration has been " +
                         "loaded.");
            }
            if (vstopo.getStaticEdgePorts() != null) {
                LOG.info("Static edge port configuration has been loaded.");
            }
        }
    }

    /**
     * MD-SAL datastore transaction task to initialize static network
     * topology.
     *
     * <p>
     *   This task returns current {@link VtnStaticTopology} instance.
     * </p>
     */
    private static class InitTask extends AbstractTxTask<VtnStaticTopology> {
        /**
         * {@inheritDoc}
         */
        @Override
        public VtnStaticTopology execute(TxContext ctx) throws VTNException {
            ReadTransaction tx = ctx.getTransaction();
            LogicalDatastoreType cstore = LogicalDatastoreType.CONFIGURATION;

            // Read current configuration.
            return DataStoreUtils.read(tx, cstore, IDENT_TOPOLOGY).orNull();
        }

        /**
         * {@inheritDoc}
         */
        @Override
        public void onSuccess(VTNManagerProvider provider,
                              VtnStaticTopology vstopo) {
            StaticTopologyManager.saveConfig(vstopo);
        }
    }

    /**
     * Construct a new instance.
     *
     * @param provider  A VTN Manager provider service.
     * @param txq       A MD-SAL datastore transaction queue used to update
     *                  inventory information.
     */
    StaticTopologyManager(VTNManagerProvider provider, TxQueue txq) {
        super(VtnStaticTopology.class);
        vtnProvider = provider;
        inventoryQueue = txq;

        // Register listener for static network topology configuration.
        DataBroker broker = provider.getDataBroker();
        try {
            registerListener(broker, LogicalDatastoreType.CONFIGURATION,
                             DataChangeScope.SUBTREE, true);
        } catch (RuntimeException e) {
            String msg = "Failed to register static network topology listener.";
            LOG.error(msg, e);
            close();
            throw new IllegalStateException(msg, e);
        }
    }

    /**
     * Handle static network topology creation or removal event.
     *
     * @param ectx  A {@link StaticLinkUpdateTask} instance.
     * @param data  A created or removed data object.
     * @param type  {@link VtnUpdateType#CREATED} on added,
     *              {@link VtnUpdateType#REMOVED} on removed.
     */
    private void onCreatedOrRemoved(StaticLinkUpdateTask ectx,
                                    IdentifiedData<?> data,
                                    VtnUpdateType type) {
        IdentifiedData<StaticEdgePort> edgeData =
            data.checkType(StaticEdgePort.class);
        if (edgeData != null) {
            StaticEdgePort edge = edgeData.getValue();
            ectx.addUpdated(edge);
            LOG.info("Static edge port has been {}: {}",
                     MiscUtils.toLowerCase(type),
                     MiscUtils.getValue(edge.getPort()));
            return;
        }

        IdentifiedData<StaticSwitchLink> linkData =
            data.checkType(StaticSwitchLink.class);
        if (linkData != null) {
            StaticSwitchLink swlink = linkData.getValue();
            ectx.addUpdated(swlink);
            LOG.info("Static inter-switch link has been {}: {}",
                     MiscUtils.toLowerCase(type),
                     InventoryUtils.toString(swlink));
        }
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
        // This class does not depend on the order of data change events.
        return true;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected StaticLinkUpdateTask enterEvent(
        AsyncDataChangeEvent<InstanceIdentifier<?>, DataObject> ev) {
        return new StaticLinkUpdateTask(LOG);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void exitEvent(StaticLinkUpdateTask ectx) {
        inventoryQueue.post(ectx);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void onCreated(StaticLinkUpdateTask ectx,
                             IdentifiedData<?> data) {
        onCreatedOrRemoved(ectx, data, VtnUpdateType.CREATED);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void onUpdated(StaticLinkUpdateTask ectx, ChangedData<?> data) {
        // StaticEdgePort should not be reported because it has only one field.
        ChangedData<StaticSwitchLink> linkData =
            data.checkType(StaticSwitchLink.class);
        if (linkData != null) {
            StaticSwitchLink swlink = linkData.getValue();
            StaticSwitchLink old = linkData.getOldValue();
            ectx.addUpdated(old, swlink);
            LOG.info("Static inter-switch link has been changed: old={}, " +
                     "new={}", InventoryUtils.toString(old),
                     InventoryUtils.toString(swlink));
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void onRemoved(StaticLinkUpdateTask ectx,
                             IdentifiedData<?> data) {
        onCreatedOrRemoved(ectx, data, VtnUpdateType.REMOVED);
    }

    // AbstractDataChangeListener

    /**
     * {@inheritDoc}
     */
    @Override
    protected InstanceIdentifier<VtnStaticTopology> getWildcardPath() {
        return IDENT_TOPOLOGY;
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
        TxTask<VtnStaticTopology> task = (master)
            ? new LoadTask() : new InitTask();
        return vtnProvider.post(task);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void initRpcServices(RpcProviderRegistry rpcReg,
                                CompositeAutoCloseable regs) {
        // Nothing to do.
    }
}
