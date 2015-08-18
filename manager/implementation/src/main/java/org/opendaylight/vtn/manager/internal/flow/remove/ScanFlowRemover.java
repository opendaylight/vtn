/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.flow.remove;

import java.util.List;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.FlowRemover;
import org.opendaylight.vtn.manager.internal.TxContext;
import org.opendaylight.vtn.manager.internal.util.flow.FlowCache;
import org.opendaylight.vtn.manager.internal.util.flow.FlowUtils;

import org.opendaylight.controller.md.sal.binding.api.ReadWriteTransaction;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.tenant.flow.info.VtnDataFlow;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.vtn.flows.VtnFlowTable;

/**
 * An implementation of {@link FlowRemover} which determines VTN data flows
 * to be removed by a sequential scan.
 */
public abstract class ScanFlowRemover implements FlowRemover {
    /**
     * Return a list of VTN flow tables to be scanned.
     *
     * @param tx  A {@link ReadWriteTransaction} instance.
     * @return  A list of VTN flow tables.
     * @throws VTNException  An error occurred.
     */
    protected abstract List<VtnFlowTable> getFlowTables(
        ReadWriteTransaction tx) throws VTNException;

    /**
     * Determine whether the given data flow should be removed or not.
     *
     * @param fc  A {@link FlowCache} instance.
     * @return  {@code true} if the given data flow should be removed.
     *          Otherwise {@code false}.
     * @throws VTNException  An error occurred.
     */
    protected abstract boolean select(FlowCache fc) throws VTNException;

    // FlowRemover

    /**
     * {@inheritDoc}
     */
    @Override
    public final RemovedDataFlows removeDataFlow(TxContext ctx)
        throws VTNException {
        ReadWriteTransaction tx = ctx.getReadWriteTransaction();
        RemovedDataFlows removed = new RemovedDataFlows<ScanFlowRemover>(this);
        for (VtnFlowTable table: getFlowTables(tx)) {
            List<VtnDataFlow> flows = table.getVtnDataFlow();
            if (flows == null) {
                continue;
            }

            String tname = table.getTenantName();
            for (VtnDataFlow vdf: flows) {
                FlowCache fc = new FlowCache(vdf);
                if (select(fc) && FlowUtils.removeDataFlow(tx, tname, fc)) {
                    removed.add(fc);
                }
            }
        }

        return removed;
    }
}
