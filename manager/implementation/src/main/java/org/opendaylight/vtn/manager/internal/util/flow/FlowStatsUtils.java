/*
 * Copyright (c) 2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.flow;

import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.TreeMap;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.util.DataStoreUtils;

import org.opendaylight.controller.md.sal.binding.api.ReadTransaction;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.tenant.flow.info.VtnDataFlowKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.vtn.data.flow.fields.FlowStatsHistory;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.vtn.data.flow.fields.FlowStatsHistoryBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.vtn.data.flow.fields.flow.stats.history.FlowStatsRecord;

import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.inventory.rev130819.FlowCapableNode;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.inventory.rev130819.FlowId;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.inventory.rev130819.tables.Table;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.inventory.rev130819.tables.TableKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.inventory.rev130819.tables.table.Flow;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.inventory.rev130819.tables.table.FlowKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.statistics.rev130819.FlowStatisticsData;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.statistics.rev130819.flow.statistics.FlowStatistics;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.NodeId;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.Nodes;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.nodes.Node;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.nodes.NodeKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.model.statistics.types.rev130925.GenericStatistics;
import org.opendaylight.yang.gen.v1.urn.opendaylight.model.statistics.types.rev130925.duration.Duration;

/**
 * {@code FlowStatsUtils} class is a collection of utility class methods for
 * flow statistics management.
 */
public final class FlowStatsUtils {
    /**
     * The lifetime, in milliseconds, of statistics history record.
     */
    private static final long  HISTORY_LIFETIME = 60000L;

    /**
     * The minimum interval in milliseconds between statistics history records.
     */
    private static final long  MIN_INTERVAL = 1000L;

    /**
     * Private constructor that protects this class from instantiating.
     */
    private FlowStatsUtils() {}


    /**
     * Create the instance identifier for the statistics history of the
     * given VTN data flow.
     *
     * @param name     The name of the VTN.
     * @param flowKey  A {@link VtnDataFlowKey} instance which contains the
     *                 target flow ID.
     * @return  An {@link InstanceIdentifier} instance.
     */
    public static InstanceIdentifier<FlowStatsHistory> getIdentifier(
        String name, VtnDataFlowKey flowKey) {
        return FlowUtils.getIdentifier(name, flowKey).
            child(FlowStatsHistory.class);
    }

    /**
     * Create the instance identifier of the statistics information for the
     * given MD-SAL flow.
     *
     * @param node    The MD-SAL node identifier.
     * @param flowId  The MD-SAL flow identifier.
     * @return  An {@link InstanceIdentifier} instance.
     */
    public static InstanceIdentifier<FlowStatistics> getIdentifier(
        NodeId node, FlowId flowId) {
        return InstanceIdentifier.builder(Nodes.class).
            child(Node.class, new NodeKey(node)).
            augmentation(FlowCapableNode.class).
            child(Table.class, new TableKey(FlowUtils.TABLE_ID)).
            child(Flow.class, new FlowKey(flowId)).
            augmentation(FlowStatisticsData.class).
            child(FlowStatistics.class).
            build();
    }

    /**
     * Read statistics information for the given MD-SAL flow.
     *
     * @param rtx     A {@link ReadTransaction} instance.
     * @param node    The MD-SAL node identifier.
     * @param flowId  The MD-SAL flow identifier.
     * @return  A {@link FlowStatistics} instance if available.
     *          {@code null} if not available.
     * @throws VTNException  An error occurred.
     */
    public static FlowStatistics read(ReadTransaction rtx, NodeId node,
                                      FlowId flowId)
        throws VTNException {
        InstanceIdentifier<FlowStatistics> path = getIdentifier(node, flowId);
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        return DataStoreUtils.read(rtx, oper, path).orNull();
    }

    /**
     * Ensure that the given flow statistics contains valid values.
     *
     * @param fstats  A flow statistics to be checked.
     * @return  {@code null} if the given flow statistics is valid.
     *          An error message if the given flow statistics is invalid.
     */
    public static String check(GenericStatistics fstats) {
        if (fstats == null) {
            return "flow statistics is null.";
        }
        if (fstats.getPacketCount() == null) {
            return "No packet count.";
        }
        if (fstats.getByteCount() == null) {
            return "No byte count.";
        }

        Duration duration = fstats.getDuration();
        if (duration == null) {
            return "No duration.";
        }

        if (duration.getSecond() == null) {
            return "No second in duration.";
        }
        if (duration.getNanosecond() == null) {
            return "No nanosecond in duration.";
        }

        return null;
    }

    /**
     * Update the flow statistics history.
     *
     * @param history    History of flow statistics.
     * @param newRecord  A statistics record to be added.
     * @return  A {@link FlowStatsHistory} instance if the history has to be
     *          updated. Otherwise {@code null}.
     */
    public static FlowStatsHistory update(FlowStatsHistory history,
                                          FlowStatsRecord newRecord) {
        TreeMap<Long, FlowStatsRecord> map = new TreeMap<>();
        if (history != null) {
            List<FlowStatsRecord> list = history.getFlowStatsRecord();
            if (list != null) {
                for (FlowStatsRecord fsr: list) {
                    map.put(fsr.getTime(), fsr);
                }
            }
        }

        // Expire old records.
        Long newTime = newRecord.getTime();
        long expire = newTime.longValue() - HISTORY_LIFETIME;
        for (Long time = map.firstKey(); time != null; time = map.firstKey()) {
            if (expire > time.longValue()) {
                map.remove(time);
            } else {
                break;
            }
        }

        // Determine the latest record.
        Map.Entry<Long, FlowStatsRecord> latest = map.lastEntry();
        if (latest != null) {
            // Prevent too frequent records.
            Long ltime = latest.getKey();
            if (newTime.longValue() - ltime.longValue() < MIN_INTERVAL) {
                FlowStatsRecord lfsr = latest.getValue();
                if (Boolean.TRUE.equals(lfsr.isPeriodic())) {
                    // Don't remove periodic record.
                    return null;
                }
                map.remove(ltime);
            }
        }

        // Return a new statistics history.
        map.put(newTime, newRecord);
        List<FlowStatsRecord> list = new ArrayList<>(map.values());
        return new FlowStatsHistoryBuilder().
            setFlowStatsRecord(list).build();
    }
}
