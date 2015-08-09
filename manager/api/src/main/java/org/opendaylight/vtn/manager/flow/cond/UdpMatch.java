/*
 * Copyright (c) 2014, 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.flow.cond;

import javax.xml.bind.annotation.XmlAccessType;
import javax.xml.bind.annotation.XmlAccessorType;
import javax.xml.bind.annotation.XmlRootElement;

import com.fasterxml.jackson.databind.annotation.JsonSerialize;

/**
 * This class describes UDP header fields to match against packets.
 *
 * <h4>Example JSON</h4>
 * <pre class="prettyprint lang-json">
 * {
 * &nbsp;&nbsp;"src": {
 * &nbsp;&nbsp;&nbsp;&nbsp;"from": 0,
 * &nbsp;&nbsp;&nbsp;&nbsp;"to": 1024
 * &nbsp;&nbsp;},
 * &nbsp;&nbsp;"dst": {
 * &nbsp;&nbsp;&nbsp;&nbsp;"from": 63
 * &nbsp;&nbsp;}
 * }</pre>
 *
 * @since  Helium
 */
@JsonSerialize(include = JsonSerialize.Inclusion.NON_NULL)
@XmlRootElement(name = "udpmatch")
@XmlAccessorType(XmlAccessType.NONE)
public final class UdpMatch extends PortProtoMatch {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = -4039616048212833637L;

    /**
     * Private constructor only for JAXB.
     */
    @SuppressWarnings("unused")
    private UdpMatch() {
    }

    /**
     * Construct a new instance with specifying source and destination
     * port number.
     *
     * @param src  A {@link PortMatch} instance which describes the range of
     *             source port numbers to match against packets.
     *             {@code null} means that every source port number is matched.
     * @param dst  A {@link PortMatch} instance which describes the range of
     *             destination port numbers to match against packets.
     *             {@code null} means that every destination port number is
     *             matched.
     */
    public UdpMatch(PortMatch src, PortMatch dst) {
        super(src, dst);
    }

    /**
     * Construct a new instance with specifying source and destination
     * port number.
     *
     * @param src  A {@link Integer} instance which describes the source
     *             port number to match against packets.
     *             {@code null} means that every source port number is matched.
     * @param dst  A {@link Integer} instance which describes the destination
     *             port number to match against packets.
     *             {@code null} means that every destination port number is
     *             matched.
     */
    public UdpMatch(Integer src, Integer dst) {
        super(src, dst);
    }

    /**
     * Construct a new instance with specifying source and destination
     * port number.
     *
     * @param src  A {@link Short} instance which describes the source
     *             port number to match against packets.
     *             {@code null} means that every source port number is matched.
     * @param dst  A {@link Short} instance which describes the destination
     *             port number to match against packets.
     *             {@code null} means that every destination port number is
     *             matched.
     */
    public UdpMatch(Short src, Short dst) {
        super(src, dst);
    }
}
