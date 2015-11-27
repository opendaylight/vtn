/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.neutron.impl;

import static org.mockito.Mockito.any;
import static org.mockito.Mockito.anyBoolean;
import static org.mockito.Mockito.anyLong;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.when;

import java.util.Set;
import java.util.concurrent.Future;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.TimeoutException;

import com.google.common.base.Optional;
import com.google.common.util.concurrent.CheckedFuture;
import com.google.common.util.concurrent.Futures;

import org.junit.Assert;

import org.opendaylight.controller.md.sal.common.api.data.ReadFailedException;

import org.opendaylight.yangtools.yang.binding.DataObject;
import org.opendaylight.yangtools.yang.common.RpcError.ErrorType;
import org.opendaylight.yangtools.yang.common.RpcError;
import org.opendaylight.yangtools.yang.common.RpcResult;
import org.opendaylight.yangtools.yang.common.RpcResultBuilder;

/**
 * Abstract base class for JUnit tests.
 */
public abstract class TestBase extends Assert {
    /**
     * Ensure that the given two objects are identical.
     *
     * @param set  A set of tested objects.
     * @param o1   An object to be tested.
     * @param o2   An object to be tested.
     */
    protected static void testEquals(Set<Object> set, Object o1, Object o2) {
        assertEquals(o1, o2);
        assertEquals(o2, o1);
        assertEquals(o1, o1);
        assertEquals(o2, o2);
        assertEquals(o1.hashCode(), o2.hashCode());
        assertFalse(o1.equals(null));
        assertFalse(o1.equals(new Object()));
        assertFalse(o1.equals("string"));
        assertFalse(o1.equals(set));

        for (Object o : set) {
            assertFalse("o1=" + o1 + ", o=" + o, o1.equals(o));
            assertFalse(o.equals(o1));
        }

        assertTrue(set.add(o1));
        assertFalse(set.add(o1));
        assertFalse(set.add(o2));
    }

    /**
     * Create a response of read request on a MD-SAL transaction.
     *
     * @param obj  An object to be read.
     *             {@code null} implies the target data is not present.
     * @return  A {@link CheckedFuture} instance.
     * @param <T>  The type of the data object.
     */
    protected static <T extends DataObject> CheckedFuture<Optional<T>, ReadFailedException> getReadResult(
        T obj) {
        Optional<T> opt = Optional.fromNullable(obj);
        return Futures.immediateCheckedFuture(opt);
    }

    /**
     * Create an error response of read request on a MD-SAL transaction.
     *
     * @param type   A class which indicates the type of the return value.
     * @param cause  A throwable which indicates the cause of error.
     * @return  A {@link CheckedFuture} instance.
     * @param <T>  The type of the return value.
     */
    protected static <T extends DataObject> CheckedFuture<Optional<T>, ReadFailedException> getReadFailure(
        Class<T> type, Throwable cause) {
        String msg = "DS read failed";
        RpcError err = RpcResultBuilder.newError(
            ErrorType.APPLICATION, "failed", msg, null, null, cause);
        ReadFailedException rfe = new ReadFailedException(msg, cause, err);
        return Futures.immediateFailedCheckedFuture(rfe);
    }

    /**
     * Create a timeout error response of read request on a MD-SAL transaction.
     *
     * @param type  A class which indicates the type of the return value.
     * @return  A {@link CheckedFuture} instance.
     * @param <T>  The type of the return value.
     * @throws Exception  An error occurred.
     */
    protected static <T extends DataObject> CheckedFuture<Optional<T>, ReadFailedException> getReadTimeoutFailure(
        Class<T> type) throws Exception {
        CheckedFuture<Optional<T>, ReadFailedException> future =
            mock(CheckedFuture.class);

        when(future.cancel(anyBoolean())).thenReturn(false);
        when(future.isCancelled()).thenReturn(false);
        when(future.isDone()).thenReturn(false);

        when(future.get()).thenThrow(
            new AssertionError("get() should never be called."));
        when(future.checkedGet()).thenThrow(
            new AssertionError("checkedGet() should never be called."));
        when(future.get(anyLong(), any(TimeUnit.class))).
            thenThrow(new TimeoutException("DS read timed out"));
        when(future.checkedGet(anyLong(), any(TimeUnit.class))).
            thenThrow(new TimeoutException("DS read timed out"));

        return future;
    }

    /**
     * Return a future that contains RPC result that indicates successful
     * completion.
     *
     * @param output  The output of the RPC.
     * @param <O>     The type of the RPC output.
     * @return  A future that contains the RPC result.
     */
    protected static <O> Future<RpcResult<O>> getRpcFuture(O output) {
        RpcResult<O> result = RpcResultBuilder.success(output).build();
        return Futures.immediateFuture(result);
    }
}
