/*
 * Copyright (c) 2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.northbound;

import javax.xml.bind.annotation.XmlAccessType;
import javax.xml.bind.annotation.XmlAccessorType;
import javax.xml.bind.annotation.XmlElement;
import javax.xml.bind.annotation.XmlRootElement;

import org.opendaylight.vtn.manager.PortMapConfig;
import org.opendaylight.vtn.manager.SwitchPort;

import org.opendaylight.controller.sal.core.NodeConnector;

/**
 * {@code PortMapInfo} class provides JAXB mapping for port mapping
 * information.
 */
@XmlRootElement(name = "portmap")
@XmlAccessorType(XmlAccessType.NONE)
public class PortMapInfo extends PortMapConfig {
    private static final long serialVersionUID =  7081865293009944033L;

    /**
     * Identifier of node connector associated with the switch port actually
     * mapped to the virtual bridge interface.
     */
    @XmlElement(name = "mapped", required = true)
    private SwitchPort  mapped;

    /**
     * Private constructor used for JAXB mapping.
     */
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
