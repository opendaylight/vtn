/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal;

import java.util.Objects;

import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.inet.types.rev100924.PortNumber;

/**
 * An implementation of {@link PortNumber} that bypasses value check.
 */
public final class TestPortNumber extends PortNumber {
    /**
     * A port number.
     */
    private final Integer  portNumber;

    /**
     * Construct an empty instance.
     */
    public TestPortNumber() {
        this(null);
    }

    /**
     * Construct a new instance.
     *
     * @param port  A port number.
     */
    public TestPortNumber(Integer port) {
        super(1);
        portNumber = port;
    }

    // PortNumber

    /**
     * Return a port number.
     *
     * @return  A port number.
     */
    @Override
    public Integer getValue() {
        return portNumber;
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
            TestPortNumber pn = (TestPortNumber)o;
            ret = Objects.equals(portNumber, pn.portNumber);
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
        return Objects.hash(getClass(), portNumber);
    }
}
