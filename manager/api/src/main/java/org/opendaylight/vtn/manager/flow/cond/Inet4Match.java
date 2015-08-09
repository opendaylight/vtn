/*
 * Copyright (c) 2014, 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.flow.cond;

import java.net.InetAddress;
import java.net.Inet4Address;

import javax.xml.bind.annotation.XmlAccessType;
import javax.xml.bind.annotation.XmlAccessorType;
import javax.xml.bind.annotation.XmlRootElement;

import com.fasterxml.jackson.databind.annotation.JsonSerialize;

/**
 * This class describes IPv4 header fields to match against packets.
 *
 * <h4>Example JSON</h4>
 * <pre class="prettyprint lang-json">
 * {
 * &nbsp;&nbsp;"src": "192.168.10.2",
 * &nbsp;&nbsp;"srcsuffix": 31,
 * &nbsp;&nbsp;"dst": "192.168.10.0",
 * &nbsp;&nbsp;"dstsuffix": 24,
 * &nbsp;&nbsp;"protocol": 6,
 * &nbsp;&nbsp;"dscp": 1
 * }</pre>
 *
 * @since  Helium
 */
@JsonSerialize(include = JsonSerialize.Inclusion.NON_NULL)
@XmlRootElement(name = "inet4match")
@XmlAccessorType(XmlAccessType.NONE)
public final class Inet4Match extends InetMatch {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = -1574031353140201888L;

    /**
     * Private constructor only for JAXB.
     */
    @SuppressWarnings("unused")
    private Inet4Match() {
    }

    /**
     * Construct a new instance which describes the condition to match IPv4
     * header.
     *
     * @param src
     *   An {@link InetAddress} instance which represents the source
     *   IP address to match.
     *   {@code null} means that every source IP address should be mathched.
     * @param srcsuff
     *   A CIDR suffix for the source IP address.
     *   <ul>
     *     <li>
     *       This value is not used unless a non-{@code null} value is
     *       specified to {@code src}.
     *     </li>
     *     <li>
     *       If a valid CIDR suffix is specified, only bits in the source IP
     *       address specified by the CIDR suffix are used for matching.
     *       <ul>
     *         <li>
     *           The valid range of the CIDR suffix for IPv4 address is
     *           from <strong>1</strong> to <strong>31</strong>.
     *         </li>
     *       </ul>
     *     </li>
     *     <li>
     *       {@code null} means that all bits in the source IP address should
     *       be used for matching.
     *     </li>
     *   </ul>
     * @param dst
     *   An {@link InetAddress} instance which represents the destination
     *   IP address to match.
     *   {@code null} means that every destination IP address should be
     *   mathched.
     * @param dstsuff
     *   A CIDR suffix for the destination IP address.
     *   <ul>
     *     <li>
     *       This value is not used unless a non-{@code null} value is
     *       specified to {@code src}.
     *     </li>
     *     <li>
     *       If a valid CIDR suffix is specified, only bits in the destination
     *       IP address specified by the CIDR suffix are used for matching.
     *       <ul>
     *         <li>
     *           The valid range of the CIDR suffix for IPv4 address is
     *           from <strong>1</strong> to <strong>31</strong>.
     *         </li>
     *       </ul>
     *     </li>
     *     <li>
     *       {@code null} means that all bits in the destination IP address
     *       should be used for matching.
     *     </li>
     *   </ul>
     * @param proto
     *   An IP protocol type value of match against packets.
     *   <ul>
     *     <li>
     *       The range of value that can be specified is from
     *       <strong>0</strong> to <strong>255</strong>.
     *     </li>
     *     <li>
     *       {@code null} means that every IP protocol type should be matched.
     *     </li>
     *   </ul>
     * @param dscp
     *   A DSCP field value of match against packets.
     *   <ul>
     *     <li>
     *       The range of value that can be specified is from
     *       <strong>0</strong> to <strong>63</strong>.
     *     </li>
     *     <li>
     *       {@code null} means that every DSCP field value should be matched.
     *     </li>
     *   </ul>
     * @throws IllegalArgumentException
     *    Invalid IP address is specified to either {@code src} or {@code dst}.
     */
    public Inet4Match(InetAddress src, Short srcsuff, InetAddress dst,
                      Short dstsuff, Short proto, Byte dscp) {
        super(src, srcsuff, dst, dstsuff, proto, dscp);
    }

    /**
     * Return a {@link Class} instance which represents the type of
     * IP address.
     *
     * @return  A {@link Class} instance for {@link Inet4Address}.
     */
    @Override
    public Class<? extends InetAddress> getAddressClass() {
        return Inet4Address.class;
    }
}
