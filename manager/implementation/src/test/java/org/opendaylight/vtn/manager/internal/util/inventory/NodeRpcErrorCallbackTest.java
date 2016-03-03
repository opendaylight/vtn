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

import com.google.common.util.concurrent.Futures;
import com.google.common.util.concurrent.SettableFuture;

import org.junit.Test;

import org.slf4j.Logger;

import org.opendaylight.vtn.manager.internal.util.flow.AddFlowRpc;

import org.opendaylight.vtn.manager.internal.TestBase;

import org.opendaylight.yangtools.yang.common.RpcResult;
import org.opendaylight.yangtools.yang.common.RpcResultBuilder;

import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.service.rev130819.AddFlowInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.service.rev130819.AddFlowOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.service.rev130819.SalFlowService;

/**
 * JUnit test for {@link NodeRpcErrorCallback}.
 */
public class NodeRpcErrorCallbackTest extends TestBase {
    /**
     * Test case for {@link NodeRpcErrorCallback#onSuccess(RpcResult)}.
     */
    @Test
    public void testSuccess() {
        SalNode snode = new SalNode(1L);
        AddFlowInput input = mock(AddFlowInput.class);
        when(input.getNode()).thenReturn(snode.getNodeRef());
        NodeRpcWatcher watcher = mock(NodeRpcWatcher.class);
        SettableFuture<RpcResult<AddFlowOutput>> future =
            SettableFuture.<RpcResult<AddFlowOutput>>create();
        SalFlowService sfs = mock(SalFlowService.class);
        when(sfs.addFlow(input)).thenReturn(future);
        AddFlowRpc rpc = new AddFlowRpc(watcher, sfs, input);
        verify(sfs).addFlow(input);
        verify(input).getNode();
        verifyNoMoreInteractions(input, sfs);

        Logger logger = mock(Logger.class);
        NodeRpcErrorCallback<AddFlowOutput> cb =
            new NodeRpcErrorCallback<>(rpc, logger, "Unexpected log 1");
        verify(watcher).registerRpc(rpc);
        verifyNoMoreInteractions(logger, watcher);
        Futures.addCallback(future, cb);

        AddFlowOutput output = mock(AddFlowOutput.class);
        RpcResult<AddFlowOutput> result = RpcResultBuilder.
            success(output).build();
        future.set(result);

        verify(logger).
            trace("{}: RPC completed successfully: result={}",
                  rpc.getName(), output);
        verify(watcher).unregisterRpc(rpc);
        verifyNoMoreInteractions(logger, watcher);
    }

    /**
     * Test case for {@link NodeRpcErrorCallback#onFailure(Throwable)}.
     *
     * <p>
     *   The RPC invocation failed due to unexpeted exception.
     * </p>
     */
    @Test
    public void testFailure() {
        SalNode snode = new SalNode(1L);
        AddFlowInput input = mock(AddFlowInput.class);
        when(input.getNode()).thenReturn(snode.getNodeRef());
        NodeRpcWatcher watcher = mock(NodeRpcWatcher.class);
        SettableFuture<RpcResult<AddFlowOutput>> future =
            SettableFuture.<RpcResult<AddFlowOutput>>create();
        SalFlowService sfs = mock(SalFlowService.class);
        when(sfs.addFlow(input)).thenReturn(future);
        AddFlowRpc rpc = new AddFlowRpc(watcher, sfs, input);
        verify(sfs).addFlow(input);
        verify(input).getNode();
        verifyNoMoreInteractions(input, sfs);

        Logger logger = mock(Logger.class);
        String format = "Caught an exception: node=%s";
        String emsg = rpc.getName() + ": Caught an exception: node=" + snode +
            ": input=" + input;
        NodeRpcErrorCallback<AddFlowOutput> cb =
            new NodeRpcErrorCallback<>(rpc, logger, format, snode);
        verify(watcher).registerRpc(rpc);
        verifyNoMoreInteractions(logger, watcher);
        Futures.addCallback(future, cb);

        IllegalStateException ise = new IllegalStateException();
        future.setException(ise);

        verify(logger).error(emsg, ise);
        verify(watcher).unregisterRpc(rpc);
        verifyNoMoreInteractions(logger, watcher);
    }

    /**
     * Test case for {@link NodeRpcErrorCallback#onFailure(Throwable)}.
     *
     * <p>
     *   The RPC invocation canceled.
     * </p>
     */
    @Test
    public void testCancel() {
        SalNode snode = new SalNode(1L);
        AddFlowInput input = mock(AddFlowInput.class);
        when(input.getNode()).thenReturn(snode.getNodeRef());
        NodeRpcWatcher watcher = mock(NodeRpcWatcher.class);
        SettableFuture<RpcResult<AddFlowOutput>> future =
            SettableFuture.<RpcResult<AddFlowOutput>>create();
        SalFlowService sfs = mock(SalFlowService.class);
        when(sfs.addFlow(input)).thenReturn(future);
        AddFlowRpc rpc = new AddFlowRpc(watcher, sfs, input);
        verify(sfs).addFlow(input);
        verify(input).getNode();
        verifyNoMoreInteractions(input, sfs);

        Logger logger = mock(Logger.class);
        NodeRpcErrorCallback<AddFlowOutput> cb =
            new NodeRpcErrorCallback<>(rpc, logger, "Unexpected log 2");
        verify(watcher).registerRpc(rpc);
        verifyNoMoreInteractions(logger, watcher);
        Futures.addCallback(future, cb);

        assertEquals(true, rpc.onNodeRemoved());

        // No error should be logged.
        verify(watcher).unregisterRpc(rpc);
        verifyNoMoreInteractions(logger, watcher);
    }
}
