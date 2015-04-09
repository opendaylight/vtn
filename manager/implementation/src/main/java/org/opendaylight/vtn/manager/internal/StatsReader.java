/*
 * Copyright (c) 2014-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal;

import static java.util.concurrent.TimeUnit.SECONDS;

import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.NavigableMap;
import java.util.Set;
import java.util.Timer;
import java.util.TimerTask;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ConcurrentSkipListMap;
import java.util.concurrent.TimeUnit;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import org.opendaylight.vtn.manager.VTNException;
import org.opendaylight.vtn.manager.VTenant;
import org.opendaylight.vtn.manager.flow.AveragedFlowStats;
import org.opendaylight.vtn.manager.flow.DataFlow;
import org.opendaylight.vtn.manager.flow.FlowStats;
import org.opendaylight.vtn.manager.internal.cluster.VTNFlow;

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
public class StatsReader extends TimerTask {
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
    private Map<FlowEntry, NavigableMap<Long, StatsHistory>> statsCache =
        new ConcurrentHashMap<>();

    // interval for updating statistics information in seconds
    private static final int STATS_INTERVAL = 10;

    // the maximum value of the statistics interval in seconds
    private static final int MAX_REQUEST_INTERVAL = 30;

    // the value of the cache size
    private static final int MAX_CACHE_SIZE = MAX_REQUEST_INTERVAL / STATS_INTERVAL + 1;

    /**
     * History record for flow statistics.
     */
    private static final class StatsHistory {
        /**
         * The number of milliseconds in a second.
         */
        private static final double  MILLISEC = 1000D;

        /**
         * The flow statistics information.
         */
        private final FlowOnNode  statistics;

        /**
         * The system time when the flow statistics are derived.
         */
        private final long  systemTime;

        /**
         * Construct a new instance.
         *
         * @param stats  A {@link FlowOnNode} instance.
         */
        private StatsHistory(FlowOnNode stats) {
            this(stats, System.currentTimeMillis());
        }

        /**
         * Construct a new instance.
         *
         * @param stats  A {@link FlowOnNode} instance.
         * @param time   The system time when the flow statistics are derived.
         */
        private StatsHistory(FlowOnNode stats, long time) {
            statistics = stats;
            systemTime = time;
        }

        /**
         * Set the flow statistics into the given data flow information.
         *
         * @param df  A {@link DataFlow} instance.
         */
        private void setStatistics(DataFlow df) {
            long sec = (long)statistics.getDurationSeconds();
            long nsec = (long)statistics.getDurationNanoseconds();
            long duration = TimeUnit.SECONDS.toMillis(sec) +
                TimeUnit.NANOSECONDS.toMillis(nsec);

            long packets = statistics.getPacketCount();
            long bytes = statistics.getByteCount();
            df.setStatistics(new FlowStats(packets, bytes, duration));
        }

        /**
         * Set the averaged flow statistics into the given data flow
         * information.
         *
         * @param df     A {@link DataFlow} instance.
         * @param start  A {@link StatsHistory} instance which contains the
         *               flow statistics at the start of the measurement
         *               period.
         */
        private void setAveragedStatistics(DataFlow df, StatsHistory start) {
            FlowOnNode startStats = start.statistics;
            double spackets = (double)startStats.getPacketCount();
            double sbytes = (double)startStats.getByteCount();
            double epackets = statistics.getPacketCount();
            double ebytes = statistics.getByteCount();
            double interval =
                (double)(systemTime - start.systemTime) / MILLISEC;

            double packets = (epackets - spackets) / interval;
            double bytes = (ebytes - sbytes) / interval;
            AveragedFlowStats average = new AveragedFlowStats(
                packets, bytes, start.systemTime, systemTime);
            df.setAveragedStatistics(average);
        }

        /**
         * Return the system time when the flow statistics are derived.
         *
         * @return  The system time when the flow statistics are derived.
         */
        private long getSystemTime() {
            return systemTime;
        }

        @Override
        public String toString() {
            return "StatsHistory[stats=" + statistics + ", time=" + systemTime + "]";
        }
    }

    /**
     * Construct a new instance.
     *
     * @param stMgr    Statistics manager service.
     * @param vtnMgr   VTN Manager service.
     * @param timer    A timer thread.
     */
    public StatsReader(IStatisticsManager stMgr, VTNManagerImpl vtnMgr,
                       Timer timer) {
        statsManager = stMgr;
        vtnManager = vtnMgr;
        timer.scheduleAtFixedRate(this, 0, SECONDS.toMillis(STATS_INTERVAL));
    }

    /**
     * Set flow statistics information about the specified flow entry into the
     * given {@link DataFlow} instance.
     *
     * @param df       A {@link DataFlow} instance.
     * @param fent     A flow entry.
     * @param update   If {@code true}, flow statistics are derived from
     *                 physical switch. Otherwise this instance will return
     *                 statistics cached in statistics manager.
     * @param interval The statistics interval
     */
    public void set(DataFlow df, FlowEntry fent, boolean update, int interval) {
        LOG.debug("Request for statistics: {}", fent);

        NavigableMap<Long, StatsHistory> history = statsCache.get(fent);
        StatsHistory latest = null;
        if (update) {
            latest = getUpdatedStats(fent);
        } else if (history != null) {
            Map.Entry<Long, StatsHistory> last = history.lastEntry();
            if (last != null) {
                latest = last.getValue();
            }
        }
        if (latest == null) {
            LOG.debug("Statistics not found: {}", fent);
            return;
        }

        latest.setStatistics(df);

        if (history == null) {
            LOG.debug("Statistics hisotry not found: {}", fent);
            return;
        }

        // Determine the oldest record in the measurement period.
        long endTime = latest.getSystemTime();
        StatsHistory oldest = getStartRecord(history, endTime, interval);
        if (oldest == null) {
            LOG.debug("Failed to determine the start record: {}", fent);
            return;
        }

        LOG.debug("Averange: fent={}, oldest={}, latest={}", fent, oldest, latest);
        latest.setAveragedStatistics(df, oldest);
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
     * @return  A {@link StatsHistory} instance.
     *          {@code null} is returned if statistics information about
     *          the specified flow entry was not found.
     */
    private StatsHistory getUpdatedStats(FlowEntry fent) {
        Node node = fent.getNode();

        List<FlowOnNode> stats = statsManager.getFlowsNoCache(node);
        LOG.trace("Statistics for node {}: {}", node, stats);

        for (FlowOnNode st: stats) {
            // Ignore flow tables other than table 0.
            if (st.getTableId() == 0) {
                FlowEntry f = new FlowEntry(null, null, st.getFlow(), node);
                if (fent.equals(f)) {
                    return new StatsHistory(st);
                }
            }
        }
        return null;
    }

    /**
     * Update the flow statistics history records.
     *
     * @throws VTNException  An error occurred.
     */
    private void update() throws VTNException {
        Map<Node, Map<FlowEntry, StatsHistory>> cache = new HashMap<>();
        Set<FlowEntry> flowEntries = new HashSet<>();
        for (VTenant tenant : vtnManager.getTenants()) {
            String tenantName = tenant.getName();
            VTNFlowDatabase fdb = vtnManager.getTenantFlowDB(tenantName);
            if (fdb == null) {
                continue;
            }

            List<VTNFlow> flist = fdb.getAllFlows();
            for (VTNFlow vflow: flist) {
                FlowEntry fent = vflow.getFlowEntries().get(0);
                flowEntries.add(fent);
                Node node = fent.getNode();
                Map<FlowEntry, StatsHistory> nodeCacheMap = cache.get(node);
                if (nodeCacheMap == null) {
                    List<FlowOnNode> stats = statsManager.getFlowsNoCache(node);
                    Map<FlowEntry, StatsHistory> nodeMap = new HashMap<>();
                    long time = System.currentTimeMillis();
                    for (FlowOnNode st: stats) {
                        // Ignore flow tables other than table 0.
                        if (st.getTableId() == 0) {
                            FlowEntry f =
                                new FlowEntry(null, null, st.getFlow(), node);
                            StatsHistory sh = new StatsHistory(st, time);
                            if (fent.equals(f)) {
                                updateCache(fent, sh);
                            }
                            nodeMap.put(f, sh);
                        }
                    }
                    cache.put(node, nodeMap);
                } else {
                    StatsHistory sh = nodeCacheMap.get(fent);
                    if (sh != null) {
                        updateCache(fent, sh);
                    }
                }
            }
        }
        filterElements(flowEntries);
    }

    /**
     * Remove obsolete statistics history records.
     *
     * @param validKeys  A set of {@link FlowEntry} instances that contains
     *                   all valid flow entries.
     */
    private void filterElements(Set<FlowEntry> validKeys) {
        for (Iterator<FlowEntry> it = statsCache.keySet().iterator();
             it.hasNext();) {
            FlowEntry fent = it.next();
            if (!validKeys.contains(fent)) {
                LOG.debug("Remove stats history: {}", fent);
                it.remove();
            }
        }
    }

    /**
     * Update the flow statistics history.
     *
     * @param fent   A {@link FlowEntry} instance.
     * @param stats  A {@link StatsHistory} instance which contains the flow
     *               statistics.
     */
    private void updateCache(FlowEntry fent, StatsHistory stats) {
        NavigableMap<Long, StatsHistory> history = statsCache.get(fent);
        if (history == null) {
            history = new ConcurrentSkipListMap<Long, StatsHistory>();
            statsCache.put(fent, history);
        }

        LOG.debug("Update stat cache: fent={}, stats={}", fent, stats);
        history.put(stats.getSystemTime(), stats);
        if (history.size() > MAX_CACHE_SIZE) {
            Long first = history.firstKey();
            if (first != null) {
                StatsHistory removed = history.remove(first);
                if (removed != null) {
                    LOG.debug("Remove stat cache: fent={}, stats={}", fent, removed);
                }
            }
        }
    }

    /**
     * Return the oldest statistics history record in the measurement period.
     *
     * @param history   A statistics history map.
     * @param end       The end time of the measurement period.
     * @param interval  The statistics interval in seconds.
     * @return  A {@link StatsHistory} instance if found.
     *          {@code null} if not found.
     */
    private StatsHistory getStartRecord(
        NavigableMap<Long, StatsHistory> history, long end, int interval) {
        int ival = (interval <= 0) ? STATS_INTERVAL : interval;
        long startTime = end - TimeUnit.SECONDS.toMillis((long)ival);
        Long key = Long.valueOf(startTime);
        Map.Entry<Long, StatsHistory> floorEntry = history.floorEntry(key);
        Map.Entry<Long, StatsHistory> ceilingEntry = history.ceilingEntry(key);
        StatsHistory start;
        if (floorEntry == null) {
            if (ceilingEntry == null) {
                return null;
            }
            start = ceilingEntry.getValue();
        } else if (ceilingEntry == null) {
            start = floorEntry.getValue();
        } else {
            // Choose nearer to the expected start time.
            StatsHistory floor = floorEntry.getValue();
            StatsHistory ceil = ceilingEntry.getValue();
            long fdiff = startTime - floor.getSystemTime();
            long cdiff = ceil.getSystemTime() - startTime;
            start = (fdiff < cdiff) ? floor : ceil;
        }

        return (start.getSystemTime() >= end) ? null : start;
    }
}
