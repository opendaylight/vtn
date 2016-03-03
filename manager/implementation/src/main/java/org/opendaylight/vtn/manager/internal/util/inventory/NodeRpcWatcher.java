/*
 * Copyright (c) 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.inventory;

/**
 * {@code NodeRpcWatcher} watches all the invocations of RPC routed to
 * OpenFlow switches.
 *
 * <p>
 *   If a swtich is removed, all the RPC invocations routed to the removed
 *   switch will be canceled.
 * </p>
 */
public interface NodeRpcWatcher {
    /**
     * Register the specified RPC invocation routed to the specifiec switch.
     *
     * @param rpc  An RPC invocation to be registered.
     */
    void registerRpc(NodeRpcInvocation<?, ?> rpc);

    /**
     * Unregister the specified RPC invocation.
     *
     * @param rpc  An RPC invocation to be unregistered.
     */
    void unregisterRpc(NodeRpcInvocation<?, ?> rpc);
}
