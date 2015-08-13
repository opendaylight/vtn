/*
 * Copyright (c) 2014, 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager;

import java.io.Serializable;
import java.util.Objects;

import javax.xml.bind.annotation.XmlAccessType;
import javax.xml.bind.annotation.XmlAccessorType;
import javax.xml.bind.annotation.XmlAttribute;
import javax.xml.bind.annotation.XmlElement;
import javax.xml.bind.annotation.XmlRootElement;
import javax.xml.bind.annotation.adapters.XmlJavaTypeAdapter;

import com.fasterxml.jackson.databind.annotation.JsonSerialize;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.VirtualRouteReason;

/**
 * {@code VNodeRoute} class is used to represent the virtual packet
 * routing within the VTN.
 *
 * <p>
 *   The route of the packet from the source to the destination virtual node
 *   is represented by a sequence of {@code VNodeRoute} instances.
 * </p>
 *
 * <h4>Example JSON</h4>
 * <pre class="prettyprint lang-json">
 * {
 * &nbsp;&nbsp;"reason": "PORTMAPPED",
 * &nbsp;&nbsp;"path": {
 * &nbsp;&nbsp;&nbsp;&nbsp;"tenant": "vtn_1",
 * &nbsp;&nbsp;&nbsp;&nbsp;"bridge": "vbridge_1",
 * &nbsp;&nbsp;&nbsp;&nbsp;"interface": "if_1"
 * &nbsp;&nbsp;}
 * }</pre>
 *
 * @since  Helium
 */
@JsonSerialize(include = JsonSerialize.Inclusion.NON_NULL)
@XmlRootElement(name = "vnoderoute")
@XmlAccessorType(XmlAccessType.NONE)
public final class VNodeRoute implements Serializable {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = -9115647284222382981L;

    /**
     * A {@link VNodePath} which represents the location of the
     * virtual node inside the VTN.
     *
     * <p>
     *   Note that this element is omitted if this instance indicates that
     *   the data flow is terminated without packet forwarding.
     * </p>
     */
    @XmlJavaTypeAdapter(VNodePathAdapter.class)
    @XmlElement(name = "path")
    private VNodePath  path;

    /**
     * The reason why the packet is forwarded to the virtual node.
     */
    private VirtualRouteReason  reason;

    /**
     * Construct a new instance which indicates the data flow is terminated.
     */
    public VNodeRoute() {
    }

    /**
     * Construct a new instance.
     *
     * @param path     A {@link VNodePath} instance which specifies
     *                 the location of the virtual node inside the VTN.
     *                 Specifying {@code null} results in undefined behavior.
     * @param reason   A {@link VirtualRouteReason} instance which represents
     *                 the reason why the packet is forwarded.
     *                 Specifying {@code null} results in undefined behavior.
     */
    public VNodeRoute(VNodePath path, VirtualRouteReason reason) {
        this.path = path;
        this.reason = reason;
    }

    /**
     * Return the location of the virtual node.
     *
     * @return  A {@link VNodePath} instance which represents the
     *          location of the virtual node.
     *          {@code null} is returned if this instance indicates that the
     *          data flow is terminated without packet forwarding.
     */
    public VNodePath getPath() {
        return path;
    }

    /**
     * Return the reason why the packet is forwarded to the virtual node.
     *
     * @return  A {@link VirtualRouteReason} instance which represents the
     *          reason why the packet is forwarded.
     *          {@code null} is returned if this instance indicates that the
     *          data flow is terminated without packet forwarding.
     */
    public VirtualRouteReason getReason() {
        return reason;
    }

    /**
     * Return a string which indicates the reason why the packet is forwarded
     * to the virtual node.
     *
     * <p>
     *   {@code null} is returned if this instance does not keep the value.
     * </p>
     * <p>
     *   Note that below description of return value of this method is
     *   written for REST API document.
     * </p>
     *
     * @return
     *   A string which indicates the reason why the packet is forwarded
     *   to the virtual node specified by the <strong>path</strong> element.
     *   Available strings are listed below.
     *   <dl style="margin-left: 1em;">
     *     <dt>PORTMAPPED
     *     <dd style="margin-left: 1.5em;">
     *       Indicates the packet is mapped by the port mapping.
     *       <p>
     *         This value is set only if the virtual node specified by the
     *         <strong>path</strong> element is the ingress node.
     *       </p>
     *     <dt>VLANMAPPED
     *     <dd style="margin-left: 1.5em;">
     *       Indicates the packet is mapped by the VLAN mapping.
     *       <p>
     *         This value is set only if the virtual node specified by the
     *         <strong>path</strong> element is the ingress node.
     *       </p>
     *     <dt>MACMAPPED
     *     <dd style="margin-left: 1.5em;">
     *       Indicates the packet is mapped by the MAC mapping.
     *       <p>
     *         This value is set only if the virtual node specified by the
     *         <strong>path</strong> element is the ingress node.
     *       </p>
     *     <dt>FORWARDED
     *     <dd style="margin-left: 1.5em;">
     *       Indicates the packet is forwarded through the virtual node.
     *       <p>
     *         For example, this reason is set when a packet is forwarded
     *         from the vBridge interface to another vBridge interface attached
     *         to the same vBridge.
     *       </p>
     *     <dt>REDIRECTED
     *     <dd style="margin-left: 1.5em;">
     *       Indicates the packet is redirected by the flow filter.
     *     <dt>LINKED
     *     <dd style="margin-left: 1.5em;">
     *       Indicates the packet is forwarded from the virtual node to
     *       another virtual node through virtual node link.
     *       <ul>
     *         <li>
     *           Currently the VTN Manager never set this reason because the
     *           virtual node link is not yet supported.
     *         </li>
     *       </ul>
     *  </dl>
     *  <p>
     *    Note that this attribute is omitted if this element indicates that
     *    the data flow is terminated without packet forwarding.
     *  </p>
     * @deprecated  Only for JAXB. Use {@link #getReason()} instead.
     */
    @XmlAttribute(name = "reason")
    public String getReasonString() {
        return (reason == null) ? null : reason.name();
    }

    /**
     * Set a string which indicates the reason why the packet is forwarded
     * to the virtual node.
     *
     * <p>
     *   This method is called by JAXB.
     * </p>
     *
     * @param str  A string representation of {@link VirtualRouteReason}.
     */
    @SuppressWarnings("unused")
    private void setReasonString(String str) {
        try {
            reason = VirtualRouteReason.valueOf(str);
        } catch (Exception e) {
            reason = null;
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
        if (!(o instanceof VNodeRoute)) {
            return false;
        }

        VNodeRoute vr = (VNodeRoute)o;
        return (Objects.equals(path, vr.path) &&
                Objects.equals(reason, vr.reason));
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        int h = 0;
        if (path != null) {
            h = path.hashCode();
        }
        if (reason != null) {
            h += reason.toString().hashCode() * 31;
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
        StringBuilder builder = new StringBuilder("VNodeRoute[");
        String sep = "";
        if (path != null) {
            builder.append("path=").append(path.toString());
            sep = ",";
        }
        if (reason != null) {
            builder.append(sep).append("reason=").append(reason.toString());
        }
        builder.append(']');

        return builder.toString();
    }
}
