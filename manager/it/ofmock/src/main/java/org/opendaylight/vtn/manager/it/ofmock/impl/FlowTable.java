/*
 * Copyright (c) 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.ofmock.impl;

import java.util.ArrayList;
import java.util.Collection;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.concurrent.TimeoutException;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.inventory.rev130819.FlowCapableNode;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.inventory.rev130819.tables.Table;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.inventory.rev130819.tables.TableKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.inventory.rev130819.tables.table.Flow;
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
     * The table ID.
     */
    private final Short  tableId;

    /**
     * The path to this flow table.
     */
    private final InstanceIdentifier<Table>  tablePath;

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
        tableId = id;
        tablePath = node.getNodePath().builder().
            augmentation(FlowCapableNode.class).
            child(Table.class, new TableKey(id)).
            build();
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
     * @param ofent  The flow entry.
     * @return  A new flow ID associated with the specified flow entry.
     * @throws IllegalArgumentException
     *    The specified flow entry is invalid.
     */
    public synchronized String newFlowId(OfMockFlowEntry ofent) {
        // Verify the flow table ID.
        int tid = ofent.getTableId();
        if (tid != tableId) {
            throw new IllegalArgumentException("Unsupported table-id: " + tid);
        }

        int fid = ++nextFlowId;
        String id = FLOW_ID_PREFIX + tid + "-" + fid;
        String current = flowMap.put(ofent, id);
        if (current != null) {
            flowMap.put(ofent, current);
            throw new IllegalArgumentException(
                "Overlapping flow entry is not supported: " + ofent);
        }

        return id;
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
        flowMap.remove(ofent);
        notifyAll();
    }

    /**
     * Remove the specified flow entries.
     *
     * @param ids  A collection of flow IDs to be removed.
     */
    public synchronized void remove(Collection<String> ids) {
        for (String id: ids) {
            OfMockFlowEntry ofent = idMap.remove(id);
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
     * @return  {@code true} if the flow table is empty.
     *          {@code false} otherwise.
     */
    public synchronized boolean isEmpty() {
        return idMap.isEmpty();
    }

    /**
     * Return the number of flow entries in the flow table.
     *
     * @return  The number of flow entries in the flow table.
     */
    public synchronized int size() {
        return idMap.size();
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
            } while (timeout > 0);

            throw new TimeoutException(cond.getTimeoutError());
        }
    }
}
