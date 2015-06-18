/*
 * Copyright (c) 2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.flow.reader;

import java.math.BigDecimal;
import java.util.Collections;
import java.util.List;
import java.util.Map;
import java.util.NavigableMap;
import java.util.TreeMap;
import java.util.concurrent.TimeUnit;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.L2Host;
import org.opendaylight.vtn.manager.internal.TxContext;
import org.opendaylight.vtn.manager.internal.TxQueue;
import org.opendaylight.vtn.manager.internal.cluster.MacVlan;
import org.opendaylight.vtn.manager.internal.flow.stats.AddFlowStatsTask;
import org.opendaylight.vtn.manager.internal.flow.stats.StatsReaderService;
import org.opendaylight.vtn.manager.internal.util.MiscUtils;
import org.opendaylight.vtn.manager.internal.util.concurrent.VTNFuture;
import org.opendaylight.vtn.manager.internal.util.flow.FlowCache;
import org.opendaylight.vtn.manager.internal.util.inventory.InventoryReader;
import org.opendaylight.vtn.manager.internal.util.inventory.SalNode;
import org.opendaylight.vtn.manager.internal.util.inventory.SalPort;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcOutputGenerator;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcUtils;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.DataFlowMode;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.GetDataFlowInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.GetDataFlowOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.GetDataFlowOutputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.get.data.flow.output.DataFlowInfo;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.get.data.flow.output.DataFlowInfoBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.vtn.data.flow.info.AveragedDataFlowStats;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.vtn.data.flow.info.AveragedDataFlowStatsBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.vtn.data.flow.info.DataFlowStatsBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.tenant.flow.info.VtnDataFlow;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.vtn.data.flow.fields.FlowStatsHistory;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.vtn.data.flow.fields.flow.stats.history.FlowStatsRecord;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VlanHost;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnSwitchPort;

import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.NodeId;
import org.opendaylight.yang.gen.v1.urn.opendaylight.model.statistics.types.rev130925.GenericStatistics;

/**
 * {@code ReadFlowFuture} describes a future associated with the task which
 * reads data flow information.
 */
public abstract class ReadFlowFuture
    extends AbstractReadFlowFuture<List<DataFlowInfo>>
    implements RpcOutputGenerator<List<DataFlowInfo>, GetDataFlowOutput> {
    /**
     * Default value for the averaged statistics interval.
     */
    private static final long  DEFAULT_AVERAGE_INTERVAL = 10L;

    /**
     * The number of milliseconds in a second.
     */
    private static final double  MILLISEC = 1000D;

    /**
     * The number of milliseconds to wait for completion of transaction for
     * getting flow statistics.
     */
    private static final long  STATS_READ_TIMEOUT = 10000L;

    /**
     * The MD-SAL transaction queue used to update the MD-SAL datastore.
     */
    private final TxQueue  txQueue;

    /**
     * Flow statistics reader service.
     */
    private final StatsReaderService  statsReader;

    /**
     * A {@link SalNode} instance which specifies the physical switch.
     */
    private SalNode  flowNode;

    /**
     * A {@link SalPort} instance which specifies the physical switch port.
     */
    private SalPort  flowPort;

    /**
     * A {@link MacVlan} instance whcih specifies the source L2 host.
     */
    private MacVlan  sourceHost;

    /**
     * Mode for retrieving data flow information.
     */
    private final DataFlowMode  flowMode;

    /**
     * The number of seconds in the measurement period for averaged flow
     * statistics.
     */
    private final long  averageInterval;

    /**
     * Construct a new instance.
     *
     * @param ctx    A {@link TxContext} instance.
     * @param txq    A {@link TxQueue} instance used to update the MD-SAL
     *               datastore.
     * @param srs    The flow statistics reader service.
     * @param input  Input of the RPC call.
     * @return  A {@link ReadFlowFuture} instance.
     * @throws VTNException  An error occurred.
     */
    public static final ReadFlowFuture create(TxContext ctx, TxQueue txq,
                                              StatsReaderService srs,
                                              GetDataFlowInput input)
        throws VTNException {
        if (input == null) {
            throw RpcUtils.getNullInputException();
        }

        return (input.getFlowId() == null)
            ? new ReadDataFlowFuture(ctx, txq, srs, input)
            : new ReadSingleFlowFuture(ctx, txq, srs, input);
    }

    /**
     * Construct a new instance.
     *
     * @param ctx    A {@link TxContext} instance.
     * @param txq    A {@link TxQueue} instance used to update the MD-SAL
     *               datastore.
     * @param srs    The flow statistics reader service.
     * @param input  Input of the RPC call.
     * @throws VTNException  An error occurred.
     */
    protected ReadFlowFuture(TxContext ctx, TxQueue txq,
                             StatsReaderService srs, GetDataFlowInput input)
        throws VTNException {
        super(ctx, input.getTenantName());
        txQueue = txq;

        DataFlowMode mode = input.getMode();
        if (mode == null) {
            // Defaults to SUMMARY mode.
            mode = DataFlowMode.SUMMARY;
        }
        flowMode = mode;

        statsReader = (mode == DataFlowMode.UPDATESTATS)
            ? srs : null;

        Integer interval = input.getAverageInterval();
        long ival = (interval == null) ? 0 : interval.longValue();
        if (ival <= 0) {
            ival = DEFAULT_AVERAGE_INTERVAL;
        }
        averageInterval = ival;

        setCondition(ctx, input);
    }

    /**
     * Return the flow statistics reader service.
     *
     * @return  A {@link StatsReaderService} instance if flow statistics
     *          information needs to be fetched from switches.
     *          Otherwise {@code null}.
     */
    protected final StatsReaderService getStatsReaderService() {
        return statsReader;
    }

    /**
     * Return a {@link SalNode} instance corresponding to the physical switch
     * specified by the RPC input.
     *
     * @return  A {@link SalNode} instance or {@code null}.
     */
    protected final SalNode getFlowNode() {
        return flowNode;
    }

    /**
     * Return a {@link SalPort} instance corresponding to the physical switch
     * port specified by the RPC input.
     *
     * @return  A {@link SalPort} instance or {@code null}.
     */
    protected final SalPort getFlowPort() {
        return flowPort;
    }

    /**
     * Return a {@link MacVlan} instance corresponding to the source L2 host
     * specified by the RPC input.
     *
     * @return  A {@link MacVlan} instance or {@code null}.
     */
    protected final MacVlan getSourceHost() {
        return sourceHost;
    }

    /**
     * Clear the physical switch specified by the RPC input.
     */
    protected final void clearFlowNode() {
        flowNode = null;
    }

    /**
     * Clear the physical switch port specified by the RPC input.
     */
    protected final void clearFlowPort() {
        flowPort = null;
    }

    /**
     * Clear the source L2 host specified by the RPC input.
     */
    protected final void clearSourceHost() {
        sourceHost = null;
    }

    /**
     * Set an empty flow list as the result.
     */
    protected final void notFound() {
        setResult(Collections.<DataFlowInfo>emptyList());
    }

    /**
     * Determine whether the given data flow should be selected or not.
     *
     * @param fc  A {@link FlowCache} instance.
     * @return  {@code true} if the given data flow should be selected.
     *          Otherwise {@code false}.
     */
    protected final boolean select(FlowCache fc) {
        // Check the source L2 host.
        if (sourceHost != null) {
            L2Host lh = fc.getSourceHost();
            if (lh == null || !sourceHost.equals(lh.getHost())) {
                return false;
            }
        }

        // Check the switch.
        if (flowNode != null && !fc.getFlowNodes().contains(flowNode)) {
            return false;
        }

        // Check the switch port.
        if (flowPort != null) {
            return fc.getFlowPorts().contains(flowPort);
        }

        return true;
    }

    /**
     * Convert the given data flow into a {@link DataFlowInfo} instance.
     *
     * <p>
     *   This method set an exception as the result of this future
     *   if {@code null} is returned.
     * </p>
     *
     * @param fc       A {@link FlowCache} instance.
     * @param current  A future which will return the current flow statistics
     *                 for the given data flow. If {@code null}, only flow
     *                 statistics cached in {@code fc} are used.
     * @return  A {@link DataFlowInfo} instance on success.
     *          {@code null} on failure.
     */
    protected final DataFlowInfo toDataFlowInfo(
        FlowCache fc, VTNFuture<FlowStatsRecord> current) {
        try {
            InventoryReader reader = getTxContext().getInventoryReader();
            DataFlowInfoBuilder builder =
                fc.toDataFlowInfoBuilder(reader, flowMode);
            if (flowMode != DataFlowMode.SUMMARY) {
                setStatistics(builder, fc, current);
            }

            return builder.build();
        } catch (VTNException | RuntimeException e) {
            Logger logger = LoggerFactory.getLogger(getClass());
            String msg = "Failed to convert data flow: " + fc.getDataFlow();
            logger.error(msg, e);
            return null;
        }
    }

    /**
     * Set up the search condition specified by the RPC input.
     *
     * @param ctx    A {@link TxContext} instance.
     * @param input  Input of the RPC call.
     * @throws VTNException  An error occurred.
     */
    private void setCondition(TxContext ctx, GetDataFlowInput input)
        throws VTNException {
        NodeId nid = input.getNode();
        if (nid != null) {
            // Verify the specified switch.
            SalNode snode = SalNode.create(nid);
            if (snode == null) {
                // Invalid node ID is specified.
                // Thus no data flow should be selected.
                notFound();
                return;
            }

            flowNode = snode;
            VtnSwitchPort vswp = input.getDataFlowPort();
            if (vswp != null) {
                // Determine the specified switch port.
                InventoryReader reader = ctx.getInventoryReader();
                SalPort sport = reader.findPort(snode, vswp);
                if (sport == null) {
                    // The specified port is not present.
                    // Thus no data flow should be selected.
                    notFound();
                    return;
                }

                flowPort = sport;

                // No need to check switch.
                flowNode = null;
            }
        }

        setSourceHostCondition(input);
    }

    /**
     * Set up the search condition for the source host.
     *
     * @param input  Input of the RPC call.
     */
    private void setSourceHostCondition(GetDataFlowInput input) {
        VlanHost vh = input.getDataFlowSource();
        if (vh != null) {
            // Try to convert VlanHost instance into MacVlan instance.
            MacVlan src;
            try {
                src = new MacVlan(vh);
            } catch (RpcException e) {
                src = null;
            }

            if (src == null || src.getMacAddress() == MacVlan.UNDEFINED) {
                // Invalid source host is specified.
                // Thus no data flow should be selected.
                notFound();
            } else {
                sourceHost = src;
            }
        }
    }

    /**
     * Return a list of flow statistics history records in the given VTN
     * data flow.
     *
     * @param vdf  The target VTN data flow.
     * @return  A list of {@link FlowStatsRecord} instance.
     */
    private List<FlowStatsRecord> getFlowStatsRecords(VtnDataFlow vdf) {
        FlowStatsHistory history = vdf.getFlowStatsHistory();
        if (history != null) {
            List<FlowStatsRecord> list = history.getFlowStatsRecord();
            if (list != null) {
                return list;
            }
        }

        // No statistics information is available.
        return Collections.<FlowStatsRecord>emptyList();
    }

    /**
     * Return the flow statistics record from the given future.
     *
     * @param vdf      The target VTN data flow.
     * @param current  A future which will return the current flow statistics
     *                 for the given data flow. Note that {@code null} is
     *                 returned if {@code null} is specified.
     * @return  A {@link FlowStatsRecord} instance returned by the given
     *          future. {@code null} if the flow statistics is not available.
     */
    private FlowStatsRecord getFlowStatsRecord(
        VtnDataFlow vdf, VTNFuture<FlowStatsRecord> current) {
        if (current != null) {
            try {
                return current.checkedGet(STATS_READ_TIMEOUT,
                                          TimeUnit.MILLISECONDS);
            } catch (VTNException | RuntimeException e) {
                Logger logger = LoggerFactory.getLogger(getClass());
                String msg = "Flow statistics is unavailable: flowId={}" +
                    vdf.getFlowId().getValue();
                logger.warn(msg, e);
            }
        }

        return null;
    }

    /**
     * Set the flow statistics information into the given data flow info
     * builder.
     *
     * @param builder  A {@link DataFlowInfoBuilder} instance.
     * @param fc       A {@link FlowCache} instance.
     * @param current  A future which will return the current flow statistics
     *                 for the given data flow. If {@code null}, only flow
     *                 statistics cached in {@code fc} are used.
     */
    private void setStatistics(DataFlowInfoBuilder builder, FlowCache fc,
                               VTNFuture<FlowStatsRecord> current) {
        // Sort history records in ascending order of the system time.
        VtnDataFlow vdf = fc.getDataFlow();
        List<FlowStatsRecord> list = getFlowStatsRecords(vdf);
        NavigableMap<Long, FlowStatsRecord> map = new TreeMap<>();
        for (FlowStatsRecord fsr: list) {
            map.put(fsr.getTime(), fsr);
        }

        // Add the current statistics if specified.
        FlowStatsRecord cfsr = getFlowStatsRecord(vdf, current);
        if (cfsr != null) {
            map.put(cfsr.getTime(), cfsr);
            AddFlowStatsTask task = new AddFlowStatsTask(
                getTenantName(), vdf.getFlowId(), cfsr);
            txQueue.post(task);
        }

        if (map.isEmpty()) {
            // No statistics information is available.
            return;
        }

        FlowStatsRecord latest = map.lastEntry().getValue();
        DataFlowStatsBuilder dfsb =
            new DataFlowStatsBuilder((GenericStatistics)latest);
        builder.setDataFlowStats(dfsb.build());

        if (map.size() > 1) {
            // Calcurate averaged flow statistics.
            long end = latest.getTime();
            FlowStatsRecord oldest = getStartRecord(map, end);
            if (oldest != null) {
                setAveragedStatistics(builder, oldest, latest, end);
            }
        }
    }

    /**
     * Set the averaged flow statistics information into the given data flow
     * info builder.
     *
     * @param builder  A {@link DataFlowInfoBuilder} instance.
     * @param oldest   A {@link FlowStatsRecord} instance that keeps the
     *                 oldest flow statistics.
     * @param latest   A {@link FlowStatsRecord} instance that keeps the
     *                 latest flow statistics.
     * @param end      The end time of the measurement period.
     */
    private void setAveragedStatistics(DataFlowInfoBuilder builder,
                                       FlowStatsRecord oldest,
                                       FlowStatsRecord latest, long end) {
        long start = oldest.getTime().longValue();
        double spackets = MiscUtils.doubleValue(oldest.getPacketCount());
        double sbytes = MiscUtils.doubleValue(oldest.getByteCount());
        double epackets = MiscUtils.doubleValue(latest.getPacketCount());
        double ebytes = MiscUtils.doubleValue(latest.getByteCount());
        double interval = (double)(end - start) / MILLISEC;

        double packets = (epackets - spackets) / interval;
        double bytes = (ebytes - sbytes) / interval;
        AveragedDataFlowStats average = new AveragedDataFlowStatsBuilder().
            setPacketCount(BigDecimal.valueOf(packets)).
            setByteCount(BigDecimal.valueOf(bytes)).
            setStartTime(start).
            setEndTime(end).
            build();
        builder.setAveragedDataFlowStats(average);
    }

    /**
     * Return the oldest statistics history record in the measurement period.
     *
     * @param history   A statistics history map.
     * @param end       The end time of the measurement period.
     * @return  A {@link FlowStatsRecord} instance if found.
     *          {@code null} if not found.
     */
    private FlowStatsRecord getStartRecord(
        NavigableMap<Long, FlowStatsRecord> history, long end) {
        long startTime = end - TimeUnit.SECONDS.toMillis(averageInterval);
        Long key = Long.valueOf(startTime);
        Map.Entry<Long, FlowStatsRecord> floorEntry = history.floorEntry(key);
        Map.Entry<Long, FlowStatsRecord> ceilingEntry =
            history.ceilingEntry(key);
        FlowStatsRecord start;
        if (floorEntry == null) {
            if (ceilingEntry == null) {
                return null;
            }
            start = ceilingEntry.getValue();
        } else if (ceilingEntry == null) {
            start = floorEntry.getValue();
        } else {
            // Choose nearer to the expected start time.
            FlowStatsRecord floor = floorEntry.getValue();
            FlowStatsRecord ceil = ceilingEntry.getValue();
            long fdiff = startTime - floor.getTime();
            long cdiff = ceil.getTime() - startTime;
            start = (fdiff < cdiff) ? floor : ceil;
        }

        return (start.getTime() >= end) ? null : start;
    }

    // RpcOutputGenerator

    /**
     * {@inheritDoc}
     */
    @Override
    public Class<GetDataFlowOutput> getOutputType() {
        return GetDataFlowOutput.class;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public GetDataFlowOutput createOutput(List<DataFlowInfo> result) {
        return new GetDataFlowOutputBuilder().setDataFlowInfo(result).build();
    }
}
