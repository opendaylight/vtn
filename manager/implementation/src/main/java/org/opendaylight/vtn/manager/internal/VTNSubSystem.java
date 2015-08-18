/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal;

import org.opendaylight.vtn.manager.internal.util.CompositeAutoCloseable;
import org.opendaylight.vtn.manager.internal.util.concurrent.VTNFuture;

import org.opendaylight.controller.sal.binding.api.RpcProviderRegistry;

/**
 * {@code VTNSubSystem} describes a manager instance for a subsystem in the
 * VTN Manager.
 */
public interface VTNSubSystem extends AutoCloseable {
    /**
     * Post a MD-SAL datastore transaction task that initializes the
     * configuration.
     *
     * @param master  {@code true} if the local node is the configuration
     *                provider.
     * @return  A {@link VTNFuture} instance associated with the task for
     *          initialization. {@code null} means that there is nothing to
     *          do on initialization.
     */
    VTNFuture<?> initConfig(boolean master);

    /**
     * Register RPC implementation required by the subsystem.
     *
     * @param rpcReg  A {@link RpcProviderRegistry} service instance.
     * @param regs    A {@link CompositeAutoCloseable} instance to store
     *                RPC registration.
     */
    void initRpcServices(RpcProviderRegistry rpcReg,
                         CompositeAutoCloseable regs);
}
