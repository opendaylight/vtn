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
import java.util.Iterator;
import java.util.List;
import java.util.ArrayList;
import java.util.Set;
import java.util.HashSet;

import org.opendaylight.vtn.manager.VNodeState;

import org.opendaylight.controller.sal.core.Node;
import org.opendaylight.controller.sal.core.Path;
import org.opendaylight.controller.sal.routing.IRouting;

/**
 * {@code VBridgeState} class keeps runtime state of the virtual L2 bridge.
 *
 * <p>
 *   Note that this class is not synchronized.
 *   If multiple threads access a {@code VBridgeState} object concurrently,
 *   it must be synchronized externally.
 * </p>
 * <p>
 *   Although this class is public to other packages, this class does not
 *   provide any API. Applications other than VTN Manager must not use this
 *   class.
 * </p>
 */
public class VBridgeState implements Serializable {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = 5174713374889497524L;

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
     * Dirty flag.
     *
     * <p>
     *   Dirty flag is set when the interface or port state is changed.
     *   This value never affects identity of the object.
     * </p>
     */
    private transient boolean  dirty = false;

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
     * <p>
     *   If at least one faulted path exists in the bridge, the bridge state
     *   is always changed to {@link VNodeState.DOWN}.
     * </p>
     *
     * @param state  The state of the virtual bridge.
     * @return  The state of the virtual bridge actually set.
     */
    VNodeState setState(VNodeState state) {
        VNodeState newState = (faultedPaths.isEmpty())
            ? state : VNodeState.DOWN;
        if (bridgeState != newState) {
            bridgeState = newState;
            dirty = true;
        }

        return newState;
    }

    /**
     * Return the set of faulted node paths.
     *
     * @return  A set of faulted node paths.
     */
    Set<ObjectPair<Node, Node>> getFaultedPaths() {
        return new HashSet<ObjectPair<Node, Node>>(faultedPaths);
    }

    /**
     * Return the number of faulted paths in the virtual bridge.
     *
     * @return  The number of faulted paths in the virtual bridge.
     */
    int getFaultedPathSize() {
        return faultedPaths.size();
    }

    /**
     * Add the given node path to the set of faulted node paths.
     *
     * <p>
     *   The bridge state is always changed to {@link VNodeState.DOWN}
     *   after the call of this method.
     * </p>
     *
     * @param snode  The source node.
     * @param dnode  The destination node.
     */
    void addFaultedPath(Node snode, Node dnode) {
        ObjectPair<Node, Node> path = new ObjectPair<Node, Node>(snode, dnode);
        if (faultedPaths.add(path)) {
            bridgeState = VNodeState.DOWN;
            dirty = true;
        }
    }

    /**
     * Look for resolved node paths in the set of faulted node paths, and
     * remove them.
     *
     * <p>
     *   This method never changes the bridge state even if all faulted paths
     *   are removed. The caller should update the bridge state appropriately
     *   after the call of this method.
     * </p>
     *
     * @param routing  Routing service.
     * @return  A list of resolved node paths.
     */
    List<ObjectPair<Node, Node>> removeResolvedPath(IRouting routing) {
        List<ObjectPair<Node, Node>> removed =
            new ArrayList<ObjectPair<Node, Node>>();
        for (Iterator<ObjectPair<Node, Node>> it = faultedPaths.iterator();
             it.hasNext();) {
            ObjectPair<Node, Node> npath = it.next();
            Node snode = npath.getLeft();
            Node dnode = npath.getRight();
            Path path = routing.getRoute(snode, dnode);
            if (path != null) {
                removed.add(npath);
                it.remove();
                dirty = true;
            }
        }

        return removed;
    }

    /**
     * Test and clear dirty flag.
     *
     * @return  {@code true} is returned only if this object is dirty.
     */
    boolean isDirty() {
        boolean ret = dirty;
        dirty = false;
        return ret;
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
        return bridgeState.toString().hashCode() ^ faultedPaths.hashCode();
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
