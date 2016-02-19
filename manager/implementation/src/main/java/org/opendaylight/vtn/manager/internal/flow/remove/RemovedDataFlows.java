/*
 * Copyright (c) 2015, 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.flow.remove;

import static org.opendaylight.vtn.manager.internal.flow.remove.FlowRemoveContext.LOG;

import java.util.ArrayList;
import java.util.List;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.FlowRemover;
import org.opendaylight.vtn.manager.internal.RemovedFlows;
import org.opendaylight.vtn.manager.internal.TxContext;
import org.opendaylight.vtn.manager.internal.util.flow.FlowCache;
import org.opendaylight.vtn.manager.internal.util.flow.FlowUtils;
import org.opendaylight.vtn.manager.internal.util.flow.RemoveFlowRpcList;
import org.opendaylight.vtn.manager.internal.util.inventory.InventoryReader;

import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.service.rev130819.SalFlowService;

/**
 * {@link RemovedDataFlows} describes a list of VTN data flows removed by
 * {@link FlowRemover}.
 *
 * @param <T>  The type of the flow remover.
 */
public class RemovedDataFlows<T extends FlowRemover> implements RemovedFlows {
    /**
     * A list of VTN data flows removed by {@link FlowRemover}.
     */
    private final List<FlowCache>  removedFlows = new ArrayList<>();

    /**
     * A flow remover that removed data flows.
     */
    private final T  flowRemover;

    /**
     * Construct an empty instance.
     *
     * @param remover  A {@link FlowRemover} instance that removed data flows.
     */
    public RemovedDataFlows(T remover) {
        flowRemover = remover;
    }

    /**
     * Return the flow remover.
     *
     * @return  A {@link FlowRemover} instance.
     */
    public final T getFlowRemover() {
        return flowRemover;
    }

    /**
     * Add the specified data flow to this instance.
     *
     * @param fc  A {@link FlowCache} instance.
     */
    final void add(FlowCache fc) {
        removedFlows.add(fc);
    }

    /**
     * Remove the given flow entries.
     *
     * @param sfs     MD-SAL flow service.
     * @param flows   A VTN data flows to be removed.
     * @param reader  An {@link InventoryReader} instance.
     * @return  A {@link RemoveFlowRpcList} instance.
     * @throws VTNException  An error occurred.
     */
    protected RemoveFlowRpcList removeFlowEntries(
        SalFlowService sfs, List<FlowCache> flows, InventoryReader reader)
        throws VTNException {
        return FlowUtils.removeFlowEntries(sfs, flows, reader);
    }

    // RemovedFlows

    /**
     * {@inheritDoc}
     */
    @Override
    public final RemoveFlowRpcList removeFlowEntries(TxContext ctx,
                                                     SalFlowService sfs)
        throws VTNException {
        if (LOG.isDebugEnabled()) {
            String desc = flowRemover.getDescription();
            for (FlowCache fc: removedFlows) {
                FlowUtils.removedLog(LOG, desc, fc);
            }
        }

        return removeFlowEntries(
            sfs, removedFlows, ctx.getReadSpecific(InventoryReader.class));
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public final boolean isEmpty() {
        return removedFlows.isEmpty();
    }
}
