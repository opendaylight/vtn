/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.core;

import java.util.Objects;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.bridge.status.FaultedPaths;

/**
 * {@code NodePath} describes a path between switches.
 */
public final class NodePath {
    /**
     * The source node of the path.
     */
    private final String  source;

    /**
     * The destination node of the path.
     */
    private final String  destination;

    /**
     * Construct a new instance.
     *
     * @param src  The node identifier that specifies the source node of the
     *             path.
     * @param dst  The node identifier that specifies the destination node of
     *             the path.
     */
    public NodePath(String src, String dst) {
        source = src;
        destination = dst;
    }

    /**
     * Construct a new instance.
     *
     * @param fpath  A {@link FaultedPaths} instance.
     */
    public NodePath(FaultedPaths fpath) {
        source = fpath.getSource().getValue();
        destination = fpath.getDestination().getValue();
    }

    /**
     * Return the identifier of the source node of the path.
     *
     * @return  The node identifier that specifies the source node of the path.
     */
    public String getSource() {
        return source;
    }

    /**
     * Return the identifier of the destination node of the path.
     *
     * @return  The node identifier that specifies the destinaion node of the
     *          path.
     */
    public String getDestination() {
        return destination;
    }

    // Object

    /**
     * Determine whether the given object is identical to this object.
     *
     * @param o  An object to be compared.
     * @return   {@code true} if identical. Otherwise {@code false}.
     */
    @Override
    public boolean equals(Object o) {
        boolean ret = (o == this);
        if (!ret && o != null && getClass().equals(o.getClass())) {
            NodePath np = (NodePath)o;
            ret = (source.equals(np.source) &&
                   destination.equals(np.destination));
        }

        return ret;
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        return Objects.hash(getClass(), source, destination);
    }

    /**
     * Return a string representation of this object.
     *
     * @return  A string representation of this object.
     */
    @Override
    public String toString() {
        return "NodePath[" + source + " -> " + destination + "]";
    }
}
