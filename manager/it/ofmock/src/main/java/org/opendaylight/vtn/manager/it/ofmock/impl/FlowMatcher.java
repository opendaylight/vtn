/*
 * Copyright (c) 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.ofmock.impl;

import java.math.BigInteger;

import org.opendaylight.vtn.manager.it.ofmock.OfMockUtils;

import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.rev131026.Flow;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.rev131026.flow.Match;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.rev131026.flow.MatchBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.NodeConnectorId;

/**
 * {@code FlowMatcher} is used to select MD-SAL flow entries that meet the
 * specified conditions.
 */
public final class FlowMatcher {
    /**
     * An empty MD-SAL flow match.
     */
    public static final Match  EMPTY_MATCH = new MatchBuilder().build();

    /**
     * The target table ID.
     */
    private final Short  tableId;

    /**
     * The flow match that specifies the condition.
     */
    private final Match  flowMatch;

    /**
     * The identifier for the output port.
     */
    private final String  outPort;

    /**
     * The condition for the flow cookie.
     */
    private final CookieMatcher  cookieMatcher;

    /**
     * Determine whether the specified flow match is valid or not.
     */
    private final boolean  validMatch;

    /**
     * Construct a new instance.
     *
     * @param node  The target node.
     * @param cond  The MD-SAL flow entry that specifies the condition to
     *              select flow entries.
     */
    public FlowMatcher(OfNode node, Flow cond) {
        tableId = cond.getTableId();
        flowMatch = cond.getMatch();
        BigInteger oport = cond.getOutPort();
        outPort = (oport == null)
            ? null
            : OfMockUtils.getPortIdentifier(node.getNodeIdentifier(), oport);
        cookieMatcher = new CookieMatcher(
            node.getOfVersion(), cond.getCookie(), cond.getCookieMask());
        validMatch = isMatchValid();
    }

    /**
     * Test whether the specified MD-SAL flow entry meets the conditions
     * specified by this instance.
     *
     * @param flow  The MD-SAL flow entry to be tested.
     * @return  {@code true} if the specified flow entry meets the conditions.
     *          {@code false} otherwise.
     */
    public boolean match(Flow flow) {
        boolean result = (checkTableId(flow) && checkMatch(flow));
        if (result) {
            result = (checkOutPort(flow) && cookieMatcher.match(flow));
        }

        return result;
    }

    /**
     * Verify the condition of the flow match.
     *
     * @return  {@code true} if the flow match is valid.
     *          {@code false} otherwise.
     */
    private boolean isMatchValid() {
        boolean result = (flowMatch == null);
        if (!result) {
            result = (flowMatch.getInPhyPort() == null &&
                      flowMatch.getMetadata() == null &&
                      flowMatch.getTunnel() == null &&
                      flowMatch.getEthernetMatch() == null &&
                      flowMatch.getVlanMatch() == null &&
                      flowMatch.getIpMatch() == null &&
                      flowMatch.getLayer3Match() == null &&
                      flowMatch.getLayer4Match() == null &&
                      flowMatch.getIcmpv4Match() == null &&
                      flowMatch.getIcmpv6Match() == null &&
                      flowMatch.getProtocolMatchFields() == null &&
                      flowMatch.getTcpFlagMatch() == null);
        }

        return result;
    }

    /**
     * Check whether the table ID configured in the specified MD-SAL flow
     * entry meets the condition specified by this instance.
     *
     * @param flow  The MD-SAL flow entry to be tested.
     * @return  {@code true} if the specified flow entry meets the conditions.
     *          {@code false} otherwise.
     */
    private boolean checkTableId(Flow flow) {
        return (tableId == null || tableId.equals(flow.getTableId()));
    }

    /**
     * Check whether the flow match configured in the specified MD-SAL flow
     * entry meets the condition specified by this instance.
     *
     * @param flow  The MD-SAL flow entry to be tested.
     * @return  {@code true} if the specified flow entry meets the conditions.
     *          {@code false} otherwise.
     */
    private boolean checkMatch(Flow flow) {
        boolean result = (flowMatch == null);
        if (!result) {
            Match match = flow.getMatch();
            if (match == null) {
                match = EMPTY_MATCH;
            }

            // Currently, only IN_PORT match is supported.
            NodeConnectorId inPort = flowMatch.getInPort();
            result = (inPort == null || inPort.equals(match.getInPort()));
            if (result) {
                // Return false if unsupported filed is specified.
                result = validMatch;
            }
        }
        return result;
    }

    /**
     * Check whether the output port configured in the specified MD-SAL flow
     * entry meets the condition specified by this instance.
     *
     * @param flow  The MD-SAL flow entry to be tested.
     * @return  {@code true} if the specified flow entry meets the conditions.
     *          {@code false} otherwise.
     */
    private boolean checkOutPort(Flow flow) {
        return (outPort == null ||
                OfMockUtils.hasOutput(flow.getInstructions(), outPort));
    }
}
