/*
 * Copyright (c) 2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.flow;

import static org.opendaylight.vtn.manager.internal.util.flow.FlowUtils.COOKIE_MASK_ALL;
import static org.opendaylight.vtn.manager.internal.util.flow.FlowUtils.COOKIE_MASK_VTN;
import static org.opendaylight.vtn.manager.internal.util.flow.FlowUtils.COOKIE_VTN;
import static org.opendaylight.vtn.manager.internal.util.flow.FlowUtils.TABLE_ID;

import java.math.BigInteger;
import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.NavigableMap;
import java.util.TreeMap;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.util.DataStoreUtils;
import org.opendaylight.vtn.manager.internal.util.inventory.SalNode;

import org.opendaylight.controller.md.sal.binding.api.ReadTransaction;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.tenant.flow.info.VtnDataFlowKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.vtn.data.flow.fields.FlowStatsHistory;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.vtn.data.flow.fields.FlowStatsHistoryBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.vtn.data.flow.fields.VtnFlowEntry;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.vtn.data.flow.fields.flow.stats.history.FlowStatsRecord;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.vtn.data.flow.fields.flow.stats.history.FlowStatsRecordBuilder;

import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.inventory.rev130819.FlowCapableNode;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.inventory.rev130819.FlowId;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.inventory.rev130819.tables.Table;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.inventory.rev130819.tables.TableKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.inventory.rev130819.tables.table.Flow;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.inventory.rev130819.tables.table.FlowKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.statistics.rev130819.FlowStatisticsData;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.statistics.rev130819.GetFlowStatisticsFromFlowTableInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.statistics.rev130819.GetFlowStatisticsFromFlowTableInputBuilder;
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
     * The lifetime of statistics history record in milliseconds.
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
            child(Table.class, new TableKey(TABLE_ID)).
            child(Flow.class, new FlowKey(flowId)).
            augmentation(FlowStatisticsData.class).
            child(FlowStatistics.class).
            build();
    }

    /**
     * Construct an RPC input which requests flow statistics for the given
     * VTN flow entry.
     *
     * @param vfent  The target VTN flow entry.
     * @return  A {@link GetFlowStatisticsFromFlowTableInput} instance.
     */
    public static GetFlowStatisticsFromFlowTableInput createGetFlowStatsInput(
        VtnFlowEntry vfent) {
        GetFlowStatisticsFromFlowTableInputBuilder builder =
            new GetFlowStatisticsFromFlowTableInputBuilder(vfent);

        SalNode snode = SalNode.create(vfent.getNode());
        return builder.setNode(snode.getNodeRef()).
            setCookieMask(COOKIE_MASK_ALL).
            build();
    }

    /**
     * Construct an RPC input which requests statistics information about all
     * VTN flow entries present in the given switch.
     *
     * <p>
     *   Note that this method uses flow cookie to select VTN flow entries.
     *   So OpenFlow 1.0 switch will return all the flow entries when it
     *   receives an input returned by this method.
     * </p>
     *
     * @param snode  A {@link SalNode} instance which specifies the target
     *               switch.
     * @return  A {@link GetFlowStatisticsFromFlowTableInput} instance.
     */
    public static GetFlowStatisticsFromFlowTableInput createGetFlowStatsInput(
        SalNode snode) {
        return new GetFlowStatisticsFromFlowTableInputBuilder().
            setNode(snode.getNodeRef()).
            setCookie(COOKIE_VTN).
            setCookieMask(COOKIE_MASK_VTN).
            setTableId(TABLE_ID).
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
        String msg = null;

        if (fstats == null) {
            msg = "flow statistics is null.";
        } else if (fstats.getPacketCount() == null) {
            msg = "No packet count.";
        } else if (fstats.getByteCount() == null) {
            msg = "No byte count.";
        } else {
            Duration duration = fstats.getDuration();
            if (duration == null) {
                msg = "No duration.";
            } else if (duration.getSecond() == null) {
                msg = "No second in duration.";
            } else if (duration.getNanosecond() == null) {
                msg = "No nanosecond in duration.";
            }
        }

        return msg;
    }

    /**
     * Put all the flow statistics records in the given statistics history
     * into a new navigable map.
     *
     * <p>
     *   All statistics records are associated with its creation time in a
     *   returned map.
     * </p>
     *
     * @param history  History of flow statistics.
     * @return  A {@link NavigableMap} instance.
     */
    public static NavigableMap<Long, FlowStatsRecord> toNavigableMap(
        FlowStatsHistory history) {
        NavigableMap<Long, FlowStatsRecord> map = new TreeMap<>();
        if (history != null) {
            List<FlowStatsRecord> list = history.getFlowStatsRecord();
            if (list != null) {
                for (FlowStatsRecord fsr: list) {
                    map.put(fsr.getTime(), fsr);
                }
            }
        }

        return map;
    }

    /**
     * Add a periodic flow statistics record to the statistics history.
     *
     * @param history  History of flow statistics.
     * @param sysTime  The system time when the statistics is derived.
     * @param gstats   A statistics record to be added.
     * @return  A {@link FlowStatsHistory} instance.
     */
    public static FlowStatsHistory addPeriodic(
        FlowStatsHistory history, Long sysTime, GenericStatistics gstats) {
        // Expire old records.
        Long expire = Long.valueOf(sysTime.longValue() - HISTORY_LIFETIME);
        NavigableMap<Long, FlowStatsRecord> map =
            toNavigableMap(history).tailMap(expire, true);

        // Determine the latest record in the given history.
        GenericStatistics newStats = gstats;
        Map.Entry<Long, FlowStatsRecord> latest = map.lastEntry();
        if (latest != null) {
            FlowStatsRecord lfsr = latest.getValue();
            if (!Boolean.TRUE.equals(lfsr.isPeriodic())) {
                // Ensure that the given statistics is newer than the latest
                // history record.
                Duration nd = newStats.getDuration();
                Duration ld = lfsr.getDuration();
                if (FlowUtils.compare(nd, ld) < 0) {
                    // The latest record is newer than the given statistics.
                    // Use the latest record as a new periodic history record.
                    newStats = lfsr;
                    map.remove(latest.getKey());
                }
            }
        }

        // Return a new statistics history.
        FlowStatsRecord newRecord = new FlowStatsRecordBuilder(newStats).
            setTime(sysTime).setPeriodic(true).build();
        map.put(sysTime, newRecord);
        List<FlowStatsRecord> list = new ArrayList<>(map.values());
        return new FlowStatsHistoryBuilder().
            setFlowStatsRecord(list).build();
    }

    /**
     * Add a non-periodic flow statistics record to the statistics history.
     *
     * @param history    History of flow statistics.
     * @param newRecord  A flow statistics record to be added.
     *                   Note that periodic flag in this record must be false.
     * @return  A {@link FlowStatsHistory} instance if the statistics history
     *          needs to be updated. Otherwise {@code null}.
     */
    public static FlowStatsHistory addNonPeriodic(
        FlowStatsHistory history, FlowStatsRecord newRecord) {
        assert !Boolean.TRUE.equals(newRecord.isPeriodic());
        Long newTime = newRecord.getTime();
        NavigableMap<Long, FlowStatsRecord> map = toNavigableMap(history);

        // Determine the latest record in the given history.
        Map.Entry<Long, FlowStatsRecord> latest = map.lastEntry();
        if (latest != null) {
            // Don't add the given record if it is older than the latest
            // record.
            Long latestTime = latest.getKey();
            long ltime = latestTime.longValue();
            long time = newTime.longValue();
            if (time <= ltime) {
                return null;
            }

            if (time - ltime < MIN_INTERVAL) {
                // The interval between the latest record and the given
                // record is too short.
                FlowStatsRecord lfsr = latest.getValue();
                if (Boolean.TRUE.equals(lfsr.isPeriodic())) {
                    // Don't remove periodic record.
                    return null;
                }

                // Overwrite the latest record.
                map.remove(latestTime);
            }
        }

        map.put(newTime, newRecord);
        List<FlowStatsRecord> list = new ArrayList<>(map.values());
        return new FlowStatsHistoryBuilder().
            setFlowStatsRecord(list).build();
    }

    /**
     * Create a key string which identifies the transaction for getting flow
     * statistics from switch.
     *
     * @param nid  A MD-SAL node identifier which specifies the switch.
     * @param xid  A OpenFlow transaction ID.
     * @return  A key string which identifies the read transaction.
     */
    public static String createTransactionKey(String nid, BigInteger xid) {
        return new StringBuilder(xid.toString()).
            append('@').append(nid).toString();
    }
}
