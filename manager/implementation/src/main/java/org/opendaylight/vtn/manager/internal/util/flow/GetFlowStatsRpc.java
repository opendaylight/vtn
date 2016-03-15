/*
 * Copyright (c) 2015, 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.flow;

import java.util.concurrent.Future;

import com.google.common.util.concurrent.Futures;

import org.opendaylight.vtn.manager.internal.util.inventory.NodeRpcInvocation;
import org.opendaylight.vtn.manager.internal.util.inventory.NodeRpcWatcher;

import org.opendaylight.yangtools.yang.common.RpcResult;

import org.opendaylight.yang.gen.v1.urn.opendaylight.direct.statistics.rev160511.GetFlowStatisticsInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.direct.statistics.rev160511.GetFlowStatisticsOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.direct.statistics.rev160511.OpendaylightDirectStatisticsService;

/**
 * {@code GetFlowStatsRpc} describes an invocation of
 * get-flow-statistics RPC provided by the MD-SAL direct statistics service.
 */
public final class GetFlowStatsRpc
    extends NodeRpcInvocation<GetFlowStatisticsInput, GetFlowStatisticsOutput> {
     /**
      * Issue a get-flow-statistics RPC request.
      *
      * @param dss  MD-SAL direct statistics service.
      * @param in   The RPC input.
      * @return  A future associated with the RPC.
      */
    private static Future<RpcResult<GetFlowStatisticsOutput>> invoke(
        OpendaylightDirectStatisticsService dss, GetFlowStatisticsInput in) {
        try {
            return dss.getFlowStatistics(in);
        } catch (RuntimeException e) {
            return Futures.<RpcResult<GetFlowStatisticsOutput>>
                immediateFailedFuture(e);
        }
    }

    /**
     * Issue a get-flow-statistics RPC request.
     *
     * @param w    The node RPC watcher.
     * @param dss  MD-SAL direct statistics service.
     * @param in   The RPC input.
     */
    public GetFlowStatsRpc(NodeRpcWatcher w,
                           OpendaylightDirectStatisticsService dss,
                           GetFlowStatisticsInput in) {
        super(w, in, in.getNode(), invoke(dss, in));
    }

    // RpcRequest

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
