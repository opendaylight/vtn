/*
 * Copyright (c) 2015, 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.flow;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.concurrent.TimeUnit;

import javax.annotation.Nonnull;

import org.slf4j.Logger;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.service.rev130819.RemoveFlowInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.service.rev130819.RemoveFlowInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.service.rev130819.SalFlowService;

/**
 * {@code RemoveFlowRpcList} describes a list of RPC calls that remove flow
 * entries in a single transaction.
 */
public final class RemoveFlowRpcList {
    /**
     * MD-SAL flow service.
     */
    private final SalFlowService  flowService;

    /**
     * A list of remove-flow RPC invocations.
     */
    private final List<RemoveFlowRpc>  rpcList = new ArrayList<>();

    /**
     * Construct a new instance.
     *
     * @param sfs   MD-SAL flow service.
     */
    public RemoveFlowRpcList(SalFlowService sfs) {
        flowService = sfs;
    }

    /**
     * Return the MD-SAL flow service.
     *
     * @return  MD-SAL flow service.
     */
    public SalFlowService getFlowService() {
        return flowService;
    }

    /**
     * Return an unmodifable list of RPC invocations.
     *
     * @return  A list of remove-flow RPC invocations.
     */
    public List<RemoveFlowRpc> getRpcs() {
        return Collections.unmodifiableList(rpcList);
    }

    /**
     * Return the number of RPC invocations in this instance.
     *
     * @return  The number of RPC invocations in this instance.
     */
    public int size() {
        return rpcList.size();
    }

    /**
     * Invoke a remove-flow RPC.
     *
     * <p>
     *   Note that this method always set barrier flag into the specified
     *   remove-flow input builder.
     * </p>
     *
     * @param builder  A {@link RemoveFlowInputBuilder} instance.
     */
    public void invoke(@Nonnull RemoveFlowInputBuilder builder) {
        RemoveFlowInput input = builder.setBarrier(true).build();
        rpcList.add(new RemoveFlowRpc(flowService, input));
    }

    /**
     * Verify the result of remove-flow RPC.
     *
     * @param logger   A logger instance.
     * @param timeout  The number of time units to wait for completion of
     *                 RPC invocations.
     * @param unit     The time unit for {@code timeout}.
     * @throws VTNException  An error occurred.
     */
    public void verify(Logger logger, long timeout, TimeUnit unit)
        throws VTNException {
        long nsec = unit.toNanos(timeout);
        long deadline = System.nanoTime() + nsec;
        TimeUnit nano = TimeUnit.NANOSECONDS;

        VTNException firstError = null;
        for (RemoveFlowRpc rpc: rpcList) {
            try {
                rpc.getResult(nsec, nano, logger);
            } catch (VTNException e) {
                if (firstError == null) {
                    firstError = e;
                }
                continue;
            }

            logger.trace("remove-flow RPC has completed successfully: {}",
                         rpc.getInput());

            nsec = deadline - System.nanoTime();
            if (nsec <= 0) {
                // Wait one more millisecond.
                nsec = TimeUnit.MILLISECONDS.toNanos(1L);
            }
        }

        if (firstError != null) {
            throw firstError;
        }
    }
}
