/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.provider;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.util.ArrayList;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.ListIterator;
import java.util.Map;
import java.util.concurrent.atomic.AtomicReference;

import org.opendaylight.vtn.manager.internal.VTNSubSystem;
import org.opendaylight.vtn.manager.internal.util.CompositeAutoCloseable;
import org.opendaylight.vtn.manager.internal.util.MiscUtils;
import org.opendaylight.vtn.manager.internal.util.concurrent.VTNFuture;

import org.opendaylight.controller.sal.binding.api.RpcProviderRegistry;

import org.opendaylight.yangtools.yang.binding.RpcService;

/**
 * {@code SubSystemRegistry} keeps instances that manage subsystems in the
 * VTN Manager.
 *
 * <p>
 *   Each subsystem manager is represented by {@link VTNSubSystem} interface,
 *   and it is indexed by its class.
 * </p>
 */
public final class SubSystemRegistry {
    /**
     * Logger instance.
     */
    private static final Logger  LOG =
        LoggerFactory.getLogger(SubSystemRegistry.class);

    /**
     * Pairs of subsystem provider classes and instances.
     */
    private final AtomicReference<Map<Class<? extends VTNSubSystem>, VTNSubSystem>> providers =
        new AtomicReference<>();

    /**
     * Registrations of RPC services provided by subsystems.
     */
    private final CompositeAutoCloseable  rpcServices =
        new CompositeAutoCloseable(LOG);

    /**
     * Construct a new instance.
     */
    public SubSystemRegistry() {
        Map<Class<? extends VTNSubSystem>, VTNSubSystem> map =
            new LinkedHashMap<>();
        providers.set(map);
    }

    /**
     * Add the given configuration provider into this set.
     *
     * @param obj  A {@link VTNSubSystem} instance.
     * @param <T>  The type of configuration provider.
     * @return  This instance.
     */
    public <T extends VTNSubSystem> SubSystemRegistry add(T obj) {
        Map<Class<? extends VTNSubSystem>, VTNSubSystem> map =
            providers.get();
        Class<? extends VTNSubSystem> type = obj.getClass();
        map.put(type, obj);
        if (LOG.isDebugEnabled()) {
            LOG.debug("New subsystem: {}", type.getSimpleName());
        }

        return this;
    }

    /**
     * Return the configuration provider indexed by the given class.
     *
     * @param type  A class which indicates the type of the configuration
     *              provider.
     * @param <T>   The type of the configuration provider.
     * @return  An instance of the specified configuration provider or
     *          {@code null}.
     */
    public <T extends VTNSubSystem> T get(Class<T> type) {
        Map<Class<? extends VTNSubSystem>, VTNSubSystem> map =
            providers.get();
        if (map != null) {
            VTNSubSystem vss = map.get(type);
            if (type.isInstance(vss)) {
                return type.cast(vss);
            }
        }

        return null;
    }

    /**
     * Post a MD-SAL datastore transaction task that initializes the
     * configuration.
     *
     * @param master  {@code true} if the local node is the configuration
     *                provider.
     * @return  A list of {@link VTNFuture} instances associated with
     *          background tasks that initialize the configuration.
     */
    public List<VTNFuture<?>> initConfig(boolean master) {
        Map<Class<? extends VTNSubSystem>, VTNSubSystem> map =
            providers.get();
        List<VTNFuture<?>> futures = new ArrayList<>();
        for (VTNSubSystem vss: map.values()) {
            if (LOG.isDebugEnabled()) {
                LOG.debug("Initializing configuration: {}",
                          vss.getClass().getSimpleName());
            }

            VTNFuture<?> f = vss.initConfig(master);
            if (f != null) {
                futures.add(f);
            }
        }

        return futures;
    }

    /**
     * Initialize RPC services provided by subsystems.
     *
     * @param rpcReg  A {@link RpcProviderRegistry} service instance.
     */
    public void initRpcServices(RpcProviderRegistry rpcReg) {
        Map<Class<? extends VTNSubSystem>, VTNSubSystem> map =
            providers.get();
        for (VTNSubSystem vss: map.values()) {
            if (LOG.isDebugEnabled()) {
                LOG.debug("Initializing RPC: {}",
                          vss.getClass().getSimpleName());
            }
            vss.initRpcServices(rpcReg, rpcServices);
        }
    }

    /**
     * Add the given RPC registration to this instance.
     *
     * @param rpcReg  A {@link RpcProviderRegistry} service instance.
     * @param type    A class which specifies the type of RPC service.
     * @param impl    An object which implements the RPC service.
     * @param <T>     Type of RPC service.
     */
    public <T extends RpcService> void addRpcService(
        RpcProviderRegistry rpcReg, Class<T> type, T impl) {
        rpcServices.add(rpcReg.addRpcImplementation(type, impl));
    }

    /**
     * Determine whether the subsystem registry is already closed.
     *
     * @return  {@code true} only if the subsystem registry is already closed.
     */
    public boolean isClosed() {
        return (providers.get() == null);
    }

    /**
     * Determine whether RPC services are closed or not.
     *
     * @return  {@code true} only if RPC services are already closed.
     */
    public boolean isRpcClosed() {
        return rpcServices.isClosed();
    }

    /**
     * Close RPC services.
     */
    public void closeRpc() {
        rpcServices.close();
    }

    /**
     * Close all subsystems.
     *
     * @return  {@code true} only if all registered subsystems have been
     *          actually closed.
     */
    public boolean close() {
        closeRpc();

        Map<Class<? extends VTNSubSystem>, VTNSubSystem> map =
            providers.getAndSet(null);
        if (map == null) {
            return false;
        }

        // Close subsystems in reverse order of registration.
        List<VTNSubSystem> list = new ArrayList<>(map.values());
        int size = list.size();
        for (ListIterator<VTNSubSystem> it = list.listIterator(size);
             it.hasPrevious();) {
            VTNSubSystem vss = it.previous();
            try {
                vss.close();
            } catch (Exception e) {
                String msg = MiscUtils.joinColon(
                    vss.getClass().getSimpleName(),
                    "Failed to close provider.");
                LOG.error(msg, e);
            }
        }

        return true;
    }
}
