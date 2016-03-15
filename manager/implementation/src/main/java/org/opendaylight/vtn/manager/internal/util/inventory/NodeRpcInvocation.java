/*
 * Copyright (c) 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.inventory;

import java.util.concurrent.Future;
import java.util.concurrent.TimeUnit;

import javax.annotation.Nonnull;

import org.slf4j.Logger;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.util.rpc.RpcInvocation;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;
import org.opendaylight.yangtools.yang.common.RpcResult;

import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.NodeRef;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.nodes.Node;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.nodes.NodeKey;

/**
 * {@code NodeRpcInvocation} describes an invocation of RPC routed to the
 * specific switch.
 *
 * @param <I>  The type of the RPC input.
 * @param <O>  The type of the RPC output.
 */
public abstract class NodeRpcInvocation<I, O> extends RpcInvocation<I, O> {
    /**
     * The RPC watcher.
     */
    private final NodeRpcWatcher  rpcWatcher;

    /**
     * The node-id value that specifies the target switch.
     */
    private final String  nodeId;

    /**
     * Determine whether the target node was removed or not.
     */
    private boolean  nodeRemoved;

    /**
     * Construct a new instance.
     *
     * @param w     The node RPC watcher.
     * @param in    The RPC input.
     * @param nref  Reference to the target node.
     * @param f     The future associated with the RPC execution with
     *              {@code in}.
     */
    public NodeRpcInvocation(NodeRpcWatcher w, I in, NodeRef nref,
                             Future<RpcResult<O>> f) {
        super(in, f);
        rpcWatcher = w;

        InstanceIdentifier<?> path = nref.getValue();
        NodeKey key = path.firstKeyOf(Node.class);
        nodeId = key.getId().getValue();
    }

    /**
     * Return the node RPC watcher.
     *
     * @return  The node RPC watcher.
     */
    public final NodeRpcWatcher getNodeRpcWatcher() {
        return rpcWatcher;
    }

    /**
     * Return the identifier of the node associated with the RPC invocation.
     *
     * @return  The node-id value that specifies the target switch.
     */
    public final String getNode() {
        return nodeId;
    }

    /**
     * Invoked when the target node has been removed.
     *
     * @return  {@code true} if the RPC invocation has been canceled.
     *          {@code false} if the RPC invocation has already completed.
     */
    public final boolean onNodeRemoved() {
        nodeRemoved = true;
        return getFuture().cancel(true);
    }

    /**
     * Determine whether the target node was removed or not.
     *
     * @return  {@code true} if the target node was removed.
     *          {@code false} otherwise.
     */
    public final boolean isNodeRemoved() {
        return nodeRemoved;
    }

    /**
     * Start the observation of the RPC invocation.
     */
    final void start() {
        rpcWatcher.registerRpc(this);
    }

    /**
     * Complete the observation of the RPC invocation.
     */
    final void complete() {
        rpcWatcher.unregisterRpc(this);
    }

    // RpcInvocation

    /**
     * Wait for the RPC execution to complete, and return the result.
     *
     * <p>
     *   The RPC invocation will be canceled if the target node is removed.
     * </p>
     *
     * @param timeout  The maximum time to wait.
     * @param unit     The time unit of the {@code timeout} argument.
     * @param logger   A {@link Logger} instance.
     * @return  The result of the RPC.
     * @throws VTNException  An error occurred.
     */
    @Override
    public final O getResult(long timeout, TimeUnit unit, Logger logger)
        throws VTNException {
        start();
        try {
            return super.getResult(timeout, unit, logger);
        } finally {
            complete();
        }
    }

    /**
     * Determine whether the RPC failure should be logged or not.
     *
     * <p>
     *   This method returns {@code false} only if the RPC was canceled
     *   due to removal of the target node.
     * </p>
     *
     * @param cause  An throwable thrown by the RPC implementation.
     * @return  {@code true} if the RPC failure should be logged.
     *          {@code false} otherwise.
     */
    @Override
    public boolean needErrorLog(@Nonnull Throwable cause) {
        return !nodeRemoved;
    }
}
