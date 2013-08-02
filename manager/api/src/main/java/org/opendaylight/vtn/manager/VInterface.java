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
 * {@code VInterface} class describes information about an interface
 * attached to the virtual layer 2 bridge.
 */
@XmlRootElement(name = "interface")
@XmlAccessorType(XmlAccessType.NONE)
public class VInterface extends VInterfaceConfig {
    private final static long serialVersionUID = -4682666193022515484L;

    /**
     * The name of the interface.
     */
    @XmlAttribute(required = true)
    private String  name;

    /**
     * State of the interface.
     */
    private VNodeState  state;

    /**
     * State of the network element mapped to this interface.
     */
    private VNodeState  entityState;

    /**
     * Private constructor used for JAXB mapping.
     */
    private VInterface() {
        super(null, null);
        state = VNodeState.UNKNOWN;
        entityState = VNodeState.UNKNOWN;
    }

    /**
     * Construct a new interface instance.
     *
     * @param ifName  The name of the interface.
     * @param state   State of the interface.
     * @param estate  State of the network element mapped to this interface.
     * @param iconf   Virthal interface configuration.
     * @throws NullPointerException
     *    {@code iconf} is {@code null}.
     */
    public VInterface(String ifName, VNodeState state, VNodeState estate,
                      VInterfaceConfig iconf) {
        super(iconf.getDescription(), iconf.getEnabled());
        name = ifName;

        if (state == null) {
            state = VNodeState.UNKNOWN;
        }
        if (estate == null) {
            estate = VNodeState.UNKNOWN;
        }
        this.state = state;
        this.entityState = estate;
    }

    /**
     * Return the name of the interface.
     *
     * @return  The name of the interface.
     */
    public String getName() {
        return name;
    }

    /**
     * Return the state of the interface.
     *
     * @return  The state of the interface.
     */
    public VNodeState getState() {
        return state;
    }

    /**
     * Return the state of the network element mapped to this interface.
     *
     * @return  The state of the network element mapped to this interface.
     */
    public VNodeState getEntityState() {
        return entityState;
    }

    /**
     * Return a numerical representation of the interface state.
     *
     * <p>
     *   This method is called by JAXB.
     * </p>
     *
     * @return  A numerical representation of the interface state.
     */
    @XmlAttribute(name = "state")
    private int getStateValue() {
        return state.getValue();
    }

    /**
     * Set the state of the interface.
     *
     * <p>
     *   This method is called by JAXB.
     * </p>
     *
     * @param st  A numerical representation of the interface state.
     */
    private void setStateValue(int st) {
        this.state = VNodeState.valueOf(st);
    }

    /**
     * Return a numerical representation of the state of the network element
     * mapped to this interface.
     *
     * <p>
     *   This method is called by JAXB.
     * </p>
     *
     * @return  A numerical representation of the network element state.
     */
    @XmlAttribute(name = "entityState")
    private int getEntityStateValue() {
        return entityState.getValue();
    }

    /**
     * Set the state of the network element mapped to this interface.
     *
     * <p>
     *   This method is called by JAXB.
     * </p>
     *
     * @param st  A numerical representation of the network element state.
     */
    private void setEntityStateValue(int st) {
        this.entityState = VNodeState.valueOf(st);
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
        if (!(o instanceof VInterface) || !super.equals(o)) {
            return false;
        }

        VInterface viface = (VInterface)o;
        if (state != viface.state) {
            return false;
        }
        if (entityState != viface.entityState) {
            return false;
        }

        if (name == null) {
            return (viface.name == null);
        }

        return name.equals(viface.name);
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        int h = super.hashCode() + state.hashCode() + entityState.hashCode();
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
        StringBuilder builder = new StringBuilder("VInterface[");
        if (name != null) {
            builder.append("name=").append(name).append(',');
        }

        String desc = getDescription();
        if (desc != null) {
            builder.append("desc=").append(desc).append(',');
        }

        Boolean en = getEnabled();
        if (en != null) {
            if (en.booleanValue()) {
                builder.append("enabled,");
            } else {
                builder.append("disabled,");
            }
        }

        builder.append("state=").append(state.toString()).
            append(",entityState=").append(entityState.toString()).append(']');

        return builder.toString();
    }
}
