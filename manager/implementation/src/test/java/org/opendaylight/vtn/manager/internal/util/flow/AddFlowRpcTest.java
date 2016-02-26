/*
 * Copyright (c) 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.flow;

import static org.mockito.Mockito.eq;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.verifyNoMoreInteractions;
import static org.mockito.Mockito.verifyZeroInteractions;
import static org.mockito.Mockito.when;

import static org.hamcrest.CoreMatchers.instanceOf;

import static org.opendaylight.yangtools.yang.common.RpcError.ErrorType.APPLICATION;
import static org.opendaylight.yangtools.yang.common.RpcError.ErrorType.PROTOCOL;

import java.util.Arrays;
import java.util.Collection;
import java.util.HashMap;
import java.util.List;
import java.util.Map.Entry;
import java.util.Map;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.Future;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.TimeoutException;

import com.google.common.util.concurrent.SettableFuture;

import org.opendaylight.controller.md.sal.dom.api.DOMRpcImplementationNotAvailableException;

import org.junit.Test;

import org.mockito.ArgumentCaptor;

import org.slf4j.Logger;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.TestBase;

import org.opendaylight.yangtools.yang.common.RpcError;
import org.opendaylight.yangtools.yang.common.RpcResult;
import org.opendaylight.yangtools.yang.common.RpcResultBuilder;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnErrorTag;

import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.service.rev130819.AddFlowInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.service.rev130819.AddFlowOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.service.rev130819.SalFlowService;

/**
 * JUnit test for {@link AddFlowRpc}.
 */
public class AddFlowRpcTest extends TestBase {
    /**
     * The name of the add-flow RPC.
     */
    private static final String  RPC_NAME = "add-flow";

    /**
     * Test case for {@link AddFlowRpc#getInput()} and
     * {@link AddFlowRpc#getInputForLog()}.
     */
    @Test
    public void testGetInput() {
        SalFlowService sfs = mock(SalFlowService.class);
        AddFlowInput input = mock(AddFlowInput.class);
        AddFlowRpc rpc = new AddFlowRpc(sfs, input);
        assertSame(input, rpc.getInput());
        assertSame(input, rpc.getInputForLog());
    }

    /**
     * Test case for {@link AddFlowRpc#getName()}.
     */
    @Test
    public void testGetName() {
        SalFlowService sfs = mock(SalFlowService.class);
        AddFlowInput input = mock(AddFlowInput.class);
        AddFlowRpc rpc = new AddFlowRpc(sfs, input);
        assertEquals(RPC_NAME, rpc.getName());
    }

    /**
     * Test case for {@link AddFlowRpc#getFuture()}.
     */
    @Test
    public void testGetFuture() {
        SalFlowService sfs = mock(SalFlowService.class);
        AddFlowInput input = mock(AddFlowInput.class);
        Future<RpcResult<AddFlowOutput>> future =
            SettableFuture.<RpcResult<AddFlowOutput>>create();
        when(sfs.addFlow(input)).thenReturn(future);
        AddFlowRpc rpc = new AddFlowRpc(sfs, input);
        assertSame(future, rpc.getFuture());
        verify(sfs).addFlow(input);
        verifyNoMoreInteractions(sfs, input);
    }

    /**
     * Test case for {@link AddFlowRpc#needErrorLog(Throwable)}.
     */
    @Test
    public void testNeedErrorLog1() {
        SalFlowService sfs = mock(SalFlowService.class);
        AddFlowInput input = mock(AddFlowInput.class);
        Future<RpcResult<AddFlowOutput>> future =
            SettableFuture.<RpcResult<AddFlowOutput>>create();
        when(sfs.addFlow(input)).thenReturn(future);
        AddFlowRpc rpc = new AddFlowRpc(sfs, input);

        Map<Throwable, Boolean> cases = new HashMap<>();
        assertNull(cases.put(new Throwable(), true));
        assertNull(cases.put(new Exception(), true));
        assertNull(cases.put(new IllegalArgumentException(), true));
        assertNull(cases.put(new IllegalStateException(), true));

        Exception e = new NumberFormatException();
        for (int i = 0; i < 5; i++) {
            e = new ExecutionException(e);
        }
        assertNull(cases.put(e, true));

        e = new DOMRpcImplementationNotAvailableException("error 1");
        assertNull(cases.put(e, true));

        e = new DOMRpcImplementationNotAvailableException("error 2");
        for (int i = 0; i < 5; i++) {
            e = new ExecutionException(e);
        }
        assertNull(cases.put(e, true));

        for (Entry<Throwable, Boolean> entry: cases.entrySet()) {
            Throwable cause = entry.getKey();
            boolean expected = entry.getValue().booleanValue();
            assertEquals(expected, rpc.needErrorLog(cause));
        }
    }

    /**
     * Test case for {@link AddFlowRpc#needErrorLog(Collection)}.
     */
    @Test
    public void testNeedErrorLog2() {
        SalFlowService sfs = mock(SalFlowService.class);
        AddFlowInput input = mock(AddFlowInput.class);
        Future<RpcResult<AddFlowOutput>> future =
            SettableFuture.<RpcResult<AddFlowOutput>>create();
        when(sfs.addFlow(input)).thenReturn(future);
        AddFlowRpc rpc = new AddFlowRpc(sfs, input);

        Map<String, Boolean> cases = new HashMap<>();
        assertNull(cases.put(null, true));
        assertNull(cases.put("Unknown error", true));
        assertNull(cases.put("Operation timed out", true));
        assertNull(cases.put("Invalid input", true));
        assertNull(cases.put("Device disconnected", true));
        assertNull(cases.put("Outbound queue wasn't able to reserve XID.",
                             true));

        for (Entry<String, Boolean> entry: cases.entrySet()) {
            String msg = entry.getKey();
            boolean expected = entry.getValue().booleanValue();
            RpcError err = mock(RpcError.class);
            when(err.getMessage()).thenReturn(msg);
            Collection<RpcError> errors = Arrays.asList(
                mock(RpcError.class), mock(RpcError.class), err);
            assertEquals(expected, rpc.needErrorLog(errors));
        }
    }

    /**
     * Test case for {@link AddFlowRpc#getResult(long,TimeUnit,Logger)}.
     *
     * <ul>
     *   <li>RPC completed successfully.</li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testGetResult() throws Exception {
        SalFlowService sfs = mock(SalFlowService.class);
        AddFlowInput input = mock(AddFlowInput.class);
        AddFlowOutput output = mock(AddFlowOutput.class);
        final SettableFuture<RpcResult<AddFlowOutput>> future =
            SettableFuture.<RpcResult<AddFlowOutput>>create();
        when(sfs.addFlow(input)).thenReturn(future);
        AddFlowRpc rpc = new AddFlowRpc(sfs, input);

        final RpcResult<AddFlowOutput> result = RpcResultBuilder.
            success(output).build();
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
     * Test case for {@link AddFlowRpc#getResult(long,TimeUnit,Logger)}.
     *
     * <ul>
     *   <li>RPC timed out.</li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testGetResultTimeout() throws Exception {
        SalFlowService sfs = mock(SalFlowService.class);
        AddFlowInput input = mock(AddFlowInput.class);
        AddFlowOutput output = mock(AddFlowOutput.class);
        Future<RpcResult<AddFlowOutput>> future =
            SettableFuture.<RpcResult<AddFlowOutput>>create();
        when(sfs.addFlow(input)).thenReturn(future);
        AddFlowRpc rpc = new AddFlowRpc(sfs, input);
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

        String msg = RPC_NAME + ": Caught an exception: canceled=true, " +
            "input=" + input;
        verify(logger).error(msg, cause);
        verifyNoMoreInteractions(logger);
    }

    /**
     * Test case for {@link AddFlowRpc#getResult(long,TimeUnit,Logger)}.
     *
     * <ul>
     *   <li>No RPC implementation.</li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testGetResultNoRpc() throws Exception {
        SalFlowService sfs = mock(SalFlowService.class);
        AddFlowInput input = mock(AddFlowInput.class);
        AddFlowOutput output = mock(AddFlowOutput.class);
        final DOMRpcImplementationNotAvailableException cause =
            new DOMRpcImplementationNotAvailableException("No implementation");
        final SettableFuture<RpcResult<AddFlowOutput>> future =
            SettableFuture.<RpcResult<AddFlowOutput>>create();
        when(sfs.addFlow(input)).thenReturn(future);
        AddFlowRpc rpc = new AddFlowRpc(sfs, input);
        Logger logger = mock(Logger.class);

        Thread t = new Thread() {
            @Override
            public void run() {
                try {
                    Thread.sleep(50);
                } catch (InterruptedException e) {
                }
                future.setException(cause);
            }
        };
        t.start();

        try {
            rpc.getResult(10L, TimeUnit.SECONDS, logger);
            unexpected();
        } catch (VTNException e) {
            assertEquals(VtnErrorTag.INTERNALERROR, e.getVtnErrorTag());
            assertEquals(cause.getMessage(), e.getMessage());
            assertEquals(cause, e.getCause());
        }

        String msg = RPC_NAME + ": Caught an exception: canceled=false, " +
            "input=" + input;
        ArgumentCaptor<Throwable> captor =
            ArgumentCaptor.forClass(Throwable.class);
        verify(logger).error(eq(msg), captor.capture());
        List<Throwable> causes = captor.getAllValues();
        assertEquals(1, causes.size());
        Throwable c = causes.get(0);
        assertThat(c, instanceOf(ExecutionException.class));
        assertEquals(cause, c.getCause());
        verifyNoMoreInteractions(logger);
    }

    /**
     * Test case for {@link AddFlowRpc#getResult(long,TimeUnit,Logger)}.
     *
     * <ul>
     *   <li>RPC failed.</li>
     *   <li>The result does not contain any {@link Throwable}.</li>
     *   <li>The result should be logged.</li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testGetResultLog1() throws Exception {
        String[] msgs = {
            null,
            "Unknown error.",
            "Invalid input.",
            "Device disconnected",
            "Outbound queue wasn't able to reserve XID.",
        };

        for (String emsg: msgs) {
            SalFlowService sfs = mock(SalFlowService.class);
            AddFlowInput input = mock(AddFlowInput.class);
            final SettableFuture<RpcResult<AddFlowOutput>> future =
                SettableFuture.<RpcResult<AddFlowOutput>>create();
            when(sfs.addFlow(input)).thenReturn(future);
            final RpcResult<AddFlowOutput> result =
                RpcResultBuilder.<AddFlowOutput>failed().
                withError(PROTOCOL, "Protocol error").
                withError(APPLICATION, emsg).
                build();
            AddFlowRpc rpc = new AddFlowRpc(sfs, input);
            Logger logger = mock(Logger.class);

            Thread t = new Thread() {
                @Override
                public void run() {
                    future.set(result);
                }
            };
            t.start();

            String msg = "RPC returned error";
            try {
                rpc.getResult(10L, TimeUnit.SECONDS, logger);
                unexpected();
            } catch (VTNException e) {
                assertEquals(VtnErrorTag.INTERNALERROR, e.getVtnErrorTag());
                assertEquals(msg, e.getMessage());
                assertEquals(null, e.getCause());
            }

            verify(logger).
                error("{}: {}: input={}, errors={}", RPC_NAME, msg, input,
                      result.getErrors());
            verifyNoMoreInteractions(logger);
        }
    }

    /**
     * Test case for {@link AddFlowRpc#getResult(long,TimeUnit,Logger)}.
     *
     * <ul>
     *   <li>RPC failed.</li>
     *   <li>The result contains an {@link IllegalStateException}.</li>
     *   <li>The result should be logged.</li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testGetResultLog2() throws Exception {
        String[] msgs = {
            null,
            "Unknown error.",
            "Invalid input.",
            "Device disconnected",
            "Outbound queue wasn't able to reserve XID.",
        };
        IllegalStateException ise = new IllegalStateException();

        for (String emsg: msgs) {
            SalFlowService sfs = mock(SalFlowService.class);
            AddFlowInput input = mock(AddFlowInput.class);
            final SettableFuture<RpcResult<AddFlowOutput>> future =
                SettableFuture.<RpcResult<AddFlowOutput>>create();
            when(sfs.addFlow(input)).thenReturn(future);
            final RpcResult<AddFlowOutput> result =
                RpcResultBuilder.<AddFlowOutput>failed().
                withError(PROTOCOL, "Protocol error").
                withError(APPLICATION, emsg, ise).
                build();
            AddFlowRpc rpc = new AddFlowRpc(sfs, input);
            Logger logger = mock(Logger.class);

            Thread t = new Thread() {
                @Override
                public void run() {
                    future.set(result);
                }
            };
            t.start();

            String msg = "RPC returned error";
            try {
                rpc.getResult(10L, TimeUnit.SECONDS, logger);
                unexpected();
            } catch (VTNException e) {
                assertEquals(VtnErrorTag.INTERNALERROR, e.getVtnErrorTag());
                assertEquals(msg, e.getMessage());
                assertEquals(null, e.getCause());
            }

            String lmsg = RPC_NAME + ": RPC returned error: input=" + input +
                ", errors=" + result.getErrors();
            verify(logger).error(lmsg, ise);
            verifyNoMoreInteractions(logger);
        }
    }
}
