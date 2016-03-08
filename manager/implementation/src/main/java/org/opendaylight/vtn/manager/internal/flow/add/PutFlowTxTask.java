/*
 * Copyright (c) 2015, 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.flow.add;

import static org.opendaylight.vtn.manager.internal.flow.add.FlowAddContext.LOG;
import static org.opendaylight.vtn.manager.internal.util.flow.FlowUtils.MAX_FLOW_ID;
import static org.opendaylight.vtn.manager.internal.util.flow.FlowUtils.MIN_FLOW_ID;

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
import org.opendaylight.vtn.manager.internal.util.log.VTNLogLevel;
import org.opendaylight.vtn.manager.internal.util.tx.AbstractTxTask;
import org.opendaylight.vtn.manager.internal.util.vnode.VTenantIdentifier;

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
     * Path to the next-flow-id container.
     */
    private static final InstanceIdentifier<NextFlowId>  NEXT_FLOW_ID_PATH =
        InstanceIdentifier.create(NextFlowId.class);

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
     * Path to the VTN data flow in the operational DS.
     */
    private InstanceIdentifier<VtnDataFlow>  flowPath;

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

    /**
     * Allocate a new VTN flow ID.
     *
     * <p>
     *   This method sets the path to the new VTN data flow to
     *   {@link #flowPath} on successful completion.
     * </p>
     *
     * @param ctx    MD-SAL datastore transaction context.
     * @param tx     A read-write MD-SAL datastore transaction.
     * @param tname  The name of the target VTN.
     * @return  A new flow ID on success.
     *          {@code null} on failure.
     * @throws VTNException  An error occurred.
     */
    private VtnFlowId allocateFlowId(TxContext ctx, ReadWriteTransaction tx,
                                     String tname) throws VTNException {
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        Optional<NextFlowId> opt =
            DataStoreUtils.read(tx, oper, NEXT_FLOW_ID_PATH);
        VtnFlowId flowId = null;
        if (opt.isPresent()) {
            flowId = opt.get().getNextId();
        }
        if (flowId == null) {
            // This should never happen.
            ctx.log(LOG, VTNLogLevel.WARN, "Use initial flow ID.");
            flowId = FlowUtils.getInitialFlowId();
        }

        // Ensure that the flow ID is not used.
        long id = flowId.getValue().longValue();
        long start = id;
        do {
            InstanceIdentifier<VtnDataFlow> path =
                FlowUtils.getIdentifier(tname, flowId);
            Optional<VtnDataFlow> fopt = DataStoreUtils.read(tx, oper, path);

            id++;
            if (id > MAX_FLOW_ID) {
                // Rewind to the minimum value.
                id = MIN_FLOW_ID;
            }

            if (!fopt.isPresent()) {
                // Update the next-flow-id for the next allocation.
                VtnFlowId nextId = new VtnFlowId(BigInteger.valueOf(id));
                NextFlowId nfid = new NextFlowIdBuilder().
                    setNextId(nextId).build();
                tx.put(oper, NEXT_FLOW_ID_PATH, nfid, true);
                flowPath = path;
                return flowId;
            }

        } while (id != start);

        flowPath = null;
        return null;
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
        VTenantIdentifier vtnId = VTenantIdentifier.create(tname, true);
        vtnId.fetch(tx);

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
        VtnFlowId flowId = allocateFlowId(ctx, tx, tname);
        if (flowId == null) {
            ctx.log(LOG, VTNLogLevel.WARN, "No flow ID is available: VTN={}",
                    tname);
            return null;
        }

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
        tx.put(oper, flowPath, vdf, true);

        return vdf;
    }

    // TxTask

    /**
     * Invoked when the task has completed successfully.
     *
     * @param provider  VTN Manager provider service.
     * @param result    The result of this task.
     */
    @Override
    public void onSuccess(VTNManagerProvider provider, VtnDataFlow result) {
        VTNFlowBuilder builder = context.getFlowBuilder();
        if (result == null) {
            // No flow entry was installed.
            LOG.trace("Already installed: match={}",
                      builder.getIngressMatchKey());
            context.setResult(null);
        } else {
            // Install flow entries.
            FlowAddTask task = new FlowAddTask(provider, txQueue, context);
            if (!flowThread.executeTask(task)) {
                // In this case DS queue is also closed.
                // So there is no way to clean up the DS.
                String msg = "Flow thread is already closed";
                LOG.warn("{}: match={}", msg, builder.getIngressMatchKey());
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
        LOG.error(msg, t);
        context.setFailure(t);
    }
}
