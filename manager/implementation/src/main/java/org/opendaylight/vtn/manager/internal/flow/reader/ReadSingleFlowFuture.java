/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.flow.reader;

import java.util.Collections;

import javax.annotation.Nonnull;

import com.google.common.base.Optional;
import com.google.common.util.concurrent.FutureCallback;
import com.google.common.util.concurrent.Futures;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.TxContext;
import org.opendaylight.vtn.manager.internal.TxQueue;
import org.opendaylight.vtn.manager.internal.flow.stats.StatsReaderService;
import org.opendaylight.vtn.manager.internal.util.concurrent.VTNFuture;
import org.opendaylight.vtn.manager.internal.util.flow.FlowCache;
import org.opendaylight.vtn.manager.internal.util.flow.FlowUtils;

import org.opendaylight.controller.md.sal.binding.api.ReadTransaction;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.GetDataFlowInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.get.data.flow.output.DataFlowInfo;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.tenant.flow.info.VtnDataFlow;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.vtn.data.flow.fields.VtnFlowEntry;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.vtn.data.flow.fields.flow.stats.history.FlowStatsRecord;

/**
 * {@code ReadSingleFlowFuture} describes a future associated with the task
 * which reads a single data flow information.
 */
public final class ReadSingleFlowFuture extends ReadFlowFuture
    implements FutureCallback<Optional<VtnDataFlow>> {
    /**
     * Construct a new instance.
     *
     * @param ctx    A {@link TxContext} instance.
     * @param txq    A {@link TxQueue} instance used to update the MD-SAL
     *               datastore.
     * @param srs    The flow statistics reader service.
     * @param input  Input of the RPC call.
     * @throws VTNException  An error occurred.
     */
    public ReadSingleFlowFuture(TxContext ctx, TxQueue txq,
                                StatsReaderService srs, GetDataFlowInput input)
        throws VTNException {
        super(ctx, txq, srs, input);

        if (!isDone()) {
            InstanceIdentifier<VtnDataFlow> path = FlowUtils.
                getIdentifier(input.getTenantName(), input.getFlowId());
            LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
            ReadTransaction rtx = ctx.getTransaction();
            Futures.addCallback(rtx.read(oper, path), this);
        }
    }

    /**
     * Return a future associated with a task which reads flow statistics for
     * the target VTN data flow.
     *
     * @param fc  The target VTN data flow.
     * @return  A future which will return flow statistics for the target
     *          VTN data flow. Note that {@code null} is returned if flow
     *          statistics does not need to be read.
     */
    private VTNFuture<FlowStatsRecord> readStats(FlowCache fc) {
        StatsReaderService srs = getStatsReaderService();
        if (srs != null) {
            // Try to update statistics for the ingress flow entry.
            VtnFlowEntry ingress = fc.getIngressFlow();
            if (ingress != null) {
                return srs.get(ingress);
            }
        }

        return null;
    }

    // FutureCallback

    /**
     * Invoked when the read operation has completed successfully.
     *
     * @param result  An {@link Optional} instance that contains the VTN data
     *                flow information.
     */
    @Override
    public void onSuccess(@Nonnull Optional<VtnDataFlow> result) {
        if (result.isPresent()) {
            // Ensure that this data flow meets the condition.
            FlowCache fc = new FlowCache(result.get());
            if (select(fc)) {
                DataFlowInfo df = toDataFlowInfo(fc, readStats(fc));
                if (df != null) {
                    setResult(Collections.singletonList(df));
                }
                return;
            }
        } else if (!checkTenant()) {
            // The target VTN is not present.
            return;
        }

        notFound();
    }

    /**
     * Invoked when the read operation has failed.
     *
     * @param cause  A {@link Throwable} that indicates the cause of failure.
     */
    @Override
    public void onFailure(Throwable cause) {
        setFailure(cause);
    }
}
