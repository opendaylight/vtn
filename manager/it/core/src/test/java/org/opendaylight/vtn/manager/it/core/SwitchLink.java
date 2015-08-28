/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.core;

import java.util.Objects;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.topology.rev150209.VtnLinkInfo;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.topology.rev150209.vtn.topology.VtnLink;

/**
 * {@code SwitchLink} describes an inter-switch link.
 */
public final class SwitchLink {
    /**
     * The identifier of the link.
     */
    private final String  linkId;

    /**
     * The source port of the link.
     */
    private final String  source;

    /**
     * The destination port of the link.
     */
    private final String  destination;

    /**
     * True if this link is a static link.
     */
    private Boolean  staticLink;

    /**
     * Construct a new instance.
     *
     * @param src  The port identifier which specifies the source port of
     *             the link.
     * @param dst  The port identifier which specifies the destination port of
     *             the link.
     * @param st   {@code true} means that the given link is a static link.
     *             {@code false} means that the given link is a dynamic link
     *             detected by topology-manager.
     */
    public SwitchLink(String src, String dst, boolean st) {
        linkId = src;
        source = src;
        destination = dst;
        if (st) {
            staticLink = Boolean.TRUE;
        }
    }

    /**
     * Construct a new instance.
     *
     * @param vli  A {@link VtnLinkInfo} instance.
     */
    public SwitchLink(VtnLinkInfo vli) {
        linkId = vli.getLinkId().getValue();
        source = vli.getSource().getValue();
        destination = vli.getDestination().getValue();
    }

    /**
     * Construct a new instance.
     *
     * @param vlink  A {@link VtnLink} instance.
     */
    public SwitchLink(VtnLink vlink) {
        this((VtnLinkInfo)vlink);
        staticLink = vlink.isStaticLink();
    }

    /**
     * Return the identifier of this link.
     *
     * @return  The identifier of this link.
     */
    public String getLinkId() {
        return linkId;
    }

    /**
     * Return the identifier of the source port of this link.
     *
     * @return  The port identifier which specifies the source port of
     *          this link.
     */
    public String getSource() {
        return source;
    }

    /**
     * Return the identifier of the destination port of this link.
     *
     * @return  The port identifier which specifies the destinaion port of
     *          this link.
     */
    public String getDestination() {
        return destination;
    }

    /**
     * Determine whether this link is a static link or not.
     *
     * @return {@code true} only if this link is a static link.
     */
    public boolean isStaticLink() {
        return Boolean.TRUE.equals(staticLink);
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
        if (o == null || !getClass().equals(o.getClass())) {
            return false;
        }

        SwitchLink swl = (SwitchLink)o;
        return (Objects.equals(linkId, swl.linkId) &&
                Objects.equals(source, swl.source) &&
                Objects.equals(destination, swl.destination) &&
                Objects.equals(staticLink, swl.staticLink));
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        return Objects.hash(getClass(), linkId, source, destination,
                            staticLink);
    }

    /**
     * Return a string representation of this object.
     *
     * @return  A string representation of this object.
     */
    @Override
    public String toString() {
        StringBuilder builder = new StringBuilder("SwitchLink[");
        builder.append(source).append(" -> ").append(destination).
            append(": static=").append(staticLink).append(']');

        return builder.toString();
    }
}
