/*
 * Copyright (c) 2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
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
import org.opendaylight.vtn.manager.internal.util.FixedLogger;
import org.opendaylight.vtn.manager.internal.util.IdentifiedData;
import org.opendaylight.vtn.manager.internal.util.LogRecord;
import org.opendaylight.vtn.manager.internal.util.MiscUtils;
import org.opendaylight.vtn.manager.internal.util.flow.FlowCache;
import org.opendaylight.vtn.manager.internal.util.flow.FlowFinder;
import org.opendaylight.vtn.manager.internal.util.flow.FlowStatsUtils;
import org.opendaylight.vtn.manager.internal.util.flow.FlowUtils;
import org.opendaylight.vtn.manager.internal.util.flow.match.FlowMatchUtils;
import org.opendaylight.vtn.manager.internal.util.inventory.InventoryUtils;
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
     * A list of flow information added by the MD-SAL statistics manager.
     */
    private final List<IdentifiedData<Flow>>  addedFlows = new ArrayList<>();

    /**
     * A logger for warning logs.
     */
    private final FixedLogger  warnLogger;

    /**
     * A logger for debug logs.
     */
    private final FixedLogger  debugLogger;

    /**
     * A logger for trace logs.
     */
    private final FixedLogger  traceLogger;

    /**
     * A list of log records.
     */
    private List<LogRecord>  logRecords;

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
     * The MAC address of the controller.
     */
    private Long  controllerAddress;

    /**
     * Construct a new instance.
     *
     * @param log  A {@link Logger} instance.
     */
    public AddedFlowStats(Logger log) {
        warnLogger = new FixedLogger.Warn(log);
        debugLogger = new FixedLogger.Debug(log);
        traceLogger = new FixedLogger.Trace(log);
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
     * @param data  An {@link IdentifiedData} instance which contains
     *              flow statistics.
     * @return  A {@link NodeId} on success. {@code null} on failure.
     */
    private NodeId getNodeId(IdentifiedData<Flow> data) {
        InstanceIdentifier<Flow> path = data.getIdentifier();
        NodeId node = InventoryUtils.getNodeId(path);
        if (node == null) {
            log(warnLogger, "Node ID is not present in flow path: %s", path);
            return null;
        }

        // Verify the table ID.
        TableKey key = path.firstKeyOf(Table.class, TableKey.class);
        Short tid = (key == null) ? null : key.getId();
        if (tid == null) {
            log(warnLogger, "Table ID is not present in flow path: %s", path);
            return null;
        }

        if (tid.intValue() != FlowUtils.TABLE_ID) {
            log(warnLogger, "Unexpected table ID in flow path: %s", path);
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
     * @param vdf    The {@link VtnDataFlow} instance to be tested.
     * @param flow   The MD-SAL flow entry to be tested.
     * @param node   The node where the {@code flow} is installed.
     * @return  {@code true} only if the MD-SAL flow specified by {@code flow}
     *          is the ingress flow entry of {@code vdf}.
     */
    private boolean isIngressFlow(VtnDataFlow vdf, Flow flow, NodeId node) {
        // Determine the ingress flow entry of the target data flow.
        FlowCache fc = new FlowCache(vdf);
        VtnFlowEntry vfent = fc.getIngressFlow();
        if (vfent == null) {
            log(warnLogger,
                "Ignore flow statistics: %s: Ingress flow not found.",
                vdf.getFlowId().getValue());
            return false;
        }

        // Compare the target node.
        NodeId vnode = vfent.getNode();
        if (!node.equals(vnode)) {
            log(traceLogger,
                "Ignore flow statistics: %s: Node ID does not match: %s, %s",
                vdf.getFlowId().getValue(), vnode, node);
            return false;
        }

        // One VTN data flow should never installs more than one flow entry
        // into the same node.
        NodeConnectorId vport =
            FlowMatchUtils.getIngressPort(vfent.getMatch());
        NodeConnectorId port =
            FlowMatchUtils.getIngressPort(flow.getMatch());
        if (!MiscUtils.equalsUri(vport, port)) {
            log(warnLogger,
                "Ignore flow statistics: %s: IN_PORT does not match: %s, %s",
                vdf.getFlowId().getValue(), vport, port);
            return false;
        }

        Integer vpri = vfent.getPriority();
        Integer pri = flow.getPriority();
        boolean ret = Objects.equals(vpri, pri);
        if (!ret) {
            log(warnLogger,
                "Ignore flow statistics: %s: Priority does not match: %s, %s",
                vdf.getFlowId().getValue(), vpri, pri);
        }

        return ret;
    }

    /**
     * Set flow statistics into the given VTN data flow builder if available.
     *
     * @param builder  A {@link VtnDataFlowBuilder} instance.
     * @param flow     A {@link Flow} instance.
     */
    private void setStatistics(VtnDataFlowBuilder builder, Flow flow) {
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
            log(warnLogger, "%s: %s: No flow statistics: %s",
                builder.getFlowId().getValue(),
                builder.getSalFlowId().getValue(), err);
        }
    }

    /**
     * Resolve the MD-SAL flow ID corresponding to the given MD-SAL flow.
     *
     * @param tx    A {@link ReadWriteTransaction} instance.
     * @param data  An {@link IdentifiedData} instance which contains
     *              flow statistics.
     * @throws VTNException  An error occurred.
     */
    private void resolve(ReadWriteTransaction tx, IdentifiedData<Flow> data)
        throws VTNException {
        // Verify the node and the table ID.
        NodeId node = getNodeId(data);
        if (node == null) {
            return;
        }

        Flow flow = data.getValue();
        FlowCookie cookie = flow.getCookie();
        FlowId fid = flow.getId();
        if (fid == null) {
            log(warnLogger, "No MD-SAL flow ID is assigned: %s", cookie);
            return;
        }

        VtnFlowId vtnId = FlowUtils.getVtnFlowId(cookie);
        if (vtnId == null) {
            log(traceLogger, "Ignore flow statistics: Unexpected cookie: %s",
                cookie);
            return;
        }

        IdentifiedData<VtnDataFlow> vdata = finder.find(vtnId);
        if (vdata == null) {
            log(warnLogger, "Ignore flow statistics: %s: Data flow not found.",
                vtnId.getValue());
            return;
        }

        // Ensure that this data flow is installed by this controller.
        VtnDataFlow vdf = vdata.getValue();
        Long mac = vdf.getControllerAddress();
        if (!controllerAddress.equals(mac)) {
            String maddr = (mac == null)
                ? "<null>"
                : Long.toHexString(mac.longValue());
            log(traceLogger, "Ignore flow statistics: %s: Not owner: %s",
                vtnId.getValue(), maddr);
            return;
        }

        FlowId id = vdf.getSalFlowId();
        if (fid.equals(id)) {
            log(traceLogger,
                "Ignore flow statistics: %s: Already resolved: %s",
                vtnId.getValue(), id.getValue());
            return;
        }

        // Check to see if the given flow is the ingress flow of the target
        // data flow.
        if (isIngressFlow(vdf, flow, node)) {
            // Copy statistics if available.
            VtnDataFlowBuilder builder = new VtnDataFlowBuilder().
                setFlowId(vtnId).setSalFlowId(fid);
            setStatistics(builder, flow);

            // Associate the VTN data flow with this MD-SAL flow ID.
            InstanceIdentifier<VtnDataFlow> path = vdata.getIdentifier();
            LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
            tx.merge(oper, path, builder.build(), false);
            if (id == null) {
                log(debugLogger, "%s: Associated with MD-SAL flow ID: %s",
                    vtnId.getValue(), fid.getValue());
            } else {
                log(debugLogger,
                    "%s: MD-SAL flow ID has been changed: %s -> %s",
                    vtnId.getValue(), id.getValue(), fid.getValue());
            }
        }
    }

    /**
     * Record a log message.
     *
     * @param log     A {@link FixedLogger} instance which specifies the logger
     *                and logging level.
     * @param format  A format string used to construct log message.
     * @param args    An object array used to construct log message.
     */
    private void log(FixedLogger log, String format, Object ... args) {
        if (log.isEnabled()) {
            logRecords.add(new LogRecord(log, format, args));
        }
    }

    // AbstractTxTask

    /**
     * {@inheritDoc}
     */
    @Override
    public Void execute(TxContext ctx) throws VTNException {
        logRecords = new ArrayList<>();
        ReadWriteTransaction tx = ctx.getReadWriteTransaction();
        finder = new FlowFinder(tx);
        if (controllerAddress == null) {
            controllerAddress = ctx.getProvider().getVTNConfig().
                getControllerMacAddress().getAddress();
        }

        for (IdentifiedData<Flow> data: addedFlows) {
            resolve(tx, data);
        }

        return null;
    }

    // TxTask

    /**
     * Invoked when the task has been completed successfully.
     *
     * @param provider  VTN Manager provider service.
     * @param result    The result of this task.
     */
    @Override
    public void onSuccess(VTNManagerProvider provider, Void result) {
        for (LogRecord r: logRecords) {
            r.log();
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
        warnLogger.getLogger().error("Failed to resolve MD-SAL flow ID.", t);
    }
}
