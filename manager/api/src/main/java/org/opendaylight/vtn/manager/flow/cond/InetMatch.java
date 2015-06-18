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
import java.net.InetAddress;
import java.util.Objects;

import javax.xml.bind.annotation.XmlAccessType;
import javax.xml.bind.annotation.XmlAccessorType;
import javax.xml.bind.annotation.XmlAttribute;
import javax.xml.bind.annotation.XmlRootElement;
import javax.xml.bind.annotation.XmlSeeAlso;

import com.fasterxml.jackson.databind.annotation.JsonSerialize;

import org.opendaylight.controller.sal.utils.Status;
import org.opendaylight.controller.sal.utils.StatusCode;

/**
 * This class describes internet protocol header fields to match against
 * packets.
 *
 * <h4>Example JSON</h4>
 * <pre class="prettyprint lang-json">
 * {
 * &nbsp;&nbsp;"src": "192.168.10.2",
 * &nbsp;&nbsp;"srcsuffix": 31,
 * &nbsp;&nbsp;"dst": "192.168.10.0",
 * &nbsp;&nbsp;"dstsuffix": 24,
 * &nbsp;&nbsp;"protocol": 6,
 * &nbsp;&nbsp;"dscp": 1
 * }</pre>
 *
 * @since  Helium
 */
@JsonSerialize(include = JsonSerialize.Inclusion.NON_NULL)
@XmlRootElement(name = "inetmatch")
@XmlAccessorType(XmlAccessType.NONE)
@XmlSeeAlso(Inet4Match.class)
public abstract class InetMatch implements Serializable {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = 5632047937827897027L;

    /**
     * Status of JAXB binding validation.
     *
     * <p>
     *   This field does not affect object identity.
     * </p>
     */
    private Status  validationStatus;

    /**
     * Source IP address to match.
     */
    private InetAddress  sourceAddress;

    /**
     * Destination IP address to match.
     */
    private InetAddress  destinationAddress;

    /**
     * A CIDR suffix for the source IP address matching.
     * <ul>
     *   <li>
     *     This attribute is ignored unless a valid IP address is specified
     *     to the <strong>src</strong> attribute.
     *   </li>
     *   <li>
     *     If a valid CIDR suffix is specified, only bits in the source IP
     *     address specified by the CIDR suffix are used for matching.
     *     <ul>
     *       <li>
     *         The valid range of the CIDR suffix for IPv4 address is
     *         from <strong>1</strong> to <strong>31</strong>.
     *       </li>
     *     </ul>
     *   </li>
     *   <li>
     *     If this attribute is omitted, all bits in the source IP address
     *     are used for matching.
     *   </li>
     * </ul>
     */
    @XmlAttribute(name = "srcsuffix")
    private Short  sourceSuffix;

    /**
     * A CIDR suffix for the destination IP address matching.
     * <ul>
     *   <li>
     *     This attribute is ignored unless a valid IP address is specified
     *     to the <strong>dst</strong> attribute.
     *   </li>
     *   <li>
     *     If a valid CIDR suffix is specified, only bits in the destination
     *     IP address specified by the CIDR suffix are used for matching.
     *     <ul>
     *       <li>
     *         The valid range of the CIDR suffix for IPv4 address is
     *         from <strong>1</strong> to <strong>31</strong>.
     *       </li>
     *     </ul>
     *   </li>
     *   <li>
     *     If this attribute is omitted, all bits in the destination
     *     IP address are used for matching.
     *   </li>
     * </ul>
     */
    @XmlAttribute(name = "dstsuffix")
    private Short  destinationSuffix;

    /**
     * An IP protocol type value to match against packets.
     * <ul>
     *   <li>
     *     The range of value that can be specified is from
     *     <strong>0</strong> to <strong>255</strong>.
     *   </li>
     *   <li>
     *     If this attribute is omitted, this elements specifies the
     *     condition that matches every IP protocol type.
     *   </li>
     * </ul>
     */
    @XmlAttribute
    private Short  protocol;

    /**
     * A DSCP field value to match against packets.
     * <ul>
     *   <li>
     *     The range of value that can be specified is from
     *     <strong>0</strong> to <strong>63</strong>.
     *   </li>
     *   <li>
     *     If this attribute is omitted, this elements specifies the
     *     condition that matches every DSCP field.
     *   </li>
     * </ul>
     */
    @XmlAttribute
    private Byte  dscp;

    /**
     * Construct a new instance which describes every IP header.
     */
    InetMatch() {
    }

    /**
     * Construct a new instance.
     *
     * @param src
     *   An {@link InetAddress} instance which represents the source
     *   IP address to match.
     *   {@code null} means that every source IP address should be mathched.
     * @param srcsuff
     *   A CIDR suffix for the source IP address.
     *   <ul>
     *     <li>
     *       This value is not used unless a non-{@code null} value is
     *       specified to {@code src}.
     *     </li>
     *     <li>
     *       If a valid CIDR suffix is specified, only bits in the source IP
     *       address specified by the CIDR suffix are used for matching.
     *     </li>
     *     <li>
     *       {@code null} means that all bits in the source IP address should
     *       be used for matching.
     *     </li>
     *   </ul>
     * @param dst
     *   An {@link InetAddress} instance which represents the destination
     *   IP address to match.
     *   {@code null} means that every destination IP address should be
     *   mathched.
     * @param dstsuff
     *   A CIDR suffix for the destination IP address.
     *   <ul>
     *     <li>
     *       This value is not used unless a non-{@code null} value is
     *       specified to {@code src}.
     *     </li>
     *     <li>
     *       If a valid CIDR suffix is specified, only bits in the destination
     *       IP address specified by the CIDR suffix are used for matching.
     *     </li>
     *     <li>
     *       {@code null} means that all bits in the destination IP address
     *       should be used for matching.
     *     </li>
     *   </ul>
     * @param proto
     *   An IP protocol type value to match against packets.
     *   <ul>
     *     <li>
     *       The range of value that can be specified is from
     *       <strong>0</strong> to <strong>255</strong>.
     *     </li>
     *     <li>
     *       {@code null} means that every IP protocol type should be matched.
     *     </li>
     *   </ul>
     * @param dscp
     *   A DSCP field value to match against packets.
     *   <ul>
     *     <li>
     *       The range of value that can be specified is from
     *       <strong>0</strong> to <strong>63</strong>.
     *     </li>
     *     <li>
     *       {@code null} means that every DSCP field value should be matched.
     *     </li>
     *   </ul>
     * @throws IllegalArgumentException
     *    Invalid IP address is specified to either {@code src} or {@code dst}.
     */
    InetMatch(InetAddress src, Short srcsuff, InetAddress dst, Short dstsuff,
              Short proto, Byte dscp) {
        sourceAddress = src;
        sourceSuffix = srcsuff;
        destinationAddress = dst;
        destinationSuffix = dstsuff;
        protocol = proto;
        this.dscp = dscp;

        Class<?> cls = getAddressClass();
        if (src != null && !cls.equals(src.getClass())) {
            String msg = unexpectedAddress(src.toString(), "source", cls);
            throw new IllegalArgumentException(msg);
        }
        if (dst != null && !cls.equals(dst.getClass())) {
            String msg = unexpectedAddress(dst.toString(), "destination", cls);
            throw new IllegalArgumentException(msg);
        }
    }

    /**
     * Return the source IP address to match against packets.
     *
     * @return  An {@link InetAddress} instance which represents the
     *          source IP address to match against packets.
     *          {@code null} is returned if this instance does not describe
     *          the source IP address to match.
     */
    public final InetAddress getSourceAddress() {
        return sourceAddress;
    }

    /**
     * Return the destination IP address to match against packets.
     *
     * @return  An {@link InetAddress} instance which represents the
     *          destination IP address to match against packets.
     *          {@code null} is returned if this instance does not describe
     *          the destination IP address to match.
     */
    public final InetAddress getDestinationAddress() {
        return destinationAddress;
    }

    /**
     * Return the CIDR suffix for the source IP address.
     *
     * @return  A {@link Short} instance which represents the CIDR suffix
     *          for the source IP address.
     *          {@code null} is returned if the CIDR suffix for the source
     *          IP address is not specified.
     */
    public final Short getSourceSuffix() {
        return sourceSuffix;
    }

    /**
     * Return the CIDR suffix for the destination IP address.
     *
     * @return  A {@link Short} instance which represents the CIDR suffix
     *          for the destination IP address.
     *          {@code null} is returned if the CIDR suffix for the destination
     *          IP address is not specified.
     */
    public final Short getDestinationSuffix() {
        return destinationSuffix;
    }

    /**
     * Return the IP protocol type to match against packets.
     *
     * @return  A {@link Short} instance which represents the IP protocol type
     *          to match against packets.
     *          {@code null} is returned if this instance does not describe the
     *          IP protocol type to match.
     */
    public final Short getProtocol() {
        return protocol;
    }

    /**
     * Return the DSCP field value to match against packets.
     *
     * @return  A {@link Byte} instance which represents the DSCP field value
     *          to match against packets.
     *          {@code null} is returned if this instance does not describe the
     *          DSCP field value to match.
     */
    public final Byte getDscp() {
        return dscp;
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
     * Return a string representation of the source IP address to match
     * against packets.
     *
     * <p>
     *   {@code null} is returned if this instance does not describe the
     *   source IP address to match.
     * </p>
     * <p>
     *   Note that below description of return value of this method is
     *   written for REST API document.
     * </p>
     *
     * @return
     *   A string representation of the source IP address to match against
     *   packets.
     *   <ul>
     *     <li>
     *       Current version supports IPv4 only.
     *       So a string representation of an IPv4 address must be specified.
     *     </li>
     *     <li>
     *       If this attribute is omitted, this elements specifies the
     *       condition that matches every source IP address.
     *     </li>
     *   </ul>
     * @deprecated  Only for JAXB.
     *              Use {@link #getSourceAddress()} instead.
     */
    @XmlAttribute(name = "src")
    public final String getSourceInetAddress() {
        return toString(sourceAddress);
    }

    /**
     * Set the source IP address to match against packets.
     *
     * <p>
     *   This method is called by JAXB.
     * </p>
     *
     * @param ip  A string representation of the IP address.
     */
    @SuppressWarnings("unused")
    private void setSourceInetAddress(String ip) {
        sourceAddress = toInetAddress(ip, "source");
    }

    /**
     * Return a string representation of the destination IP address to match
     * against packets.
     *
     * <p>
     *   {@code null} is returned if this instance does not describe the
     *   destination IP address to match.
     * </p>
     * <p>
     *   Note that below description of return value of this method is
     *   written for REST API document.
     * </p>
     *
     * @return
     *   A string representation of the destination IP address to match
     *   against packets.
     *   <ul>
     *     <li>
     *       Current version supports IPv4 only.
     *       So a string representation of an IPv4 address must be specified.
     *     </li>
     *     <li>
     *       If this attribute is omitted, this elements specifies the
     *       condition that matches every destination IP address.
     *     </li>
     *   </ul>
     * @deprecated  Only for JAXB.
     *              Use {@link #getDestinationAddress()} instead.
     */
    @XmlAttribute(name = "dst")
    public final String getDestinationInetAddress() {
        return toString(destinationAddress);
    }

    /**
     * Set the destination IP address to match against packets.
     *
     * <p>
     *   This method is called by JAXB.
     * </p>
     *
     * @param ip  A string representation of the IP address.
     */
    @SuppressWarnings("unused")
    private void setDestinationInetAddress(String ip) {
        destinationAddress = toInetAddress(ip, "destination");
    }

    /**
     * Convert an {@link InetAddress} instance to a string.
     *
     * @param addr  An {@link InetAddress} instance.
     * @return  A string representation of the specified instance.
     *         {@code null} is returned if {@code null} is specified.
     */
    private String toString(InetAddress addr) {
        return (addr == null) ? null : addr.getHostAddress();
    }

    /**
     * Create a string which indicates unexpected IP address is specified.
     *
     * @param ip    A string representation of the IP address.
     * @param desc  A brief description about the IP address.
     * @param cls   A {@link Class} instance associated with the IP address
     *              class.
     * @return  A string which indicates unexpected IP address is specified.
     */
    private String unexpectedAddress(String ip, String desc, Class<?> cls) {
        StringBuilder builder =
            new StringBuilder("Unexpected ");
        builder.append(desc).append(" address type: addr=").append(ip).
            append(", expected=").append(cls.getSimpleName());
        return builder.toString();
    }

    /**
     * Create a string which indicates an invalid IP address is specified.
     *
     * @param ip     A string representation of the IP address.
     * @param desc   A brief description about the IP address.
     * @param cause  A throwable which indicates the cause of error.
     * @return  A string which indicates an invalid IP address is specified.
     */
    private String invalidAddress(String ip, String desc, Throwable cause) {
        return new StringBuilder("Invalid ").
            append(desc).append(" IP address: ").append(ip).
            append(": ").append(cause.getMessage()).
            toString();
    }

    /**
     * Convert the given string into an {@link InetAddress} instance.
     *
     * @param ip    A string representation of the IP address.
     * @param desc  A brief description about the value.
     * @return  An {@link InetAddress} instance.
     */
    private InetAddress toInetAddress(String ip, String desc) {
        if (ip != null) {
            if (ip.length() == 0) {
                StringBuilder builder = new StringBuilder("Empty ");
                builder.append(desc).append(" address.");
                validationStatus =
                    new Status(StatusCode.BADREQUEST, builder.toString());
                return null;
            }

            try {
                InetAddress iaddr = InetAddress.getByName(ip);
                Class<?> cls = getAddressClass();
                if (!cls.equals(iaddr.getClass())) {
                    validationStatus =
                        new Status(StatusCode.BADREQUEST,
                                   unexpectedAddress(ip, desc, cls));
                    iaddr = null;
                }
                return iaddr;
            } catch (Exception e) {
                String msg = invalidAddress(ip, desc, e);
                validationStatus = new Status(StatusCode.BADREQUEST, msg);
            }
        }

        return null;
    }

    /**
     * Determine whether the given match contains the same condition for
     * IP address.
     *
     * @param im  The object to be compared.
     * @return  {@code true} only if the given match contains the same
     *          condition for IP address.
     */
    private boolean equalsAddress(InetMatch im) {
        return (Objects.equals(sourceAddress, im.sourceAddress) &&
                Objects.equals(destinationAddress, im.destinationAddress) &&
                Objects.equals(sourceSuffix, im.sourceSuffix) &&
                Objects.equals(destinationSuffix, im.destinationSuffix));
    }

    /**
     * Return a {@link Class} instance which represents the type of
     * IP address.
     *
     * @return  A {@link Class} instance which represents the type of
     *          IP address.
     */
    public abstract Class<? extends InetAddress> getAddressClass();

    /**
     * Determine whether the given object is identical to this object.
     *
     * @param o  An object to be compared.
     * @return   {@code true} if identical. Otherwise {@code false}.
     */
    @Override
    public final boolean equals(Object o) {
        if (o == this) {
            return true;
        }
        if (o == null || !getClass().equals(o.getClass())) {
            return false;
        }

        InetMatch im = (InetMatch)o;
        return (equalsAddress(im) && Objects.equals(protocol, im.protocol) &&
                Objects.equals(dscp, im.dscp));
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public final int hashCode() {
        int h = getClass().getName().hashCode();
        return h + Objects.hash(sourceAddress, destinationAddress,
                                sourceSuffix, destinationSuffix,
                                protocol, dscp);
    }

    /**
     * Return a string representation of this object.
     *
     * @return  A string representation of this object.
     */
    @Override
    public final String toString() {
        StringBuilder builder = new StringBuilder(getClass().getSimpleName());
        builder.append('[');
        String sep = "";
        if (sourceAddress != null) {
            builder.append("src=").append(sourceAddress.getHostAddress());
            sep = ",";
        }
        if (sourceSuffix != null) {
            builder.append(sep).append("srcsuff=").
                append(sourceSuffix.toString());
            sep = ",";
        }
        if (destinationAddress != null) {
            builder.append(sep)
                .append("dst=").append(destinationAddress.getHostAddress());
            sep = ",";
        }
        if (destinationSuffix != null) {
            builder.append(sep).append("dstsuff=").
                append(destinationSuffix.toString());
            sep = ",";
        }
        if (protocol != null) {
            builder.append(sep).append("proto=").append(protocol.toString());
            sep = ",";
        }
        if (dscp != null) {
            builder.append(sep).append("dscp=").append(dscp.toString());
        }
        builder.append(']');

        return builder.toString();
    }
}
