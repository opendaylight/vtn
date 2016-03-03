/*
 * Copyright (c) 2015, 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.rpc;

import java.util.Collection;
import java.util.concurrent.Future;
import java.util.concurrent.TimeUnit;

import javax.annotation.Nonnull;

import org.slf4j.Logger;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.util.concurrent.AbstractVTNFuture;

import org.opendaylight.yangtools.yang.common.RpcError;
import org.opendaylight.yangtools.yang.common.RpcResult;

/**
 * {@code RpcInvocation} keeps a pair of RPC input and future associated with
 * the RPC execution.
 *
 * @param <I>  The type of the RPC input.
 * @param <O>  The type of the RPC output.
 */
public abstract class RpcInvocation<I, O> implements RpcRequest {
    /**
     * The RPC input.
     */
    private final I  input;

    /**
     * The future associated with the RPC execution with the given input.
     */
    private final Future<RpcResult<O>>  future;

    /**
     * Construct a new instance.
     *
     * @param in  The RPC input.
     * @param f   The future associated with the RPC execution with {@code in}.
     */
    public RpcInvocation(I in, Future<RpcResult<O>> f) {
        input = in;
        future = f;
    }

    /**
     * Return the RPC input.
     *
     * @return  The RPC input.
     */
    public final I getInput() {
        return input;
    }

    /**
     * Return the future associated with the RPC execution.
     *
     * @return  The future associated with the RPC execution.
     */
    public final Future<RpcResult<O>> getFuture() {
        return future;
    }

    /**
     * Wait for the RPC execution to complete, and return the result.
     *
     * @param timeout  The maximum time to wait.
     * @param unit     The time unit of the {@code timeout} argument.
     * @param logger   A {@link Logger} instance.
     * @return  The result of the RPC.
     * @throws VTNException  An error occurred.
     */
    public O getResult(long timeout, TimeUnit unit, Logger logger)
        throws VTNException {
        RpcResult<O> result;
        try {
            result = future.get(timeout, unit);
        } catch (Exception e) {
            if (needErrorLog(e)) {
                boolean canceled = future.cancel(true);
                String msg = new StringBuilder(getName()).
                    append(": Caught an exception: canceled=").
                    append(canceled).append(", input=").
                    append(getInputForLog()).toString();
                logger.error(msg, e);
            }
            throw AbstractVTNFuture.getException(e);
        }

        return RpcUtils.checkResult(this, result, logger);
    }

    /**
     * Determine whether the RPC failure should be logged or not.
     *
     * @param cause  An throwable thrown by the RPC implementation.
     * @return  {@code true} if the RPC failure should be logged.
     *          {@code false} otherwise.
     */
    public boolean needErrorLog(@Nonnull Throwable cause) {
        return true;
    }

    // RpcRequest

    /**
     * Return an object that indicates an RPC input.
     *
     * <p>
     *   Returned object is embedded into a log message.
     * </p>
     *
     * @return  An object that indicates an RPC input.
     */
    @Override
    public Object getInputForLog() {
        return input;
    }

    /**
     * Determine whether the RPC failure should be logged or not.
     *
     * @param errors  A collection of RPC errors returned by the RPC
     *                implementation.
     * @return  {@code true} if the RPC failure should be logged.
     *          {@code false} otherwise.
     */
    @Override
    public boolean needErrorLog(@Nonnull Collection<RpcError> errors) {
        return true;
    }
}
