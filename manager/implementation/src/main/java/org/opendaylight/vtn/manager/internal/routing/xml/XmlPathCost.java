/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.routing.xml;

import java.util.Objects;

import javax.xml.bind.annotation.XmlAccessType;
import javax.xml.bind.annotation.XmlAccessorType;
import javax.xml.bind.annotation.XmlElement;
import javax.xml.bind.annotation.XmlRootElement;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.VtnPathCostConfig;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnPortDesc;

/**
 * {@code XmlPathCost} provides XML binding to data model for the cost of using
 * specific inter-switch link.
 */
@XmlRootElement(name = "vtn-path-cost")
@XmlAccessorType(XmlAccessType.NONE)
public final class XmlPathCost implements VtnPathCostConfig {
    /**
     * A string which specifies the location of the physical switch port
     * linked to another physical switch.
     */
    @XmlElement(name = "port-desc", required = true)
    private VtnPortDesc  portDesc;

    /**
     * The cost of using physical switch port.
     */
    @XmlElement
    private Long  cost;

    /**
     * Private constructor only for JAXB.
     */
    private XmlPathCost() {
    }

    /**
     * Construct a new instance from the given {@link VtnPathCostConfig}
     * instance.
     *
     * <p>
     *   Note that this constructor assumes that the given instance is valid.
     * </p>
     *
     * @param vpcc  A {@link VtnPathCostConfig} instance.
     */
    public XmlPathCost(VtnPathCostConfig vpcc) {
        portDesc = vpcc.getPortDesc();
        cost = vpcc.getCost();
    }

    // DataContainer

    /**
     * Return a class which indicates the data model type implemented by this
     * instance.
     *
     * @return  A class instance of {@link VtnPathCostConfig}.
     */
    @Override
    public Class<VtnPathCostConfig> getImplementedInterface() {
        return VtnPathCostConfig.class;
    }

    // VtnPathCostConfig

    /**
     * Return the location of the physical switch port.
     *
     * @return  A {@link VtnPortDesc} instance which indicates the location of
     *          the physical switch port.
     */
    @Override
    public VtnPortDesc getPortDesc() {
        return portDesc;
    }

    /**
     * Return the cost of using physical switch port.
     *
     * @return  A {@link Long} instance which represents the cost of using
     *          physical switch port.
     */
    @Override
    public Long getCost() {
        return cost;
    }

    // Object

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

        boolean ret;
        if (o != null && getClass().equals(o.getClass())) {
            XmlPathCost xpc = (XmlPathCost)o;
            ret = (Objects.equals(portDesc, xpc.portDesc) &&
                   Objects.equals(cost, xpc.cost));
        } else {
            ret = false;
        }

        return ret;
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        return Objects.hash(getClass(), portDesc, cost);
    }

    /**
     * Return a string representation of this object.
     *
     * @return  A string representation of this object.
     */
    @Override
    public String toString() {
        String desc = (portDesc == null) ? null : portDesc.getValue();
        return "vtn-path-cost[port=" + desc + ", cost=" + cost + "]";
    }
}
