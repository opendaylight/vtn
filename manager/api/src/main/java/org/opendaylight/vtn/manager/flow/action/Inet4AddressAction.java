/*
 * Copyright (c) 2014, 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.flow.action;

import java.net.InetAddress;
import java.net.Inet4Address;

import javax.xml.bind.annotation.XmlAccessType;
import javax.xml.bind.annotation.XmlAccessorType;
import javax.xml.bind.annotation.XmlRootElement;

import com.fasterxml.jackson.databind.annotation.JsonSerialize;

import org.opendaylight.vtn.manager.util.Ip4Network;
import org.opendaylight.vtn.manager.util.IpNetwork;

/**
 * This class describes a flow action that takes an IPv4 address as argument.
 *
 * <h4>Example JSON</h4>
 * <pre class="prettyprint lang-json">
 * {
 * &nbsp;&nbsp;"address": "192.168.10.1"
 * }</pre>
 *
 * @since  Helium
 */
@JsonSerialize(include = JsonSerialize.Inclusion.NON_NULL)
@XmlRootElement(name = "setinet4addr")
@XmlAccessorType(XmlAccessType.NONE)
public abstract class Inet4AddressAction extends InetAddressAction {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = -2967072189478447959L;

    /**
     * Dummy constructor only for JAXB and sub classes.
     */
    Inet4AddressAction() {
    }

    /**
     * Construct a new instance.
     *
     * @param addr  An {@link InetAddress} instance which representse an
     *              IP address.
     * @throws IllegalArgumentException
     *    An invalid IP address is specified to {@code addr}.
     */
    Inet4AddressAction(InetAddress addr) {
        super(addr);
    }

    /**
     * Construct a new instance.
     *
     * @param addr  An {@link IpNetwork} instance which representse an
     *              IP address.
     * @throws IllegalArgumentException
     *    An invalid IP address is specified to {@code addr}.
     * @since  Lithium
     */
    Inet4AddressAction(IpNetwork addr) {
        super(addr);
    }

    /**
     * Return a {@link Class} instance which represents the type of
     * IP address.
     *
     * @return  A {@link Class} instance which represents the type of
     *          IPv4 address.
     */
    public final Class<Inet4Address> getAddressClass() {
        return Inet4Address.class;
    }

    /**
     * Return a {@link Class} instance which represents the type of
     * IP network.
     *
     * @return  A {@link Class} instance which represents the type of
     *          IPv4 network.
     * @since   Lithium
     */
    public final Class<Ip4Network> getNetworkClass() {
        return Ip4Network.class;
    }
}
