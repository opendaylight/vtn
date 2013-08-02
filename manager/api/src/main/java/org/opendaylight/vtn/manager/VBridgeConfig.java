/*
 * Copyright (c) 2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager;

import java.io.Serializable;

import javax.xml.bind.annotation.XmlAccessType;
import javax.xml.bind.annotation.XmlAccessorType;
import javax.xml.bind.annotation.XmlAttribute;
import javax.xml.bind.annotation.XmlRootElement;

/**
 * {@code VBridgeConfig} class describes configuration for the virtual layer 2
 * bridge.
 */
@XmlRootElement(name = "vbridgeconf")
@XmlAccessorType(XmlAccessType.NONE)
public class VBridgeConfig implements Serializable {
    private final static long serialVersionUID = -4582273744010592577L;

    /**
     * An arbitrary description about the bridge.
     */
    @XmlAttribute
    private String  description;

    /**
     * The number of seconds between MAC address table aging.
     */
    private int  ageInterval;

    /**
     * Private constructor used for JAXB mapping.
     */
    private VBridgeConfig() {
        ageInterval = -1;
    }

    /**
     * Construct a new virtual bridge configuration.
     *
     * <p>
     *   The interval of MAC address table aging is determined by the system.
     * </p>
     *
     * @param desc      Description about the bridge.
     */
    public VBridgeConfig(String desc) {
        this(desc, -1);
    }

    /**
     * Construct a new virtual bridge configuration.
     *
     * @param desc   Description about the bridge.
     * @param age    The number of seconds between MAC address table aging.
     *               The interval of MAC address aging is determined by the
     *               system if a negative value is passed.
     */
    public VBridgeConfig(String desc, int age) {
        description = desc;
        this.ageInterval = (age < 0) ? -1 : age;
    }

    /**
     * Return description about the bridge.
     *
     * @return  Description about the bridge.
     *          {@code null} is returned if description is not set.
     */
    public String getDescription() {
        return description;
    }

    /**
     * Return the number of seconds between MAC address table aging.
     *
     * @return  The number of seconds between MAC address table aging.
     *          -1 is returned if this object does not keep the value.
     */
    public int getAgeInterval() {
        return ageInterval;
    }

    /**
     * Return an {@code Integer} object which represents the number of seconds
     * between MAC address table aging.
     *
     * @return  An {@code Integer} object which represents the number of
     *          seconds between MAC address table aging.
     *          {@code null} is returned if this object does not keep the
     *          value.
     */
    @XmlAttribute(name = "ageInterval")
    public Integer getAgeIntervalValue() {
        return (ageInterval >= 0) ? new Integer(ageInterval) : null;
    }

    /**
     * Set the number of seconds between MAC address table aging.
     *
     * <p>
     *   This method is called by JAXB.
     * </p>
     *
     * @param age  An {@code Integer} object which represents the number of
     *             seconds between MAC address table aging.
     */
    private void setAgeIntervalValue(Integer age) {
        if (age == null || age.intValue() < 0) {
            ageInterval = -1;
        } else {
            ageInterval = age.intValue();
        }
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
        if (!(o instanceof VBridgeConfig)) {
            return false;
        }

        VBridgeConfig bconf = (VBridgeConfig)o;
        if (ageInterval != bconf.ageInterval) {
            return false;
        }

        if (description == null) {
            return (bconf.description == null);
        }

        return description.equals(bconf.description);
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        int h = 229 + ageInterval;
        if (description != null) {
            h += description.hashCode();
        }

        return h;
    }

    /**
     * Return a string representation of this object.
     *
     * @return  A string representation of this object.
     */
    @Override
    public String toString() {
        StringBuilder builder = new StringBuilder("VBridgeConfig[");
        if (description != null) {
            builder.append("desc=").append(description);
        }
        if (ageInterval >= 0) {
            if (description != null) {
                builder.append(',');
            }
            builder.append("ageInterval=").append(ageInterval);
        }
        builder.append(']');

        return builder.toString();
    }
}
