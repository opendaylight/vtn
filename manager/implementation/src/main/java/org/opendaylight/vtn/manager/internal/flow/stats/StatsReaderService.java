/*
 * Copyright (c) 2015, 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.flow.stats;

import java.util.List;
import java.util.TimerTask;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import com.google.common.util.concurrent.FutureCallback;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.VTNManagerProvider;
import org.opendaylight.vtn.manager.internal.util.concurrent.SettableVTNFuture;
import org.opendaylight.vtn.manager.internal.util.concurrent.VTNFuture;
import org.opendaylight.vtn.manager.internal.util.flow.FlowEntryDesc;
import org.opendaylight.vtn.manager.internal.util.flow.FlowStatsUtils;
import org.opendaylight.vtn.manager.internal.util.flow.GetFlowStatsRpc;
import org.opendaylight.vtn.manager.internal.util.inventory.SalNode;
import org.opendaylight.vtn.manager.internal.util.log.VTNLogLevel;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcUtils;

import org.opendaylight.yangtools.yang.common.RpcResult;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.vtn.data.flow.fields.VtnFlowEntry;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.vtn.data.flow.fields.flow.stats.history.FlowStatsRecord;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.vtn.data.flow.fields.flow.stats.history.FlowStatsRecordBuilder;

import org.opendaylight.yang.gen.v1.urn.opendaylight.direct.statistics.rev160511.GetFlowStatisticsInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.direct.statistics.rev160511.GetFlowStatisticsOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.direct.statistics.rev160511.OpendaylightDirectStatisticsService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.statistics.rev130819.FlowAndStatisticsMap;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.statistics.rev130819.flow.and.statistics.map.list.FlowAndStatisticsMapList;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.rev131026.FlowCookie;
import org.opendaylight.yang.gen.v1.urn.opendaylight.model.statistics.types.rev130925.GenericStatistics;

/**
 * {@code StatsReaderService} provides interfaces to fetch flow statistics
 * information from OpenFlow switch.
 */
public final class StatsReaderService {
    /**
     * Logger instance.
     */
    private static final Logger  LOG =
        LoggerFactory.getLogger(StatsReaderService.class);

    /**
     * The timeout for transaction in milliseconds.
     */
    private static final long  TX_TIMEOUT = 10000L;

    /**
     * VTN Manager provider service.
     */
    private final VTNManagerProvider  vtnProvider;

    /**
     * MD-SAL direct statistics service.
     */
    private OpendaylightDirectStatisticsService  statsService;

    /**
     * {@code RpcTask} is a base class for tasks which issue a
     *  get-flow-statistics RPC request.
     */
    private abstract static class RpcTask extends TimerTask
        implements FutureCallback<RpcResult<GetFlowStatisticsOutput>> {
        /**
         * The invocation of get-flow-statistics RPC.
         */
        private final GetFlowStatsRpc  statsRpc;

        /**
         * Construt a new instance.
         *
         * @param provider  VTN Manager provider service.
         * @param dss       MD-SAL direct statistics service.
         * @param input     Input for get-flow-statistics RPC.
         */
        protected RpcTask(VTNManagerProvider provider,
                          OpendaylightDirectStatisticsService dss,
                          GetFlowStatisticsInput input) {
            statsRpc = new GetFlowStatsRpc(provider, dss, input);
        }

        /**
         * Start the task.
         *
         * @param provider  VTN Manager provider service.
         * @return  {@code true} on success. {@code false} on failure.
         */
        final boolean start(VTNManagerProvider provider) {
            // Configure task timeout.
            boolean ret = provider.getTimer().
                checkedSchedule(this, TX_TIMEOUT);
            if (ret) {
                // Set callback to the RPC future.
                provider.setCallback(statsRpc.getFuture(), this);
            }

            return ret;
        }

        /**
         * Return the callback associated with the transaction.
         *
         * @return  A {@link StatsReaderCallback} instance.
         */
        protected abstract StatsReaderCallback getCallback();

        /**
         * Return a brief description about the request.
         *
         * <p>
         *   This method is used only for debugging.
         * </p>
         *
         * @return  A brief description about the request.
         */
        protected abstract String getDescription();

        /**
         * Invoke the stats reader callback.
         *
         * @param output  Output of get-flow-statistics RPC.
         */
        private void invoke(GetFlowStatisticsOutput output) {
            StatsReaderCallback cb = getCallback();
            List<FlowAndStatisticsMapList> mapList =
                output.getFlowAndStatisticsMapList();
            if (mapList != null) {
                for (FlowAndStatisticsMapList map: mapList) {
                    String err = FlowStatsUtils.check(map);
                    if (err == null) {
                        cb.flowStatsReceived(map);
                    } else {
                        LOG.warn("Ignore broken flow statistics: {}: flow={}",
                                 err, map);
                    }
                }
            }
            cb.transactionCompleted();
        }

        // Runnable

        /**
         * Invoked when the RPC task has timed out.
         */
        @Override
        public final void run() {
            if (cancel() && statsRpc.getFuture().cancel(true)) {
                LOG.error("Flow stats transaction has timed out: input={}",
                          getDescription());
            }
        }

        // FutureCallback

        /**
         * Invoked when the get-flow-statistics RPC has completed successfully.
         *
         * @param result  The result of the RPC.
         */
        @Override
        public final void onSuccess(RpcResult<GetFlowStatisticsOutput> result) {
            if (cancel()) {
                try {
                    GetFlowStatisticsOutput output =
                        RpcUtils.checkResult(statsRpc, result, LOG);
                    invoke(output);
                } catch (VTNException | RuntimeException e) {
                    VTNLogLevel.ERROR.
                        log(LOG, e, "Flow stats RPC failed: input=%s",
                            getDescription());
                    getCallback().transactionCanceled();
                }
            }
        }

        /**
         * Invoked when the get-flow-statistics RPC task has failed.
         *
         * @param cause  A throwable that indicates the cause of failure.
         */
        @Override
        public final void onFailure(Throwable cause) {
            if (cancel()) {
                VTNLogLevel.ERROR.
                    log(LOG, cause, "Caught unhandled exception: input=%s",
                        getDescription());
                getCallback().transactionCanceled();
            }
        }
    }

    /**
     * {@code SingleFlowStatsTask} issues an RPC that obtains statistics
     * information about the given flow entry.
     */
    private static final class SingleFlowStatsTask extends RpcTask
        implements StatsReaderCallback {
        /**
         * The target flow entry.
         */
        private final VtnFlowEntry  targetFlow;

        /**
         * The flow cookie configured in the target VTN flow entry.
         */
        private final FlowCookie  targetCookie;

        /**
         * A future associated with the transaction for getting the statistics.
         */
        private final SettableVTNFuture<FlowStatsRecord>  future =
            new SettableVTNFuture<>();

        /**
         * Construct a new instance.
         *
         * @param provider  VTN Manager provider service.
         * @param dss       MD-SAL direct statistics service.
         * @param vfent     The target VTN flow entry.
         */
        private SingleFlowStatsTask(VTNManagerProvider provider,
                                    OpendaylightDirectStatisticsService dss,
                                    VtnFlowEntry vfent) {
            super(provider, dss,
                  FlowStatsUtils.createGetFlowStatsInput(vfent));
            targetFlow = vfent;
            targetCookie = vfent.getCookie();
        }

        /**
         * Return the future associated with the read transaction.
         *
         * @return  A {@link VTNFuture} instance.
         */
        private SettableVTNFuture<FlowStatsRecord> getFuture() {
            return future;
        }

        // RpcTask

        /**
         * {@inheritDoc}
         */
        @Override
        protected StatsReaderCallback getCallback() {
            return this;
        }

        /**
         * {@inheritDoc}
         */
        @Override
        protected String getDescription() {
            return new StringBuilder("SingleFlow[").
                append(new FlowEntryDesc(targetFlow, false)).append(']').
                toString();
        }

        // StatsReaderCallback

        /**
         * {@inheritDoc}
         */
        @Override
        public void flowStatsReceived(FlowAndStatisticsMap fstats) {
            if (targetCookie.equals(fstats.getCookie())) {
                Long time = System.currentTimeMillis();
                GenericStatistics gen = fstats;
                FlowStatsRecord fsr = new FlowStatsRecordBuilder(gen).
                    setTime(time).build();
                future.set(fsr);
            }
        }

        /**
         * {@inheritDoc}
         */
        @Override
        public void transactionCompleted() {
            // Set null as the result of the future.
            if (future.set(null) && LOG.isDebugEnabled()) {
                LOG.debug("Flow statistics not found: {}", getDescription());
            }
        }

        /**
         * {@inheritDoc}
         */
        @Override
        public void transactionCanceled() {
            future.cancel(false);
        }
    }

    /**
     * {@code AllFlowStatsTask} issues an RPC that obtains statistics
     * information about all the flow entries present in the given switch.
     */
    private final class AllFlowStatsTask extends RpcTask {
        /**
         * The target switch.
         */
        private final SalNode  targetNode;

        /**
         * The flow stats reader callback.
         */
        private final StatsReaderCallback  callback;

        /**
         * Construct a new instance.
         *
         * @param provider  VTN Manager provider service.
         * @param dss       MD-SAL direct statistics service.
         * @param snode     The target switch.
         * @param cb        The callback to be invoked when flow statistics are
         *                  received.
         */
        private AllFlowStatsTask(VTNManagerProvider provider,
                                 OpendaylightDirectStatisticsService dss,
                                 SalNode snode, StatsReaderCallback cb) {
            super(provider, dss, FlowStatsUtils.createGetFlowStatsInput(snode));
            targetNode = snode;
            callback = cb;
        }

        // RpcTask

        /**
         * {@inheritDoc}
         */
        @Override
        protected StatsReaderCallback getCallback() {
            return callback;
        }

        /**
         * {@inheritDoc}
         */
        @Override
        protected String getDescription() {
            return new StringBuilder("AllFlows[").
                append(targetNode).append(']').toString();
        }
    }

    /**
     * Construct a new instance.
     *
     * @param provider  A VTN Manager provider service.
     */
    public StatsReaderService(VTNManagerProvider provider) {
        vtnProvider = provider;

        try {
            // Get MD-SAL RPC service.
            statsService = provider.
                getRpcService(OpendaylightDirectStatisticsService.class);
        } catch (Exception e) {
            String msg = "Failed to initialize VTN flow statistics reader.";
            LOG.error(msg, e);
            throw new IllegalStateException(msg, e);
        }
    }

    /**
     * Get the statistics information about the given VTN flow entry.
     *
     * @param vfent  The target VTN flow entry.
     * @return  A {@link VTNFuture} instance associated with the stats read
     *          transaction.
     */
    public VTNFuture<FlowStatsRecord> get(VtnFlowEntry vfent) {
        SingleFlowStatsTask task =
            new SingleFlowStatsTask(vtnProvider, statsService, vfent);
        boolean succeeded = task.start(vtnProvider);
        VTNFuture<FlowStatsRecord> future = task.getFuture();
        if (!succeeded) {
            future.cancel(false);
        }

        return future;
    }

    /**
     * Start the flow stats reader transaction for getting statistics
     * information about all flow entries present in the given switch.
     *
     * @param snode  The target switch.
     * @param cb     The callback to be invoked when flow statistics are
     *               received.
     * @return  {@code true} on success. {@code false} on failure.
     */
    public boolean start(SalNode snode, StatsReaderCallback cb) {
        AllFlowStatsTask task =
            new AllFlowStatsTask(vtnProvider, statsService, snode, cb);
        return task.start(vtnProvider);
    }
}
