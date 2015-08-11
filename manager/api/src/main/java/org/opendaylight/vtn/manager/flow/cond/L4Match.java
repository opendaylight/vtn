/*
 * Copyright (c) 2014, 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.flow.cond;

import java.io.Serializable;

import javax.xml.bind.annotation.XmlAccessType;
import javax.xml.bind.annotation.XmlAccessorType;
import javax.xml.bind.annotation.XmlRootElement;
import javax.xml.bind.annotation.XmlSeeAlso;

import com.fasterxml.jackson.databind.annotation.JsonSerialize;

/**
 * This class describes layer 4 protocol header fields to match against
 * packets.
 * Typically, an instance of this class represents header fields for protocols
 * on internet protocol.
 *
 * @since  Helium
 */
@JsonSerialize(include = JsonSerialize.Inclusion.NON_NULL)
@XmlRootElement(name = "l4match")
@XmlAccessorType(XmlAccessType.NONE)
@XmlSeeAlso({TcpMatch.class, UdpMatch.class, IcmpMatch.class})
public abstract class L4Match implements Serializable {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = 5816920933128604720L;

    /**
     * Construct a new instance which describes every layer 4 protocol header.
     */
    L4Match() {
    }
}
