/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.flow.add;

import java.math.BigInteger;
import java.util.concurrent.CancellationException;

import com.google.common.base.Optional;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.TxContext;
import org.opendaylight.vtn.manager.internal.TxQueue;
import org.opendaylight.vtn.manager.internal.VTNManagerProvider;
import org.opendaylight.vtn.manager.internal.util.DataStoreUtils;
import org.opendaylight.vtn.manager.internal.util.concurrent.VTNThreadPool;
import org.opendaylight.vtn.manager.internal.util.flow.FlowUtils;
import org.opendaylight.vtn.manager.internal.util.flow.VTNFlowBuilder;
import org.opendaylight.vtn.manager.internal.util.inventory.SalNode;
import org.opendaylight.vtn.manager.internal.util.inventory.SalPort;
import org.opendaylight.vtn.manager.internal.util.tx.AbstractTxTask;
import org.opendaylight.vtn.manager.internal.util.vnode.VTenantUtils;

import org.opendaylight.controller.md.sal.binding.api.ReadWriteTransaction;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.VtnFlowId;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.NextFlowId;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.NextFlowIdBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.flow.id.set.FlowIdList;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.flow.id.set.FlowIdListBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.flow.id.set.FlowIdListKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.tenant.flow.info.MatchFlows;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.tenant.flow.info.MatchFlowsBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.tenant.flow.info.MatchFlowsKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.tenant.flow.info.NodeFlows;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.tenant.flow.info.NodeFlowsKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.tenant.flow.info.PortFlows;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.tenant.flow.info.PortFlowsKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.tenant.flow.info.SourceHostFlows;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.tenant.flow.info.SourceHostFlowsKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.tenant.flow.info.VtnDataFlow;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.vtn.flows.VtnFlowTable;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeName;

/**
 * {@code PutFlowTxTask} describes the MD-SAL datastore transaction task that
 * installs a new VTN flow information into the datastore.
 *
 * <p>
 *   This task returns a {@link VtnDataFlow} instance corresponding to a new
 *   VTN data flow.
 * </p>
 */
public final class PutFlowTxTask extends AbstractTxTask<VtnDataFlow> {
    /**
     * The number of times for retrying transaction.
     */
    private static final int  MAX_FLOW_RETRY = 10;

    /**
     * A thread on which flow entries are installed.
     */
    private final VTNThreadPool  flowThread;

    /**
     * A {@link FlowAddContext} instance which contains a data flow to be
     * installed.
     */
    private final FlowAddContext  context;

    /**
     * The MD-SAL transaction queue that runs this task.
     */
    private final TxQueue  txQueue;

    /**
     * Construct a new instance.
     *
     * @param ctx     A {@link FlowAddContext} instance.
     * @param thread  A {@link VTNThreadPool} instance used to install flow
     *                entries.
     * @param txq     A {@link TxQueue} instance that runs this task.
     */
    public PutFlowTxTask(FlowAddContext ctx, VTNThreadPool thread,
                         TxQueue txq) {
        context = ctx;
        flowThread = thread;
        txQueue = txq;
    }

    // AbstractTxTask

    /**
     * Return the number of times for retrying transaction.
     *
     * <p>
     *   This method in this class returns {@link #MAX_FLOW_RETRY}.
     * </p>
     *
     * @return  The number of times for retrying transaction.
     */
    @Override
    protected int getMaxRetry() {
        return MAX_FLOW_RETRY;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public VtnDataFlow execute(TxContext ctx) throws VTNException {
        // Ensure that the target VTN is present.
        ReadWriteTransaction tx = ctx.getReadWriteTransaction();
        VTNFlowBuilder builder = context.getFlowBuilder();
        String tname = builder.getTenantName();
        VnodeName vname = new VnodeName(tname);
        VTenantUtils.readVtn(tx, vname);

        // Check to see if the data flow is already installed.
        String condKey = builder.getIngressMatchKey();
        InstanceIdentifier<VtnFlowTable> tpath =
            FlowUtils.getIdentifier(tname);
        MatchFlowsKey mkey = new MatchFlowsKey(condKey);
        InstanceIdentifier<MatchFlows> mpath =
            tpath.child(MatchFlows.class, mkey);
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        Optional<MatchFlows> mopt = DataStoreUtils.read(tx, oper, mpath);
        if (mopt.isPresent()) {
            // This data flow is already installed.
            return null;
        }

        // Determine VTN flow ID.
        InstanceIdentifier<NextFlowId> ipath =
            InstanceIdentifier.create(NextFlowId.class);
        Optional<NextFlowId> iopt = DataStoreUtils.read(tx, oper, ipath);
        VtnFlowId flowId = null;
        if (iopt.isPresent()) {
            flowId = iopt.get().getNextId();
        }
        if (flowId == null) {
            // This should never happen.
            FlowAddContext.LOG.warn("Use initial flow ID.");
            flowId = FlowUtils.getInitialFlowId();
        }

        // Update the flow ID for the next allocation.
        BigInteger bi = flowId.getValue();
        VtnFlowId nextId = new VtnFlowId(bi.add(BigInteger.ONE));
        NextFlowId nfid = new NextFlowIdBuilder().setNextId(nextId).build();
        tx.put(oper, ipath, nfid, true);

        // Update the match index.
        MatchFlows mindex = new MatchFlowsBuilder().
            setKey(mkey).setFlowId(flowId).build();
        tx.put(oper, mpath, mindex, true);

        // Update the node index.
        FlowIdListKey fidKey = new FlowIdListKey(flowId);
        FlowIdList fid = new FlowIdListBuilder().setKey(fidKey).build();
        for (SalNode snode: builder.getFlowNodes()) {
            InstanceIdentifier<FlowIdList> path = tpath.builder().
                child(NodeFlows.class, new NodeFlowsKey(snode.getNodeId())).
                child(FlowIdList.class, fidKey).
                build();
            tx.merge(oper, path, fid, true);
        }

        // Update the port index.
        for (SalPort sport: builder.getFlowPorts()) {
            InstanceIdentifier<FlowIdList> path = tpath.builder().
                child(PortFlows.class,
                      new PortFlowsKey(sport.getNodeConnectorId())).
                child(FlowIdList.class, fidKey).
                build();
            tx.merge(oper, path, fid, true);
        }

        // Update the source host index.
        SourceHostFlowsKey srcHost = builder.getSourceHostFlowsKey();
        if (srcHost != null) {
            InstanceIdentifier<FlowIdList> path = tpath.builder().
                child(SourceHostFlows.class, srcHost).
                child(FlowIdList.class, fidKey).
                build();
            tx.merge(oper, path, fid, true);
        }

        // Install the VTN data flow into the DS.
        VtnDataFlow vdf = builder.createVtnDataFlow(flowId);
        InstanceIdentifier<VtnDataFlow> path =
            FlowUtils.getIdentifier(tname, flowId);
        tx.put(oper, path, vdf, true);

        return vdf;
    }

    // TxTask

    /**
     * Invoked when the task has been completed successfully.
     *
     * @param provider  VTN Manager provider service.
     * @param result    The result of this task.
     */
    @Override
    public void onSuccess(VTNManagerProvider provider, VtnDataFlow result) {
        VTNFlowBuilder builder = context.getFlowBuilder();
        if (result == null) {
            // No flow entry was installed.
            FlowAddContext.LOG.trace("Already installed: match={}",
                                     builder.getIngressMatchKey());
            context.setResult(null);
        } else {
            // Install flow entries.
            FlowAddTask task = new FlowAddTask(provider, txQueue, context);
            if (!flowThread.executeTask(task)) {
                // In this case DS queue is also closed.
                // So there is no way to clean up the DS.
                String msg = "Flow thread is already closed";
                FlowAddContext.LOG.
                    warn("{}: match={}", msg, builder.getIngressMatchKey());
                context.setFailure(new CancellationException(msg));
            }
        }
    }

    /**
     * Invoked when the task has failed.
     *
     * @param provider  VTN Manager provider service.
     * @param t         A {@link Throwable} thrown by the task.
     */
    @Override
    public void onFailure(VTNManagerProvider provider, Throwable t) {
        VTNFlowBuilder builder = context.getFlowBuilder();
        String msg = "Failed to add data flow into DS: match=" +
            builder.getIngressMatchKey();
        FlowAddContext.LOG.error(msg, t);
        context.setFailure(t);
    }
}
