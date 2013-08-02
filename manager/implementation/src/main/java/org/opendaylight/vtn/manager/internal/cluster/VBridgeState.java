/*
 * Copyright (c) 2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.cluster;

import java.io.Serializable;
import java.util.Set;
import java.util.HashSet;

import org.opendaylight.vtn.manager.VNodeState;

import org.opendaylight.controller.sal.core.Node;

/**
 * {@code VBridgeState} class keeps runtime state of the virtual L2 bridge.
 *
 * <p>
 *   Although this class is public to other packages, this class does not
 *   provide any API. Applications other than VTN Manager must not use this
 *   class.
 * </p>
 */
public class VBridgeState implements Serializable {
    private final static long serialVersionUID = 1612550382068903129L;

    /**
     * State of the bridge.
     */
    private VNodeState  bridgeState;

    /**
     * Set of faulted paths.
     */
    private final Set<ObjectPair<Node, Node>>  faultedPaths =
        new HashSet<ObjectPair<Node, Node>>();

    /**
     * Construct a new virtual bridge state.
     *
     * @param state  The state of the virtual bridge.
     *               Specifying {@code null} results in undefined behavior.
     */
    VBridgeState(VNodeState state) {
        bridgeState = state;
    }

    /**
     * Return the state of the virtual bridge.
     *
     * @return  The state of the virtual bridge.
     */
    VNodeState getState() {
        return bridgeState;
    }

    /**
     * Set the state of the virtual bridge.
     *
     * @param state  The state of the virtual bridge.
     * @return  {@code true} is returned only if the state was changed.
     */
    boolean setState(VNodeState state) {
        if (bridgeState != state) {
            bridgeState = state;
            return true;
        }

        return false;
    }

    /**
     * Return the set of faulted node paths.
     *
     * @return  A set of faulted node paths.
     */
    Set<ObjectPair<Node, Node>> getFaultedPaths() {
        return faultedPaths;
    }

    /**
     * Determine whether the given object is identical to this object.
     *
     * @param o  An object to be compared.
     * @return   {@code true} if identical. Otherwise {@code false}.
     */
    @Override
    public boolean equals(Object o) {
        if (!(o instanceof VBridgeState)) {
            return false;
        }

        VBridgeState bst = (VBridgeState)o;
        if (bridgeState != bst.bridgeState) {
            return false;
        }

        return faultedPaths.equals(bst.faultedPaths);
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        return 641 + bridgeState.hashCode() + faultedPaths.hashCode();
    }

    /**
     * Return a string representation of this object.
     *
     * @return  A string representation of this object.
     */
    @Override
    public String toString() {
        StringBuilder builder = new StringBuilder("VBridgeState[");
        builder.append("state=").append(bridgeState.toString()).
            append(",faulted=").append(faultedPaths).append(']');
        return builder.toString();
    }
}
