/*
 * Copyright (c) 2013, 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.cluster;

import java.io.Serializable;
import java.util.Collections;
import java.util.Iterator;
import java.util.List;
import java.util.ArrayList;
import java.util.Set;
import java.util.HashSet;

import org.opendaylight.vtn.manager.internal.RouteResolver;
import org.opendaylight.vtn.manager.internal.TxContext;
import org.opendaylight.vtn.manager.internal.util.inventory.InventoryReader;
import org.opendaylight.vtn.manager.internal.util.inventory.LinkEdge;
import org.opendaylight.vtn.manager.internal.util.inventory.SalNode;

import org.opendaylight.controller.sal.core.Node;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeState;

/**
 * {@code BridgeState} class keeps runtime state of the virtual bridge node.
 *
 * <p>
 *   Note that this class is not synchronized.
 *   If multiple threads access a {@code BridgeState} object concurrently,
 *   it must be synchronized externally.
 * </p>
 * <p>
 *   Although this class is public to other packages, this class does not
 *   provide any API. Applications other than VTN Manager must not use this
 *   class.
 * </p>
 */
public class BridgeState implements Serializable {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = -6810750007547565021L;

    /**
     * State of the bridge.
     */
    private VnodeState  bridgeState;

    /**
     * Set of faulted paths.
     */
    private Set<ObjectPair<Node, Node>>  faultedPaths;

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
    BridgeState(VnodeState state) {
        bridgeState = state;
    }

    /**
     * Return the state of the virtual bridge.
     *
     * @return  The state of the virtual bridge.
     */
    VnodeState getState() {
        return bridgeState;
    }

    /**
     * Set the state of the virtual bridge.
     *
     * <p>
     *   If at least one faulted path exists in the bridge, the bridge state
     *   is always changed to {@link VnodeState#DOWN}.
     * </p>
     *
     * @param state  The state of the virtual bridge.
     * @return  The state of the virtual bridge actually set.
     */
    VnodeState setState(VnodeState state) {
        VnodeState newState = (faultedPaths == null || faultedPaths.isEmpty())
            ? state : VnodeState.DOWN;
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
        return (faultedPaths == null)
            ? new HashSet<ObjectPair<Node, Node>>()
            : new HashSet<ObjectPair<Node, Node>>(faultedPaths);
    }

    /**
     * Return the number of faulted paths in the virtual bridge.
     *
     * @return  The number of faulted paths in the virtual bridge.
     */
    int getFaultedPathSize() {
        return (faultedPaths == null) ? 0 : faultedPaths.size();
    }

    /**
     * Add the given node path to the set of faulted node paths.
     *
     * <p>
     *   The bridge state is always changed to {@link VnodeState#DOWN}
     *   after the call of this method.
     * </p>
     *
     * @param snode  The source node.
     * @param dnode  The destination node.
     * @return  {@code true} is returned if the specified node path is
     *          actually added to the faulted path set.
     *          {@code false} is returned if it already exists in the set.
     */
    boolean addFaultedPath(Node snode, Node dnode) {
        ObjectPair<Node, Node> path = new ObjectPair<Node, Node>(snode, dnode);
        Set<ObjectPair<Node, Node>> set = faultedPaths;
        if (set == null) {
            set = new HashSet<ObjectPair<Node, Node>>();
            faultedPaths = set;
        }
        boolean ret = set.add(path);
        if (ret) {
            bridgeState = VnodeState.DOWN;
            dirty = true;
        }

        return ret;
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
     * @param ctx  MD-SAL datastore transaction context.
     * @return  A list of resolved node paths.
     */
    List<ObjectPair<Node, Node>> removeResolvedPath(TxContext ctx) {
        List<ObjectPair<Node, Node>> removed =
            new ArrayList<ObjectPair<Node, Node>>();
        if (faultedPaths != null) {
            for (Iterator<ObjectPair<Node, Node>> it = faultedPaths.iterator();
                 it.hasNext();) {
                ObjectPair<Node, Node> npath = it.next();
                SalNode snode = SalNode.create(npath.getLeft());
                SalNode dnode = SalNode.create(npath.getRight());
                if (snode == null || dnode == null) {
                    // This should never happen.
                    continue;
                }

                RouteResolver rr = ctx.getProvider().getRouteResolver();
                InventoryReader reader = ctx.getInventoryReader();
                List<LinkEdge> path = rr.getRoute(reader, snode, dnode);
                if (path != null) {
                    removed.add(npath);
                    it.remove();
                    dirty = true;
                }
            }
            if (faultedPaths.isEmpty()) {
                faultedPaths = null;
            }
        }

        return removed;
    }

    /**
     * Remove paths that contain the specified node from the set of
     * faulted paths.
     *
     * @param node  A {@link Node} corresponding to a physical switch.
     * @return  The number of removed faulted paths.
     */
    int removeFaultedPath(Node node) {
        int removed = 0;
        if (faultedPaths != null) {
            for (Iterator<ObjectPair<Node, Node>> it = faultedPaths.iterator();
                 it.hasNext();) {
                ObjectPair<Node, Node> npath = it.next();
                Node snode = npath.getLeft();
                Node dnode = npath.getRight();
                if (snode.equals(node) || dnode.equals(node)) {
                    it.remove();
                    removed++;
                    dirty = true;
                }
            }
            if (faultedPaths.isEmpty()) {
                faultedPaths = null;
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
        if (!(o instanceof BridgeState)) {
            return false;
        }

        BridgeState bst = (BridgeState)o;
        if (bridgeState != bst.bridgeState) {
            return false;
        }

        if (faultedPaths == null) {
            return (bst.faultedPaths == null);
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
        int h = bridgeState.toString().hashCode();
        if (faultedPaths != null) {
            h += faultedPaths.hashCode() * 13;
        }

        return h;
    }

    /**
     * Return a string representation of this object.
     *
     * @return  A string representation of this object.
     */
    @Override
    public String toString() {
        StringBuilder builder = new StringBuilder("BridgeState[");
        Set<ObjectPair<Node, Node>> set = faultedPaths;
        if (set == null) {
            set = Collections.emptySet();
        }
        builder.append("state=").append(bridgeState.toString()).
            append(",faulted=").append(set).append(']');
        return builder.toString();
    }
}
