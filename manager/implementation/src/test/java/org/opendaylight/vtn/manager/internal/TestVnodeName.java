/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal;

import java.util.Objects;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeName;

/**
 * An implementation of {@link VnodeName} that bypasses value check.
 */
public final class TestVnodeName extends VnodeName {
    /**
     * The name of the virtual node.
     */
    private final String  vnodeName;

    /**
     * Construct an empty instance.
     */
    public TestVnodeName() {
        this(null);
    }

    /**
     * Construct a new instance.
     *
     * @param name  The name of the virtual node.
     */
    public TestVnodeName(String name) {
        super("vtn");
        vnodeName = name;
    }

    // VnodeName

    /**
     * Return the name of the virtual node.
     *
     * @return  The name of the virtual node.
     */
    @Override
    public String getValue() {
        return vnodeName;
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
            TestVnodeName vid = (TestVnodeName)o;
            ret = Objects.equals(vnodeName, vid.vnodeName);
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
        return Objects.hash(getClass(), vnodeName);
    }
}
