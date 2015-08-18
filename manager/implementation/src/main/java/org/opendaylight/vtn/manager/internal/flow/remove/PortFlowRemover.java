/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.flow.remove;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.TxContext;
import org.opendaylight.vtn.manager.internal.util.flow.FlowCache;
import org.opendaylight.vtn.manager.internal.util.flow.FlowUtils;
import org.opendaylight.vtn.manager.internal.util.inventory.SalPort;

import org.opendaylight.controller.md.sal.binding.api.ReadWriteTransaction;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.tenant.flow.info.PortFlows;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.tenant.flow.info.PortFlowsKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.vtn.flows.VtnFlowTable;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.vtn.flows.VtnFlowTableKey;

/**
 * An implementation of
 * {@link org.opendaylight.vtn.manager.internal.FlowRemover} which removes
 * VTN data flows related to the specified switch port.
 *
 * <p>
 *   This flow remover affects flow entries in all the VTNs.
 * </p>
 */
public final class PortFlowRemover
    extends IndexFlowRemover<PortFlows, RemovedPortFlows> {
    /**
     * The target port.
     */
    private final SalPort  flowPort;

    /**
     * The key which specifies the target port in the port flow index.
     */
    private final PortFlowsKey  portKey;

    /**
     * Construct a new instance.
     *
     * @param sport  A {@link SalPort} instance.
     */
    public PortFlowRemover(SalPort sport) {
        flowPort = sport;
        portKey = new PortFlowsKey(sport.getNodeConnectorId());
    }

    /**
     * Return the target switch port.
     *
     * @return  A {@link SalPort} instance whcih specifies the target switch
     *          port.
     */
    public SalPort getFlowPort() {
        return flowPort;
    }

    // IndexFlowRemover

    /**
     * Return the path to the port flow index to be removed.
     *
     * @param key  A {@link VtnFlowTableKey} instance which specifies the
     *             target VTN flow table.
     * @return  Path to the port flow index to be removed.
     */
    @Override
    protected InstanceIdentifier<PortFlows> getPath(VtnFlowTableKey key) {
        return FlowUtils.getIdentifier(key, portKey);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void add(RemovedPortFlows removed, FlowCache fc) {
        removed.add(fc);
    }

    // FlowRemover

    /**
     * {@inheritDoc}
     */
    @Override
    public RemovedPortFlows removeDataFlow(TxContext ctx)
        throws VTNException {
        ReadWriteTransaction tx = ctx.getReadWriteTransaction();
        RemovedPortFlows removed = new RemovedPortFlows(this);
        for (VtnFlowTable table: FlowUtils.getFlowTables(tx)) {
            remove(removed, tx, table);
        }

        return removed;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public String getDescription() {
        return new StringBuilder("port[").
            append(flowPort).append(']').toString();
    }
}
