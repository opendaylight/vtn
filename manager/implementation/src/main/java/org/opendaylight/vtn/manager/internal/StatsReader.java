/*
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal;

import static java.util.concurrent.TimeUnit.SECONDS;

import java.util.Deque;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.HashMap;
import java.util.Set;
import java.util.Timer;
import java.util.TimerTask;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ConcurrentLinkedDeque;
import java.util.concurrent.TimeUnit;

import org.opendaylight.controller.forwardingrulesmanager.FlowEntry;
import org.opendaylight.controller.sal.core.Node;
import org.opendaylight.controller.sal.reader.FlowOnNode;
import org.opendaylight.controller.statisticsmanager.IStatisticsManager;
import org.opendaylight.vtn.manager.VTNException;
import org.opendaylight.vtn.manager.VTenant;
import org.opendaylight.vtn.manager.flow.FlowStats;
import org.opendaylight.vtn.manager.internal.cluster.VTNFlow;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

/**
 * This class is used to read flow statistics from statistics manager.
 *
 * <p>
 *   This class is not synchronized.
 * </p>
 */
public class StatsReader extends TimerTask {

    private Timer timer;

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
     * VTN manager service.
     */
    private volatile VTNManagerImpl vtnManager;

    /**
     * A map which caches statistics derived from statistics manager for the last MAX_REQUEST_INTERVAL seconds
     * with STATS_INTERVAL seconds intervals.
     */
    private Map<FlowEntry, Deque<FlowOnNode>> statsCache = new ConcurrentHashMap<>();

    // interval for updating statistics information in seconds
    private static final int STATS_INTERVAL = 10;

    // the maximum value of the statistics interval in seconds
    private static final int MAX_REQUEST_INTERVAL = 30;

    // the value of the cache size
    private static final int MAX_CACHE_SIZE = MAX_REQUEST_INTERVAL / STATS_INTERVAL + 1;

    /**
     * Construct a new instance.
     *
     * @param stMgr    Statistics manager service.
     */
    public StatsReader(IStatisticsManager stMgr, VTNManagerImpl vtnMgr) {
        statsManager = stMgr;
        vtnManager = vtnMgr;
        timer = new Timer(true);
        timer.scheduleAtFixedRate(this, 0, SECONDS.toMillis(STATS_INTERVAL));
    }

    /**
     * Return statistics information about the specified flow entry.
     *
     * @param fent     A flow entry.
     * @param update   If {@code true}, flow statistics are derived from
     *                 physical switch. Otherwise this instance will return
     *                 statistics cached in statistics manager.
     * @param interval The statistics interval
     * @return         A {@link FlowStats} instance.
     *                 {@code null} is returned if statistics information about
     *                 the specified flow entry was not found.
     */
    public FlowStats get(FlowEntry fent, boolean update, int interval) {
        LOG.debug("Request for statistics: {}", fent);

        long packets;
        long bytes;
        long packetsPerSec;
        long bytesPerSec;
        int sec;
        int nsec;

        if (update) {
            FlowOnNode statsFromSwitch = getUpdatedStats(fent);
            if (statsFromSwitch != null) {
                packets = statsFromSwitch.getPacketCount();
                bytes = statsFromSwitch.getByteCount();
                sec = statsFromSwitch.getDurationSeconds();
                nsec = statsFromSwitch.getDurationNanoseconds();
            } else {
                LOG.debug("Statistics not found: {}", fent);
                return null;
            }

            Deque<FlowOnNode> stats = statsCache.get(fent);
            if (stats != null) {
                int valuesCount = Math.min(interval / STATS_INTERVAL + 1, stats.size());
                int i = 0;
                long[] packetsValues = new long[valuesCount];
                long[] bytesValues = new long[valuesCount];
                for (Iterator<FlowOnNode> flowStats = stats.iterator(); i < valuesCount; ++i) {
                    FlowOnNode nextStats = flowStats.next();
                    packetsValues[i] = nextStats.getPacketCount();
                    bytesValues[i] = nextStats.getByteCount();
                }
                packetsPerSec = averageStatistics(packetsValues);
                bytesPerSec = averageStatistics(bytesValues);
            } else {
                packetsPerSec = 0;
                bytesPerSec = 0;
            }
        } else {
            Deque<FlowOnNode> stats = statsCache.get(fent);
            if (stats != null) {
                FlowOnNode recentStats = stats.peekFirst();
                packets = recentStats.getPacketCount();
                bytes = recentStats.getByteCount();
                sec = recentStats.getDurationSeconds();
                nsec = recentStats.getDurationNanoseconds();

                int valuesCount = Math.min(interval / STATS_INTERVAL + 1, stats.size());
                int i = 0;
                long[] packetsValues = new long[valuesCount];
                long[] bytesValues = new long[valuesCount];
                for (Iterator<FlowOnNode> flowStats = stats.iterator(); i < valuesCount; ++i) {
                    FlowOnNode nextStats = flowStats.next();
                    packetsValues[i] = nextStats.getPacketCount();
                    bytesValues[i] = nextStats.getByteCount();
                }
                packetsPerSec = averageStatistics(packetsValues);
                bytesPerSec = averageStatistics(bytesValues);
            } else {
                LOG.debug("Statistics not found: {}", fent);
                return null;
            }
        }

        // Convert duration into the number of milliseconds.
        long duration = TimeUnit.SECONDS.toMillis((long)sec) +
            TimeUnit.NANOSECONDS.toMillis((long)nsec);

        FlowStats flowStats = new FlowStats(packets, bytes, duration, packetsPerSec, bytesPerSec, interval);
        LOG.debug("Statistics found: fent={}, stat={}", fent, flowStats);
        return flowStats;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void run() {
        try {
            update();
        } catch (VTNException | RuntimeException e) {
            LOG.error("Exception during statistics update", e);
        }
    }

    /**
     * Return a {@link FlowOnNode} instance which represents statistics
     * information from the switch about the specified flow entry.
     *
     * @param fent  A flow entry.
     * @return  A {@link FlowStats} instance.
     *          {@code null} is returned if statistics information about
     *          the specified flow entry was not found.
     */
    private FlowOnNode getUpdatedStats(FlowEntry fent) {
        Node node = fent.getNode();

        FlowOnNode found = null;
        List<FlowOnNode> stats = statsManager.getFlowsNoCache(node);
        LOG.trace("Statistics for node {}: {}", node, stats);

        for (FlowOnNode st: stats) {
            // Ignore flow tables other than table 0.
            if (st.getTableId() == 0) {
                FlowEntry f = new FlowEntry(null, null, st.getFlow(), node);
                if (fent.equals(f)) {
                    found = st;
                }
            }
        }
        return found;
    }

    private void update() throws VTNException {
        Map<Node, Map<FlowEntry, FlowOnNode>> cache = new HashMap<Node, Map<FlowEntry, FlowOnNode>>();
        Set<FlowEntry> flowEntries = new HashSet<>();
        for (VTenant tenant : vtnManager.getTenants()) {
            String tenantName = tenant.getName();
            VTNFlowDatabase fdb = vtnManager.getTenantFlowDB(tenantName);
            List<VTNFlow> flist = fdb.getAllFlows();
            for (VTNFlow vflow: flist) {
                FlowEntry fent = vflow.getFlowEntries().get(0);
                flowEntries.add(fent);
                Node node = fent.getNode();
                Map<FlowEntry, FlowOnNode> nodeCacheMap = cache.get(node);
                if (nodeCacheMap == null) {
                    List<FlowOnNode> stats = statsManager.getFlowsNoCache(node);
                    Map<FlowEntry, FlowOnNode> nodeMap = new HashMap<FlowEntry, FlowOnNode>();
                    for (FlowOnNode st: stats) {
                        // Ignore flow tables other than table 0.
                        if (st.getTableId() == 0) {
                            FlowEntry f = new FlowEntry(null, null, st.getFlow(), node);
                            if (fent.equals(f)) {
                                updateCache(fent, st);
                            }
                            nodeMap.put(f, st);
                        }
                    }
                    cache.put(node, nodeMap);
                } else {
                    FlowOnNode fentStats = nodeCacheMap.get(fent);
                    updateCache(fent, fentStats);
                }
            }
        }
        filterElements(flowEntries);
    }

    private void filterElements(Set<FlowEntry> validKeys) {
        if (statsCache.size() != validKeys.size()) {
            Set<FlowEntry> keys = statsCache.keySet();
            for (Iterator<FlowEntry> iterator = keys.iterator(); iterator.hasNext();) {
                FlowEntry key = iterator.next();
                if (!validKeys.contains(key)) {
                    statsCache.remove(key);
                }
            }
        }
    }

    private void updateCache(FlowEntry fent, FlowOnNode fentStats) {
        Deque<FlowOnNode> cache = statsCache.get(fent);
        if (cache == null) {
            cache = new ConcurrentLinkedDeque<FlowOnNode>();
        }
        cache.push(fentStats);
        if (cache.size() > MAX_CACHE_SIZE) {
            cache.pollLast();
        }
        statsCache.put(fent, cache);

    }

    private static long averageStatistics(long[] statisticsValues) {
        int length = statisticsValues.length;
        if (length > 1) {
            return (statisticsValues[0] - statisticsValues[length - 1]) / ((length - 1) * STATS_INTERVAL);
        } else {
            return 0;
        }
    }
}
