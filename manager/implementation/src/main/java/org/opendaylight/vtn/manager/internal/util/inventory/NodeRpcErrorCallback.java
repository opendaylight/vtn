/*
 * Copyright (c) 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.inventory;

import org.slf4j.Logger;

import org.opendaylight.vtn.manager.internal.util.rpc.RpcErrorCallback;

import org.opendaylight.yangtools.yang.common.RpcResult;

/**
 * An implementation of {@link RpcErrorCallback} used to record error logs
 * caused by asynchronous RPC invocation routed to OpenFlow switch.
 *
 * @param <T>  The type of the object to be returned by the RPC.
 */
public class NodeRpcErrorCallback<T> extends RpcErrorCallback<T> {
    /**
     * The invocation of node-routed RPC.
     */
    private final NodeRpcInvocation<?, T>  invocation;

    /**
     * Construct a new instance.
     *
     * <p>
     *   This constructor takes a format string and variable-sized arguments.
     *   An error message is created by {@link String#format(String, Object[])}
     *   with specifying the given format string and arguments.
     * </p>
     *
     * @param rpc   An {@link NodeRpcInvocation} instance that indicates an
     *              RPC invocation.
     * @param log   A logger instance.
     * @param fmt   A format string.
     * @param args  Arguments for an error message.
     */
    public NodeRpcErrorCallback(NodeRpcInvocation<?, T> rpc, Logger log,
                                String fmt, Object ... args) {
        super(rpc, log, fmt, args);
        invocation = rpc;
        rpc.start();
    }

    // FutureCallback

    /**
     * Invoked when the RPC has completed.
     *
     * @param result  An object returned by the RPC.
     */
    @Override
    public void onSuccess(RpcResult<T> result) {
        invocation.complete();
        super.onSuccess(result);
    }

    /**
     * Invoked when the RPC has failed.
     *
     * @param t  A {@link Throwable} thrown by the RPC implementation.
     */
    @Override
    public void onFailure(Throwable t) {
        invocation.complete();

        // Suppress error log if the RPC has been canceled.
        if (invocation.needErrorLog(t)) {
            super.onFailure(t);
        }
    }
}
