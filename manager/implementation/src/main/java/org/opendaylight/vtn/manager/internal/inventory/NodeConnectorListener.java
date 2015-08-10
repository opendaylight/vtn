/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.inventory;


import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import org.opendaylight.vtn.manager.internal.TxQueue;
import org.opendaylight.vtn.manager.internal.util.ChangedData;
import org.opendaylight.vtn.manager.internal.util.IdentifiedData;
import org.opendaylight.vtn.manager.internal.util.MiscUtils;
import org.opendaylight.vtn.manager.internal.util.inventory.InventoryUtils;
import org.opendaylight.vtn.manager.internal.util.inventory.SalPort;

import org.opendaylight.controller.md.sal.binding.api.DataBroker;
import org.opendaylight.controller.md.sal.common.api.data.AsyncDataBroker.DataChangeScope;
import org.opendaylight.controller.md.sal.common.api.data.AsyncDataChangeEvent;

import org.opendaylight.yangtools.yang.binding.DataObject;
import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;

import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.inventory.rev130819.FlowCapableNodeConnector;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.NodeConnectorId;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.Nodes;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.node.NodeConnector;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.nodes.Node;

/**
 * Listener class that listens the change of MD-SAL node connectors.
 */
public final class NodeConnectorListener
    extends InventoryMaintainer<FlowCapableNodeConnector, PortUpdateTask> {
    /**
     * Logger instance.
     */
    private static final Logger  LOG =
        LoggerFactory.getLogger(NodeConnectorListener.class);

    /**
     * Construct a new instance.
     *
     * @param queue   A {@link TxQueue} instance used to update the
     *                VTN inventory.
     * @param broker  A {@link DataBroker} service instance.
     */
    public NodeConnectorListener(TxQueue queue, DataBroker broker) {
        super(queue, broker, FlowCapableNodeConnector.class,
              DataChangeScope.SUBTREE);
    }

    /**
     * Add the given node connector information to the port update task.
     *
     * @param ectx  A {@link PortUpdateTask} instance.
     * @param data  An {@link IdentifiedData} instance.
     * @param type  A {@link VtnUpdateType} instance which indicates the type
     *              of event.
     */
    private void addUpdated(PortUpdateTask ectx,
                            IdentifiedData<FlowCapableNodeConnector> data,
                            VtnUpdateType type) {
        InstanceIdentifier<FlowCapableNodeConnector> path =
            data.getIdentifier();
        NodeConnectorId id = InventoryUtils.getNodeConnectorId(path);
        SalPort sport = SalPort.create(id);

        if (sport == null) {
            Logger log = (SalPort.isLogicalPort(id))
                ? MiscUtils.VERBOSE_LOG : LOG;
            if (log.isDebugEnabled()) {
                log.debug("{}: Ignore unsupported node connector event: {}",
                          type, path);
            }
        } else {
            ectx.addUpdated(path, sport);
        }
    }

    // DataStoreListener

    /**
     * {@inheritDoc}
     */
    @Override
    protected PortUpdateTask enterEvent(
        AsyncDataChangeEvent<InstanceIdentifier<?>, DataObject> ev) {
        return new PortUpdateTask(LOG);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void exitEvent(PortUpdateTask ectx) {
        if (ectx.hasUpdates()) {
            submit(ectx);
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void onCreated(PortUpdateTask ectx,
                             IdentifiedData<FlowCapableNodeConnector> data) {
        addUpdated(ectx, data, VtnUpdateType.CREATED);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void onUpdated(PortUpdateTask ectx,
                             ChangedData<FlowCapableNodeConnector> data) {
        addUpdated(ectx, data, VtnUpdateType.CHANGED);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void onRemoved(PortUpdateTask ectx,
                             IdentifiedData<FlowCapableNodeConnector> data) {
        addUpdated(ectx, data, VtnUpdateType.REMOVED);
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

    // CloseableContainer

    /**
     * {@inheritDoc}
     */
    @Override
    protected Logger getLogger() {
        return LOG;
    }
}
