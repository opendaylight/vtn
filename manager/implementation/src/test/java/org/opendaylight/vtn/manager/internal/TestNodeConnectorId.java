/*
 * Copyright (c) 2017 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal;

import java.util.Objects;

import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.NodeConnectorId;

/**
 * An implementation of {@link NodeConnectorId} that bypasses value check.
 */
public final class TestNodeConnectorId extends NodeConnectorId {
    /**
     * A value of node-connector-id.
     */
    private final String  value;

    /**
     * Construct an empty instance.
     */
    public TestNodeConnectorId() {
        this(null);
    }

    /**
     * Construct a new instance.
     *
     * @param v  A value of node-connector-id.
     */
    public TestNodeConnectorId(String v) {
        super("openflow:1:2");
        value = v;
    }

    // Uri

    /**
     * Return the value of this node-connector-id.
     *
     * @return  The value of this node-connector-id.
     */
    @Override
    public String getValue() {
        return value;
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
            TestNodeConnectorId ncId = (TestNodeConnectorId)o;
            ret = Objects.equals(value, ncId.value);
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
        return Objects.hash(getClass(), value);
    }
}
