/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.flow.filter;

import org.opendaylight.vtn.manager.internal.util.vnode.VInterfaceIdentifier;

/**
 * An exception which indicates a packet was redirected to the virtual
 * interface by flow filter.
 */
public final class RedirectFlowException extends Exception {
    /**
     * A string that indicates the virtual node that contains the REDIRECT
     * flow filter.
     */
    private final String  filterPath;

    /**
     * The name of the flow condition configured in the REDIRECT flow filter.
     */
    private final String  condition;

    /**
     * The location of the destination virtual interface.
     */
    private final VInterfaceIdentifier<?>  destination;

    /**
     * Determine the direction of the packet redirection.
     *
     * <p>
     *   {@code true} means the packet is redirected as outgoing packet.
     * </p>
     */
    private final boolean  outgoing;

    /**
     * Construct a new instance.
     *
     * @param fpath  A string that indicates the location of the REDIRECT
     *               flow filter.
     * @param cond   The name of the flow condition configured in the REDIRECT
     *               flow filter.
     * @param dst    The identifier for the destination virtual interface.
     * @param toOut  A boolean value that determines the direction of the
     *               redirection.
     */
    public RedirectFlowException(String fpath, String cond,
                                 VInterfaceIdentifier<?> dst, boolean toOut) {
        filterPath = fpath;
        condition = cond;
        destination = dst;
        outgoing = toOut;
    }

    /**
     * Return a string that indicates the location of the REDIRECT flow filter.
     *
     * @return  A string that indicates the location of the REDIRECT flow
     *          filter.
     */
    public String getFilterPath() {
        return filterPath;
    }

    /**
     * Return the name of the flow condition configured in the REDIRECT
     * flow filter.
     *
     * @return  The name of the flow condition.
     */
    public String getCondition() {
        return condition;
    }

    /**
     * Return the identifier for the virtual interface where the packet should
     * be redirected.
     *
     * @return  The identifier for the destination virtual interface.
     */
    public VInterfaceIdentifier<?> getDestination() {
        return destination;
    }

    /**
     * Determine whether the direction of packet redirection.
     *
     * @return  {@code true} if the redirected packet should be treated as
     *          an outgoing packet.
     *          {@code false} if the redirected packet should be treated as
     *          an incoming packet.
     */
    public boolean isOutgoing() {
        return outgoing;
    }
}
