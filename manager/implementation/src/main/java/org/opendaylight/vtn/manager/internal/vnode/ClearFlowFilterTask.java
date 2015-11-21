/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.vnode;

import java.util.List;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.TxContext;
import org.opendaylight.vtn.manager.internal.VTNManagerProvider;
import org.opendaylight.vtn.manager.internal.flow.remove.TenantFlowRemover;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcOutputGenerator;
import org.opendaylight.vtn.manager.internal.util.tx.AbstractRpcTask;
import org.opendaylight.vtn.manager.internal.util.vnode.VNodeIdentifier;

import org.opendaylight.controller.md.sal.binding.api.ReadWriteTransaction;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.RemoveFlowFilterOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.RemoveFlowFilterOutputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.flow.filter.result.FlowFilterResult;

/**
 * {@code ClearFlowFilterTask} describes the MD-SAL datastore transaction task
 * that deletes the specified container that contains flow filter list.
 */
public final class ClearFlowFilterTask
    extends AbstractRpcTask<List<FlowFilterResult>>
    implements RpcOutputGenerator<List<FlowFilterResult>,
                                  RemoveFlowFilterOutput> {
    /**
     * The identifier for the target virtual node.
     */
    private final VNodeIdentifier<?>  identifier;

    /**
     * A boolean value which specifies the direction of the packet.
     */
    private final boolean  output;

    /**
     * Construct a new instance.
     *
     * @param ident   A {@link VNodeIdentifier} instance that specifies the
     *                target virtual node.
     * @param out     {@code true} indicates the output filter list.
     *                {@code false} indicates the input filter list.
     */
    ClearFlowFilterTask(VNodeIdentifier<?> ident, boolean out) {
        identifier = ident;
        output = out;
    }

    // AbstractTxTask

    /**
     * Clear the specified flow filter list.
     *
     * @param ctx  A runtime context for transaction task.
     * @return  A list of {@link FlowFilterResult} instances that contains
     *          all the removed flow filter indices.
     *          {@code null} if the specified flow filter list is empty.
     * @throws VTNException  An error occurred.
     */
    @Override
    protected List<FlowFilterResult> execute(TxContext ctx)
        throws VTNException {
        // Ensure that the target virtual node is present.
        ReadWriteTransaction tx = ctx.getReadWriteTransaction();
        identifier.fetch(tx);

        // Clear the flow filter list.
        // This will throw an exception if the specified virtual node does not
        // support flow filter.
        return identifier.clearFlowFilter(tx, output);
    }

    // TxTask

    /**
     * {@inheritDoc}
     */
    @Override
    public void onSuccess(VTNManagerProvider provider,
                          List<FlowFilterResult> result) {
        if (result != null) {
            // REVISIT: Select flow entries affected by the change.
            String tname = identifier.getTenantNameString();
            TenantFlowRemover remover = new TenantFlowRemover(tname);
            addBackgroundTask(provider.removeFlows(remover));
        }
    }

    // RpcOutputGenerator

    /**
     * {@inheritDoc}
     */
    @Override
    public Class<RemoveFlowFilterOutput> getOutputType() {
        return RemoveFlowFilterOutput.class;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public RemoveFlowFilterOutput createOutput(List<FlowFilterResult> result) {
        return new RemoveFlowFilterOutputBuilder().
            setFlowFilterResult(result).build();
    }
}
