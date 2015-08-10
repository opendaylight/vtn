/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
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

import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.rev131026.Flow;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.rev131026.FlowModFlags;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.rev131026.flow.Instructions;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.rev131026.flow.Match;

/**
 * An internal implementation of {@link OfMockFlow}.
 */
public final class OfMockFlowEntry implements OfMockFlow {
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
     * Construct a new instance.
     *
     * @param nid   The node identifier.
     * @param flow  A {@link Flow} instance.
     * @throws IllegalArgumentException
     *    {@code flow} is invalid.
     */
    public OfMockFlowEntry(String nid, Flow flow) {
        if (flow == null) {
            throw new IllegalArgumentException("Input cannot be null.");
        }

        nodeIdentifier = nid;
        tableId = getInteger(flow.getTableId(), "table-id");
        priority = getInteger(flow.getPriority(), "priority");
        idleTimeout = getInteger(flow.getIdleTimeout(), "idle-timeout");
        hardTimeout = getInteger(flow.getHardTimeout(), "hard-timeout");

        flowCookie = OfMockUtils.getCookie(flow.getCookie());
        flowMatch = flow.getMatch();
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
        flowMatch = match;
        flowInstructions = null;
        flowModFlags = null;
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

        return Objects.equals(flowMatch, ofent.flowMatch);
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        return Objects.hash(tableId, priority, flowMatch);
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
