/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.rpc;

import org.junit.Test;

import org.mockito.Mockito;

import org.slf4j.Logger;

import org.opendaylight.vtn.manager.internal.TestBase;

import org.opendaylight.yangtools.yang.common.RpcError.ErrorType;
import org.opendaylight.yangtools.yang.common.RpcResult;
import org.opendaylight.yangtools.yang.common.RpcResultBuilder;

/**
 * JUnit test for {@link RpcErrorCallback}
 */
public class RpcErrorCallbackTest extends TestBase {
    /**
     * Test case for the following callbacks.
     *
     * <ul>
     *   <li>{@link RpcErrorCallback#onSuccess(RpcResult)}</li>
     *   <li>{@link RpcErrorCallback#onFailure(Throwable)}</li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testOnSuccess() throws Exception {
        // RPC succeeded.
        Logger logger = Mockito.mock(Logger.class);
        Long ret = System.nanoTime();
        String desc = "rpc-error-callback-test";
        RpcResult<Long> result = RpcResultBuilder.success(ret).build();
        String format = "test message: %s, %d";
        String arg1 = "arg1:" + result;
        Long arg2 = System.currentTimeMillis();
        String emsg = desc + ": " + String.format(format, arg1, arg2);

        RpcErrorCallback<Long> cb =
            new RpcErrorCallback<>(logger, desc, format, arg1, arg2);
        cb.onSuccess(result);
        Mockito.verify(logger).
            trace("{}: RPC completed successfully: result={}", desc, ret);
        Mockito.verify(logger, Mockito.never()).
            error(Mockito.eq(emsg), Mockito.any(Throwable.class));
        Mockito.verify(logger, Mockito.never()).error(emsg);
        Mockito.reset(logger);

        // RPC failed.
        ErrorType etype = ErrorType.APPLICATION;
        String errormsg = "RPC error";
        String exmsg = "RPC returned error";
        result = RpcResultBuilder.<Long>failed().
            withError(etype, errormsg).build();
        cb.onSuccess(result);

        Mockito.verify(logger).
            error("{}: {}: errors={}", desc, exmsg, result.getErrors());
        Mockito.verify(logger).error(emsg);
        Mockito.verify(logger, Mockito.never()).
            trace("{}: RPC completed successfully: result={}", desc, ret);
        Mockito.verify(logger, Mockito.never()).
            error(Mockito.eq(emsg), Mockito.any(Throwable.class));
        Mockito.reset(logger);

        // Caught an exception.
        IllegalStateException ise = new IllegalStateException();
        cb.onFailure(ise);
        Mockito.verify(logger, Mockito.never()).
            trace("{}: RPC completed successfully: result={}", desc, ret);
        Mockito.verify(logger, Mockito.never()).
            error("{}: {}: errors={}", desc, exmsg, result.getErrors());
        Mockito.verify(logger, Mockito.never()).error(emsg);
        Mockito.verify(logger).error(emsg, ise);
    }
}
