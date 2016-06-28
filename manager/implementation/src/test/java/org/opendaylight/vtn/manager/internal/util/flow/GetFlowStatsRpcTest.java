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
import java.util.concurrent.CancellationException;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.Future;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.TimeoutException;

import com.google.common.util.concurrent.Futures;
import com.google.common.util.concurrent.SettableFuture;

import org.junit.Test;

import org.mockito.ArgumentCaptor;

import org.slf4j.Logger;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.util.inventory.NodeRpcWatcher;
import org.opendaylight.vtn.manager.internal.util.inventory.SalNode;

import org.opendaylight.vtn.manager.internal.TestBase;

import org.opendaylight.controller.md.sal.dom.api.DOMRpcImplementationNotAvailableException;

import org.opendaylight.yangtools.yang.common.RpcError;
import org.opendaylight.yangtools.yang.common.RpcResult;
import org.opendaylight.yangtools.yang.common.RpcResultBuilder;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnErrorTag;

import org.opendaylight.yang.gen.v1.urn.opendaylight.direct.statistics.rev160511.GetFlowStatisticsInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.direct.statistics.rev160511.GetFlowStatisticsOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.direct.statistics.rev160511.OpendaylightDirectStatisticsService;

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
        SalNode snode = new SalNode(1L);
        GetFlowStatisticsInput input =
            mock(GetFlowStatisticsInput.class);
        when(input.getNode()).thenReturn(snode.getNodeRef());
        NodeRpcWatcher watcher = mock(NodeRpcWatcher.class);
        OpendaylightDirectStatisticsService dss =
            mock(OpendaylightDirectStatisticsService.class);
        GetFlowStatsRpc rpc = new GetFlowStatsRpc(watcher, dss, input);
        assertSame(input, rpc.getInput());
        assertSame(input, rpc.getInputForLog());
    }

    /**
     * Test case for {@link GetFlowStatsRpc#getName()}.
     */
    @Test
    public void testGetName() {
        SalNode snode = new SalNode(1L);
        GetFlowStatisticsInput input =
            mock(GetFlowStatisticsInput.class);
        when(input.getNode()).thenReturn(snode.getNodeRef());
        NodeRpcWatcher watcher = mock(NodeRpcWatcher.class);
        OpendaylightDirectStatisticsService dss =
            mock(OpendaylightDirectStatisticsService.class);
        GetFlowStatsRpc rpc = new GetFlowStatsRpc(watcher, dss, input);
        assertEquals(RPC_NAME, rpc.getName());
    }

    /**
     * Test case for {@link GetFlowStatsRpc#getFuture()}.
     */
    @Test
    public void testGetFuture() {
        SalNode snode = new SalNode(1L);
        GetFlowStatisticsInput input =
            mock(GetFlowStatisticsInput.class);
        when(input.getNode()).thenReturn(snode.getNodeRef());
        NodeRpcWatcher watcher = mock(NodeRpcWatcher.class);
        OpendaylightDirectStatisticsService dss =
            mock(OpendaylightDirectStatisticsService.class);
        Future<RpcResult<GetFlowStatisticsOutput>> future =
            SettableFuture.<RpcResult<GetFlowStatisticsOutput>>
            create();
        when(dss.getFlowStatistics(input)).thenReturn(future);
        GetFlowStatsRpc rpc = new GetFlowStatsRpc(watcher, dss, input);
        assertSame(future, rpc.getFuture());
        verify(dss).getFlowStatistics(input);
        verify(input).getNode();
        verifyNoMoreInteractions(watcher, dss, input);
    }

    /**
     * Test case for {@link GetFlowStatsRpc#needErrorLog(Throwable)}.
     *
     * <ul>
     *   <li>The target node is not removed.</li>
     * </ul>
     */
    @Test
    public void testNeedErrorLog1() {
        Map<Throwable, Boolean> cases = new HashMap<>();
        assertNull(cases.put(new Throwable(), false));
        assertNull(cases.put(new Exception(), false));
        assertNull(cases.put(new IllegalArgumentException(), false));
        assertNull(cases.put(new IllegalStateException(), false));

        Exception e = new NumberFormatException();
        for (int i = 0; i < 5; i++) {
            e = new ExecutionException(e);
        }
        assertNull(cases.put(e, false));

        e = new DOMRpcImplementationNotAvailableException("error 1");
        assertNull(cases.put(e, true));

        e = new DOMRpcImplementationNotAvailableException("error 2");
        for (int i = 0; i < 5; i++) {
            e = new ExecutionException(e);
        }
        assertNull(cases.put(e, true));

        SalNode snode = new SalNode(1L);
        for (Entry<Throwable, Boolean> entry: cases.entrySet()) {
            Throwable cause = entry.getKey();
            boolean disconnected = entry.getValue().booleanValue();

            GetFlowStatisticsInput input =
                mock(GetFlowStatisticsInput.class);
            when(input.getNode()).thenReturn(snode.getNodeRef());
            NodeRpcWatcher watcher = mock(NodeRpcWatcher.class);
            OpendaylightDirectStatisticsService dss =
                mock(OpendaylightDirectStatisticsService.class);
            Future<RpcResult<GetFlowStatisticsOutput>> future =
                SettableFuture.<RpcResult<GetFlowStatisticsOutput>>
                create();
            when(dss.getFlowStatistics(input)).thenReturn(future);
            GetFlowStatsRpc rpc = new GetFlowStatsRpc(watcher, dss, input);
            assertEquals(false, rpc.isDisconnected());

            assertEquals(true, rpc.needErrorLog(cause));
            assertEquals(disconnected, rpc.isDisconnected());
            assertEquals(false, rpc.isNodeRemoved());
            assertEquals(false, future.isCancelled());
        }
    }

    /**
     * Test case for {@link GetFlowStatsRpc#needErrorLog(Throwable)}.
     *
     * <ul>
     *   <li>The target node is removed.</li>
     * </ul>
     */
    @Test
    public void testNeedErrorLog2() {
        Map<Throwable, Boolean> cases = new HashMap<>();
        assertNull(cases.put(new Throwable(), false));
        assertNull(cases.put(new Exception(), false));
        assertNull(cases.put(new IllegalArgumentException(), false));
        assertNull(cases.put(new IllegalStateException(), false));

        Exception e = new NumberFormatException();
        for (int i = 0; i < 5; i++) {
            e = new ExecutionException(e);
        }
        assertNull(cases.put(e, false));

        e = new DOMRpcImplementationNotAvailableException("error 1");
        assertNull(cases.put(e, true));

        e = new DOMRpcImplementationNotAvailableException("error 2");
        for (int i = 0; i < 5; i++) {
            e = new ExecutionException(e);
        }
        assertNull(cases.put(e, true));

        SalNode snode = new SalNode(1L);
        for (Entry<Throwable, Boolean> entry: cases.entrySet()) {
            Throwable cause = entry.getKey();
            boolean disconnected = entry.getValue().booleanValue();

            GetFlowStatisticsInput input =
                mock(GetFlowStatisticsInput.class);
            when(input.getNode()).thenReturn(snode.getNodeRef());
            NodeRpcWatcher watcher = mock(NodeRpcWatcher.class);
            OpendaylightDirectStatisticsService dss =
                mock(OpendaylightDirectStatisticsService.class);
            Future<RpcResult<GetFlowStatisticsOutput>> future =
                SettableFuture.<RpcResult<GetFlowStatisticsOutput>>
                create();
            when(dss.getFlowStatistics(input)).thenReturn(future);
            GetFlowStatsRpc rpc = new GetFlowStatsRpc(watcher, dss, input);
            assertEquals(false, rpc.isDisconnected());
            assertEquals(false, future.isCancelled());
            assertEquals(true, rpc.onNodeRemoved());
            assertEquals(true, rpc.isNodeRemoved());
            assertEquals(true, future.isCancelled());

            assertEquals(false, rpc.needErrorLog(cause));
            assertEquals(disconnected, rpc.isDisconnected());

            for (int i = 0; i < 5; i++) {
                assertEquals(false, rpc.onNodeRemoved());
                assertEquals(true, rpc.isNodeRemoved());
                assertEquals(true, future.isCancelled());
            }
        }
    }

    /**
     * Test case for {@link GetFlowStatsRpc#needErrorLog(Collection)}.
     */
    @Test
    public void testNeedErrorLog3() {
        Map<String, Boolean> cases = new HashMap<>();
        assertNull(cases.put(null, false));
        assertNull(cases.put("Unknown error", false));
        assertNull(cases.put("Operation timed out", false));
        assertNull(cases.put("Invalid input", false));
        assertNull(cases.put("Device disconnected", true));
        assertNull(cases.put("Outbound queue wasn't able to reserve XID.",
                             true));

        SalNode snode = new SalNode(1L);
        for (Entry<String, Boolean> entry: cases.entrySet()) {
            String msg = entry.getKey();
            boolean disconnected = entry.getValue().booleanValue();

            GetFlowStatisticsInput input =
                mock(GetFlowStatisticsInput.class);
            when(input.getNode()).thenReturn(snode.getNodeRef());
            NodeRpcWatcher watcher = mock(NodeRpcWatcher.class);
            OpendaylightDirectStatisticsService dss =
                mock(OpendaylightDirectStatisticsService.class);
            Future<RpcResult<GetFlowStatisticsOutput>> future =
                SettableFuture.<RpcResult<GetFlowStatisticsOutput>>
                create();
            when(dss.getFlowStatistics(input)).thenReturn(future);
            GetFlowStatsRpc rpc = new GetFlowStatsRpc(watcher, dss, input);
            assertEquals(false, rpc.isDisconnected());

            RpcError err = mock(RpcError.class);
            when(err.getMessage()).thenReturn(msg);
            Collection<RpcError> errors = Arrays.asList(
                mock(RpcError.class), mock(RpcError.class), err);
            assertEquals(true, rpc.needErrorLog(errors));
            assertEquals(disconnected, rpc.isDisconnected());
            assertEquals(false, rpc.isNodeRemoved());
            assertEquals(false, future.isCancelled());
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
        SalNode snode = new SalNode(1L);
        GetFlowStatisticsInput input =
            mock(GetFlowStatisticsInput.class);
        when(input.getNode()).thenReturn(snode.getNodeRef());
        NodeRpcWatcher watcher = mock(NodeRpcWatcher.class);
        OpendaylightDirectStatisticsService dss =
            mock(OpendaylightDirectStatisticsService.class);
        GetFlowStatisticsOutput output =
            mock(GetFlowStatisticsOutput.class);
        RpcResult<GetFlowStatisticsOutput> result =
            RpcResultBuilder.success(output).build();
        Future<RpcResult<GetFlowStatisticsOutput>> future =
            Futures.<RpcResult<GetFlowStatisticsOutput>>
            immediateFuture(result);
        when(dss.getFlowStatistics(input)).thenReturn(future);
        Logger logger = mock(Logger.class);

        GetFlowStatsRpc rpc = new GetFlowStatsRpc(watcher, dss, input);
        assertSame(output, rpc.getResult(1L, TimeUnit.SECONDS, logger));
        assertEquals(false, rpc.isNodeRemoved());
        assertEquals(false, rpc.isDisconnected());

        verify(watcher).registerRpc(rpc);
        verify(watcher).unregisterRpc(rpc);
        verifyNoMoreInteractions(watcher, logger);
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
        SalNode snode = new SalNode(1L);
        GetFlowStatisticsInput input =
            mock(GetFlowStatisticsInput.class);
        when(input.getNode()).thenReturn(snode.getNodeRef());
        NodeRpcWatcher watcher = mock(NodeRpcWatcher.class);
        OpendaylightDirectStatisticsService dss =
            mock(OpendaylightDirectStatisticsService.class);
        Future<RpcResult<GetFlowStatisticsOutput>> future =
            SettableFuture.<RpcResult<GetFlowStatisticsOutput>>
            create();
        when(dss.getFlowStatistics(input)).thenReturn(future);
        GetFlowStatsRpc rpc = new GetFlowStatsRpc(watcher, dss, input);
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

        assertEquals(false, rpc.isNodeRemoved());
        assertEquals(false, rpc.isDisconnected());

        String msg = RPC_NAME + ": Caught an exception: canceled=true, " +
            "input=" + input;
        verify(logger).error(msg, cause);
        verify(watcher).registerRpc(rpc);
        verify(watcher).unregisterRpc(rpc);
        verifyNoMoreInteractions(watcher, logger);
    }

    /**
     * Test case for {@link GetFlowStatsRpc#getResult(long,TimeUnit,Logger)}.
     *
     * <ul>
     *   <li>RPC was canceled.</li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testGetResultCancel() throws Exception {
        SalNode snode = new SalNode(1L);
        GetFlowStatisticsInput input =
            mock(GetFlowStatisticsInput.class);
        when(input.getNode()).thenReturn(snode.getNodeRef());
        NodeRpcWatcher watcher = mock(NodeRpcWatcher.class);
        OpendaylightDirectStatisticsService dss =
            mock(OpendaylightDirectStatisticsService.class);
        SettableFuture<RpcResult<GetFlowStatisticsOutput>> future =
            SettableFuture.<RpcResult<GetFlowStatisticsOutput>>
            create();
        when(dss.getFlowStatistics(input)).thenReturn(future);
        final GetFlowStatsRpc rpc = new GetFlowStatsRpc(watcher, dss, input);

        Logger logger = mock(Logger.class);

        Thread t = new Thread() {
            @Override
            public void run() {
                try {
                    Thread.sleep(50);
                } catch (InterruptedException e) {
                }
                rpc.onNodeRemoved();
            }
        };
        t.start();

        Throwable cause = null;
        try {
            rpc.getResult(10L, TimeUnit.SECONDS, logger);
            unexpected();
        } catch (VTNException e) {
            assertEquals(VtnErrorTag.INTERNALERROR, e.getVtnErrorTag());
            cause = e.getCause();
            assertThat(cause, instanceOf(CancellationException.class));
        }

        assertEquals(true, rpc.isNodeRemoved());
        assertEquals(false, rpc.isDisconnected());

        // No error should be logged.
        verify(watcher).registerRpc(rpc);
        verify(watcher).unregisterRpc(rpc);
        verifyNoMoreInteractions(watcher, logger);

        assertEquals(true, future.isCancelled());
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
        SalNode snode = new SalNode(1L);
        GetFlowStatisticsInput input =
            mock(GetFlowStatisticsInput.class);
        when(input.getNode()).thenReturn(snode.getNodeRef());
        NodeRpcWatcher watcher = mock(NodeRpcWatcher.class);
        OpendaylightDirectStatisticsService dss =
            mock(OpendaylightDirectStatisticsService.class);
        DOMRpcImplementationNotAvailableException cause =
            new DOMRpcImplementationNotAvailableException("No implementation");
        Future<RpcResult<GetFlowStatisticsOutput>> future =
            Futures.<RpcResult<GetFlowStatisticsOutput>>
            immediateFailedFuture(cause);
        when(dss.getFlowStatistics(input)).thenReturn(future);
        Logger logger = mock(Logger.class);

        GetFlowStatsRpc rpc = new GetFlowStatsRpc(watcher, dss, input);
        try {
            rpc.getResult(1L, TimeUnit.SECONDS, logger);
            unexpected();
        } catch (VTNException e) {
            assertEquals(VtnErrorTag.INTERNALERROR, e.getVtnErrorTag());
            assertEquals(cause.getMessage(), e.getMessage());
            assertEquals(cause, e.getCause());
        }

        assertEquals(false, rpc.isNodeRemoved());
        assertEquals(true, rpc.isDisconnected());

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
        verify(watcher).registerRpc(rpc);
        verify(watcher).unregisterRpc(rpc);
        verifyNoMoreInteractions(watcher, logger);

        assertEquals(false, future.isCancelled());
    }

    /**
     * Test case for {@link GetFlowStatsRpc#getResult(long,TimeUnit,Logger)}.
     *
     * <ul>
     *   <li>
     *     RPC implementation throws an {@link IllegalArgumentException}.
     *   </li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testGetResultException() throws Exception {
        SalNode snode = new SalNode(1L);
        GetFlowStatisticsInput input = mock(GetFlowStatisticsInput.class);
        when(input.getNode()).thenReturn(snode.getNodeRef());
        NodeRpcWatcher watcher = mock(NodeRpcWatcher.class);
        OpendaylightDirectStatisticsService dss =
            mock(OpendaylightDirectStatisticsService.class);
        IllegalArgumentException cause =
            new IllegalArgumentException("Invalid argument");
        when(dss.getFlowStatistics(input)).thenThrow(cause);
        Logger logger = mock(Logger.class);

        GetFlowStatsRpc rpc = new GetFlowStatsRpc(watcher, dss, input);
        try {
            rpc.getResult(1L, TimeUnit.SECONDS, logger);
            unexpected();
        } catch (VTNException e) {
            assertEquals(VtnErrorTag.INTERNALERROR, e.getVtnErrorTag());
            assertEquals(cause.getMessage(), e.getMessage());
            assertEquals(cause, e.getCause());
        }

        assertEquals(false, rpc.isNodeRemoved());
        assertEquals(false, rpc.isDisconnected());

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
        verify(watcher).registerRpc(rpc);
        verify(watcher).unregisterRpc(rpc);
        verifyNoMoreInteractions(watcher, logger);
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
        Map<String, Boolean> cases = new HashMap<>();
        assertNull(cases.put(null, false));
        assertNull(cases.put("Unknown error", false));
        assertNull(cases.put("Operation timed out", false));
        assertNull(cases.put("Invalid input", false));
        assertNull(cases.put("Device disconnected", true));
        assertNull(cases.put("Outbound queue wasn't able to reserve XID.",
                             true));
        SalNode snode = new SalNode(1L);
        for (Entry<String, Boolean> entry: cases.entrySet()) {
            String emsg = entry.getKey();
            boolean disconnected = entry.getValue().booleanValue();

            GetFlowStatisticsInput input =
                mock(GetFlowStatisticsInput.class);
            when(input.getNode()).thenReturn(snode.getNodeRef());
            NodeRpcWatcher watcher = mock(NodeRpcWatcher.class);
            OpendaylightDirectStatisticsService dss =
                mock(OpendaylightDirectStatisticsService.class);
            RpcResult<GetFlowStatisticsOutput> result =
                RpcResultBuilder.<GetFlowStatisticsOutput>failed().
                withError(PROTOCOL, "Protocol error").
                withError(APPLICATION, emsg).
                build();
            Future<RpcResult<GetFlowStatisticsOutput>> future =
                Futures.<RpcResult<GetFlowStatisticsOutput>>
                immediateFuture(result);
            when(dss.getFlowStatistics(input)).thenReturn(future);
            Logger logger = mock(Logger.class);

            GetFlowStatsRpc rpc = new GetFlowStatsRpc(watcher, dss, input);
            String msg = "RPC returned error";
            try {
                rpc.getResult(10L, TimeUnit.SECONDS, logger);
                unexpected();
            } catch (VTNException e) {
                assertEquals(VtnErrorTag.INTERNALERROR, e.getVtnErrorTag());
                assertEquals(msg, e.getMessage());
                assertEquals(null, e.getCause());
            }

            assertEquals(false, rpc.isNodeRemoved());
            assertEquals(disconnected, rpc.isDisconnected());

            verify(logger).
                error("{}: {}: input={}, errors={}", RPC_NAME, msg, input,
                      result.getErrors());
            verify(watcher).registerRpc(rpc);
            verify(watcher).unregisterRpc(rpc);
            verifyNoMoreInteractions(watcher, logger);

            assertEquals(false, future.isCancelled());
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
        Map<String, Boolean> cases = new HashMap<>();
        assertNull(cases.put(null, false));
        assertNull(cases.put("Unknown error", false));
        assertNull(cases.put("Operation timed out", false));
        assertNull(cases.put("Invalid input", false));
        assertNull(cases.put("Device disconnected", true));
        assertNull(cases.put("Outbound queue wasn't able to reserve XID.",
                             true));

        SalNode snode = new SalNode(1L);
        IllegalStateException ise = new IllegalStateException();
        for (Entry<String, Boolean> entry: cases.entrySet()) {
            String emsg = entry.getKey();
            boolean disconnected = entry.getValue().booleanValue();

            GetFlowStatisticsInput input =
                mock(GetFlowStatisticsInput.class);
            when(input.getNode()).thenReturn(snode.getNodeRef());
            NodeRpcWatcher watcher = mock(NodeRpcWatcher.class);
            OpendaylightDirectStatisticsService dss =
                mock(OpendaylightDirectStatisticsService.class);
            RpcResult<GetFlowStatisticsOutput> result =
                RpcResultBuilder.<GetFlowStatisticsOutput>failed().
                withError(PROTOCOL, "Protocol error").
                withError(APPLICATION, emsg, ise).
                build();
            Future<RpcResult<GetFlowStatisticsOutput>> future =
                Futures.<RpcResult<GetFlowStatisticsOutput>>
                immediateFuture(result);
            when(dss.getFlowStatistics(input)).thenReturn(future);
            Logger logger = mock(Logger.class);

            GetFlowStatsRpc rpc = new GetFlowStatsRpc(watcher, dss, input);
            String msg = "RPC returned error";
            try {
                rpc.getResult(1L, TimeUnit.SECONDS, logger);
                unexpected();
            } catch (VTNException e) {
                assertEquals(VtnErrorTag.INTERNALERROR, e.getVtnErrorTag());
                assertEquals(msg, e.getMessage());
                assertEquals(null, e.getCause());
            }

            assertEquals(false, rpc.isNodeRemoved());
            assertEquals(disconnected, rpc.isDisconnected());

            String lmsg = RPC_NAME + ": RPC returned error: input=" + input +
                ", errors=" + result.getErrors();
            verify(logger).error(lmsg, ise);
            verify(watcher).registerRpc(rpc);
            verify(watcher).unregisterRpc(rpc);
            verifyNoMoreInteractions(watcher, logger);

            assertEquals(false, future.isCancelled());
        }
    }
}
