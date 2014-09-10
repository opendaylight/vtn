/*
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal;

import java.util.List;
import java.util.Map;
import java.util.HashMap;
import java.util.concurrent.TimeUnit;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import org.opendaylight.vtn.manager.flow.FlowStats;

import org.opendaylight.controller.forwardingrulesmanager.FlowEntry;
import org.opendaylight.controller.sal.core.Node;
import org.opendaylight.controller.sal.reader.FlowOnNode;
import org.opendaylight.controller.statisticsmanager.IStatisticsManager;

/**
 * This class is used to read flow statistics from statistics manager.
 *
 * <p>
 *   This class is not synchronized.
 * </p>
 */
public class StatsReader {
    /**
     * Logger instance.
     */
    private static final Logger  LOG =
        LoggerFactory.getLogger(StatsReader.class);

    /**
     * Statistics manager service.
     */
    private final IStatisticsManager  statsManager;

    /**
     * Determine whether the statistics should be updated.
     */
    private final boolean  doUpdate;

    /**
     * A map which caches statistics derived from statistics manager.
     */
    private Map<Node, Map<FlowEntry, FlowOnNode>> statsCache;

    /**
     * Construct a new instance.
     *
     * @param stMgr    Statistics manager service.
     * @param update   if {@code true}, flow statistics are derived from
     *                 physical switch. Otherwise this instance will return
     *                 statistics cached in statistics manager.
     * @param cache    If {@code true} is specified, statistics information
     *                 derived from statistics manager will be cached in this
     *                 instance for succeeding request.
     */
    public StatsReader(IStatisticsManager stMgr, boolean update,
                       boolean cache) {
        statsManager = stMgr;
        doUpdate = update;
        if (cache) {
            statsCache = new HashMap<Node, Map<FlowEntry, FlowOnNode>>();
        }
    }

    /**
     * Return statistics information about the specified flow entry.
     *
     * @param fent  A flow entry.
     * @return  A {@link FlowStats} instance.
     *          {@code null} is returned if statistics information about
     *          the specified flow entry was not found.
     */
    public FlowStats get(FlowEntry fent) {
        LOG.debug("Request for statistics: {}", fent);
        FlowOnNode stats = getStats(fent);
        if (stats == null) {
            LOG.debug("Statistics not found: {}", fent);
            return null;
        }

        LOG.debug("Statistics found: fent={}, stat={}", fent, stats);

        // Convert duration into the number of milliseconds.
        int sec = stats.getDurationSeconds();
        int nsec = stats.getDurationNanoseconds();
        long duration = TimeUnit.SECONDS.toMillis((long)sec) +
            TimeUnit.NANOSECONDS.toMillis((long)nsec);

        return new FlowStats(stats.getPacketCount(), stats.getByteCount(),
                             duration);
    }

    /**
     * Return a {@link FlowOnNode} instance which represents statistics
     * information about the specified flow entry.
     *
     * @param fent  A flow entry.
     * @return  A {@link FlowStats} instance.
     *          {@code null} is returned if statistics information about
     *          the specified flow entry was not found.
     */
    private FlowOnNode getStats(FlowEntry fent) {
        Node node = fent.getNode();

        // Try to return from the cache.
        Map<FlowEntry, FlowOnNode> nodeMap;
        if (statsCache != null) {
            nodeMap = statsCache.get(node);
            if (nodeMap != null) {
                LOG.trace("Use statistics cache: node={}, cache={}",
                          node, nodeMap);
                return nodeMap.get(fent);
            }

            // Prepare map to cache statistics on a switch.
            nodeMap = new HashMap<FlowEntry, FlowOnNode>();
            statsCache.put(node, nodeMap);
        } else {
            nodeMap = null;
        }

        // Derive all the statistics for flows configured in the node.
        FlowOnNode found = null;
        List<FlowOnNode> stats = (doUpdate)
            ? statsManager.getFlowsNoCache(node)
            : statsManager.getFlows(node);

        LOG.trace("Statistics for node {}: {}", node, stats);
        for (FlowOnNode st: stats) {
            // Ignore flow tables other than table 0.
            if (st.getTableId() == 0) {
                FlowEntry f = new FlowEntry(null, null, st.getFlow(), node);
                if (fent.equals(f)) {
                    if (nodeMap == null) {
                        return st;
                    }
                    found = st;
                }
                if (nodeMap != null) {
                    nodeMap.put(f, st);
                }
            }
        }

        return found;
    }
}
