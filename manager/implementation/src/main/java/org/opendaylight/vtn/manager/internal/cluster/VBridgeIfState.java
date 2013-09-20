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

import org.opendaylight.vtn.manager.VNodeState;

import org.opendaylight.controller.sal.core.NodeConnector;

/**
 * {@code VBridgeIfState} class keeps runtime state of the virtual L2 bridge
 * interface.
 *
 * <p>
 *   Although this class is public to other packages, this class does not
 *   provide any API. Applications other than VTN Manager must not use this
 *   class.
 * </p>
 */
public class VBridgeIfState implements Serializable {
    private static final long serialVersionUID = -8925788210043958442L;

    /**
     * State of the interface.
     */
    private VNodeState  ifState;

    /**
     * State of the switch port mapped to this interface.
     */
    private VNodeState portState;

    /**
     * Node connector actually mapped to this interface.
     */
    private NodeConnector  mappedPort;

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
     * Construct a new virtual bridge interface state.
     *
     * @param state   The state of the virtual bridge interface.
     *                Specifying {@code null} results in undefined behavior.
     */
    VBridgeIfState(VNodeState state) {
        this(state, VNodeState.UNKNOWN, null);
    }

    /**
     * Construct a new virtual bridge interface state.
     *
     * @param state      The state of the virtual bridge interface.
     *                   Specifying {@code null} results in undefined behavior.
     * @param portState  The state of the virtual bridge interface.
     *                   Specifying {@code null} results in undefined behavior.
     * @param mapped     Node connector mapped to the bridge interface.
     */
    VBridgeIfState(VNodeState state, VNodeState portState,
                   NodeConnector mapped) {
        ifState = state;
        mappedPort = mapped;
        this.portState = portState;
    }

    /**
     * Return the state of the virtual bridge interface.
     *
     * @return  The state of the virtual bridge interface.
     */
    VNodeState getState() {
        return ifState;
    }

    /**
     * Set the state of the virtual bridge interface.
     *
     * <p>
     *   If the specified state differs from the state in this object,
     *   the dirty flag is turned on.
     * </p>
     *
     * @param state  The state of the virtual bridge interface.
     */
    void setState(VNodeState state) {
        if (ifState != state) {
            ifState = state;
            dirty = true;
        }
    }

    /**
     * Return the state of the switch port mapped to this interface.
     *
     * @return  The state of the switch port mapped to this interface.
     */
    VNodeState getPortState() {
        return portState;
    }

    /**
     * Set the state of the switch port mapped to this interface.
     *
     * <p>
     *   If the specified state differs from the state in this object,
     *   the dirty flag is turned on.
     * </p>
     *
     * @param state  The state of the switch port.
     */
    void setPortState(VNodeState state) {
        if (portState != state) {
            portState = state;
            dirty = true;
        }
    }

    /**
     * Return the node connector associated with the mapped physical switch
     * port.
     *
     * @return  Node connector. {@code null} is returned if no port is mapped.
     */
    NodeConnector getMappedPort() {
        return mappedPort;
    }

    /**
     * Set the node connector associated with the mapped physical switch
     * port.
     *
     * @param mapped  Node connector. {@code null} must be passed if no port
     *                is mapped.
     */
    void setMappedPort(NodeConnector mapped) {
        mappedPort = mapped;
        if (mapped == null) {
            setPortState(VNodeState.UNKNOWN);
        }
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
        if (!(o instanceof VBridgeIfState)) {
            return false;
        }

        VBridgeIfState ist = (VBridgeIfState)o;
        if (ifState != ist.ifState) {
            return false;
        }
        if (portState != ist.portState) {
            return false;
        }

        if (mappedPort == null) {
            return (ist.mappedPort == null);
        }

        return mappedPort.equals(ist.mappedPort);
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        int h = ifState.toString().hashCode() ^
            portState.toString().hashCode();
        if (mappedPort != null) {
            h ^= mappedPort.hashCode();
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
        StringBuilder builder = new StringBuilder("VBridgeIfState[");
        if (mappedPort != null) {
            builder.append("mapped=").append(mappedPort.toString()).
                append(',');
        }

        builder.append("state=").append(ifState.toString()).
            append(",portState=").append(portState.toString()).append(']');
        return builder.toString();
    }
}
