/*
 * Copyright (c) 2014, 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.flow.cond;

import java.util.Objects;

import javax.xml.bind.annotation.XmlAccessType;
import javax.xml.bind.annotation.XmlAccessorType;
import javax.xml.bind.annotation.XmlAttribute;
import javax.xml.bind.annotation.XmlRootElement;

import com.fasterxml.jackson.databind.annotation.JsonSerialize;

/**
 * This class describes ICMP header fields in IPv4 packet to match against
 * packets.
 *
 * <h4>Example JSON</h4>
 * <pre class="prettyprint lang-json">
 * {
 * &nbsp;&nbsp;"type": 3,
 * &nbsp;&nbsp;"code": 0
 * }</pre>
 *
 * @since  Helium
 */
@JsonSerialize(include = JsonSerialize.Inclusion.NON_NULL)
@XmlRootElement(name = "icmpmatch")
@XmlAccessorType(XmlAccessType.NONE)
public final class IcmpMatch extends L4Match {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = 7819676315587047891L;

    /**
     * An ICMP type value to match against packets.
     *
     * <ul>
     *   <li>
     *     The range of value that can be specified is from
     *     <strong>0</strong> to <strong>255</strong>.
     *   </li>
     *   <li>
     *     If this attribute is omitted, this elements specifies the
     *     condition that matches every ICMP type.
     *   </li>
     * </ul>
     */
    @XmlAttribute
    private Short  type;

    /**
     * An ICMP code value to match against packets.
     *
     * <ul>
     *   <li>
     *     The range of value that can be specified is from
     *     <strong>0</strong> to <strong>255</strong>.
     *   </li>
     *   <li>
     *     If this attribute is omitted, this elements specifies the
     *     condition that matches every ICMP code.
     *   </li>
     * </ul>
     */
    @XmlAttribute
    private Short  code;

    /**
     * Private constructor only for JAXB.
     */
    @SuppressWarnings("unused")
    private IcmpMatch() {
    }

    /**
     * Construct a new instance.
     *
     * @param type
     *   An ICMP type value to match against packets.
     *   <ul>
     *     <li>
     *       The range of value that can be specified is from
     *       <strong>0</strong> to <strong>255</strong>.
     *     </li>
     *     <li>
     *       {@code null} means that every ICMP type value should be matched.
     *     </li>
     *   </ul>
     * @param code
     *   An ICMP code value to match against packets.
     *   <ul>
     *     <li>
     *       The range of value that can be specified is from
     *       <strong>0</strong> to <strong>255</strong>.
     *     </li>
     *     <li>
     *       {@code null} means that every ICMP type value should be matched.
     *     </li>
     *   </ul>
     */
    public IcmpMatch(Short type, Short code) {
        this.type = type;
        this.code = code;
    }

    /**
     * Return the ICMP type to match against packets.
     *
     * @return  A {@link Short} instance which represents the ICMP type to
     *          match against packets.
     *          {@code null} is returned if this instance does not describe
     *          the ICMP type to match.
     */
    public Short getType() {
        return type;
    }

    /**
     * Return the ICMP code to match against packets.
     *
     * @return  A {@link Short} instance which represents the ICMP code to
     *          match against packets.
     *          {@code null} is returned if this instance does not describe
     *          the ICMP code to match.
     */
    public Short getCode() {
        return code;
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
        if (!(o instanceof IcmpMatch)) {
            return false;
        }

        IcmpMatch icmp = (IcmpMatch)o;
        return (Objects.equals(type, icmp.type) &&
                Objects.equals(code, icmp.code));
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        return Objects.hash(type, code);
    }

    /**
     * Return a string representation of this object.
     *
     * @return  A string representation of this object.
     */
    @Override
    public String toString() {
        StringBuilder builder = new StringBuilder("IcmpMatch[");
        String sep = "";
        if (type != null) {
            builder.append("type=").append(type.toString());
            sep = ",";
        }
        if (code != null) {
            builder.append(sep).append("code=").append(code.toString());
        }
        builder.append(']');

        return builder.toString();
    }
}
