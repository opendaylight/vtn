/*
 * Copyright (c) 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.rpc;

import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.verifyNoMoreInteractions;
import static org.mockito.Mockito.verifyZeroInteractions;

import static org.hamcrest.CoreMatchers.instanceOf;

import java.util.Collection;
import java.util.concurrent.Future;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.TimeoutException;

import com.google.common.util.concurrent.SettableFuture;

import org.junit.Test;

import org.slf4j.Logger;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.TestBase;

import org.opendaylight.yangtools.yang.common.RpcError;
import org.opendaylight.yangtools.yang.common.RpcResult;
import org.opendaylight.yangtools.yang.common.RpcResultBuilder;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnErrorTag;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.UpdateVtnInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.UpdateVtnOutput;

/**
 * JUnit test for {@link RpcInvocation}.
 */
public class RpcInvocationTest extends TestBase {
    /**
     * The name of the test RPC.
     */
    private static final String  TEST_RPC_NAME = "test-rpc";

    /**
     * Test class that extends {@link RpcInvocation}.
     */
    private static final class TestRpc
        extends RpcInvocation<UpdateVtnInput, UpdateVtnOutput> {
        /**
         * Construct a new instance.
         *
         * @param in  The RPC input.
         * @param f   The future associated with the RPC execution with
         *            {@code in}.
         */
        private TestRpc(UpdateVtnInput in,
                        Future<RpcResult<UpdateVtnOutput>> f) {
            super(in, f);
        }

        /**
         * Return the name of the RPC.
         *
         * @return  {@link RpcInvocationTest#TEST_RPC_NAME}.
         */
        @Override
        public String getName() {
            return TEST_RPC_NAME;
        }
    }

    /**
     * Test case for {@link RpcInvocation#getInput()}.
     */
    @Test
    public void testGetInput() {
        UpdateVtnInput input = mock(UpdateVtnInput.class);
        TestRpc rpc = new TestRpc(input, null);
        assertSame(input, rpc.getInput());
    }

    /**
     * Test case for {@link RpcInvocation#getFuture()}.
     */
    @Test
    public void testGetFuture() {
        Future<RpcResult<UpdateVtnOutput>> future =
            SettableFuture.<RpcResult<UpdateVtnOutput>>create();
        TestRpc rpc = new TestRpc(null, future);
        assertSame(future, rpc.getFuture());
    }

    /**
     * Test case for {@link RpcInvocation#needErrorLog(Throwable)}.
     */
    @Test
    public void testNeedErrorLog1() {
        TestRpc rpc = new TestRpc(null, null);
        assertEquals(true, rpc.needErrorLog((Throwable)null));
    }

    /**
     * Test case for {@link RpcInvocation#needErrorLog(Collection)}.
     */
    @Test
    public void testNeedErrorLog2() {
        Collection<RpcError> errors = null;
        TestRpc rpc = new TestRpc(null, null);
        assertEquals(true, rpc.needErrorLog(errors));
    }

    /**
     * Test case for {@link RpcInvocation#getInputForLog()}.
     */
    @Test
    public void testGetInputForLog() {
        UpdateVtnInput input = mock(UpdateVtnInput.class);
        TestRpc rpc = new TestRpc(input, null);
        assertSame(input, rpc.getInputForLog());
    }

    /**
     * Test case for {@link RpcInvocation#getResult(long,TimeUnit,Logger)}.
     *
     * <ul>
     *   <li>RPC completed successfully.</li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testGetResult() throws Exception {
        UpdateVtnOutput output = mock(UpdateVtnOutput.class);
        final RpcResult<UpdateVtnOutput> result = RpcResultBuilder.
            success(output).build();
        final SettableFuture<RpcResult<UpdateVtnOutput>> future =
            SettableFuture.<RpcResult<UpdateVtnOutput>>create();

        UpdateVtnInput input = mock(UpdateVtnInput.class);
        TestRpc rpc = new TestRpc(input, future);
        Logger logger = mock(Logger.class);

        Thread t = new Thread() {
            @Override
            public void run() {
                try {
                    Thread.sleep(50);
                } catch (InterruptedException e) {
                }
                future.set(result);
            }
        };
        t.start();

        assertSame(output, rpc.getResult(10L, TimeUnit.SECONDS, logger));
        verifyZeroInteractions(logger);
    }

    /**
     * Test case for {@link RpcInvocation#getResult(long,TimeUnit,Logger)}.
     *
     * <ul>
     *   <li>RPC timed out.</li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testGetResultTimeout() throws Exception {
        SettableFuture<RpcResult<UpdateVtnOutput>> future =
            SettableFuture.<RpcResult<UpdateVtnOutput>>create();

        UpdateVtnInput input = mock(UpdateVtnInput.class);
        TestRpc rpc = new TestRpc(input, future);
        Logger logger = mock(Logger.class);

        Throwable cause = null;
        try {
            rpc.getResult(1L, TimeUnit.MILLISECONDS, logger);
            unexpected();
        } catch (VTNException e) {
            assertEquals(VtnErrorTag.TIMEOUT, e.getVtnErrorTag());
            cause = e.getCause();
            assertThat(cause, instanceOf(TimeoutException.class));
        }

        String msg = TEST_RPC_NAME + ": Caught an exception: canceled=true, " +
            "input=" + input;
        verify(logger).error(msg, cause);
        verifyNoMoreInteractions(logger);
    }
}
