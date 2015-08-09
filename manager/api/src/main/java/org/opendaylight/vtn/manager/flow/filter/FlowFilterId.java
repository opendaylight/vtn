/*
 * Copyright (c) 2014, 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.flow.filter;

import java.io.Serializable;

import org.opendaylight.vtn.manager.VBridgeIfPath;
import org.opendaylight.vtn.manager.VBridgePath;
import org.opendaylight.vtn.manager.VTenantPath;
import org.opendaylight.vtn.manager.VTerminalIfPath;

/**
 * {@code FlowFilter} class describes the location of the list of
 * {@linkplain <a href="package-summary.html">flow filter</a>}.
 *
 * <p>
 *   An instance of this class keeps the location of the virtual node, and the
 *   flow direction to be applied. Flow filter APIs use this class to
 *   specify the target flow filter list.
 * </p>
 *
 * @since  Helium
 */
public final class FlowFilterId implements Serializable {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = -5923835918886627614L;

    /**
     * The location of the target virtual node.
     */
    private final VTenantPath  nodePath;

    /**
     * Flow direction for the flow filter.
     *
     * <p>
     *   {@code true} specifies the flow filter to be applied to outgoing
     *   packets.
     * </p>
     */
    private final boolean  output;

    /**
     * Construct a new instance that specifies the flow filter in the
     * specified {@linkplain <a href="../../package-summary.html#VTN">VTN</a>}.
     *
     * @param path  The location of the target VTN.
     */
    public FlowFilterId(VTenantPath path) {
        nodePath = path;
        output = false;
    }

    /**
     * Construct a new instance that specifies the flow filter in the
     * specified
     * {@linkplain <a href="../../package-summary.html#vBridge">vBridge</a>}.
     *
     * @param path  The location of the target vBridge.
     * @param out
     *   Flow direction for the flow filter.
     *   <ul>
     *     <li>
     *       If {@code true} is specified, a new instance specifies the
     *       flow filter to be applied to outgoing packets.
     *     </li>
     *     <li>
     *       If {@code false} is specified, a new instance specifies the
     *       flow filter to be applied to incoming packets.
     *     </li>
     *   </ul>
     */
    public FlowFilterId(VBridgePath path, boolean out) {
        nodePath = path;
        output = out;
    }

    /**
     * Construct a new instance that specifies the flow filter in the
     * specified
     * {@linkplain <a href="../../package-summary.html#vInterface">virtual interface</a>}
     * inside the
     * {@linkplain <a href="../../package-summary.html#vBridge">vBridge</a>}.
     *
     * @param path  The location of the target vBridge interface.
     * @param out
     *   Flow direction for the flow filter.
     *   <ul>
     *     <li>
     *       If {@code true} is specified, a new instance specifies the
     *       flow filter to be applied to outgoing packets.
     *     </li>
     *     <li>
     *       If {@code false} is specified, a new instance specifies the
     *       flow filter to be applied to incoming packets.
     *     </li>
     *   </ul>
     */
    public FlowFilterId(VBridgeIfPath path, boolean out) {
        nodePath = path;
        output = out;
    }

    /**
     * Construct a new instance that specifies the flow filter in the
     * specified
     * {@linkplain <a href="../../package-summary.html#vInterface">virtual interface</a>}
     * inside the
     * {@linkplain <a href="../../package-summary.html#vTerminal">vTerminal</a>}.
     *
     * @param path  The location of the target vTerminal interface.
     * @param out
     *   Flow direction for the flow filter.
     *   <ul>
     *     <li>
     *       If {@code true} is specified, a new instance specifies the
     *       flow filter to be applied to outgoing packets.
     *     </li>
     *     <li>
     *       If {@code false} is specified, a new instance specifies the
     *       flow filter to be applied to incoming packets.
     *     </li>
     *   </ul>
     */
    public FlowFilterId(VTerminalIfPath path, boolean out) {
        nodePath = path;
        output = out;
    }

    /**
     * Return the location of the target virtual node.
     *
     * @return  The location of the target virtual node.
     */
    public VTenantPath getPath() {
        return nodePath;
    }

    /**
     * Determine flow direction for the flow filter.
     *
     * @return
     *   <ul>
     *     <li>
     *       {@code true} is returned if this instance specifies the
     *       flow filter to be applied to outgoing packets.
     *     </li>
     *     <li>
     *       {@code false} is returned if this instance specifies the
     *       flow filter to be applied to incoming packets.
     *     </li>
     *   </ul>
     */
    public boolean isOutput() {
        return output;
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
        if (o == null || !getClass().equals(o.getClass())) {
            return false;
        }

        FlowFilterId fid = (FlowFilterId)o;
        return (nodePath.equals(fid.nodePath) && output == fid.output);
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        return nodePath.hashCode() * Boolean.valueOf(output).hashCode();
    }

    /**
     * Return a string representation of this object.
     *
     * @return  A string representation of this object.
     */
    @Override
    public String toString() {
        StringBuilder builder = new StringBuilder("FlowFilterId[path=");
        return builder.append(nodePath).append(",output=").append(output).
            append(']').toString();
    }
}
