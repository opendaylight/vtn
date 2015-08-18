/*
 * Copyright (c) 2014, 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * This file is used only for REST API documentation.
 */

package org.opendaylight.controller.sal.core;

import javax.xml.bind.annotation.XmlAccessType;
import javax.xml.bind.annotation.XmlAccessorType;
import javax.xml.bind.annotation.XmlElement;
import javax.xml.bind.annotation.XmlRootElement;

/**
 * {@code Node} class describes a generic network element, such as a SDN
 * switch, managed by the OpenDaylight controller.
 *
 * <p>
 *   A node is identified by a pair of node type and node ID.
 * </p>
 *
 * <h4>Example JSON</h4>
 * <pre class="prettyprint lang-json">
 * {
 * &nbsp;&nbsp;"type": "OF",
 * &nbsp;&nbsp;"id": "00:00:00:00:00:00:00:01"
 * }</pre>
 */
@XmlAccessorType(XmlAccessType.NONE)
@XmlRootElement
public class Node {
    /**
     * A string which represents the type of the node.
     *
     * <ul>
     *   <li>Specify <strong>"OF"</strong> for OpenFlow switch.</li>
     * </ul>
     */
    @XmlElement(required = true)
    private String  type;

    /**
     * A string which represents the identifier of the node.
     *
     * <ul>
     *   <li>Specify a string representation of DPID for OpenFlow switch.</li>
     * </ul>
     */
    @XmlElement(required = true)
    private String  id;
}
