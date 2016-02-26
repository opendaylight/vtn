/*
 * Copyright (c) 2015, 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.rpc;

import org.slf4j.Logger;

import com.google.common.util.concurrent.FutureCallback;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.yangtools.yang.common.RpcResult;

/**
 * An implementation of {@link FutureCallback} used to log error of
 * asynchronous RPC execution.
 *
 * @param <T>  The type of the object to be returned by the RPC.
 */
public class RpcErrorCallback<T> implements FutureCallback<RpcResult<T>> {
    /**
     * An {@link RpcRequest} instance that indicates an RPC request.
     */
    private final RpcRequest  request;

    /**
     * A logger instance.
     */
    private final Logger  logger;

    /**
     * A format string to construct an error message.
     */
    private final String  format;

    /**
     * An array of objects used to construct an error message.
     */
    private final Object[]  arguments;

    /**
     * Construct a new instance.
     *
     * <p>
     *   This constructor takes a format string and variable-sized arguments.
     *   An error message is created by {@link String#format(String, Object[])}
     *   with specifying the given format string and arguments.
     * </p>
     *
     * @param req   An {@link RpcRequest} instance that indicates an RPC
     *              request.
     * @param log   A logger instance.
     * @param fmt   A format string.
     * @param args  Arguments for an error message.
     */
    public RpcErrorCallback(RpcRequest req, Logger log, String fmt,
                            Object ... args) {
        request = req;
        logger = log;
        format = fmt;
        arguments = (args == null) ? null : args.clone();
    }

    /**
     * Return the error message.
     *
     * @return  The error message.
     */
    private String getErrorMessage() {
        return new StringBuilder(request.getName()).
            append(": ").append(String.format(format, arguments)).
            append(": input=").append(request.getInputForLog()).
            toString();
    }

    // FutureCallback

    /**
     * Invoked when the future has completed successfully.
     *
     * @param result  An object returned by the RPC.
     */
    @Override
    public void onSuccess(RpcResult<T> result) {
        try {
            T ret = RpcUtils.checkResult(request, result, logger);
            logger.trace("{}: RPC completed successfully: result={}",
                         request.getName(), ret);
            return;
        } catch (VTNException e) {
            // This error should be logged by checkResult().
        } catch (RuntimeException e) {
            logger.error(request.getName() +
                         ": Failed to check the RPC result.", e);
        }

        logger.error(getErrorMessage());
    }

    /**
     * Invoked when the future has failed.
     *
     * @param t  A {@link Throwable} thrown by the RPC implementation.
     */
    @Override
    public void onFailure(Throwable t) {
        logger.error(getErrorMessage(), t);
    }
}
