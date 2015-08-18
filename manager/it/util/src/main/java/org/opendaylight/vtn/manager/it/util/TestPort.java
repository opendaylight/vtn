/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.util;

import java.util.Objects;

import org.opendaylight.controller.sal.core.NodeConnector;

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
    private short  vlan;

    /**
     * Construct a new instance.
     *
     * @param pid  The identifier of the MD-SAL node connector.
     */
    public TestPort(String pid) {
        portIdentifier = pid;
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
     * Return the AD-SAL node connector corresponding to the switch port.
     *
     * @return  A {@link NodeConnector} instance.
     */
    public NodeConnector getNodeConnector() {
        return TestBase.toAdNodeConnector(portIdentifier);
    }

    /**
     * Return VLAN ID for test.
     *
     * @return  VLAN ID.
     */
    public short getVlan() {
        return vlan;
    }

    /**
     * Set VLAN ID for test.
     *
     * @param vid  VLAN ID.
     * @return  This instance.
     */
    public TestPort setVlan(short vid) {
        vlan = vid;
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
                vlan == tp.vlan);
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        return Objects.hash(portIdentifier, vlan);
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
            append(",vlan=").append(vlan).append(']');
        return builder.toString();
    }

    /**
     * Clone this object.
     *
     * @return  A shallow copy of this object.
     */
    @Override
    public TestPort clone() {
        try {
            TestPort tp = (TestPort)super.clone();
            tp.portIdentifier = portIdentifier;
            tp.vlan = vlan;
            return tp;
        } catch (CloneNotSupportedException e) {
            // This should never happen.
            throw new IllegalStateException("clone() failed", e);
        }
    }
}
