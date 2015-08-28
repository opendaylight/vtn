/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.inventory;

import java.util.Collections;
import java.util.ArrayList;
import java.util.EnumSet;
import java.util.List;
import java.util.Set;

import com.google.common.base.Optional;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.TxContext;
import org.opendaylight.vtn.manager.internal.TxQueue;
import org.opendaylight.vtn.manager.internal.VTNManagerProvider;
import org.opendaylight.vtn.manager.internal.util.ChangedData;
import org.opendaylight.vtn.manager.internal.util.DataStoreUtils;
import org.opendaylight.vtn.manager.internal.util.IdentifiedData;
import org.opendaylight.vtn.manager.internal.util.MiscUtils;
import org.opendaylight.vtn.manager.internal.util.inventory.InventoryUtils;
import org.opendaylight.vtn.manager.internal.util.inventory.SalNode;
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

import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.inventory.rev130819.FlowCapableNode;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.NodeId;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.Nodes;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.nodes.Node;

/**
 * Listener class that listens the change of MD-SAL nodes.
 */
public final class NodeListener
    extends InventoryMaintainer<FlowCapableNode, NodeUpdateTask> {
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
            // Check to see if the vtn-nodes container is present.
            // Initialize it only if it is not present.
            ReadWriteTransaction tx = ctx.getReadWriteTransaction();
            InstanceIdentifier<VtnNodes> path =
                InstanceIdentifier.create(VtnNodes.class);
            LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
            Optional<VtnNodes> opt = DataStoreUtils.read(tx, oper, path);
            if (!opt.isPresent()) {
                // Read all the nodes in the MD-SAL datastore.
                InstanceIdentifier<Nodes> nodesPath =
                    InstanceIdentifier.create(Nodes.class);
                Nodes nodes = DataStoreUtils.read(tx, oper, nodesPath).
                    orNull();
                List<VtnNode> vnList = getVtnNodeList(nodes);
                VtnNodes vnodes = new VtnNodesBuilder().
                    setVtnNode(vnList).build();

                // Initialize vtn-nodes container.
                tx.put(oper, path, vnodes, true);
            }

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
     * Construct a new instance.
     *
     * @param queue   A {@link TxQueue} instance used to update the
     *                VTN inventory.
     * @param broker  A {@link DataBroker} service instance.
     */
    public NodeListener(TxQueue queue, DataBroker broker) {
        super(queue, broker, FlowCapableNode.class, DataChangeScope.SUBTREE);
        submitInitial(new NodesInitTask());
    }

    /**
     * Add the given node information to the node update task.
     *
     * @param ectx  A {@link NodeUpdateTask} instance.
     * @param data  An {@link IdentifiedData} instance.
     * @param type  A {@link VtnUpdateType} instance which indicates the type
     *              of event.
     */
    private void addUpdated(NodeUpdateTask ectx,
                            IdentifiedData<FlowCapableNode> data,
                            VtnUpdateType type) {
        InstanceIdentifier<FlowCapableNode> path = data.getIdentifier();
        NodeId nid = InventoryUtils.getNodeId(path);
        SalNode snode = SalNode.create(nid);
        if (snode == null) {
            LOG.debug("{}: Ignore unsupported node event: {}", type, path);
        } else {
            ectx.addUpdated(path, snode);
        }
    }

    // DataStoreListener

    /**
     * {@inheritDoc}
     */
    @Override
    protected NodeUpdateTask enterEvent(
        AsyncDataChangeEvent<InstanceIdentifier<?>, DataObject> ev) {
        return new NodeUpdateTask(LOG);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void exitEvent(NodeUpdateTask ectx) {
        if (ectx.hasUpdates()) {
            submit(ectx);
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void onCreated(NodeUpdateTask ectx,
                             IdentifiedData<FlowCapableNode> data) {
        addUpdated(ectx, data, VtnUpdateType.CREATED);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void onUpdated(NodeUpdateTask ectx,
                             ChangedData<FlowCapableNode> data) {
        throw MiscUtils.unexpected();
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void onRemoved(NodeUpdateTask ectx,
                             IdentifiedData<FlowCapableNode> data) {
        addUpdated(ectx, data, VtnUpdateType.REMOVED);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected InstanceIdentifier<FlowCapableNode> getWildcardPath() {
        return InstanceIdentifier.builder(Nodes.class).child(Node.class).
            augmentation(FlowCapableNode.class).build();
    }

    /**
     * Return a set of {@link VtnUpdateType} instances that specifies
     * event types to be listened.
     *
     * @return  A set of {@link VtnUpdateType} instances.
     */
    @Override
    protected Set<VtnUpdateType> getRequiredEvents() {
        return REQUIRED_EVENTS;
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
