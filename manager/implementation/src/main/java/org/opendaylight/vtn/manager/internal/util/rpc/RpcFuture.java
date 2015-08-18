/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.rpc;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.util.concurrent.ExecutionException;

import com.google.common.util.concurrent.FutureCallback;
import com.google.common.util.concurrent.Futures;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.util.concurrent.SettableVTNFuture;
import org.opendaylight.vtn.manager.internal.util.concurrent.VTNFuture;

import org.opendaylight.yangtools.yang.common.RpcResult;
import org.opendaylight.yangtools.yang.common.RpcResultBuilder;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnErrorTag;

/**
 * {@link RpcFuture} describes an ongoing RPC request.
 *
 * <p>
 *   An instance of {@link RpcFuture} is always associated with another future,
 *   and will return the result when the given future completes the execution.
 * </p>
 *
 * @param <I>   The type of the result returned by the future associated with
 *              this future.
 * @param <O>   The type of the RPC output.
 */
public final class RpcFuture<I, O>
    extends SettableVTNFuture<RpcResult<O>> implements FutureCallback<I> {
    /**
     * Logger instance.
     */
    private static final Logger  LOG =
        LoggerFactory.getLogger(RpcFuture.class);

    /**
     * RPC output generator.
     */
    private final RpcOutputGenerator<I, O>  generator;

    /**
     * Construct a new future which represents an ongoing RPC request.
     *
     * @param f    A {@link VTNFuture} associated with the main procedure of
     *             RPC.
     * @param gen  RPC output generator.
     */
    public RpcFuture(VTNFuture<I> f, RpcOutputGenerator<I, O> gen) {
        generator = gen;
        Futures.addCallback(f, this);
    }

    // FutureCallback

    /**
     * Invoked when the main task of the RPC has completed successfully.
     *
     * @param result  The result of the main task.
     */
    @Override
    public void onSuccess(I result) {
        O output = generator.createOutput(result);
        RpcResult<O> res = RpcResultBuilder.success(output).build();
        set(res);
    }

    /**
     * Invoked when the main task of the RPC has failed.
     *
     * @param cause  A {@link Throwable} that indicates the cause of failure.
     */
    @Override
    public void onFailure(Throwable cause) {
        Throwable t = cause;
        if (t instanceof ExecutionException) {
            t = t.getCause();
        }

        RpcResultBuilder<O> builder;
        Class<O> type = generator.getOutputType();
        if (t instanceof VTNException) {
            builder = RpcUtils.getErrorBuilder(type, (VTNException)t);
        } else {
            String msg = "Operation failed due to exception.";
            builder = RpcUtils.getErrorBuilder(
                type, RpcErrorTag.OPERATION_FAILED, msg,
                VtnErrorTag.INTERNALERROR, t.toString());
            LOG.error(msg, t);
        }

        set(builder.build());
    }
}
