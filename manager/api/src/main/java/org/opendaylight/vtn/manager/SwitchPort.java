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
 * {@code SwitchPort} class describes condition to identify a node connector
 * in a network element.
 */
@XmlRootElement(name = "switchport")
@XmlAccessorType(XmlAccessType.NONE)
public class SwitchPort implements Serializable {
    private static final long serialVersionUID = -2599650909042057330L;

    /**
     * The name of the switch port.
     */
    @XmlAttribute
    private String  name;

    /**
     * Type of the node connector.
     */
    @XmlAttribute
    private String  type;

    /**
     * A string representation of the node connector ID.
     */
    @XmlAttribute
    private String  id;

    /**
     * Private constructor used for JAXB mapping.
     */
    private SwitchPort() {
    }

    /**
     * Construct a new {@code SwitchPort} instance.
     *
     * <p>
     *   This constructor creates an instance to identify the node connector
     *   by the name of the switch port.
     * </p>
     *
     * @param name  The name of the switch port.
     */
    public SwitchPort(String name) {
        this.name = name;
    }

    /**
     * Construct a new {@code SwitchPort} instance.
     *
     * <p>
     *   This constructor creates an instance to identify the node connector
     *   by its type and ID.
     * </p>
     *
     * @param type  The type of the node connector.
     * @param id    Identifier of the node connector.
     */
    public SwitchPort(String type, String id) {
        this.type = type;
        this.id = id;
    }

    /**
     * Construct a new {@code SwitchPort} instance.
     *
     * <p>
     *   This constructor creates an instance to identify the node connector
     *   by the name of the switch port and a pair of its type and ID.
     * </p>
     *
     * @param name  The name of the switch port.
     * @param type  The type of the node connector.
     * @param id    Identifier of the node connector.
     */
    public SwitchPort(String name, String type, String id) {
        this.name = name;
        this.type = type;
        this.id = id;
    }

    /**
     * Return the name of the switch port.
     *
     * @return  The name of the switch port.
     */
    public String getName() {
        return name;
    }

    /**
     * Return the type of the node connector.
     *
     * @return  The type of the node connector.
     */
    public String getType() {
        return type;
    }

    /**
     * Return a string representation of the node connector ID.
     *
     * @return  The type of the node connector.
     */
    public String getId() {
        return id;
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
        if (!(o instanceof SwitchPort)) {
            return false;
        }

        SwitchPort swport = (SwitchPort)o;
        if (name == null) {
            if (swport.name != null) {
                return false;
            }
        } else if (!name.equals(swport.name)) {
            return false;
        }

        if (type == null) {
            if (swport.type != null) {
                return false;
            }
        } else if (!type.equals(swport.type)) {
            return false;
        }

        if (id == null) {
            return (swport.id == null);
        }

        return id.equals(swport.id);
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        int h = 0;
        if (name != null) {
            h ^= name.hashCode();
        }
        if (type != null) {
            h ^= type.hashCode();
        }
        if (id != null) {
            h ^= id.hashCode();
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
        StringBuilder builder = new StringBuilder("SwitchPort[");
        char sep = 0;
        if (name != null) {
            builder.append("name=").append(name);
            sep = ',';
        }

        if (type != null) {
            if (sep != 0) {
                builder.append(sep);
            }
            builder.append("type=").append(type);
            sep = ',';
        }

        if (id != null) {
            if (sep != 0) {
                builder.append(sep);
            }
            builder.append("id=").append(id);
        }

        builder.append(']');

        return builder.toString();
    }
}
