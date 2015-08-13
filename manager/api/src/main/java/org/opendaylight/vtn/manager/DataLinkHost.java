/*
 * Copyright (c) 2014, 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager;

import java.io.Serializable;

import org.opendaylight.controller.sal.packet.address.DataLinkAddress;

/**
 * {@code DataLinkHost} class is an abstract class used to represent
 * a host information for data link layer.
 *
 * @since  Helium
 */
public abstract class DataLinkHost implements Serializable {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = 885155389617749850L;

    /**
     * Data link layer address.
     *
     * <p>
     *   {@code null} means that the address is not specified.
     * </p>
     */
    private final DataLinkAddress  address;

    /**
     * Construct a new instance which represents a host information for
     * data link layer.
     *
     * @param addr  A {@link DataLinkAddress} instance which represents a
     *              data link layer address. Specifying {@code null} means
     *              that the address is not specified.
     */
    protected DataLinkHost(DataLinkAddress addr) {
        address = addr;
    }

    /**
     * Return a data link layer address configured in this instance.
     *
     * @return  A {@link DataLinkAddress} instance which represents a data
     *          link address. {@code null} is returned if it is not configured
     *          in this instance.
     */
    public DataLinkAddress getAddress() {
        return address;
    }

    /**
     * Determine whether the given object is identical to this object.
     *
     * <p>
     *   {@code true} is returned only if all the following conditions are met.
     * </p>
     * <ul>
     *   <li>
     *     The class of {@code o} is identical to the class of this instance.
     *   </li>
     *   <li>
     *     The following values stored in {@code o} are the same as in this
     *     object.
     *     <ul>
     *       <li>
     *         A {@link DataLinkAddress} instance which represents a data link
     *         layer address.
     *       </li>
     *     </ul>
     *   </li>
     * </ul>
     *
     * @param o  An object to be compared.
     * @return   {@code true} if identical. Otherwise {@code false}.
     */
    @Override
    public boolean equals(Object o) {
        if (o == null || !getClass().equals(o.getClass())) {
            return false;
        }

        DataLinkHost dlhost = (DataLinkHost)o;
        if (address == null) {
            return (dlhost.address == null);
        }

        return address.equals(dlhost.address);
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        int h = getClass().getName().hashCode();
        if (address != null) {
            h += address.hashCode() * 127;
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
        StringBuilder builder = new StringBuilder("DataLinkHost[");
        if (address != null) {
            builder.append("address=").append(address.toString());
        }

        builder.append(']');

        return builder.toString();
    }
}
