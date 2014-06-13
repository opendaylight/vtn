/*
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.flow.action;

import java.net.InetAddress;
import java.net.Inet4Address;

import javax.xml.bind.annotation.XmlAccessType;
import javax.xml.bind.annotation.XmlAccessorType;
import javax.xml.bind.annotation.XmlRootElement;

import com.fasterxml.jackson.databind.annotation.JsonSerialize;

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
    private static final long serialVersionUID = 7533646485946661001L;

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
     * Return a {@link Class} instance which represents the type of
     * IP address.
     *
     * @return  A {@link Class} instance which represents the type of
     *          IP address.
     */
    public final Class<? extends InetAddress> getAddressClass() {
        return Inet4Address.class;
    }
}
