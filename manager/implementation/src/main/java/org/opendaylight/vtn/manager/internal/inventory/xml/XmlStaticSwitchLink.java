/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.inventory.xml;

import java.util.Objects;

import javax.xml.bind.annotation.XmlAccessType;
import javax.xml.bind.annotation.XmlAccessorType;
import javax.xml.bind.annotation.XmlElement;
import javax.xml.bind.annotation.XmlRootElement;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.topology._static.rev150801.vtn._static.topology._static._switch.links.StaticSwitchLink;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.topology._static.rev150801.vtn._static.topology._static._switch.links.StaticSwitchLinkBuilder;

import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.NodeConnectorId;

/**
 * {@code XmlStaticSwitchLink} provides XML binding to static inter-switch
 * link configuration.
 */
@XmlRootElement(name = "static-switch-link")
@XmlAccessorType(XmlAccessType.NONE)
public final class XmlStaticSwitchLink {
    /**
     * The source switch port of the link.
     */
    @XmlElement(required = true)
    private String  source;

    /**
     * The destination switch port of the link.
     */
    @XmlElement(required = true)
    private String  destination;

    /**
     * Determine whether the given port ID is valid or not.
     *
     * @param portId  The port ID to be tested.
     * @return  {@code true} only if the given port ID is valid.
     */
    static boolean isValidPortId(String portId) {
        return (portId != null && !portId.isEmpty());
    }

    /**
     * Constructor only for JAXB.
     */
    @SuppressWarnings("unused")
    private XmlStaticSwitchLink() {
    }

    /**
     * Construct a new instance.
     *
     * @param swlink  A {@link StaticSwitchLink} instance.
     * @throws IllegalArgumentException
     *    The given instance is invalid.
     */
    public XmlStaticSwitchLink(StaticSwitchLink swlink) {
        source = getPortId(swlink.getSource(), "Source");
        destination = getPortId(swlink.getDestination(), "Destination");
        if (source.equals(destination)) {
            throw new IllegalArgumentException(
                "Destination must not be the same as source.");
        }
    }

    /**
     * Return the source switch port identifier of the link.
     *
     * @return  The source switch port identifier.
     */
    public String getSource() {
        return source;
    }

    /**
     * Return the destination switch port identifier of the link.
     *
     * @return  The destination switch port identifier.
     */
    public String getDestination() {
        return destination;
    }

    /**
     * Determine whether this instance is valid or not.
     *
     * @return  {@code true} only if this instance is valid.
     */
    public boolean isValid() {
        return (isValidPortId(source) && isValidPortId(destination) &&
                !source.equals(destination));
    }

    /**
     * Convert this instance into a {@link StaticSwitchLink} instance.
     *
     * @return  A {@link StaticSwitchLink} instance.
     * @throws IllegalArgumentException
     *    This instance contains invalid configuration.
     */
    public StaticSwitchLink toStaticSwitchLink() {
        StaticSwitchLink swlink = new StaticSwitchLinkBuilder().
            setSource(new NodeConnectorId(source)).
            setDestination(new NodeConnectorId(destination)).
            build();
        return swlink;
    }

    /**
     * Return a port ID configured in the given node connector ID.
     *
     * @param nc    A node connector ID.
     * @param desc  A brief description about the given node connector ID.
     * @return  A port ID configured in {@code nc}.
     * @throws IllegalArgumentException
     *    The given instance is invalid.
     */
    private String getPortId(NodeConnectorId nc, String desc) {
        if (nc != null) {
            String id = nc.getValue();
            if (isValidPortId(id)) {
                return id;
            }
            throw new IllegalArgumentException(
                desc + " port is invalid: " + id);
        }

        throw new IllegalArgumentException(desc + " port cannot be null.");
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
        if (o == null || !getClass().equals(o.getClass())) {
            return false;
        }

        XmlStaticSwitchLink xswl = (XmlStaticSwitchLink)o;
        return (Objects.equals(source, xswl.source) &&
                Objects.equals(destination, xswl.destination));
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        return Objects.hash(getClass(), source, destination);
    }

    /**
     * Return a string representation of this instance.
     *
     * @return  A string representation of this instance.
     */
    @Override
    public String toString() {
        StringBuilder builder = new StringBuilder("static-switch-link[src=").
            append(source).append(", dst=").append(destination).append(']');
        return builder.toString();
    }
}
