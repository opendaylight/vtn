/*
 * Copyright (c) 2015, 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.neutron.impl;

import static org.opendaylight.vtn.manager.util.ByteUtils.HEX_RADIX;
import static org.opendaylight.vtn.manager.util.ByteUtils.HEX_SEPARATOR_MATCHER;
import static org.opendaylight.vtn.manager.util.NumberUtils.getUnsigned;

import java.math.BigInteger;

import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.NodeId;

import org.opendaylight.yang.gen.v1.urn.opendaylight.params.xml.ns.yang.ovsdb.rev150105.DatapathId;

/**
 * {@code OfNode} describes an identifier for a OpenFlow switch.
 */
public final class OfNode {
    /**
     * A node-id prefix that indicates OpenFlow switch.
     */
    private static final String  OF_PREFIX = "openflow:";

    /**
     * An integer value that indicates the data path ID.
     */
    private final BigInteger  nodeNumber;

    /**
     * Cache for the identifier string.
     *
     * <p>
     *   Note that this field does not affect object identity.
     * </p>
     */
    private String  identString;

    /**
     * Construct a new instance.
     *
     * @param dpid  A {@link DatapathId} instance.
     */
    public OfNode(DatapathId dpid) {
        // Eliminate octet separator.
        String hex = HEX_SEPARATOR_MATCHER.removeFrom(dpid.getValue());
        nodeNumber = new BigInteger(hex, HEX_RADIX);
    }

    /**
     * Construct a new instance.
     *
     * @param value  A long value that indicates a datapath ID.
     */
    public OfNode(long value) {
        nodeNumber = getUnsigned(value);
    }

    /**
     * Return the node number.
     *
     * @return  The node number.
     */
    public BigInteger getNodeNumber() {
        return nodeNumber;
    }

    /**
     * Return a {@link NodeId} instance that specifies the same OpenFlow switch
     * as this instance.
     *
     * @return  A {@link NodeId} instance.
     */
    public NodeId getNodeId() {
        return new NodeId(toString());
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
            OfNode on = (OfNode)o;
            ret = nodeNumber.equals(on.nodeNumber);
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
        return getClass().hashCode() + nodeNumber.hashCode();
    }

    /**
     * Return a string representation of this object.
     *
     * <p>
     *   This method returns a string representation of MD-SAL node identifier.
     * </p>
     *
     * @return  A string representation of this object.
     */
    @Override
    public String toString() {
        String ident = identString;
        if (ident == null) {
            ident = OF_PREFIX + nodeNumber.toString();
            identString = ident;
        }

        return ident;
    }
}
