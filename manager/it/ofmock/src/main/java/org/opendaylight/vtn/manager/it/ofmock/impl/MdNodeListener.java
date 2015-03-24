/*
 * Copyright (c) 2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.ofmock.impl;

import java.util.Set;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import org.opendaylight.controller.md.sal.binding.api.DataBroker;
import org.opendaylight.controller.md.sal.common.api.data.AsyncDataBroker.DataChangeScope;
import org.opendaylight.controller.md.sal.common.api.data.AsyncDataChangeEvent;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.controller.sal.core.UpdateType;

import org.opendaylight.yangtools.yang.binding.DataObject;
import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;

import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.inventory.rev130819.FlowCapableNode;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.NodeId;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.Nodes;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.nodes.Node;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.nodes.NodeKey;

/**
 * {@code MdNodeListener} listens changes to the MD-SAL node information.
 */
public final class MdNodeListener
    extends DataStoreListener<FlowCapableNode, Void> {
    /**
     * Logger instance.
     */
    private static final Logger  LOG =
        LoggerFactory.getLogger(MdNodeListener.class);

    /**
     * AD-SAL inventory manager.
     */
    private final AdSalInventory  adInventory;

    /**
     * Construct a new instance.
     *
     * @param broker  Data broker service.
     * @param adsal   AD-SAL inventory manager.
     */
    public MdNodeListener(DataBroker broker, AdSalInventory adsal) {
        super(FlowCapableNode.class);
        adInventory = adsal;
        registerListener(broker, LogicalDatastoreType.OPERATIONAL,
                         DataChangeScope.SUBTREE);
    }

    /**
     * Return a MD-SAL node identifier in the given instance identifier.
     *
     * @param path  An {@link InstanceIdentifier} instance.
     * @return  A MD-SAL node identifier if found.
     *          {@code null} if not found.
     */
    private String getNodeIdentifier(InstanceIdentifier<?> path) {
        if (path == null) {
            return null;
        }

        NodeKey key = path.firstKeyOf(Node.class, NodeKey.class);
        if (key == null) {
            return null;
        }

        NodeId nodeId = key.getId();
        return (nodeId == null) ? null : nodeId.getValue();
    }

    /**
     * Invoked when a MD-SAL node has been updated.
     *
     * @param path  Path to the flow-capable-node.
     * @param fcn   A {@link FlowCapableNode} instance.
     * @param type  An {@link UpdateType} instance which indicates the type of
     *              the event.
     */
    private void nodeUpdated(InstanceIdentifier<FlowCapableNode> path,
                             FlowCapableNode fcn, UpdateType type) {
        if (fcn == null) {
            return;
        }

        String nid = getNodeIdentifier(path);
        if (nid != null) {
            LOG.trace("{}: MD-SAL node has been updated: nid={}, fcn={}",
                      type, nid, fcn);
            adInventory.notifyNodeUpdated(nid, fcn, type);
        }
    }

    /**
     * Invoked when a MD-SAL node has been removed.
     *
     * @param path  Path to the flow-capable-node.
     */
    private void nodeRemoved(InstanceIdentifier<FlowCapableNode> path) {
        String nid = getNodeIdentifier(path);
        if (nid != null) {
            LOG.trace("REMOVED: MD-SAL node has been removed: {}", nid);
            adInventory.notifyNodeRemoved(nid);
        }
    }

    // DataStoreListener

    /**
     * {@inheritDoc}
     */
    @Override
    protected Void enterEvent(
        AsyncDataChangeEvent<InstanceIdentifier<?>, DataObject> ev) {
        return null;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void exitEvent(Void ectx) {
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void onCreated(Void ectx,
                             InstanceIdentifier<FlowCapableNode> path,
                             FlowCapableNode data) {
        nodeUpdated(path, data, UpdateType.ADDED);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void onUpdated(Void ectx,
                             InstanceIdentifier<FlowCapableNode> path,
                             FlowCapableNode oldData, FlowCapableNode newData) {
        nodeUpdated(path, newData, UpdateType.CHANGED);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void onRemoved(Void ectx,
                             InstanceIdentifier<FlowCapableNode> path,
                             FlowCapableNode data) {
        nodeRemoved(path);
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
