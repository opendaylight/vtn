/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.flow.filter;

import static org.opendaylight.vtn.manager.util.NumberUtils.HASH_PRIME;

import org.opendaylight.vtn.manager.internal.util.vnode.VNodeIdentifier;

/**
 * {@code FlowFilterListId} describes the location of the flow filter list.
 */
public final class FlowFilterListId {
    /**
     * The identifier for the virtual node that contains the flow filter list.
     */
    private final VNodeIdentifier<?>  identifier;

    /**
     * A boolean value that specifies the flow direction.
     */
    private final boolean  output;

    /**
     * A cache for a string representation of this instance.
     */
    private String  idString;

    /**
     * Return a string that indicates the direction of the flow.
     *
     * @param out    A boolean value that specifies the flow direction.
     *               {@code true} indicates the output filter list.
     *               {@code false} indicates the input filter list.
     * @return  A string that indicates the direction of the flow.
     */
    public static String getFlowDirectionName(boolean out) {
        return (out) ? "OUT" : "IN";
    }

    /**
     * Construct a new instance.
     *
     * @param ident  The identifier for the virtual node that contains the
     *               flow filter list.
     * @param out    A boolean value that specifies the flow direction.
     *               {@code true} indicates the output filter list.
     *               {@code false} indicates the input filter list.
     */
    public FlowFilterListId(VNodeIdentifier<?> ident, boolean out) {
        identifier = ident;
        output = out;
    }

    /**
     * Return the identifier for the virtual node that contains the flow filter
     * list.
     *
     * @return  A {@link VNodeIdentifier} instance that specifies the
     *          virtual node that contains the flow filter list.
     */
    public VNodeIdentifier<?> getIdentifier() {
        return identifier;
    }

    /**
     * Return a boolean value that specifies the flow direction.
     *
     * @return  {@code true} if this instance specifies the output flow filter
     *          list.
     *          {@code false} if this instance specifies the input flow filter
     *          list.
     */
    public boolean isOutput() {
        return output;
    }

    /**
     * Return a string that specifies the flow filter specified by the
     * filter index.
     *
     * @param index  An index value assigned to the flow filter.
     * @return  A string that specifies the flow filter specified by the
     *          filter index.
     */
    public String getFilterId(int index) {
        return toString() + "." + index;
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
            FlowFilterListId flid = (FlowFilterListId)o;
            ret = (identifier.equals(flid.identifier) &&
                   output == flid.output);
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
        return identifier.hashCode() * HASH_PRIME +
            Boolean.valueOf(output).hashCode();
    }

    /**
     * Return a string representation of this instance.
     *
     * @return  A string representation of this instance.
     */
    @Override
    public String toString() {
        String id = idString;
        if (id == null) {
            id = identifier.toString() + "%" + getFlowDirectionName(output);
            idString = id;
        }

        return id;
    }
}
