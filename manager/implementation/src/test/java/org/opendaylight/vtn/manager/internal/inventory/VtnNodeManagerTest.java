/*
 * Copyright (c) 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.inventory;

import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.when;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.concurrent.CancellationException;
import java.util.concurrent.Future;

import com.google.common.util.concurrent.Futures;
import com.google.common.util.concurrent.SettableFuture;

import org.junit.Test;

import org.opendaylight.vtn.manager.internal.util.flow.AddFlowRpc;
import org.opendaylight.vtn.manager.internal.util.flow.RemoveFlowRpc;
import org.opendaylight.vtn.manager.internal.util.inventory.NodeRpcInvocation;
import org.opendaylight.vtn.manager.internal.util.inventory.SalNode;

import org.opendaylight.vtn.manager.internal.TestBase;

import org.opendaylight.yangtools.yang.common.RpcResult;

import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.service.rev130819.AddFlowInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.service.rev130819.AddFlowOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.service.rev130819.RemoveFlowInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.service.rev130819.RemoveFlowOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.service.rev130819.SalFlowService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.nodes.VtnNode;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.nodes.VtnNodeBuilder;

/**
 * JUnit test for {@link VtnNodeManager}.
 */
public class VtnNodeManagerTest extends TestBase {
    /**
     * Test case for {@link VtnNodeManager#add(VtnNode)} and
     * {@link VtnNodeManager#remove(VtnNode)}.
     */
    @Test
    public void testAddRemove() {
        VtnNodeManager mgr = new VtnNodeManager();
        Map<String, Set<NodeRpcInvocation<?, ?>>> expected = new HashMap<>();
        assertEquals(expected, mgr.getNodeMap());

        SalNode[] snodes = {
            new SalNode(1L),
            new SalNode(2L),
            new SalNode(1234567890L),
            new SalNode(-1L),
        };

        for (SalNode snode: snodes) {
            String id = snode.toString();
            VtnNode vnode = new VtnNodeBuilder().
                setId(snode.getNodeId()).build();
            // remove(VtnNode) should return null if the node is not present.
            assertEquals(null, mgr.remove(vnode));
            assertEquals(expected, mgr.getNodeMap());

            assertEquals(id, mgr.add(vnode));
            Set<NodeRpcInvocation<?, ?>> rpcs = new HashSet<>();
            expected.put(id, rpcs);
            assertEquals(expected, mgr.getNodeMap());

            // add(VtnNode) should return null if the node is already added.
            assertEquals(null, mgr.add(vnode));
            assertEquals(expected, mgr.getNodeMap());

            // Remove the node, and then add again.
            assertEquals(id, mgr.remove(vnode));
            expected.remove(id);
            assertEquals(expected, mgr.getNodeMap());

            assertEquals(id, mgr.add(vnode));
            expected.put(id, rpcs);
            assertEquals(expected, mgr.getNodeMap());
        }
    }

    /**
     * Test case for the following methods.
     *
     * <ul>
     *   <li>{@link VtnNodeManager#registerRpc(NodeRpcInvocation)}</li>
     *   <li>{@link VtnNodeManager#unregisterRpc(NodeRpcInvocation)}</li>
     *   <li>{@link VtnNodeManager#close()}</li>
     * </ul>
     */
    @Test
    public void testRpc() {
        VtnNodeManager mgr = new VtnNodeManager();
        Map<String, Set<NodeRpcInvocation<?, ?>>> expected = new HashMap<>();

        SalNode[] snodes = {
            new SalNode(1L),
            new SalNode(2L),
            new SalNode(1234567890L),
            new SalNode(-1L),
        };

        // Register RPC invocations.
        Set<NodeRpcInvocation<?, ?>> canceled = new HashSet<>();
        CancellationException ce = new CancellationException();
        int nrpcs = 5;
        SalFlowService sfs = mock(SalFlowService.class);
        for (SalNode snode: snodes) {
            String id = snode.toString();
            VtnNode vnode = new VtnNodeBuilder().
                setId(snode.getNodeId()).build();
            Set<NodeRpcInvocation<?, ?>> rpcs = new HashSet<>();
            assertEquals(id, mgr.add(vnode));
            expected.put(id, rpcs);

            for (int i = 0; i < nrpcs; i++) {
                AddFlowInput ainput = mock(AddFlowInput.class);
                when(ainput.getNode()).thenReturn(snode.getNodeRef());
                SettableFuture<RpcResult<AddFlowOutput>> afuture =
                    SettableFuture.<RpcResult<AddFlowOutput>>create();
                when(sfs.addFlow(ainput)).thenReturn(afuture);
                AddFlowRpc af = new AddFlowRpc(mgr, sfs, ainput);
                mgr.registerRpc(af);
                assertEquals(true, rpcs.add(af));
                assertEquals(expected, mgr.getNodeMap());
                assertEquals(false, af.isNodeRemoved());
                assertEquals(true, af.needErrorLog(ce));

                RemoveFlowInput rinput = mock(RemoveFlowInput.class);
                when(rinput.getNode()).thenReturn(snode.getNodeRef());
                SettableFuture<RpcResult<RemoveFlowOutput>> rfuture =
                    SettableFuture.<RpcResult<RemoveFlowOutput>>create();
                when(sfs.removeFlow(rinput)).thenReturn(rfuture);
                RemoveFlowRpc rf = new RemoveFlowRpc(mgr, sfs, rinput);
                mgr.registerRpc(rf);
                assertEquals(true, rpcs.add(rf));
                assertEquals(expected, mgr.getNodeMap());
                assertEquals(false, rf.isNodeRemoved());
                assertEquals(true, rf.needErrorLog(ce));
            }

            // Add one more RPC invocation that is already canceled.
            // This is only for increasing test coverage.
            AddFlowInput ainput = mock(AddFlowInput.class);
            when(ainput.getNode()).thenReturn(snode.getNodeRef());
            Future<RpcResult<AddFlowOutput>> afuture =
                Futures.<RpcResult<AddFlowOutput>>immediateCancelledFuture();
            when(sfs.addFlow(ainput)).thenReturn(afuture);
            AddFlowRpc af = new AddFlowRpc(mgr, sfs, ainput);
            mgr.registerRpc(af);
            assertEquals(true, rpcs.add(af));
            assertEquals(expected, mgr.getNodeMap());
            assertEquals(true, canceled.add(af));
        }

        // RPC should be canceled immediately if the target node is not
        // present.
        SalNode[] unknownNodes = {
            new SalNode(10L),
            new SalNode(20L),
            new SalNode(1234567890123L),
            new SalNode(-2L),
        };
        for (SalNode snode: unknownNodes) {
            AddFlowInput ainput = mock(AddFlowInput.class);
            when(ainput.getNode()).thenReturn(snode.getNodeRef());
            Future<RpcResult<AddFlowOutput>> afuture =
                SettableFuture.<RpcResult<AddFlowOutput>>create();
            when(sfs.addFlow(ainput)).thenReturn(afuture);
            AddFlowRpc af = new AddFlowRpc(mgr, sfs, ainput);
            mgr.registerRpc(af);
            assertEquals(expected, mgr.getNodeMap());
            assertEquals(true, afuture.isCancelled());
            assertEquals(true, afuture.isDone());
            assertEquals(true, af.isNodeRemoved());
            assertEquals(false, af.needErrorLog(ce));

            RemoveFlowInput rinput = mock(RemoveFlowInput.class);
            when(rinput.getNode()).thenReturn(snode.getNodeRef());
            Future<RpcResult<RemoveFlowOutput>> rfuture =
                SettableFuture.<RpcResult<RemoveFlowOutput>>create();
            when(sfs.removeFlow(rinput)).thenReturn(rfuture);
            RemoveFlowRpc rf = new RemoveFlowRpc(mgr, sfs, rinput);
            mgr.registerRpc(rf);
            assertEquals(expected, mgr.getNodeMap());
            assertEquals(true, rfuture.isCancelled());
            assertEquals(true, rfuture.isDone());
            assertEquals(true, rf.isNodeRemoved());
            assertEquals(false, rf.needErrorLog(ce));
        }

        // Unregister some RPCs.
        for (int i = 0; i < snodes.length; i++) {
            String id = snodes[i].toString();
            Set<NodeRpcInvocation<?, ?>> rpcs = expected.get(id);
            Iterator<NodeRpcInvocation<?, ?>> it = rpcs.iterator();
            int size = rpcs.size();
            int nremove = (i == snodes.length - 1) ? size : 2;
            for (int j = 0; j < nremove; j++) {
                NodeRpcInvocation<?, ?> rpc = it.next();
                it.remove();
                mgr.unregisterRpc(rpc);
                assertEquals(expected, mgr.getNodeMap());
                assertEquals(false, rpc.isNodeRemoved());
                assertEquals(true, rpc.needErrorLog(ce));
                Future<?> future = rpc.getFuture();
                boolean c = canceled.contains(rpc);
                assertEquals(c, future.isCancelled());
                assertEquals(c, future.isDone());
                assertEquals(false, rpc.isNodeRemoved());
                assertEquals(true, rpc.needErrorLog(ce));
            }
            assertEquals(size - nremove, rpcs.size());
        }

        // Remove snodes[0] and snodes[2].
        // RPCs routed to the removed node should be canceled.
        SalNode[] targets = {snodes[0], snodes[2]};
        for (SalNode snode: targets) {
            String id = snode.toString();
            Set<NodeRpcInvocation<?, ?>> rpcs = expected.remove(id);
            assertEquals(false, rpcs.isEmpty());
            for (NodeRpcInvocation<?, ?> rpc: rpcs) {
                boolean c = canceled.contains(rpc);
                Future<?> future = rpc.getFuture();
                assertEquals(c, future.isCancelled());
                assertEquals(c, future.isDone());
                assertEquals(false, rpc.isNodeRemoved());
                assertEquals(true, rpc.needErrorLog(ce));
            }

            VtnNode vnode = new VtnNodeBuilder().
                setId(snode.getNodeId()).build();
            assertEquals(id, mgr.remove(vnode));
            assertEquals(expected, mgr.getNodeMap());

            for (NodeRpcInvocation<?, ?> rpc: rpcs) {
                Future<?> future = rpc.getFuture();
                assertEquals(true, future.isCancelled());
                assertEquals(true, future.isDone());
                assertEquals(true, rpc.isNodeRemoved());
                assertEquals(false, rpc.needErrorLog(ce));
            }
        }

        // Close the VTN node manager.
        // All the RPCs should be canceled.
        List<NodeRpcInvocation<?, ?>> rpcList = new ArrayList<>();
        for (Set<NodeRpcInvocation<?, ?>> rpcs: expected.values()) {
            for (NodeRpcInvocation<?, ?> rpc: rpcs) {
                boolean c = canceled.contains(rpc);
                Future<?> future = rpc.getFuture();
                assertEquals(c, future.isCancelled());
                assertEquals(c, future.isDone());
                assertEquals(false, rpc.isNodeRemoved());
                assertEquals(true, rpc.needErrorLog(ce));
                rpcList.add(rpc);
            }
        }

        mgr.close();
        expected.clear();
        assertEquals(expected, mgr.getNodeMap());

        for (NodeRpcInvocation<?, ?> rpc: rpcList) {
            Future<?> future = rpc.getFuture();
            assertEquals(true, future.isCancelled());
            assertEquals(true, future.isDone());
            assertEquals(true, rpc.isNodeRemoved());
            assertEquals(false, rpc.needErrorLog(ce));
        }
    }
}
