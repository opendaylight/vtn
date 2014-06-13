/*
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.flow.action;

import javax.xml.bind.annotation.XmlAccessType;
import javax.xml.bind.annotation.XmlAccessorType;
import javax.xml.bind.annotation.XmlRootElement;

import com.fasterxml.jackson.databind.annotation.JsonSerialize;

/**
 * This class describes a flow action that set the specified data layer
 * address into the packet as the source address.
 *
 * <h4>Example JSON</h4>
 * <pre class="prettyprint lang-json">
 * {
 * &nbsp;&nbsp;"address": "00:11:22:33:44:55"
 * }</pre>
 *
 * @since  Helium
 */
@JsonSerialize(include = JsonSerialize.Inclusion.NON_NULL)
@XmlRootElement(name = "setdlsrc")
@XmlAccessorType(XmlAccessType.NONE)
public final class SetDlSrcAction extends DlAddrAction {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = 5369755097245864497L;

    /**
     * Private constructor only for JAXB.
     */
    @SuppressWarnings("unused")
    private SetDlSrcAction() {
    }

    /**
     * Construct a new instance.
     *
     * @param addr  A byte array which represents a data layer address.
     */
    public SetDlSrcAction(byte[] addr) {
        super(addr);
    }
}
