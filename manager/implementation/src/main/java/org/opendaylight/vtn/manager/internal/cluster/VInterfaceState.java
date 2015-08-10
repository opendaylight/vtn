/*
 * Copyright (c) 2013, 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.cluster;

import java.io.Serializable;

import org.opendaylight.controller.sal.core.NodeConnector;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeState;

/**
 * {@code VInterfaceState} class keeps runtime state of the virtual interface.
 *
 * <p>
 *   Note that this class is not synchronized.
 *   If multiple threads access a {@code VInterfaceState} object concurrently,
 *   it must be synchronized externally.
 * </p>
 * <p>
 *   Although this class is public to other packages, this class does not
 *   provide any API. Applications other than VTN Manager must not use this
 *   class.
 * </p>
 */
public class VInterfaceState implements Serializable {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = 1742777937956679222L;

    /**
     * State of the interface.
     */
    private VnodeState  ifState;

    /**
     * State of the switch port mapped to this interface.
     */
    private VnodeState portState;

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
     * Construct a new virtual interface state.
     *
     * @param state   The state of the virtual interface.
     *                Specifying {@code null} results in undefined behavior.
     */
    VInterfaceState(VnodeState state) {
        this(state, VnodeState.UNKNOWN, null);
    }

    /**
     * Construct a new virtual interface state.
     *
     * @param state      The state of the virtual interface.
     *                   Specifying {@code null} results in undefined behavior.
     * @param portState  The state of the virtual interface.
     *                   Specifying {@code null} results in undefined behavior.
     * @param mapped     Node connector mapped to the virtual interface.
     */
    VInterfaceState(VnodeState state, VnodeState portState,
                    NodeConnector mapped) {
        ifState = state;
        mappedPort = mapped;
        this.portState = portState;
    }

    /**
     * Return the state of the virtual interface.
     *
     * @return  The state of the virtual interface.
     */
    VnodeState getState() {
        return ifState;
    }

    /**
     * Set the state of the virtual interface.
     *
     * <p>
     *   If the specified state differs from the state in this object,
     *   the dirty flag is turned on.
     * </p>
     *
     * @param state  The state of the virtual interface.
     */
    void setState(VnodeState state) {
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
    VnodeState getPortState() {
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
    void setPortState(VnodeState state) {
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
            setPortState(VnodeState.UNKNOWN);
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
        if (!(o instanceof VInterfaceState)) {
            return false;
        }

        VInterfaceState ist = (VInterfaceState)o;
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
        StringBuilder builder = new StringBuilder("VInterfaceState[");
        if (mappedPort != null) {
            builder.append("mapped=").append(mappedPort.toString()).
                append(',');
        }

        builder.append("state=").append(ifState.toString()).
            append(",portState=").append(portState.toString()).append(']');
        return builder.toString();
    }
}
