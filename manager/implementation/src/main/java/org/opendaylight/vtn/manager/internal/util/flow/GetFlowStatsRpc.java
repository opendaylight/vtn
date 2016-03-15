/*
 * Copyright (c) 2015, 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.flow;

import java.math.BigInteger;
import java.util.concurrent.Future;
import java.util.concurrent.TimeUnit;

import org.slf4j.Logger;

import com.google.common.util.concurrent.Futures;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.util.inventory.NodeRpcInvocation;
import org.opendaylight.vtn.manager.internal.util.inventory.NodeRpcWatcher;

import org.opendaylight.yangtools.yang.common.RpcResult;

import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.statistics.rev130819.GetFlowStatisticsFromFlowTableInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.statistics.rev130819.GetFlowStatisticsFromFlowTableOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.statistics.rev130819.OpendaylightFlowStatisticsService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.transaction.rev150304.TransactionId;

/**
 * {@code GetFlowStatsRpc} describes an invocation of
 * get-flow-statistics-from-flow-table RPC provided by the MD-SAL flow
 * statistics service.
 */
public final class GetFlowStatsRpc
    extends NodeRpcInvocation<GetFlowStatisticsFromFlowTableInput,
                              GetFlowStatisticsFromFlowTableOutput> {
    /**
     * Issue a get-flow-statistics-from-flow-table RPC request.
     *
     * @param fss  MD-SAL flow statistics service.
     * @param in   The RPC input.
     * @return  A future associated with the RPC.
     */
    private static Future<RpcResult<GetFlowStatisticsFromFlowTableOutput>> invoke(
        OpendaylightFlowStatisticsService fss,
        GetFlowStatisticsFromFlowTableInput in) {
        try {
            return fss.getFlowStatisticsFromFlowTable(in);
        } catch (IllegalAccessError | RuntimeException e) {
            return Futures.<RpcResult<GetFlowStatisticsFromFlowTableOutput>>
                immediateFailedFuture(e);
        }
    }

    /**
     * Issue a get-flow-statistics-from-flow-table RPC request.
     *
     * @param w    The node RPC watcher.
     * @param fss  MD-SAL flow statistics service.
     * @param in   The RPC input.
     */
    public GetFlowStatsRpc(NodeRpcWatcher w,
                           OpendaylightFlowStatisticsService fss,
                           GetFlowStatisticsFromFlowTableInput in) {
        super(w, in, in.getNode(), invoke(fss, in));
    }

    /**
     * Wait for the RPC execution to complete, and return the transaction ID
     * returned by the RPC.
     *
     * @param timeout  The maximum time to wait.
     * @param unit     The time unit of the {@code timeout} argument.
     * @param logger   A {@link Logger} instance.
     * @return  The transaction ID associated with the read transaction.
     * @throws VTNException  An error occurred.
     */
    public BigInteger getTransactionId(long timeout, TimeUnit unit,
                                       Logger logger) throws VTNException {
        GetFlowStatisticsFromFlowTableOutput result =
            getResult(timeout, unit, logger);
        if (result == null) {
            String msg = "Flow stats RPC did not return the result";
            logger.error("{}: {}", getName(), msg);
            throw new VTNException(msg);
        }

        TransactionId txId = result.getTransactionId();
        if (txId == null) {
            String msg = "Flow stats RPC did not return XID";
            logger.error("{}: {}: {}", getName(), msg, result);
            throw new VTNException(msg);
        }

        return txId.getValue();
    }

    // RpcInvocation

    /**
     * Return the name of the RPC.
     *
     * @return  "get-flow-stats".
     */
    @Override
    public String getName() {
        return "get-flow-stats";
    }
}
