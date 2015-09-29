/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal;

import java.util.Objects;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnPortDesc;

/**
 * An implementation of {@link VtnPortDesc} that bypasses value check.
 */
public final class TestVtnPortDesc extends VtnPortDesc {
    /**
     * The port descriptor.
     */
    private final String  portDesc;

    /**
     * Construct an empty instance.
     */
    public TestVtnPortDesc() {
        this(null);
    }

    /**
     * Construct a new instance.
     *
     * @param desc  The port descriptor string.
     */
    public TestVtnPortDesc(String desc) {
        super("openflow:1,1,port-1");
        portDesc = desc;
    }

    // VtnPortDesc

    /**
     * Return the port descriptor.
     *
     * @return  The port descriptor string.
     */
    @Override
    public String getValue() {
        return portDesc;
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
            TestVtnPortDesc vdesc = (TestVtnPortDesc)o;
            ret = Objects.equals(portDesc, vdesc.portDesc);
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
        return Objects.hash(getClass(), portDesc);
    }
}
