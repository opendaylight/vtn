/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.flow;

import org.opendaylight.vtn.manager.internal.util.rpc.RpcInvocation;

import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.service.rev130819.AddFlowInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.service.rev130819.AddFlowOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.service.rev130819.SalFlowService;

/**
 * {@code AddFlowRpc} describes an invocation of add-flow RPC provided by the
 * MD-SAL flow service.
 */
public final class AddFlowRpc
    extends RpcInvocation<AddFlowInput, AddFlowOutput> {

    /**
     * Issue an add-flow RPC request.
     *
     * @param sfs  MD-SAL flow service.
     * @param in   The RPC input.
     */
    public AddFlowRpc(SalFlowService sfs, AddFlowInput in) {
        super(in, sfs.addFlow(in));
    }

    // RpcInvocation

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
