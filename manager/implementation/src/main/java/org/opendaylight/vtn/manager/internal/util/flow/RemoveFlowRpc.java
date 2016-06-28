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

import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.service.rev130819.RemoveFlowInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.service.rev130819.RemoveFlowOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.service.rev130819.SalFlowService;

/**
 * {@code RemoveFlowRpc} describes an invocation of remove-flow RPC provided
 * by the MD-SAL flow service.
 */
public final class RemoveFlowRpc
    extends NodeRpcInvocation<RemoveFlowInput, RemoveFlowOutput> {
    /**
     * Issue an remove-flow RPC request.
     *
     * @param w    The node RPC watcher.
     * @param sfs  MD-SAL flow service.
     * @param in   The RPC input.
     */
    public RemoveFlowRpc(NodeRpcWatcher w, SalFlowService sfs,
                         RemoveFlowInput in) {
        super(w, in, in.getNode(), sfs.removeFlow(in), false);
    }

    // RpcRequest

    /**
     * Return the name of the RPC.
     *
     * @return  "remove-flow".
     */
    @Override
    public String getName() {
        return "remove-flow";
    }
}
