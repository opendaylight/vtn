/*
 * Copyright (c) 2014, 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.northbound;

import org.opendaylight.controller.sal.packet.address.DataLinkAddress;

import org.opendaylight.vtn.manager.DataLinkHost;

/**
 * A pseudo host information in data link layer.
 */
public class TestDataLinkHost extends DataLinkHost {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = 1L;

    /**
     * Construct a new host information.
     *
     * @param addr  A {@link DataLinkAddress} which represents a data link
     *              layer address.
     */
    public TestDataLinkHost(DataLinkAddress addr) {
        super(addr);
    }

    /**
     * Determine whether the given object is identical to this object.
     *
     * @param o  An object to be compared.
     * @return   {@code true} if identical. Otherwise {@code false}.
     */
    @Override
    public boolean equals(Object o) {
        return ((o instanceof TestDataLinkHost) && super.equals(o));
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        return getClass().hashCode() * super.hashCode();
    }
}
