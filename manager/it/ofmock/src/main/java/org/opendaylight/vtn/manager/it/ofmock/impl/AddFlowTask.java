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

import com.google.common.util.concurrent.FutureCallback;
import com.google.common.util.concurrent.Futures;

import org.opendaylight.controller.md.sal.binding.api.ReadWriteTransaction;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;
import org.opendaylight.yangtools.yang.common.RpcResult;
import org.opendaylight.yangtools.yang.common.RpcResultBuilder;

import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.inventory.rev130819.FlowId;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.inventory.rev130819.tables.table.Flow;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.inventory.rev130819.tables.table.FlowBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.inventory.rev130819.tables.table.FlowKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.service.rev130819.AddFlowInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.service.rev130819.AddFlowOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.service.rev130819.AddFlowOutputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.rev131026.FlowRef;

/**
 * {@code AddFlowTask} describes a task that installs the specified MD-SAL
 * flow entry into the MD-SAL operational datastore.
 */
public final class AddFlowTask extends TxTask<RpcResult<AddFlowOutput>>
    implements FutureCallback<RpcResult<AddFlowOutput>> {
    /**
     * Logger instance.
     */
    private static final Logger  LOG =
        LoggerFactory.getLogger(AddFlowTask.class);

    /**
     * The target switch.
     */
    private final OfNode  targetNode;

    /**
     * The target flow table.
     */
    private final FlowTable  flowTable;

    /**
     * The path to the target flow entry.
     */
    private final InstanceIdentifier<Flow>  flowPath;

    /**
     * The MD-SAL flow entry to add.
     */
    private final Flow  flowEntry;

    /**
     * The ofmock flow entry.
     */
    private final OfMockFlowEntry  mockFlow;

    /**
     * The MD-SAL flow ID associated with the flow entry.
     */
    private final String  flowId;

    /**
     * Construct a new instance.
     *
     * @param node   The target switch.
     * @param input  The input for the add-flow RPC.
     * @param ofent  The ofmock flow entry created from the RPC input.
     * @param id     The MD-SAL flow ID associated with the flow entry.
     */
    public AddFlowTask(OfNode node, AddFlowInput input, OfMockFlowEntry ofent,
                       String id) {
        super(node.getOfMockProvider().getDataBroker());

        FlowRef ref = input.getFlowRef();
        if (ref != null) {
            throw new IllegalArgumentException(
                "flow-ref should not be specified: " + ref);
        }
        FlowId fid = new FlowId(id);
        FlowTable table = node.getFlowTable();

        targetNode = node;
        flowTable = table;
        mockFlow = ofent;
        flowId = id;
        flowEntry = new FlowBuilder(input).setId(fid).build();
        flowPath = table.getTablePath().
            child(Flow.class, new FlowKey(fid));
        ofent.setFlowEntry(flowEntry);
        Futures.addCallback(getFuture(), this);
    }

    // TxTask

    /**
     * {@inheritDoc}
     */
    @Override
    protected void execute(ReadWriteTransaction tx) {
        tx.put(LogicalDatastoreType.OPERATIONAL, flowPath, flowEntry, false);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected Logger getLogger() {
        return LOG;
    }

    /**
     * Return the value to be retuned by the task on successful completion.
     *
     * @return  The output of the add-flow RPC request.
     */
    @Override
    protected RpcResult<AddFlowOutput> getResult() {
        AddFlowOutput output = new AddFlowOutputBuilder().
            setTransactionId(targetNode.createTransactionId()).build();
        return RpcResultBuilder.success(output).build();
    }

    // FutureCallback

    /**
     * Invoked when the specified flow entry has been added successfully.
     *
     * @param result  The result of the add-flow RPC.
     */
    @Override
    public void onSuccess(RpcResult<AddFlowOutput> result) {
        flowTable.complete(flowId, mockFlow);
    }

    /**
     * Invoked when the specified flow entry was not be added.
     *
     * @param cause  A throwable that indicates the cause of error.
     */
    @Override
    public void onFailure(Throwable cause) {
        flowTable.abort(mockFlow);
    }
}
