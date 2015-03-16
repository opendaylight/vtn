/*
 * Copyright (c) 2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.packet;

import org.opendaylight.vtn.manager.util.IpNetwork;

/**
 * {@code InetHeader} describes the contents of IP header.
 */
public interface InetHeader extends ProtocolHeader {
    /**
     * Return the source IP address.
     *
     * @return  An {@link IpNetwork} instance which represents the source
     *          IP address.
     */
    IpNetwork getSourceAddress();

    /**
     * Set the source IP address.
     *
     * @param ipn  An {@link IpNetwork} instance which represents the source
     *             IP address.
     */
    void setSourceAddress(IpNetwork ipn);

    /**
     * Return the destination IP address.
     *
     * @return  An {@link IpNetwork} instance which represents the destination
     *          IP address.
     */
    IpNetwork getDestinationAddress();

    /**
     * Set the destination IP address.
     *
     * @param ipn  An {@link IpNetwork} instance which represents the
     *             destination IP address.
     */
    void setDestinationAddress(IpNetwork ipn);

    /**
     * Return the IP protocol number.
     *
     * @return  A short integer value which indicates the IP protocol number.
     */
    short getProtocol();

    /**
     * Return the DSCP field value.
     *
     * @return  A short value which indicates the DSCP field value.
     */
    short getDscp();

    /**
     * Set the DSCP field value.
     *
     * @param value  A short value which indicates the DSCP field value.
     */
    void setDscp(short value);
}
