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

import java.math.BigInteger;
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

import com.google.common.util.concurrent.Futures;
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

import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.statistics.rev130819.GetFlowStatisticsFromFlowTableInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.statistics.rev130819.GetFlowStatisticsFromFlowTableOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.statistics.rev130819.OpendaylightFlowStatisticsService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.transaction.rev150304.TransactionId;

/**
 * JUnit test for {@link GetFlowStatsRpc}.
 */
public class GetFlowStatsRpcTest extends TestBase {
    /**
     * The name of the get-flow-statistics-from-flow-table RPC.
     */
    private static final String  RPC_NAME = "get-flow-stats";

    /**
     * Test case for {@link GetFlowStatsRpc#getInput()} and
     * {@link GetFlowStatsRpc#getInputForLog()}.
     */
    @Test
    public void testGetInput() {
        OpendaylightFlowStatisticsService fss =
            mock(OpendaylightFlowStatisticsService.class);
        GetFlowStatisticsFromFlowTableInput input =
            mock(GetFlowStatisticsFromFlowTableInput.class);
        GetFlowStatsRpc rpc = new GetFlowStatsRpc(fss, input);
        assertSame(input, rpc.getInput());
        assertSame(input, rpc.getInputForLog());
    }

    /**
     * Test case for {@link GetFlowStatsRpc#getName()}.
     */
    @Test
    public void testGetName() {
        OpendaylightFlowStatisticsService fss =
            mock(OpendaylightFlowStatisticsService.class);
        GetFlowStatisticsFromFlowTableInput input =
            mock(GetFlowStatisticsFromFlowTableInput.class);
        GetFlowStatsRpc rpc = new GetFlowStatsRpc(fss, input);
        assertEquals(RPC_NAME, rpc.getName());
    }

    /**
     * Test case for {@link GetFlowStatsRpc#getFuture()}.
     */
    @Test
    public void testGetFuture() {
        OpendaylightFlowStatisticsService fss =
            mock(OpendaylightFlowStatisticsService.class);
        GetFlowStatisticsFromFlowTableInput input =
            mock(GetFlowStatisticsFromFlowTableInput.class);
        Future<RpcResult<GetFlowStatisticsFromFlowTableOutput>> future =
            SettableFuture.<RpcResult<GetFlowStatisticsFromFlowTableOutput>>
            create();
        when(fss.getFlowStatisticsFromFlowTable(input)).thenReturn(future);
        GetFlowStatsRpc rpc = new GetFlowStatsRpc(fss, input);
        assertSame(future, rpc.getFuture());
        verify(fss).getFlowStatisticsFromFlowTable(input);
        verifyNoMoreInteractions(fss, input);
    }

    /**
     * Test case for {@link GetFlowStatsRpc#needErrorLog(Throwable)}.
     */
    @Test
    public void testNeedErrorLog1() {
        OpendaylightFlowStatisticsService fss =
            mock(OpendaylightFlowStatisticsService.class);
        GetFlowStatisticsFromFlowTableInput input =
            mock(GetFlowStatisticsFromFlowTableInput.class);
        Future<RpcResult<GetFlowStatisticsFromFlowTableOutput>> future =
            SettableFuture.<RpcResult<GetFlowStatisticsFromFlowTableOutput>>
            create();
        when(fss.getFlowStatisticsFromFlowTable(input)).thenReturn(future);
        GetFlowStatsRpc rpc = new GetFlowStatsRpc(fss, input);

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
     * Test case for {@link GetFlowStatsRpc#needErrorLog(Collection)}.
     */
    @Test
    public void testNeedErrorLog2() {
        OpendaylightFlowStatisticsService fss =
            mock(OpendaylightFlowStatisticsService.class);
        GetFlowStatisticsFromFlowTableInput input =
            mock(GetFlowStatisticsFromFlowTableInput.class);
        Future<RpcResult<GetFlowStatisticsFromFlowTableOutput>> future =
            SettableFuture.<RpcResult<GetFlowStatisticsFromFlowTableOutput>>
            create();
        when(fss.getFlowStatisticsFromFlowTable(input)).thenReturn(future);
        GetFlowStatsRpc rpc = new GetFlowStatsRpc(fss, input);

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
     * Test case for {@link GetFlowStatsRpc#getResult(long,TimeUnit,Logger)}.
     *
     * <ul>
     *   <li>RPC completed successfully.</li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testGetResult() throws Exception {
        OpendaylightFlowStatisticsService fss =
            mock(OpendaylightFlowStatisticsService.class);
        GetFlowStatisticsFromFlowTableInput input =
            mock(GetFlowStatisticsFromFlowTableInput.class);
        GetFlowStatisticsFromFlowTableOutput output =
            mock(GetFlowStatisticsFromFlowTableOutput.class);
        RpcResult<GetFlowStatisticsFromFlowTableOutput> result =
            RpcResultBuilder.success(output).build();
        Future<RpcResult<GetFlowStatisticsFromFlowTableOutput>> future =
            Futures.<RpcResult<GetFlowStatisticsFromFlowTableOutput>>
            immediateFuture(result);
        when(fss.getFlowStatisticsFromFlowTable(input)).thenReturn(future);
        Logger logger = mock(Logger.class);

        GetFlowStatsRpc rpc = new GetFlowStatsRpc(fss, input);
        assertSame(output, rpc.getResult(1L, TimeUnit.SECONDS, logger));
        verifyZeroInteractions(logger);
    }

    /**
     * Test case for {@link GetFlowStatsRpc#getResult(long,TimeUnit,Logger)}.
     *
     * <ul>
     *   <li>RPC timed out.</li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testGetResultTimeout() throws Exception {
        OpendaylightFlowStatisticsService fss =
            mock(OpendaylightFlowStatisticsService.class);
        GetFlowStatisticsFromFlowTableInput input =
            mock(GetFlowStatisticsFromFlowTableInput.class);
        GetFlowStatisticsFromFlowTableOutput output =
            mock(GetFlowStatisticsFromFlowTableOutput.class);
        Future<RpcResult<GetFlowStatisticsFromFlowTableOutput>> future =
            SettableFuture.<RpcResult<GetFlowStatisticsFromFlowTableOutput>>
            create();
        when(fss.getFlowStatisticsFromFlowTable(input)).thenReturn(future);
        GetFlowStatsRpc rpc = new GetFlowStatsRpc(fss, input);
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
     * Test case for {@link GetFlowStatsRpc#getResult(long,TimeUnit,Logger)}.
     *
     * <ul>
     *   <li>No RPC implementation.</li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testGetResultNoRpc() throws Exception {
        OpendaylightFlowStatisticsService fss =
            mock(OpendaylightFlowStatisticsService.class);
        GetFlowStatisticsFromFlowTableInput input =
            mock(GetFlowStatisticsFromFlowTableInput.class);
        GetFlowStatisticsFromFlowTableOutput output =
            mock(GetFlowStatisticsFromFlowTableOutput.class);
        DOMRpcImplementationNotAvailableException cause =
            new DOMRpcImplementationNotAvailableException("No implementation");
        Future<RpcResult<GetFlowStatisticsFromFlowTableOutput>> future =
            Futures.<RpcResult<GetFlowStatisticsFromFlowTableOutput>>
            immediateFailedFuture(cause);
        when(fss.getFlowStatisticsFromFlowTable(input)).thenReturn(future);
        Logger logger = mock(Logger.class);

        GetFlowStatsRpc rpc = new GetFlowStatsRpc(fss, input);
        try {
            rpc.getResult(1L, TimeUnit.SECONDS, logger);
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
     * Test case for {@link GetFlowStatsRpc#getResult(long,TimeUnit,Logger)}.
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
            OpendaylightFlowStatisticsService fss =
                mock(OpendaylightFlowStatisticsService.class);
            GetFlowStatisticsFromFlowTableInput input =
                mock(GetFlowStatisticsFromFlowTableInput.class);
            RpcResult<GetFlowStatisticsFromFlowTableOutput> result =
                RpcResultBuilder.<GetFlowStatisticsFromFlowTableOutput>failed().
                withError(PROTOCOL, "Protocol error").
                withError(APPLICATION, emsg).
                build();
            Future<RpcResult<GetFlowStatisticsFromFlowTableOutput>> future =
                Futures.<RpcResult<GetFlowStatisticsFromFlowTableOutput>>
                immediateFuture(result);
            when(fss.getFlowStatisticsFromFlowTable(input)).thenReturn(future);
            Logger logger = mock(Logger.class);

            GetFlowStatsRpc rpc = new GetFlowStatsRpc(fss, input);
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
     * Test case for {@link GetFlowStatsRpc#getResult(long,TimeUnit,Logger)}.
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
            OpendaylightFlowStatisticsService fss =
                mock(OpendaylightFlowStatisticsService.class);
            GetFlowStatisticsFromFlowTableInput input =
                mock(GetFlowStatisticsFromFlowTableInput.class);
            RpcResult<GetFlowStatisticsFromFlowTableOutput> result =
                RpcResultBuilder.<GetFlowStatisticsFromFlowTableOutput>failed().
                withError(PROTOCOL, "Protocol error").
                withError(APPLICATION, emsg, ise).
                build();
            Future<RpcResult<GetFlowStatisticsFromFlowTableOutput>> future =
                Futures.<RpcResult<GetFlowStatisticsFromFlowTableOutput>>
                immediateFuture(result);
            when(fss.getFlowStatisticsFromFlowTable(input)).thenReturn(future);
            Logger logger = mock(Logger.class);

            GetFlowStatsRpc rpc = new GetFlowStatsRpc(fss, input);
            String msg = "RPC returned error";
            try {
                rpc.getResult(1L, TimeUnit.SECONDS, logger);
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

    /**
     * Test case for
     * {@link GetFlowStatsRpc#getTransactionId(long,TimeUnit,Logger)}.
     *
     * <ul>
     *   <li>RPC completed successfully.</li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testGetTransactionId() throws Exception {
        OpendaylightFlowStatisticsService fss =
            mock(OpendaylightFlowStatisticsService.class);
        GetFlowStatisticsFromFlowTableInput input =
            mock(GetFlowStatisticsFromFlowTableInput.class);
        GetFlowStatisticsFromFlowTableOutput output =
            mock(GetFlowStatisticsFromFlowTableOutput.class);
        BigInteger xid = BigInteger.valueOf(System.currentTimeMillis());
        TransactionId txId = new TransactionId(xid);
        when(output.getTransactionId()).thenReturn(txId);
        final SettableFuture<RpcResult<GetFlowStatisticsFromFlowTableOutput>> future =
            SettableFuture.<RpcResult<GetFlowStatisticsFromFlowTableOutput>>
            create();
        when(fss.getFlowStatisticsFromFlowTable(input)).thenReturn(future);
        GetFlowStatsRpc rpc = new GetFlowStatsRpc(fss, input);

        final RpcResult<GetFlowStatisticsFromFlowTableOutput> result =
            RpcResultBuilder.success(output).build();
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

        assertEquals(xid, rpc.getTransactionId(10L, TimeUnit.SECONDS, logger));
        verifyZeroInteractions(logger);
    }

    /**
     * Test case for
     * {@link GetFlowStatsRpc#getTransactionId(long,TimeUnit,Logger)}.
     *
     * <ul>
     *   <li>RPC returned {@code null}.</li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testGetTransactionNoOutput() throws Exception {
        OpendaylightFlowStatisticsService fss =
            mock(OpendaylightFlowStatisticsService.class);
        GetFlowStatisticsFromFlowTableInput input =
            mock(GetFlowStatisticsFromFlowTableInput.class);
        GetFlowStatisticsFromFlowTableOutput output = null;
        final SettableFuture<RpcResult<GetFlowStatisticsFromFlowTableOutput>> future =
            SettableFuture.<RpcResult<GetFlowStatisticsFromFlowTableOutput>>
            create();
        when(fss.getFlowStatisticsFromFlowTable(input)).thenReturn(future);
        GetFlowStatsRpc rpc = new GetFlowStatsRpc(fss, input);

        final RpcResult<GetFlowStatisticsFromFlowTableOutput> result =
            RpcResultBuilder.success(output).build();
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

        String msg = "Flow stats RPC did not return the result";
        try {
            rpc.getTransactionId(10L, TimeUnit.SECONDS, logger);
            unexpected();
        } catch (VTNException e) {
            assertEquals(VtnErrorTag.INTERNALERROR, e.getVtnErrorTag());
            assertEquals(msg, e.getMessage());
            assertEquals(null, e.getCause());
        }

        verify(logger).error("{}: {}", RPC_NAME, msg);
        verifyNoMoreInteractions(logger);
    }

    /**
     * Test case for
     * {@link GetFlowStatsRpc#getTransactionId(long,TimeUnit,Logger)}.
     *
     * <ul>
     *   <li>No transaction ID in RPC output.</li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testGetTransactionNoXid() throws Exception {
        OpendaylightFlowStatisticsService fss =
            mock(OpendaylightFlowStatisticsService.class);
        GetFlowStatisticsFromFlowTableInput input =
            mock(GetFlowStatisticsFromFlowTableInput.class);
        GetFlowStatisticsFromFlowTableOutput output =
            mock(GetFlowStatisticsFromFlowTableOutput.class);
        final SettableFuture<RpcResult<GetFlowStatisticsFromFlowTableOutput>> future =
            SettableFuture.<RpcResult<GetFlowStatisticsFromFlowTableOutput>>
            create();
        when(fss.getFlowStatisticsFromFlowTable(input)).thenReturn(future);
        GetFlowStatsRpc rpc = new GetFlowStatsRpc(fss, input);

        final RpcResult<GetFlowStatisticsFromFlowTableOutput> result =
            RpcResultBuilder.success(output).build();
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

        String msg = "Flow stats RPC did not return XID";
        try {
            rpc.getTransactionId(10L, TimeUnit.SECONDS, logger);
            unexpected();
        } catch (VTNException e) {
            assertEquals(VtnErrorTag.INTERNALERROR, e.getVtnErrorTag());
            assertEquals(msg, e.getMessage());
            assertEquals(null, e.getCause());
        }

        verify(logger).error("{}: {}: {}", RPC_NAME, msg, output);
        verifyNoMoreInteractions(logger);
    }
}
