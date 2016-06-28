/*
 * Copyright (c) 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.inventory;

import static java.util.regex.Pattern.CASE_INSENSITIVE;

import java.util.Collection;
import java.util.concurrent.Future;
import java.util.concurrent.TimeUnit;
import java.util.regex.Pattern;

import javax.annotation.Nonnull;

import org.slf4j.Logger;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.util.rpc.RpcInvocation;

import org.opendaylight.controller.md.sal.dom.api.DOMRpcImplementationNotAvailableException;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;
import org.opendaylight.yangtools.yang.common.RpcError;
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
     * Regular expression that matches error messages that indicate
     * disconnection of OpenFlow secure channel.
     */
    private static final Pattern  REGEXP_DISCONNECT =
        Pattern.compile("disconnected|wasn't able to reserve XID",
                        CASE_INSENSITIVE);

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
     * Determine whether the secure channel for the target node was
     * disconnected or not.
     */
    private boolean  disconnected;

    /**
     * Determine whether an error caused by disconnection of OpenFlow secure
     * channel should be logged or not.
     */
    private final boolean  disconnectionLog;

    /**
     * Construct a new instance.
     *
     * <p>
     *   An error caused by secure channel disconnection will be logged.
     * </p>
     *
     * @param w     The node RPC watcher.
     * @param in    The RPC input.
     * @param nref  Reference to the target node.
     * @param f     The future associated with the RPC execution with
     *              {@code in}.
     */
    public NodeRpcInvocation(NodeRpcWatcher w, I in, NodeRef nref,
                             Future<RpcResult<O>> f) {
        this(w, in, nref, f, true);
    }

    /**
     * Construct a new instance.
     *
     * @param w       The node RPC watcher.
     * @param in      The RPC input.
     * @param nref    Reference to the target node.
     * @param f       The future associated with the RPC execution with
     *                {@code in}.
     * @param discon  Determine whether an error caused by disconnection of
     *                OpenFlow secure channel should be logged or not.
     */
    public NodeRpcInvocation(NodeRpcWatcher w, I in, NodeRef nref,
                             Future<RpcResult<O>> f, boolean discon) {
        super(in, f);
        rpcWatcher = w;

        InstanceIdentifier<?> path = nref.getValue();
        NodeKey key = path.firstKeyOf(Node.class);
        nodeId = key.getId().getValue();
        disconnectionLog = discon;
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
     * Determine whether the secure channel for the target node was
     * disconnected or not.
     *
     * @return  {@code true} if the secure channel for the target node was
     *          disconnected. {@code false} otherwise.
     */
    public final boolean isDisconnected() {
        return disconnected;
    }

    /**
     * Start the observation of the RPC invocation.
     */
    public final void start() {
        rpcWatcher.registerRpc(this);
    }

    /**
     * Complete the observation of the RPC invocation.
     */
    public final void complete() {
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
     * {@inheritDoc}
     */
    @Override
    public final boolean needErrorLog(@Nonnull Throwable cause) {
        Throwable t = cause;
        do {
            if (t instanceof DOMRpcImplementationNotAvailableException) {
                disconnected = true;
                break;
            }
            t = t.getCause();
        } while (t != null);

        return (nodeRemoved) ? false : (disconnectionLog || !disconnected);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public boolean needErrorLog(@Nonnull Collection<RpcError> errors) {
        // Check to see if the given throwable indicates the disconnection
        // of the secure channel.
        for (RpcError re: errors) {
            String msg = re.getMessage();
            if (msg != null && REGEXP_DISCONNECT.matcher(msg).find()) {
                disconnected = true;
                break;
            }
        }

        return (disconnectionLog || !disconnected);
    }
}
