/*
 * Copyright (c) 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.flow;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import com.google.common.util.concurrent.FutureCallback;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.util.concurrent.AbstractVTNFuture;
import org.opendaylight.vtn.manager.internal.util.inventory.NodeRpcInvocation;
import org.opendaylight.vtn.manager.internal.util.inventory.NodeRpcWatcher;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcUtils;

import org.opendaylight.yangtools.yang.common.RpcResult;

import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.NodeRef;

import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.transaction.rev150304.FlowCapableTransactionService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.transaction.rev150304.SendBarrierInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.transaction.rev150304.SendBarrierInputBuilder;

/**
 * {@code SendBarrierRpc} describes an invocation of send-barrier RPC provided
 * by the flow-capable-transaction service.
 */
public final class SendBarrierRpc
    extends NodeRpcInvocation<SendBarrierInput, Void>
    implements FutureCallback<RpcResult<Void>> {
    /**
     * Logger instance.
     */
    private static final Logger  LOG =
        LoggerFactory.getLogger(SendBarrierRpc.class);

    /**
     * Send a barrier request to the specified node.
     *
     * @param w     The node RPC watcher.
     * @param fcts  MD-SAL flow-capable-transaction service.
     * @param node  Reference to the target node.
     * @return  A {@link SendBarrierRpc} instance.
     */
    public static SendBarrierRpc create(
        NodeRpcWatcher w, FlowCapableTransactionService fcts, NodeRef node) {
        SendBarrierInput input = new SendBarrierInputBuilder().
            setNode(node).build();
        return new SendBarrierRpc(w, fcts, input);
    }

    /**
     * Issue a send-barrier RPC request.
     *
     * @param w     The node RPC watcher.
     * @param fcts  MD-SAL flow-capable-transaction service.
     * @param in    The RPC input.
     */
    public SendBarrierRpc(
        NodeRpcWatcher w, FlowCapableTransactionService fcts,
        SendBarrierInput in) {
        super(w, in, in.getNode(), fcts.sendBarrier(in));
    }

    // RpcRequest

    /**
     * Return the name of the RPC.
     *
     * @return  "send-barrier".
     */
    @Override
    public String getName() {
        return "send-barrier";
    }

    // FutureCallback

    /**
     * Invoked when a barrier request has been processed successfully.
     *
     * @param result  The result of the send-barrier RPC.
     */
    @Override
    public void onSuccess(RpcResult<Void> result) {
        try {
            RpcUtils.checkResult(this, result, LOG);
            LOG.trace("send-barrier RPC has completed: node={}", getNode());
        } catch (VTNException e) {
            LOG.error("send-barrier RPC has failed: node=" + getNode(), e);
        }
    }

    /**
     * Invoked when a barrier request has failed.
     *
     * @param t  A throwable that indicates the cause of failure.
     */
    @Override
    public void onFailure(Throwable t) {
        Throwable cause = AbstractVTNFuture.getCause(t);
        LOG.error("send-barrier RPC has failed: node=" + getNode(), cause);
    }
}
