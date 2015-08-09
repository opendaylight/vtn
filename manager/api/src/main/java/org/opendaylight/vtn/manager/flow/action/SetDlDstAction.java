/*
 * Copyright (c) 2014, 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.flow.action;

import javax.xml.bind.annotation.XmlAccessType;
import javax.xml.bind.annotation.XmlAccessorType;
import javax.xml.bind.annotation.XmlRootElement;

import com.fasterxml.jackson.databind.annotation.JsonSerialize;

import org.opendaylight.vtn.manager.util.EtherAddress;

/**
 * This class describes a flow action that sets the specified data layer
 * address into the packet as the destination address.
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
@XmlRootElement(name = "setdldst")
@XmlAccessorType(XmlAccessType.NONE)
public final class SetDlDstAction extends DlAddrAction {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = 2185430970030623786L;

    /**
     * Private constructor only for JAXB.
     */
    @SuppressWarnings("unused")
    private SetDlDstAction() {
    }

    /**
     * Construct a new instance.
     *
     * @param addr  A byte array which represents a data layer address.
     */
    public SetDlDstAction(byte[] addr) {
        super(addr);
    }

    /**
     * Construct a new instance.
     *
     * @param addr  An {@link EtherAddress} instance.
     * @since  Lithium
     */
    public SetDlDstAction(EtherAddress addr) {
        super(addr);
    }
}
