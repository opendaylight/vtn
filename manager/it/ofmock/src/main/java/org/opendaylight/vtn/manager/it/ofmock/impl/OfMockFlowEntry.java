/*
 * Copyright (c) 2015, 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.ofmock.impl;

import java.math.BigInteger;
import java.util.Objects;

import org.opendaylight.vtn.manager.it.ofmock.OfMockFlow;
import org.opendaylight.vtn.manager.it.ofmock.OfMockUtils;

import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.inventory.rev130819.tables.table.Flow;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.rev131026.FlowModFlags;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.rev131026.flow.Instructions;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.rev131026.flow.Match;

/**
 * An internal implementation of {@link OfMockFlow}.
 */
public final class OfMockFlowEntry implements OfMockFlow {
    /**
     * The name of the table-id field.
     */
    private static final String  FIELD_TABLE_ID = "table-id";

    /**
     * The name of the priority field.
     */
    private static final String  FIELD_PRIORITY = "priority";

    /**
     * The name of the idle-timeout field.
     */
    private static final String  FIELD_IDLE_TIMEOUT = "idle-timeout";

    /**
     * The name of the hard-timeout field.
     */
    private static final String  FIELD_HARD_TIMEOUT = "hard-timeout";

    /**
     * The node identifier corresponding to an OpenFlow switch.
     */
    private final String  nodeIdentifier;

    /**
     * The table identifier.
     */
    private final int  tableId;
    /**
     * Priority of this flow entry.
     */
    private final int  priority;

    /**
     * The idle timeout of this flow entry.
     */
    private final int  idleTimeout;

    /**
     * The hard timeout of this flow entry.
     */
    private final int  hardTimeout;

    /**
     * The cookie associated with this flow entry.
     */
    private final BigInteger  flowCookie;

    /**
     * The condition to match patckets.
     */
    private final Match  flowMatch;

    /**
     * The instructions of this flow entry.
     */
    private final Instructions  flowInstructions;

    /**
     * Flow-mod flags assigned to this flow entry.
     */
    private final FlowModFlags  flowModFlags;

    /**
     * The MD-SAL flow entry associated with this instance.
     */
    private Flow  flowEntry;

    /**
     * A cache for the hash code.
     */
    private int  hash;

    /**
     * Construct a new instance.
     *
     * @param nid   The node identifier.
     * @param flow  A {@link Flow} instance.
     * @throws IllegalArgumentException
     *    {@code flow} is invalid.
     */
    public OfMockFlowEntry(
        String nid,
        org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.rev131026.Flow flow) {
        if (flow == null) {
            throw new IllegalArgumentException("Input cannot be null.");
        }

        nodeIdentifier = nid;
        tableId = getInteger(flow.getTableId(), FIELD_TABLE_ID);
        priority = getInteger(flow.getPriority(), FIELD_PRIORITY);
        idleTimeout = getInteger(flow.getIdleTimeout(), FIELD_IDLE_TIMEOUT);
        hardTimeout = getInteger(flow.getHardTimeout(), FIELD_HARD_TIMEOUT);

        flowCookie = OfMockUtils.getCookie(flow.getCookie());
        flowMatch = verify(flow.getMatch());
        flowInstructions = flow.getInstructions();
        flowModFlags = flow.getFlags();
    }

    /**
     * Construct a new instance.
     *
     * @param nid    The node identifier.
     * @param table  The flow table identifier.
     * @param match  A {@link Match} instance.
     * @param pri    A priority value.
     */
    public OfMockFlowEntry(String nid, int table, Match match, int pri) {
        nodeIdentifier = nid;
        tableId = table;
        priority = pri;
        idleTimeout = 0;
        hardTimeout = 0;
        flowCookie = BigInteger.ZERO;
        flowMatch = verify(match);
        flowInstructions = null;
        flowModFlags = null;
    }

    /**
     * Determine whether the given MD-SAL flow entry is identical to this
     * instance or not.
     *
     * @param flow  The MD-SAL flow entry to be compared.
     * @return   {@code true} if identical. Otherwise {@code false}.
     */
    public boolean equalsFlow(
        org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.rev131026.Flow flow) {
        return (tableId == getInteger(flow.getTableId(), FIELD_TABLE_ID) &&
                priority == getInteger(flow.getPriority(), FIELD_PRIORITY) &&
                flowMatch.equals(flow.getMatch()));
    }

    /**
     * Return the MD-SAL flow entry associated with this instance.
     *
     * @return  The MD-SAL flow entry if configured.
     *          {@code null} if not configured.
     */
    public Flow getFlowEntry() {
        return flowEntry;
    }

    /**
     * Associate the given MD-SAL flow entry with this instance.
     *
     * @param flow  The MD-SAL flow entry.
     */
    public void setFlowEntry(Flow flow) {
        flowEntry = flow;
    }

    /**
     * Return an integer value in the given {@link Number} instance.
     *
     * @param num  A {@link Number} instance.
     * @param tag  A brief description of the given value.
     * @return  An integer value.
     * @throws IllegalArgumentException
     *    Invalid flow entry is specified.
     */
    private int getInteger(Number num, String tag) {
        if (num == null) {
            String msg = tag + ": Value cannot be null.";
            throw new IllegalArgumentException(msg);
        }

        return num.intValue();
    }

    /**
     * Ensure that the given flow match is not {@code null}.
     *
     * @param match  The MD-SAL flow match to be verified.
     * @return  {@code flow}.
     * @throws IllegalArgumentException
     *    The specified flow is {@code null}.
     */
    private Match verify(Match match) {
        if (match == null) {
            throw new IllegalArgumentException("Flow match cannot be null.");
        }
        return match;
    }

    /**
     * Determine whether the given object is identical to this object.
     *
     * @param o  An object to be compared.
     * @return   {@code true} if identical. Otherwise {@code false}.
     */
    @Override
    public boolean equals(Object o) {
        if (o == this) {
            return true;
        }
        if (o == null || !getClass().equals(o.getClass())) {
            return false;
        }

        OfMockFlowEntry ofent = (OfMockFlowEntry)o;
        if (tableId != ofent.tableId || priority != ofent.priority) {
            return false;
        }

        return flowMatch.equals(ofent.flowMatch);
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        int h = hash;
        if (h == 0) {
            h = Objects.hash(tableId, priority, flowMatch);
            hash = h;
        }

        return h;
    }

    /**
     * Return a string representation of this instance.
     *
     * @return  A string representation of this instance.
     */
    @Override
    public String toString() {
        String cookie = Long.toHexString(flowCookie.longValue());
        StringBuilder builder = new StringBuilder("OfMockFlowEntry[node=").
            append(nodeIdentifier).append(",table=").append(tableId).
            append(",pri=").append(priority).
            append(",cookie=0x").append(cookie).
            append(",idle=").append(idleTimeout).
            append(",hard=").append(hardTimeout).
            append(",match=").append(flowMatch).
            append(",inst=").append(flowInstructions).
            append(",flags=").append(flowModFlags).
            append(']');

        return builder.toString();
    }

    // OfMockFlow

    /**
     * {@inheritDoc}
     */
    @Override
    public String getNodeIdentifier() {
        return nodeIdentifier;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public int getTableId() {
        return tableId;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public int getPriority() {
        return priority;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public int getIdleTimeout() {
        return idleTimeout;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public int getHardTimeout() {
        return hardTimeout;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public BigInteger getCookie() {
        return flowCookie;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public Match getMatch() {
        return flowMatch;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public Instructions getInstructions() {
        return flowInstructions;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public FlowModFlags getFlowModFlags() {
        return flowModFlags;
    }
}
