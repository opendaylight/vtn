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

import org.opendaylight.vtn.manager.internal.util.flow.FlowUtils;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathmap.rev150328.VtnPathMapConfig;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeName;

/**
 * {@code XmlPathMap} provides XML binding to data model for path map
 * configuration.
 */
@XmlRootElement(name = "vtn-path-map")
@XmlAccessorType(XmlAccessType.NONE)
public final class XmlPathMap implements VtnPathMapConfig {
    /**
     * An index value assigned to this path map.
     */
    @XmlElement(required = true)
    private Integer  index;

    /**
     * The name of the flow condition that selects packets.
     */
    @XmlElement(required = true)
    private VnodeName  condition;

    /**
     * The identifier of the path policy which determines the route of packets.
     */
    @XmlElement
    private Integer  policy;

    /**
     * The number of seconds to be configured in {@code idle_timeout} field
     * in flow entries.
     */
    @XmlElement(name = "idle-timeout")
    private Integer  idleTimeout;

    /**
     * The number of seconds to be configured in {@code hard_timeout} field
     * in flow entries.
     */
    @XmlElement(name = "hard-timeout")
    private Integer  hardTimeout;

    /**
     * Private constructor only for JAXB.
     */
    private XmlPathMap() {
    }

    /**
     * Construct a new instance from the given {@link VtnPathMapConfig}
     * instance.
     *
     * <p>
     *   Note that this constructor assumes that the given instance is valid.
     * </p>
     *
     * @param vpmc  A {@link VtnPathMapConfig} instance.
     */
    public XmlPathMap(VtnPathMapConfig vpmc) {
        index = vpmc.getIndex();
        condition = vpmc.getCondition();
        policy = vpmc.getPolicy();
        idleTimeout = vpmc.getIdleTimeout();
        hardTimeout = vpmc.getHardTimeout();
    }

    // DataContainer

    /**
     * Return a class which indicates the data model type implemented by this
     * instance.
     *
     * @return  A class instance of {@link VtnPathMapConfig}.
     */
    @Override
    public Class<VtnPathMapConfig> getImplementedInterface() {
        return VtnPathMapConfig.class;
    }

    // VtnIndex

    /**
     * Return an index number assigned to this path map.
     *
     * @return  An {@link Integer} instance which indicates the index number.
     */
    @Override
    public Integer getIndex() {
        return index;
    }

    // VtnFlowTimeoutConfig

    /**
     * Return the number of seconds to be set to {@code idle-timeout} field in
     * flow entries.
     *
     * @return  An {@link Integer} instance which indicates the value for
     *          {@code idle-timeout}.
     */
    @Override
    public Integer getIdleTimeout() {
        return idleTimeout;
    }

    /**
     * Return the number of seconds to be set to {@code hard-timeout} field in
     * flow entries.
     *
     * @return  An {@link Integer} instance which indicates the value for
     *          {@code hard-timeout}.
     */
    @Override
    public Integer getHardTimeout() {
        return hardTimeout;
    }

    // VtnPathMapConfig

    /**
     * Return the name of the flow condition that selects packets.
     *
     * @return  A {@link VnodeName} instance which contains the flow condition
     *          name.
     */
    @Override
    public VnodeName getCondition() {
        return condition;
    }

    /**
     * Return the identifier of the path policy which determines the route of
     * packets.
     *
     * @return  An {@link Integer} instance which indicates the path policy
     *          identifier.
     */
    @Override
    public Integer getPolicy() {
        return policy;
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
            XmlPathMap xpm = (XmlPathMap)o;
            ret = (Objects.equals(index, xpm.index) &&
                   Objects.equals(condition, xpm.condition) &&
                   Objects.equals(policy, xpm.policy) &&
                   FlowUtils.equalsFlowTimeoutConfig(this, xpm));
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
        return Objects.hash(getClass(), index, condition, policy, idleTimeout,
                            hardTimeout);
    }

    /**
     * Return a string representation of this object.
     *
     * @return  A string representation of this object.
     */
    @Override
    public String toString() {
        String cond = (condition == null) ? null : condition.getValue();
        return "vtn-path-map[index=" + index + ", condition=" + cond +
            ", policy=" + policy + ", idle-timeout=" + idleTimeout +
            ", hard-timeout=" + hardTimeout + "]";
    }
}
