/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.flow.stats;

import java.math.BigInteger;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ConcurrentMap;
import java.util.concurrent.atomic.AtomicInteger;

import org.opendaylight.vtn.manager.internal.util.concurrent.SettableVTNFuture;
import org.opendaylight.vtn.manager.internal.util.concurrent.VTNFuture;
import org.opendaylight.vtn.manager.internal.util.inventory.SalNode;

import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.statistics.rev130819.FlowAndStatisticsMap;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.NodeId;
import org.opendaylight.yang.gen.v1.urn.opendaylight.model.statistics.types.rev130925.GenericStatistics;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.vtn.data.flow.fields.VtnFlowEntry;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.vtn.data.flow.fields.flow.stats.history.FlowStatsRecord;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.vtn.data.flow.fields.flow.stats.history.FlowStatsRecordBuilder;

import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.rev131026.FlowCookie;

/**
 * {@code NodeFlowStatsReader} reads statistics information about the specified
 * flow entries in the same switch.
 *
 * <p>
 *   Note that this method reads all flow statistics present in the specified
 *   switch.
 * </p>
 */
public final class NodeFlowStatsReader implements StatsReaderCallback {
    /**
     * Initial transaction state.
     */
    private static final int  STATE_INITIAL = 0;

    /**
     * Transaction state which indicates the transaction is being started.
     */
    private static final int  STATE_STARTING = 1;

    /**
     * Transaction state which indicates the transaction was started
     * successfully.
     */
    private static final int  STATE_STARTED = 2;

    /**
     * Transaction state which indicates the transaction completed.
     */
    private static final int  STATE_COMPLETED = 3;

    /**
     * Transaction state which indicates the transaction failed to start.
     */
    private static final int  STATE_FAILED = 4;

    /**
     * A map that keeps statistics read transactions and results.
     */
    private final ConcurrentMap<BigInteger, SettableVTNFuture<FlowStatsRecord>>  txMap =
        new ConcurrentHashMap<>();

    /**
     * The target switch.
     */
    private final SalNode  targetNode;

    /**
     * The statistics read transaction state.
     */
    private final AtomicInteger  txState = new AtomicInteger(STATE_INITIAL);

    /**
     * Construct a new instance.
     *
     * @param snode  A {@link SalNode} which specifies the target switch.
     */
    public NodeFlowStatsReader(SalNode snode) {
        targetNode = snode;
    }

    /**
     * Return a future associated with the task to read flow statistics
     * about the given VTN flow entry.
     *
     * @param vfent  The target flow entry.
     * @return  A {@link VTNFuture} instance which returns the flow statistics
     *          about the specified flow entry.
     */
    public VTNFuture<FlowStatsRecord> get(VtnFlowEntry vfent) {
        assert vfent.getNode().equals(targetNode.getNodeId());

        BigInteger cookie = vfent.getCookie().getValue();
        SettableVTNFuture<FlowStatsRecord> f = getFuture(cookie);
        if (txState.get() >= STATE_COMPLETED) {
            // No more statistics will be received.
            f.set(null);
        }

        return f;
    }

    /**
     * Start the flow statistics read transaction.
     *
     * <p>
     *   This method must be called only once.
     *   No flow statistics will be returned if this method is not called.
     * </p>
     *
     * @param srs  The flow statistics reader service.
     */
    public void start(StatsReaderService srs) {
        if (txState.compareAndSet(STATE_INITIAL, STATE_STARTING)) {
            boolean succeeded = false;
            try {
                if (srs.start(targetNode, this)) {
                    txState.compareAndSet(STATE_STARTING, STATE_STARTED);
                    succeeded = true;
                }
            } finally {
                if (!succeeded) {
                    txState.set(STATE_FAILED);
                    canceled();
                }
            }
        }
    }

    /**
     * Return a future which returns the flow statistics information about the
     * given VTN data flow.
     *
     * @param cookie  The flow cookie which specifies the target flow entry.
     * @return  A {@link SettableVTNFuture} instance.
     */
    private SettableVTNFuture<FlowStatsRecord> getFuture(BigInteger cookie) {
        SettableVTNFuture<FlowStatsRecord> f = txMap.get(cookie);
        if (f == null) {
            // Associate a new feature with the given VTN data flow.
            f = new SettableVTNFuture<>();
            SettableVTNFuture<FlowStatsRecord> old =
                txMap.putIfAbsent(cookie, f);
            if (old != null) {
                // Lost the race.
                f = old;
            }
        }

        return f;
    }

    /**
     * Invoked when the statistics read transaction has completed.
     */
    private void completed() {
        txState.set(STATE_COMPLETED);
        canceled();
    }

    /**
     * Make all requests for statistics fail.
     */
    private void canceled() {
        for (SettableVTNFuture<FlowStatsRecord> f: txMap.values()) {
            // This does nothing if this future has a result.
            f.set(null);
        }
    }

    // StatsReaderCallback

    /**
     * {@inheritDoc}
     */
    @Override
    public void flowStatsReceived(NodeId node, FlowAndStatisticsMap fstats) {
        assert node.equals(targetNode.getNodeId());

        FlowCookie cookie = fstats.getCookie();
        if (cookie != null) {
            Long time = System.currentTimeMillis();
            GenericStatistics gen = fstats;
            FlowStatsRecord fsr = new FlowStatsRecordBuilder(gen).
                setTime(time).build();
            getFuture(cookie.getValue()).set(fsr);
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void transactionCompleted() {
        completed();
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void transactionCanceled() {
        completed();
    }
}
