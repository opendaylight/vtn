/*
 * Copyright (c) 2015, 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.flow.stats;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.Objects;

import org.slf4j.Logger;

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
import org.opendaylight.vtn.manager.internal.util.inventory.InventoryUtils;
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
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.inventory.rev130819.tables.Table;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.inventory.rev130819.tables.TableKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.inventory.rev130819.tables.table.Flow;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.statistics.rev130819.FlowStatisticsData;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.rev131026.FlowCookie;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.NodeConnectorId;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.NodeId;
import org.opendaylight.yang.gen.v1.urn.opendaylight.model.statistics.types.rev130925.GenericStatistics;

/**
 * {@code AddedFlowStats} keeps a list of flow statistics added by the
 * MD-SAL statistics manager.
 */
public final class AddedFlowStats extends AbstractTxTask<Void> {
    /**
     * A log message that indicates the flow statistics is ignored.
     */
    private static final String  IGNORE_STATS = "Ignore flow statistics";

    /**
     * A list of flow information added by the MD-SAL statistics manager.
     */
    private final List<IdentifiedData<Flow>>  addedFlows = new ArrayList<>();

    /**
     * A logger instance.
     */
    private final Logger  logger;

    /**
     * The system time when the flow statistics are collected.
     */
    private final Long  systemTime;

    /**
     * A {@link FlowFinder} instance used to determine the VTN data flow
     * associated with the VTN flow ID.
     */
    private FlowFinder  finder;

    /**
     * Construct a new instance.
     *
     * @param log  A {@link Logger} instance.
     */
    public AddedFlowStats(Logger log) {
        logger = log;
        systemTime = System.currentTimeMillis();
    }

    /**
     * Add the given flow information into this instance.
     *
     * @param data  An {@link IdentifiedData} instance which contains
     *              flow statistics.
     */
    public void add(IdentifiedData<Flow> data) {
        addedFlows.add(data);
    }

    /**
     * Resolve MD-SAL flow IDs.
     *
     * <p>
     *   Flow entries installed by the VTN Manager has no MD-SAL flow ID
     *   because VTN Manager installs flow entries without using the MD-SAL
     *   FRM. But pseudo MD-SAL flow identifier will be assigned to every
     *   flow entries by openflowplugin. This method determines MD-SAL flow
     *   IDs assigned to newly added flow entries.
     * </p>
     *
     * @param txq  A {@link TxQueue} instance to run operation.
     */
    public void apply(TxQueue txq) {
        if (!addedFlows.isEmpty()) {
            txq.post(this);
        }
    }

    /**
     * Return the node ID associated with the specified MD-SAL flow.
     *
     * @param ctx   MD-SAL datastore transaction context.
     * @param data  An {@link IdentifiedData} instance which contains
     *              flow statistics.
     * @return  A {@link NodeId} on success. {@code null} on failure.
     */
    private NodeId getNodeId(TxContext ctx, IdentifiedData<Flow> data) {
        InstanceIdentifier<Flow> path = data.getIdentifier();
        NodeId node = InventoryUtils.getNodeId(path);
        if (node == null) {
            ctx.log(logger, VTNLogLevel.WARN,
                    "Node ID is not present in flow path: {}", path);
            return null;
        }

        // Verify the table ID.
        TableKey key = path.firstKeyOf(Table.class);
        Short tid = (key == null) ? null : key.getId();
        if (tid == null) {
            ctx.log(logger, VTNLogLevel.WARN,
                    "Table ID is not present in flow path: {}", path);
            return null;
        }

        if (tid.intValue() != FlowUtils.TABLE_ID) {
            ctx.log(logger, VTNLogLevel.WARN,
                    "Unexpected table ID in flow path: {}", path);
            return null;
        }

        return node;
    }

    /**
     * Determine whether the given MD-SAL flow entry is the ingress flow entry
     * of the given VTN data flow.
     *
     * <p>
     *   This method compares the following attributes.
     * </p>
     * <ul>
     *   <li>Target node</li>
     *   <li>Ingress switch port in flow match (IN_PORT)</li>
     *   <li>Flow priority</li>
     * </ul>
     *
     * @param ctx   MD-SAL datastore transaction context.
     * @param vdf   The {@link VtnDataFlow} instance to be tested.
     * @param flow  The MD-SAL flow entry to be tested.
     * @param node  The node where the {@code flow} is installed.
     * @return  {@code true} only if the MD-SAL flow specified by {@code flow}
     *          is the ingress flow entry of {@code vdf}.
     */
    private boolean isIngressFlow(TxContext ctx, VtnDataFlow vdf, Flow flow,
                                  NodeId node) {
        // Determine the ingress flow entry of the target data flow.
        FlowCache fc = new FlowCache(vdf);
        VtnFlowEntry vfent = fc.getIngressFlow();
        if (vfent == null) {
            ctx.log(logger, VTNLogLevel.WARN,
                    "{}: {}: Ingress flow not found.",
                    IGNORE_STATS, vdf.getFlowId().getValue());
            return false;
        }

        // Compare the target node.
        NodeId vnode = vfent.getNode();
        if (!node.equals(vnode)) {
            ctx.log(logger, VTNLogLevel.TRACE,
                    "{}: {}: Node ID does not match: {}, {}",
                    IGNORE_STATS, vdf.getFlowId().getValue(), vnode, node);
            return false;
        }

        // One VTN data flow should never installs more than one flow entry
        // into the same node.
        NodeConnectorId vport =
            FlowMatchUtils.getIngressPort(vfent.getMatch());
        NodeConnectorId port =
            FlowMatchUtils.getIngressPort(flow.getMatch());
        if (!MiscUtils.equalsUri(vport, port)) {
            ctx.log(logger, VTNLogLevel.WARN,
                    "{}: {}: IN_PORT does not match: {}, {}",
                    IGNORE_STATS, vdf.getFlowId().getValue(), vport, port);
            return false;
        }

        Integer vpri = vfent.getPriority();
        Integer pri = flow.getPriority();
        boolean ret = Objects.equals(vpri, pri);
        if (!ret) {
            ctx.log(logger, VTNLogLevel.WARN,
                    "{}: {}: Priority does not match: {}, {}",
                    IGNORE_STATS, vdf.getFlowId().getValue(), vpri, pri);
        }

        return ret;
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
        FlowStatisticsData data =
            flow.getAugmentation(FlowStatisticsData.class);
        GenericStatistics fstats = (data == null)
            ? null
            : data.getFlowStatistics();

        String err = FlowStatsUtils.check(fstats);
        if (err == null) {
            FlowStatsRecord fsr = new FlowStatsRecordBuilder(fstats).
                setTime(systemTime).build();
            FlowStatsHistoryBuilder sb = new FlowStatsHistoryBuilder().
                setFlowStatsRecord(Collections.singletonList(fsr));
            builder.setFlowStatsHistory(sb.build());
        } else {
            ctx.log(logger, VTNLogLevel.WARN,
                    "{}: {}: No flow statistics: {}",
                    builder.getFlowId().getValue(),
                    builder.getSalFlowId().getValue(), err);
        }
    }

    /**
     * Try to associate the given MD-SAL flow with the VTN data flow.
     *
     * @param ctx    MD-SAL datastore transaction context.
     * @param node   A MD-SAL node identifier.
     * @param flow   A MD-SAL flow entry.
     * @param vtnId  Identifier of the VTN data flow.
     * @throws VTNException  An error occurred.
     */
    private void resolve(TxContext ctx, NodeId node, Flow flow,
                         VtnFlowId vtnId) throws VTNException {
        IdentifiedData<VtnDataFlow> vdata = finder.find(vtnId);
        if (vdata == null) {
            ctx.log(logger, VTNLogLevel.WARN, "{}: {}: Data flow not found.",
                    IGNORE_STATS, vtnId.getValue());
            return;
        }

        VtnDataFlow vdf = vdata.getValue();
        FlowId id = vdf.getSalFlowId();
        FlowId mdId = flow.getId();
        if (mdId.equals(id)) {
            ctx.log(logger, VTNLogLevel.TRACE,
                    "{}: {}: Already resolved: {}",
                    IGNORE_STATS, vtnId.getValue(), id.getValue());
            return;
        }

        if (isIngressFlow(ctx, vdf, flow, node)) {
            // The given flow is the ingress flow of the target data flow.
            // Copy statistics if available.
            VtnDataFlowBuilder builder = new VtnDataFlowBuilder().
                setFlowId(vtnId).setSalFlowId(mdId);
            setStatistics(ctx, builder, flow);

            // Associate the VTN data flow with this MD-SAL flow ID.
            InstanceIdentifier<VtnDataFlow> path = vdata.getIdentifier();
            LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
            ReadWriteTransaction tx = ctx.getReadWriteTransaction();
            tx.merge(oper, path, builder.build(), false);
            if (id == null) {
                ctx.log(logger, VTNLogLevel.DEBUG,
                        "{}: Associated with MD-SAL flow ID: {}",
                        vtnId.getValue(), mdId.getValue());
            } else {
                ctx.log(logger, VTNLogLevel.DEBUG,
                        "{}: MD-SAL flow ID has been changed: {} -> {}",
                        vtnId.getValue(), id.getValue(), mdId.getValue());
            }
        }
    }

    // AbstractTxTask

    /**
     * {@inheritDoc}
     */
    @Override
    public Void execute(TxContext ctx) throws VTNException {
        ReadWriteTransaction tx = ctx.getReadWriteTransaction();
        finder = new FlowFinder(tx);

        for (IdentifiedData<Flow> data: addedFlows) {
            // Verify the node and the table ID.
            NodeId node = getNodeId(ctx, data);
            if (node == null) {
                continue;
            }

            Flow flow = data.getValue();
            FlowCookie cookie = flow.getCookie();
            if (flow.getId() == null) {
                ctx.log(logger, VTNLogLevel.WARN,
                        "No MD-SAL flow ID is assigned: {}", cookie);
                continue;
            }

            VtnFlowId vtnId = FlowUtils.getVtnFlowId(cookie);
            if (vtnId == null) {
                ctx.log(logger, VTNLogLevel.TRACE, "{}: Unexpected cookie: {}",
                        IGNORE_STATS, cookie);
            } else {
                resolve(ctx, node, flow, vtnId);
            }
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
        logger.error("Failed to resolve MD-SAL flow ID.", t);
    }
}
