/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.ofmock;

import java.math.BigInteger;

import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.rev131026.FlowModFlags;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.rev131026.flow.Instructions;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.rev131026.flow.Match;

/**
 * {@code OfMockFlow} describes a flow entry created by the mock-up of
 * openflowplugin.
 */
public interface OfMockFlow {
    /**
     * Return the node identifier corresponding to an OpenFlow switch.
     *
     * @return  The node identifier corresponding to an OpenFlow switch.
     */
    String getNodeIdentifier();

    /**
     * Return the table identifier assigned to this flow entry.
     *
     * @return  The table identifier.
     */
    int getTableId();

    /**
     * Return the priority value of this flow entry.
     *
     * @return  The priority of this flow entry.
     */
    int getPriority();

    /**
     * Return the idle timeout value of this flow entry.
     *
     * @return  The idle timeout of this flow entry.
     */
    int getIdleTimeout();

    /**
     * Return the hard timeout value of this flow entry.
     *
     * @return  The hard timeout of this flow entry.
     */
    int getHardTimeout();

    /**
     * Return the cookie associated with this flow entry.
     *
     * @return  The cookie of this flow entry.
     */
    BigInteger getCookie();

    /**
     * Return the condition to match packets.
     *
     * @return  A {@link Match} instance.
     */
    Match getMatch();

    /**
     * Return the instruction of this flow entry.
     *
     * @return  A {@link Instructions} instance.
     */
    Instructions getInstructions();

    /**
     * Return flow-mod flags assigned to this flow entry.
     *
     * @return  A {@link FlowModFlags} instance.
     */
    FlowModFlags getFlowModFlags();
}
