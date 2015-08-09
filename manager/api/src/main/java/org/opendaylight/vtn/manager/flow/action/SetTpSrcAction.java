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

/**
 * This class describes a flow action that sets the source port number into the
 * transport layer header in the packet.
 *
 * <h4>Example JSON</h4>
 * <pre class="prettyprint lang-json">
 * {
 * &nbsp;&nbsp;"port": 10
 * }</pre>
 *
 * @since  Helium
 */
@JsonSerialize(include = JsonSerialize.Inclusion.NON_NULL)
@XmlRootElement(name = "settpsrc")
@XmlAccessorType(XmlAccessType.NONE)
public final class SetTpSrcAction extends TpPortAction {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = -2880139916063275691L;

    /**
     * Private constructor only for JAXB.
     */
    @SuppressWarnings("unused")
    private SetTpSrcAction() {
    }

    /**
     * Construct a new instance.
     *
     * @param port  A source port number for the transport layer protocol.
     */
    public SetTpSrcAction(int port) {
        super(port);
    }
}
