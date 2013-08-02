/*
 * Copyright (c) 2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager;

/**
 * {@code VNodeState} class represents the state of the virtual node in the
 * VTN, such as virtual L2 bridge.
 */
public enum VNodeState {
    /**
     * Node state value that indicates the state is unavailable.
     */
    UNKNOWN,

    /**
     * Node state value that indicates the node is down.
     */
    DOWN,

    /**
     * Node state value that indicates the node is up.
     */
    UP;

    /**
     * Return a numerical representtion of the node state.
     *
     * @return  A numerical representation of the node state.
     */
    public int getValue() {
        return ordinal() - 1;
    }

    /**
     * Return a {@code VNodeState} object equivalent to the specified integer.
     *
     * @param st  A numerical representation of the node state.
     * @return  A {@code VNodeState} object.
     */
    public static VNodeState valueOf(int st) {
        int idx = st + 1;
        VNodeState[] states = VNodeState.values();
        try {
            return states[idx];
        } catch (ArrayIndexOutOfBoundsException e) {
            return VNodeState.UNKNOWN;
        }
    }
}
