/*
 * Copyright (c) 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.flow.add;

import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicReference;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.VTNConfig;
import org.opendaylight.vtn.manager.internal.VTNManagerProvider;
import org.opendaylight.vtn.manager.internal.util.concurrent.RunnableVTNFuture;
import org.opendaylight.vtn.manager.internal.util.flow.AddFlowRpc;

import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.service.rev130819.AddFlowInput;

import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.service.rev130819.SalFlowService;

/**
 * {@code AddMdFlowTask} describes the task that installs an MD-SAL flow entry
 * into the specified switch.
 */
public final class AddMdFlowTask extends RunnableVTNFuture<Void> {
    /**
     * Logger instance.
     */
    private static final Logger  LOG =
        LoggerFactory.getLogger(AddMdFlowTask.class);

    /**
     * VTN Manager provider service.
     */
    private final VTNManagerProvider  vtnProvider;

    /**
     * A MD-SAL flow service.
     */
    private final SalFlowService  flowService;

    /**
     * The add-flow RPC input that specifies the flow entry.
     */
    private final AddFlowInput  input;

    /**
     * The invocation of add-flow RPC.
     */
    private final AtomicReference<AddFlowRpc>  rpcInvocation =
        new AtomicReference<>();

    /**
     * Construct a new instance.
     *
     * @param provider  VTN Manager provider service.
     * @param sfs       A {@link SalFlowService} instance.
     * @param flow      An {@link AddFlowInput} instance that specifies the
     *                  flow entry.
     */
    public AddMdFlowTask(VTNManagerProvider provider, SalFlowService sfs,
                         AddFlowInput flow) {
        vtnProvider = provider;
        flowService = sfs;
        input = flow;
    }

    /**
     * Determine whether the add-flow RPC invocation was canceled due to
     * removal of the target node.
     *
     * @return  {@code true} if the add-flow RPC invocation was canceled
     *          due to removal of the target node. {@code false} otherwise.
     */
    public boolean isNodeRemoved() {
        AddFlowRpc rpc = rpcInvocation.get();
        return (rpc != null && rpc.isNodeRemoved());
    }

    // Runnable

    /**
     * Install the specified flow entry.
     */
    @Override
    public void run() {
        try {
            VTNConfig vcfg = vtnProvider.getVTNConfig();
            long timeout = (long)vcfg.getFlowModTimeout();
            AddFlowRpc rpc = new AddFlowRpc(vtnProvider, flowService, input);
            rpcInvocation.set(rpc);
            rpc.getResult(timeout, TimeUnit.MILLISECONDS, LOG);
            set(null);
        } catch (VTNException | RuntimeException e) {
            setException(e);
        }
    }
}
