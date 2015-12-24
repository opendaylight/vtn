/*
 * Copyright (c) 2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.flow.remove;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.concurrent.TimeUnit;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.VTNConfig;
import org.opendaylight.vtn.manager.internal.VTNManagerProvider;
import org.opendaylight.vtn.manager.internal.flow.stats.StatsReaderCallback;
import org.opendaylight.vtn.manager.internal.flow.stats.StatsReaderService;
import org.opendaylight.vtn.manager.internal.util.concurrent.SettableVTNFuture;
import org.opendaylight.vtn.manager.internal.util.concurrent.VTNFuture;
import org.opendaylight.vtn.manager.internal.util.flow.FlowUtils;
import org.opendaylight.vtn.manager.internal.util.flow.RemoveFlowRpc;
import org.opendaylight.vtn.manager.internal.util.inventory.SalNode;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.VtnFlowId;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.VtnOpenflowVersion;

import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.service.rev130819.FlowTableRef;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.service.rev130819.RemoveFlowInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.service.rev130819.RemoveFlowInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.service.rev130819.SalFlowService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.statistics.rev130819.FlowAndStatisticsMap;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.rev131026.FlowCookie;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.NodeId;

import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.inet.types.rev100924.Uri;

/**
 * {@code ClearNodeFlowsTask} describes a task which uninstalls VTN flow
 * entries present in the given switch.
 *
 * <p>
 *   This task will remove only flow entries installed by the VTN Manager, and
 *   will keep flow entries installed by another component.
 * </p>
 */
public final class ClearNodeFlowsTask
    implements Runnable, StatsReaderCallback {
    /**
     * Logger instance.
     */
    private static final Logger  LOG =
        LoggerFactory.getLogger(ClearNodeFlowsTask.class);

    /**
     * The timeout, in seconds, of flow statistics read transaction.
     */
    private static final long  STATS_READ_TIMEOUT = 20;

    /**
     * VTN Manager provider service.
     */
    private final VTNManagerProvider  vtnProvider;

    /**
     * The MD-SAL flow service.
     */
    private final SalFlowService  flowService;

    /**
     * The target switch.
     */
    private final SalNode  targetNode;

    /**
     * A future associated with this task.
     */
    private final SettableVTNFuture<Void>  taskFuture =
        new SettableVTNFuture<>();

    /**
     * A future used to wait for RPC invocations.
     */
    private final SettableVTNFuture<Void>  rpcFuture =
        new SettableVTNFuture<>();

    /**
     * A list of RPC invocations which uninstalls VTN flows.
     */
    private final List<RemoveFlowRpc>  rpcList;

    /**
     * An input builder for remove-flow RPC.
     */
    private RemoveFlowInputBuilder  inputBuilder;

    /**
     * Construct a new instance.
     *
     * @param provider  VTN Manager provider service.
     * @param sfs       The MD-SAL flow service.
     * @param srs       Flow statistics reader service.
     * @param snode     A {@link SalNode} instance which specifies the target
     *                  switch.
     * @param ofver     A {@link VtnOpenflowVersion} instance which indicates
     *                  the OpenFlow protocol version.
     */
    public ClearNodeFlowsTask(VTNManagerProvider provider, SalFlowService sfs,
                              StatsReaderService srs, SalNode snode,
                              VtnOpenflowVersion ofver) {
        vtnProvider = provider;
        flowService = sfs;
        targetNode = snode;

        if (ofver == VtnOpenflowVersion.OF13) {
            LOG.debug("Remove all VTN flows by cookie mask: {}", snode);
            RemoveFlowInput input = FlowUtils.
                createRemoveFlowInputBuilder(snode).
                setBarrier(true).
                build();
            rpcList = Collections.singletonList(new RemoveFlowRpc(sfs, input));
            rpcFuture.set(null);
        } else {
            // Read flow entries in the given switch.
            LOG.debug("Scanning flow table to remove VTN flows: {}", snode);
            rpcList = new ArrayList<>();
            if (!srs.start(snode, this)) {
                transactionCanceled();
            }
        }
    }

    /**
     * Return a future associated with this task.
     *
     * @return  A {@link VTNFuture} instance associated with this task.
     */
    public VTNFuture<Void> getFuture() {
        return taskFuture;
    }

    /**
     * Wait for all VTN flows in the target switch to be uninstalled.
     *
     * @throws VTNException  An error occurred.
     */
    private void uninstall() throws VTNException {
        // Wait for remove-flow request to be sent to the target switch.
        try {
            rpcFuture.checkedGet(STATS_READ_TIMEOUT, TimeUnit.SECONDS);
        } catch (VTNException e) {
            if (rpcFuture.cancel(false)) {
                // This should never happen because the flow statistics read
                // transaction will be timed out in 10 seconds.
                LOG.error("Failed to determine flow entries to be removed: " +
                          targetNode, e);
                throw e;
            }
        }

        // Wait for completion of RPC tasks.
        TimeUnit nano = TimeUnit.NANOSECONDS;
        TimeUnit milli = TimeUnit.MILLISECONDS;
        VTNConfig vcfg = vtnProvider.getVTNConfig();
        int msec = vcfg.getBulkFlowModTimeout();
        long timeout = milli.toNanos((long)msec);
        long deadline = System.nanoTime() + timeout;

        VTNException firstError = null;
        for (RemoveFlowRpc rpc: rpcList) {
            try {
                rpc.getResult(timeout, nano, LOG);
            } catch (VTNException e) {
                if (firstError == null) {
                    firstError = e;
                }
                continue;
            }
            LOG.trace("remove-flow has completed successfully: input={}",
                      rpc.getInput());

            timeout = deadline - System.nanoTime();
            if (timeout <= 0) {
                // Wait one more millisecond.
                timeout = milli.toNanos(1L);
            }
        }

        if (firstError == null) {
            LOG.debug("VTN flows has been cleared: {}", targetNode);
        } else {
            LOG.error("Failed to remove VTN flows in " + targetNode,
                      firstError);
            throw firstError;
        }
    }

    /**
     * Schedule a call of remove-flow RPC.
     *
     * @param builder  An input builder for remove-flow RPC.
     *                 {@code null} indicates that all the RPC calls have been
     *                 scheduled.
     */
    private void schedule(RemoveFlowInputBuilder builder) {
        RemoveFlowInputBuilder prev = inputBuilder;
        inputBuilder = builder;

        if (prev != null) {
            if (builder == null) {
                // Set barrier flag to the last input.
                prev.setBarrier(true);
            }

            rpcList.add(new RemoveFlowRpc(flowService, prev.build()));
        }
    }

    // Runnable

    /**
     * Wait for all VTN flows in the target switch to be uninstalled.
     */
    @Override
    public void run() {
        try {
            uninstall();
            taskFuture.set(null);
        } catch (VTNException e) {
            taskFuture.setException(e);
        } catch (RuntimeException e) {
            LOG.error("Caught an unexpected exception", e);
            taskFuture.setException(e);
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
        VtnFlowId fid = FlowUtils.getVtnFlowId(cookie);
        if (fid != null) {
            LOG.trace("Remove VTN flow entry: node={}, flow={}",
                      node.getValue(), fstats);
            Uri uri = new Uri("clear-node-flows:" + fid.getValue());
            RemoveFlowInputBuilder builder = FlowUtils.
                createRemoveFlowInputBuilder(targetNode, fstats, uri);
            schedule(builder);
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void transactionCompleted() {
        schedule(null);
        LOG.debug("All the flow entries have been scanned: {}", targetNode);
        rpcFuture.set(null);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void transactionCanceled() {
        LOG.warn("Failed to read VTN flows: {}", targetNode);

        if (!rpcFuture.isDone()) {
            LOG.warn("All flow entries in {} will be removed.", targetNode);

            Short tid = Short.valueOf(FlowUtils.TABLE_ID);
            FlowTableRef tref =
                new FlowTableRef(targetNode.getFlowTableIdentifier(tid));
            StringBuilder sb = new StringBuilder("clean-up:").
                append(targetNode);
            RemoveFlowInput input = new RemoveFlowInputBuilder().
                setNode(targetNode.getNodeRef()).
                setFlowTable(tref).
                setTransactionUri(new Uri(sb.toString())).
                setTableId(tid).
                setStrict(false).
                setBarrier(true).
                build();

            rpcList.add(new RemoveFlowRpc(flowService, input));
            rpcFuture.set(null);
        }
    }
}
