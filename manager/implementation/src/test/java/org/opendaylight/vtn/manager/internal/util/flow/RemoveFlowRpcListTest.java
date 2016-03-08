/*
 * Copyright (c) 2015, 2016 NEC Corporation. All rights reserved.
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

import java.math.BigInteger;
import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map.Entry;
import java.util.Map;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.Future;
import java.util.concurrent.TimeUnit;

import org.slf4j.Logger;

import com.google.common.util.concurrent.Futures;
import com.google.common.util.concurrent.SettableFuture;

import org.junit.Test;

import org.mockito.ArgumentCaptor;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.util.inventory.NodeRpcWatcher;
import org.opendaylight.vtn.manager.internal.util.inventory.SalNode;

import org.opendaylight.vtn.manager.internal.TestBase;

import org.opendaylight.yangtools.yang.common.RpcResult;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnErrorTag;

import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.service.rev130819.RemoveFlowInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.service.rev130819.RemoveFlowInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.service.rev130819.RemoveFlowOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.service.rev130819.RemoveFlowOutputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.service.rev130819.SalFlowService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.rev131026.FlowCookie;

/**
 * JUnit test for {@link RemoveFlowRpcList}.
 */
public class RemoveFlowRpcListTest extends TestBase {
    /**
     * Test case for {@link RemoveFlowRpcList#getFlowService()}
     */
    @Test
    public void testGetFlowService() {
        NodeRpcWatcher watcher = mock(NodeRpcWatcher.class);
        SalFlowService sfs = mock(SalFlowService.class);
        RemoveFlowRpcList rpcList = new RemoveFlowRpcList(watcher, sfs);
        assertEquals(sfs, rpcList.getFlowService());
    }

    /**
     * Test case for successful completion.
     *
     * <ul>
     *   <li>{@link RemoveFlowRpcList#invoke(RemoveFlowInputBuilder)}</li>
     *   <li>{@link RemoveFlowRpcList#getRpcs()}</li>
     *   <li>{@link RemoveFlowRpcList#size()}</li>
     *   <li>{@link RemoveFlowRpcList#flush()}</li>
     *   <li>{@link RemoveFlowRpcList#verify(Logger, long, TimeUnit)}</li>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testSuccess() throws Exception {
        NodeRpcWatcher watcher = mock(NodeRpcWatcher.class);
        SalFlowService sfs = mock(SalFlowService.class);
        RemoveFlowRpcList rpcList = new RemoveFlowRpcList(watcher, sfs);
        assertEquals(Collections.<RemoveFlowRpc>emptyList(),
                     rpcList.getRpcs());
        assertEquals(0, rpcList.size());

        Logger logger = mock(Logger.class);
        rpcList.verify(logger, 1L, TimeUnit.SECONDS);
        verifyZeroInteractions(watcher, sfs, logger);

        List<RemoveFlowInput> inputs = new ArrayList<>();
        SalNode snode = new SalNode(12345L);
        long ninputs = 10L;
        for (long l = 1L; l <= ninputs; l++) {
            FlowCookie fc = new FlowCookie(BigInteger.valueOf(l));
            boolean barrier = (l == ninputs);
            RemoveFlowInput input = new RemoveFlowInputBuilder().
                setCookie(fc).
                setNode(snode.getNodeRef()).
                setBarrier(barrier).
                build();
            RemoveFlowInputBuilder builder = new RemoveFlowInputBuilder().
                setCookie(fc).
                setNode(snode.getNodeRef());
            RemoveFlowOutput output = new RemoveFlowOutputBuilder().build();
            Future<RpcResult<RemoveFlowOutput>> future = getRpcResult(output);
            when(sfs.removeFlow(input)).thenReturn(future);
            rpcList.invoke(builder);
            inputs.add(input);
        }
        rpcList.flush();
        for (RemoveFlowInput input: inputs) {
            verify(sfs).removeFlow(input);
        }
        verifyNoMoreInteractions(watcher, sfs, logger);

        assertEquals(inputs.size(), rpcList.size());
        Iterator<RemoveFlowInput> it = inputs.iterator();
        for (RemoveFlowRpc rpc: rpcList.getRpcs()) {
            assertEquals(rpc.getInput(), it.next());
        }
        assertEquals(false, it.hasNext());

        rpcList.verify(logger, 10L, TimeUnit.SECONDS);
        String msg = "remove-flow RPC has completed successfully: {}";
        for (RemoveFlowInput input: inputs) {
            verify(logger).trace(msg, input);
        }

        for (RemoveFlowRpc rpc: rpcList.getRpcs()) {
            verify(watcher).registerRpc(rpc);
            verify(watcher).unregisterRpc(rpc);
        }
        verifyNoMoreInteractions(watcher, sfs, logger);
    }

    /**
     * Test case for RPC failure.
     *
     * <ul>
     *   <li>{@link RemoveFlowRpcList#invoke(RemoveFlowInputBuilder)}</li>
     *   <li>{@link RemoveFlowRpcList#getRpcs()}</li>
     *   <li>{@link RemoveFlowRpcList#size()}</li>
     *   <li>{@link RemoveFlowRpcList#flush()}</li>
     *   <li>{@link RemoveFlowRpcList#verify(Logger, long, TimeUnit)}</li>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testFailure() throws Exception {
        NodeRpcWatcher watcher = mock(NodeRpcWatcher.class);
        SalFlowService sfs = mock(SalFlowService.class);
        RemoveFlowRpcList rpcList = new RemoveFlowRpcList(watcher, sfs);
        Logger logger = mock(Logger.class);
        VTNException first = new VTNException("first error");
        VTNException second = new VTNException("second error");
        Map<RemoveFlowInput, VTNException> errors = new HashMap<>();

        List<RemoveFlowInput> inputs = new ArrayList<>();
        SalNode snode = new SalNode(12345L);
        long ninputs = 10L;
        for (long l = 1L; l <= ninputs; l++) {
            FlowCookie fc = new FlowCookie(BigInteger.valueOf(l));
            boolean barrier = (l == ninputs);
            RemoveFlowInput input = new RemoveFlowInputBuilder().
                setNode(snode.getNodeRef()).
                setCookie(fc).
                setNode(snode.getNodeRef()).
                setBarrier(barrier).
                build();
            RemoveFlowInputBuilder builder = new RemoveFlowInputBuilder().
                setCookie(fc).
                setNode(snode.getNodeRef());
            VTNException e;
            if (l == 3L) {
                e = first;
            } else if (l == 7L) {
                e = second;
            } else {
                e = null;
            }

            Future<RpcResult<RemoveFlowOutput>> future;
            if (e == null) {
                RemoveFlowOutput output = new RemoveFlowOutputBuilder().
                    build();
                future = getRpcResult(output);
            } else {
                future = Futures.
                    <RpcResult<RemoveFlowOutput>>immediateFailedFuture(e);
                errors.put(input, e);
            }
            when(sfs.removeFlow(input)).thenReturn(future);
            rpcList.invoke(builder);
            inputs.add(input);
        }
        rpcList.flush();
        for (RemoveFlowInput input: inputs) {
            verify(sfs).removeFlow(input);
        }
        verifyNoMoreInteractions(watcher, sfs, logger);

        assertEquals(inputs.size(), rpcList.size());
        Iterator<RemoveFlowInput> it = inputs.iterator();
        for (RemoveFlowRpc rpc: rpcList.getRpcs()) {
            assertEquals(rpc.getInput(), it.next());
        }
        assertEquals(false, it.hasNext());

        try {
            rpcList.verify(logger, 10L, TimeUnit.SECONDS);
            unexpected();
        } catch (VTNException e) {
            assertEquals(first, e);
        }

        for (Entry<RemoveFlowInput, VTNException> entry: errors.entrySet()) {
            String msg = "remove-flow: Caught an exception: canceled=false," +
                " input=" + entry.getKey();
            ArgumentCaptor<ExecutionException> captor =
                ArgumentCaptor.forClass(ExecutionException.class);
            verify(logger).error(eq(msg), captor.capture());
            List<ExecutionException> wrappers = captor.getAllValues();
            assertEquals(1, wrappers.size());
            ExecutionException ee = wrappers.get(0);
            assertEquals(entry.getValue(), ee.getCause());
        }

        String msg = "remove-flow RPC has completed successfully: {}";
        for (RemoveFlowInput input: inputs) {
            if (!errors.containsKey(input)) {
                verify(logger).trace(msg, input);
            }
        }

        for (RemoveFlowRpc rpc: rpcList.getRpcs()) {
            verify(watcher).registerRpc(rpc);
            verify(watcher).unregisterRpc(rpc);
        }
        verifyNoMoreInteractions(watcher, sfs, logger);
    }

    /**
     * Test case for timeout error.
     *
     * <ul>
     *   <li>{@link RemoveFlowRpcList#invoke(RemoveFlowInputBuilder)}</li>
     *   <li>{@link RemoveFlowRpcList#getRpcs()}</li>
     *   <li>{@link RemoveFlowRpcList#size()}</li>
     *   <li>{@link RemoveFlowRpcList#flush()}</li>
     *   <li>{@link RemoveFlowRpcList#verify(Logger, long, TimeUnit)}</li>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testTimeout() throws Exception {
        NodeRpcWatcher watcher = mock(NodeRpcWatcher.class);
        SalFlowService sfs = mock(SalFlowService.class);
        RemoveFlowRpcList rpcList = new RemoveFlowRpcList(watcher, sfs);
        Logger logger = mock(Logger.class);
        RemoveFlowInput badInput = null;

        List<RemoveFlowInput> inputs = new ArrayList<>();
        SalNode snode = new SalNode(12345L);
        long ninputs = 10L;
        for (long l = 1L; l <= ninputs; l++) {
            FlowCookie fc = new FlowCookie(BigInteger.valueOf(l));
            boolean barrier = (l == ninputs);
            RemoveFlowInput input = new RemoveFlowInputBuilder().
                setCookie(fc).
                setNode(snode.getNodeRef()).
                setBarrier(barrier).
                build();
            RemoveFlowInputBuilder builder = new RemoveFlowInputBuilder().
                setCookie(fc).
                setNode(snode.getNodeRef());
            Future<RpcResult<RemoveFlowOutput>> future;
            if (l == 6L) {
                future = SettableFuture.<RpcResult<RemoveFlowOutput>>create();
                badInput = input;
            } else {
                RemoveFlowOutput output = new RemoveFlowOutputBuilder().
                    build();
                future = getRpcResult(output);
            }
            when(sfs.removeFlow(input)).thenReturn(future);
            rpcList.invoke(builder);
            inputs.add(input);
        }
        rpcList.flush();
        for (RemoveFlowInput input: inputs) {
            verify(sfs).removeFlow(input);
        }
        verifyNoMoreInteractions(watcher, sfs, logger);

        assertEquals(inputs.size(), rpcList.size());
        Iterator<RemoveFlowInput> it = inputs.iterator();
        for (RemoveFlowRpc rpc: rpcList.getRpcs()) {
            assertEquals(rpc.getInput(), it.next());
        }
        assertEquals(false, it.hasNext());

        try {
            rpcList.verify(logger, 1L, TimeUnit.SECONDS);
            unexpected();
        } catch (VTNException e) {
            assertEquals(VtnErrorTag.TIMEOUT, e.getVtnErrorTag());

            String msg = "remove-flow: Caught an exception: canceled=true," +
                " input=" + badInput;
            verify(logger).error(msg, e.getCause());
        }

        String msg = "remove-flow RPC has completed successfully: {}";
        for (RemoveFlowInput input: inputs) {
            if (!badInput.equals(input)) {
                verify(logger).trace(msg, input);
            }
        }

        for (RemoveFlowRpc rpc: rpcList.getRpcs()) {
            verify(watcher).registerRpc(rpc);
            verify(watcher).unregisterRpc(rpc);
        }
        verifyNoMoreInteractions(watcher, sfs, logger);
    }
}
