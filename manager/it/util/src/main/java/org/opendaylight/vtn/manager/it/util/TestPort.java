/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.util;

import java.util.Objects;

/**
 * {@code TestPort} describes a switch port and VLAN network used for
 * integration test.
 */
public final class TestPort implements Cloneable {
    /**
     * MD-SAL node connector identifier.
     */
    private String  portIdentifier;

    /**
     * VLAN ID for test.
     */
    private int  vlanId;

    /**
     * Construct a new instance.
     *
     * @param pid  The identifier of the MD-SAL node connector.
     */
    public TestPort(String pid) {
        portIdentifier = pid;
    }

    /**
     * Construct a new instance.
     *
     * @param pid  The identifier of the MD-SAL node connector.
     * @param vid  The VLAN ID.
     */
    public TestPort(String pid, int vid) {
        portIdentifier = pid;
        vlanId = vid;
    }

    /**
     * Return the MD-SAL port identifier.
     *
     * @return  The MD-SAL port identifier.
     */
    public String getPortIdentifier() {
        return portIdentifier;
    }

    /**
     * Return VLAN ID for test.
     *
     * @return  VLAN ID.
     */
    public int getVlanId() {
        return vlanId;
    }

    /**
     * Set VLAN ID for test.
     *
     * @param vid  VLAN ID.
     * @return  This instance.
     */
    public TestPort setVlanId(int vid) {
        vlanId = vid;
        return this;
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

        TestPort tp = (TestPort)o;

        return (Objects.equals(portIdentifier, tp.portIdentifier) &&
                vlanId == tp.vlanId);
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        return Objects.hash(portIdentifier, vlanId);
    }

    /**
     * Return a string representation of this object.
     *
     * @return  A string representation of this object.
     */
    @Override
    public String toString() {
        StringBuilder builder = new StringBuilder("TestPort[id=").
            append(portIdentifier).
            append(",vlan=").append(vlanId).append(']');
        return builder.toString();
    }

    /**
     * Make a clone of this object.
     *
     * @return  A shallow copy of this object.
     */
    @Override
    public TestPort clone() {
        try {
            return (TestPort)super.clone();
        } catch (CloneNotSupportedException e) {
            // This should never happen.
            throw new IllegalStateException("clone() failed", e);
        }
    }
}
