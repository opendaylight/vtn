/*
 * Copyright (c) 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.flow;

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
import java.util.Map.Entry;
import java.util.Map;
import java.util.concurrent.CancellationException;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.Future;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.TimeoutException;

import com.google.common.util.concurrent.SettableFuture;

import org.junit.Test;

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

import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.NodeRef;

import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.transaction.rev150304.FlowCapableTransactionService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.transaction.rev150304.SendBarrierInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.transaction.rev150304.SendBarrierInputBuilder;

/**
 * JUnit test for {@link SendBarrierRpc}.
 */
public class SendBarrierRpcTest extends TestBase {
    /**
     * The name of the send-barrier RPC.
     */
    private static final String  RPC_NAME = "send-barrier";

    /**
     * Test case for the following methods.
     *
     * <ul>
     *   <li>{@link SendBarrierRpc#create(NodeRpcWatcher,FlowCapableTransactionService,NodeRef)}</li>
     *   <li>{@link SendBarrierRpc#getInput()}</li>
     *   <li>{@link SendBarrierRpc#getInputForLog()}</li>
     * </ul>
     */
    @Test
    public void testGetInput() {
        long[] dpids = {1L, 12345L, 6666777788889999L, -1L};
        for (long dpid: dpids) {
            SalNode snode = new SalNode(dpid);
            NodeRef nref = snode.getNodeRef();
            SendBarrierInput input = new SendBarrierInputBuilder().
                setNode(nref).build();
            NodeRpcWatcher watcher = mock(NodeRpcWatcher.class);
            FlowCapableTransactionService fcts =
                mock(FlowCapableTransactionService.class);
            SendBarrierRpc rpc = SendBarrierRpc.create(watcher, fcts, nref);
            assertEquals(input, rpc.getInput());
            assertEquals(input, rpc.getInputForLog());
        }
    }

    /**
     * Test case for {@link SendBarrierRpc#getName()}.
     */
    @Test
    public void testGetName() {
        SalNode snode = new SalNode(1L);
        SendBarrierInput input = mock(SendBarrierInput.class);
        when(input.getNode()).thenReturn(snode.getNodeRef());
        NodeRpcWatcher watcher = mock(NodeRpcWatcher.class);
        FlowCapableTransactionService fcts =
            mock(FlowCapableTransactionService.class);
        SendBarrierRpc rpc = new SendBarrierRpc(watcher, fcts, input);
        assertEquals(RPC_NAME, rpc.getName());
    }

    /**
     * Test case for {@link SendBarrierRpc#getFuture()}.
     */
    @Test
    public void testGetFuture() {
        SalNode snode = new SalNode(1L);
        SendBarrierInput input = mock(SendBarrierInput.class);
        when(input.getNode()).thenReturn(snode.getNodeRef());
        NodeRpcWatcher watcher = mock(NodeRpcWatcher.class);
        FlowCapableTransactionService fcts =
            mock(FlowCapableTransactionService.class);
        Future<RpcResult<Void>> future =
            SettableFuture.<RpcResult<Void>>create();
        when(fcts.sendBarrier(input)).thenReturn(future);
        SendBarrierRpc rpc = new SendBarrierRpc(watcher, fcts, input);
        assertSame(future, rpc.getFuture());
        verify(fcts).sendBarrier(input);
        verify(input).getNode();
        verifyNoMoreInteractions(watcher, fcts, input);
    }

    /**
     * Test case for {@link SendBarrierRpc#needErrorLog(Throwable)}.
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

            SendBarrierInput input = mock(SendBarrierInput.class);
            when(input.getNode()).thenReturn(snode.getNodeRef());
            NodeRpcWatcher watcher = mock(NodeRpcWatcher.class);
            FlowCapableTransactionService fcts =
                mock(FlowCapableTransactionService.class);
            Future<RpcResult<Void>> future =
                SettableFuture.<RpcResult<Void>>create();
            when(fcts.sendBarrier(input)).thenReturn(future);
            SendBarrierRpc rpc = new SendBarrierRpc(watcher, fcts, input);
            assertEquals(false, rpc.isDisconnected());

            assertEquals(!disconnected, rpc.needErrorLog(cause));
            assertEquals(disconnected, rpc.isDisconnected());
            assertEquals(false, rpc.isNodeRemoved());
            assertEquals(false, future.isCancelled());
        }
    }

    /**
     * Test case for {@link SendBarrierRpc#needErrorLog(Throwable)}.
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

            SendBarrierInput input = mock(SendBarrierInput.class);
            when(input.getNode()).thenReturn(snode.getNodeRef());
            NodeRpcWatcher watcher = mock(NodeRpcWatcher.class);
            FlowCapableTransactionService fcts =
                mock(FlowCapableTransactionService.class);
            Future<RpcResult<Void>> future =
                SettableFuture.<RpcResult<Void>>create();
            when(fcts.sendBarrier(input)).thenReturn(future);
            SendBarrierRpc rpc = new SendBarrierRpc(watcher, fcts, input);
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
     * Test case for {@link SendBarrierRpc#needErrorLog(Collection)}.
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

            SendBarrierInput input = mock(SendBarrierInput.class);
            when(input.getNode()).thenReturn(snode.getNodeRef());
            NodeRpcWatcher watcher = mock(NodeRpcWatcher.class);
            FlowCapableTransactionService fcts =
                mock(FlowCapableTransactionService.class);
            Future<RpcResult<Void>> future =
                SettableFuture.<RpcResult<Void>>create();
            when(fcts.sendBarrier(input)).thenReturn(future);
            SendBarrierRpc rpc = new SendBarrierRpc(watcher, fcts, input);

            RpcError err = mock(RpcError.class);
            when(err.getMessage()).thenReturn(msg);
            Collection<RpcError> errors = Arrays.asList(
                mock(RpcError.class), mock(RpcError.class), err);
            assertEquals(!disconnected, rpc.needErrorLog(errors));
            assertEquals(disconnected, rpc.isDisconnected());
            assertEquals(false, rpc.isNodeRemoved());
            assertEquals(false, future.isCancelled());
        }
    }

    /**
     * Test case for {@link SendBarrierRpc#getResult(long,TimeUnit,Logger)}.
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
        SendBarrierInput input = mock(SendBarrierInput.class);
        when(input.getNode()).thenReturn(snode.getNodeRef());
        NodeRpcWatcher watcher = mock(NodeRpcWatcher.class);
        FlowCapableTransactionService fcts =
            mock(FlowCapableTransactionService.class);
        Void output = null;
        final SettableFuture<RpcResult<Void>> future =
            SettableFuture.<RpcResult<Void>>create();
        when(fcts.sendBarrier(input)).thenReturn(future);
        SendBarrierRpc rpc = new SendBarrierRpc(watcher, fcts, input);

        final RpcResult<Void> result = RpcResultBuilder.
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
        assertEquals(false, rpc.isNodeRemoved());
        assertEquals(false, rpc.isDisconnected());

        verify(watcher).registerRpc(rpc);
        verify(watcher).unregisterRpc(rpc);
        verifyNoMoreInteractions(watcher, logger);
    }

    /**
     * Test case for {@link SendBarrierRpc#getResult(long,TimeUnit,Logger)}.
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
        SendBarrierInput input = mock(SendBarrierInput.class);
        when(input.getNode()).thenReturn(snode.getNodeRef());
        NodeRpcWatcher watcher = mock(NodeRpcWatcher.class);
        FlowCapableTransactionService fcts =
            mock(FlowCapableTransactionService.class);
        Future<RpcResult<Void>> future =
            SettableFuture.<RpcResult<Void>>create();
        when(fcts.sendBarrier(input)).thenReturn(future);
        SendBarrierRpc rpc = new SendBarrierRpc(watcher, fcts, input);
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
     * Test case for {@link SendBarrierRpc#getResult(long,TimeUnit,Logger)}.
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
        SendBarrierInput input = mock(SendBarrierInput.class);
        when(input.getNode()).thenReturn(snode.getNodeRef());
        NodeRpcWatcher watcher = mock(NodeRpcWatcher.class);
        FlowCapableTransactionService fcts =
            mock(FlowCapableTransactionService.class);
        SettableFuture<RpcResult<Void>> future =
            SettableFuture.<RpcResult<Void>>create();
        when(fcts.sendBarrier(input)).thenReturn(future);
        final SendBarrierRpc rpc = new SendBarrierRpc(watcher, fcts, input);

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
     * Test case for {@link SendBarrierRpc#getResult(long,TimeUnit,Logger)}.
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
        SendBarrierInput input = mock(SendBarrierInput.class);
        when(input.getNode()).thenReturn(snode.getNodeRef());
        NodeRpcWatcher watcher = mock(NodeRpcWatcher.class);
        FlowCapableTransactionService fcts =
            mock(FlowCapableTransactionService.class);
        final DOMRpcImplementationNotAvailableException cause =
            new DOMRpcImplementationNotAvailableException("No implementation");
        final SettableFuture<RpcResult<Void>> future =
            SettableFuture.<RpcResult<Void>>create();
        when(fcts.sendBarrier(input)).thenReturn(future);
        SendBarrierRpc rpc = new SendBarrierRpc(watcher, fcts, input);
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

        assertEquals(false, rpc.isNodeRemoved());
        assertEquals(true, rpc.isDisconnected());

        // No error message should be logged.
        verify(watcher).registerRpc(rpc);
        verify(watcher).unregisterRpc(rpc);
        verifyNoMoreInteractions(watcher, logger);
    }

    /**
     * Test case for {@link SendBarrierRpc#getResult(long,TimeUnit,Logger)}.
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
        };

        SalNode snode = new SalNode(1L);
        for (String emsg: msgs) {
            SendBarrierInput input = mock(SendBarrierInput.class);
            when(input.getNode()).thenReturn(snode.getNodeRef());
            NodeRpcWatcher watcher = mock(NodeRpcWatcher.class);
            FlowCapableTransactionService fcts =
                mock(FlowCapableTransactionService.class);
            final SettableFuture<RpcResult<Void>> future =
                SettableFuture.<RpcResult<Void>>create();
            when(fcts.sendBarrier(input)).thenReturn(future);
            final RpcResult<Void> result =
                RpcResultBuilder.<Void>failed().
                withError(PROTOCOL, "Protocol error").
                withError(APPLICATION, emsg).
                build();
            SendBarrierRpc rpc = new SendBarrierRpc(watcher, fcts, input);
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

            assertEquals(false, rpc.isNodeRemoved());
            assertEquals(false, rpc.isDisconnected());

            verify(logger).
                error("{}: {}: input={}, errors={}", RPC_NAME, msg, input,
                      result.getErrors());
            verify(watcher).registerRpc(rpc);
            verify(watcher).unregisterRpc(rpc);
            verifyNoMoreInteractions(watcher, logger);
        }
    }

    /**
     * Test case for {@link SendBarrierRpc#getResult(long,TimeUnit,Logger)}.
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
        };
        IllegalStateException ise = new IllegalStateException();

        SalNode snode = new SalNode(1L);
        for (String emsg: msgs) {
            SendBarrierInput input = mock(SendBarrierInput.class);
            when(input.getNode()).thenReturn(snode.getNodeRef());
            NodeRpcWatcher watcher = mock(NodeRpcWatcher.class);
            FlowCapableTransactionService fcts =
                mock(FlowCapableTransactionService.class);
            final SettableFuture<RpcResult<Void>> future =
                SettableFuture.<RpcResult<Void>>create();
            when(fcts.sendBarrier(input)).thenReturn(future);
            final RpcResult<Void> result =
                RpcResultBuilder.<Void>failed().
                withError(PROTOCOL, "Protocol error").
                withError(APPLICATION, emsg, ise).
                build();
            SendBarrierRpc rpc = new SendBarrierRpc(watcher, fcts, input);
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

            assertEquals(false, rpc.isNodeRemoved());
            assertEquals(false, rpc.isDisconnected());

            String lmsg = RPC_NAME + ": RPC returned error: input=" + input +
                ", errors=" + result.getErrors();
            verify(logger).error(lmsg, ise);
            verify(watcher).registerRpc(rpc);
            verify(watcher).unregisterRpc(rpc);
            verifyNoMoreInteractions(watcher, logger);
        }
    }

    /**
     * Test case for {@link SendBarrierRpc#getResult(long,TimeUnit,Logger)}.
     *
     * <ul>
     *   <li>RPC failed.</li>
     *   <li>The result should not be logged.</li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testGetResultNoLog() throws Exception {
        String[] msgs = {
            "Device disconnected",
            "Outbound queue wasn't able to reserve XID.",
        };
        Throwable[] causes = {
            null,
            new IllegalStateException(),
            new IllegalArgumentException(),
        };


        SalNode snode = new SalNode(1L);
        for (String emsg: msgs) {
            for (Throwable cause: causes) {
                SendBarrierInput input = mock(SendBarrierInput.class);
                when(input.getNode()).thenReturn(snode.getNodeRef());
                NodeRpcWatcher watcher = mock(NodeRpcWatcher.class);
                FlowCapableTransactionService fcts =
                    mock(FlowCapableTransactionService.class);
                final SettableFuture<RpcResult<Void>> future =
                    SettableFuture.<RpcResult<Void>>create();
                when(fcts.sendBarrier(input)).thenReturn(future);
                final RpcResult<Void> result =
                    RpcResultBuilder.<Void>failed().
                    withError(PROTOCOL, "Protocol error").
                    withError(APPLICATION, emsg, cause).
                    build();
                SendBarrierRpc rpc = new SendBarrierRpc(watcher, fcts, input);
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

                assertEquals(false, rpc.isNodeRemoved());
                assertEquals(true, rpc.isDisconnected());

                // No error message should be logged.
                verify(watcher).registerRpc(rpc);
                verify(watcher).unregisterRpc(rpc);
                verifyNoMoreInteractions(watcher, logger);
            }
        }
    }
}
