/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.flow.reader;

import java.util.List;

import com.google.common.base.Optional;
import com.google.common.util.concurrent.FutureCallback;
import com.google.common.util.concurrent.Futures;

import org.opendaylight.vtn.manager.internal.TxContext;
import org.opendaylight.vtn.manager.internal.util.flow.FlowUtils;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcOutputGenerator;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcUtils;

import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.GetDataFlowCountInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.GetDataFlowCountOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.GetDataFlowCountOutputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.tenant.flow.info.VtnDataFlow;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.vtn.flows.VtnFlowTable;

/**
 * {@code FlowCountFuture} describes a future associated with the task which
 * retrieves the number of data flows present in the specified VTN.
 */
public final class FlowCountFuture extends AbstractReadFlowFuture<Integer>
    implements FutureCallback<Optional<VtnFlowTable>>,
               RpcOutputGenerator<Integer, GetDataFlowCountOutput> {
    /**
     * Construct a new instance.
     *
     * @param ctx    A {@link TxContext} instance.
     * @param input  Input of the RPC call.
     * @return  A {@link FlowCountFuture} instance.
     * @throws RpcException  An error occurred.
     */
    public static FlowCountFuture create(TxContext ctx,
                                         GetDataFlowCountInput input)
        throws RpcException {
        if (input == null) {
            throw RpcUtils.getNullInputException();
        }

        return new FlowCountFuture(ctx, input);
    }

    /**
     * Construct a new instance.
     *
     * @param ctx    A {@link TxContext} instance.
     * @param input  Input of the RPC call.
     * @throws RpcException  An error occurred.
     */
    private FlowCountFuture(TxContext ctx, GetDataFlowCountInput input)
        throws RpcException {
        super(ctx, input.getTenantName());

        // Read the VTN flow table.
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        InstanceIdentifier<VtnFlowTable> path =
            FlowUtils.getIdentifier(input.getTenantName());
        Futures.addCallback(ctx.getTransaction().read(oper, path), this);
    }

    // FutureCallback

    /**
     * Invoked when the read operation has completed successfully.
     *
     * @param result  An {@link Optional} instance that contains the VTN flow
     *                table.
     */
    @Override
    public void onSuccess(Optional<VtnFlowTable> result) {
        if (result.isPresent()) {
            List<VtnDataFlow> flows = result.get().getVtnDataFlow();
            int count = (flows == null) ? 0 : flows.size();
            setResult(Integer.valueOf(count));
        } else if (checkTenant()) {
            // The target VTN is present, but no data flow is installed.
            setResult(Integer.valueOf(0));
        }
    }

    /**
     * Invoked when the read operation has failed.
     *
     * @param cause  A {@link Throwable} that indicates the cause of failure.
     */
    @Override
    public void onFailure(Throwable cause) {
        setFailure(cause);
    }

    // RpcOutputGenerator

    /**
     * {@inheritDoc}
     */
    @Override
    public Class<GetDataFlowCountOutput> getOutputType() {
        return GetDataFlowCountOutput.class;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public GetDataFlowCountOutput createOutput(Integer result) {
        return new GetDataFlowCountOutputBuilder().setCount(result).build();
    }
}
