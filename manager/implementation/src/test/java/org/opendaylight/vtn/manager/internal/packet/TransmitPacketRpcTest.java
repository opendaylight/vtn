/*
 * Copyright (c) 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.packet;

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

import com.google.common.util.concurrent.SettableFuture;

import org.junit.Test;

import org.mockito.ArgumentCaptor;

import org.slf4j.Logger;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.util.inventory.NodeRpcWatcher;
import org.opendaylight.vtn.manager.internal.util.inventory.SalPort;

import org.opendaylight.vtn.manager.internal.TestBase;

import org.opendaylight.controller.md.sal.dom.api.DOMRpcImplementationNotAvailableException;

import org.opendaylight.yangtools.yang.common.RpcError;
import org.opendaylight.yangtools.yang.common.RpcResult;
import org.opendaylight.yangtools.yang.common.RpcResultBuilder;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnErrorTag;

import org.opendaylight.yang.gen.v1.urn.opendaylight.packet.service.rev130709.PacketProcessingService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.packet.service.rev130709.TransmitPacketInput;

/**
 * JUnit test for {@link TransmitPacketRpc}.
 */
public class TransmitPacketRpcTest extends TestBase {
    /**
     * The name of the transmit-packet RPC.
     */
    private static final String  RPC_NAME = "transmit-packet";

    /**
     * Test case for {@link TransmitPacketRpc#getInput()}.
     */
    @Test
    public void testGetInput() {
        SalPort egress = new SalPort(1L, 2L);
        TransmitPacketInput input = mock(TransmitPacketInput.class);
        when(input.getNode()).thenReturn(egress.getNodeRef());
        when(input.getEgress()).thenReturn(egress.getNodeConnectorRef());
        NodeRpcWatcher watcher = mock(NodeRpcWatcher.class);
        PacketProcessingService pps = mock(PacketProcessingService.class);
        TransmitPacketRpc rpc = new TransmitPacketRpc(watcher, pps, input);
        assertSame(input, rpc.getInput());
    }

    /**
     * Test case for {@link TransmitPacketRpc#getInputForLog()}.
     */
    @Test
    public void testGetInputForLog() {
        String[] ports = {
            "openflow:1:1",
            "openflow:123:456",
            "openflow:18446744073709551615:4294967040",
        };
        for (String port: ports) {
            SalPort egress = SalPort.create(port);
            TransmitPacketInput input = mock(TransmitPacketInput.class);
            when(input.getNode()).thenReturn(egress.getNodeRef());
            when(input.getEgress()).thenReturn(egress.getNodeConnectorRef());
            NodeRpcWatcher watcher = mock(NodeRpcWatcher.class);
            PacketProcessingService pps = mock(PacketProcessingService.class);
            TransmitPacketRpc rpc = new TransmitPacketRpc(watcher, pps, input);
            String egressStr = "{egress=" + port + "}";
            assertEquals(egressStr, rpc.getInputForLog());
        }
    }

    /**
     * Test case for {@link TransmitPacketRpc#getName()}.
     */
    @Test
    public void testGetName() {
        SalPort egress = new SalPort(1L, 2L);
        TransmitPacketInput input = mock(TransmitPacketInput.class);
        when(input.getNode()).thenReturn(egress.getNodeRef());
        when(input.getEgress()).thenReturn(egress.getNodeConnectorRef());
        NodeRpcWatcher watcher = mock(NodeRpcWatcher.class);
        PacketProcessingService pps = mock(PacketProcessingService.class);
        TransmitPacketRpc rpc = new TransmitPacketRpc(watcher, pps, input);
        assertEquals(RPC_NAME, rpc.getName());
    }

    /**
     * Test case for {@link TransmitPacketRpc#getFuture()}.
     */
    @Test
    public void testGetFuture() {
        SalPort egress = new SalPort(1L, 2L);
        TransmitPacketInput input = mock(TransmitPacketInput.class);
        when(input.getNode()).thenReturn(egress.getNodeRef());
        when(input.getEgress()).thenReturn(egress.getNodeConnectorRef());
        NodeRpcWatcher watcher = mock(NodeRpcWatcher.class);
        PacketProcessingService pps = mock(PacketProcessingService.class);
        Future<RpcResult<Void>> future =
            SettableFuture.<RpcResult<Void>>create();
        when(pps.transmitPacket(input)).thenReturn(future);
        TransmitPacketRpc rpc = new TransmitPacketRpc(watcher, pps, input);
        assertSame(future, rpc.getFuture());
        verify(pps).transmitPacket(input);
        verify(input).getNode();
        verifyNoMoreInteractions(watcher, pps, input);
    }

    /**
     * Test case for {@link TransmitPacketRpc#needErrorLog(Throwable)}.
     */
    @Test
    public void testNeedErrorLog1() {
        SalPort egress = new SalPort(1L, 2L);
        TransmitPacketInput input = mock(TransmitPacketInput.class);
        when(input.getNode()).thenReturn(egress.getNodeRef());
        when(input.getEgress()).thenReturn(egress.getNodeConnectorRef());
        NodeRpcWatcher watcher = mock(NodeRpcWatcher.class);
        PacketProcessingService pps = mock(PacketProcessingService.class);
        Future<RpcResult<Void>> future =
            SettableFuture.<RpcResult<Void>>create();
        when(pps.transmitPacket(input)).thenReturn(future);
        TransmitPacketRpc rpc = new TransmitPacketRpc(watcher, pps, input);

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
            assertEquals(false, rpc.isNodeRemoved());
        }

        // Cancel the RPC.
        assertEquals(true, rpc.onNodeRemoved());
        assertEquals(true, future.isCancelled());
        for (Throwable cause: cases.keySet()) {
            assertEquals(false, rpc.needErrorLog(cause));
            assertEquals(true, rpc.isNodeRemoved());
        }
    }

    /**
     * Test case for {@link TransmitPacketRpc#needErrorLog(Collection)}.
     */
    @Test
    public void testNeedErrorLog2() {
        SalPort egress = new SalPort(1L, 2L);
        TransmitPacketInput input = mock(TransmitPacketInput.class);
        when(input.getNode()).thenReturn(egress.getNodeRef());
        when(input.getEgress()).thenReturn(egress.getNodeConnectorRef());
        NodeRpcWatcher watcher = mock(NodeRpcWatcher.class);
        PacketProcessingService pps = mock(PacketProcessingService.class);
        Future<RpcResult<Void>> future =
            SettableFuture.<RpcResult<Void>>create();
        when(pps.transmitPacket(input)).thenReturn(future);
        TransmitPacketRpc rpc = new TransmitPacketRpc(watcher, pps, input);

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
     * Test case for {@link TransmitPacketRpc#getResult(long,TimeUnit,Logger)}.
     *
     * <ul>
     *   <li>RPC completed successfully.</li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testGetResult() throws Exception {
        SalPort egress = new SalPort(1L, 2L);
        TransmitPacketInput input = mock(TransmitPacketInput.class);
        when(input.getNode()).thenReturn(egress.getNodeRef());
        when(input.getEgress()).thenReturn(egress.getNodeConnectorRef());
        NodeRpcWatcher watcher = mock(NodeRpcWatcher.class);
        PacketProcessingService pps = mock(PacketProcessingService.class);
        final SettableFuture<RpcResult<Void>> future =
            SettableFuture.<RpcResult<Void>>create();
        when(pps.transmitPacket(input)).thenReturn(future);
        TransmitPacketRpc rpc = new TransmitPacketRpc(watcher, pps, input);

        final RpcResult<Void> result = RpcResultBuilder.
            success((Void)null).build();
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

        assertSame(null, rpc.getResult(10L, TimeUnit.SECONDS, logger));

        verify(watcher).registerRpc(rpc);
        verify(watcher).unregisterRpc(rpc);
        verifyNoMoreInteractions(watcher, logger);
    }

    /**
     * Test case for {@link TransmitPacketRpc#getResult(long,TimeUnit,Logger)}.
     *
     * <ul>
     *   <li>RPC timed out.</li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testGetResultTimeout() throws Exception {
        SalPort egress = new SalPort(1L, 2L);
        TransmitPacketInput input = mock(TransmitPacketInput.class);
        when(input.getNode()).thenReturn(egress.getNodeRef());
        when(input.getEgress()).thenReturn(egress.getNodeConnectorRef());
        NodeRpcWatcher watcher = mock(NodeRpcWatcher.class);
        PacketProcessingService pps = mock(PacketProcessingService.class);
        Future<RpcResult<Void>> future =
            SettableFuture.<RpcResult<Void>>create();
        when(pps.transmitPacket(input)).thenReturn(future);
        TransmitPacketRpc rpc = new TransmitPacketRpc(watcher, pps, input);
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
            "input={egress=" + egress + "}";
        verify(logger).error(msg, cause);
        verify(watcher).registerRpc(rpc);
        verify(watcher).unregisterRpc(rpc);
        verifyNoMoreInteractions(watcher, logger);
    }

    /**
     * Test case for {@link TransmitPacketRpc#getResult(long,TimeUnit,Logger)}.
     *
     * <ul>
     *   <li>RPC was canceled.</li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testGetResultCancel() throws Exception {
        SalPort egress = new SalPort(1L, 2L);
        TransmitPacketInput input = mock(TransmitPacketInput.class);
        when(input.getNode()).thenReturn(egress.getNodeRef());
        when(input.getEgress()).thenReturn(egress.getNodeConnectorRef());
        NodeRpcWatcher watcher = mock(NodeRpcWatcher.class);
        PacketProcessingService pps = mock(PacketProcessingService.class);
        SettableFuture<RpcResult<Void>> future =
            SettableFuture.<RpcResult<Void>>create();
        when(pps.transmitPacket(input)).thenReturn(future);
        final TransmitPacketRpc rpc =
            new TransmitPacketRpc(watcher, pps, input);

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

        // No error should be logged.
        verify(watcher).registerRpc(rpc);
        verify(watcher).unregisterRpc(rpc);
        verifyNoMoreInteractions(watcher, logger);

        assertEquals(true, future.isCancelled());
    }

    /**
     * Test case for {@link TransmitPacketRpc#getResult(long,TimeUnit,Logger)}.
     *
     * <ul>
     *   <li>No RPC implementation.</li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testGetResultNoRpc() throws Exception {
        SalPort egress = new SalPort(1L, 2L);
        TransmitPacketInput input = mock(TransmitPacketInput.class);
        when(input.getNode()).thenReturn(egress.getNodeRef());
        when(input.getEgress()).thenReturn(egress.getNodeConnectorRef());
        NodeRpcWatcher watcher = mock(NodeRpcWatcher.class);
        PacketProcessingService pps = mock(PacketProcessingService.class);
        final DOMRpcImplementationNotAvailableException cause =
            new DOMRpcImplementationNotAvailableException("No implementation");
        final SettableFuture<RpcResult<Void>> future =
            SettableFuture.<RpcResult<Void>>create();
        when(pps.transmitPacket(input)).thenReturn(future);
        TransmitPacketRpc rpc = new TransmitPacketRpc(watcher, pps, input);
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
            "input={egress=" + egress + "}";
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
     * Test case for {@link TransmitPacketRpc#getResult(long,TimeUnit,Logger)}.
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

        SalPort egress = new SalPort(1L, 2L);
        String egressStr = "{egress=" + egress + "}";
        for (String emsg: msgs) {
            TransmitPacketInput input = mock(TransmitPacketInput.class);
            when(input.getNode()).thenReturn(egress.getNodeRef());
            when(input.getEgress()).thenReturn(egress.getNodeConnectorRef());
            NodeRpcWatcher watcher = mock(NodeRpcWatcher.class);
            PacketProcessingService pps = mock(PacketProcessingService.class);
            final SettableFuture<RpcResult<Void>> future =
                SettableFuture.<RpcResult<Void>>create();
            when(pps.transmitPacket(input)).thenReturn(future);
            final RpcResult<Void> result =
                RpcResultBuilder.<Void>failed().
                withError(PROTOCOL, "Protocol error").
                withError(APPLICATION, emsg).
                build();
            TransmitPacketRpc rpc = new TransmitPacketRpc(watcher, pps, input);
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
                error("{}: {}: input={}, errors={}", RPC_NAME, msg, egressStr,
                      result.getErrors());
            verify(watcher).registerRpc(rpc);
            verify(watcher).unregisterRpc(rpc);
            verifyNoMoreInteractions(watcher, logger);
        }
    }

    /**
     * Test case for {@link TransmitPacketRpc#getResult(long,TimeUnit,Logger)}.
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

        SalPort egress = new SalPort(1L, 2L);
        String egressStr = "{egress=" + egress + "}";
        for (String emsg: msgs) {
            TransmitPacketInput input = mock(TransmitPacketInput.class);
            when(input.getNode()).thenReturn(egress.getNodeRef());
            when(input.getEgress()).thenReturn(egress.getNodeConnectorRef());
            NodeRpcWatcher watcher = mock(NodeRpcWatcher.class);
            PacketProcessingService pps = mock(PacketProcessingService.class);
            final SettableFuture<RpcResult<Void>> future =
                SettableFuture.<RpcResult<Void>>create();
            when(pps.transmitPacket(input)).thenReturn(future);
            final RpcResult<Void> result = RpcResultBuilder.<Void>failed().
                withError(PROTOCOL, "Protocol error").
                withError(APPLICATION, emsg, ise).
                build();
            TransmitPacketRpc rpc = new TransmitPacketRpc(watcher, pps, input);
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

            String lmsg = RPC_NAME + ": RPC returned error: input=" +
                egressStr + ", errors=" + result.getErrors();
            verify(logger).error(lmsg, ise);
            verify(watcher).registerRpc(rpc);
            verify(watcher).unregisterRpc(rpc);
            verifyNoMoreInteractions(watcher, logger);
        }
    }
}
