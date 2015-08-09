/*
 * Copyright (c) 2014, 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.flow.filter;

import javax.xml.bind.annotation.XmlAccessType;
import javax.xml.bind.annotation.XmlAccessorType;
import javax.xml.bind.annotation.XmlAttribute;
import javax.xml.bind.annotation.XmlElement;
import javax.xml.bind.annotation.XmlRootElement;
import javax.xml.bind.annotation.adapters.XmlJavaTypeAdapter;

import com.fasterxml.jackson.databind.annotation.JsonSerialize;

import org.opendaylight.vtn.manager.VBridgeIfPath;
import org.opendaylight.vtn.manager.VInterfacePath;
import org.opendaylight.vtn.manager.VInterfacePathAdapter;
import org.opendaylight.vtn.manager.VTerminalIfPath;

/**
 * {@code RedirectFilter} class describes the REDIRECT flow filter which
 * forwards the specified packets to another virtual interface in the VTN.
 *
 * <h4>Example JSON</h4>
 * <pre class="prettyprint lang-json">
 * {
 * &nbsp;&nbsp;"destination": {
 * &nbsp;&nbsp;&nbsp;&nbsp;"bridge": "vbridge_1",
 * &nbsp;&nbsp;&nbsp;&nbsp;"interface": "if0",
 * &nbsp;&nbsp;},
 * &nbsp;&nbsp;"output": true
 * }</pre>
 *
 * @since  Helium
 */
@JsonSerialize(include = JsonSerialize.Inclusion.NON_NULL)
@XmlRootElement(name = "redirect")
@XmlAccessorType(XmlAccessType.NONE)
public final class RedirectFilter extends FilterType {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = 2299914147430572837L;

    /**
     * The path to the virtual interface where the specified packets are
     * forwarded.
     *
     * <ul>
     *   <li>
     *     The VTN name configured in the <strong>tenant</strong> attribute is
     *     always ignored. The VTN name is always determined by the location
     *     of the virtual node that contains the flow filter.
     *   </li>
     *   <li>
     *     The location of the virtual interface that contains this
     *     flow filter can not be specified.
     *   </li>
     *   <li>
     *     Note that every packet redirected by the flow filter is discarded
     *     if the virtual interface specified by this element does not exist
     *     in the VTN.
     *   </li>
     * </ul>
     * <p>
     *   Packet redirection should be configured not to cause the packet loop.
     *   The number of virtual node hops per a flow (the number of packet
     *   redirections) is limited to <strong>100</strong>.
     *   If the number of virtual node hops exceeds the limit, it is treated
     *   as the packet loop and then the packet is discarded.
     * </p>
     */
    @XmlJavaTypeAdapter(VInterfacePathAdapter.class)
    @XmlElement(required = true)
    private VInterfacePath  destination;

    /**
     * Determine the direction of the packet redirection.
     *
     * <ul>
     *   <li>
     *     If <strong>true</strong> is specified, the packet is redirected
     *     as outgoing packet.
     *     <ul>
     *       <li>
     *         The redirected packet will be treated as if it is transmitted
     *         from the virtual interface specified by the
     *         <strong>destination</strong> element.
     *       </li>
     *       <li>
     *         A list of flow filters for outgoing packets configured in the
     *         virtual interface specified by <strong>destination</strong>
     *         will be evaluated against the redirected packet.
     *         If the packet is passed by the flow filter, it is transmitted
     *         to the physical network mapped to the virtual interface by
     *         port mapping. The packet will be discarded if port mapping
     *         is not configured to the virtual interface.
     *       </li>
     *     </ul>
     *   </li>
     *   <li>
     *     If <strong>false</strong> is specified, the packet is redirected
     *     as incoming packet.
     *     <ul>
     *       <li>
     *         The redirected packet will be treated as if it is received from
     *         the virtual interface specified by the
     *         <strong>destination</strong> element.
     *         The packet is redirected even if no physical network is mapped
     *         to the destination virtual interface by port mapping.
     *       </li>
     *       <li>
     *         A list of flow filters for incoming packets configured in the
     *         virtual interface specified by <strong>destination</strong>
     *         will be evaluated against the redirected packet.
     *         If the packet is passed by the flow filter, it is forwarded to
     *         the virtual node that contains the virtual interface.
     *       </li>
     *     </ul>
     *   </li>
     *   <li>
     *     If omitted, it will be treated as if <strong>false</strong> is
     *     specified.
     *   </li>
     * </ul>
     */
    @XmlAttribute
    private boolean  output;

    /**
     * Private constructor only for JAXB.
     */
    private RedirectFilter() {
    }

    /**
     * Construct a new instance that specifies the packet redirection to be
     * done by a flow filter.
     *
     * @param path
     *   The path to the vBridge interface where the specified packets should
     *   be redirected.
     *   <ul>
     *     <li>{@code null} can not be specified.</li>
     *     <li>
     *       The VTN name configured in {@code path} is always ignored.
     *       The VTN name is always determined by the location of the virtual
     *       node that contains the flow filter.
     *     </li>
     *   </ul>
     * @param out     {@code true} means the redirected packet should be
     *                treated as outgoing packet.
     *                {@code false} means the redirected packet should be
     *                treated as incoming packet.
     */
    public RedirectFilter(VBridgeIfPath path, boolean out) {
        destination = path;
        output = out;
    }

    /**
     * Construct a new instance that specifies the packet redirection to be
     * done by a flow filter.
     *
     * @param path
     *   The path to the vTerminal interface where the specified packets should
     *   be redirected.
     *   <ul>
     *     <li>{@code null} can not be specified.</li>
     *     <li>
     *       The VTN name configured in {@code path} is always ignored.
     *       The VTN name is always determined by the location of the virtual
     *       node that contains the flow filter.
     *     </li>
     *   </ul>
     * @param out     {@code true} means the redirected packet should be
     *                treated as outgoing packet.
     *                {@code false} means the redirected packet should be
     *                treated as incoming packet.
     */
    public RedirectFilter(VTerminalIfPath path, boolean out) {
        destination = path;
        output = out;
    }

    /**
     * Return the location of the virtual interface where the packet should
     * be redirected.
     *
     * @return  A {@link VInterfacePath} instance.
     */
    public VInterfacePath getDestination() {
        return destination;
    }

    /**
     * Determine whether the direction of packet redirection.
     *
     * @return  {@code true} is returned if the redirected packet should be
     *          treated as outgoing packet.
     *          {@code false} is returned if the redirected packet should be
     *          treated as incoming packet.
     */
    public boolean isOutput() {
        return output;
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
        if (!super.equals(o)) {
            return false;
        }

        RedirectFilter rf = (RedirectFilter)o;
        if (output != rf.output) {
            return false;
        }

        if (destination == null) {
            return (rf.destination == null);
        }

        return destination.equals(rf.destination);
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        int h = super.hashCode();
        if (output) {
            h++;
        }
        if (destination != null) {
            h += (destination.hashCode() * 17);
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
        StringBuilder builder = new StringBuilder("RedirectFilter[");
        if (destination != null) {
            builder.append("destination=").append(destination).append(',');
        }
        return builder.append("output=").append(output).append(']').toString();
    }
}
