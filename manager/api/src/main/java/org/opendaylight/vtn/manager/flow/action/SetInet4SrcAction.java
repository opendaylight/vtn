/*
 * Copyright (c) 2014-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.flow.action;

import java.net.InetAddress;

import javax.xml.bind.annotation.XmlAccessType;
import javax.xml.bind.annotation.XmlAccessorType;
import javax.xml.bind.annotation.XmlRootElement;

import com.fasterxml.jackson.databind.annotation.JsonSerialize;

import org.opendaylight.vtn.manager.util.IpNetwork;

import org.opendaylight.controller.sal.action.SetNwSrc;

/**
 * This class describes a flow action that sets the specified IPv4 address
 * into the IPv4 packet as the source address.
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
@XmlRootElement(name = "setinet4src")
@XmlAccessorType(XmlAccessType.NONE)
public final class SetInet4SrcAction extends Inet4AddressAction {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = -5348967690083462053L;

    /**
     * Private constructor only for JAXB.
     */
    @SuppressWarnings("unused")
    private SetInet4SrcAction() {
    }

    /**
     * Construct a new instance.
     *
     * @param addr  An {@link InetAddress} instance which representse an
     *              IP address.
     * @throws IllegalArgumentException
     *    An invalid IP address is specified to {@code addr}.
     */
    public SetInet4SrcAction(InetAddress addr) {
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
    public SetInet4SrcAction(IpNetwork addr) {
        super(addr);
    }

    /**
     * Construct a new instance.
     *
     * @param act  A SAL action that sets the source IP address.
     * @throws NullPointerException
     *    {@code null} is passed to {@code act}.
     */
    public SetInet4SrcAction(SetNwSrc act) {
        super(act.getAddress());
    }
}
