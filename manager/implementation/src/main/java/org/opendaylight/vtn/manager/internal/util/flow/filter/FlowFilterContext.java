/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.flow.filter;

import org.opendaylight.vtn.manager.internal.util.flow.action.FlowActionContext;
import org.opendaylight.vtn.manager.internal.util.flow.cond.VTNFlowCondition;
import org.opendaylight.vtn.manager.internal.util.flow.match.FlowMatchContext;
import org.opendaylight.vtn.manager.internal.util.packet.UnsupportedPacketException;

/**
 * {@code FlowFilterContext} describes a runtime context to evaluate
 * flow filter against packets.
 */
public interface FlowFilterContext extends FlowMatchContext, FlowActionContext {
    /**
     * Determine whether the packet is going to be broadcasted in the vBridge
     * or not.
     *
     * @return  {@code true} is returned only if the packet is going to be
     *          broadcasted in the vBridge.
     */
    boolean isFlooding();

    /**
     * Set VLAN ID used for packet matching.
     *
     * @param vid  A VLAN ID.
     */
    void setVlanId(int vid);

    /**
     * Install a flow entry that discards the packet.
     */
    void installDropFlow();

    /**
     * Return a {@link VTNFlowCondition} instance which determines whether the
     * specified flow filter needs to be applied to the packet.
     *
     * <p>
     *   Note that this method also checks whether this flow filter supports
     *   the given packet or not.
     * </p>
     *
     * @param ff  A {@link VTNFlowFilter} instance.
     * @return  A {@link VTNFlowCondition} instance which selects the packet.
     * @throws UnsupportedPacketException
     *    The specified flow filter does not support the given packet.
     */
    VTNFlowCondition getFlowCondition(VTNFlowFilter ff)
        throws UnsupportedPacketException;

    /**
     * Set a {@link RedirectFlowException} which represents the first packet
     * redirection in a flow.
     *
     * @param rex  An {@link RedirectFlowException} instance.
     * @return  {@code true} if the given instance represents the first packet
     *          redirection in a flow.
     *          {@code false} otherwise.
     */
    boolean setFirstRedirection(RedirectFlowException rex);

    /**
     * Return a brief description of the ethernet frame in this context.
     *
     * @return  A brief description of the ethernet frame in ths context.
     */
    String getDescription();
}
