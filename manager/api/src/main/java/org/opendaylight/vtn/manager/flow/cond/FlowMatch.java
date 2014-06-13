/*
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.flow.cond;

import java.net.InetAddress;
import java.io.Serializable;
import java.util.Objects;

import javax.xml.bind.annotation.XmlAccessType;
import javax.xml.bind.annotation.XmlAccessorType;
import javax.xml.bind.annotation.XmlAttribute;
import javax.xml.bind.annotation.XmlElement;
import javax.xml.bind.annotation.XmlElements;
import javax.xml.bind.annotation.XmlRootElement;

import com.fasterxml.jackson.databind.annotation.JsonSerialize;

import org.opendaylight.controller.sal.match.Match;
import org.opendaylight.controller.sal.match.MatchField;
import org.opendaylight.controller.sal.match.MatchType;
import org.opendaylight.controller.sal.packet.address.EthernetAddress;
import org.opendaylight.controller.sal.utils.IPProtocols;
import org.opendaylight.controller.sal.utils.NetUtils;

/**
 * This class describes the condition to select packets by protocol header
 * fields.
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
public final class FlowMatch implements Serializable {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = 5480507922005106178L;

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
     * </ul>
     */
    @XmlElements({
        @XmlElement(name = "tcp", type = TcpMatch.class),
        @XmlElement(name = "udp", type = UdpMatch.class),
        @XmlElement(name = "icmp", type = IcmpMatch.class)
    })
    private L4Match  l4Match;

    /**
     * An index value assigned to this condition.
     * <ul>
     *   <li>
     *     The range of value that can be specified is from
     *     <strong>0</strong> to <strong>4095</strong>.
     *   </li>
     *   <li>
     *     This value is used to determine order of match evaluation.
     *     Conditions in a <strong>flowcondition</strong> element is
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
    private FlowMatch() {
    }

    /**
     * Construct a new instance which represents the specified SAL match.
     *
     * <p>
     *   This constructor does not assign match index to a new instance.
     * </p>
     *
     * @param match  A SAL match. If {@code null} is specified, an instance
     *               which matches every packet is created.
     * @throws IllegalArgumentException
     *    Unexpected value is configured in {@code match}.
     * @throws ClassCastException
     *    Unexcepted instance of class is configured in {@code match}.
     */
    public FlowMatch(Match match) {
        if (match == null) {
            return;
        }

        EthernetAddress dlsrc = null;
        EthernetAddress dldst = null;
        Integer dltype = null;
        Short vlanId = null;
        Byte vlanPri = null;
        InetAddress nwsrc = null;
        Short nwsrcsuff = null;
        InetAddress nwdst = null;
        Short nwdstsuff = null;
        Short nwproto = null;
        Byte nwtos = null;
        Short tpsrc = null;
        Short tpdst = null;

        MatchField field = match.getField(MatchType.DL_SRC);
        if (field != null) {
            dlsrc = getEthernetAddress(field);
        }

        field = match.getField(MatchType.DL_DST);
        if (field != null) {
            dldst = getEthernetAddress(field);
        }

        field = match.getField(MatchType.DL_VLAN);
        if (field != null) {
            vlanId = (Short)field.getValue();
        }

        field = match.getField(MatchType.DL_VLAN_PR);
        if (field != null) {
            vlanPri = (Byte)field.getValue();
        }

        field = match.getField(MatchType.DL_TYPE);
        if (field != null) {
            short sval = ((Short)field.getValue()).shortValue();
            dltype = Integer.valueOf(NetUtils.getUnsignedShort(sval));
        }

        field = match.getField(MatchType.NW_TOS);
        if (field != null) {
            nwtos = (Byte)field.getValue();
        }

        field = match.getField(MatchType.NW_PROTO);
        if (field != null) {
            byte bval = ((Byte)field.getValue()).byteValue();
            int ival = NetUtils.getUnsignedByte(bval);
            nwproto = Short.valueOf((short)ival);
        }

        field = match.getField(MatchType.NW_SRC);
        if (field != null) {
            nwsrc = (InetAddress)field.getValue();
            InetAddress mask = (InetAddress)field.getMask();
            if (mask != null) {
                int len = NetUtils.getSubnetMaskLength(mask);
                nwsrcsuff = Short.valueOf((short)len);
            }
        }

        field = match.getField(MatchType.NW_DST);
        if (field != null) {
            nwdst = (InetAddress)field.getValue();
            InetAddress mask = (InetAddress)field.getMask();
            if (mask != null) {
                int len = NetUtils.getSubnetMaskLength(mask);
                nwdstsuff = Short.valueOf((short)len);
            }
        }

        field = match.getField(MatchType.TP_SRC);
        if (field != null) {
            tpsrc = (Short)field.getValue();
        }

        field = match.getField(MatchType.TP_DST);
        if (field != null) {
            tpdst = (Short)field.getValue();
        }

        if (dlsrc != null || dldst != null || dltype != null ||
            vlanId != null || vlanPri != null) {
            ethernetMatch = new EthernetMatch(dlsrc, dldst, dltype, vlanId,
                                              vlanPri);
        }

        if (nwsrc != null || nwdst != null || nwproto != null ||
            nwtos != null) {
            inetMatch = new Inet4Match(nwsrc, nwsrcsuff, nwdst, nwdstsuff,
                                       nwproto, nwtos);
        }

        if (tpsrc != null || tpdst != null) {
            // Layer 4 match requires NW_PROTO match.
            if (nwproto == null) {
                throw new IllegalArgumentException
                    ("L4 match without NW_PROTO: " + tpsrc + ", " + tpdst);
            }

            int proto = nwproto.intValue();
            if (IPProtocols.TCP.intValue() == proto) {
                l4Match = new TcpMatch(tpsrc, tpdst);
            } else if (IPProtocols.UDP.intValue() == proto) {
                l4Match = new UdpMatch(tpsrc, tpdst);
            } else if (IPProtocols.ICMP.intValue() == proto) {
                l4Match = new IcmpMatch(tpsrc, tpdst);
            } else {
                throw new IllegalArgumentException
                    ("Unexpected IP protocol: " + nwproto);
            }
        }
    }

    /**
     * Construct a new instance without specifying index.
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
     */
    public FlowMatch(EthernetMatch ether, InetMatch ip, L4Match l4) {
        ethernetMatch = ether;
        inetMatch = ip;
        l4Match = l4;
    }

    /**
     * Construct a new instance with specifying index.
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
        matchIndex = index;
        ethernetMatch = ether;
        inetMatch = ip;
        l4Match = l4;
    }

    /**
     * Return an index value assigned to this instance.
     *
     * @return  An {@link Integer} instance which represents an  index value
     *          assigned to this instance.
     *          {@code null} is returned if no index is assigned.
     */
    public Integer getIndex() {
        return matchIndex;
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
     * Determine whether this instance matches ICMP packet or not.
     *
     * @return  {@code true} is returned if this instance matches ICMP packet.
     *          Otherwise {@code false} is returned.
     */
    public boolean isIcmp() {
        if (inetMatch != null) {
            Short proto = inetMatch.getProtocol();
            return (proto != null &&
                    proto.intValue() == IPProtocols.ICMP.intValue());
        }

        return false;
    }

    /**
     * Return a value configured in the specified SAL match field as an
     * {@link EthernetAddress} instance.
     *
     * @param field  A SAL match field.
     * @return  A {@link EthernetAddress} instance.
     * @throws IllegalArgumentException
     *    Unexpected match value is configured in {@code match}.
     */
    private EthernetAddress getEthernetAddress(MatchField field) {
        try {
            byte[] mac = (byte[])field.getValue();
            return new EthernetAddress(mac);
        } catch (Exception e) {
            throw new IllegalArgumentException("Failed to get MAC address", e);
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
            builder.append(sep).append(",inet=").append(inetMatch.toString());
            sep = ",";
        }
        if (l4Match != null) {
            builder.append(sep).append(",l4=").append(l4Match.toString());
        }
        builder.append(']');

        return builder.toString();
    }
}
