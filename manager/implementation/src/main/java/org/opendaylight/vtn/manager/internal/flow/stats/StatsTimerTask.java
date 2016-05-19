/*
 * Copyright (c) 2015, 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.flow.stats;

import java.math.BigInteger;
import java.util.List;
import java.util.Timer;
import java.util.TimerTask;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import com.google.common.base.Optional;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.TxContext;
import org.opendaylight.vtn.manager.internal.TxQueue;
import org.opendaylight.vtn.manager.internal.VTNManagerProvider;
import org.opendaylight.vtn.manager.internal.util.DataStoreUtils;
import org.opendaylight.vtn.manager.internal.util.flow.FlowCache;
import org.opendaylight.vtn.manager.internal.util.flow.FlowStatsUtils;
import org.opendaylight.vtn.manager.internal.util.flow.FlowUtils;
import org.opendaylight.vtn.manager.internal.util.log.VTNLogLevel;
import org.opendaylight.vtn.manager.internal.util.tx.AbstractTxTask;

import org.opendaylight.controller.md.sal.binding.api.ReadWriteTransaction;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.VtnFlows;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.tenant.flow.info.VtnDataFlow;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.tenant.flow.info.VtnDataFlowKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.vtn.data.flow.fields.FlowStatsHistory;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.vtn.data.flow.fields.VtnFlowEntry;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.vtn.flows.VtnFlowTable;

import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.inventory.rev130819.FlowId;
import org.opendaylight.yang.gen.v1.urn.opendaylight.model.statistics.types.rev130925.GenericStatistics;

/**
 * The timer task that collects flow statistics periodically.
 */
public final class StatsTimerTask extends TimerTask {
    /**
     * Logger instance.
     */
    private static final Logger  LOG =
        LoggerFactory.getLogger(StatsTimerTask.class);

    /**
     * The interval in milliseconds for updating flow statistics information.
     */
    private static final long  STATS_INTERVAL = 10000;

    /**
     * The MD-SAL transaction queue used to update the MD-SAL datastore.
     */
    private final TxQueue  txQueue;

    /**
     * {@code StatsUpdator} describes the MD-SAL datastore transaction task
     * that updates flow statistics information.
     */
    private static class StatsUpdator extends AbstractTxTask<Void> {
        /**
         * The system time when the flow statistics are collected.
         */
        private Long  systemTime;

        /**
         * Update flow statistics in the specified VTN.
         *
         * @param ctx    A MD-SAL transaction context.
         * @param tx     A {@link ReadWriteTransaction} instance.
         * @param table  A flow table that contains all the flow entries in
         *               the VTN.
         * @throws VTNException  An error occurred.
         */
        private void update(TxContext ctx, ReadWriteTransaction tx,
                            VtnFlowTable table)
            throws VTNException {
            List<VtnDataFlow> dataFlows = table.getVtnDataFlow();
            if (dataFlows != null) {
                String tname = table.getTenantName();
                for (VtnDataFlow vdf: dataFlows) {
                    update(ctx, tx, tname, vdf);
                }
            }
        }

        /**
         * Update statistics information for the given flow entry.
         *
         * @param ctx    A MD-SAL transaction context.
         * @param tx     A {@link ReadWriteTransaction} instance.
         * @param tname  The name of the VTN.
         * @param vdf    A {@link VtnDataFlow} instance.
         * @throws VTNException  An error occurred.
         */
        private void update(TxContext ctx, ReadWriteTransaction tx,
                            String tname, VtnDataFlow vdf) throws VTNException {
            VtnDataFlowKey key = vdf.getKey();
            BigInteger id = key.getFlowId().getValue();
            FlowId fid = FlowUtils.createMdFlowId(id);

            // Read flow statistics collected by the MD-SAL statistics manager.
            FlowCache fc = new FlowCache(vdf);
            VtnFlowEntry vfent = fc.getIngressFlow();
            GenericStatistics fstats =
                FlowStatsUtils.read(tx, vfent.getNode(), fid);
            String err = FlowStatsUtils.check(fstats);
            if (err != null) {
                ctx.log(LOG, VTNLogLevel.TRACE, "Skip flow entry: {}: {}",
                        id, err);
                return;
            }

            // Update the hisotry of flow statistics if needed.
            FlowStatsHistory history = FlowStatsUtils.addPeriodic(
                vdf.getFlowStatsHistory(), systemTime, fstats);
            InstanceIdentifier<FlowStatsHistory> path =
                FlowStatsUtils.getIdentifier(tname, key);
            LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
            tx.put(oper, path, history, false);
            ctx.log(LOG, VTNLogLevel.TRACE,
                    "Flow statistics has been updated: {}, {}",
                    id, fid.getValue());
        }

        // AbstractTxTask

        /**
         * {@inheritDoc}
         */
        @Override
        public Void execute(TxContext ctx) throws VTNException {
            systemTime = System.currentTimeMillis();

            // Read the root container of flow tables.
            InstanceIdentifier<VtnFlows> path =
                InstanceIdentifier.create(VtnFlows.class);
            LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
            ReadWriteTransaction tx = ctx.getReadWriteTransaction();
            Optional<VtnFlows> opt = DataStoreUtils.read(tx, oper, path);
            if (opt.isPresent()) {
                List<VtnFlowTable> tables = opt.get().getVtnFlowTable();
                if (tables != null) {
                    for (VtnFlowTable table: tables) {
                        // Update flow statistics for this VTN.
                        update(ctx, tx, table);
                    }
                }
            }

            return null;
        }

        // TxTask

        /**
         * Invoked when the task has completed successfully.
         *
         * @param provider  VTN Manager provider service.
         * @param result    The result of this task.
         */
        @Override
        public void onSuccess(VTNManagerProvider provider, Void result) {
            LOG.debug("Flow statistics have been updated successfully.");
        }

        /**
         * Invoked when the task has failed.
         *
         * @param provider  VTN Manager provider service.
         * @param t         A {@link Throwable} thrown by the task.
         */
        @Override
        public void onFailure(VTNManagerProvider provider, Throwable t) {
            LOG.error("Failed to update flow statistics.", t);
        }
    }

    /**
     * Construct a new instance.
     *
     * @param txq  A {@link TxQueue} instance used to update the MD-SAL
     *             datastore.
     */
    public StatsTimerTask(TxQueue txq) {
        txQueue = txq;
    }

    /**
     * Start the periodic timer task that updates the flow statistics.
     *
     * @param timer  A timer thread.
     */
    public void start(Timer timer) {
        timer.scheduleAtFixedRate(this, STATS_INTERVAL, STATS_INTERVAL);
    }

    // Runnable

    /**
     * Run the timer task to update flow statistics.
     */
    @Override
    public void run() {
        txQueue.post(new StatsUpdator());
    }
}
