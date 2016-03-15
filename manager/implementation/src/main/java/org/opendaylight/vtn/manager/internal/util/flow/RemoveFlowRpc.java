/*
 * Copyright (c) 2015, 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.flow;

import static java.util.regex.Pattern.CASE_INSENSITIVE;

import java.util.Collection;
import java.util.regex.Pattern;

import javax.annotation.Nonnull;

import org.opendaylight.vtn.manager.internal.util.inventory.NodeRpcInvocation;
import org.opendaylight.vtn.manager.internal.util.inventory.NodeRpcWatcher;

import org.opendaylight.controller.md.sal.dom.api.DOMRpcImplementationNotAvailableException;

import org.opendaylight.yangtools.yang.common.RpcError;

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
     * Regular expression that matches error messages that indicate
     * disconnection of OpenFlow secure channel.
     */
    private static final Pattern  REGEXP_DISCONNECT =
        Pattern.compile("disconnected|wasn't able to reserve XID",
                        CASE_INSENSITIVE);

    /**
     * Issue an remove-flow RPC request.
     *
     * @param w    The node RPC watcher.
     * @param sfs  MD-SAL flow service.
     * @param in   The RPC input.
     */
    public RemoveFlowRpc(NodeRpcWatcher w, SalFlowService sfs,
                         RemoveFlowInput in) {
        super(w, in, in.getNode(), sfs.removeFlow(in));
    }

    // RpcInvocation

    /**
     * Determine whether the RPC failure should be logged or not.
     *
     * <p>
     *   This method returns {@code false} if the specified throwable seems to
     *   be caused by a disconnection of OpenFlow secure channel.
     * <p>
     *
     * @param cause  An throwable thrown by the RPC implementation.
     * @return  {@code true} if the RPC failure should be logged.
     *          {@code false} otherwise.
     */
    @Override
    public boolean needErrorLog(@Nonnull Throwable cause) {
        Throwable t = cause;
        do {
            if (t instanceof DOMRpcImplementationNotAvailableException) {
                return false;
            }
            t = t.getCause();
        } while (t != null);

        return super.needErrorLog(cause);
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

    /**
     * Determine whether the RPC failure should be logged or not.
     *
     * <p>
     *   This method returns {@code false} if the specified error seems to be
     *   caused by a disconnection of OpenFlow secure channel.
     * </p>
     *
     * @param errors  A collection of RPC errors returned by the RPC
     *                implementation.
     * @return  {@code true} if the RPC failure should be logged.
     *          {@code false} otherwise.
     */
    @Override
    public boolean needErrorLog(@Nonnull Collection<RpcError> errors) {
        for (RpcError re: errors) {
            String msg = re.getMessage();
            if (msg != null &&  REGEXP_DISCONNECT.matcher(msg).find()) {
                return false;
            }
        }

        return true;
    }
}
