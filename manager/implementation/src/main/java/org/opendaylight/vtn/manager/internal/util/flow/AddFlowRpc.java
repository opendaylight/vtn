/*
 * Copyright (c) 2015, 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.flow;

import org.opendaylight.vtn.manager.internal.util.inventory.NodeRpcInvocation;
import org.opendaylight.vtn.manager.internal.util.inventory.NodeRpcWatcher;

import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.service.rev130819.AddFlowInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.service.rev130819.AddFlowOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.service.rev130819.SalFlowService;

/**
 * {@code AddFlowRpc} describes an invocation of add-flow RPC provided by the
 * MD-SAL flow service.
 */
public final class AddFlowRpc
    extends NodeRpcInvocation<AddFlowInput, AddFlowOutput> {
    /**
     * Issue an add-flow RPC request.
     *
     * <p>
     *   A barrier request will be sent in background after issueing add-flow
     *   RPC request.
     * </p>
     *
     * @param w    The flow RPC watcher.
     * @param sfs  MD-SAL flow service.
     * @param in   The RPC input.
     * @return  An {@link AddFlowRpc} instance associated with an add-flow
     *          RPC invocation.
     */
    public static AddFlowRpc create(FlowRpcWatcher w, SalFlowService sfs,
                                    AddFlowInput in) {
        AddFlowRpc rpc = new AddFlowRpc(w, sfs, in);
        w.asyncBarrier(in.getNode());
        return rpc;
    }

    /**
     * Issue an add-flow RPC request.
     *
     * @param w    The node RPC watcher.
     * @param sfs  MD-SAL flow service.
     * @param in   The RPC input.
     */
    public AddFlowRpc(NodeRpcWatcher w, SalFlowService sfs, AddFlowInput in) {
        super(w, in, in.getNode(), sfs.addFlow(in));
    }

    // RpcRequest

    /**
     * Return the name of the RPC.
     *
     * @return  "add-flow".
     */
    @Override
    public String getName() {
        return "add-flow";
    }
}
