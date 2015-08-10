/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.packet;

/**
 * {@code PacketHeader} describes a class that keeps protocol headers
 * for each protocol layer.
 */
public interface PacketHeader {
    /**
     * Return an {@link EtherHeader} instance which describes an Ethernet
     * header and IEEE 802.1Q VLAN tag.
     *
     * @return  An {@link EtherHeader} instance.
     */
    EtherHeader getEtherHeader();

    /**
     * Returns an {@link InetHeader} instance which describes an IP header.
     *
     * @return  An {@link InetHeader} instance if a packet associated with
     *          this instance is an IP packet. Otherwise {@code null}.
     */
    InetHeader getInetHeader();

    /**
     * Returns a {@link Layer4Header} instance which describes a layer 4
     * protocol header.
     *
     * @return  An {@link Layer4Header} instance if a packet associated with
     *          this instance contains a layer 4 protocol header.
     *          Otherwise {@code null}.
     */
    Layer4Header getLayer4Header();

    /**
     * Return a description about protocol headers.
     *
     * @return  A description about protocol headers.
     */
    String getHeaderDescription();
}
