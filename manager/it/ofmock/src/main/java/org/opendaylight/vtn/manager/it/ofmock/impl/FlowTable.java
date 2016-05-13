/*
 * Copyright (c) 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.ofmock.impl;

import static org.opendaylight.vtn.manager.it.ofmock.OfMockUtils.isOutput;
import static org.opendaylight.vtn.manager.it.ofmock.OfMockUtils.verify;
import static org.opendaylight.vtn.manager.it.ofmock.impl.FlowMatcher.EMPTY_MATCH;
import static org.opendaylight.vtn.manager.it.ofmock.impl.OfNode.BYTE_COUNT;
import static org.opendaylight.vtn.manager.it.ofmock.impl.OfNode.DURATION_NANOSEC;
import static org.opendaylight.vtn.manager.it.ofmock.impl.OfNode.DURATION_SEC;
import static org.opendaylight.vtn.manager.it.ofmock.impl.OfNode.PACKET_COUNT;

import java.math.BigInteger;
import java.util.ArrayList;
import java.util.Collection;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.concurrent.TimeoutException;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.VtnOpenflowVersion;

import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.inventory.rev130819.FlowCapableNode;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.inventory.rev130819.tables.Table;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.inventory.rev130819.tables.TableKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.inventory.rev130819.tables.table.Flow;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.inventory.rev130819.tables.table.FlowKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.service.rev130819.AddFlowInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.service.rev130819.SwitchFlowRemovedBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.rev131026.FlowModFlags;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.rev131026.FlowRef;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.rev131026.OutputPortValues;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.rev131026.RemovedReasonFlags;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.rev131026.flow.Instructions;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.rev131026.flow.mod.removed.Match;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.rev131026.flow.mod.removed.MatchBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.NodeRef;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.nodes.Node;

/**
 * {@code FlowTable} describes the flow table in a OpenFlow switch.
 */
public final class FlowTable {
    /**
     * The prefix of the MD-SAL flow ID.
     */
    private static final String  FLOW_ID_PREFIX = "#OFMOCK*TABLE*";

    /**
     * A fixed cookie value for a table miss flow entry.
     */
    private static final long  MISS_COOKIE = 0x7f57ffffffffffffL;

    /**
     * A flow timeout to be configured in a table miss flow entry.
     */
    private static final Integer  MISS_TIMEOUT = 0;

    /**
     * A flow priority to be configured in a table miss flow entry.
     */
    private static final Integer  MISS_PRIORITY = 0;

    /**
     * A removed-reason-flags value that indicates a flow entry was removed by
     * a FLOW_MOD.
     */
    private static final RemovedReasonFlags  REMOVED_REASON_DELETE =
        new RemovedReasonFlags(true, false, false, false);

    /**
     * The ofmock provider service.
     */
    private final OfMockProvider  ofMockProvider;

    /**
     * OpenFlow protocol version.
     */
    private final VtnOpenflowVersion  ofVersion;

    /**
     * The table ID.
     */
    private final Short  tableId;

    /**
     * The path to this flow table.
     */
    private final InstanceIdentifier<Table>  tablePath;

    /**
     * The MD-SAL flow ID to be associated with a table miss flow entry.
     */
    private final String  missFlowId;

    /**
     * A map that associates flow entries with flow IDs.
     */
    private final Map<OfMockFlowEntry, String>  flowMap = new HashMap<>();

    /**
     * A map that keeps flow entries installed into the MD-SAL datastore.
     */
    private final Map<String, OfMockFlowEntry>  idMap = new HashMap<>();

    /**
     * The identifier for the next flow entry.
     */
    private int  nextFlowId;

    /**
     * {@code TableCond} describes the condition to be satisfied.
     */
    private interface TableCond {
        /**
         * Determine whether the specified condition is satisfied or not.
         *
         * @return  {@code true} if the specified condition is satisfied.
         *          {@code false} otherwise.
         */
        boolean check();

        /**
         * Return an error message that indicates timeout.
         *
         * @return  An error message.
         */
        String getTimeoutError();
    }

    /**
     * {@code FlowAdded} describes the table condition used to check whether
     * the specified flow entry is added to the flow table.
     */
    private final class FlowAdded implements TableCond {
        /**
         * The target flow entry.
         */
        private final OfMockFlowEntry  targetFlow;

        /**
         * The flow entry found in the flow table.
         */
        private OfMockFlowEntry  result;

        /**
         * Construct a new instance.
         *
         * @param ofent  The target flow entry.
         */
        private FlowAdded(OfMockFlowEntry ofent) {
            targetFlow = ofent;
        }

        /**
         * Return the flow entry found in the flow table.
         *
         * @return  A {@link OfMockFlowEntry} instance if found.
         *          {@code null} if not found.
         */
        private OfMockFlowEntry getResult() {
            return result;
        }

        // TableCond

        /**
         * Determine whether the specified flow entry is present in the
         * flow table or not.
         *
         * @return  {@code true} if the specified flow entry is present in the
         *          flow table. {@code false} otherwise.
         */
        @Override
        public boolean check() {
            OfMockFlowEntry ofent = getFlow(targetFlow);
            boolean found = (ofent != null);
            if (found) {
                result = ofent;
            }

            return found;
        }

        /**
         * {@inheritDoc}
         */
        @Override
        public String getTimeoutError() {
            return "Flow entry did not added: " + targetFlow;
        }
    }

    /**
     * {@code FlowRemoved} describes the table condition used to check whether
     * the specified flow entry is removed from the flow table.
     */
    private final class FlowRemoved implements TableCond {
        /**
         * The ID for the target flow entry.
         */
        private final String  flowId;

        /**
         * Construct a new instance.
         *
         * @param id  The ID for the target flow entry.
         */
        private FlowRemoved(String id) {
            flowId = id;
        }

        // TableCond

        /**
         * Determine whether the specified flow entry is not present in the
         * flow table or not.
         *
         * @return  {@code true} if the specified flow entry is not present in
         *          the flow table. {@code false} otherwise.
         */
        @Override
        public boolean check() {
            return (getFlow(flowId) == null);
        }

        /**
         * {@inheritDoc}
         */
        @Override
        public String getTimeoutError() {
            return "Flow entry did not removed: " + flowId;
        }
    }

    /**
     * {@code TableCleared} describes the table condition used to check whether
     * the flow table is empty or not.
     */
    private final class TableCleared implements TableCond {
        /**
         * Construct a new instance.
         */
        private TableCleared() {
        }

        // TableCond

        /**
         * Determine whether the flow table is empty or not.
         *
         * @return  {@code true} if the flow table is empty.
         *          {@code false} otherwise.
         */
        @Override
        public boolean check() {
            return isEmpty();
        }

        /**
         * {@inheritDoc}
         */
        @Override
        public String getTimeoutError() {
            String nodeId = getTablePath().firstKeyOf(Node.class).
                getId().getValue();
            return "Flow entry was not cleared: " + nodeId;
        }
    }

    /**
     * Construct a new instance.
     *
     * @param node  The node that owns this flow table.
     * @param id    The flow table ID.
     */
    public FlowTable(OfNode node, Short id) {
        ofMockProvider = node.getOfMockProvider();
        ofVersion = node.getOfVersion();
        tableId = id;
        tablePath = node.getNodePath().builder().
            augmentation(FlowCapableNode.class).
            child(Table.class, new TableKey(id)).
            build();
        missFlowId = "vtn:table-miss:" + node.getNodeIdentifier();
    }

    /**
     * Return the flow table ID.
     *
     * @return  The flow table ID.
     */
    public Short getTableId() {
        return tableId;
    }

    /**
     * Return the path to the flow table.
     *
     * @return  The path to the flow table.
     */
    public InstanceIdentifier<Table> getTablePath() {
        return tablePath;
    }

    /**
     * Associate a new flow ID with the specified flow entry.
     *
     * @param nid    The MD-SAL node ID that specifies the target switch.
     * @param input  An add-flow input.
     * @return  An {@link OfMockFlowEntry} instance.
     * @throws IllegalArgumentException
     *    The specified flow entry is invalid.
     */
    public OfMockFlowEntry newFlowId(String nid, AddFlowInput input) {
        OfMockFlowEntry ofent = new OfMockFlowEntry(nid, input);

        // Verify the flow table ID.
        int tid = ofent.getTableId();
        if (tid != tableId) {
            throw new IllegalArgumentException("Unsupported table-id: " + tid);
        }

        FlowRef fref = input.getFlowRef();
        BigInteger cookie = ofent.getCookie();
        String id;
        if (cookie.longValue() == MISS_COOKIE) {
            // The specified flow entry must be a table miss flow entry.
            verifyTableMissFlowEntry(input);
            id = getFlowId(fref);
            verify(missFlowId, id, "Invalid flow-id");
        } else {
            verify(null, fref, "flow-ref must be null");
            id = newFlowId(ofent);
        }
        ofent.setFlowId(id);

        return ofent;
    }

    /**
     * Abort the flow installation.
     *
     * @param ofent  The flow entry.
     */
    public synchronized void abort(OfMockFlowEntry ofent) {
        flowMap.remove(ofent);
    }

    /**
     * Complete the flow installation.
     *
     * @param id     The flow ID associated with the flow entry.
     * @param ofent  The flow entry.
     */
    public synchronized void complete(String id, OfMockFlowEntry ofent) {
        idMap.put(id, ofent);
        notifyAll();
    }

    /**
     * Remove the specified flow entry.
     *
     * @param id  The flow ID to be removed.
     */
    public synchronized void remove(String id) {
        OfMockFlowEntry ofent = idMap.remove(id);
        publishFlowRemoved(getNodeRef(), ofent);
        flowMap.remove(ofent);
        notifyAll();
    }

    /**
     * Remove the specified flow entries.
     *
     * @param ids  A collection of flow IDs to be removed.
     */
    public synchronized void remove(Collection<String> ids) {
        NodeRef nref = getNodeRef();
        for (String id: ids) {
            OfMockFlowEntry ofent = idMap.remove(id);
            publishFlowRemoved(nref, ofent);
            flowMap.remove(ofent);
        }
        notifyAll();
    }

    /**
     * Return a MD-SAL flow table with eliminating flow entries that do not
     * satisfy the specified condition.
     *
     * @param matcher  The flow matcher that specifies the condition.
     * @return  A {@link RemovedFlows} instance if at least one flow entry
     *          was removed.
     *          {@code null} if no flow entry was removed.
     */
    public synchronized RemovedFlows removeFlows(FlowMatcher matcher) {
        RemovedFlows removed = new RemovedFlows(tableId);
        for (OfMockFlowEntry ofent: idMap.values()) {
            Flow flow = ofent.getFlowEntry();
            if (matcher.match(flow)) {
                removed.addRemoved(flow);
            } else {
                removed.addRetained(flow);
            }
        }

        return (removed.isChanged()) ? removed : null;
    }

    /**
     * Return the flow ID associated with the specified flow entry.
     *
     * @param ofent  The flow entry.
     * @return  The MD-SAL flow ID associated with the flow entry if found.
     *          {@code null} if not found.
     */
    public synchronized String getFlowId(OfMockFlowEntry ofent) {
        return flowMap.get(ofent);
    }

    /**
     * Return the flow entry specified by the given ofmock flow entry.
     *
     * @param target  An {@link OfMockFlowEntry} instance which represents
     *                the target flow.
     * @return  An {@link OfMockFlowEntry} instance if found.
     *          {@code null} if not found.
     */
    public synchronized OfMockFlowEntry getFlow(OfMockFlowEntry target) {
        String id = flowMap.get(target);
        return (id == null) ? null : idMap.get(id);
    }

    /**
     * Return the flow entry associated with the given flow ID.
     *
     * @param id  The flow ID.
     * @return  An {@link OfMockFlowEntry} instance if found.
     *          {@code null} if not found.
     */
    public synchronized OfMockFlowEntry getFlow(String id) {
        return idMap.get(id);
    }

    /**
     * Return all the flow entries in the flow table.
     *
     * @return  A list of flow entries.
     */
    public synchronized List<OfMockFlowEntry> getFlows() {
        return new ArrayList<>(idMap.values());
    }

    /**
     * Determine whether the flow table is empty or not.
     *
     * <p>
     *   If the OpenFlow protocol is OF1.3+, this method returns {@code false}
     *   unless the table miss flow entry is installed.
     * </p>
     *
     * @return  {@code true} if the flow table is empty.
     *          {@code false} otherwise.
     */
    public synchronized boolean isEmpty() {
        return (ofVersion == VtnOpenflowVersion.OF10)
            ? idMap.isEmpty()
            : (idMap.size() == 1 && idMap.containsKey(missFlowId));
    }

    /**
     * Return the number of flow entries in the flow table.
     *
     * <p>
     *   Note that this method never counts a table miss flow entry for
     *   OF1.3+ switch.
     * </p>
     *
     * @return  The number of flow entries in the flow table excluding a
     *          table miss flow entry.
     */
    public synchronized int size() {
        int size = idMap.size();
        if (ofVersion != VtnOpenflowVersion.OF10 &&
            idMap.containsKey(missFlowId)) {
            size--;
        }

        return size;
    }

    /**
     * Wait for the specified flow entry to be added to the flow table.
     *
     * @param target   An {@link OfMockFlowEntry} instance that specifies the
     *                 target flow.
     * @param timeout  The number of milliseconds to wait.
     * @return  A {@link OfMockFlowEntry} found in the flow table.
     * @throws InterruptedException
     *    The calling thread was interrupted.
     * @throws TimeoutException
     *    The specified flow entry was not added within the timeout.
     */
    public OfMockFlowEntry awaitAdded(OfMockFlowEntry target, long timeout)
        throws InterruptedException, TimeoutException {
        FlowAdded added = new FlowAdded(target);
        await(added, timeout);
        return added.getResult();
    }

    /**
     * Wait for the specified flow entry to be removed from the flow table.
     *
     * @param target   An {@link OfMockFlowEntry} instance that specifies the
     *                 target flow.
     * @param timeout  The number of milliseconds to wait.
     * @throws InterruptedException
     *    The calling thread was interrupted.
     * @throws TimeoutException
     *    The specified flow entry was not removed within the timeout.
     */
    public synchronized void awaitRemoved(OfMockFlowEntry target, long timeout)
        throws InterruptedException, TimeoutException {
        String id = flowMap.get(target);
        if (id != null) {
            await(new FlowRemoved(id), timeout);
        }
    }

    /**
     * Wait for the flow table to be cleared.
     *
     * @param timeout  The number of milliseconds to wait.
     * @throws InterruptedException
     *    The calling thread was interrupted.
     * @throws TimeoutException
     *    The flow table was not cleared within the timeout.
     */
    public synchronized void awaitCleared(long timeout)
        throws InterruptedException, TimeoutException {
        await(new TableCleared(), timeout);
    }

    /**
     * Block the calling thread until the specified condition is satisfied.
     *
     * @param cond     The condition to be satisfied.
     * @param timeout  The number of milliseconds to wait.
     * @throws InterruptedException
     *    The calling thread was interrupted.
     * @throws TimeoutException
     *    The specified condition was not satisfied within the timeout.
     */
    private synchronized void await(TableCond cond, long timeout)
        throws InterruptedException, TimeoutException {
        if (!cond.check()) {
            long msec = timeout;
            long deadline = System.currentTimeMillis() + timeout;
            do {
                wait(msec);
                if (cond.check()) {
                    return;
                }
                msec = deadline - System.currentTimeMillis();
            } while (msec > 0);

            throw new TimeoutException(cond.getTimeoutError());
        }
    }

    /**
     * Return a MD-SAL flow ID configured in the given flow-ref instance.
     *
     * @param fref  A flow-ref instance.
     * @return  A MD-SAL flow ID configured in the given flow-ref instance.
     * @throws IllegalArgumentException
     *    The given flow-ref instance is invalid.
     */
    private String getFlowId(FlowRef fref) {
        if (fref == null) {
            throw new IllegalArgumentException("flow-ref cannot be null.");
        }

        // Verify the specified path.
        InstanceIdentifier<?> path = fref.getValue();
        FlowKey fkey = path.firstKeyOf(Flow.class);
        if (fkey != null) {
            InstanceIdentifier<Flow> expected =
                tablePath.child(Flow.class, fkey);
            if (expected.equals(path)) {
                return fkey.getId().getValue();
            }
        }

        throw new IllegalArgumentException("Invalid flow-ref: " + path);
    }

    /**
     * Verify the specified table miss flow entry.
     *
     * @param input  An add-flow input to be verified.
     * @throws IllegalArgumentException
     *    The specified table miss flow entry is invalid.
     */
    private void verifyTableMissFlowEntry(AddFlowInput input) {
        if (ofVersion == VtnOpenflowVersion.OF10) {
            throw new IllegalArgumentException(
                "No table miss flow entry is needed for OF 1.0 switch.");
        }

        verify(MISS_TIMEOUT, input.getIdleTimeout(), "Invalid idle-timeout");
        verify(MISS_TIMEOUT, input.getHardTimeout(), "Invalid hard-timeout");
        verify(MISS_PRIORITY, input.getPriority(), "Invalid priority");
        verify(EMPTY_MATCH, input.getMatch(), "Invalid match");

        Instructions insts = input.getInstructions();
        String dst = OutputPortValues.CONTROLLER.toString();
        if (!isOutput(insts, dst)) {
            throw new IllegalArgumentException(
                "Invalid instructions: " + insts);
        }
    }

    /**
     * Associate a new flow ID with the specified flow entry.
     *
     * @param ofent  The flow entry.
     * @return  A new flow ID associated with the specified flow entry.
     * @throws IllegalArgumentException
     *    The specified flow entry is invalid.
     */
    private synchronized String newFlowId(OfMockFlowEntry ofent) {
        int fid = ++nextFlowId;
        String id = FLOW_ID_PREFIX + ofent.getTableId() + "-" + fid;
        String current = flowMap.put(ofent, id);
        if (current != null) {
            flowMap.put(ofent, current);
            throw new IllegalArgumentException(
                "Overlapping flow entry is not supported: " + ofent);
        }

        return id;
    }

    /**
     * Return a reference to the node that contains this flow table.
     *
     * @return  A reference to the node that contains this flow table.
     */
    private NodeRef getNodeRef() {
        return new NodeRef(tablePath.firstIdentifierOf(Node.class));
    }

    /**
     * Publish a switch-flow-removed notification only if the specified
     * flow entry needs to be notified by a FLOW_REMOVED.
     *
     * @param nref   A reference to the node that contains this flow table.
     * @param ofent  A {@link OfMockFlowEntry} instance that indicates a
     *               removed flow entry.
     */
    private void publishFlowRemoved(NodeRef nref, OfMockFlowEntry ofent) {
        FlowModFlags flags = ofent.getFlowModFlags();
        if (flags != null && Boolean.TRUE.equals(flags.isSENDFLOWREM())) {
            Match match = new MatchBuilder(ofent.getMatch()).build();
            SwitchFlowRemovedBuilder builder =
                new SwitchFlowRemovedBuilder(ofent.getFlowEntry()).
                setPacketCount(BigInteger.valueOf(PACKET_COUNT)).
                setByteCount(BigInteger.valueOf(BYTE_COUNT)).
                setDurationSec(Long.valueOf(DURATION_SEC)).
                setDurationNsec(Long.valueOf(DURATION_NANOSEC)).
                setRemovedReason(REMOVED_REASON_DELETE).
                setMatch(match).
                setNode(nref);
            ofMockProvider.publish(builder.build());
        }
    }
}
