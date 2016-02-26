/*
 * Copyright (c) 2015, 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.flow;

import java.util.EnumSet;
import java.util.List;
import java.util.Set;
import java.util.Timer;
import java.util.TimerTask;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ConcurrentMap;
import java.util.concurrent.Future;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.TimeoutException;
import java.util.concurrent.atomic.AtomicReference;

import com.google.common.collect.ImmutableSet;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import com.google.common.base.Optional;
import com.google.common.util.concurrent.FutureCallback;
import com.google.common.util.concurrent.Futures;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.FlowRemover;
import org.opendaylight.vtn.manager.internal.TxContext;
import org.opendaylight.vtn.manager.internal.TxTask;
import org.opendaylight.vtn.manager.internal.VTNManagerProvider;
import org.opendaylight.vtn.manager.internal.VTNSubSystem;
import org.opendaylight.vtn.manager.internal.flow.add.FlowAddContext;
import org.opendaylight.vtn.manager.internal.flow.add.PutFlowTxTask;
import org.opendaylight.vtn.manager.internal.flow.reader.FlowCountFuture;
import org.opendaylight.vtn.manager.internal.flow.reader.ReadFlowFuture;
import org.opendaylight.vtn.manager.internal.flow.remove.ClearNodeFlowsTask;
import org.opendaylight.vtn.manager.internal.flow.remove.DeleteFlowTxTask;
import org.opendaylight.vtn.manager.internal.flow.remove.FlowRemoveContext;
import org.opendaylight.vtn.manager.internal.flow.remove.NodeFlowRemover;
import org.opendaylight.vtn.manager.internal.flow.remove.PortFlowRemover;
import org.opendaylight.vtn.manager.internal.flow.remove.RemovedFlowRemover;
import org.opendaylight.vtn.manager.internal.flow.stats.AddedFlowStats;
import org.opendaylight.vtn.manager.internal.flow.stats.StatsReaderService;
import org.opendaylight.vtn.manager.internal.flow.stats.StatsTimerTask;
import org.opendaylight.vtn.manager.internal.inventory.VTNInventoryListener;
import org.opendaylight.vtn.manager.internal.inventory.VtnNodeEvent;
import org.opendaylight.vtn.manager.internal.inventory.VtnPortEvent;
import org.opendaylight.vtn.manager.internal.util.ChangedData;
import org.opendaylight.vtn.manager.internal.util.CompositeAutoCloseable;
import org.opendaylight.vtn.manager.internal.util.DataStoreListener;
import org.opendaylight.vtn.manager.internal.util.DataStoreUtils;
import org.opendaylight.vtn.manager.internal.util.IdentifiedData;
import org.opendaylight.vtn.manager.internal.util.MiscUtils;
import org.opendaylight.vtn.manager.internal.util.VTNEntityType;
import org.opendaylight.vtn.manager.internal.util.concurrent.TimeoutCounter;
import org.opendaylight.vtn.manager.internal.util.concurrent.VTNFuture;
import org.opendaylight.vtn.manager.internal.util.concurrent.VTNThreadPool;
import org.opendaylight.vtn.manager.internal.util.flow.FlowFinder;
import org.opendaylight.vtn.manager.internal.util.flow.FlowUtils;
import org.opendaylight.vtn.manager.internal.util.flow.VTNFlowBuilder;
import org.opendaylight.vtn.manager.internal.util.inventory.SalNode;
import org.opendaylight.vtn.manager.internal.util.log.VTNLogLevel;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcFuture;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcUtils;
import org.opendaylight.vtn.manager.internal.util.tx.AbstractTxTask;
import org.opendaylight.vtn.manager.internal.util.tx.TxQueueImpl;

import org.opendaylight.controller.md.sal.binding.api.NotificationService;
import org.opendaylight.controller.md.sal.binding.api.ReadTransaction;
import org.opendaylight.controller.md.sal.binding.api.ReadWriteTransaction;
import org.opendaylight.controller.md.sal.common.api.clustering.EntityOwnershipChange;
import org.opendaylight.controller.md.sal.common.api.clustering.EntityOwnershipListener;
import org.opendaylight.controller.md.sal.common.api.clustering.EntityOwnershipListenerRegistration;
import org.opendaylight.controller.md.sal.common.api.data.AsyncDataBroker.DataChangeScope;
import org.opendaylight.controller.md.sal.common.api.data.AsyncDataChangeEvent;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;
import org.opendaylight.controller.sal.binding.api.RpcProviderRegistry;

import org.opendaylight.yangtools.yang.binding.DataObject;
import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;
import org.opendaylight.yangtools.yang.common.RpcResult;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.GetDataFlowCountInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.GetDataFlowCountOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.GetDataFlowInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.GetDataFlowOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.VtnFlowId;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.VtnFlowService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.get.data.flow.output.DataFlowInfo;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.NextFlowId;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.NextFlowIdBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.VtnFlows;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.VtnFlowsBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.tenant.flow.info.VtnDataFlow;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.VtnOpenflowVersion;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;

import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.inventory.rev130819.FlowCapableNode;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.inventory.rev130819.tables.Table;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.inventory.rev130819.tables.TableKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.inventory.rev130819.tables.table.Flow;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.service.rev130819.SalFlowService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.rev131026.FlowCookie;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.Nodes;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.nodes.Node;

/**
 * Flow entry manager.
 */
public final class VTNFlowManager
    extends DataStoreListener<Flow, AddedFlowStats>
    implements VTNSubSystem, VTNInventoryListener, VtnFlowService,
               EntityOwnershipListener {
    /**
     * Logger instance.
     */
    private static final Logger  LOG =
        LoggerFactory.getLogger(VTNFlowManager.class);

    /**
     * Required event types.
     */
    private static final Set<VtnUpdateType>  REQUIRED_EVENTS;

    /**
     * The timeout in seconds for the flow service shutdown.
     */
    private static final long  SHUTDOWN_TIMEOUT = 10L;

    /**
     * The number of milliseconds to cache flow cookie notified by
     * switch-flow-removed notification.
     */
    private static final long  REMOVED_COOKIE_EXPIRE = 3000L;

    /**
     * VTN Manager provider service.
     */
    private final VTNManagerProvider  vtnProvider;

    /**
     * MD-SAL flow service.
     */
    private SalFlowService  flowService;

    /**
     * MD-SAL transaction queue thread for updating flow information.
     */
    private final TxQueueImpl  txQueue;

    /**
     * A single-threaded task queue used for updating flow.
     */
    private final VTNThreadPool  flowThread;

    /**
     * The number of active flow transactions.
     */
    private int  txCount;

    /**
     * A map that keeps flow cookies notified by switch-flow-removed
     * notification.
     */
    private final ConcurrentMap<FlowCookie, VtnFlowId>  removedCookies =
        new ConcurrentHashMap<>();

    /**
     * Flow statistics reader service.
     */
    private final StatsReaderService  statsReader;

    /**
     * A periodic timer task that updates the flow statistics.
     */
    private final AtomicReference<StatsTimerTask>  statsTimer =
        new AtomicReference<>();

    /**
     * Registration of the entity ownership listener.
     */
    private final AtomicReference<EntityOwnershipListenerRegistration>  entityListener =
        new AtomicReference<>();

    /**
     * Initialize static fields.
     */
    static {
        Set<VtnUpdateType> set = EnumSet.of(
            VtnUpdateType.CREATED, VtnUpdateType.REMOVED);
        REQUIRED_EVENTS = ImmutableSet.copyOf(set);
    }

    /**
     * MD-SAL transaction task to initialize internal flow containers.
     */
    private static final class InitTask extends AbstractTxTask<Void> {
        /**
         * {@code true} if the local node is the configuration provider.
         */
        private final boolean  configProvider;

        /**
         * {@code true} means that the vtn-flows container has been created.
         */
        private boolean  flowCreated;

        /**
         * {@code true} means that the next-flow-id container has been created.
         */
        private boolean  idCreated;

        /**
         * Construct a new instance.
         *
         * @param master  {@code true} if the local node is the configuration
         *                provider.
         */
        private InitTask(boolean master) {
            configProvider = master;
        }

        /**
         * Initialize the vtn-flow container.
         *
         * @param tx  A {@link ReadWriteTransaction} instance.
         * @return  {@code true} if the vtn-flow container has been created.
         *          {@code false} if the vtn-flow container is present.
         * @throws VTNException  An error occurred.
         */
        private boolean initVtnFlow(ReadWriteTransaction tx)
            throws VTNException {
            // Determine whether the vtn-flows container is present or not.
            InstanceIdentifier<VtnFlows> path =
                InstanceIdentifier.create(VtnFlows.class);
            LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
            Optional<VtnFlows> opt = DataStoreUtils.read(tx, oper, path);
            boolean created = !opt.isPresent();
            if (created) {
                // Create an empty root container.
                VtnFlows root = new VtnFlowsBuilder().build();
                tx.put(oper, path, root, true);
            }

            return created;
        }

        /**
         * Initialize the next-flow-id container.
         *
         * @param tx  A {@link ReadWriteTransaction} instance.
         * @return  {@code true} if the next-flow-id container has been
         *          created.
         *          {@code false} if the next-flow-id container is present.
         * @throws VTNException  An error occurred.
         */
        private boolean initNextFlowId(ReadWriteTransaction tx)
            throws VTNException {
            // Determine whether the next-flow-id container is present or not.
            InstanceIdentifier<NextFlowId> path =
                InstanceIdentifier.create(NextFlowId.class);
            LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
            Optional<NextFlowId> opt = DataStoreUtils.read(tx, oper, path);
            boolean created =
                (!opt.isPresent() || opt.get().getNextId() != null);
            if (created) {
                // Create the next-flow-id container with initial flow ID.
                NextFlowId root = new NextFlowIdBuilder().
                    setNextId(FlowUtils.getInitialFlowId()).build();
                tx.put(oper, path, root, true);
            }

            return created;
        }

        /**
         * {@inheritDoc}
         */
        @Override
        public Void execute(TxContext ctx) throws VTNException {
            ReadWriteTransaction tx = ctx.getReadWriteTransaction();
            flowCreated = initVtnFlow(tx);
            idCreated = initNextFlowId(tx);
            return null;
        }

        /**
         * {@inheritDoc}
         */
        @Override
        public void onSuccess(VTNManagerProvider provider, Void result) {
            if (!configProvider) {
                if (flowCreated) {
                    LOG.info("The vtn-flows container has been created.");
                }
                if (idCreated) {
                    LOG.info("The next-flow-id container has been created.");
                }
            }
        }
    }

    /**
     * A callback for flow transaction future that decrements the transaction
     * counter.
     *
     * @param <T>  The type of the value returned by the future.
     */
    private final class TxCounterCallback<T> implements FutureCallback<T> {
        /**
         * Invoked when the future has completed successfully.
         *
         * @param result  An object returned by the flow transaction.
         */
        @Override
        public void onSuccess(T result) {
            decrementCounter();
        }

        /**
         * Invoked when the future has failed.
         *
         * @param t  A {@link Throwable} thrown by the flow transaction.
         */
        @Override
        public void onFailure(Throwable t) {
            decrementCounter();
        }
    }

    /**
     * Timer task that expires flow cookie for removed flow entry.
     */
    private final class CookieExpireTask extends TimerTask {
        /**
         * The flow cookie to be removed.
         */
        private final FlowCookie  cookie;

        /**
         * Construct a new instance.
         *
         * @param c  A flow cookie.
         */
        private CookieExpireTask(FlowCookie c) {
            cookie = c;
        }

        // Runnable

        /**
         * Expire flow cookie for removed flow entry.
         */
        @Override
        public void run() {
            removedCookies.remove(cookie);
        }
    }

    /**
     * Construct a new instance.
     *
     * @param provider  A VTN Manager provider service.
     * @param nsv       A {@link NotificationService} service instance.
     */
    public VTNFlowManager(VTNManagerProvider provider,
                          NotificationService nsv) {
        super(Flow.class);
        vtnProvider = provider;
        txQueue = new TxQueueImpl("VTN Flow DS", provider);
        addCloseable(txQueue);
        flowThread = new VTNThreadPool("VTN Flow Thread");
        addCloseable(flowThread);

        try {
            // Get MD-SAL RPC service.
            flowService = provider.getRpcService(SalFlowService.class);

            // Register MD-SAL flow listener.
            registerListener(provider.getDataBroker(),
                             LogicalDatastoreType.OPERATIONAL,
                             DataChangeScope.BASE, false);

            // Start flow statistics reader service.
            statsReader = new StatsReaderService(provider, nsv);
            addCloseable(statsReader);
        } catch (Exception e) {
            String msg = "Failed to initialize VTN flow service.";
            LOG.error(msg, e);
            close();
            throw new IllegalStateException(msg, e);
        }
    }

    /**
     * Shut down the VTN flow service.
     */
    public void shutdown() {
        // Unregister entity ownership listener before stopping the
        // flow service.
        EntityOwnershipListenerRegistration reg =
            entityListener.getAndSet(null);
        if (reg != null) {
            reg.close();
        }

        shutdownFlowService();
        stopStatsTimer();

        StatsReaderService srs = statsReader;
        if (srs != null) {
            srs.shutdown();
        }
    }

    /**
     * Add a new VTN data flow.
     *
     * @param builder  A {@link VTNFlowBuilder} instance which contains
     *                 data flow to be installed.
     * @return  A future associated with the task which adds a VTN data flow.
     */
    public synchronized VTNFuture<VtnFlowId> addFlow(VTNFlowBuilder builder) {
        FlowAddContext ctx = new FlowAddContext(flowService, builder);
        VTNFuture<VtnFlowId> future = ctx.getContextFuture();
        if (flowService == null) {
            // The flow service is already closed.
            future.cancel(false);
        } else {
            // Start add-flow transaction task.
            PutFlowTxTask task = new PutFlowTxTask(ctx, flowThread, txQueue);
            startTransaction(future, task);
        }

        return future;
    }

    /**
     * Remove VTN data flows selected by the given flow remover.
     *
     * @param remover  A {@link FlowRemover} instance which determines VTN data
     *                 flows to be removed.
     * @return  A future associated with the task which removes VTN data flows.
     */
    public synchronized VTNFuture<Void> removeFlows(FlowRemover remover) {
        FlowRemoveContext ctx = new FlowRemoveContext(
            flowService, flowThread, remover);
        VTNFuture<Void> future = ctx.getContextFuture();
        if (flowService == null) {
            // The flow service is already closed.
            future.cancel(false);
        } else {
            // Start remove-flow transaction task.
            startTransaction(future, new DeleteFlowTxTask(ctx));
        }

        return future;
    }

    /**
     * Shut down the VTN flow service.
     */
    private synchronized void shutdownFlowService() {
        if (flowService != null) {
            // No more flow transaction will be accepted.
            flowService = null;

            // Wait for all flow transactions to complete.
            TimeoutCounter tc = TimeoutCounter.
                newTimeout(SHUTDOWN_TIMEOUT, TimeUnit.SECONDS);
            try {
                while (txCount > 0) {
                    tc.await(this);
                }
                return;
            } catch (TimeoutException e) {
                if (txCount > 0) {
                    LOG.warn("Flow transaction did not complete: {}", txCount);
                }
            } catch (InterruptedException e) {
                LOG.warn("Shutdown wait has been interrupted.");
            }
        }
    }

    /**
     * Start the flow transaction.
     *
     * @param future  A future associated with the whole flow transaction.
     * @param task    A MD-SAL DS transaction task to be executed.
     * @param <T>     The type of the value to be returned by the flow
     *                transaction.
     * @param <D>     The type of the value to be returned by {@code task}.
     */
    private synchronized <T, D> void startTransaction(VTNFuture<T> future,
                                                      TxTask<D> task) {
        // Increment the transaction counter.
        assert txCount >= 0;
        txCount++;

        // Add callback which will decrement the transaction counter when
        // the future completes.
        TxCounterCallback<T> cb = new TxCounterCallback<>();
        Futures.addCallback(future, cb);

        // Start the task.
        VTNFuture<D> txf = txQueue.post(task);
        if (txf.isCancelled()) {
            // This should never happen.
            LOG.warn("Flow DS queue is already closed unexpectedly.");
            future.cancel(false);
        }
    }

    /**
     * Decrement the transaction counter.
     */
    private synchronized void decrementCounter() {
        assert txCount > 0;
        --txCount;
        if (txCount == 0) {
            notifyAll();
        }
    }

    /**
     * Clear the flow table in the specified switch.
     *
     * @param snode  A {@link SalNode} instance which specifies the target
     *               switch.
     * @param ofver  A {@link VtnOpenflowVersion} instance which specifies the
     *               OpenFlow protocol version. {@code null} means the protocol
     *               version is not yet determined.
     */
    private synchronized void clearFlowTable(SalNode snode,
                                             VtnOpenflowVersion ofver) {
        SalFlowService sfs = flowService;
        if (sfs == null) {
            return;
        }

        // Increment the transaction counter.
        assert txCount >= 0;
        txCount++;

        // Start ClearNodeFlowsTask.
        ClearNodeFlowsTask task = new ClearNodeFlowsTask(
            vtnProvider, sfs, statsReader, snode, ofver);
        VTNFuture<Void> f = task.getFuture();
        if (!flowThread.executeTask(task)) {
            f.cancel(false);
        }

        // Add callback which will decrement the transaction counter when
        // the future completes.
        TxCounterCallback<Void> cb = new TxCounterCallback<>();
        Futures.addCallback(f, cb);
    }

    /**
     * Search for the VTN data flow associated with the given ID.
     *
     * @param flowId  Identifier for the VTN data flow.
     * @return  An {@link IdentifiedData} instance which contains VTN data flow
     *          if found. {@code null} if not found.
     */
    private IdentifiedData<VtnDataFlow> findFlow(VtnFlowId flowId) {
        TxContext ctx = vtnProvider.newTxContext();
        try {
            ReadTransaction rtx = ctx.getTransaction();
            return new FlowFinder(rtx).find(flowId);
        } catch (VTNException e) {
            String msg = "Failed to find VTN data flow: " + flowId.getValue();
            LOG.error(msg, e);
            return null;
        } finally {
            ctx.cancelTransaction();
        }
    }

    /**
     * Start the flow statistics updater if not yet started.
     */
    private void startStatsTimer() {
        StatsTimerTask current = statsTimer.get();
        if (current == null) {
            StatsTimerTask task = new StatsTimerTask(txQueue);
            if (statsTimer.compareAndSet(null, task)) {
                task.start(vtnProvider.getTimer());
                LOG.info("Flow statistics timer task has been started.");
            }
        }
    }

    /**
     * Terminate the flow statistics updater.
     */
    private void stopStatsTimer() {
        StatsTimerTask task = statsTimer.getAndSet(null);
        if (task != null) {
            task.cancel();
            LOG.info("Flow statistics timer task has been canceled.");
        }
    }

    /**
     * Record a log message that indicates the given removed flow entry is
     * ignored.
     *
     * @param level  The logging level.
     * @param msg    A log message.
     * @param snode  The node identifier that specifies the switch.
     * @param flow   The removed flow entry.
     */
    private void ignoreFlowRemoved(VTNLogLevel level, String msg,
                                   SalNode snode, Flow flow) {
        level.log(LOG, "Ignore FLOW_REMOVED: {}: node={}, flow={}",
                  msg, snode, flow);
    }

    // AutoCloseable

    /**
     * Close the VTN flow manager service.
     */
    @Override
    public void close() {
        shutdown();
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

    // VTNSubSystem

    /**
     * {@inheritDoc}
     */
    @Override
    public VTNFuture<?> initConfig(boolean master) {
        TxTask<?> task = new InitTask(master);
        VTNFuture<?> f = txQueue.postFirst(task);
        txQueue.start();

        // Register Entity ownership listener for the flow statistics.
        // Flow statistics timer will be started immediately if this process
        // is the owner of the flow statistics.
        entityListener.set(
            vtnProvider.registerListener(VTNEntityType.FLOW_STATS, this));

        return f;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void initRpcServices(RpcProviderRegistry rpcReg,
                                CompositeAutoCloseable regs) {
        regs.add(rpcReg.addRpcImplementation(VtnFlowService.class, this));
    }

    // VTNInventoryListener

    /**
     * {@inheritDoc}
     */
    @Override
    public void notifyVtnNode(VtnNodeEvent ev) throws VTNException {
        VtnUpdateType type = ev.getUpdateType();
        if (type == VtnUpdateType.CREATED) {
            // Clear the flow table in the new switch.
            VtnOpenflowVersion ofver = ev.getVtnNode().getOpenflowVersion();
            clearFlowTable(ev.getSalNode(), ofver);
        }

        // Uninstall VTN flows affected by the node.
        removeFlows(new NodeFlowRemover(ev.getSalNode()));
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void notifyVtnPort(VtnPortEvent ev) {
        // Uninstall VTN flows affected by the port if it has been disabled
        // or its link state has been changed.
        if (ev.isDisabled() || ev.getInterSwitchLinkChange() != null) {
            removeFlows(new PortFlowRemover(ev.getSalPort()));
        }
    }

    // VtnFlowService

    /**
     * Return information about data flows specified by the RPC input.
     *
     * @param input  Input of the RPC.
     * @return  A {@link Future} associated with the RPC task.
     */
    @Override
    public Future<RpcResult<GetDataFlowOutput>> getDataFlow(
        GetDataFlowInput input) {
        TxContext ctx = vtnProvider.newTxContext();
        try {
            ReadFlowFuture f =
                ReadFlowFuture.create(ctx, txQueue, statsReader, input);
            return new RpcFuture<List<DataFlowInfo>, GetDataFlowOutput>(f, f);
        } catch (VTNException | RuntimeException e) {
            ctx.cancelTransaction();
            return RpcUtils.getErrorBuilder(GetDataFlowOutput.class, e).
                buildFuture();
        }
    }

    /**
     * Return the number of data flows present in the specified VTN.
     *
     * @param input  Input of the RPC.
     * @return  A {@link Future} associated with the RPC task.
     */
    @Override
    public Future<RpcResult<GetDataFlowCountOutput>> getDataFlowCount(
        GetDataFlowCountInput input) {
        TxContext ctx = vtnProvider.newTxContext();
        try {
            FlowCountFuture f = FlowCountFuture.create(ctx, input);
            return new RpcFuture<Integer, GetDataFlowCountOutput>(f, f);
        } catch (RpcException | RuntimeException e) {
            ctx.cancelTransaction();
            return RpcUtils.getErrorBuilder(GetDataFlowCountOutput.class, e).
                buildFuture();
        }
    }

    // DataStoreListener

    /**
     * {@inheritDoc}
     */
    @Override
    protected AddedFlowStats enterEvent(
        AsyncDataChangeEvent<InstanceIdentifier<?>, DataObject> ev) {
        return new AddedFlowStats(LOG);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void exitEvent(AddedFlowStats ectx) {
        ectx.apply(txQueue);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void onCreated(AddedFlowStats ectx, IdentifiedData<Flow> data) {
        ectx.add(data);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void onUpdated(AddedFlowStats ectx, ChangedData<Flow> data) {
        throw MiscUtils.unexpected();
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void onRemoved(AddedFlowStats ectx, IdentifiedData<Flow> data) {
        InstanceIdentifier<Flow> path = data.getIdentifier();
        SalNode snode = SalNode.create(path.firstKeyOf(Node.class));
        if (snode == null) {
            LOG.debug("Ignore FLOW_REMOVED: Invalid flow path: {}", path);
            return;
        }

        Flow flow = data.getValue();
        FlowCookie cookie = flow.getCookie();
        VtnFlowId flowId = FlowUtils.getVtnFlowId(cookie);
        if (flowId == null) {
            ignoreFlowRemoved(VTNLogLevel.DEBUG, "Unexpected cookie",
                              snode, flow);
        } else if (removedCookies.putIfAbsent(cookie, flowId) != null) {
            ignoreFlowRemoved(VTNLogLevel.TRACE, "Already removed",
                              snode, flow);
        } else {
            Timer timer = vtnProvider.getTimer();
            CookieExpireTask task = new CookieExpireTask(cookie);
            timer.schedule(task, REMOVED_COOKIE_EXPIRE);

            // Determine the VTN data flow to be removed.
            IdentifiedData<VtnDataFlow> vdata = findFlow(flowId);
            if (vdata == null) {
                ignoreFlowRemoved(VTNLogLevel.DEBUG, "Data flow not found",
                                  snode, flow);
            } else {
                // Remove VTN data flow that contains removed flow entry.
                removeFlows(new RemovedFlowRemover(vdata, snode));
            }
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected InstanceIdentifier<Flow> getWildcardPath() {
        return InstanceIdentifier.builder(Nodes.class).
            child(Node.class).
            augmentation(FlowCapableNode.class).
            child(Table.class, new TableKey(FlowUtils.TABLE_ID)).
            child(Flow.class).
            build();
    }

    /**
     * Return a set of {@link VtnUpdateType} instances that specifies
     * event types to be listened.
     *
     * @return  A set of {@link VtnUpdateType} instances.
     */
    @Override
    protected Set<VtnUpdateType> getRequiredEvents() {
        return REQUIRED_EVENTS;
    }

    // EntityOwnershipListener

    /**
     * Invoked when the ownership status of the flow statistics has been
     * changed.
     *
     * @param change  An {@link EntityOwnershipChange} instance.
     */
    @Override
    public void ownershipChanged(EntityOwnershipChange change) {
        LOG.debug("Received entity ownerchip change: {}", change);

        if (entityListener.get() != null) {
            if (change.isOwner()) {
                startStatsTimer();

                // Ensure that the listener is still valid.
                if (entityListener.get() == null) {
                    stopStatsTimer();
                }
            } else {
                stopStatsTimer();
            }
        }
    }
}
