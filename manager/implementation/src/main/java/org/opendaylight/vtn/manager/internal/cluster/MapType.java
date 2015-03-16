/*
 * Copyright (c) 2013-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.cluster;

import org.opendaylight.vtn.manager.VNodeRoute.Reason;

import org.opendaylight.vtn.manager.internal.util.flow.match.FlowMatchType;

/**
 * {@code MapType} class represents types of mappings between virtual and
 * physical network.
 */
public enum MapType {
    /**
     * Port mapping.
     */
    PORT(1 << 0, Reason.PORTMAPPED),

    /**
     * MAC mapping.
     */
    MAC(1 << 1, Reason.MACMAPPED, FlowMatchType.DL_SRC),

    /**
     * VLAN mapping.
     */
    VLAN(1 << 2, Reason.VLANMAPPED),

    /**
     * A pseudo mapping type which means a wildcard.
     */
    ALL(-1);

    /**
     * A bitmask which identifies the type.
     */
    private final int  mask;

    /**
     * A {@link Reason} instance which represents the packet is mapped by
     * this type of virtual mapping.
     */
    private final Reason  reason;

    /**
     * A {@link FlowMatchType} instance which represents flow match field
     * to be specfied in the ingress flow entry.
     */
    private final FlowMatchType  matchType;

    /**
     * Construct a new mapping type.
     *
     * @param mask    A bitmask which identifies the mapping type.
     */
    private MapType(int mask) {
        this(mask, null, null);
    }

    /**
     * Construct a new mapping type.
     *
     * @param mask    A bitmask which identifies the mapping type.
     * @param reason  A {@link Reason} instance associated with the mapping
     *                type.
     */
    private MapType(int mask, Reason reason) {
        this(mask, reason, null);
    }

    /**
     * Construct a new mapping type.
     *
     * @param mask    A bitmask which identifies the mapping type.
     * @param reason  A {@link Reason} instance associated with the mapping
     *                type.
     * @param mtype   A {@link FlowMatchType} instance which represents flow
     *                match fields to specify the packet.
     */
    private MapType(int mask, Reason reason, FlowMatchType mtype) {
        this.mask = mask;
        this.reason = reason;
        matchType = mtype;
    }

    /**
     * Determine whether the specified mapping type matches this type.
     *
     * @param type  A mapping type to be tested.
     * @return  {@code true} is returned only if {@code type} matches this
     *          type.
     */
    public boolean match(MapType type) {
        return ((mask & type.mask) != 0);
    }

    /**
     * Return a {@link Reason} instance associated with this type of
     * virtual mapping.
     *
     * @return  A {@link Reason} instance.
     */
    public Reason getReason() {
        return reason;
    }

    /**
     * Return a {@link FlowMatchType} instance which represents the flow match
     * field to be specified in the ingress flow entry.
     *
     * @return  A {@link FlowMatchType} instance.
     *          {@code null} is returned if no additional match field is
     *          required.
     */
    public FlowMatchType getMatchType() {
        return matchType;
    }
}
