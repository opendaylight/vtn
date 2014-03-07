/*
 * Copyright (c) 2013-2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.cluster;

/**
 * {@code MapType} class represents types of mappings between virtual and
 * physical network.
 */
public enum MapType {
    /**
     * Port mapping.
     */
    PORT(1 << 0),

    /**
     * VLAN mapping.
     */
    VLAN(1 << 1),

    /**
     * A pseudo mapping type which means a wildcard.
     */
    ALL(-1);

    /**
     * A bitmask which identifies the type.
     */
    private final int  mask;

    /**
     * Construct a new mapping type.
     *
     * @param mask  A bitmask which identifies the mapping type.
     */
    private MapType(int mask) {
        this.mask = mask;
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
}
