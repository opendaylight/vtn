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

import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.inventory.rev130819.tables.table.Flow;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.statistics.rev130819.flow.and.statistics.map.list.FlowAndStatisticsMapList;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.transaction.rev150304.TransactionId;

/**
 * {@code FlowStatsNotifier} is used to publish MD-SAL notification that
 * indicates the result of get-flow-statistics-from-table RPC.
 */
public final class FlowStatsNotifier implements Runnable {
    /**
     * The target switch.
     */
    private final OfNode  targetNode;

    /**
     * The flow matcher to select flow entries.
     */
    private final FlowMatcher  matcher;

    /**
     * The transaction ID associated with the flow statistics request.
     */
    private final TransactionId  txId;

    /**
     * Construct a new instance.
     *
     * @param node   The target switch.
     * @param match  The flow matcher to select flow entries.
     * @param xid    The transaction ID associated with the flow statistics
     *               request.
     */
    public FlowStatsNotifier(OfNode node, FlowMatcher match,
                             TransactionId xid) {
        targetNode = node;
        matcher = match;
        txId = xid;
    }

    // Runnable

    /**
     * Collect flow statistics for the target flow table.
     */
    @Override
    public void run() {
        List<OfMockFlowEntry> flows = targetNode.getFlowTable().getFlows();
        List<FlowAndStatisticsMapList> stats = new ArrayList<>();
        for (OfMockFlowEntry ofent: flows) {
            Flow flow = ofent.getFlowEntry();
            if (matcher.match(flow)) {
                stats.add(OfNode.getStatistics(flow));
            }
        }

        // Publish a flows-statistics-update notification.
        targetNode.publishFlowStatsUpdate(txId, stats);
    }
}
