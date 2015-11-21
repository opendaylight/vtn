/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.flow.stats;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import com.google.common.base.Optional;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.TxContext;
import org.opendaylight.vtn.manager.internal.VTNManagerProvider;
import org.opendaylight.vtn.manager.internal.util.DataStoreUtils;
import org.opendaylight.vtn.manager.internal.util.flow.FlowStatsUtils;
import org.opendaylight.vtn.manager.internal.util.flow.FlowUtils;
import org.opendaylight.vtn.manager.internal.util.tx.AbstractTxTask;

import org.opendaylight.controller.md.sal.binding.api.ReadWriteTransaction;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.VtnFlowId;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.tenant.flow.info.VtnDataFlow;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.vtn.data.flow.fields.FlowStatsHistory;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.vtn.data.flow.fields.flow.stats.history.FlowStatsRecord;

/**
 * {@code AddFlowStatsTask} describes the MD-SAL datastore transaction task
 * that adds the given flow statistics history record to the VTN data flow.
 *
 * <p>
 *   This task returns {@link Boolean#TRUE} if the given flow statistics was
 *   added successfully.
 * </p>
 */
public final class AddFlowStatsTask extends AbstractTxTask<Boolean> {
    /**
     * Logger instance.
     */
    public static final Logger  LOG =
        LoggerFactory.getLogger(AddFlowStatsTask.class);

    /**
     * The name of the target VTN.
     */
    private final String  tenantName;

    /**
     * The identifier of the target VTN data flow.
     */
    private final VtnFlowId  flowId;

    /**
     * The flow statistics record to be added.
     */
    private final FlowStatsRecord  statsRecord;

    /**
     * Construct a new instance.
     *
     * @param tname  The name of the target VTN.
     * @param fid    The identifier of the target VTN data flow.
     * @param fsr    The flow statistics record to be added.
     */
    public AddFlowStatsTask(String tname, VtnFlowId fid, FlowStatsRecord fsr) {
        tenantName = tname;
        flowId = fid;
        statsRecord = fsr;
    }

    /**
     * Update the flow statistics history.
     *
     * @param ctx  A runtime context for MD-SAL datastore transaction task.
     * @return  {@link Boolean#TRUE} if the flow statistics history was
     *          updated. {@link Boolean#FALSE} if not updated.
     * @throws VTNException  An error occurred.
     */
    private Boolean update(TxContext ctx) throws VTNException {
        // Read the current statistics history.
        InstanceIdentifier<FlowStatsHistory> path = FlowUtils.
            getIdentifier(tenantName, flowId).
            child(FlowStatsHistory.class);
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        ReadWriteTransaction tx = ctx.getReadWriteTransaction();
        Optional<FlowStatsHistory> opt = DataStoreUtils.read(tx, oper, path);
        FlowStatsHistory history;
        if (opt.isPresent()) {
            history = opt.get();
        } else {
            // Check to see if the target VTN data flow is present.
            InstanceIdentifier<VtnDataFlow> fpath =
                path.firstIdentifierOf(VtnDataFlow.class);
            Optional<VtnDataFlow> fopt = DataStoreUtils.read(tx, oper, fpath);
            if (!fopt.isPresent()) {
                // The target VTN data flow is no longer present.
                return Boolean.FALSE;
            }
            history = null;
        }

        // Update the statistics history.
        FlowStatsHistory newHist =
            FlowStatsUtils.addNonPeriodic(history, statsRecord);
        if (newHist != null) {
            tx.put(oper, path, newHist, false);
            return Boolean.TRUE;
        }

        return Boolean.FALSE;
    }

    // AbstractTxTask

    /**
     * {@inheritDoc}
     */
    @Override
    public Boolean execute(TxContext ctx) throws VTNException {
        Boolean ret = update(ctx);
        if (ret == null) {
            ctx.cancelTransaction();
        }

        return ret;
    }

    // TxTask

    /**
     * Invoked when the task has completed successfully.
     *
     * @param provider  VTN Manager provider service.
     * @param result    The result of this task.
     */
    @Override
    public void onSuccess(VTNManagerProvider provider, Boolean result) {
        if (Boolean.TRUE.equals(result)) {
            LOG.debug("Flow statistics has been added: id={}, time={}",
                      flowId.getValue(), statsRecord.getTime());
        } else if (LOG.isTraceEnabled()) {
            LOG.trace("Flow statistics was ignored: id={}, record={}",
                      flowId.getValue(), statsRecord);
        }
    }

    /**
     * Invoked when the task has failed.
     *
     * @param provider  VTN Manager provider service.
     * @param t         A {@link Throwable} thrown by the task.
     */
    @Override
    public void onFailure(VTNManagerProvider provider, Throwable t) {
        StringBuilder builder =
            new StringBuilder("Failed to add flow statistics: id=");
        String msg = builder.append(flowId.getValue()).
            append(", record=").append(statsRecord).toString();
        LOG.error(msg, t);
    }
}
