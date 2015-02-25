/*
 * Copyright (c) 2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.inventory;

import java.util.Collections;
import java.util.ArrayList;
import java.util.EnumSet;
import java.util.HashMap;
import java.util.List;
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
import org.opendaylight.vtn.manager.internal.util.tx.AbstractTxTask;

import org.opendaylight.controller.md.sal.binding.api.DataBroker;
import org.opendaylight.controller.md.sal.binding.api.ReadWriteTransaction;
import org.opendaylight.controller.md.sal.common.api.data.AsyncDataBroker.DataChangeScope;
import org.opendaylight.controller.md.sal.common.api.data.AsyncDataChangeEvent;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.yangtools.yang.binding.DataObject;
import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.VtnNodes;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.VtnNodesBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.nodes.VtnNode;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;

import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.NodeId;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.Nodes;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.nodes.Node;

/**
 * Listener class that listens the change of MD-SAL nodes.
 */
public final class NodeListener
    extends InventoryMaintainer<Node, NodeEventContext> {
    /**
     * Logger instance.
     */
    private static final Logger  LOG =
        LoggerFactory.getLogger(NodeListener.class);

    /**
     * Required event types.
     */
    private static final Set<VtnUpdateType>  REQUIRED_EVENTS =
        Collections.unmodifiableSet(
            EnumSet.of(VtnUpdateType.CREATED, VtnUpdateType.REMOVED));

    /**
     * MD-SAL transaction task that initializes the VTN node tree.
     */
    private static class NodesInitTask extends AbstractTxTask<Void> {
        /**
         * Return a list of VTN nodes associated with the given MD-SAL node
         * list.
         *
         * @param nodes  A MD-SAL node list.
         * @return  A list of {@link VtnNode} instances.
         *          {@code null} if no node is present.
         */
        private List<VtnNode> getVtnNodeList(Nodes nodes) {
            if (nodes == null) {
                return null;
            }

            List<Node> nodeList = nodes.getNode();
            if (nodeList == null || nodeList.isEmpty()) {
                return null;
            }

            List<VtnNode> vnodeList = new ArrayList<VtnNode>();
            for (Node node: nodeList) {
                NodeId id = node.getId();
                SalNode snode = SalNode.create(id);
                if (snode == null) {
                    LOG.debug("Ignore unsupported node: {}", id);
                    continue;
                }

                VtnNode vnode = InventoryUtils.toVtnNodeBuilder(node).build();
                vnodeList.add(vnode);
            }

            return (vnodeList.isEmpty()) ? null : vnodeList;
        }

        /**
         * {@inheritDoc}
         */
        @Override
        public Void execute(TxContext ctx) throws VTNException {
            // Read all nodes in the MD-SAL datastore.
            InstanceIdentifier<Nodes> nodesPath =
                InstanceIdentifier.create(Nodes.class);
            LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
            ReadWriteTransaction tx = ctx.getReadWriteTransaction();
            Nodes nodes = DataStoreUtils.read(tx, oper, nodesPath).orNull();
            List<VtnNode> vnList = getVtnNodeList(nodes);
            VtnNodes vnodes = new VtnNodesBuilder().setVtnNode(vnList).build();

            // Initialize VTN node repository.
            InstanceIdentifier<VtnNodes> path =
                InstanceIdentifier.create(VtnNodes.class);
            tx.delete(oper, path);
            tx.merge(oper, path, vnodes, true);

            return null;
        }

        /**
         * {@inheritDoc}
         */
        @Override
        public void onFailure(VTNManagerProvider provider, Throwable t) {
            LOG.warn("Failed to initialize VTN node datastore.", t);
        }
    }

    /**
     * MD-SAL transaction task that creates the VTN node.
     */
    private class NodeUpdatedTask extends AbstractTxTask<Void>
        implements NodeEventContext {
        /**
         * A map that keeps created MD-SAL nodes.
         */
        private final Map<InstanceIdentifier<Node>, SalNode>  created =
            new HashMap<>();

        /**
         * A map that keeps removed list of removed MD-SAL nodes.
         */
        private final Map<InstanceIdentifier<Node>, SalNode>  removed =
            new HashMap<>();

        /**
         * Add the given node to the VTN inventory datastore.
         *
         * @param ctx    A MD-SAL datastore transaction context.
         * @param tx     A {@link ReadWriteTransaction} instance.
         * @param path   Instance identifier of the created MD-SAL node.
         * @param snode  A {@link SalNode} instance corresponding to the
         *               created MD-SAL node.
         * @throws VTNException  An error occurred.
         */
        private void add(TxContext ctx, ReadWriteTransaction tx,
                         InstanceIdentifier<Node> path, SalNode snode)
            throws VTNException {
            // Read MD-SAL node from the datastore.
            LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
            Node node = DataStoreUtils.read(tx, oper, path).orNull();
            if (node == null) {
                LOG.debug("Ignore bogus creation event: {}", snode);
            } else {
                // Create a VTN node.
                VtnNode vnode = InventoryUtils.toVtnNodeBuilder(node).build();
                tx.merge(oper, snode.getVtnNodeIdentifier(), vnode, true);

                // Cache this node into the inventory reader.
                InventoryReader reader = ctx.getInventoryReader();
                reader.prefetch(snode, vnode);
            }
        }

        /**
         * Remove the given node from the VTN inventory datastore.
         *
         * @param tx     A {@link ReadWriteTransaction} instance.
         * @param path   Instance identifier of the removed MD-SAL node.
         * @param snode  A {@link SalNode} instance corresponding to the
         *               removed MD-SAL node.
         * @throws VTNException  An error occurred.
         */
        private void remove(ReadWriteTransaction tx,
                            InstanceIdentifier<Node> path, SalNode snode)
            throws VTNException {
            // Remove VTN links affected by the removed node.
            removeVtnLink(tx, snode);

            // Delete a VTN node associated with the given MD-SAL node.
            DataStoreUtils.delete(tx, LogicalDatastoreType.OPERATIONAL,
                                  snode.getVtnNodeIdentifier());
        }

        /**
         * {@inheritDoc}
         */
        @Override
        public Void execute(TxContext ctx) throws VTNException {
            // Process node deletion events.
            ReadWriteTransaction tx = ctx.getReadWriteTransaction();
            for (Map.Entry<InstanceIdentifier<Node>, SalNode> entry:
                     removed.entrySet()) {
                InstanceIdentifier<Node> path = entry.getKey();
                SalNode snode = entry.getValue();
                remove(tx, path, snode);
            }

            // Process node creation events.
            for (Map.Entry<InstanceIdentifier<Node>, SalNode> entry:
                     created.entrySet()) {
                InstanceIdentifier<Node> path = entry.getKey();
                SalNode snode = entry.getValue();
                add(ctx, tx, path, snode);
            }

            if (!created.isEmpty()) {
                // Try to resolve ignored inter-switch links.
                resolveIgnoredLinks(ctx, LOG);
            }

            return null;
        }

        /**
         * {@inheritDoc}
         */
        @Override
        public void onFailure(VTNManagerProvider provider, Throwable t) {
            LOG.error("Failed to update VTN node information.", t);
        }

        // NodeEventContext

        /**
         * {@inheritDoc}
         */
        @Override
        public void addCreated(InstanceIdentifier<Node> path, Node node) {
            NodeId id = node.getId();
            SalNode snode = SalNode.create(id);
            if (snode == null) {
                LOG.debug("Ignore unsupported node creation: {}", id);
            } else {
                created.put(path, snode);
            }
        }

        /**
         * {@inheritDoc}
         */
        @Override
        public void addRemoved(InstanceIdentifier<Node> path, Node node) {
            NodeId id = node.getId();
            SalNode snode = SalNode.create(id);
            if (snode == null) {
                LOG.debug("Ignore unsupported node deletion: {}", id);
            } else {
                removed.put(path, snode);
            }
        }

        /**
         * {@inheritDoc}
         */
        @Override
        public boolean hasNode() {
            return !(created.isEmpty() && removed.isEmpty());
        }
    }

    /**
     * Construct a new instance.
     *
     * @param queue   A {@link TxQueue} instance used to update the
     *                VTN inventory.
     * @param broker  A {@link DataBroker} service instance.
     */
    public NodeListener(TxQueue queue, DataBroker broker) {
        super(queue, broker, Node.class, DataChangeScope.BASE);
        submitInitial(new NodesInitTask());
    }

    // DataStoreListener

    /**
     * {@inheritDoc}
     */
    @Override
    protected NodeEventContext enterEvent(
        AsyncDataChangeEvent<InstanceIdentifier<?>, DataObject> ev) {
        return new NodeUpdatedTask();
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void exitEvent(NodeEventContext ectx) {
        if (ectx.hasNode()) {
            submit(ectx);
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void onCreated(NodeEventContext ectx,
                             InstanceIdentifier<Node> key, Node value) {
        ectx.addCreated(key, value);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void onUpdated(NodeEventContext ectx,
                             InstanceIdentifier<Node> key, Node oldValue,
                             Node newValue) {
        throw new IllegalStateException("Should never be called.");
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void onRemoved(NodeEventContext ectx,
                             InstanceIdentifier<Node> key, Node value) {
        ectx.addRemoved(key, value);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected InstanceIdentifier<Node> getWildcardPath() {
        return InstanceIdentifier.builder(Nodes.class).child(Node.class).
            build();
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
        return REQUIRED_EVENTS;
    }
}
