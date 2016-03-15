/*
 * Copyright (c) 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.inventory;

import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.verifyNoMoreInteractions;
import static org.mockito.Mockito.when;

import static org.hamcrest.CoreMatchers.instanceOf;

import java.util.concurrent.CancellationException;
import java.util.concurrent.Future;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.TimeoutException;

import com.google.common.util.concurrent.SettableFuture;

import org.junit.Test;

import org.slf4j.Logger;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.TestBase;

import org.opendaylight.yangtools.yang.common.RpcResult;
import org.opendaylight.yangtools.yang.common.RpcResultBuilder;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnErrorTag;

import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.NodeRef;
import org.opendaylight.yang.gen.v1.urn.opendaylight.port.service.rev131107.UpdatePortInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.port.service.rev131107.UpdatePortOutput;

/**
 * JUnit test for {@link NodeRpcInvocation}.
 */
public class NodeRpcInvocationTest extends TestBase {
    /**
     * The name of the test RPC.
     */
    private static final String TEST_RPC_NAME = "test-node-rpc";

    /**
     * Test class that extends {@link NodeRpcInvocation}.
     */
    private static final class TestNodeRpc
        extends NodeRpcInvocation<UpdatePortInput, UpdatePortOutput> {
        /**
         * Construct a new instance.
         *
         * @param w   The node RPC watcher.
         * @param in  The RPC input.
         * @param f   The future associated with the RPC execution with
         *            {@code in}.
         */
        private TestNodeRpc(NodeRpcWatcher w, UpdatePortInput in,
                            Future<RpcResult<UpdatePortOutput>> f) {
            super(w, in, in.getNode(), f);
        }


        /**
         * Return the name of the RPC.
         *
         * @return  {@link NodeRpcInvocationTest#TEST_RPC_NAME}.
         */
        @Override
        public String getName() {
            return TEST_RPC_NAME;
        }
    }

    /**
     * Test case for the constructor.
     */
    @Test
    public void testConstructor() {
        String nid = "openflow:1234567";
        SalNode snode = SalNode.create(nid);
        NodeRef nref = snode.getNodeRef();
        UpdatePortInput input = mock(UpdatePortInput.class);
        when(input.getNode()).thenReturn(nref);
        NodeRpcWatcher watcher = mock(NodeRpcWatcher.class);
        Future<RpcResult<UpdatePortOutput>> future =
            SettableFuture.<RpcResult<UpdatePortOutput>>create();
        TestNodeRpc rpc = new TestNodeRpc(watcher, input, future);
        assertSame(input, rpc.getInput());
        assertSame(future, rpc.getFuture());
        assertSame(watcher, rpc.getNodeRpcWatcher());
        assertEquals(nid, rpc.getNode());
    }

    /**
     * Test case for {@link NodeRpcInvocation#start()} and
     * {@link NodeRpcInvocation#complete()}.
     */
    @Test
    public void testObservation() {
        SalNode snode = new SalNode(999L);
        NodeRef nref = snode.getNodeRef();
        UpdatePortInput input = mock(UpdatePortInput.class);
        when(input.getNode()).thenReturn(nref);
        NodeRpcWatcher watcher = mock(NodeRpcWatcher.class);
        Future<RpcResult<UpdatePortOutput>> future =
            SettableFuture.<RpcResult<UpdatePortOutput>>create();
        TestNodeRpc rpc = new TestNodeRpc(watcher, input, future);
        verifyNoMoreInteractions(watcher);

        rpc.start();
        verify(watcher).registerRpc(rpc);
        verifyNoMoreInteractions(watcher);

        rpc.complete();
        verify(watcher).unregisterRpc(rpc);
        verifyNoMoreInteractions(watcher);
    }

    /**
     * Test case for the following methods.
     *
     * <ul>
     *   <li>{@link NodeRpcInvocation#onNodeRemoved()}</li>
     *   <li>{@link NodeRpcInvocation#isNodeRemoved()}</li>
     *   <li>{@link NodeRpcInvocation#needErrorLog(Throwable)}</li>
     * </ul>
     */
    @Test
    public void testOnNodeRemoved() {
        SalNode snode = new SalNode(999L);
        NodeRef nref = snode.getNodeRef();
        UpdatePortInput input = mock(UpdatePortInput.class);
        when(input.getNode()).thenReturn(nref);
        NodeRpcWatcher watcher = mock(NodeRpcWatcher.class);
        Future<RpcResult<UpdatePortOutput>> future =
            SettableFuture.<RpcResult<UpdatePortOutput>>create();
        TestNodeRpc rpc = new TestNodeRpc(watcher, input, future);
        CancellationException ce = new CancellationException();
        assertEquals(true, rpc.needErrorLog(ce));
        assertEquals(false, rpc.isNodeRemoved());
        assertEquals(false, future.isCancelled());

        assertEquals(true, rpc.onNodeRemoved());
        assertEquals(false, rpc.needErrorLog(ce));
        assertEquals(true, rpc.isNodeRemoved());
        assertEquals(true, future.isCancelled());

        assertEquals(false, rpc.onNodeRemoved());
        assertEquals(false, rpc.needErrorLog(ce));
        assertEquals(true, rpc.isNodeRemoved());
        assertEquals(true, future.isCancelled());
    }

    /**
     * Test case for {@link NodeRpcInvocation#getResult(long,TimeUnit,Logger)}.
     *
     * <ul>
     *   <li>RPC completed successfully.</li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testGetResult() throws Exception {
        SalNode snode = new SalNode(112233L);
        NodeRef nref = snode.getNodeRef();
        UpdatePortInput input = mock(UpdatePortInput.class);
        when(input.getNode()).thenReturn(nref);

        UpdatePortOutput output = mock(UpdatePortOutput.class);
        final RpcResult<UpdatePortOutput> result = RpcResultBuilder.
            success(output).build();
        final SettableFuture<RpcResult<UpdatePortOutput>> future =
            SettableFuture.<RpcResult<UpdatePortOutput>>create();

        NodeRpcWatcher watcher = mock(NodeRpcWatcher.class);
        TestNodeRpc rpc = new TestNodeRpc(watcher, input, future);
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

        verify(watcher).registerRpc(rpc);
        verify(watcher).unregisterRpc(rpc);
        verifyNoMoreInteractions(logger, watcher);
    }

    /**
     * Test case for {@link NodeRpcInvocation#getResult(long,TimeUnit,Logger)}.
     *
     * <ul>
     *   <li>RPC timed out.</li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testGetResultTimeout() throws Exception {
        SettableFuture<RpcResult<UpdatePortOutput>> future =
            SettableFuture.<RpcResult<UpdatePortOutput>>create();

        SalNode snode = new SalNode(112233L);
        NodeRef nref = snode.getNodeRef();
        UpdatePortInput input = mock(UpdatePortInput.class);
        when(input.getNode()).thenReturn(nref);

        NodeRpcWatcher watcher = mock(NodeRpcWatcher.class);
        TestNodeRpc rpc = new TestNodeRpc(watcher, input, future);
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

        verify(watcher).registerRpc(rpc);
        verify(watcher).unregisterRpc(rpc);
        verifyNoMoreInteractions(logger, watcher);
    }

    /**
     * Test case for {@link NodeRpcInvocation#getResult(long,TimeUnit,Logger)}.
     *
     * <ul>
     *   <li>RPC was canceled.</li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testGetResultCancel() throws Exception {
        SettableFuture<RpcResult<UpdatePortOutput>> future =
            SettableFuture.<RpcResult<UpdatePortOutput>>create();

        SalNode snode = new SalNode(777L);
        NodeRef nref = snode.getNodeRef();
        UpdatePortInput input = mock(UpdatePortInput.class);
        when(input.getNode()).thenReturn(nref);

        NodeRpcWatcher watcher = mock(NodeRpcWatcher.class);
        final TestNodeRpc rpc = new TestNodeRpc(watcher, input, future);
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
        verifyNoMoreInteractions(logger, watcher);

        assertEquals(true, future.isCancelled());
    }
}
