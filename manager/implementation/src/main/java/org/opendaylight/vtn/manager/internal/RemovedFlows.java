/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal;

import java.util.List;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.util.flow.RemoveFlowRpc;

import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.service.rev130819.SalFlowService;

/**
 * {@code RemovedFlows} describes flow entries removed by {@link FlowRemover}.
 */
public interface RemovedFlows {
    /**
     * Remove flow entries specified by this instance.
     *
     * @param ctx    A runtime context for read-only transaction task.
     * @param sfs    MD-SAL flow service.
     * @return  A list of remove-flow RPC invocations.
     * @throws VTNException  An error occurred.
     */
    List<RemoveFlowRpc> removeFlowEntries(TxContext ctx, SalFlowService sfs)
        throws VTNException;

    /**
     * Determine whether this instance is empty or not.
     *
     * @return  {@code true} if this instance does not specifies flow entry
     *          to be removed. Otherwise {@code false}.
     */
    boolean isEmpty();
}
