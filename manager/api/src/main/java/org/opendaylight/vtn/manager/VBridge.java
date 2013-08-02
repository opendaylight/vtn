/*
 * Copyright (c) 2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager;

import javax.xml.bind.annotation.XmlAccessType;
import javax.xml.bind.annotation.XmlAccessorType;
import javax.xml.bind.annotation.XmlAttribute;
import javax.xml.bind.annotation.XmlRootElement;

/**
 * {@code VBridge} class describes information about the virtual layer 2
 * bridge.
 */
@XmlRootElement(name = "vbridge")
@XmlAccessorType(XmlAccessType.NONE)
public class VBridge extends VBridgeConfig {
    private final static long serialVersionUID = 3020097134642324374L;

    /**
     * The name of the bridge.
     */
    @XmlAttribute(required = true)
    private String  name;

    /**
     * State of the bridge.
     */
    private VNodeState  state;

    /**
     * The number of path faults in the bridge.
     */
    @XmlAttribute
    private int  faults;

    /**
     * Private constructor used for JAXB mapping.
     */
    private VBridge() {
        super(null);
        state = VNodeState.UNKNOWN;
    }

    /**
     * Construct a new virtual bridge instance.
     *
     * @param bridgeName  The name of the bridge.
     * @param state       State of the bridge.
     * @param faults      The number of path faults in the bridge.
     * @param bconf       VIrtual bridge configuation.
     * @throws NullPointerException
     *    {@code bconf} is {@code null}.
     */
    public VBridge(String bridgeName, VNodeState state, int faults,
                   VBridgeConfig bconf) {
        super(bconf.getDescription(), bconf.getAgeInterval());
        name = bridgeName;
        this.faults = faults;

        if (state == null) {
            state = VNodeState.UNKNOWN;
        }
        this.state = state;
    }

    /**
     * Return the name of the bridge.
     *
     * @return  The name of the bridge.
     */
    public String getName() {
        return name;
    }

    /**
     * Return the state of the bridge.
     *
     * @return  The state of the bridge.
     */
    public VNodeState getState() {
        return state;
    }

    /**
     * Return the number of path faults in the bridge.
     *
     * @return  The number of path faults in the bridge.
     */
    public int getFaults() {
        return faults;
    }

    /**
     * Return a numerical representation of the bridge state.
     *
     * <p>
     *   This method is called by JAXB.
     * </p>
     *
     * @return  A numerical representation of the bridge state.
     */
    @XmlAttribute(name = "state")
    private int getStateValue() {
        return state.getValue();
    }

    /**
     * Set the state of the bridge.
     *
     * <p>
     *   This method is called by JAXB.
     * </p>
     *
     * @param st  A numerical representation of the bridge state.
     */
    private void setStateValue(int st) {
        this.state = VNodeState.valueOf(st);
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
        if (!(o instanceof VBridge) || !super.equals(o)) {
            return false;
        }

        VBridge vbridge = (VBridge)o;
        if (state != vbridge.state) {
            return false;
        }
        if (faults != vbridge.faults) {
            return false;
        }

        if (name == null) {
            return (vbridge.name == null);
        }

        return name.equals(vbridge.name);
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        int h = super.hashCode() + state.hashCode() + faults;
        if (name != null) {
            h += name.hashCode();
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
        StringBuilder builder = new StringBuilder("VBridge[");
        if (name != null) {
            builder.append("name=").append(name).append(',');
        }

        String desc = getDescription();
        if (desc != null) {
            builder.append("desc=").append(desc).append(',');
        }

        int age = getAgeInterval();
        if (age >= 0) {
            builder.append("ageInterval=").append(age).append(',');
        }

        builder.append("faults=").append(faults).
            append(",state=").append(state.toString()).append("]");

        return builder.toString();
    }
}
