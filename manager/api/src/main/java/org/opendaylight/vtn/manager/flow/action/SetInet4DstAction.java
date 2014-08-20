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

import javax.xml.bind.annotation.XmlAccessType;
import javax.xml.bind.annotation.XmlAccessorType;
import javax.xml.bind.annotation.XmlRootElement;

import com.fasterxml.jackson.databind.annotation.JsonSerialize;

import org.opendaylight.controller.sal.action.SetNwDst;

/**
 * This class describes a flow action that sets the specified IPv4 address
 * into the IPv4 packet as the destination address.
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
@XmlRootElement(name = "setinet4dst")
@XmlAccessorType(XmlAccessType.NONE)
public final class SetInet4DstAction extends Inet4AddressAction {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = -6356309794337002311L;

    /**
     * Private constructor only for JAXB.
     */
    @SuppressWarnings("unused")
    private SetInet4DstAction() {
    }

    /**
     * Construct a new instance.
     *
     * @param addr  An {@link InetAddress} instance which representse an
     *              IP address.
     * @throws IllegalArgumentException
     *    An invalid IP address is specified to {@code addr}.
     */
    public SetInet4DstAction(InetAddress addr) {
        super(addr);
    }

    /**
     * Construct a new instance.
     *
     * @param act  A SAL action that sets the destination IP address.
     * @throws NullPointerException
     *    {@code null} is passed to {@code act}.
     */
    public SetInet4DstAction(SetNwDst act) {
        super(act.getAddress());
    }
}
