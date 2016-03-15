/*
 * Copyright (c) 2015, 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.flow.remove;

import static org.opendaylight.vtn.manager.internal.util.flow.FlowUtils.EMPTY_MATCH;

import java.util.concurrent.TimeUnit;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.VTNConfig;
import org.opendaylight.vtn.manager.internal.VTNManagerProvider;
import org.opendaylight.vtn.manager.internal.flow.stats.StatsReaderCallback;
import org.opendaylight.vtn.manager.internal.flow.stats.StatsReaderService;
import org.opendaylight.vtn.manager.internal.util.concurrent.SettableVTNFuture;
import org.opendaylight.vtn.manager.internal.util.concurrent.RunnableVTNFuture;
import org.opendaylight.vtn.manager.internal.util.flow.FlowUtils;
import org.opendaylight.vtn.manager.internal.util.flow.RemoveFlowRpcList;
import org.opendaylight.vtn.manager.internal.util.inventory.SalNode;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.VtnFlowId;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.VtnOpenflowVersion;

import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.service.rev130819.FlowTableRef;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.service.rev130819.RemoveFlowInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.service.rev130819.SalFlowService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.statistics.rev130819.FlowAndStatisticsMap;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.rev131026.FlowCookie;

import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.inet.types.rev100924.Uri;

/**
 * {@code ClearNodeFlowsTask} describes a task which uninstalls VTN flow
 * entries present in the given switch.
 *
 * <p>
 *   This task will remove only flow entries installed by the VTN Manager, and
 *   will keep flow entries installed by another component.
 *   Note that this task never removes table miss flow entries from OF1.3
 *   switches.
 * </p>
 */
public final class ClearNodeFlowsTask extends RunnableVTNFuture<Void>
    implements StatsReaderCallback {
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
     * The target switch.
     */
    private final SalNode  targetNode;

    /**
     * A future used to wait for RPC invocations.
     */
    private final SettableVTNFuture<Void>  rpcFuture =
        new SettableVTNFuture<>();

    /**
     * A list of RPC invocations which uninstalls VTN flows.
     */
    private final RemoveFlowRpcList  rpcList;

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
        targetNode = snode;
        rpcList = new RemoveFlowRpcList(provider, sfs);

        if (ofver == VtnOpenflowVersion.OF10) {
            // Read flow entries in the given switch.
            LOG.debug("Scanning flow table to remove VTN flows: {}", snode);
            if (!srs.start(snode, this)) {
                transactionCanceled();
            }
        } else {
            LOG.debug("Remove all VTN flows by cookie mask: {}", snode);
            RemoveFlowInputBuilder builder = FlowUtils.
                createRemoveFlowInputBuilder(snode);
            rpcList.invoke(builder);
            completeRpc();
        }
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
        VTNConfig vcfg = vtnProvider.getVTNConfig();
        long timeout = (long)vcfg.getBulkFlowModTimeout();
        try {
            rpcList.verify(LOG, timeout, TimeUnit.MILLISECONDS);
            LOG.debug("VTN flows has been cleared: {}", targetNode);
        } catch (VTNException e) {
            LOG.error("Failed to remove VTN flows in " + targetNode, e);
        }
    }

    /**
     * Complete the task that invokes remove-flow RPC.
     */
    private void completeRpc() {
        rpcList.flush();
        rpcFuture.set(null);
    }

    // Runnable

    /**
     * Wait for all VTN flows in the target switch to be uninstalled.
     */
    @Override
    public void run() {
        try {
            uninstall();
            set(null);
        } catch (VTNException e) {
            setException(e);
        } catch (RuntimeException e) {
            LOG.error("Caught an unexpected exception", e);
            setException(e);
        }
    }

    // StatsReaderCallback

    /**
     * {@inheritDoc}
     */
    @Override
    public void flowStatsReceived(FlowAndStatisticsMap fstats) {
        FlowCookie cookie = fstats.getCookie();
        VtnFlowId fid = FlowUtils.getVtnFlowId(cookie);
        if (fid != null) {
            LOG.trace("Remove VTN flow entry: node={}, flow={}",
                      targetNode, fstats);
            Uri uri = new Uri("clear-node-flows:" + fid.getValue());
            RemoveFlowInputBuilder builder = FlowUtils.
                createRemoveFlowInputBuilder(targetNode, fstats, uri);
            rpcList.invoke(builder);
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void transactionCompleted() {
        LOG.debug("All the flow entries have been scanned: {}", targetNode);
        completeRpc();
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

            RemoveFlowInputBuilder builder = new RemoveFlowInputBuilder().
                setNode(targetNode.getNodeRef()).
                setFlowTable(tref).
                setTransactionUri(new Uri(sb.toString())).
                setTableId(tid).
                setMatch(EMPTY_MATCH).
                setStrict(false);

            rpcList.invoke(builder);
            completeRpc();
        }
    }
}
