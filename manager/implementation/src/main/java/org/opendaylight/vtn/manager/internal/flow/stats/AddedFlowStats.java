/*
 * Copyright (c) 2015, 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.flow.stats;

import java.util.Collections;
import java.util.HashMap;
import java.util.Map.Entry;
import java.util.Map;
import java.util.Objects;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.TxContext;
import org.opendaylight.vtn.manager.internal.TxQueue;
import org.opendaylight.vtn.manager.internal.VTNManagerProvider;
import org.opendaylight.vtn.manager.internal.util.IdentifiedData;
import org.opendaylight.vtn.manager.internal.util.MiscUtils;
import org.opendaylight.vtn.manager.internal.util.flow.FlowCache;
import org.opendaylight.vtn.manager.internal.util.flow.FlowFinder;
import org.opendaylight.vtn.manager.internal.util.flow.FlowStatsUtils;
import org.opendaylight.vtn.manager.internal.util.flow.FlowUtils;
import org.opendaylight.vtn.manager.internal.util.flow.match.FlowMatchUtils;
import org.opendaylight.vtn.manager.internal.util.inventory.SalNode;
import org.opendaylight.vtn.manager.internal.util.log.VTNLogLevel;
import org.opendaylight.vtn.manager.internal.util.tx.AbstractTxTask;

import org.opendaylight.controller.md.sal.binding.api.ReadWriteTransaction;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.VtnFlowId;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.tenant.flow.info.VtnDataFlow;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.tenant.flow.info.VtnDataFlowBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.vtn.data.flow.fields.FlowStatsHistoryBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.vtn.data.flow.fields.VtnFlowEntry;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.vtn.data.flow.fields.flow.stats.history.FlowStatsRecord;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.vtn.data.flow.fields.flow.stats.history.FlowStatsRecordBuilder;

import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.inventory.rev130819.FlowId;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.inventory.rev130819.tables.table.Flow;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.statistics.rev130819.FlowStatisticsData;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.rev131026.FlowCookie;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.NodeConnectorId;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.nodes.Node;
import org.opendaylight.yang.gen.v1.urn.opendaylight.model.statistics.types.rev130925.GenericStatistics;

/**
 * {@code AddedFlowStats} keeps a list of flow statistics added by the
 * MD-SAL statistics manager.
 */
public final class AddedFlowStats extends AbstractTxTask<Void> {
    /**
     * Logger instance.
     */
    private static final Logger  LOG =
        LoggerFactory.getLogger(AddedFlowStats.class);

    /**
     * A map that keeps newly added flow information.
     */
    private final Map<VtnFlowId, Map<SalNode, Flow>>  addedFlows =
        new HashMap<>();

    /**
     * The system time when the flow statistics are collected.
     */
    private final Long  systemTime = System.currentTimeMillis();

    /**
     * A {@link FlowFinder} instance used to determine the VTN data flow
     * associated with the VTN flow ID.
     */
    private FlowFinder  finder;

    /**
     * Add the given flow information into this instance.
     *
     * @param data  An {@link IdentifiedData} instance which contains
     *              flow statistics.
     */
    public void add(IdentifiedData<Flow> data) {
        Flow flow = data.getValue();
        FlowCookie cookie = flow.getCookie();
        VtnFlowId vtnId = FlowUtils.getVtnFlowId(cookie);
        if (vtnId == null) {
            LOG.debug("{}: Unwanted flow cookie: {}",
                      flow.getId().getValue(), cookie);
        } else {
            InstanceIdentifier<Flow> path = data.getIdentifier();
            SalNode snode = SalNode.create(path.firstKeyOf(Node.class));
            if (snode == null) {
                LOG.debug("{}: Invalid flow path: {}",
                          flow.getId().getValue(), path);
            } else {
                Map<SalNode, Flow> flowMap = addedFlows.get(vtnId);
                if (flowMap == null) {
                    flowMap = new HashMap<>();
                    addedFlows.put(vtnId, flowMap);
                }
                flowMap.put(snode, flow);
            }
        }
    }

    /**
     * Resolve MD-SAL flow IDs.
     *
     * @param txq  A {@link TxQueue} instance to run operation.
     */
    public void apply(TxQueue txq) {
        if (!addedFlows.isEmpty()) {
            txq.post(this);
        }
    }

    /**
     * Search the specified MD-SAL flow map for the ingress flow entry of the
     * specified VTN data flow.
     *
     * @param ctx      MD-SAL datastore transaction context.
     * @param vdf      The VTN data flow.
     * @param flowMap  A map that contains MD-SAL flows associated with the
     *                 specified VTN data flow.
     * @return  A MD-SAL flow entry associated with the ingress flow entry
     *          if found. {@code null} if not found.
     * @throws VTNException  An error occurred.
     */
    private Flow getIngressFlow(TxContext ctx, VtnDataFlow vdf,
                                Map<SalNode, Flow> flowMap)
        throws VTNException {
        FlowCache fc = new FlowCache(vdf);
        VtnFlowEntry vfent = fc.getIngressFlow();
        Flow flow;
        if (vfent == null) {
            ctx.log(LOG, VTNLogLevel.WARN, "{}: No ingress flow entry.",
                    vdf.getFlowId().getValue());
            flow = null;
        } else {
            SalNode ingressNode = SalNode.create(vfent.getNode());
            flow = flowMap.get(ingressNode);
            if (flow == null) {
                ctx.log(LOG, VTNLogLevel.TRACE,
                        "{}: No ingress flow in added flow entries.",
                        vdf.getFlowId().getValue());
            } else {
                // Verify the ingress flow entry.
                flow = verifyIngressFlow(ctx, vdf, vfent, flow);
            }
        }

        return flow;
    }

    /**
     * Verify the ingress flow entry of the VTN data flow.
     *
     * @param ctx    MD-SAL datastore transaction context.
     * @param vdf    The VTN data flow.
     * @param vfent  The ingress flow entry of the VTN data flow.
     * @param flow   The MD-SAL flow entry associated with the ingress flow
     *               entry.
     * @return  {@code flow} on success. {@code null} on failure.
     */
    private Flow verifyIngressFlow(TxContext ctx, VtnDataFlow vdf,
                                   VtnFlowEntry vfent, Flow flow) {
        // One VTN data flow should never installs more than one flow
        // entry into the same node.
        NodeConnectorId vport =
            FlowMatchUtils.getIngressPort(vfent.getMatch());
        NodeConnectorId port = FlowMatchUtils.getIngressPort(flow.getMatch());
        Flow result = null;
        if (MiscUtils.equalsUri(vport, port)) {
            // Verify flow priority.
            Integer vpri = vfent.getPriority();
            Integer pri = flow.getPriority();
            if (Objects.equals(vpri, pri)) {
                result = flow;
            } else {
                ctx.log(LOG, VTNLogLevel.WARN,
                        "{}: Priority does not match: pri={}, expected={}",
                        vdf.getFlowId().getValue(), pri, vpri);
            }
        } else {
            ctx.log(LOG, VTNLogLevel.WARN,
                    "{}: IN_PORT does not match: port={}, expected={}",
                    vdf.getFlowId().getValue(), port, vport);
        }

        return result;
    }

    /**
     * Set flow statistics into the given VTN data flow builder if available.
     *
     * @param ctx      MD-SAL datastore transaction context.
     * @param builder  A {@link VtnDataFlowBuilder} instance.
     * @param flow     A {@link Flow} instance.
     */
    private void setStatistics(TxContext ctx, VtnDataFlowBuilder builder,
                               Flow flow) {
        // Do nothing if the given data flow already has a flow statistics.
        if (builder.getFlowStatsHistory() == null) {
            FlowStatisticsData data =
                flow.getAugmentation(FlowStatisticsData.class);
            if (data != null) {
                GenericStatistics fstats = data.getFlowStatistics();
                String err = FlowStatsUtils.check(fstats);
                if (err == null) {
                    FlowStatsRecord fsr = new FlowStatsRecordBuilder(fstats).
                        setTime(systemTime).build();
                    FlowStatsHistoryBuilder sb = new FlowStatsHistoryBuilder().
                        setFlowStatsRecord(Collections.singletonList(fsr));
                    builder.setFlowStatsHistory(sb.build());
                } else {
                    ctx.log(LOG, VTNLogLevel.WARN,
                            "{}: {}: No flow statistics: {}",
                            builder.getFlowId().getValue(),
                            flow.getId().getValue(), err);
                }
            }
        }
    }

    /**
     * Try to resolve MD-SAL flow ID to be associated with the specified
     * VTN data flow.
     *
     * @param ctx      MD-SAL datastore transaction context.
     * @param vtnId    Identifier of the VTN data flow.
     * @param flowMap  A map that contains MD-SAL flows associated with the
     *                 specified VTN data flow.
     * @throws VTNException  An error occurred.
     */
    private void resolve(TxContext ctx, VtnFlowId vtnId,
                         Map<SalNode, Flow> flowMap) throws VTNException {
        IdentifiedData<VtnDataFlow> vdata = finder.find(vtnId);
        if (vdata == null) {
            ctx.log(LOG, VTNLogLevel.WARN, "{}: Data flow not found.",
                    vtnId.getValue());
            return;
        }

        // Determine the ingress flow entry of the target data flow.
        VtnDataFlow vdf = vdata.getValue();
        Flow flow = getIngressFlow(ctx, vdf, flowMap);
        if (flow != null) {
            FlowId id = vdf.getSalFlowId();
            FlowId mdId = flow.getId();
            if (mdId.equals(id)) {
                ctx.log(LOG, VTNLogLevel.TRACE,
                        "{}: MD-SAL flow ID is already resolved: {}",
                        vtnId.getValue(), id.getValue());
            } else {
                // Associate this MD-SAL flow ID with the VTN data flow.
                if (id == null) {
                    ctx.log(LOG, VTNLogLevel.DEBUG,
                            "{}: Associated with MD-SAL flow ID: {}",
                            vtnId.getValue(), mdId.getValue());
                } else {
                    ctx.log(LOG, VTNLogLevel.INFO,
                            "{}: MD-SAL flow ID has been changed: {} -> {}",
                            vtnId.getValue(), id.getValue(), mdId.getValue());
                }

                VtnDataFlowBuilder builder = new VtnDataFlowBuilder(vdf).
                    setSalFlowId(mdId);

                // Update flow statistics.
                setStatistics(ctx, builder, flow);

                // Update the VTN data flow.
                InstanceIdentifier<VtnDataFlow> path = vdata.getIdentifier();
                LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
                ReadWriteTransaction tx = ctx.getReadWriteTransaction();
                tx.put(oper, path, builder.build(), false);
            }
        }
    }

    // AbstractTxTask

    /**
     * {@inheritDoc}
     */
    @Override
    public Void execute(TxContext ctx) throws VTNException {
        finder = new FlowFinder(ctx.getReadWriteTransaction());
        for (Entry<VtnFlowId, Map<SalNode, Flow>> entry:
                 addedFlows.entrySet()) {
            resolve(ctx, entry.getKey(), entry.getValue());
        }

        return null;
    }

    // TxTask

    /**
     * Invoked when the task has failed.
     *
     * @param provider  VTN Manager provider service.
     * @param t         A {@link Throwable} thrown by the task.
     */
    @Override
    public void onFailure(VTNManagerProvider provider, Throwable t) {
        LOG.error("Failed to resolve MD-SAL flow ID.", t);
    }
}
