/*
 * Copyright (c) 2015, 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.rpc;

import static org.mockito.Mockito.any;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.reset;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.verifyNoMoreInteractions;
import static org.mockito.Mockito.when;

import java.util.Collection;

import org.junit.Test;

import org.slf4j.Logger;

import org.opendaylight.vtn.manager.internal.TestBase;

import org.opendaylight.yangtools.yang.common.RpcError.ErrorType;
import org.opendaylight.yangtools.yang.common.RpcResult;
import org.opendaylight.yangtools.yang.common.RpcResultBuilder;

/**
 * JUnit test for {@link RpcErrorCallback}.
 */
public class RpcErrorCallbackTest extends TestBase {
    /**
     * Test case for the following callbacks with specifying arguments for
     * a log message.
     *
     * <ul>
     *   <li>{@link RpcErrorCallback#onSuccess(RpcResult)}</li>
     *   <li>{@link RpcErrorCallback#onFailure(Throwable)}</li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testLogArgs() throws Exception {
        String name = "rpc-error-callback-test-1";
        String input = "{test input 1}";
        RpcRequest req = mock(RpcRequest.class);
        when(req.getName()).thenReturn(name);
        when(req.getInputForLog()).thenReturn(input);
        when(req.needErrorLog(any(Collection.class))).thenReturn(true);

        // RPC succeeded.
        Logger logger = mock(Logger.class);
        Long ret = System.nanoTime();
        RpcResult<Long> result = RpcResultBuilder.success(ret).build();
        String format = "test message 1: %s, %d";
        String arg1 = "arg1:" + result;
        Long arg2 = System.currentTimeMillis();
        String emsg = name + ": " + String.format(format, arg1, arg2) +
            ": input=" + input;

        RpcErrorCallback<Long> cb =
            new RpcErrorCallback<>(req, logger, format, arg1, arg2);
        cb.onSuccess(result);
        verify(logger).
            trace("{}: RPC completed successfully: result={}", name, ret);
        verifyNoMoreInteractions(logger);
        reset(logger);

        // RPC failed.
        ErrorType etype = ErrorType.APPLICATION;
        String errormsg = "RPC error";
        String exmsg = "RPC returned error";
        result = RpcResultBuilder.<Long>failed().
            withError(etype, errormsg).build();
        cb.onSuccess(result);

        verify(logger).
            error("{}: {}: input={}, errors={}", name, exmsg, input,
                  result.getErrors());
        verify(logger).error(emsg);
        verifyNoMoreInteractions(logger);
        reset(logger);

        // Caught an exception.
        IllegalStateException ise = new IllegalStateException();
        cb.onFailure(ise);
        verify(logger).error(emsg, ise);
        verifyNoMoreInteractions(logger);
    }

    /**
     * Test case for the following callbacks without specifying arguments for
     * a log message.
     *
     * <ul>
     *   <li>{@link RpcErrorCallback#onSuccess(RpcResult)}</li>
     *   <li>{@link RpcErrorCallback#onFailure(Throwable)}</li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testNoLogArgs() throws Exception {
        String name = "rpc-error-callback-test-2";
        String input = "{test input 2}";
        RpcRequest req = mock(RpcRequest.class);
        when(req.getName()).thenReturn(name);
        when(req.getInputForLog()).thenReturn(input);
        when(req.needErrorLog(any(Collection.class))).thenReturn(true);

        // RPC succeeded.
        Logger logger = mock(Logger.class);
        Long ret = System.nanoTime();
        RpcResult<Long> result = RpcResultBuilder.success(ret).build();
        String format = "test message 2.";
        Object[] args = null;
        String emsg = name + ": " + format + ": input=" + input;

        RpcErrorCallback<Long> cb =
            new RpcErrorCallback<>(req, logger, format, args);
        cb.onSuccess(result);
        verify(logger).
            trace("{}: RPC completed successfully: result={}", name, ret);
        verifyNoMoreInteractions(logger);
        reset(logger);

        // RPC failed.
        ErrorType etype = ErrorType.APPLICATION;
        String errormsg = "RPC error";
        String exmsg = "RPC returned error";
        result = RpcResultBuilder.<Long>failed().
            withError(etype, errormsg).build();
        cb.onSuccess(result);

        verify(logger).
            error("{}: {}: input={}, errors={}", name, exmsg, input,
                  result.getErrors());
        verify(logger).error(emsg);
        verifyNoMoreInteractions(logger);
        reset(logger);

        // Caught an exception.
        IllegalStateException ise = new IllegalStateException();
        cb.onFailure(ise);
        verify(logger).error(emsg, ise);
        verifyNoMoreInteractions(logger);
    }

    /**
     * Test case for {@link RpcErrorCallback#onSuccess(RpcResult)}.
     *
     * <p>
     *   In case where {@link RpcUtils#checkResult(RpcRequest,RpcResult,Logger)}
     *   throws an {@link RuntimeException}.
     * </p>
     */
    @Test
    public void testRuntimeException() {
        String name = "rpc-error-callback-test-3";
        String input = "{test input 3}";
        RpcRequest req = mock(RpcRequest.class);
        when(req.getName()).thenReturn(name);
        when(req.getInputForLog()).thenReturn(input);
        when(req.needErrorLog(any(Collection.class))).thenReturn(true);

        Logger logger = mock(Logger.class);
        String emsg = name + ": Failed to check the RPC result.";
        String format = "test message 3.";
        String emsg1 = name + ": Failed to check the RPC result.";
        String emsg2 = name + ": " + format + ": input=" + input;

        @SuppressWarnings("unchecked")
        RpcResult<Long> result = mock(RpcResult.class);
        IllegalStateException ise = new IllegalStateException();
        when(result.isSuccessful()).thenThrow(ise);

        RpcErrorCallback<Long> cb =
            new RpcErrorCallback<>(req, logger, format);
        cb.onSuccess(result);
        verify(logger).error(emsg1, ise);
        verify(logger).error(emsg2);
        verifyNoMoreInteractions(logger);
    }
}
