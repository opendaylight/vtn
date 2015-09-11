/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.routing.xml;

import java.util.ArrayList;
import java.util.List;
import java.util.Objects;

import javax.xml.bind.annotation.XmlAccessType;
import javax.xml.bind.annotation.XmlAccessorType;
import javax.xml.bind.annotation.XmlElement;
import javax.xml.bind.annotation.XmlElementWrapper;
import javax.xml.bind.annotation.XmlRootElement;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.VtnPathPolicyConfig;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.vtn.path.policy.config.VtnPathCost;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.vtn.path.policy.config.VtnPathCostBuilder;

/**
 * {@code XmlPathPolicy} provides XML binding to data model for path policy
 * configuration.
 */
@XmlRootElement(name = "vtn-path-policy")
@XmlAccessorType(XmlAccessType.NONE)
public final class XmlPathPolicy implements VtnPathPolicyConfig {
    /**
     * The identifier of this path policy.
     */
    @XmlElement(required = true)
    private Integer  id;

    /**
     * The default cost for unspecified link.
     */
    @XmlElement(name = "default")
    private Long  defaultCost;

    /**
     * A list of path costs which specifies the cost of switch link for
     * transmitting.
     */
    @XmlElementWrapper(name = "vtn-path-costs")
    @XmlElement(name = "vtn-path-cost")
    private List<XmlPathCost>  pathCosts;

    /**
     * Private constructor only for JAXB.
     */
    private XmlPathPolicy() {
    }

    /**
     * Consturct a new instance from the given {@link VtnPathPolicyConfig}
     * instance.
     *
     * <p>
     *   Note that this constructor assumes that the given instance is valid.
     * </p>
     *
     * @param vppc  A {@link VtnPathPolicyConfig} instance.
     */
    public XmlPathPolicy(VtnPathPolicyConfig vppc) {
        id = vppc.getId();
        defaultCost = vppc.getDefaultCost();
        List<VtnPathCost> costs = vppc.getVtnPathCost();
        if (costs != null && !costs.isEmpty()) {
            List<XmlPathCost> xcosts = new ArrayList<>(costs.size());
            for (VtnPathCost vpc: costs) {
                xcosts.add(new XmlPathCost(vpc));
            }
            pathCosts = xcosts;
        }
    }

    // DataContainer

    /**
     * Return a class which indicates the data model type implemented by this
     * instance.
     *
     * @return  A class instance of {@link VtnPathPolicyConfig}.
     */
    @Override
    public Class<VtnPathPolicyConfig> getImplementedInterface() {
        return VtnPathPolicyConfig.class;
    }

    // VtnPathPolicyConfig

    /**
     * Return the identifier of this path policy.
     *
     * @return  An {@link Integer} instance which indicates the path policy
     *          identifier.
     */
    @Override
    public Integer getId() {
        return id;
    }

    /**
     * Reurn the default cost for unspecified link.
     *
     * @return  A {@link Long} instance which indicates the default cost.
     */
    @Override
    public Long getDefaultCost() {
        return defaultCost;
    }

    /**
     * Return a list of path costs configured in this path policy.
     *
     * @return  A list of {@link VtnPathCost} instances.
     */
    @Override
    public List<VtnPathCost> getVtnPathCost() {
        List<VtnPathCost> costs;
        if (pathCosts == null) {
            costs = null;
        } else {
            costs = new ArrayList<>(pathCosts.size());
            for (XmlPathCost xpc: pathCosts) {
                costs.add(new VtnPathCostBuilder(xpc).build());
            }
        }

        return costs;
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
            XmlPathPolicy xpp = (XmlPathPolicy)o;
            ret = (Objects.equals(id, xpp.id) &&
                   Objects.equals(defaultCost, xpp.defaultCost) &&
                   Objects.equals(pathCosts, xpp.pathCosts));
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
        return Objects.hash(getClass(), id, defaultCost, pathCosts);
    }

    /**
     * Return a string representation of this object.
     *
     * @return  A string representation of this object.
     */
    @Override
    public String toString() {
        return "vtn-path-policy[id=" + id + ", default=" + defaultCost +
            ", costs=" + pathCosts + "]";
    }
}
