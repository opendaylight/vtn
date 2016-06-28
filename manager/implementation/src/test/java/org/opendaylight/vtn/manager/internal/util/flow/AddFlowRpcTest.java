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

import com.google.common.util.concurrent.SettableFuture;

import org.junit.Test;

import org.mockito.ArgumentCaptor;

import org.slf4j.Logger;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.util.inventory.NodeRpcInvocation;
import org.opendaylight.vtn.manager.internal.util.inventory.NodeRpcWatcher;
import org.opendaylight.vtn.manager.internal.util.inventory.SalNode;

import org.opendaylight.vtn.manager.internal.TestBase;

import org.opendaylight.controller.md.sal.dom.api.DOMRpcImplementationNotAvailableException;

import org.opendaylight.yangtools.yang.common.RpcError;
import org.opendaylight.yangtools.yang.common.RpcResult;
import org.opendaylight.yangtools.yang.common.RpcResultBuilder;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnErrorTag;

import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.service.rev130819.AddFlowInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.service.rev130819.AddFlowOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.service.rev130819.SalFlowService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.NodeRef;

/**
 * JUnit test for {@link AddFlowRpc}.
 */
public class AddFlowRpcTest extends TestBase {
    /**
     * The name of the add-flow RPC.
     */
    private static final String  RPC_NAME = "add-flow";

    /**
     * Test case for
     * {@link AddFlowRpc#create(FlowRpcWatcher,SalFlowService,AddFlowInput)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testCreate1() throws Exception {
        long[] dpids = {1L, 99L, 5555666677778888L, -1L};
        for (long dpid: dpids) {
            SalNode snode = new SalNode(dpid);
            NodeRef nref = snode.getNodeRef();
            AddFlowInput input = mock(AddFlowInput.class);
            when(input.getNode()).thenReturn(nref);
            FlowRpcWatcher watcher = mock(FlowRpcWatcher.class);
            SalFlowService sfs = mock(SalFlowService.class);
            AddFlowRpc rpc = AddFlowRpc.create(watcher, sfs, input);
            assertSame(input, rpc.getInput());
            assertSame(input, rpc.getInputForLog());
            assertSame(watcher, rpc.getNodeRpcWatcher());
            assertEquals(snode.toString(), rpc.getNode());
            assertEquals(Boolean.TRUE, getFieldValue(
                             rpc, NodeRpcInvocation.class, Boolean.class,
                             "disconnectionLog"));
            verify(sfs).addFlow(input);
            verify(watcher).asyncBarrier(nref);
            verifyNoMoreInteractions(sfs, watcher);
        }
    }

    /**
     * Test case for
     * {@link AddFlowRpc#create(FlowRpcWatcher,SalFlowService,AddFlowInput, boolean)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testCreate2() throws Exception {
        boolean[] bools = {true, false};
        long[] dpids = {1L, 99L, 5555666677778888L, -1L};
        for (boolean discon: bools) {
            for (long dpid: dpids) {
                SalNode snode = new SalNode(dpid);
                NodeRef nref = snode.getNodeRef();
                AddFlowInput input = mock(AddFlowInput.class);
                when(input.getNode()).thenReturn(nref);
                FlowRpcWatcher watcher = mock(FlowRpcWatcher.class);
                SalFlowService sfs = mock(SalFlowService.class);
                AddFlowRpc rpc = AddFlowRpc.create(watcher, sfs, input, discon);
                assertSame(input, rpc.getInput());
                assertSame(input, rpc.getInputForLog());
                assertSame(watcher, rpc.getNodeRpcWatcher());
                assertEquals(snode.toString(), rpc.getNode());
                assertEquals(Boolean.valueOf(discon), getFieldValue(
                                 rpc, NodeRpcInvocation.class, Boolean.class,
                                 "disconnectionLog"));
                verify(sfs).addFlow(input);
                verify(watcher).asyncBarrier(nref);
                verifyNoMoreInteractions(sfs, watcher);
            }
        }
    }

    /**
     * Test case for {@link AddFlowRpc#getInput()} and
     * {@link AddFlowRpc#getInputForLog()}.
     */
    @Test
    public void testGetInput() {
        SalNode snode = new SalNode(1L);
        boolean[] bools = {true, false};
        for (boolean discon: bools) {
            AddFlowInput input = mock(AddFlowInput.class);
            when(input.getNode()).thenReturn(snode.getNodeRef());
            NodeRpcWatcher watcher = mock(NodeRpcWatcher.class);
            SalFlowService sfs = mock(SalFlowService.class);
            AddFlowRpc rpc = new AddFlowRpc(watcher, sfs, input, discon);
            assertSame(input, rpc.getInput());
            assertSame(input, rpc.getInputForLog());
        }
    }

    /**
     * Test case for {@link AddFlowRpc#getName()}.
     */
    @Test
    public void testGetName() {
        SalNode snode = new SalNode(1L);
        boolean[] bools = {true, false};
        for (boolean discon: bools) {
            AddFlowInput input = mock(AddFlowInput.class);
            when(input.getNode()).thenReturn(snode.getNodeRef());
            NodeRpcWatcher watcher = mock(NodeRpcWatcher.class);
            SalFlowService sfs = mock(SalFlowService.class);
            AddFlowRpc rpc = new AddFlowRpc(watcher, sfs, input, discon);
            assertEquals(RPC_NAME, rpc.getName());
        }
    }

    /**
     * Test case for {@link AddFlowRpc#getFuture()}.
     */
    @Test
    public void testGetFuture() {
        SalNode snode = new SalNode(1L);
        boolean[] bools = {true, false};
        for (boolean discon: bools) {
            AddFlowInput input = mock(AddFlowInput.class);
            when(input.getNode()).thenReturn(snode.getNodeRef());
            NodeRpcWatcher watcher = mock(NodeRpcWatcher.class);
            SalFlowService sfs = mock(SalFlowService.class);
            Future<RpcResult<AddFlowOutput>> future =
                SettableFuture.<RpcResult<AddFlowOutput>>create();
            when(sfs.addFlow(input)).thenReturn(future);
            AddFlowRpc rpc = new AddFlowRpc(watcher, sfs, input, discon);
            assertSame(future, rpc.getFuture());
            verify(sfs).addFlow(input);
            verify(input).getNode();
            verifyNoMoreInteractions(watcher, sfs, input);
        }
    }

    /**
     * Test case for {@link AddFlowRpc#needErrorLog(Throwable)}.
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
        boolean[] bools = {true, false};
        for (Entry<Throwable, Boolean> entry: cases.entrySet()) {
            Throwable cause = entry.getKey();
            boolean disconnected = entry.getValue().booleanValue();
            for (boolean discon: bools) {
                AddFlowInput input = mock(AddFlowInput.class);
                when(input.getNode()).thenReturn(snode.getNodeRef());
                NodeRpcWatcher watcher = mock(NodeRpcWatcher.class);
                SalFlowService sfs = mock(SalFlowService.class);
                Future<RpcResult<AddFlowOutput>> future =
                    SettableFuture.<RpcResult<AddFlowOutput>>create();
                when(sfs.addFlow(input)).thenReturn(future);
                AddFlowRpc rpc = new AddFlowRpc(watcher, sfs, input, discon);
                assertEquals(false, rpc.isDisconnected());

                boolean needLog = (discon || !disconnected);
                assertEquals(needLog, rpc.needErrorLog(cause));
                assertEquals(disconnected, rpc.isDisconnected());
                assertEquals(false, rpc.isNodeRemoved());
                assertEquals(false, future.isCancelled());
            }
        }
    }

    /**
     * Test case for {@link AddFlowRpc#needErrorLog(Throwable)}.
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
        boolean[] bools = {true, false};
        for (Entry<Throwable, Boolean> entry: cases.entrySet()) {
            Throwable cause = entry.getKey();
            boolean disconnected = entry.getValue().booleanValue();
            for (boolean discon: bools) {
                AddFlowInput input = mock(AddFlowInput.class);
                when(input.getNode()).thenReturn(snode.getNodeRef());
                NodeRpcWatcher watcher = mock(NodeRpcWatcher.class);
                SalFlowService sfs = mock(SalFlowService.class);
                Future<RpcResult<AddFlowOutput>> future =
                    SettableFuture.<RpcResult<AddFlowOutput>>create();
                when(sfs.addFlow(input)).thenReturn(future);
                AddFlowRpc rpc = new AddFlowRpc(watcher, sfs, input, discon);
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
    }

    /**
     * Test case for {@link AddFlowRpc#needErrorLog(Collection)}.
     *
     * <ul>
     *   <li>Need a log for disconnection.</li>
     * </ul>
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
        boolean[] bools = {true, false};
        for (Entry<String, Boolean> entry: cases.entrySet()) {
            String msg = entry.getKey();
            boolean disconnected = entry.getValue().booleanValue();
            for (boolean discon: bools) {
                AddFlowInput input = mock(AddFlowInput.class);
                when(input.getNode()).thenReturn(snode.getNodeRef());
                NodeRpcWatcher watcher = mock(NodeRpcWatcher.class);
                SalFlowService sfs = mock(SalFlowService.class);
                Future<RpcResult<AddFlowOutput>> future =
                    SettableFuture.<RpcResult<AddFlowOutput>>create();
                when(sfs.addFlow(input)).thenReturn(future);
                AddFlowRpc rpc = new AddFlowRpc(watcher, sfs, input, discon);
                assertEquals(false, rpc.isDisconnected());

                RpcError err = mock(RpcError.class);
                when(err.getMessage()).thenReturn(msg);
                Collection<RpcError> errors = Arrays.asList(
                    mock(RpcError.class), mock(RpcError.class), err);
                boolean needLog = (discon || !disconnected);
                assertEquals(needLog, rpc.needErrorLog(errors));
                assertEquals(disconnected, rpc.isDisconnected());
                assertEquals(false, rpc.isNodeRemoved());
                assertEquals(false, future.isCancelled());
            }
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
        SalNode snode = new SalNode(1L);
        boolean[] bools = {true, false};
        for (boolean discon: bools) {
            AddFlowInput input = mock(AddFlowInput.class);
            when(input.getNode()).thenReturn(snode.getNodeRef());
            NodeRpcWatcher watcher = mock(NodeRpcWatcher.class);
            SalFlowService sfs = mock(SalFlowService.class);
            AddFlowOutput output = mock(AddFlowOutput.class);
            final SettableFuture<RpcResult<AddFlowOutput>> future =
                SettableFuture.<RpcResult<AddFlowOutput>>create();
            when(sfs.addFlow(input)).thenReturn(future);
            AddFlowRpc rpc = new AddFlowRpc(watcher, sfs, input, discon);

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
            assertEquals(false, rpc.isNodeRemoved());
            assertEquals(false, rpc.isDisconnected());

            verify(watcher).registerRpc(rpc);
            verify(watcher).unregisterRpc(rpc);
            verifyNoMoreInteractions(watcher, logger);
        }
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
        SalNode snode = new SalNode(1L);
        boolean[] bools = {true, false};
        for (boolean discon: bools) {
            AddFlowInput input = mock(AddFlowInput.class);
            when(input.getNode()).thenReturn(snode.getNodeRef());
            NodeRpcWatcher watcher = mock(NodeRpcWatcher.class);
            SalFlowService sfs = mock(SalFlowService.class);
            Future<RpcResult<AddFlowOutput>> future =
                SettableFuture.<RpcResult<AddFlowOutput>>create();
            when(sfs.addFlow(input)).thenReturn(future);
            AddFlowRpc rpc = new AddFlowRpc(watcher, sfs, input, discon);
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
    }

    /**
     * Test case for {@link AddFlowRpc#getResult(long,TimeUnit,Logger)}.
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
        boolean[] bools = {true, false};
        for (boolean discon: bools) {
            AddFlowInput input = mock(AddFlowInput.class);
            when(input.getNode()).thenReturn(snode.getNodeRef());
            NodeRpcWatcher watcher = mock(NodeRpcWatcher.class);
            SalFlowService sfs = mock(SalFlowService.class);
            SettableFuture<RpcResult<AddFlowOutput>> future =
                SettableFuture.<RpcResult<AddFlowOutput>>create();
            when(sfs.addFlow(input)).thenReturn(future);
            final AddFlowRpc rpc = new AddFlowRpc(watcher, sfs, input, discon);

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
        SalNode snode = new SalNode(1L);
        boolean[] bools = {true, false};
        for (boolean discon: bools) {
            AddFlowInput input = mock(AddFlowInput.class);
            when(input.getNode()).thenReturn(snode.getNodeRef());
            NodeRpcWatcher watcher = mock(NodeRpcWatcher.class);
            SalFlowService sfs = mock(SalFlowService.class);
            final DOMRpcImplementationNotAvailableException cause =
                new DOMRpcImplementationNotAvailableException("No impl");
            final SettableFuture<RpcResult<AddFlowOutput>> future =
                SettableFuture.<RpcResult<AddFlowOutput>>create();
            when(sfs.addFlow(input)).thenReturn(future);
            AddFlowRpc rpc = new AddFlowRpc(watcher, sfs, input, discon);
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

            if (discon) {
                String msg = RPC_NAME +
                    ": Caught an exception: canceled=false, input=" + input;
                ArgumentCaptor<Throwable> captor =
                    ArgumentCaptor.forClass(Throwable.class);
                verify(logger).error(eq(msg), captor.capture());
                List<Throwable> causes = captor.getAllValues();
                assertEquals(1, causes.size());
                Throwable c = causes.get(0);
                assertThat(c, instanceOf(ExecutionException.class));
                assertEquals(cause, c.getCause());
            }

            verify(watcher).registerRpc(rpc);
            verify(watcher).unregisterRpc(rpc);
            verifyNoMoreInteractions(watcher, logger);

            assertEquals(false, future.isCancelled());
        }
    }

    /**
     * Test case for {@link AddFlowRpc#getResult(long,TimeUnit,Logger)}.
     *
     * <ul>
     *   <li>RPC failed.</li>
     *   <li>The result does not contain any {@link Throwable}.</li>
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
        boolean[] bools = {true, false};
        SalNode snode = new SalNode(1L);
        for (Entry<String, Boolean> entry: cases.entrySet()) {
            String emsg = entry.getKey();
            boolean disconnected = entry.getValue().booleanValue();
            for (boolean discon: bools) {
                AddFlowInput input = mock(AddFlowInput.class);
                when(input.getNode()).thenReturn(snode.getNodeRef());
                NodeRpcWatcher watcher = mock(NodeRpcWatcher.class);
                SalFlowService sfs = mock(SalFlowService.class);
                final SettableFuture<RpcResult<AddFlowOutput>> future =
                    SettableFuture.<RpcResult<AddFlowOutput>>create();
                when(sfs.addFlow(input)).thenReturn(future);
                final RpcResult<AddFlowOutput> result =
                    RpcResultBuilder.<AddFlowOutput>failed().
                    withError(PROTOCOL, "Protocol error").
                    withError(APPLICATION, emsg).
                    build();
                AddFlowRpc rpc = new AddFlowRpc(watcher, sfs, input, discon);
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
                assertEquals(disconnected, rpc.isDisconnected());

                if (discon || !disconnected) {
                    verify(logger).
                        error("{}: {}: input={}, errors={}", RPC_NAME, msg,
                              input, result.getErrors());
                }
                verify(watcher).registerRpc(rpc);
                verify(watcher).unregisterRpc(rpc);
                verifyNoMoreInteractions(watcher, logger);

                assertEquals(false, future.isCancelled());
            }
        }
    }

    /**
     * Test case for {@link AddFlowRpc#getResult(long,TimeUnit,Logger)}.
     *
     * <ul>
     *   <li>RPC failed.</li>
     *   <li>The result contains an {@link IllegalStateException}.</li>
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
        boolean[] bools = {true, false};
        IllegalStateException ise = new IllegalStateException();
        for (Entry<String, Boolean> entry: cases.entrySet()) {
            String emsg = entry.getKey();
            boolean disconnected = entry.getValue().booleanValue();
            for (boolean discon: bools) {
                AddFlowInput input = mock(AddFlowInput.class);
                when(input.getNode()).thenReturn(snode.getNodeRef());
                NodeRpcWatcher watcher = mock(NodeRpcWatcher.class);
                SalFlowService sfs = mock(SalFlowService.class);
                final SettableFuture<RpcResult<AddFlowOutput>> future =
                    SettableFuture.<RpcResult<AddFlowOutput>>create();
                when(sfs.addFlow(input)).thenReturn(future);
                final RpcResult<AddFlowOutput> result =
                    RpcResultBuilder.<AddFlowOutput>failed().
                    withError(PROTOCOL, "Protocol error").
                    withError(APPLICATION, emsg, ise).
                    build();
                AddFlowRpc rpc = new AddFlowRpc(watcher, sfs, input, discon);
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
                assertEquals(disconnected, rpc.isDisconnected());

                if (discon || !disconnected) {
                    String lmsg = RPC_NAME + ": RPC returned error: input=" +
                        input + ", errors=" + result.getErrors();
                    verify(logger).error(lmsg, ise);
                }
                verify(watcher).registerRpc(rpc);
                verify(watcher).unregisterRpc(rpc);
                verifyNoMoreInteractions(watcher, logger);

                assertEquals(false, future.isCancelled());
            }
        }
    }
}
