/*
 * Copyright (c) 2013, 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.northbound;

import javax.xml.bind.annotation.XmlAccessType;
import javax.xml.bind.annotation.XmlAccessorType;
import javax.xml.bind.annotation.XmlElement;
import javax.xml.bind.annotation.XmlRootElement;

import com.fasterxml.jackson.databind.annotation.JsonSerialize;

import org.opendaylight.vtn.manager.PortMapConfig;
import org.opendaylight.vtn.manager.SwitchPort;

import org.opendaylight.controller.sal.core.NodeConnector;

/**
 * {@code PortMapInfo} class describes information about the port mapping
 * configured in the vBridge interface.
 *
 * <p>
 *   This class is used to return information about the port mapping to
 *   REST client.
 * </p>
 *
 * <h4>Example JSON</h4>
 * <pre class="prettyprint lang-json">
 * {
 * &nbsp;&nbsp;"vlan": "100",
 * &nbsp;&nbsp;"node": {
 * &nbsp;&nbsp;&nbsp;&nbsp;"type": "OF",
 * &nbsp;&nbsp;&nbsp;&nbsp;"id": "00:00:00:00:00:00:00:03"
 * &nbsp;&nbsp;},
 * &nbsp;&nbsp;"port": {
 * &nbsp;&nbsp;&nbsp;&nbsp;"name": "port-1",
 * &nbsp;&nbsp;&nbsp;&nbsp;"type": "OF",
 * &nbsp;&nbsp;&nbsp;&nbsp;"id": "1"
 * &nbsp;&nbsp;},
 * &nbsp;&nbsp;"mapped": {
 * &nbsp;&nbsp;&nbsp;&nbsp;"type": "OF",
 * &nbsp;&nbsp;&nbsp;&nbsp;"id": "1"
 * &nbsp;&nbsp;}
 * }</pre>
 */
@JsonSerialize(include = JsonSerialize.Inclusion.NON_NULL)
@XmlRootElement(name = "portmap")
@XmlAccessorType(XmlAccessType.NONE)
public class PortMapInfo extends PortMapConfig {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID =  7081865293009944033L;

    /**
     * {@link NodeConnector} information corresponding to the physical switch
     * port actually mapped to the vBridge interface.
     *
     * <ul>
     *   <li>
     *     This element is omitted if no physical switch port meets the
     *     condition specified by the port mapping configuration information.
     *   </li>
     * </ul>
     */
    @XmlElement(name = "mapped")
    private SwitchPort  mapped;

    /**
     * Private constructor used for JAXB mapping.
     */
    @SuppressWarnings("unused")
    private PortMapInfo() {
        super(null, null, (short)0);
    }

    /**
     * Construct a new port mapping information.
     *
     * @param pmconf  Port mapping configuration.
     * @param nc      Node connector actually mapped.
     * @throws NullPointerException
     *    {@code pmconf} is {@code null}.
     */
    public PortMapInfo(PortMapConfig pmconf, NodeConnector nc) {
        super(pmconf.getNode(), pmconf.getPort(), pmconf.getVlan());

        if (nc == null) {
            mapped = null;
        } else {
            String type = nc.getType();
            String id = nc.getNodeConnectorIDString();
            mapped = new SwitchPort(type, id);
        }
    }

    /**
     * Return {@code SwitchPort} object which represents the physical port
     * mapped by the port mapping.
     *
     * @return  A {@code SwitchPort} object if the physical port is actually
     *          mapped. {@code null} is returned if not mapped.
     */
    SwitchPort getMappedPort() {
        return mapped;
    }

    /**
     * Determine whether the given object is identical to this object.
     *
     * @param o  An object to be compared.
     * @return   {@code true} if identical. Otherwise {@code false}.
     */
    @Override
    public boolean equals(Object o) {
        if (o == this) {
            return true;
        }
        if (!(o instanceof PortMapInfo) || !super.equals(o)) {
            return false;
        }

        PortMapInfo pi = (PortMapInfo)o;
        if (mapped == null) {
            return (pi.mapped == null);
        }

        return mapped.equals(pi.mapped);
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        int h = super.hashCode();
        if (mapped != null) {
            h ^= mapped.hashCode();
        }

        return h;
    }
}
