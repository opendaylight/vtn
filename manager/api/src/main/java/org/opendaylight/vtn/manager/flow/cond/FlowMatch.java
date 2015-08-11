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
import javax.xml.bind.annotation.XmlElement;
import javax.xml.bind.annotation.XmlElements;
import javax.xml.bind.annotation.XmlRootElement;

import com.fasterxml.jackson.databind.annotation.JsonSerialize;

import org.opendaylight.vtn.manager.util.VTNIdentifiable;

/**
 * This class describes the condition to select packets by protocol header
 * fields.
 *
 * <p>
 *   A {@code FlowMatch} instance matches every packet if it is empty.
 * </p>
 *
 * <h4>Example JSON</h4>
 * <pre class="prettyprint lang-json">
 * {
 * &nbsp;&nbsp;"index": 1,
 * &nbsp;&nbsp;"ethernet": {
 * &nbsp;&nbsp;&nbsp;&nbsp;"src": "00:11:22:33:44:55",
 * &nbsp;&nbsp;&nbsp;&nbsp;"dst": "00:aa:bb:cc:dd:ee",
 * &nbsp;&nbsp;&nbsp;&nbsp;"type": 2048
 * &nbsp;&nbsp;},
 * &nbsp;&nbsp;"inetMatch": {
 * &nbsp;&nbsp;&nbsp;&nbsp;"inet4": {
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"src": "192.168.10.1",
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"dst": "192.168.20.2",
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"protocol": 1
 * &nbsp;&nbsp;&nbsp;&nbsp;}
 * &nbsp;&nbsp;},
 * &nbsp;&nbsp;"l4Match": {
 * &nbsp;&nbsp;&nbsp;&nbsp;"icmp": {
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"type": 3,
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"code": 0
 * &nbsp;&nbsp;&nbsp;&nbsp;}
 * &nbsp;&nbsp;}
 * }</pre>
 *
 * @see    FlowMatchBuilder
 * @since  Helium
 */
@JsonSerialize(include = JsonSerialize.Inclusion.NON_NULL)
@XmlRootElement(name = "flowmatch")
@XmlAccessorType(XmlAccessType.NONE)
public final class FlowMatch
    implements Serializable, VTNIdentifiable<Integer> {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = -7259430214355407143L;

    /**
     * This element describes the Ethernet header fields to match against
     * packets.
     *
     * <p>
     *   If omitted, every Ethernet header in packet is matched.
     * </p>
     */
    @XmlElement(name = "ethernet")
    private EthernetMatch  ethernetMatch;

    /**
     * This element describes the IP header fields to match against packets.
     *
     * <ul>
     *   <li>
     *     IP version to match is determined by the element name.
     *     Currently, the element name must be <strong>inet4</strong>
     *     which indicates IPv4 packets, and its type must be
     *     {@link Inet4Match}.
     *   </li>
     *   <li>If omitted, every IP header in packet is matched.</li>
     *   <li>
     *     In JSON notation, this element must be wrapped by
     *     <strong>inetMatch</strong> element.
     *   </li>
     *   <li>
     *     If this element is configured, the VTN Manager will complement
     *     corresponding Ethernet type in <strong>ethernet</strong> element
     *     if it is not configured.
     *     For example, if an <strong>inet4</strong> element is configured in
     *     <strong>flowmatch</strong> element, the VTN Manager behaves
     *     as follows.
     *     <ul>
     *       <li>
     *         If <strong>ethernet</strong> element is missing, the VTN Manager
     *         will add an <strong>ethernet</strong> element which specifies
     *         2048 as Ethernet type.
     *       </li>
     *       <li>
     *         If <strong>ethernet</strong> element does not have
     *         <strong>type</strong> attribute, the VTN manager will add
     *         <strong>type</strong> attribute which specifies 2048 as
     *         Ethernet type.
     *       </li>
     *       <li>
     *         If the Ethernet type configured in <strong>ethernet</strong>
     *         element is not 2048, the VTN Manager treats it as invalid.
     *       </li>
     *     </ul>
     *   </li>
     * </ul>
     */
    @XmlElements({@XmlElement(name = "inet4", type = Inet4Match.class)})
    private InetMatch  inetMatch;

    /**
     * This element describes the layer 4 protocol header fields, such as
     * TCP, to match against packets.
     *
     * <ul>
     *   <li>
     *     Layer 4 protocol to match is determined by the element name.
     *     One of the following elements can be used to specify condition for
     *     layer 4 protocol header fields.
     *     <dl style="margin-left: 1em;">
     *       <dt>tcp
     *       <dd style="margin-left: 1.5em;">
     *         Specifies the condition for TCP header fields.
     *         The type of this element must be {@link TcpMatch}.
     *
     *       <dt>udp
     *       <dd style="margin-left: 1.5em;">
     *         Specifies the condition for UDP header fields.
     *         The type of this element must be {@link UdpMatch}.
     *
     *       <dt>icmp
     *       <dd style="margin-left: 1.5em;">
     *         Specifies the condition for ICMP (for IPv4) header fields.
     *         The type of this element must be {@link IcmpMatch}.
     *     </dl>
     *   </li>
     *   <li>
     *     If omitted, every layer 4 protocol header in packet is matched.
     *   </li>
     *   <li>
     *     In JSON notation, this element must be wrapped by
     *     <strong>l4Match</strong> element.
     *   </li>
     *   <li>
     *     If this element is configured, the VTN Manager will complement
     *     corresponding IP protocol number in <strong>inet4</strong> element
     *     if it is not configured.
     *     For example, if a <strong>tcp</strong> element is configured in
     *     <strong>flowmatch</strong> element, the VTN Manager behaves
     *     as follows.
     *     <ul>
     *       <li>
     *         If <strong>inet4</strong> element is missing, the VTN Manager
     *         will add an <strong>inet4</strong> element which specifies 6 as
     *         IP protocol number.
     *         In this case, the VTN Manager may also add an
     *         <strong>ethernet</strong> element as described in documents for
     *         <strong>inet4</strong> element.
     *       </li>
     *       <li>
     *         If <strong>inet4</strong> element does not have
     *         <strong>protocol</strong> attribute, the VTN manager will add
     *         <strong>protocol</strong> attribute which specifies 6 as
     *         IP protocol number.
     *       </li>
     *       <li>
     *         If the IP protocol number configured in <strong>inet4</strong>
     *         element is not 6, the VTN Manager treats it as invalid.
     *       </li>
     *     </ul>
     *   </li>
     * </ul>
     */
    @XmlElements({
        @XmlElement(name = "tcp", type = TcpMatch.class),
        @XmlElement(name = "udp", type = UdpMatch.class),
        @XmlElement(name = "icmp", type = IcmpMatch.class)})
    private L4Match  l4Match;

    /**
     * An index value assigned to this condition.
     * <ul>
     *   <li>
     *     The range of value that can be specified is from
     *     <strong>1</strong> to <strong>65535</strong>.
     *   </li>
     *   <li>
     *     This value is used to determine order of match evaluation.
     *     Conditions in a <strong>flowcondition</strong> element are
     *     evaluated in ascending order of indices assigned to
     *     <strong>flowmatch</strong> elements.
     *   </li>
     *   <li>
     *     This attribute is omitted if <strong>flowmatch</strong> element
     *     is used to represent match field in a flow entry.
     *   </li>
     * </ul>
     */
    @XmlAttribute(name = "index")
    private Integer  matchIndex;

    /**
     * Private constructor only for JAXB.
     */
    @SuppressWarnings("unused")
    private FlowMatch() {
    }

    /**
     * Construct a new instance without specifying index.
     *
     * <p>
     *   This constructor is used to create a {@code FlowMatch} instance
     *   used to configure flow condition.
     * </p>
     *
     * @param ether  A {@link EthernetMatch} instance which describes the
     *               condition for Ethernet header.
     *               {@code null} means that no condition for Ethernet header
     *               is specified.
     * @param ip     A {@link InetMatch} instance which describes the
     *               condition for IP heder.
     *               {@code null} means that no condition for IP header
     *               is specified.
     * @param l4     A {@link L4Match} instance which describes the condition
     *               for layer 4 protocol header.
     *               {@code null} means that no condition for layer 4 protocol
     *               is specified.
     * @see #FlowMatch(int, EthernetMatch, InetMatch, L4Match)
     */
    public FlowMatch(EthernetMatch ether, InetMatch ip, L4Match l4) {
        ethernetMatch = ether;
        inetMatch = ip;
        l4Match = l4;
    }

    /**
     * Construct a new instance with specifying index.
     *
     * <p>
     *   This constructor is used to create a {@code FlowMatch} instance
     *   used to configure flow condition.
     * </p>
     * <ul>
     *   <li>
     *     If an {@link InetMatch} instance is passed to {@code ip} parameter,
     *     the VTN Manager will complement corresponding Ethernet type if
     *     if it is not specified by an {@link EthernetMatch} instance.
     *     For example, if an {@link Inet4Match} instance is passed to
     *     {@code ip} parameter, the VTN Manager behaves as follows.
     *     <ul>
     *       <li>
     *         If {@code null} is specified to {@code ether} parameter,
     *         or the Ethernet type is not specified in {@link EthernetMatch}
     *         instance, the VTN Manager will add the condition to match
     *         Ethernet type 2048 into the flow match condition.
     *       </li>
     *       <li>
     *         If the Ethernet type configured in {@code ether} is not 2048,
     *         the VTN Manager treats it as invalid.
     *       </li>
     *     </ul>
     *   </li>
     *   <li>
     *     If a {@link L4Match} instance is passed to {@code l4} parameter,
     *     the VTN Manager will complement corresponding IP protocol number
     *     if it is not specified by {@link InetMatch} instance.
     *     For example, if a {@link TcpMatch} instance is passed to {@code l4}
     *     parameter, the VTN Manager behaves as follows.
     *     <ul>
     *       <li>
     *         If {@code null} is specified to {@code ip} parameter, or
     *         IP protocol number is not specified in {@link InetMatch}
     *         instance, the VTN Manager will add the condition to match
     *         IP protocol number 6 into the flow match condition.
     *         In this case, the VTN Manager may also add the condition for
     *         Ethernet type as described above.
     *       </li>
     *       <li>
     *         If the IP protocol number configured in {@code ip} is not 6,
     *         the VTN Manager treats it as invalid.
     *       </li>
     *     </ul>
     *   </li>
     * </ul>
     *
     * @param index  An index value to be assigned to a new instance.
     * @param ether  A {@link EthernetMatch} instance which describes the
     *               condition for Ethernet header.
     *               {@code null} means that no condition for Ethernet header
     *               is specified.
     * @param ip     A {@link InetMatch} instance which describes the
     *               condition for IP heder.
     *               {@code null} means that no condition for IP header
     *               is specified.
     * @param l4     A {@link L4Match} instance which describes the condition
     *               for layer 4 protocol header.
     *               {@code null} means that no condition for layer 4 protocol
     *               is specified.
     */
    public FlowMatch(int index, EthernetMatch ether, InetMatch ip,
                     L4Match l4) {
        matchIndex = Integer.valueOf(index);
        ethernetMatch = ether;
        inetMatch = ip;
        l4Match = l4;
    }

    /**
     * Return an index value assigned to this instance.
     *
     * @return  An {@link Integer} instance which represents an index value
     *          assigned to this instance.
     *          {@code null} is returned if no index is assigned.
     */
    public Integer getIndex() {
        return matchIndex;
    }

    /**
     * Assign a match index to {@code FlowMatch} instance.
     *
     * <p>
     *   If the specified index is already assigned to this instance, this
     *   method returns this instance. Otherwise, this method returns a copy
     *   of this instance with setting the specified index.
     * </p>
     *
     * @param index  A match index to be assigned.
     * @return  A {@code FlowMatch} instance.
     */
    public FlowMatch assignIndex(int index) {
        return (matchIndex != null && matchIndex.intValue() == index)
            ? this
            : new FlowMatch(index, ethernetMatch, inetMatch, l4Match);
    }

    /**
     * Return an {@link EthernetMatch} instance which describes the Ethernet
     * header fields to match against packets.
     *
     * @return  An {@link EthernetMatch} instance is returned if the condition
     *          for the Ethernet header is configured.
     *          {@code null} is returned if not configured.
     */
    public EthernetMatch getEthernetMatch() {
        return ethernetMatch;
    }

    /**
     * Return an {@link InetMatch} instance which describes the IP header
     * fields to match against packets.
     *
     * @return  An {@link InetMatch} instance is returned if the condition
     *          for the IP header is configured.
     *          {@code null} is returned if not configured.
     */
    public InetMatch getInetMatch() {
        return inetMatch;
    }

    /**
     * Return an {@link L4Match} instance which describes the layer 4 protocol
     * header fields to match against packets.
     *
     * @return  An {@link L4Match} instance is returned if the condition
     *          for the layer 4 protocol header is configured.
     *          {@code null} is returned if not configured.
     */
    public L4Match getLayer4Match() {
        return l4Match;
    }

    /**
     * Return IP protocol number configured in this instance.
     *
     * @return  An IP protocol number configured in this instance.
     *          -1 is returned if not configured.
     */
    public int getInetProtocol() {
        if (inetMatch != null) {
            Short proto = inetMatch.getProtocol();
            if (proto != null) {
                return proto.intValue();
            }
        }

        return -1;
    }

    // Objects

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
        if (!(o instanceof FlowMatch)) {
            return false;
        }

        FlowMatch fm = (FlowMatch)o;
        return (Objects.equals(matchIndex, fm.matchIndex) &&
                Objects.equals(ethernetMatch, fm.ethernetMatch) &&
                Objects.equals(inetMatch, fm.inetMatch) &&
                Objects.equals(l4Match, fm.l4Match));
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        return Objects.hash(matchIndex, ethernetMatch, inetMatch, l4Match);
    }

    /**
     * Return a string representation of this object.
     *
     * @return  A string representation of this object.
     */
    @Override
    public String toString() {
        StringBuilder builder = new StringBuilder("FlowMatch[");
        String sep = "";
        if (matchIndex != null) {
            builder.append("index=").append(matchIndex.toString());
            sep = ",";
        }
        if (ethernetMatch != null) {
            builder.append(sep).
                append("ether=").append(ethernetMatch.toString());
            sep = ",";
        }
        if (inetMatch != null) {
            builder.append(sep).append("inet=").append(inetMatch.toString());
            sep = ",";
        }
        if (l4Match != null) {
            builder.append(sep).append("L4=").append(l4Match.toString());
        }
        builder.append(']');

        return builder.toString();
    }

    // VTNIdentifiable

    /**
     * Return the identifier of this instance.
     *
     * <p>
     *   This method returns the index of the flow match.
     * </p>
     *
     * @return  The index of the flow match.
     * @since   Lithium
     */
    @Override
    public Integer getIdentifier() {
        return matchIndex;
    }
}
