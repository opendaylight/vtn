/*
 * Copyright (c) 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.ofmock.impl;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import com.google.common.base.Optional;
import com.google.common.util.concurrent.FutureCallback;
import com.google.common.util.concurrent.Futures;

import org.opendaylight.vtn.manager.it.ofmock.DataStoreUtils;

import org.opendaylight.controller.md.sal.binding.api.ReadWriteTransaction;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;
import org.opendaylight.yangtools.yang.common.RpcResult;
import org.opendaylight.yangtools.yang.common.RpcResultBuilder;

import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.inventory.rev130819.FlowId;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.inventory.rev130819.tables.Table;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.inventory.rev130819.tables.table.Flow;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.inventory.rev130819.tables.table.FlowKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.service.rev130819.RemoveFlowInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.service.rev130819.RemoveFlowOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.service.rev130819.RemoveFlowOutputBuilder;

/**
 * {@code RemoveFlowTask} describes a task that removes the specified MD-SAL
 * flow entries from the MD-SAL operational datastore.
 */
public abstract class RemoveFlowTask
    extends TxTask<RpcResult<RemoveFlowOutput>>
    implements FutureCallback<RpcResult<RemoveFlowOutput>> {
    /**
     * Logger instance.
     */
    private static final Logger  LOG =
        LoggerFactory.getLogger(RemoveFlowTask.class);

    /**
     * The target switch.
     */
    private final OfNode  targetNode;

    /**
     * The target flow table.
     */
    private final FlowTable  flowTable;

    /**
     * A task that implements strict remove-flow RPC operation.
     */
    private static final class Strict extends RemoveFlowTask {
        /**
         * The ofmock flow entry that specifies the condition.
         */
        private final OfMockFlowEntry  targetFlow;

        /**
         * The condition for the flow cookie.
         */
        private final CookieMatcher  cookieMatcher;

        /**
         * The path to the target flow entry.
         */
        private final InstanceIdentifier<Flow>  flowPath;

        /**
         * The ID of the removed flow entry.
         */
        private String  removedFlow;

        /**
         * Construct a new instance.
         *
         * @param node   The target node.
         * @param input  The input for the remove-flow RPC.
         */
        protected Strict(OfNode node, RemoveFlowInput input) {
            super(node, input);
            targetFlow = new OfMockFlowEntry(node.getNodeIdentifier(), input);
            cookieMatcher = new CookieMatcher(
                node.getOfVersion(), input.getCookie(), input.getCookieMask());

            // Determine the flow ID.
            FlowTable table = getFlowTable();
            String id = table.getFlowId(targetFlow);
            if (id == null) {
                // Already removed.
                flowPath = null;
                complete();
            } else {
                flowPath = table.getTablePath().
                    child(Flow.class, new FlowKey(new FlowId(id)));
            }
        }

        /**
         * {@inheritDoc}
         */
        @Override
        protected void execute(ReadWriteTransaction tx) {
            removedFlow = null;

            // Fetch the target flow entry.
            LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
            Optional<Flow> opt = DataStoreUtils.read(tx, oper, flowPath);
            if (opt.isPresent()) {
                Flow flow =  opt.get();
                if (targetFlow.equalsFlow(flow) &&
                    cookieMatcher.match(flow)) {
                    // Delete the target flow entry.
                    tx.delete(oper, flowPath);
                    removedFlow = flow.getId().getValue();
                }
            }
        }

        // FutureCallback

        /**
         * Invoked when the specified flow entry has been removed successfully.
         *
         * @param result  The result of the remove-flow RPC.
         */
        @Override
        public void onSuccess(RpcResult<RemoveFlowOutput> result) {
            if (removedFlow != null) {
                getFlowTable().remove(removedFlow);
            }
        }
    }

    /**
     * A task that implements bulk remove-flow RPC operation.
     */
    private static final class Bulk extends RemoveFlowTask {
        /**
         * The flow matcher that filters the flow entry.
         */
        private final FlowMatcher  matcher;

        /**
         * The path to the target flow table.
         */
        private final InstanceIdentifier<Table>  tablePath;

        /**
         * Removed flow entries.
         */
        private RemovedFlows  removedFlows;

        /**
         * Construct a new instance.
         *
         * @param node   The target node.
         * @param input  The input for the remove-flow RPC.
         */
        protected Bulk(OfNode node, RemoveFlowInput input) {
            super(node, input);
            matcher = new FlowMatcher(node, input);
            tablePath = getFlowTable().getTablePath();
        }

        // TxTask

        /**
         * {@inheritDoc}
         */
        @Override
        protected void execute(ReadWriteTransaction tx) {
            // Remove all the flow entries that do not satisfy the condition.
            removedFlows = getFlowTable().removeFlows(matcher);
            if (removedFlows != null) {
                tx.put(LogicalDatastoreType.OPERATIONAL, tablePath,
                       removedFlows.getTable(), false);
            }
        }

        // FutureCallback

        /**
         * Invoked when the specified flow entry has been removed successfully.
         *
         * @param result  The result of the remove-flow RPC.
         */
        @Override
        public void onSuccess(RpcResult<RemoveFlowOutput> result) {
            if (removedFlows != null) {
                getFlowTable().remove(removedFlows.getRemoved());
            }
        }
    }

    /**
     * Create a task that removes the specified flow entries.
     *
     * @param node   The target node.
     * @param input  The input for the remove-flow RPC.
     * @return  A {@link RemoveFlowTask} instance.
     */
    public static final RemoveFlowTask create(OfNode node,
                                              RemoveFlowInput input) {
        return (Boolean.TRUE.equals(input.isStrict()))
            ? new Strict(node, input)
            : new Bulk(node, input);
    }

    /**
     * Construct a new instance.
     *
     * @param node   The target node.
     * @param input  The input for the remove-flow RPC.
     */
    private RemoveFlowTask(OfNode node, RemoveFlowInput input) {
        super(node.getOfMockProvider().getDataBroker());
        targetNode = node;
        flowTable = node.getFlowTable();
        Futures.addCallback(getFuture(), this);
    }

    /**
     * Return the flow table.
     *
     * @return  A {@link FlowTable} instance.
     */
    protected final FlowTable getFlowTable() {
        return flowTable;
    }

    // TxTask

    /**
     * {@inheritDoc}
     */
    @Override
    protected final Logger getLogger() {
        return LOG;
    }

    /**
     * Return the value to be retuned by the task on successful completion.
     *
     * @return  The output of the remove-flow RPC request.
     */
    @Override
    protected final RpcResult<RemoveFlowOutput> getResult() {
        RemoveFlowOutput output = new RemoveFlowOutputBuilder().
            setTransactionId(targetNode.createTransactionId()).build();
        return RpcResultBuilder.success(output).build();
    }

    // FutureCallback

    /**
     * Invoked when the specified flow entry was not be removed.
     *
     * @param cause  A throwable that indicates the cause of error.
     */
    @Override
    public final void onFailure(Throwable cause) {
    }
}
