/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.flow.stats;

import java.util.Collections;
import java.util.Set;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import org.opendaylight.vtn.manager.internal.TxQueue;
import org.opendaylight.vtn.manager.internal.VTNManagerProvider;
import org.opendaylight.vtn.manager.internal.util.ChangedData;
import org.opendaylight.vtn.manager.internal.util.DataStoreListener;
import org.opendaylight.vtn.manager.internal.util.IdentifiedData;
import org.opendaylight.vtn.manager.internal.util.MiscUtils;
import org.opendaylight.vtn.manager.internal.util.flow.FlowUtils;

import org.opendaylight.controller.md.sal.common.api.data.AsyncDataBroker.DataChangeScope;
import org.opendaylight.controller.md.sal.common.api.data.AsyncDataChangeEvent;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.yangtools.yang.binding.DataObject;
import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;

import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.inventory.rev130819.tables.table.Flow;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.inventory.rev130819.FlowCapableNode;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.inventory.rev130819.tables.Table;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.inventory.rev130819.tables.TableKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.Nodes;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.nodes.Node;

/**
 * {@code SalFlowIdResolver} is used to resolve MD-SAL flow ID assigned to
 * the VTN flow.
 *
 * <p>
 *   Flow entries installed by the VTN Manager has no MD-SAL flow ID because
 *   it installs flow entries without using the MD-SAL FRM. But pseudo MD-SAL
 *   flow identifier will be assigned to every flow entries by the MD-SAL
 *   statistics manager. This class listens the flow statistics in the MD-SAL
 *   datastore, and determine MD-SAL flow ID assigned assigned by the MD-SAL
 *   statistics manager.
 * </p>
 */
public final class SalFlowIdResolver
    extends DataStoreListener<Flow, AddedFlowStats> {
    /**
     * Logger instance.
     */
    private static final Logger  LOG =
        LoggerFactory.getLogger(SalFlowIdResolver.class);

    /**
     * Required event types.
     */
    private static final Set<VtnUpdateType>  REQUIRED_EVENTS =
        Collections.singleton(VtnUpdateType.CREATED);

    /**
     * The MD-SAL datastore transaction queue for updating VTN flow
     * information.
     */
    private final TxQueue  txQueue;

    /**
     * Construct a new instance.
     *
     * @param provider  VTN Manager provider service.
     * @param txq       A {@link TxQueue} instance used to update the VTN
     *                  flow information.
     */
    public SalFlowIdResolver(VTNManagerProvider provider, TxQueue txq) {
        super(Flow.class);
        txQueue = txq;

        registerListener(provider.getDataBroker(),
                         LogicalDatastoreType.OPERATIONAL,
                         DataChangeScope.BASE, false);
    }

    // DataStoreListener

    /**
     * {@inheritDoc}
     */
    @Override
    protected AddedFlowStats enterEvent(
        AsyncDataChangeEvent<InstanceIdentifier<?>, DataObject> ev) {
        return new AddedFlowStats(LOG);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void exitEvent(AddedFlowStats ectx) {
        ectx.apply(txQueue);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void onCreated(AddedFlowStats ectx, IdentifiedData<Flow> data) {
        ectx.add(data);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void onUpdated(AddedFlowStats ectx, ChangedData<Flow> data) {
        throw MiscUtils.unexpected();
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void onRemoved(AddedFlowStats ectx, IdentifiedData<Flow> data) {
        throw MiscUtils.unexpected();
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected InstanceIdentifier<Flow> getWildcardPath() {
        return InstanceIdentifier.builder(Nodes.class).
            child(Node.class).
            augmentation(FlowCapableNode.class).
            child(Table.class, new TableKey(FlowUtils.TABLE_ID)).
            child(Flow.class).
            build();
    }

    // AbstractDataChangeListener

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
