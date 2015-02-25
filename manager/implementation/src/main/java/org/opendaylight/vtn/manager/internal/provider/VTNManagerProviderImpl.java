/*
 * Copyright (c) 2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.provider;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.Timer;
import java.util.concurrent.Future;
import java.util.concurrent.atomic.AtomicReference;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import com.google.common.util.concurrent.FutureCallback;
import com.google.common.util.concurrent.Futures;
import com.google.common.util.concurrent.ListenableFuture;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.RouteResolver;
import org.opendaylight.vtn.manager.internal.TxContext;
import org.opendaylight.vtn.manager.internal.TxTask;
import org.opendaylight.vtn.manager.internal.VTNConfig;
import org.opendaylight.vtn.manager.internal.VTNFlowMatch;
import org.opendaylight.vtn.manager.internal.VTNManagerImpl;
import org.opendaylight.vtn.manager.internal.VTNManagerProvider;
import org.opendaylight.vtn.manager.internal.config.VTNConfigImpl;
import org.opendaylight.vtn.manager.internal.config.VTNConfigManager;
import org.opendaylight.vtn.manager.internal.inventory.VTNInventoryManager;
import org.opendaylight.vtn.manager.internal.packet.VTNPacketService;
import org.opendaylight.vtn.manager.internal.routing.VTNRoutingManager;
import org.opendaylight.vtn.manager.internal.util.CompositeAutoCloseable;
import org.opendaylight.vtn.manager.internal.util.SalPort;
import org.opendaylight.vtn.manager.internal.util.concurrent.CanceledFuture;
import org.opendaylight.vtn.manager.internal.util.concurrent.FutureCallbackTask;
import org.opendaylight.vtn.manager.internal.util.concurrent.FutureCanceller;
import org.opendaylight.vtn.manager.internal.util.concurrent.VTNFuture;
import org.opendaylight.vtn.manager.internal.util.concurrent.VTNThreadPool;
import org.opendaylight.vtn.manager.internal.util.tx.ReadTxContext;
import org.opendaylight.vtn.manager.internal.util.tx.TxQueueImpl;
import org.opendaylight.vtn.manager.internal.util.tx.TxSyncFuture;

import org.opendaylight.controller.md.sal.binding.api.DataBroker;
import org.opendaylight.controller.sal.binding.api.NotificationProviderService;
import org.opendaylight.controller.sal.binding.api.RpcProviderRegistry;

import org.opendaylight.controller.sal.packet.Packet;
import org.opendaylight.controller.sal.utils.StatusCode;

import org.opendaylight.yangtools.yang.binding.Notification;
import org.opendaylight.yangtools.yang.binding.RpcService;

/**
 * MD-SAL service provider of the VTN Manager.
 */
public final class VTNManagerProviderImpl implements VTNManagerProvider {
    /**
     * Logger instance.
     */
    private static final Logger  LOG =
        LoggerFactory.getLogger(VTNManagerProviderImpl.class);

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
     * VTN configuration manager.
     */
    private final AtomicReference<VTNConfigManager>  configManager;

    /**
     * VTN inventory manager.
     */
    private final AtomicReference<VTNInventoryManager>  inventoryManager;

    /**
     * VTN packet routing manager.
     */
    private final AtomicReference<VTNRoutingManager>  routingManager;

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
    private final AtomicReference<Timer>  globalTimer;

    /**
     * Registrations of RPC services provided by the VTN Manager.
     */
    private final CompositeAutoCloseable  rpcServices =
        new CompositeAutoCloseable(LOG);

    /**
     * AD-SAL VTN Manager service.
     */
    private AtomicReference<VTNManagerImpl>  vtnManager =
        new AtomicReference<>();

    /**
     * Construct a new instance.
     *
     * @param broker  A {@link DataBroker} service instance.
     * @param rpcReg  A {@link RpcProviderRegistry} service instance.
     * @param nsv     A {@link NotificationProviderService} service instance.
     */
    public VTNManagerProviderImpl(DataBroker broker,
                                  RpcProviderRegistry rpcReg,
                                  NotificationProviderService nsv) {
        dataBroker = broker;
        rpcRegistry = rpcReg;
        notificationService = nsv;
        globalExecutor =
            new VTNThreadPool("VTN Async Thread", THREAD_POOL_MAXSIZE,
                              THREAD_POOL_KEEPALIVE);
        globalTimer = new AtomicReference<Timer>(
            new Timer("Global timer for VTN provider"));

        TxQueueImpl globq =  new TxQueueImpl("VTN Main", this);
        globalQueue = new AtomicReference<TxQueueImpl>(globq);
        VTNConfigManager cfm = new VTNConfigManager(this);
        configManager = new AtomicReference<VTNConfigManager>(cfm);
        boolean configProvider = cfm.isConfigProvider();

        VTNInventoryManager vim = new VTNInventoryManager(this);
        inventoryManager = new AtomicReference<VTNInventoryManager>(vim);

        List<VTNFuture<?>> futures = new ArrayList<>();
        VTNRoutingManager rtm = new VTNRoutingManager(this);
        routingManager = new AtomicReference<VTNRoutingManager>(rtm);
        futures.add(rtm.initConfig(configProvider));

        VTNPacketService psv = new VTNPacketService(this, nsv);
        vim.addListener(psv);
        packetService = new AtomicReference<VTNPacketService>(psv);

        globq.start();

        // Wait for completion of initialization.
        try {
            for (VTNFuture<?> f: futures) {
                f.checkedGet();
            }
        } catch (Exception e) {
            String msg = "Failed to initialize VTN Manager provider.";
            LOG.error(msg, e);
            throw new IllegalStateException(msg, e);
        }

        // Register RPC services.
        try {
            rtm.initRpcServices(rpcReg, rpcServices);
        } catch (RuntimeException e) {
            rpcServices.close();
            throw e;
        }

        LOG.info("VTN Manager provider has been initialized.");
    }

    /**
     * Determine whether this instance can be reused or not.
     *
     * @return  {@code true} only if this instance can be reused.
     */
    public boolean canReuse() {
        AtomicReference<?>[] refs = {
            configManager,
            globalQueue,
            globalTimer,
            packetService,
        };
        for (AtomicReference<?> ref: refs) {
            if (ref.get() == null) {
                return false;
            }
        }

        VTNInventoryManager vim = inventoryManager.get();
        if (vim == null || !vim.isAlive()) {
            return false;
        }

        if (rpcServices.isClosed()) {
            return false;
        }

        return globalExecutor.isAlive();
    }

    // VTNManagerProvider

    /**
     * {@inheritDoc}
     */
    @Override
    public void setVTNManager(VTNManagerImpl mgr) {
        VTNInventoryManager vim = inventoryManager.get();
        if (vim != null) {
            vim.addListener(mgr);
        }

        VTNRoutingManager rtm = routingManager.get();
        if (rtm != null) {
            rtm.addListener(mgr);
        }

        VTNPacketService psv = packetService.get();
        if (psv != null) {
            psv.addListener(mgr);
        }

        vtnManager.set(mgr);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void shutdown() {
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
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void configLoaded() {
        VTNConfigManager cfm = configManager.get();
        if (cfm != null) {
            cfm.initDone();
        }
    }

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
        return globalTimer.get();
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
                new FutureCallbackTask<T>(future, cb, globalTimer.get());
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
    public RouteResolver getRouteResolver(int id) {
        VTNRoutingManager rtm = routingManager.get();
        return (rtm == null) ? null : rtm.getRouteResolver(id);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public List<VTNFuture<?>> removeFlows(VTNFlowMatch fmatch) {
        VTNManagerImpl mgr = vtnManager.get();
        if (mgr != null) {
            List<VTNFuture<?>> list = new ArrayList<>();
            list.addAll(mgr.removeFlows(fmatch));
            return list;
        }

        return Collections.<VTNFuture<?>>emptyList();
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
            throw new VTNException(StatusCode.NOSERVICE, msg);
        }

        return impl;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public <T extends RpcService> T getVtnRpcService(Class<T> type)
        throws VTNException {
        if (rpcServices.isClosed()) {
            String msg = "VTN service is already closed.";
            throw new VTNException(StatusCode.NOSERVICE, msg);
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
        Timer timer = globalTimer.get();
        if (timer == null) {
            return f;
        }

        // Schedule a timer task that cancels the wait for the background
        // tasks.
        TxSyncFuture<T> bgf = new TxSyncFuture<T>(task, f);
        FutureCanceller.set(timer, TX_BGTASK_TIMEOUT, bgf);

        return bgf;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void close() {
        shutdown();
        rpcServices.close();
        globalExecutor.shutdown();

        VTNRoutingManager rtm = routingManager.getAndSet(null);
        if (rtm != null) {
            rtm.close();

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

            Timer timer = globalTimer.getAndSet(null);
            if (timer != null) {
                timer.cancel();
            }

            globalExecutor.close();
            vtnManager.set(null);

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
}
