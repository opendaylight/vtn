/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.flow.stats;

import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ConcurrentMap;

import org.opendaylight.vtn.manager.internal.util.concurrent.VTNFuture;
import org.opendaylight.vtn.manager.internal.util.flow.FlowCache;
import org.opendaylight.vtn.manager.internal.util.inventory.SalNode;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.vtn.data.flow.fields.VtnFlowEntry;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.vtn.data.flow.fields.flow.stats.history.FlowStatsRecord;

/**
 * {@code FlowStatsReader} reads statistics information about the specified
 * flow entries from switches.
 */
public abstract class FlowStatsReader {
    /**
     * A threshold to switch method to read flow statistics.
     *
     * <p>
     *   If the estimated number of flow entries exceeds this value, this class
     *   will read statistics for all flow entries in switches by one RPC.
     *   Otherwise this class will read statistics for each flow entry one by
     *   one.
     * </p>
     */
    private static final int  COUNT_THRESHOLD = 10;

    /**
     * A flow statistics reader which never reads flow statistics.
     */
    private static final NullReader  NULL_READER = new NullReader();

    /**
     * Return a flow statistics reader.
     *
     * @param srs    The flow statistics reader service.
     * @param count  The estimated number of flow entries present in switches.
     * @return  A {@link FlowStatsReader} instance.
     */
    public static final FlowStatsReader getInstance(
        StatsReaderService srs, int count) {
        if (srs == null) {
            // No need to update flow statistics.
            return NULL_READER;
        }

        return (count > COUNT_THRESHOLD)
            ? new MultiReader(srs)
            : new SingleReader(srs);
    }

    /**
     * Construct a new instance.
     */
    private FlowStatsReader() {
    }

    /**
     * Return a future associated with the transaction for reading flow
     * statistics information.
     *
     * @param fc  The target flow entry.
     * @return  A {@link VTNFuture} instance which returns the flow statistics
     *          about the specified flow entry.
     */
    public abstract VTNFuture<FlowStatsRecord> get(FlowCache fc);

    /**
     * An implementation of {@link FlowStatsReader} which never reads flow
     * statistics.
     *
     * <p>
     *   This reader class is used when flow statistics does not need to be
     *   updated.
     * </p>
     */
    private static final class NullReader extends FlowStatsReader {
        // FlowStatsReader

        /**
         * {@code null} is always returned because this class never reads
         * flow statistics.
         *
         * @param fc  The target flow entry.
         * @return  {@code null}.
         */
        @Override
        public VTNFuture<FlowStatsRecord> get(FlowCache fc) {
            return null;
        }
    }

    /**
     * An implementation of {@link FlowStatsReader} which reads flow statistics
     * one by one.
     */
    private static final class SingleReader extends FlowStatsReader {
        /**
         * The flow statistics reader service.
         */
        private final StatsReaderService  statsReader;

        /**
         * Construct a new instance.
         *
         * @param srs  The flow statistics reader service.
         */
        private SingleReader(StatsReaderService srs) {
            statsReader = srs;
        }

        // FlowStatsReader

        /**
         * {@inheritDoc}
         */
        @Override
        public VTNFuture<FlowStatsRecord> get(FlowCache fc) {
            VtnFlowEntry ingress = fc.getIngressFlow();
            return (ingress == null) ? null : statsReader.get(ingress);
        }
    }

    /**
     * An implementation of {@link FlowStatsReader} which reads flow statistics
     * by reading all flow entries in the target switches.
     */
    private static final class MultiReader extends FlowStatsReader {
        /**
         * The flow statistics reader service.
         */
        private final StatsReaderService  statsReader;

        /**
         * A map which keeps statistics read transaction per switch.
         */
        private final ConcurrentMap<SalNode, NodeFlowStatsReader> txMap =
            new ConcurrentHashMap<>();

        /**
         * Construct a new instance.
         *
         * @param srs  The flow statistics reader service.
         */
        private MultiReader(StatsReaderService srs) {
            statsReader = srs;
        }

        /**
         * Return a flow statistics reader which read all flow statistics
         * present in the given switch.
         *
         * @param snode  A {@link SalNode} instance which specifies the switch.
         * @return  A {@link NodeFlowStatsReader} instance.
         */
        private NodeFlowStatsReader getNodeFlowStatsReader(SalNode snode) {
            NodeFlowStatsReader reader = txMap.get(snode);
            if (reader == null) {
                // Create a new flow statistics reader for the given node.
                reader = new NodeFlowStatsReader(snode);
                NodeFlowStatsReader old = txMap.putIfAbsent(snode, reader);
                if (old == null) {
                    // Start the read transaction.
                    reader.start(statsReader);
                } else {
                    // Lost the race.
                    reader = old;
                }
            }

            return reader;
        }

        /**
         * {@inheritDoc}
         */
        @Override
        public VTNFuture<FlowStatsRecord> get(FlowCache fc) {
            VtnFlowEntry ingress = fc.getIngressFlow();
            if (ingress == null) {
                return null;
            }

            SalNode snode = SalNode.create(ingress.getNode());
            if (snode == null) {
                // This should never happen.
                return null;
            }

            return getNodeFlowStatsReader(snode).get(ingress);
        }
    }
}
