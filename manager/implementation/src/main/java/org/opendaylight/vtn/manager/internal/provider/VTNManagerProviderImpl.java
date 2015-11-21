/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.provider;

import java.util.List;
import java.util.Timer;
import java.util.concurrent.Future;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.concurrent.atomic.AtomicReference;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import com.google.common.util.concurrent.FutureCallback;
import com.google.common.util.concurrent.Futures;
import com.google.common.util.concurrent.ListenableFuture;

import org.apache.commons.lang3.tuple.Pair;

import org.osgi.framework.Bundle;
import org.osgi.framework.BundleContext;
import org.osgi.framework.BundleEvent;
import org.osgi.framework.SynchronousBundleListener;
import org.osgi.framework.Version;

import org.opendaylight.vtn.manager.VTNException;
import org.opendaylight.vtn.manager.packet.Packet;

import org.opendaylight.vtn.manager.internal.FlowRemover;
import org.opendaylight.vtn.manager.internal.RouteResolver;
import org.opendaylight.vtn.manager.internal.TxContext;
import org.opendaylight.vtn.manager.internal.TxTask;
import org.opendaylight.vtn.manager.internal.VTNConfig;
import org.opendaylight.vtn.manager.internal.VTNManagerProvider;
import org.opendaylight.vtn.manager.internal.config.VTNConfigImpl;
import org.opendaylight.vtn.manager.internal.config.VTNConfigManager;
import org.opendaylight.vtn.manager.internal.flow.VTNFlowManager;
import org.opendaylight.vtn.manager.internal.flow.cond.FlowCondManager;
import org.opendaylight.vtn.manager.internal.inventory.VTNInventoryManager;
import org.opendaylight.vtn.manager.internal.packet.VTNPacketService;
import org.opendaylight.vtn.manager.internal.routing.PathMapManager;
import org.opendaylight.vtn.manager.internal.routing.VTNRoutingManager;
import org.opendaylight.vtn.manager.internal.util.CompositeAutoCloseable;
import org.opendaylight.vtn.manager.internal.util.VTNEntityType;
import org.opendaylight.vtn.manager.internal.util.VTNTimer;
import org.opendaylight.vtn.manager.internal.util.concurrent.CanceledFuture;
import org.opendaylight.vtn.manager.internal.util.concurrent.FutureCallbackTask;
import org.opendaylight.vtn.manager.internal.util.concurrent.FutureCanceller;
import org.opendaylight.vtn.manager.internal.util.concurrent.SettableVTNFuture;
import org.opendaylight.vtn.manager.internal.util.concurrent.VTNFuture;
import org.opendaylight.vtn.manager.internal.util.concurrent.VTNThreadPool;
import org.opendaylight.vtn.manager.internal.util.flow.VTNFlowBuilder;
import org.opendaylight.vtn.manager.internal.util.inventory.SalPort;
import org.opendaylight.vtn.manager.internal.util.pathpolicy.PathPolicyUtils;
import org.opendaylight.vtn.manager.internal.util.tx.ReadTxContext;
import org.opendaylight.vtn.manager.internal.util.tx.TxQueueImpl;
import org.opendaylight.vtn.manager.internal.util.tx.TxSyncFuture;
import org.opendaylight.vtn.manager.internal.vnode.VTenantManager;

import org.opendaylight.controller.md.sal.binding.api.DataBroker;
import org.opendaylight.controller.md.sal.common.api.clustering.CandidateAlreadyRegisteredException;
import org.opendaylight.controller.md.sal.common.api.clustering.Entity;
import org.opendaylight.controller.md.sal.common.api.clustering.EntityOwnershipCandidateRegistration;
import org.opendaylight.controller.md.sal.common.api.clustering.EntityOwnershipListener;
import org.opendaylight.controller.md.sal.common.api.clustering.EntityOwnershipListenerRegistration;
import org.opendaylight.controller.md.sal.common.api.clustering.EntityOwnershipService;
import org.opendaylight.controller.sal.binding.api.NotificationProviderService;
import org.opendaylight.controller.sal.binding.api.RpcProviderRegistry;

import org.opendaylight.yangtools.yang.binding.Notification;
import org.opendaylight.yangtools.yang.binding.RpcService;
import org.opendaylight.yangtools.yang.common.RpcResult;
import org.opendaylight.yangtools.yang.common.RpcResultBuilder;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.VtnFlowId;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnErrorTag;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.version.rev150901.GetManagerVersionOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.version.rev150901.GetManagerVersionOutputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.version.rev150901.VtnVersionService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.version.rev150901.get.manager.version.output.BundleVersion;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.version.rev150901.get.manager.version.output.BundleVersionBuilder;

/**
 * MD-SAL service provider of the VTN Manager.
 */
public final class VTNManagerProviderImpl
    implements SynchronousBundleListener, VTNManagerProvider,
               VtnVersionService {
    /**
     * Logger instance.
     */
    private static final Logger  LOG =
        LoggerFactory.getLogger(VTNManagerProviderImpl.class);

    /**
     * Current API version of the VTN Manager.
     */
    public static final long  API_VERSION = 2;

    /**
     * The maximum number of threads in the global thread pool for
     * asynchronous tasks.
     */
    private static final int  THREAD_POOL_MAXSIZE = 16;

    /**
     * The number of milliseconds to keep threads in the global thread pool for
     * asynchronous tasks.
     */
    private static final int  THREAD_POOL_KEEPALIVE = 10000;

    /**
     * The number of milliseconds to wait for completion of background tasks
     * started by the MD-SAL transaction task.
     */
    private static final long  TX_BGTASK_TIMEOUT = 30000L;

    /**
     * The number of milliseconds to wait for completion of the global timer
     * thread.
     */
    private static final long  TIMER_SHUTDOWN_TIMEOUT = 10000L;

    /**
     * Internal state that indicates the VTN Manager is active.
     */
    private static final int  STATE_ACTIVE = 0;

    /**
     * Internal state that indicates the VTN Manager is about to be closed.
     */
    private static final int  STATE_CLOSING = 1;

    /**
     * Internal state that indicates the VTN Manager is closed.
     */
    private static final int  STATE_CLOSED = 2;

    /**
     * The OSGi bundle associated with the manager.implementation bundle.
     */
    private final Bundle  implBundle;

    /**
     * Data broker SAL service.
     */
    private final DataBroker  dataBroker;

    /**
     * RPC registry service.
     */
    private final RpcProviderRegistry  rpcRegistry;

    /**
     * Notification provider service.
     */
    private final NotificationProviderService  notificationService;

    /**
     * Entity ownership service.
     */
    private final EntityOwnershipService  entityOwnerService;

    /**
     * VTN configuration manager.
     */
    private final AtomicReference<VTNConfigManager>  configManager;

    /**
     * VTN inventory manager.
     */
    private final AtomicReference<VTNInventoryManager>  inventoryManager;

    /**
     * VTN packet service.
     */
    private final AtomicReference<VTNPacketService>  packetService;

    /**
     * The global transaction submit queue.
     */
    private final AtomicReference<TxQueueImpl>  globalQueue;

    /**
     * The global thread pool to execute commands.
     */
    private final VTNThreadPool  globalExecutor;

    /**
     * The global timer.
     */
    private final VTNTimer  globalTimer;

    /**
     * Registry for internal subsystems.
     */
    private final SubSystemRegistry  subSystems = new SubSystemRegistry();

    /**
     * Registrations of the global entities.
     */
    private final CompositeAutoCloseable  globalEntities =
        new CompositeAutoCloseable(LOG);

    /**
     * The internal state of the VTN Manager.
     */
    private final AtomicInteger  managerState =
        new AtomicInteger(STATE_ACTIVE);

    /**
     * Construct a new instance.
     *
     * @param bctx    A {@link BundleContext} instance associated with the
     *                bundle that contains this class.
     * @param broker  A {@link DataBroker} service instance.
     * @param rpcReg  A {@link RpcProviderRegistry} service instance.
     * @param nsv     A {@link NotificationProviderService} service instance.
     * @param eos     A {@link EntityOwnershipService} serivce instance.
     */
    public VTNManagerProviderImpl(BundleContext bctx, DataBroker broker,
                                  RpcProviderRegistry rpcReg,
                                  NotificationProviderService nsv,
                                  EntityOwnershipService eos) {
        implBundle = bctx.getBundle();
        dataBroker = broker;
        rpcRegistry = rpcReg;
        notificationService = nsv;
        entityOwnerService = eos;
        globalExecutor =
            new VTNThreadPool("VTN Async Thread", THREAD_POOL_MAXSIZE,
                              THREAD_POOL_KEEPALIVE);
        globalTimer = new VTNTimer("Global timer for VTN provider");

        // Initialize global entities in a cluster node.
        initGlobalEntities(eos);

        TxQueueImpl globq =  new TxQueueImpl("VTN Main", this);
        globalQueue = new AtomicReference<>(globq);
        VTNConfigManager cfm = new VTNConfigManager(this);
        configManager = new AtomicReference<>(cfm);
        boolean master = cfm.isConfigProvider();

        VTNInventoryManager vim = new VTNInventoryManager(this);
        inventoryManager = new AtomicReference<>(vim);

        // Initialize internal subsystems.
        VTNFlowManager vfm;
        VTenantManager tnm;
        VTNRoutingManager rtm;
        try {
            vfm = new VTNFlowManager(this, nsv);
            subSystems.add(vfm).
                add(vim.newStaticTopologyManager());
            tnm = new VTenantManager(this);
            rtm = new VTNRoutingManager(this);
            subSystems.add(tnm).
                add(rtm).
                add(new FlowCondManager(this)).
                add(new PathMapManager(this));
        } catch (RuntimeException e) {
            LOG.error("Failed to initialize subsystem.", e);
            subSystems.close();
            throw e;
        }

        vim.addListener(vfm);

        // Resume configurations.
        List<VTNFuture<?>> futures = subSystems.initConfig(master);

        VTNPacketService psv = new VTNPacketService(this, nsv);
        vim.addListener(psv);
        packetService = new AtomicReference<>(psv);

        vim.addListener(tnm);
        rtm.addListener(tnm);
        psv.addListener(tnm);
        globq.start();

        // Wait for completion of initialization.
        try {
            for (VTNFuture<?> f: futures) {
                f.checkedGet();
            }
        } catch (Exception e) {
            String msg = "Failed to initialize VTN Manager provider.";
            LOG.error(msg, e);
            subSystems.close();
            throw new IllegalStateException(msg, e);
        }

        // Start inventory service.
        vim.start();

        // Register RPC services.
        try {
            // Register vtn-version service.
            subSystems.addRpcService(rpcReg, VtnVersionService.class, this);

            subSystems.initRpcServices(rpcReg);
        } catch (RuntimeException e) {
            LOG.error("Failed to initialize RPC service.", e);
            subSystems.close();
            throw e;
        }

        cfm.initDone();

        // Register bundle listener to detect the start of shutdown.
        bctx.addBundleListener(this);

        LOG.info("VTN Manager provider has been initialized.");
    }

    /**
     * Determine whether this instance can be reused or not.
     *
     * @return  {@code true} only if this instance can be reused.
     */
    public boolean canReuse() {
        return (managerState.get() == STATE_ACTIVE);
    }

    /**
     * Initialize global entities in a cluster node.
     *
     * @param eos  A {@link EntityOwnershipService} serivce instance.
     */
    private void initGlobalEntities(EntityOwnershipService eos) {
        for (Entity ent: VTNEntityType.getGlobalEntities()) {
            try {
                globalEntities.add(eos.registerCandidate(ent));
            } catch (Exception e) {
                LOG.error("Failed to register entity candidate: " + ent, e);
                // FALLTHROUGH
            }
        }
    }

    /**
     * Create an output for get-manager-version RPC.
     *
     * @return  A {@link GetManagerVersionOutput} instance.
     */
    private GetManagerVersionOutput getManagerVersionOutput() {
        // Determine OSGi bundle version.
        Version ver = implBundle.getVersion();
        BundleVersion bv = new BundleVersionBuilder().
            setMajor((long)ver.getMajor()).
            setMinor((long)ver.getMinor()).
            setMicro((long)ver.getMicro()).
            setQualifier(ver.getQualifier()).
            build();

        return new GetManagerVersionOutputBuilder().
            setApiVersion(API_VERSION).
            setBundleVersion(bv).
            build();
    }

    /**
     * Shut down all listener services.
     */
    private void shutdown() {
        // Remove OSGi lifecycle listener.
        BundleContext bctx = implBundle.getBundleContext();
        if (bctx != null) {
            bctx.removeBundleListener(this);
        }

        if (managerState.compareAndSet(STATE_ACTIVE, STATE_CLOSING)) {
            LOG.info("VTN Manager provider is going to be closed.");

            // Close packet service.
            VTNPacketService psv = packetService.getAndSet(null);
            if (psv != null) {
                psv.close();
            }

            // Stop inventory event delivery.
            VTNInventoryManager vim = inventoryManager.get();
            if (vim != null) {
                vim.shutdown();
            }

            // Stop RPC services.
            subSystems.closeRpc();

            // Shut down the flow service.
            VTNFlowManager vfm = subSystems.get(VTNFlowManager.class);
            if (vfm != null) {
                vfm.shutdown();
            }

            // Shut down the VTN service.
            VTenantManager tnm = subSystems.get(VTenantManager.class);
            if (tnm != null) {
                tnm.shutdown();
            }

            // Shut down the gloabl timer.
            if (globalTimer.shutdown()) {
                // Flush all the timers previously scheduled.
                try {
                    globalTimer.flush(TIMER_SHUTDOWN_TIMEOUT,
                                      TimeUnit.MILLISECONDS);
                } catch (VTNException e) {
                    LOG.warn("Failed to synchronize timer tasks.", e);
                }
            }

            // Unregister global entities.
            globalEntities.close();
        }
    }

    // SynchronousBundleListener

    /**
     * Invoked when the manager.implementation bundle has a lifecycle change.
     *
     * @param ev  A {@link BundleEvent} instance.
     */
    @Override
    public void bundleChanged(BundleEvent ev) {
        Bundle bundle = ev.getBundle();
        if (bundle.getBundleId() == implBundle.getBundleId() &&
            ev.getType() == BundleEvent.STOPPING) {
            // Shut down the service.
            shutdown();
        }
    }

    // VTNManagerProvider

    /**
     * {@inheritDoc}
     */
    @Override
    public VTNConfig getVTNConfig() {
        VTNConfigManager cfm = configManager.get();
        return (cfm == null) ? new VTNConfigImpl() : cfm;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public TxContext newTxContext() {
        return new ReadTxContext(this);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public Timer getTimer() {
        return globalTimer;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public boolean executeTask(Runnable task) {
        return globalExecutor.executeTask(task);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public <T> void setCallback(Future<T> future,
                                FutureCallback<? super T> cb) {
        if (future instanceof ListenableFuture) {
            @SuppressWarnings("unchecked")
            ListenableFuture<T> lf = (ListenableFuture<T>)future;
            Futures.addCallback(lf, cb);
        } else {
            // Wait for the future using global thread pool.
            FutureCallbackTask<T> task =
                new FutureCallbackTask<T>(future, cb, globalTimer);
            globalExecutor.executeTask(task);
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public DataBroker getDataBroker() {
        return dataBroker;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void transmit(SalPort egress, Packet packet) {
        VTNPacketService psv = packetService.get();
        if (psv != null) {
            psv.transmit(egress, packet);
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void transmit(List<Pair<SalPort, Packet>> packets) {
        VTNPacketService psv = packetService.get();
        if (psv != null) {
            psv.transmit(packets);
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public RouteResolver getRouteResolver() {
        return getRouteResolver(PathPolicyUtils.DEFAULT_POLICY);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public RouteResolver getRouteResolver(Integer id) {
        VTNRoutingManager rtm = subSystems.get(VTNRoutingManager.class);
        return (rtm == null) ? null : rtm.getRouteResolver(id);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public VTNFuture<VtnFlowId> addFlow(VTNFlowBuilder builder) {
        VTNFlowManager vfm = subSystems.get(VTNFlowManager.class);
        return (vfm == null)
            ? new CanceledFuture<VtnFlowId>()
            : vfm.addFlow(builder);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public VTNFuture<Void> removeFlows(FlowRemover remover) {
        VTNFlowManager vfm = subSystems.get(VTNFlowManager.class);
        return (vfm == null)
            ? new CanceledFuture<Void>()
            : vfm.removeFlows(remover);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public EntityOwnershipCandidateRegistration registerEntity(Entity ent)
        throws CandidateAlreadyRegisteredException {
        return entityOwnerService.registerCandidate(ent);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public EntityOwnershipListenerRegistration registerListener(
        VTNEntityType etype, EntityOwnershipListener listener) {
        return entityOwnerService.registerListener(etype.getType(), listener);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public <T extends RpcService> T getRpcService(Class<T> type)
        throws VTNException  {
        T impl = rpcRegistry.getRpcService(type);
        if (impl == null) {
            // This should never happen.
            String msg = "RPC service not found: " + type.getSimpleName();
            throw new VTNException(VtnErrorTag.NOSERVICE, msg);
        }

        return impl;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public <T extends RpcService> T getVtnRpcService(Class<T> type)
        throws VTNException {
        if (subSystems.isRpcClosed()) {
            String msg = "VTN service is already closed.";
            throw new VTNException(VtnErrorTag.NOSERVICE, msg);
        }

        return getRpcService(type);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void publish(Notification n) {
        notificationService.publish(n, globalExecutor);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public <T> VTNFuture<T> postSync(TxTask<T> task) {
        VTNFuture<T> f = post(task);
        if (globalTimer.isAvailable()) {
            // Schedule a timer task that cancels the wait for the background
            // tasks.
            TxSyncFuture<T> bgf = new TxSyncFuture<T>(task, f);
            FutureCanceller.set(globalTimer, TX_BGTASK_TIMEOUT, bgf);
            return bgf;
        }

        return f;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void close() {
        shutdown();
        globalExecutor.shutdown();

        if (managerState.getAndSet(STATE_CLOSED) != STATE_CLOSED) {
            subSystems.close();
            VTNInventoryManager vim = inventoryManager.getAndSet(null);
            if (vim != null) {
                vim.close();
            }

            VTNConfigManager cfm = configManager.getAndSet(null);
            if (cfm != null) {
                cfm.close();
            }

            TxQueueImpl txq = globalQueue.getAndSet(null);
            if (txq != null) {
                txq.close();
            }

            globalTimer.cancel();
            globalExecutor.close();

            LOG.info("VTN Manager provider has been closed.");
        }
    }

    // Executor

    /**
     * Execute the specified task on one of worker threads in the pool.
     *
     * @param task  A task to be executed on this thread pool.
     * @throws NullPointerException
     *    {@code task} is {@code null}.
     */
    @Override
    public void execute(Runnable task) {
        globalExecutor.execute(task);
    }

    // TxQueue

    /**
     * {@inheritDoc}
     */
    @Override
    public <T> VTNFuture<T> post(TxTask<T> task) {
        TxQueueImpl txq = globalQueue.get();
        return (txq == null) ? new CanceledFuture<T>() : txq.post(task);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public <T> VTNFuture<T> postFirst(TxTask<T> task) {
        TxQueueImpl txq = globalQueue.get();
        return (txq == null) ? new CanceledFuture<T>() : txq.postFirst(task);
    }

    // VtnVersionService

    /**
     * Return the version information of the VTN Manager.
     *
     * @return  A {@link Future} associated with the RPC task.
     */
    @Override
    public Future<RpcResult<GetManagerVersionOutput>> getManagerVersion() {
        SettableVTNFuture<RpcResult<GetManagerVersionOutput>> future =
            new SettableVTNFuture<>();
        try {
            GetManagerVersionOutput output = getManagerVersionOutput();
            future.set(RpcResultBuilder.success(output).build());
        } catch (RuntimeException e) {
            // This should never happen.
            future.setException(e);
        }

        return future;
    }
}
