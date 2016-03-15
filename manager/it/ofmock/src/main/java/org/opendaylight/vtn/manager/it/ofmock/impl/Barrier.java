/*
 * Copyright (c) 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.ofmock.impl;

import java.util.concurrent.Callable;

import com.google.common.util.concurrent.ListenableFutureTask;

import org.opendaylight.yangtools.yang.common.RpcResult;
import org.opendaylight.yangtools.yang.common.RpcResultBuilder;

/**
 * {@code Barrier} describes a call of send-barrier RPC.
 */
public final class Barrier implements Callable<RpcResult<Void>> {
    /**
     * Create a new future task associated with the send-barrier RPC task.
     *
     * @return  The future task associated with the send-barrier RPC task.
     */
    public static ListenableFutureTask<RpcResult<Void>> create() {
        return ListenableFutureTask.create(new Barrier());
    }

    /**
     * Construct a new instance.
     */
    private Barrier() {
    }

    // Callable

    /**
     * Return a result of send-barrier RPC.
     *
     * @return  The result of send-barrier RPC.
     */
    @Override
    public RpcResult<Void> call() {
        return RpcResultBuilder.success((Void)null).build();
    }
}
