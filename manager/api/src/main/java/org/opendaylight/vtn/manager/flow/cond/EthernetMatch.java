/*
 * Copyright (c) 2014-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.flow.cond;

import java.io.Serializable;
import java.util.Objects;

import javax.xml.bind.annotation.XmlAccessType;
import javax.xml.bind.annotation.XmlAccessorType;
import javax.xml.bind.annotation.XmlAttribute;
import javax.xml.bind.annotation.XmlRootElement;

import com.fasterxml.jackson.databind.annotation.JsonSerialize;

import org.opendaylight.vtn.manager.util.EtherAddress;

import org.opendaylight.controller.sal.packet.address.EthernetAddress;
import org.opendaylight.controller.sal.utils.Status;
import org.opendaylight.controller.sal.utils.StatusCode;

/**
 * This class describes Ethernet header fields to match against packets.
 *
 * <h4>Example JSON</h4>
 * <pre class="prettyprint lang-json">
 * {
 * &nbsp;&nbsp;"src": "00:11:22:33:44:55",
 * &nbsp;&nbsp;"dst": "00:aa:bb:cc:dd:ee",
 * &nbsp;&nbsp;"type": 2048,
 * &nbsp;&nbsp;"vlan": 1,
 * &nbsp;&nbsp;"vlanpri": 2
 * }</pre>
 *
 * @since  Helium
 */
@JsonSerialize(include = JsonSerialize.Inclusion.NON_NULL)
@XmlRootElement(name = "ethermatch")
@XmlAccessorType(XmlAccessType.NONE)
public final class EthernetMatch implements Serializable {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = -3985178322341719720L;

    /**
     * Status of JAXB binding validation.
     *
     * <p>
     *   This field does not affect object identity.
     * </p>
     */
    private Status  validationStatus;

    /**
     * Source MAC address to match.
     */
    private EtherAddress  sourceAddress;

    /**
     * Destination MAC address to match.
     */
    private EtherAddress  destinationAddress;

    /**
     * An Ethernet type value to match against packets.
     * <ul>
     *   <li>
     *     The range of value that can be specified is from
     *     <strong>0</strong> to <strong>65535</strong>.
     *   </li>
     *   <li>
     *     If this attribute is omitted, this element describes the
     *     condition that matches every Ethernet type.
     *   </li>
     * </ul>
     */
    @XmlAttribute
    private Integer  type;

    /**
     * A VLAN ID value to match against packets.
     * <ul>
     *   <li>
     *     The range of value that can be specified is from
     *     <strong>0</strong> to <strong>4095</strong>.
     *   </li>
     *   <li>
     *     <strong>0</strong> implies untagged Ethernet frame.
     *   </li>
     *   <li>
     *     If this attribute is omitted, this element describes the condition
     *     that matches every VLAN ID.
     *   </li>
     * </ul>
     */
    @XmlAttribute
    private Short  vlan;

    /**
     * A VLAN priority value to match against packets.
     * <ul>
     *   <li>
     *     The range of value that can be specified is from
     *     <strong>0</strong> to <strong>7</strong>.
     *   </li>
     *   <li>
     *     If a valid priority is specified, a valid VLAN ID except for
     *     zero must be specified to the <strong>vlan</strong> attribute.
     *   </li>
     *   <li>
     *     If this attribute is omitted, this element describes the
     *     condition that matches every VLAN priority, including
     *     untagged Ethernet frame.
     *   </li>
     * </ul>
     */
    @XmlAttribute(name = "vlanpri")
    private Byte  priority;

    /**
     * Private constructor only for JAXB.
     */
    @SuppressWarnings("unused")
    private EthernetMatch() {
    }

    /**
     * Construct a new instance.
     *
     * @param src   An {@link EthernetAddress} instance which represents
     *              the source MAC address to match.
     *              {@code null} means that every source MAC address should
     *              be matched.
     * @param dst   An {@link EthernetAddress} instance which represents
     *              the destination MAC address to match.
     *              {@code null} means that every destination MAC address
     *              should be matched.
     * @param type
     *   An {@link Integer} instance which represents the  Ethernet type value
     *   to match against packets.
     *   <ul>
     *     <li>
     *       The range of value that can be specified is from
     *       <strong>0</strong> to <strong>65535</strong>.
     *     </li>
     *     <li>
     *       {@code null} means that every Ethernet type should be matched.
     *     </li>
     *   </ul>
     * @param vlan
     *   A {@link Short} instance which represents the VLAN ID to match
     *   against packets.
     *   <ul>
     *     <li>
     *       The range of value that can be specified is from
     *       <strong>0</strong> to <strong>4095</strong>.
     *     </li>
     *     <li>
     *       <strong>0</strong> implies untagged Ethernet frame.
     *     </li>
     *     <li>
     *       {@code null} means that every VLAN frame, including untagged
     *       Ethernet frame, should be matched.
     *     </li>
     *   </ul>
     * @param pri
     *   A {@link Byte} instance which represents VLAN priority value to match
     *   against packets.
     *   <ul>
     *     <li>
     *       The range of value that can be specified is from
     *       <strong>0</strong> to <strong>7</strong>.
     *     </li>
     *     <li>
     *       If a valid priority is specified, a valid VLAN ID except for
     *       zero must be specified to {@code vlan}.
     *       {@code pri}.
     *     </li>
     *     <li>
     *       {@code null} means that every VLAN priority, including untagged
     *       Ethernet frame, should be matched.
     *     </li>
     *   </ul>
     */
    public EthernetMatch(EthernetAddress src, EthernetAddress dst,
                         Integer type, Short vlan, Byte pri) {
        sourceAddress = EtherAddress.create(src);
        destinationAddress = EtherAddress.create(dst);
        this.type = type;
        this.vlan = vlan;
        priority = pri;
    }

    /**
     * Construct a new instance.
     *
     * @param src   An {@link EtherAddress} instance which represents
     *              the source MAC address to match.
     *              {@code null} means that every source MAC address should
     *              be matched.
     * @param dst   An {@link EtherAddress} instance which represents
     *              the destination MAC address to match.
     *              {@code null} means that every destination MAC address
     *              should be matched.
     * @param type
     *   An {@link Integer} instance which represents the  Ethernet type value
     *   to match against packets.
     *   <ul>
     *     <li>
     *       The range of value that can be specified is from
     *       <strong>0</strong> to <strong>65535</strong>.
     *     </li>
     *     <li>
     *       {@code null} means that every Ethernet type should be matched.
     *     </li>
     *   </ul>
     * @param vlan
     *   A {@link Short} instance which represents the VLAN ID to match
     *   against packets.
     *   <ul>
     *     <li>
     *       The range of value that can be specified is from
     *       <strong>0</strong> to <strong>4095</strong>.
     *     </li>
     *     <li>
     *       <strong>0</strong> implies untagged Ethernet frame.
     *     </li>
     *     <li>
     *       {@code null} means that every VLAN frame, including untagged
     *       Ethernet frame, should be matched.
     *     </li>
     *   </ul>
     * @param pri
     *   A {@link Byte} instance which represents VLAN priority value to match
     *   against packets.
     *   <ul>
     *     <li>
     *       The range of value that can be specified is from
     *       <strong>0</strong> to <strong>7</strong>.
     *     </li>
     *     <li>
     *       If a valid priority is specified, a valid VLAN ID except for
     *       zero must be specified to {@code vlan}.
     *       {@code pri}.
     *     </li>
     *     <li>
     *       {@code null} means that every VLAN priority, including untagged
     *       Ethernet frame, should be matched.
     *     </li>
     *   </ul>
     * @since  Lithium
     */
    public EthernetMatch(EtherAddress src, EtherAddress dst, Integer type,
                         Short vlan, Byte pri) {
        sourceAddress = src;
        destinationAddress = dst;
        this.type = type;
        this.vlan = vlan;
        priority = pri;
    }

    /**
     * Return the source MAC address to match against packets.
     *
     * @return  An {@link EthernetAddress} instance which represents the
     *          source MAC address to match against packets.
     *          {@code null} is returned if this instance does not describe
     *          the source MAC address to match.
     */
    public EthernetAddress getSourceAddress() {
        EtherAddress addr = sourceAddress;
        return (addr == null) ? null : addr.getEthernetAddress();
    }

    /**
     * Return the destination MAC address to match against packets.
     *
     * @return  An {@link EthernetAddress} instance which represents the
     *          destination MAC address to match against packets.
     *          {@code null} is returned if this instance does not describe
     *          the destination MAC address to match.
     */
    public EthernetAddress getDestinationAddress() {
        EtherAddress addr = destinationAddress;
        return (addr == null) ? null : addr.getEthernetAddress();
    }

    /**
     * Return the source MAC address to match against packets.
     *
     * @return  An {@link EtherAddress} instance which represents the
     *          source MAC address to match against packets.
     *          {@code null} is returned if this instance does not describe
     *          the source MAC address to match.
     * @since  Lithium
     */
    public EtherAddress getSourceEtherAddress() {
        return sourceAddress;
    }

    /**
     * Return the destination MAC address to match against packets.
     *
     * @return  An {@link EtherAddress} instance which represents the
     *          destination MAC address to match against packets.
     *          {@code null} is returned if this instance does not describe
     *          the destination MAC address to match.
     * @since  Lithium
     */
    public EtherAddress getDestinationEtherAddress() {
        return destinationAddress;
    }

    /**
     * Return the Ethernet type to match against packets.
     *
     * @return  An {@link Integer} instance which represents the Ethernet type
     *          to match against packets.
     *          {@code null} is returned if this instance does not describe
     *          the Ethernet type to match.
     */
    public Integer getType() {
        return type;
    }

    /**
     * Return the VLAN ID to match against packets.
     *
     * @return  A {@link Short} instance which represents the VLAN ID to match
     *          against packets.
     *          {@code null} is returned if this instance does not describe
     *          the VLAN ID to match.
     */
    public Short getVlan() {
        return vlan;
    }

    /**
     * Return the VLAN priority to match against packets.
     *
     * @return  A {@link Byte} instance which represents the VLAN priority
     *          to match against packets.
     *          {@code null} is returned if this instance does not describe the
     *          VLAN priority to match.
     */
    public Byte getVlanPriority() {
        return priority;
    }

    /**
     * Return the result of validation.
     *
     * <p>
     *   If this instance is created from XML data, it should be validated
     *   by this method.
     * </p>
     *
     * @return  A {@link Status} which indicates the result of validation.
     *          If this instance keeps invalid contents, a {@link Status}
     *          instance which keeps an error status is returned.
     */
    public Status getValidationStatus() {
        return validationStatus;
    }

    /**
     * Return a string representation of the source MAC address to match
     * against packets.
     *
     * <p>
     *   {@code null} is returned if this instance does not describe the
     *   source MAC address to match.
     * </p>
     * <p>
     *   Note that below description of return value of this method is
     *   written for REST API document.
     * </p>
     *
     * @return
     *   A string representation of the source MAC address to match against
     *   packets.
     *   <ul>
     *     <li>
     *       A MAC address is represented by hexadecimal notation with
     *       {@code ':'} inserted between octets.
     *       (e.g. {@code "11:22:33:aa:bb:cc"})
     *     </li>
     *     <li>
     *       If this attribute is omitted, this element describes the
     *       condition that matches every source MAC address.
     *     </li>
     *   </ul>
     * @deprecated  Only for JAXB.
     *              Use {@link #getSourceAddress()} instead.
     */
    @XmlAttribute(name = "src")
    public String getSourceMacAddress() {
        return toString(sourceAddress);
    }

    /**
     * Set the source MAC address to match against packets.
     *
     * <p>
     *   This method is called by JAXB.
     * </p>
     *
     * @param mac  A string representation of the MAC address.
     */
    @SuppressWarnings("unused")
    private void setSourceMacAddress(String mac) {
        sourceAddress = toEtherAddress(mac, "source");
    }

    /**
     * Return a string representation of the destination MAC address to match
     * against packets.
     *
     * <p>
     *   {@code null} is returned if this instance does not describe the
     *   destination MAC address to match.
     * </p>
     * <p>
     *   Note that below description of return value of this method is
     *   written for REST API document.
     * </p>
     *
     * @return
     *   A string representation of the destination MAC address to match
     *   against packets.
     *   <ul>
     *     <li>
     *       A MAC address is represented by hexadecimal notation with
     *       {@code ':'} inserted between octets.
     *       (e.g. {@code "11:22:33:aa:bb:cc"})
     *     </li>
     *     <li>
     *       If this attribute is omitted, this element describes the
     *       condition that matches every destination MAC address.
     *     </li>
     *   </ul>
     * @deprecated  Only for JAXB.
     *              Use {@link #getDestinationAddress()} instead.
     */
    @XmlAttribute(name = "dst")
    public String getDestinationMacAddress() {
        return toString(destinationAddress);
    }

    /**
     * Set the destination MAC address to match against packets.
     *
     * <p>
     *   This method is called by JAXB.
     * </p>
     *
     * @param mac  A string representation of the MAC address.
     */
    @SuppressWarnings("unused")
    private void setDestinationMacAddress(String mac) {
        destinationAddress = toEtherAddress(mac, "destination");
    }

    /**
     * Convert an {@link EtherAddress} instance to a string.
     *
     * @param addr  An {@link EtherAddress} instance.
     * @return  A string representation of the specified instance.
     *         {@code null} is returned if {@code null} is specified.
     */
    private String toString(EtherAddress addr) {
        return (addr == null) ? null : addr.getText();
    }

    /**
     * Create a string which indicates an invalid MAC address is specified.
     *
     * @param mac    A string representation of the MAC address.
     * @param desc   A brief description about the MAC address.
     * @param cause  A throwable which indicates the cause of error.
     * @return  A string which indicates an invalid MAC address is specified.
     */
    private String invalidAddress(String mac, String desc, Throwable cause) {
        return new StringBuilder("Invalid ").
            append(desc).append(" MAC address: ").append(mac).
            append(": ").append(cause.getMessage()).
            toString();
    }

    /**
     * Convert the given string into an {@link EtherAddress} instance.
     *
     * @param mac   A string representation of the MAC address.
     * @param desc  A brief description about the value.
     * @return  An {@link EtherAddress} instance.
     */
    private EtherAddress toEtherAddress(String mac, String desc) {
        try {
            return EtherAddress.create(mac);
        } catch (Exception e) {
            String msg = invalidAddress(mac, desc, e);
            validationStatus = new Status(StatusCode.BADREQUEST, msg);
        }

        return null;
    }

    /**
     * Determine whether the given ethernet match contains the same condition
     * for MAC address.
     *
     * @param em  The ethernet match object to be compared.
     * @return  {@code true} only if the given object contains the same
     *          condition for MAC address.
     */
    private boolean equalsAddress(EthernetMatch em) {
        return (Objects.equals(sourceAddress, em.sourceAddress) &&
                Objects.equals(destinationAddress, em.destinationAddress));
    }

    /**
     * Determine whether the given ethernet match contains the same condition
     * for VLAN.
     *
     * @param em  The ethernet match object to be compared.
     * @return  {@code true} only if the given object contains the same
     *          condition for VLAN.
     */
    private boolean equalsVlan(EthernetMatch em) {
        return (Objects.equals(vlan, em.vlan) &&
                Objects.equals(priority, em.priority));
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
        if (!(o instanceof EthernetMatch)) {
            return false;
        }

        EthernetMatch em = (EthernetMatch)o;
        return (equalsAddress(em) && equalsVlan(em) &&
                Objects.equals(type, em.type));
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        return Objects.hash(sourceAddress, destinationAddress, type,
                            vlan, priority);
    }

    /**
     * Return a string representation of this object.
     *
     * @return  A string representation of this object.
     */
    @Override
    public String toString() {
        StringBuilder builder = new StringBuilder("EthernetMatch[");
        String sep = "";
        if (sourceAddress != null) {
            builder.append("src=").append(sourceAddress.getText());
            sep = ",";
        }
        if (destinationAddress != null) {
            builder.append(sep)
                .append("dst=").append(destinationAddress.getText());
            sep = ",";
        }
        if (type != null) {
            builder.append(sep).append("type=0x").
                append(Integer.toHexString(type.intValue()));
            sep = ",";
        }
        if (vlan != null) {
            builder.append(sep).append("vlan=").append(vlan.toString());
            sep = ",";
        }
        if (priority != null) {
            builder.append(sep).append("pri=").append(priority.toString());
        }
        builder.append(']');

        return builder.toString();
    }
}
