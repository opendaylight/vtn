/*
 * Copyright (c) 2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.flow.add;

import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.TimeUnit;

import org.slf4j.Logger;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.TxContext;
import org.opendaylight.vtn.manager.internal.TxQueue;
import org.opendaylight.vtn.manager.internal.VTNConfig;
import org.opendaylight.vtn.manager.internal.VTNManagerProvider;
import org.opendaylight.vtn.manager.internal.util.MiscUtils;
import org.opendaylight.vtn.manager.internal.util.concurrent.VTNFuture;
import org.opendaylight.vtn.manager.internal.util.flow.AddFlowRpc;
import org.opendaylight.vtn.manager.internal.util.flow.FlowEntryDesc;
import org.opendaylight.vtn.manager.internal.util.flow.FlowUtils;
import org.opendaylight.vtn.manager.internal.util.flow.RemoveFlowRpc;
import org.opendaylight.vtn.manager.internal.util.flow.RemoveFlowRpcList;
import org.opendaylight.vtn.manager.internal.util.flow.VTNFlowBuilder;
import org.opendaylight.vtn.manager.internal.util.inventory.InventoryReader;
import org.opendaylight.vtn.manager.internal.util.inventory.SalNode;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcErrorCallback;

import org.opendaylight.controller.sal.utils.StatusCode;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.VtnFlowId;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.vtn.data.flow.fields.VtnFlowEntry;

import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.service.rev130819.AddFlowInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.service.rev130819.RemoveFlowInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.service.rev130819.RemoveFlowOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.service.rev130819.SalFlowService;

/**
 * {@code FlowAddTask} describes a task that installs the specified VTN data
 * flow.
 */
public final class FlowAddTask implements Runnable {
    /**
     * VTN Manager service provider.
     */
    private final VTNManagerProvider  vtnProvider;

    /**
     * The MD-SAL transaction queue used to update the MD-SAL datastore.
     */
    private final TxQueue  txQueue;

    /**
     * A {@link FlowAddContext} instance which contains a data flow to be
     * installed.
     */
    private final FlowAddContext  context;

    /**
     * The ingress flow entry.
     */
    private final VtnFlowEntry  ingressFlow;

    /**
     * A list of flow entries configured in the target data flow except for
     * the ingress flow.
     */
    private final List<VtnFlowEntry>  flowEntries;

    /**
     * Current VTN configuration.
     */
    private VTNConfig  vtnConfig;

    /**
     * Create a new task which adds VTN data flow.
     *
     * @param provider  VTN Manager provider service.
     * @param txq       A {@link TxQueue} instance used to update the MD-SAL
     *                  datastore.
     * @param ctx       A {@link FlowAddContext} instance.
     */
    public FlowAddTask(VTNManagerProvider provider, TxQueue txq,
                       FlowAddContext ctx) {
        vtnProvider = provider;
        txQueue = txq;
        context = ctx;

        VTNFlowBuilder builder = ctx.getFlowBuilder();
        VtnFlowEntry ingress = null;
        List<VtnFlowEntry> entries = builder.getDataFlow().getVtnFlowEntry();
        List<VtnFlowEntry> list = new ArrayList<>(entries.size() - 1);
        Integer order = Integer.valueOf(MiscUtils.ORDER_MIN);
        for (VtnFlowEntry vfent: entries) {
            if (order.equals(vfent.getOrder())) {
                ingress = vfent;
            } else {
                list.add(vfent);
            }
        }

        assert ingress != null;
        ingressFlow = ingress;
        flowEntries = list;
    }

    /**
     * Ensure that all the target switches are present.
     *
     * @param ctx      A read-only MD-SAL datastore transaction.
     * @param builder  A {@link VTNFlowBuilder} instance which contains
     *                 data flow to be installed.
     * @throws VTNException  The target switch is not present.
     */
    private void checkNodes(TxContext ctx, VTNFlowBuilder builder)
        throws VTNException {
        InventoryReader reader = ctx.getInventoryReader();
        for (SalNode snode: builder.getFlowNodes()) {
            if (reader.get(snode) == null) {
                String msg = "Target node is not present: " + snode;
                FlowAddContext.LOG.error(msg);
                throw new VTNException(StatusCode.NOTFOUND, msg);
            }
        }
    }

    /**
     * Install intermediate flows and egress flow.
     *
     * @param service  MD-SAL flow service.
     * @throws VTNException  Failed to install flow entries.
     */
    private void installFlows(SalFlowService service)
        throws VTNException {
        int size = flowEntries.size();
        if (size <= 0) {
            // The target data flow contains only one flow entry.
            return;
        }

        List<AddFlowRpc> rpcs = new ArrayList<>(size);
        for (VtnFlowEntry vfent: flowEntries) {
            AddFlowInput input = FlowUtils.createAddFlowInput(vfent);
            rpcs.add(new AddFlowRpc(service, input));
        }

        TimeUnit nano = TimeUnit.NANOSECONDS;
        TimeUnit milli = TimeUnit.MILLISECONDS;
        int msec = vtnConfig.getBulkFlowModTimeout();
        long timeout = milli.toNanos((long)msec);
        long deadline = System.nanoTime() + timeout;

        Logger logger = FlowAddContext.LOG;
        VTNException firstError = null;
        int index = 0;
        for (AddFlowRpc rpc: rpcs) {
            VtnFlowEntry vfent = flowEntries.get(index);
            try {
                rpc.getResult(timeout, nano, logger);
            } catch (VTNException e) {
                if (firstError == null) {
                    firstError = e;
                }
                continue;
            }
            traceLog(logger, vfent);

            timeout = deadline - System.nanoTime();
            if (timeout <= 0) {
                // Wait one more millisecond.
                timeout = milli.toNanos(1L);
            }

            index++;
        }

        if (firstError != null) {
            throw firstError;
        }
    }

    /**
     * Install the ingress flow entry and wait for the result.
     *
     * @param service  MD-SAL flow service.
     * @throws VTNException  Failed to install the ingress flow.
     */
    private void installIngressFlow(SalFlowService service)
        throws VTNException {
        AddFlowInput input = FlowUtils.createAddFlowInput(ingressFlow);
        AddFlowRpc rpc = new AddFlowRpc(service, input);
        long timeout = (long)vtnConfig.getFlowModTimeout();
        Logger logger = FlowAddContext.LOG;
        rpc.getResult(timeout, TimeUnit.MILLISECONDS, logger);
        traceLog(logger, ingressFlow);
    }

    /**
     * Record a trace log which indicates the given flow entry has been
     * installed successfully.
     *
     * @param logger  A {@link Logger} instance.
     * @param vfent   A {@link VtnFlowEntry} instance.
     */
    private void traceLog(Logger logger, VtnFlowEntry vfent) {
        if (logger.isTraceEnabled()) {
            logger.trace("Flow entry has been installed: {}",
                         new FlowEntryDesc(vfent));
        }
    }

    /**
     * Roll back the data flow.
     *
     * <p>
     *   This method invokes remove-flow RPC for all the flow entries
     *   configured in the target data flow, irrespective of whether they were
     *   successfully installed or not.
     * </p>
     *
     * @param ctx      A read-only MD-SAL datastore transaction.
     * @param service  MD-SAL flow service.
     * @param builder  A {@link VTNFlowBuilder} instance which contains
     *                 data flow to be installed.
     */
    private void rollback(TxContext ctx, SalFlowService service,
                          VTNFlowBuilder builder) {
        RemoveFlowRpcList rpcList = new RemoveFlowRpcList();
        List<VtnFlowEntry> vfents = new ArrayList<>();
        for (VtnFlowEntry vfent: builder.getDataFlow().getVtnFlowEntry()) {
            SalNode snode = SalNode.create(vfent.getNode());
            RemoveFlowInputBuilder rfib;
            InventoryReader reader = ctx.getInventoryReader();
            try {
                rfib = FlowUtils.
                    createRemoveFlowInputBuilder(snode, vfent, reader);
            } catch (VTNException e) {
                FlowAddContext.LOG.
                    warn("Failed to create remove-flow input.", e);
                continue;
            }

            if (rfib != null) {
                rpcList.add(snode, rfib);
                vfents.add(vfent);
            }
        }

        int idx = 0;
        for (RemoveFlowRpc rpc: rpcList.invoke(service)) {
            RpcErrorCallback<RemoveFlowOutput> cb = new RpcErrorCallback<>(
                FlowAddContext.LOG, "add-flow-rollback",
                "Failed to rollback flow entry: %s",
                new FlowEntryDesc(vfents.get(idx)));
            vtnProvider.setCallback(rpc.getFuture(), cb);
            idx++;
        }

        RollbackTxTask task = new RollbackTxTask(context);
        VTNFuture<Void> f = txQueue.post(task);
        if (f.isCancelled()) {
            // There is no way to rollback because the DS queue is already
            // closed.
            FlowAddContext.LOG.
                warn("{}: Flow DS queue is already closed: match={}",
                     builder.getDataFlow().getFlowId().getValue(),
                     builder.getIngressMatchKey());
        }
    }

    // Runnable

    /**
     * Install the specified VTN flow.
     */
    @Override
    public void run() {
        vtnConfig = vtnProvider.getVTNConfig();
        VTNFlowBuilder builder = context.getFlowBuilder();
        SalFlowService service = context.getFlowService();
        TxContext ctx = vtnProvider.newTxContext();
        try {
            checkNodes(ctx, builder);
            installFlows(service);
            installIngressFlow(service);
        } catch (VTNException | RuntimeException t) {
            context.setFailure(t);
            rollback(ctx, service, builder);
            return;
        } finally {
            ctx.cancelTransaction();
        }

        Logger logger = FlowAddContext.LOG;
        VtnFlowId flowId = builder.getDataFlow().getFlowId();
        if (logger.isDebugEnabled()) {
            logger.debug("Data flow has been installed: id={}",
                         flowId.getValue());
        }

        context.setResult(flowId);
    }
}
