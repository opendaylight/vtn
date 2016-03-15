/*
 * Copyright (c) 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.ofmock.impl;

import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.Callable;

import com.google.common.util.concurrent.ListenableFutureTask;

import org.opendaylight.yangtools.yang.common.RpcResult;
import org.opendaylight.yangtools.yang.common.RpcResultBuilder;

import org.opendaylight.yang.gen.v1.urn.opendaylight.direct.statistics.rev160511.GetFlowStatisticsOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.direct.statistics.rev160511.GetFlowStatisticsOutputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.inventory.rev130819.tables.table.Flow;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.statistics.rev130819.flow.and.statistics.map.list.FlowAndStatisticsMapList;

/**
 * {@code GetFlowStatsTask} describes a task that implements
 * get-flow-statistics RPC.
 */
public final class GetFlowStatsTask
    implements Callable<RpcResult<GetFlowStatisticsOutput>> {
    /**
     * The target switch.
     */
    private final OfNode  targetNode;

    /**
     * The flow matcher to select flow entries.
     */
    private final FlowMatcher  matcher;

    /**
     * Create a new future task associated with the get-flow-statistics
     * RPC task.
     *
     * @param node   The target switch.
     * @param match  The flow matcher to select flow entries.
     * @return  The future task associated with the get-flow-statistics RPC
     *          task.
     */
    public static ListenableFutureTask<RpcResult<GetFlowStatisticsOutput>> create(
        OfNode node, FlowMatcher match) {
        return ListenableFutureTask.create(new GetFlowStatsTask(node, match));
    }

    /**
     * Construct a new instance.
     *
     * @param node   The target switch.
     * @param match  The flow matcher to select flow entries.
     */
    private GetFlowStatsTask(OfNode node, FlowMatcher match) {
        targetNode = node;
        matcher = match;
    }

    // Callable

    /**
     * Collect flow statistics for the target flow table.
     */
    @Override
    public RpcResult<GetFlowStatisticsOutput> call() {
        List<OfMockFlowEntry> flows = targetNode.getFlowTable().getFlows();
        List<FlowAndStatisticsMapList> stats = new ArrayList<>();
        for (OfMockFlowEntry ofent: flows) {
            Flow flow = ofent.getFlowEntry();
            if (matcher.match(flow)) {
                stats.add(OfNode.getStatistics(flow));
            }
        }

        GetFlowStatisticsOutput output = new GetFlowStatisticsOutputBuilder().
            setFlowAndStatisticsMapList(stats).
            build();
        return RpcResultBuilder.success(output).build();
    }
}
