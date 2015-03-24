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

import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.inventory.rev130819.FlowCapableNodeConnector;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.NodeConnectorId;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.Nodes;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.node.NodeConnector;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.node.NodeConnectorKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.nodes.Node;

/**
 * {@code MdPortListener} listens changes to the MD-SAL node connector
 * information.
 */
public final class MdPortListener
    extends DataStoreListener<FlowCapableNodeConnector, Void> {
    /**
     * Logger instance.
     */
    private static final Logger  LOG =
        LoggerFactory.getLogger(MdPortListener.class);

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
    public MdPortListener(DataBroker broker, AdSalInventory adsal) {
        super(FlowCapableNodeConnector.class);
        adInventory = adsal;
        registerListener(broker, LogicalDatastoreType.OPERATIONAL,
                         DataChangeScope.SUBTREE);
    }

    /**
     * Return a MD-SAL node connector identifier in the given instance
     * identifier.
     *
     * @param path  An {@link InstanceIdentifier} instance.
     * @return  A MD-SAL node connector identifier if found.
     *          {@code null} if not found.
     */
    private String getPortIdentifier(InstanceIdentifier<?> path) {
        if (path == null) {
            return null;
        }

        NodeConnectorKey key =
            path.firstKeyOf(NodeConnector.class, NodeConnectorKey.class);
        if (key == null) {
            return null;
        }

        NodeConnectorId ncId = key.getId();
        return (ncId == null) ? null : ncId.getValue();
    }

    /**
     * Invoked when a MD-SAL node connector has been updated.
     *
     * @param path  Path to the flow-capable-node-connector.
     * @param fcnc  A {@link FlowCapableNodeConnector} instance.
     * @param type  An {@link UpdateType} instance which indicates the type of
     *              the event.
     */
    private void portUpdated(InstanceIdentifier<FlowCapableNodeConnector> path,
                             FlowCapableNodeConnector fcnc, UpdateType type) {
        if (fcnc == null) {
            return;
        }

        String pid = getPortIdentifier(path);
        if (pid != null) {
            LOG.trace("{}: MD-SAL node connector has been updated: pid={}, " +
                      "fcnc={}", type, pid, fcnc);
            adInventory.notifyPortUpdated(pid, fcnc, type);
        }
    }

    /**
     * Invoked when a MD-SAL node connector has been removed.
     *
     * @param path  Path to the flow-capable-node-capable.
     */
    private void portRemoved(
        InstanceIdentifier<FlowCapableNodeConnector> path) {
        String pid = getPortIdentifier(path);
        if (pid != null) {
            LOG.trace("REMOVED: MD-SAL node connector has been removed: {}",
                      pid);
            adInventory.notifyPortRemoved(pid);
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
                             InstanceIdentifier<FlowCapableNodeConnector> path,
                             FlowCapableNodeConnector data) {
        portUpdated(path, data, UpdateType.ADDED);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void onUpdated(Void ectx,
                             InstanceIdentifier<FlowCapableNodeConnector> path,
                             FlowCapableNodeConnector oldData,
                             FlowCapableNodeConnector newData) {
        portUpdated(path, newData, UpdateType.CHANGED);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void onRemoved(Void ectx,
                             InstanceIdentifier<FlowCapableNodeConnector> path,
                             FlowCapableNodeConnector data) {
        portRemoved(path);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected InstanceIdentifier<FlowCapableNodeConnector> getWildcardPath() {
        return InstanceIdentifier.builder(Nodes.class).child(Node.class).
            child(NodeConnector.class).
            augmentation(FlowCapableNodeConnector.class).build();
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
