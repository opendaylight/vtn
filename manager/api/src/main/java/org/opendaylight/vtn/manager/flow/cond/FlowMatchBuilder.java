/*
 * Copyright (c) 2014, 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.flow.cond;

import java.net.InetAddress;
import java.net.Inet4Address;

import org.opendaylight.vtn.manager.util.EtherAddress;

import org.opendaylight.controller.sal.packet.address.EthernetAddress;

/**
 * This class provides a utility to instantiate {@link FlowMatch} class.
 *
 * <p>
 *   Note that this class is not synchronized.
 *   If a {@code FlowMatchBuilder} instance is accessed on multiple threads
 *   concurrently, it must be synchronized externally.
 * </p>
 *
 * @since  Helium
 */
public final class FlowMatchBuilder {
    /**
     * Source MAC address to be specified to {@link EthernetMatch}.
     */
    private EtherAddress  sourceMacAddress;

    /**
     * Destination MAC address to be specified to {@link EthernetMatch}.
     */
    private EtherAddress  destinationMacAddress;

    /**
     * Ethernet type value to be specified to {@link EthernetMatch}.
     */
    private Integer  etherType;

    /**
     * VLAN ID to be specified to {@link EthernetMatch}.
     */
    private Short  vlanId;

    /**
     * VLAN priority to be specified to {@link EthernetMatch}.
     */
    private Byte  vlanPriority;

    /**
     * Source IP address to be specified to {@link InetMatch}.
     */
    private InetAddress  sourceAddress;

    /**
     * Destination IP address to be specified to {@link InetMatch}.
     */
    private InetAddress  destinationAddress;

    /**
     * CIDR suffix for the source IP address to be specified to
     * {@link InetMatch}.
     */
    private Short  sourceSuffix;

    /**
     * CIDR suffix for the destination IP address to be specified to
     * {@link InetMatch}.
     */
    private Short  destinationSuffix;

    /**
     * IP protocol type to be specified to {@link InetMatch}.
     */
    private Short  inetProtocol;

    /**
     * DSCP field value to be specified to {@link InetMatch}.
     */
    private Byte  inetDscp;

    /**
     * A {@link Class} instance which specifies the class which describes
     * the condition for layer 4 protocol header.
     */
    private Class<? extends L4Match>  l4Class;

    /**
     * A {@link PortMatch} instance which specifies the source port number
     * to be passed to {@link TcpMatch} or {@link UdpMatch}.
     */
    private PortMatch  sourcePort;

    /**
     * A {@link PortMatch} instance which specifies the destination port number
     * to be passed to {@link TcpMatch} or {@link UdpMatch}.
     */
    private PortMatch  destinationPort;

    /**
     * ICMP type value to be specified to {@link IcmpMatch}.
     */
    private Short  icmpType;

    /**
     * ICMP code value to be specified to {@link IcmpMatch}.
     */
    private Short  icmpCode;

    /**
     * Construct a new instance without any condition for {@link FlowMatch}.
     */
    public FlowMatchBuilder() {
    }

    /**
     * Clear all conditions configured in this instance.
     *
     * @return  This instance.
     */
    public FlowMatchBuilder reset() {
        sourceMacAddress = null;
        destinationMacAddress = null;
        etherType = null;
        vlanId = null;
        vlanPriority = null;
        sourceAddress = null;
        destinationAddress = null;
        sourceSuffix = null;
        destinationSuffix = null;
        inetProtocol = null;
        inetDscp = null;
        l4Class = null;
        sourcePort = null;
        destinationPort = null;
        icmpType = null;
        icmpCode = null;

        return this;
    }

    /**
     * Set the source MAC address to match against Ethernet header.
     *
     * @param mac  A {@link EthernetAddress} instance which represents the
     *             MAC address. {@code null} means that every source MAC
     *             address should be matched.
     * @return  This instance.
     */
    public FlowMatchBuilder setSourceMacAddress(EthernetAddress mac) {
        sourceMacAddress = EtherAddress.create(mac);
        return this;
    }

    /**
     * Set the source MAC address to match against Ethernet header.
     *
     * @param mac  A {@link EtherAddress} instance which represents the
     *             MAC address. {@code null} means that every source MAC
     *             address should be matched.
     * @return  This instance.
     * @since   Lithium
     */
    public FlowMatchBuilder setSourceMacAddress(EtherAddress mac) {
        sourceMacAddress = mac;
        return this;
    }

    /**
     * Set the destination MAC address to match against Ethernet header.
     *
     * @param mac  A {@link EthernetAddress} instance which represents the
     *             MAC address. {@code null} means that every destination MAC
     *             address should be matched.
     * @return  This instance.
     */
    public FlowMatchBuilder setDestinationMacAddress(EthernetAddress mac) {
        destinationMacAddress = EtherAddress.create(mac);
        return this;
    }

    /**
     * Set the destination MAC address to match against Ethernet header.
     *
     * @param mac  A {@link EtherAddress} instance which represents the
     *             MAC address. {@code null} means that every destination MAC
     *             address should be matched.
     * @return  This instance.
     * @since   Lithium
     */
    public FlowMatchBuilder setDestinationMacAddress(EtherAddress mac) {
        destinationMacAddress = mac;
        return this;
    }

    /**
     * Set the Ethernet type value to match against Ethernet header.
     *
     * @param type
     *   An Ethernet type to match against packets.
     *   <ul>
     *     <li>
     *       The range of value that can be specified is from
     *       <strong>0</strong> to <strong>65535</strong>.
     *     </li>
     *     <li>
     *       {@code null} means that every Ethernet type should be matched.
     *     </li>
     *   </ul>
     * @return  This instance.
     * @since   Lithium
     */
    public FlowMatchBuilder setEtherType(Integer type) {
        etherType = type;
        return this;
    }

    /**
     * Set the VLAN ID to match against Ethernet header.
     *
     * @param vlan
     *   A VLAN ID to match against packets.
     *   <ul>
     *     <li>
     *       The range of value that can be specified is from
     *       <strong>0</strong> to <strong>4095</strong>.
     *     </li>
     *     <li>
     *       <strong>0</strong> implies untagged Ethernet frame.
     *     </li>
     *     <li>
     *       {@code null} means that every VLAN ID, including untagged,
     *       should be matched.
     *     </li>
     *   </ul>
     * @return  This instance.
     * @since   Lithium
     */
    public FlowMatchBuilder setVlanId(Short vlan) {
        vlanId = vlan;
        return this;
    }

    /**
     * Set the VLAN priority to match against Ethernet header.
     *
     * @param pri
     *   A VLAN priority to match against packets.
     *   <ul>
     *     <li>
     *       The range of value that can be specified is from
     *       <strong>0</strong> to <strong>7</strong>.
     *     </li>
     *     <li>
     *       If a valid priority is specified, a valid VLAN ID except for
     *       zero must be configured by the call of {@link #setVlanId(Short)}.
     *       {@code pri}.
     *     </li>
     *     <li>
     *       {@code null} means that every VLAN priority should be matched.
     *     </li>
     *   </ul>
     * @return  This instance.
     * @since   Lithium
     */
    public FlowMatchBuilder setVlanPriority(Byte pri) {
        vlanPriority = pri;
        return this;
    }

    /**
     * Set the source IP address to match against IP header.
     *
     * @param iaddr
     *   An {@link InetAddress} instance which represents the IP address.
     *   <ul>
     *     <li>
     *       Current version supports IPv4 only.
     *       So an {@link Inet4Address} instance must be specified.
     *     </li>
     *     <li>
     *       {@code null} means that every source IP address should be matched.
     *     </li>
     *   </ul>
     * @return  This instance.
     * @throws IllegalArgumentException
     *    An invalid IP address is passed to {@code iaddr}.
     */
    public FlowMatchBuilder setSourceInetAddress(InetAddress iaddr) {
        checkInetAddress(iaddr);
        sourceAddress = iaddr;
        return this;
    }

    /**
     * Set the destination IP address to match against IP header.
     *
     * @param iaddr
     *   An {@link InetAddress} instance which represents the IP address.
     *   <ul>
     *     <li>
     *       Current version supports IPv4 only.
     *       So an {@link Inet4Address} instance must be specified.
     *     </li>
     *     <li>
     *       {@code null} means that every destination IP address should be
     *        matched.
     *     </li>
     *   </ul>
     * @return  This instance.
     * @throws IllegalArgumentException
     *    An invalid IP address is passed to {@code iaddr}.
     */
    public FlowMatchBuilder setDestinationInetAddress(InetAddress iaddr) {
        checkInetAddress(iaddr);
        destinationAddress = iaddr;
        return this;
    }

    /**
     * Set the CIDR suffix for the source IP address matching.
     *
     * @param suffix
     *   A CIDR suffix for the source IP address matching.
     *   <ul>
     *     <li>
     *       The specified value is ignored unless a valid source IP address
     *       is configured by the call of
     *       {@link #setSourceInetAddress(InetAddress)}.
     *     </li>
     *     <li>
     *       If a valid CIDR suffix is specified, only bits in the source IP
     *       address specified by the CIDR suffix are used for matching.
     *       <ul>
     *         <li>
     *           The valid range of the CIDR suffix for IPv4 address is
     *           from <strong>1</strong> to <strong>31</strong>.
     *         </li>
     *       </ul>
     *     </li>
     *     <li>
     *       {@code null} means that all bits in the source IP address
     *       specified by {@link #setSourceInetAddress(InetAddress)} should
     *       be matched.
     *     </li>
     *   </ul>
     * @return  This instance.
     * @since   Lithium
     */
    public FlowMatchBuilder setSourceInetSuffix(Short suffix) {
        sourceSuffix = suffix;
        return this;
    }

    /**
     * Set the CIDR suffix for the destination IP address matching.
     *
     * @param suffix
     *   A CIDR suffix for the destination IP address matching.
     *   <ul>
     *     <li>
     *       The specified value is ignored unless a valid destination
     *       IP address is configured by the call of
     *       {@link #setDestinationInetAddress(InetAddress)}.
     *     </li>
     *     <li>
     *       If a valid CIDR suffix is specified, only bits in the destination
     *       IP address specified by the CIDR suffix are used for matching.
     *       <ul>
     *         <li>
     *           The valid range of the CIDR suffix for IPv4 address is
     *           from <strong>1</strong> to <strong>31</strong>.
     *         </li>
     *       </ul>
     *     </li>
     *     <li>
     *       {@code null} means that all bits in the destination IP address
     *       specified by {@link #setDestinationInetAddress(InetAddress)}
     *       should be matched.
     *     </li>
     *   </ul>
     * @return  This instance.
     * @since   Lithium
     */
    public FlowMatchBuilder setDestinationInetSuffix(Short suffix) {
        destinationSuffix = suffix;
        return this;
    }

    /**
     * Set the IP protocol type to match against IP header.
     *
     * @param proto
     *   An IP protocol type value to match against packets.
     *   <ul>
     *     <li>
     *       The range of value that can be specified is from
     *       <strong>0</strong> to <strong>255</strong>.
     *     </li>
     *     <li>
     *       {@code null} means that every IP protocol should be matched.
     *     </li>
     *   </ul>
     * @return  This instance.
     * @since   Lithium
     */
    public FlowMatchBuilder setInetProtocol(Short proto) {
        inetProtocol = proto;
        return this;
    }

    /**
     * Set the DSCP field value to match against IP header.
     *
     * @param dscp
     *   A DSCP field value to match against packets.
     *   <ul>
     *     <li>
     *       The range of value that can be specified is from
     *       <strong>0</strong> to <strong>63</strong>.
     *     </li>
     *     <li>
     *       {@code null} means that every DSCP value should be matched.
     *     </li>
     *   </ul>
     * @return  This instance.
     * @since   Lithium
     */
    public FlowMatchBuilder setInetDscp(Byte dscp) {
        inetDscp = dscp;
        return this;
    }

    /**
     * Set the TCP source port number to match against TCP header.
     *
     * <p>
     *   Previously configured conditions for layer 4 protocol other than TCP
     *   will be reset to initial state.
     * </p>
     *
     * @param port
     *   The TCP source port number to match against TCP header.
     *   <ul>
     *     <li>
     *       The range of value that can be specified is from
     *       <strong>0</strong> to <strong>65535</strong>.
     *     </li>
     *     <li>
     *       {@code null} means that every TCP source port should be matched.
     *     </li>
     *   </ul>
     * @return  This instance.
     * @since   Lithium
     */
    public FlowMatchBuilder setTcpSourcePort(Integer port) {
        resetL4Conditions(TcpMatch.class);
        sourcePort = (port == null) ? null : new PortMatch(port);
        return this;
    }

    /**
     * Set the range of TCP source port numbers to match against TCP header.
     *
     * <p>
     *   Previously configured conditions for layer 4 protocol other than TCP
     *   will be reset to initial state.
     * </p>
     *
     * @param from
     *   The minumum value (inclusive) in the range of TCP source port numbers
     *   to match against packets.
     *   <ul>
     *     <li>
     *       The range of value that can be specified is from
     *       <strong>0</strong> to <strong>65535</strong>.
     *     </li>
     *   </ul>
     * @param to
     *   The maximum value (inclusive) in the range of TCP source port numbers
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
     *   </ul>
     * @return  This instance.
     */
    public FlowMatchBuilder setTcpSourcePort(int from, int to) {
        resetL4Conditions(TcpMatch.class);
        sourcePort = new PortMatch(Integer.valueOf(from), Integer.valueOf(to));
        return this;
    }

    /**
     * Set the TCP destination port number to match against TCP header.
     *
     * <p>
     *   Previously configured conditions for layer 4 protocol other than TCP
     *   will be reset to initial state.
     * </p>
     *
     * @param port
     *   The TCP destination port number to match against TCP header.
     *   <ul>
     *     <li>
     *       The range of value that can be specified is from
     *       <strong>0</strong> to <strong>65535</strong>.
     *     </li>
     *     <li>
     *       {@code null} means that every TCP destination port should be
     *       matched.
     *     </li>
     *   </ul>
     * @return  This instance.
     * @since   Lithium
     */
    public FlowMatchBuilder setTcpDestinationPort(Integer port) {
        resetL4Conditions(TcpMatch.class);
        destinationPort = (port == null) ? null : new PortMatch(port);
        return this;
    }

    /**
     * Set the range of TCP destination port numbers to match against TCP
     * header.
     *
     * <p>
     *   Previously configured conditions for layer 4 protocol other than TCP
     *   will be reset to initial state.
     * </p>
     *
     * @param from
     *   The minumum value (inclusive) in the range of TCP destination
     *   port numbers to match against packets.
     *   <ul>
     *     <li>
     *       The range of value that can be specified is from
     *       <strong>0</strong> to <strong>65535</strong>.
     *     </li>
     *   </ul>
     * @param to
     *   The maximum value (inclusive) in the range of TCP destination
     *   port numbers to match against packets.
     *   <ul>
     *     <li>
     *       The range of value that can be specified is from
     *       <strong>0</strong> to <strong>65535</strong>.
     *     </li>
     *     <li>
     *       The value must be greater than or equal to the value passed to
     *       {@code from}.
     *     </li>
     *   </ul>
     * @return  This instance.
     */
    public FlowMatchBuilder setTcpDestinationPort(int from, int to) {
        resetL4Conditions(TcpMatch.class);
        destinationPort =
            new PortMatch(Integer.valueOf(from), Integer.valueOf(to));
        return this;
    }

    /**
     * Set the UDP source port number to match against UDP header.
     *
     * <p>
     *   Previously configured conditions for layer 4 protocol other than UDP
     *   will be reset to initial state.
     * </p>
     *
     * @param port
     *   The UDP source port number to match against UDP header.
     *   <ul>
     *     <li>
     *       The range of value that can be specified is from
     *       <strong>0</strong> to <strong>65535</strong>.
     *     </li>
     *     <li>
     *       {@code null} means that every UDP source port should be matched.
     *     </li>
     *   </ul>
     * @return  This instance.
     * @since   Lithium
     */
    public FlowMatchBuilder setUdpSourcePort(Integer port) {
        resetL4Conditions(UdpMatch.class);
        sourcePort = (port == null) ? null : new PortMatch(port);
        return this;
    }

    /**
     * Set the range of UDP source port numbers to match against UDP header.
     *
     * <p>
     *   Previously configured conditions for layer 4 protocol other than UDP
     *   will be reset to initial state.
     * </p>
     *
     * @param from
     *   The minumum value (inclusive) in the range of UDP source port numbers
     *   to match against packets.
     *   <ul>
     *     <li>
     *       The range of value that can be specified is from
     *       <strong>0</strong> to <strong>65535</strong>.
     *     </li>
     *   </ul>
     * @param to
     *   The maximum value (inclusive) in the range of UDP source port numbers
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
     *   </ul>
     * @return  This instance.
     */
    public FlowMatchBuilder setUdpSourcePort(int from, int to) {
        resetL4Conditions(UdpMatch.class);
        sourcePort = new PortMatch(Integer.valueOf(from), Integer.valueOf(to));
        return this;
    }

    /**
     * Set the UDP destination port number to match against UDP header.
     *
     * <p>
     *   Previously configured conditions for layer 4 protocol other than UDP
     *   will be reset to initial state.
     * </p>
     *
     * @param port
     *   The UDP destination port number to match against UDP header.
     *   <ul>
     *     <li>
     *       The range of value that can be specified is from
     *       <strong>0</strong> to <strong>65535</strong>.
     *     </li>
     *     <li>
     *       {@code null} means that every UDP destination port should be
     *       matched.
     *     </li>
     *   </ul>
     * @return  This instance.
     * @since   Lithium
     */
    public FlowMatchBuilder setUdpDestinationPort(Integer port) {
        resetL4Conditions(UdpMatch.class);
        destinationPort = (port == null) ? null : new PortMatch(port);
        return this;
    }

    /**
     * Set the range of UDP destination port numbers to match against UDP
     * header.
     *
     * <p>
     *   Previously configured conditions for layer 4 protocol other than UDP
     *   will be reset to initial state.
     * </p>
     *
     * @param from
     *   The minumum value (inclusive) in the range of UDP destination
     *   port numbers to match against packets.
     *   <ul>
     *     <li>
     *       The range of value that can be specified is from
     *       <strong>0</strong> to <strong>65535</strong>.
     *     </li>
     *   </ul>
     * @param to
     *   The maximum value (inclusive) in the range of UDP destination
     *   port numbers to match against packets.
     *   <ul>
     *     <li>
     *       The range of value that can be specified is from
     *       <strong>0</strong> to <strong>65535</strong>.
     *     </li>
     *     <li>
     *       The value must be greater than or equal to the value passed to
     *       {@code from}.
     *     </li>
     *   </ul>
     * @return  This instance.
     */
    public FlowMatchBuilder setUdpDestinationPort(int from, int to) {
        resetL4Conditions(UdpMatch.class);
        destinationPort =
            new PortMatch(Integer.valueOf(from), Integer.valueOf(to));
        return this;
    }

    /**
     * Set the ICMP type for IPv4 to match against ICMP header.
     *
     * <p>
     *   Previously configured conditions for layer 4 protocol other than ICMP
     *   will be reset to initial state.
     * </p>
     *
     * @param type
     *   An ICMP type value to match against packets.
     *   <ul>
     *     <li>
     *       The range of value that can be specified is from
     *       <strong>0</strong> to <strong>255</strong>.
     *     </li>
     *     <li>
     *       {@code null} means that every ICMP type should be matched.
     *     </li>
     *   </ul>
     * @return  This instance.
     * @since   Lithium
     */
    public FlowMatchBuilder setIcmpType(Short type) {
        resetL4Conditions(IcmpMatch.class);
        icmpType = type;
        return this;
    }

    /**
     * Set the ICMP code for IPv4 to match against ICMP header.
     *
     * <p>
     *   Previously configured conditions for layer 4 protocol other than ICMP
     *   will be reset to initial state.
     * </p>
     *
     * @param code
     *   An ICMP code value to match against packets.
     *   <ul>
     *     <li>
     *       The range of value that can be specified is from
     *       <strong>0</strong> to <strong>255</strong>.
     *     </li>
     *     <li>
     *       {@code null} means that every ICMP code should be matched.
     *     </li>
     *   </ul>
     * @return  This instance.
     * @since   Lithium
     */
    public FlowMatchBuilder setIcmpCode(Short code) {
        resetL4Conditions(IcmpMatch.class);
        icmpCode = code;
        return this;
    }

    /**
     * Construct a new {@link FlowMatch} instance without specifying index.
     *
     * @return  A {@link FlowMatch} instance.
     * @since   Lithium
     */
    public FlowMatch build() {
        EthernetMatch em = buildEthernetMatch();
        InetMatch im = buildInetMatch();
        L4Match l4m = buildL4Match();

        return new FlowMatch(em, im, l4m);
    }

    /**
     * Construct a new {@link FlowMatch} instance specifying conditions
     * configured in this instance.
     *
     * @param index  An index value to be assigned to a new {@link FlowMatch}
     *               instance.
     * @return  A {@link FlowMatch} instance.
     */
    public FlowMatch build(int index) {
        EthernetMatch em = buildEthernetMatch();
        InetMatch im = buildInetMatch();
        L4Match l4m = buildL4Match();

        return new FlowMatch(index, em, im, l4m);
    }

    /**
     * Validate the specified IP address.
     *
     * @param iaddr  An {@link InetAddress} instance to be validated.
     * @throws IllegalArgumentException
     *   Invalid IP address is configured.
     */
    private void checkInetAddress(InetAddress iaddr) {
        // Currently only IPv4 is supported.
        if (iaddr != null && !(iaddr instanceof Inet4Address)) {
            StringBuilder builder =
                new StringBuilder("Unsupported IP address type: ");
            builder.append(iaddr.toString());
            throw new IllegalArgumentException(builder.toString());
        }
    }

    /**
     * Reset conditions for layer 4 protocol if the target layer 4 protocol
     * is changed.
     *
     * @param cls  A class of {@link L4Match} which specifies the target
     *             layer 4 protocol.
     */
    private void resetL4Conditions(Class<? extends L4Match> cls) {
        if (!cls.equals(l4Class)) {
            if (l4Class != null) {
                sourcePort = null;
                destinationPort = null;
                icmpType = null;
                icmpCode = null;
            }
            l4Class = cls;
        }
    }

    /**
     * Build a new {@link EthernetMatch} instance which specifies Ethernet
     * protocol conditions configured in this instance.
     *
     * @return  A {@link EthernetMatch} instance.
     *          {@code null} is returned if no condition for Ethernet protocol
     *          is configured.
     */
    private EthernetMatch buildEthernetMatch() {
        boolean hasAddr = (sourceMacAddress != null ||
                           destinationMacAddress != null);
        boolean hasVlan = (vlanId != null || vlanPriority != null);

        return (hasAddr || hasVlan || etherType != null)
            ? new EthernetMatch(sourceMacAddress, destinationMacAddress,
                                etherType, vlanId, vlanPriority)
            : null;
    }

    /**
     * Build a new {@link InetMatch} instance which specifies IP protocol
     * conditions configured in this instance.
     *
     * @return  A {@link InetMatch} instance.
     *          {@code null} is returned if no condition for IP protocol
     *          is configured.
     */
    private InetMatch buildInetMatch() {
        boolean hasSrc = (sourceAddress != null || sourceSuffix != null);
        boolean hasDst = (destinationAddress != null ||
                          destinationSuffix != null);
        boolean hasMisc = (inetProtocol != null || inetDscp != null);

        return (hasSrc || hasDst || hasMisc)
            ? new Inet4Match(sourceAddress, sourceSuffix,
                             destinationAddress, destinationSuffix,
                             inetProtocol, inetDscp)
            : null;
    }

    /**
     * Build a new {@link L4Match} instance which specifies conditions
     * configured in this instance.
     *
     * @return  A {@link L4Match} instance.
     *          {@code null} is returned if no condition for layer 4 protocol
     *          is configured.
     */
    private L4Match buildL4Match() {
        L4Match l4m;
        if (l4Class == null) {
            l4m = null;
        } else if (l4Class.equals(TcpMatch.class)) {
            l4m = buildTcpMatch();
        } else if (l4Class.equals(UdpMatch.class)) {
            l4m = buildUdpMatch();
        } else if (l4Class.equals(IcmpMatch.class)) {
            l4m = buildIcmpMatch();
        } else {
            // This should never happen.
            throw new IllegalStateException("Unexpected L4Match class: " +
                                            l4Class);
        }

        return l4m;
    }

    /**
     * Build a new {@link TcpMatch} instance which specifies conditions
     * configured in this instance.
     *
     * @return  A {@link TcpMatch} instance.
     *          {@code null} is returned if no condition for TCP is configured.
     */
    private TcpMatch buildTcpMatch() {
        return (sourcePort == null && destinationPort == null)
            ? null : new TcpMatch(sourcePort, destinationPort);
    }

    /**
     * Build a new {@link UdpMatch} instance which specifies conditions
     * configured in this instance.
     *
     * @return  An {@link UdpMatch} instance.
     *          {@code null} is returned if no condition for UDP is configured.
     */
    private UdpMatch buildUdpMatch() {
        return (sourcePort == null && destinationPort == null)
            ? null : new UdpMatch(sourcePort, destinationPort);
    }

    /**
     * Build a new {@link IcmpMatch} instance which specifies conditions
     * configured in this instance.
     *
     * @return  An {@link IcmpMatch} instance.
     *          {@code null} is returned if no condition for ICMP is
     *          configured.
     */
    private IcmpMatch buildIcmpMatch() {
        return (icmpType == null && icmpCode == null)
            ? null : new IcmpMatch(icmpType, icmpCode);
    }
}
