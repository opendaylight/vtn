/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.packet;

/**
 * {@code IcmpHeader} describes the contents of ICMP header.
 */
public interface IcmpHeader extends Layer4Header {
    /**
     * Return the ICMP type.
     *
     * @return  A short integer value which indicates the ICMP type.
     */
    short getIcmpType();

    /**
     * Set the ICMP type.
     *
     * @param type  A short integer value which indicates the ICMP type.
     */
    void setIcmpType(short type);

    /**
     * Return the ICMP code.
     *
     * @return  A short integer value which indicates the ICMP code.
     */
    short getIcmpCode();

    /**
     * Set the ICMP code.
     *
     * @param code  A short integer value which indicates the ICMP code.
     */
    void setIcmpCode(short code);
}
