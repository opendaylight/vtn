/*
 * Copyright (c) 2014, 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.flow.cond;

import java.io.Serializable;
import java.util.Objects;

import javax.xml.bind.annotation.XmlAccessType;
import javax.xml.bind.annotation.XmlAccessorType;
import javax.xml.bind.annotation.XmlAttribute;
import javax.xml.bind.annotation.XmlRootElement;

import com.fasterxml.jackson.databind.annotation.JsonSerialize;

import org.opendaylight.vtn.manager.util.NumberUtils;

/**
 * This class describes the range of TCP/UDP port numbers to match against
 * packets.
 *
 * <h4>Example JSON</h4>
 * <pre class="prettyprint lang-json">
 * {
 * &nbsp;&nbsp;"from": 10,
 * &nbsp;&nbsp;"to": 20
 * }</pre>
 *
 * @since  Helium
 */
@JsonSerialize(include = JsonSerialize.Inclusion.NON_NULL)
@XmlRootElement(name = "portmatch")
@XmlAccessorType(XmlAccessType.NONE)
public final class PortMatch implements Serializable {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = -4334110900796924492L;

    /**
     * The minimum value (inclusive) in the range of TCP/UDP port numbers
     * to match against packets.
     * <ul>
     *   <li>
     *     This attribute is mandatory.
     *   </li>
     *   <li>
     *     The range of value that can be specified is from
     *     <strong>0</strong> to <strong>65535</strong>.
     *   </li>
     * </ul>
     */
    @XmlAttribute(name = "from", required = true)
    private Integer  portFrom;

    /**
     * The maximum value (inclusive) in the range of TCP/UDP port numbers
     * to match against packets.
     * <ul>
     *   <li>
     *     The range of value that can be specified is from
     *     <strong>0</strong> to <strong>65535</strong>.
     *   </li>
     *   <li>
     *     The value must be greater than or equal to the value specified
     *     to the <strong>from</strong> attribute.
     *   </li>
     *   <li>
     *     If this attribute is omitted, it will be treated as the port
     *     number specified to <strong>from</strong> attribute is specified.
     *   </li>
     * </ul>
     */
    @XmlAttribute(name = "to")
    private Integer  portTo;

    /**
     * Private constructor only for JAXB.
     */
    @SuppressWarnings("unused")
    private PortMatch() {
    }

    /**
     * Construct a new instance which describes the specified TCP/UDP port
     * number to match against packets.
     *
     * @param port
     *   A TCP/UDP port number to match against packets.
     *   <ul>
     *     <li>
     *       The range of value that can be specified is from
     *       <strong>0</strong> to <strong>65535</strong>.
     *     </li>
     *     <li>
     *       {@code null} cannot be specified.
     *     </li>
     *   </ul>
     */
    public PortMatch(Integer port) {
        portFrom = port;
        portTo = port;
    }

    /**
     * Construct a new instance which describes the specified TCP/UDP port
     * number to match against packets.
     *
     * @param port
     *   A TCP/UDP port number to match against packets.
     *   <ul>
     *     <li>
     *       The specified vlaue is treated as unsigned 16-bit integer.
     *     </li>
     *     <li>
     *       {@code null} cannot be specified.
     *     </li>
     *   </ul>
     */
    public PortMatch(Short port) {
        int iport = NumberUtils.getUnsigned(port.shortValue());
        portFrom = Integer.valueOf(iport);
        portTo = portFrom;
    }

    /**
     * Construct a new instance which describes the range of TCP/UDP port
     * numbers to match against packets.
     *
     * @param from
     *   The minumum value (inclusive) in the range of TCP/UDP port numbers
     *   to match against packets.
     *   <ul>
     *     <li>
     *       The range of value that can be specified is from
     *       <strong>0</strong> to <strong>65535</strong>.
     *     </li>
     *     <li>
     *       {@code null} cannot be specified.
     *     </li>
     *   </ul>
     * @param to
     *   The maximum value (inclusive) in the range of TCP/UDP port numbers
     *   to match against packets.
     *   <ul>
     *     <li>
     *       The range of value that can be specified is from
     *       <strong>0</strong> to <strong>65535</strong>.
     *     </li>
     *     <li>
     *       The value must be greater than or equal to the value passed to
     *       {@code from}.
     *     </li>
     *     <li>
     *       {@code null} is treated as if the value specified to {@code from}
     *       is specified.
     *     </li>
     *   </ul>
     */
    public PortMatch(Integer from, Integer to) {
        portFrom = from;
        portTo = (to == null) ? from : to;
    }

    /**
     * Construct a new instance which describes the range of TCP/UDP port
     * numbers to match against packets.
     *
     * @param from
     *   The minumum value (inclusive) in the range of TCP/UDP port numbers
     *   to match against packets.
     *   <ul>
     *     <li>
     *       The specified vlaue is treated as unsigned 16-bit integer.
     *     </li>
     *     <li>
     *       {@code null} cannot be specified.
     *     </li>
     *   </ul>
     * @param to
     *   The maximum value (inclusive) in the range of TCP/UDP port numbers
     *   to match against packets.
     *   <ul>
     *     <li>
     *       The specified vlaue is treated as unsigned 16-bit integer.
     *     </li>
     *     <li>
     *       The value must be greater than or equal to the value passed to
     *       {@code from}.
     *     </li>
     *     <li>
     *       {@code null} is treated as if the value specified to {@code from}
     *       is specified.
     *     </li>
     *   </ul>
     */
    public PortMatch(Short from, Short to) {
        int iport = NumberUtils.getUnsigned(from.shortValue());
        portFrom = Integer.valueOf(iport);
        if (to == null) {
            portTo = portFrom;
        } else {
            iport = NumberUtils.getUnsigned(to.shortValue());
            portTo = Integer.valueOf(iport);
        }
    }

    /**
     * Return the minimum (inclusive) value in the range of TCP/UDP port
     * numbers to match against packets.
     *
     * @return  An {@link Integer} instance which represents the minimum value
     *          in the range of TCP/UDP port numbers to match.
     */
    public Integer getPortFrom() {
        return portFrom;
    }

    /**
     * Return the maximum (inclusive) value in the range of TCP/UDP port
     * numbers to match against packets.
     *
     * @return  An {@link Integer} instance which represents the maximum value
     *          in the range of TCP/UDP port numbers to match.
     */
    public Integer getPortTo() {
        return portTo;
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
        if (!(o instanceof PortMatch)) {
            return false;
        }

        PortMatch pm = (PortMatch)o;
        return (Objects.equals(portFrom, pm.portFrom) &&
                Objects.equals(portTo, pm.portTo));
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        return Objects.hash(portFrom, portTo);
    }

    /**
     * Return a string representation of this object.
     *
     * @return  A string representation of this object.
     */
    @Override
    public String toString() {
        StringBuilder builder = new StringBuilder("PortMatch[");
        String sep = "";
        if (portFrom != null) {
            builder.append("from=").append(portFrom.toString());
            sep = ",";
        }
        if (portTo != null) {
            builder.append(sep).append("to=").append(portTo.toString());
        }
        builder.append(']');

        return builder.toString();
    }
}
