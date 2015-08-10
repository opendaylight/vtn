/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.flow.stats;

import static org.opendaylight.vtn.manager.internal.util.MiscUtils.VERBOSE_LOG;
import static org.opendaylight.vtn.manager.internal.util.MiscUtils.checkNotNull;

import java.math.BigInteger;
import java.util.HashSet;
import java.util.List;
import java.util.Set;
import java.util.Timer;
import java.util.TimerTask;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ConcurrentMap;
import java.util.concurrent.TimeUnit;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.VTNManagerProvider;
import org.opendaylight.vtn.manager.internal.util.SalNotificationListener;
import org.opendaylight.vtn.manager.internal.util.concurrent.SettableVTNFuture;
import org.opendaylight.vtn.manager.internal.util.concurrent.VTNFuture;
import org.opendaylight.vtn.manager.internal.util.concurrent.VTNThreadPool;
import org.opendaylight.vtn.manager.internal.util.flow.FlowEntryDesc;
import org.opendaylight.vtn.manager.internal.util.flow.FlowStatsUtils;
import org.opendaylight.vtn.manager.internal.util.flow.GetFlowStatsRpc;
import org.opendaylight.vtn.manager.internal.util.inventory.SalNode;

import org.opendaylight.controller.sal.binding.api.NotificationService;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.vtn.data.flow.fields.VtnFlowEntry;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.vtn.data.flow.fields.flow.stats.history.FlowStatsRecord;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.vtn.data.flow.fields.flow.stats.history.FlowStatsRecordBuilder;

import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.statistics.rev130819.AggregateFlowStatisticsUpdate;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.statistics.rev130819.FlowAndStatisticsMap;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.statistics.rev130819.FlowsStatisticsUpdate;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.statistics.rev130819.GetFlowStatisticsFromFlowTableInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.statistics.rev130819.OpendaylightFlowStatisticsListener;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.statistics.rev130819.OpendaylightFlowStatisticsService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.statistics.rev130819.flow.and.statistics.map.list.FlowAndStatisticsMapList;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.transaction.rev150304.TransactionId;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.rev131026.FlowCookie;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.NodeId;
import org.opendaylight.yang.gen.v1.urn.opendaylight.model.statistics.types.rev130925.GenericStatistics;

/**
 * {@code StatsReaderService} provides interfaces to fetch flow statistics
 * information from OpenFlow switch.
 */
public final class StatsReaderService extends SalNotificationListener
    implements OpendaylightFlowStatisticsListener {
    /**
     * Logger instance.
     */
    private static final Logger  LOG =
        LoggerFactory.getLogger(StatsReaderService.class);

    /**
     * The maximum number of milliseconds to wait for completion of RPC.
     */
    private static final long  RPC_TIMEOUT = 5000L;

    /**
     * The timeout for transaction in milliseconds.
     */
    private static final long  TX_TIMEOUT = 10000L;

    /**
     * MD-SAL flow statistics service.
     */
    private OpendaylightFlowStatisticsService  statsService;

    /**
     * A timer thread.
     */
    private final Timer  timer;

    /**
     * Active transactions.
     */
    private final ConcurrentMap<String, ReaderContext>  txMap =
        new ConcurrentHashMap<>();

    /**
     * A thread to execute transaction tasks.
     */
    private final VTNThreadPool  taskThread;

    /**
     * {@code ReaderContext} describes a transaction associated with a request
     * for flow statistics.
     */
    private final class ReaderContext extends TimerTask {
        /**
         * The identifier associated with this transaction.
         */
        private final String  transactionKey;

        /**
         * The callback to be invoked when flow statistics are received.
         */
        private final StatsReaderCallback  callback;

        /**
         * Construct a new instance.
         *
         * @param xkey  The key string which identifies the transaction.
         * @param cb    The callback to be invoked when flow statistics are
         *              received.
         */
        private ReaderContext(String xkey, StatsReaderCallback cb) {
            transactionKey = xkey;
            callback = cb;
        }

        /**
         * Return the transaction key.
         *
         * @return  The transaction key.
         */
        private String getTransactionKey() {
            return transactionKey;
        }

        /**
         * Return the callback.
         *
         * @return  A {@link StatsReaderCallback} instance.
         */
        private StatsReaderCallback getCallback() {
            return callback;
        }

        // Runnable

        /**
         * Post a task which expires this transaction.
         */
        @Override
        public void run() {
            if (!taskThread.executeTask(new ExpireTask(this))) {
                expire(this);
            }
        }
    }

    /**
     * {@code ExpireTask} expires the transaction.
     */
    private final class ExpireTask implements Runnable {
        /**
         * The target reader context.
         */
        private final ReaderContext  context;

        /**
         * Construct a new instance.
         *
         * @param ctx  The target reader context.
         */
        private ExpireTask(ReaderContext ctx) {
            context = ctx;
        }

        // Runnable

        /**
         * Expire the target transaction.
         */
        @Override
        public void run() {
            expire(context);
        }
    }

    /**
     * {@code RpcTask} is a base class for tasks which issue an RPC request.
     */
    private abstract class RpcTask implements Runnable {
        /**
         * Return a MD-SAL node identifier which indicates the target switch.
         *
         * @return  A MD-SAL node identifier.
         */
        protected abstract String getTargetNodeId();

        /**
         * Return the callback associated with the transaction.
         *
         * @return  A {@link StatsReaderCallback} instance.
         */
        protected abstract StatsReaderCallback getCallback();

        /**
         * Return an RPC input for getting flow statistics.
         *
         * @return  A {@link GetFlowStatisticsFromFlowTableInput} instance.
         */
        protected abstract GetFlowStatisticsFromFlowTableInput getInput();

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

        // Runnable

        /**
         * Issue an RPC request for getting flow statistics.
         */
        @Override
        public final void run() {
            String nid = getTargetNodeId();
            GetFlowStatisticsFromFlowTableInput input = getInput();
            GetFlowStatsRpc rpc = new GetFlowStatsRpc(statsService, input);
            StatsReaderCallback cb = getCallback();
            String xkey = startTransaction(nid, rpc, cb);
            if (xkey == null) {
                cb.transactionCanceled();
            } else if (LOG.isDebugEnabled()) {
                LOG.debug("Read transaction has started: xid={}, input={}",
                          xkey, getDescription());
            }
        }
    }

    /**
     * {@code SingleFlowStatsTask} issues an RPC request which requests
     * statistics information about the given flow entry.
     */
    private final class SingleFlowStatsTask extends RpcTask
        implements StatsReaderCallback {
        /**
         * The target flow entry.
         */
        private final VtnFlowEntry  targetFlow;

        /**
         * The node identifier which specifies the target node.
         */
        private final String  targetNodeId;

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
         * @param vfent  The target VTN flow entry.
         */
        private SingleFlowStatsTask(VtnFlowEntry vfent) {
            NodeId nid = checkNotNull(vfent.getNode(), LOG,
                                      "No node in VTN flow entry.");
            targetFlow = vfent;
            targetNodeId = nid.getValue();
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
        protected String getTargetNodeId() {
            return targetNodeId;
        }

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
        protected GetFlowStatisticsFromFlowTableInput getInput() {
            return FlowStatsUtils.createGetFlowStatsInput(targetFlow);
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
        public void flowStatsReceived(NodeId node,
                                      FlowAndStatisticsMap fstats) {
            if (targetNodeId.equals(node.getValue()) &&
                targetCookie.equals(fstats.getCookie())) {
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
     * {@code AllFlowStatsTask} issues an RPC request which requests statistics
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
         * @param snode  The target switch.
         * @param cb     The callback to be invoked when flow statistics are
         *               received.
         */
        private AllFlowStatsTask(SalNode snode, StatsReaderCallback cb) {
            targetNode =
                checkNotNull(snode, LOG, "Target node cannot be null.");
            callback = cb;
        }

        // RpcTask

        /**
         * {@inheritDoc}
         */
        @Override
        protected String getTargetNodeId() {
            return targetNode.toString();
        }

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
        protected GetFlowStatisticsFromFlowTableInput getInput() {
            return FlowStatsUtils.createGetFlowStatsInput(targetNode);
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
     * {@code NotificationTask} notifies the flow-statistics-update
     * notification to callback associated with the notification.
     */
    private final class NotificationTask implements Runnable {
        /**
         * The key string which specifies the tranaction.
         */
        private final String  transactionKey;

        /**
         * A flow-statistics-update notification.
         */
        private final FlowsStatisticsUpdate  notification;

        /**
         * Construct a new instance.
         *
         * @param xkey    The key string which identifies the transaction.
         * @param update  A flow-statistics-update notification.
         */
        private NotificationTask(String xkey, FlowsStatisticsUpdate update) {
            transactionKey = xkey;
            notification = update;
        }

        // Runnable

        /**
         * Notifiy the flow statistics to the callback associated with the
         * transaction.
         */
        @Override
        public void run() {
            invoke(transactionKey, notification);
        }
    }

    /**
     * Construct a new instance.
     *
     * @param provider  A VTN Manager provider service.
     * @param nsv       A {@link NotificationService} service instance.
     */
    public StatsReaderService(VTNManagerProvider provider,
                              NotificationService nsv) {
        timer = provider.getTimer();
        taskThread = new VTNThreadPool("StatsReader Task Thread");
        addCloseable(taskThread);

        try {
            // Get MD-SAL RPC services.
            statsService = provider.
                getRpcService(OpendaylightFlowStatisticsService.class);

            // Register MD-SAL notification listener.
            registerListener(nsv);
        } catch (Exception e) {
            String msg = "Failed to initialize VTN flow statistics reader.";
            LOG.error(msg, e);
            close();
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
        SingleFlowStatsTask task = new SingleFlowStatsTask(vfent);
        SettableVTNFuture<FlowStatsRecord> f = task.getFuture();
        if (!taskThread.executeTask(task)) {
            f.set(null);
        }

        return f;
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
        return taskThread.executeTask(new AllFlowStatsTask(snode, cb));
    }

    /**
     * Shut down the flow statistics reader service.
     */
    public void shutdown() {
        taskThread.shutdown();
    }

    /**
     * Wait for completion of the given RPC, and return the transaction ID
     * return by the RPC.
     *
     * @param nid  A MD-SAL node identifier which specifies the target switch.
     * @param rpc  A {@link GetFlowStatsRpc} instance.
     * @param cb   A callback associated with the given RPC.
     * @return  The key string associated with the read transaction on success.
     *          {@code null} on failure.
     */
    private String startTransaction(String nid, GetFlowStatsRpc rpc,
                                    StatsReaderCallback cb) {
        try {
            BigInteger xid = rpc.getTransactionId(
                RPC_TIMEOUT, TimeUnit.MILLISECONDS, LOG);
            String xkey = FlowStatsUtils.createTransactionKey(nid, xid);
            ReaderContext ctx = new ReaderContext(xkey, cb);
            if (txMap.putIfAbsent(xkey, ctx) == null) {
                // Register expiration timer.
                timer.schedule(ctx, TX_TIMEOUT);
                return xkey;
            }
            ctx.cancel();
            LOG.error("Duplicated XID: {}", xkey);
        } catch (VTNException | RuntimeException e) {
            LOG.error("Failed to issue flow stats RPC: " + rpc.getName(), e);
        }

        return null;
    }

    /**
     * Invoke the callback associated with the given transaction.
     *
     * @param xkey    The key string which identifies the transaction.
     * @param update  A flow-statistics-update notification.
     */
    private void invoke(String xkey, FlowsStatisticsUpdate update) {
        ReaderContext ctx = txMap.get(xkey);
        if (ctx == null) {
            VERBOSE_LOG.trace("Ignore unwanted notification: xid={}", xkey);
            return;
        }

        StatsReaderCallback cb = ctx.getCallback();
        List<FlowAndStatisticsMapList> mapList =
            update.getFlowAndStatisticsMapList();
        if (mapList != null) {
            NodeId node = update.getId();
            for (FlowAndStatisticsMapList map: mapList) {
                String err = FlowStatsUtils.check(map);
                if (err == null) {
                    cb.flowStatsReceived(node, map);
                } else {
                    LOG.warn("Ignore broken flow statistics: {}: xid={}, " +
                             "flow={}", err, xkey, map);
                }
            }
        }

        if (!Boolean.TRUE.equals(update.isMoreReplies()) &&
            txMap.remove(xkey, ctx)) {
            // All statistics has been received.
            ctx.cancel();
            LOG.debug("Read transaction has completed: xid={}", xkey);
            cb.transactionCompleted();
        }
    }

    /**
     * Expire the given transaction.
     *
     * @param ctx  The target reader context.
     */
    private void expire(ReaderContext ctx) {
        String xkey = ctx.getTransactionKey();
        if (txMap.remove(xkey, ctx)) {
            LOG.error("Transaction has expired: xid={}", xkey);
            ctx.getCallback().transactionCanceled();
        }
    }

    /**
     * Record a broken notification as a warning message.
     * ignored.
     *
     * @param notification  A {@link FlowsStatisticsUpdate} instance.
     * @param msg           A warning message.
     */
    private void broken(FlowsStatisticsUpdate notification, String msg) {
        LOG.warn("Ignore broken flow-statistics-update notification: {}: {}",
                 msg, notification);
    }

    // AutoCloseable

    /**
     * Close the flow statistics reader service.
     */
    @Override
    public void close() {
        shutdown();

        // Cancel all transactions.
        while (!txMap.isEmpty()) {
            Set<String> xkeys = new HashSet<>(txMap.keySet());
            for (String xkey: xkeys) {
                ReaderContext ctx = txMap.remove(xkey);
                if (ctx != null) {
                    ctx.cancel();
                    ctx.getCallback().transactionCanceled();
                }
            }
        }

        super.close();
    }

    // CloseableContainer

    /**
     * {@inheritDoc}
     */
    @Override
    protected Logger getLogger() {
        return LOG;
    }

    // OpendaylightFlowStatisticsListener

    /**
     * Invoked when an aggregated flow statistics for a flow table has been
     * send by a switch.
     *
     * @param notification  An {@link AggregateFlowStatisticsUpdate} instance.
     */
    @Override
    public void onAggregateFlowStatisticsUpdate(
        AggregateFlowStatisticsUpdate notification) {
        // Nothing to do.
    }

    /**
     * Invoked when a flow statistics has been send by a switch.
     *
     * @param notification  A {@link FlowsStatisticsUpdate} instance.
     */
    @Override
    public void onFlowsStatisticsUpdate(FlowsStatisticsUpdate notification) {
        if (notification == null) {
            LOG.warn("Null flow-statistics-update notification.");
            return;
        }

        NodeId node = notification.getId();
        if (node == null) {
            broken(notification, "No node-id");
            return;
        }

        TransactionId txId = notification.getTransactionId();
        if (txId == null) {
            broken(notification, "No XID");
            return;
        }

        String xkey = FlowStatsUtils.createTransactionKey(
            node.getValue(), txId.getValue());
        taskThread.executeTask(new NotificationTask(xkey, notification));
    }
}
