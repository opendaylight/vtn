/*
 * Copyright (c) 2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.inventory;

import java.util.HashMap;
import java.util.Map;
import java.util.Set;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.TxContext;
import org.opendaylight.vtn.manager.internal.TxQueue;
import org.opendaylight.vtn.manager.internal.VTNManagerProvider;
import org.opendaylight.vtn.manager.internal.util.DataStoreUtils;
import org.opendaylight.vtn.manager.internal.util.InventoryReader;
import org.opendaylight.vtn.manager.internal.util.InventoryUtils;
import org.opendaylight.vtn.manager.internal.util.SalNode;
import org.opendaylight.vtn.manager.internal.util.SalPort;
import org.opendaylight.vtn.manager.internal.util.tx.AbstractTxTask;

import org.opendaylight.controller.md.sal.binding.api.DataBroker;
import org.opendaylight.controller.md.sal.binding.api.ReadWriteTransaction;
import org.opendaylight.controller.md.sal.common.api.data.AsyncDataBroker.DataChangeScope;
import org.opendaylight.controller.md.sal.common.api.data.AsyncDataChangeEvent;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.yangtools.yang.binding.DataObject;
import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.VtnOpenflowVersion;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.node.info.VtnPort;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.nodes.VtnNode;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.nodes.VtnNodeBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;

import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.NodeConnectorId;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.Nodes;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.node.NodeConnector;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.nodes.Node;

/**
 * Listener class that listens the change of MD-SAL node connectors.
 */
public final class NodeConnectorListener
    extends InventoryMaintainer<NodeConnector, NodeConnectorEventContext> {
    /**
     * Logger instance.
     */
    private static final Logger  LOG =
        LoggerFactory.getLogger(NodeConnectorListener.class);

    /**
     * MD-SAL transaction task that updates the VTN port.
     */
    private class PortUpdatedTask extends AbstractTxTask<Void>
        implements NodeConnectorEventContext {
        /**
         * A map that keeps created or updated MD-SAL node connectors.
         */
        private final Map<InstanceIdentifier<NodeConnector>, SalPort> updated =
            new HashMap<>();

        /**
         * A map that keeps removed MD-SAL node connectors.
         p*/
        private final Map<InstanceIdentifier<NodeConnector>, SalPort> removed =
            new HashMap<>();

        /**
         * Update the given node connector in the VTN inventory datastore.
         *
         * @param ctx    A MD-SAL datastore transaction context.
         * @param tx     A {@link ReadWriteTransaction} instance.
         * @param path   Instance identifier of the updated MD-SAL node
         *               connector.
         * @param sport  A {@link SalPort} instance corresponding to the
         *               updated MD-SAL node connector.
         * @throws VTNException  An error occurred.
         */
        private void add(TxContext ctx, ReadWriteTransaction tx,
                         InstanceIdentifier<NodeConnector> path,
                         SalPort sport) throws VTNException {
            // Read MD-SAL node connector from the datastore.
            LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
            NodeConnector nc = DataStoreUtils.read(tx, oper, path).orNull();
            if (nc == null) {
                LOG.debug("Ignore bogus event: {}", sport);
                return;
            }

            // Read VTN node from the datastore.
            SalNode snode = sport.getSalNode();
            InstanceIdentifier<VtnNode> vnpath = snode.getVtnNodeIdentifier();
            VtnNode vnode = DataStoreUtils.read(tx, oper, vnpath).orNull();
            if (vnode != null) {
                // Create or update a VTN port.
                VtnPort vport = InventoryUtils.toVtnPortBuilder(nc).build();
                tx.merge(oper, sport.getVtnPortIdentifier(), vport, true);

                if (vnode.getOpenflowVersion() == null) {
                    // Estimate protocol version.
                    VtnOpenflowVersion v =
                        InventoryUtils.getOpenflowVersion(nc);
                    VtnNode vn = new VtnNodeBuilder().setId(snode.getNodeId()).
                        setOpenflowVersion(v).build();
                    tx.merge(oper, vnpath, vn, true);
                }

                // Cache this port into the inventory reader.
                InventoryReader reader = ctx.getInventoryReader();
                reader.prefetch(snode, vnode);
                resolveIgnoredLinks(ctx, LOG);
            } else {
                LOG.debug("Ignore port because VTN node is not present: {}",
                          sport);
            }
        }

        /**
         * Remove the given node connector from the VTN inventory datastore.
         *
         * @param tx     A {@link ReadWriteTransaction} instance.
         * @param path   Instance identifier of the updated MD-SAL node
         *               connector.
         * @param sport  A {@link SalPort} instance corresponding to the
         *               updated MD-SAL node connector.
         * @throws VTNException  An error occurred.
         */
        private void remove(ReadWriteTransaction tx,
                            InstanceIdentifier<NodeConnector> path,
                            SalPort sport) throws VTNException {
            LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
            InstanceIdentifier<VtnPort> vpath = sport.getVtnPortIdentifier();
            VtnPort vport = DataStoreUtils.read(tx, oper, vpath).orNull();
            if (vport != null) {
                // Remove VTN links affected by the removed port.
                removeVtnLink(tx, vport);

                // Remove a VTN port.
                tx.delete(oper, vpath);
            }
        }

        /**
         * Convert the given MD-SAL node connector into a {@link SalPort}
         * instance.
         *
         * @param nc     A {@link NodeConnector} instance.
         * @param label  A label used only for logging.
         * @return  A {@link SalPort} instance on success.
         *          {@code null} on failure.
         */
        private SalPort toSalPort(NodeConnector nc, String label) {
            NodeConnectorId id = nc.getId();
            SalPort sport = SalPort.create(id);

            // Ignore notification on logical port completely.
            if (sport == null && LOG.isDebugEnabled() &&
                !SalPort.isLogicalPort(id)) {
                LOG.debug("{}: Ignore unsupported node connector: {}",
                          label, id);
            }
            return sport;
        }

        /**
         * {@inheritDoc}
         */
        @Override
        public Void execute(TxContext ctx) throws VTNException {
            // Process node connector deletion events.
            ReadWriteTransaction tx = ctx.getReadWriteTransaction();
            for (Map.Entry<InstanceIdentifier<NodeConnector>, SalPort> entry:
                     removed.entrySet()) {
                InstanceIdentifier<NodeConnector> path = entry.getKey();
                SalPort sport = entry.getValue();
                remove(tx, path, sport);
            }

            // Process node connector update events.
            for (Map.Entry<InstanceIdentifier<NodeConnector>, SalPort> entry:
                     updated.entrySet()) {
                InstanceIdentifier<NodeConnector> path = entry.getKey();
                SalPort sport = entry.getValue();
                add(ctx, tx, path, sport);
            }

            if (!updated.isEmpty()) {
                resolveIgnoredLinks(ctx, LOG);
            }

            return null;
        }

        /**
         * {@inheritDoc}
         */
        @Override
        public void onFailure(VTNManagerProvider provider, Throwable t) {
            LOG.error("Failed to update VTN port information.", t);
        }

        // NodeConnectorEventContext

        /**
         * {@inheritDoc}
         */
        @Override
        public void addUpdated(InstanceIdentifier<NodeConnector> path,
                               NodeConnector nc, String label) {
            SalPort sport = toSalPort(nc, label);
            if (sport != null) {
                updated.put(path, sport);
            }
        }

        /**
         * {@inheritDoc}
         */
        @Override
        public void addRemoved(InstanceIdentifier<NodeConnector> path,
                               NodeConnector nc) {
            SalPort sport = toSalPort(nc, "onRemoved");
            if (sport != null) {
                removed.put(path, sport);
            }
        }

        /**
         * {@inheritDoc}
         */
        @Override
        public boolean hasPort() {
            return !(updated.isEmpty() && removed.isEmpty());
        }
    }

    /**
     * Construct a new instance.
     *
     * @param queue   A {@link TxQueue} instance used to update the
     *                VTN inventory.
     * @param broker  A {@link DataBroker} service instance.
     */
    public NodeConnectorListener(TxQueue queue, DataBroker broker) {
        super(queue, broker, NodeConnector.class, DataChangeScope.BASE);
    }

    // DataStoreListener

    /**
     * {@inheritDoc}
     */
    @Override
    protected NodeConnectorEventContext enterEvent(
        AsyncDataChangeEvent<InstanceIdentifier<?>, DataObject> ev) {
        return new PortUpdatedTask();
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void exitEvent(NodeConnectorEventContext ectx) {
        if (ectx.hasPort()) {
            submit(ectx);
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void onCreated(NodeConnectorEventContext ectx,
                             InstanceIdentifier<NodeConnector> key,
                             NodeConnector value) {
        ectx.addUpdated(key, value, "onCreated");
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void onUpdated(NodeConnectorEventContext ectx,
                             InstanceIdentifier<NodeConnector> key,
                             NodeConnector oldValue, NodeConnector newValue) {
        ectx.addUpdated(key, newValue, "onUpdated");
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void onRemoved(NodeConnectorEventContext ectx,
                             InstanceIdentifier<NodeConnector> key,
                             NodeConnector value) {
        ectx.addRemoved(key, value);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected InstanceIdentifier<NodeConnector> getWildcardPath() {
        return InstanceIdentifier.builder(Nodes.class).child(Node.class).
            child(NodeConnector.class).build();
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
}
