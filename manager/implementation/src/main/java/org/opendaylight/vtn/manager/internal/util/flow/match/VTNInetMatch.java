/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.flow.match;

import java.net.InetAddress;
import java.util.Objects;

import javax.xml.bind.annotation.XmlAccessType;
import javax.xml.bind.annotation.XmlAccessorType;
import javax.xml.bind.annotation.XmlElement;
import javax.xml.bind.annotation.XmlElements;
import javax.xml.bind.annotation.XmlRootElement;
import javax.xml.bind.annotation.XmlSeeAlso;

import org.opendaylight.vtn.manager.flow.cond.Inet4Match;
import org.opendaylight.vtn.manager.flow.cond.InetMatch;
import org.opendaylight.vtn.manager.util.Ip4Network;
import org.opendaylight.vtn.manager.util.IpNetwork;
import org.opendaylight.vtn.manager.util.NumberUtils;

import org.opendaylight.vtn.manager.internal.util.MiscUtils;
import org.opendaylight.vtn.manager.internal.util.ProtocolUtils;
import org.opendaylight.vtn.manager.internal.util.packet.InetHeader;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcErrorTag;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;

import org.opendaylight.controller.sal.utils.Status;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.VtnInetMatchFields;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.vtn.match.fields.VtnInetMatch;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.vtn.match.fields.VtnInetMatchBuilder;

import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.rev131026.flow.MatchBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.model.match.types.rev131026.IpMatchFields;
import org.opendaylight.yang.gen.v1.urn.opendaylight.model.match.types.rev131026.Ipv4MatchFields;
import org.opendaylight.yang.gen.v1.urn.opendaylight.model.match.types.rev131026.Match;
import org.opendaylight.yang.gen.v1.urn.opendaylight.model.match.types.rev131026.match.IpMatchBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.model.match.types.rev131026.match.Layer3Match;

import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.inet.types.rev100924.Dscp;
import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.inet.types.rev100924.IpPrefix;
import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.inet.types.rev100924.IpVersion;

/**
 * {@code VTNInetMatch} describes the condition for IP header to match against
 * packets.
 *
 * <p>
 *   Note that this class is not synchronized.
 * </p>
 */
@XmlRootElement(name = "vtn-inet-match")
@XmlAccessorType(XmlAccessType.NONE)
@XmlSeeAlso(VTNInet4Match.class)
public abstract class VTNInetMatch {
    /**
     * The source IP network to match.
     */
    @XmlElements({
        @XmlElement(name = "source-network-v4", type = Ip4Network.class)})
    private IpNetwork  sourceNetwork;

    /**
     * The destination IP network to match.
     */
    @XmlElements({
        @XmlElement(name = "destination-network-v4", type = Ip4Network.class)})
    private IpNetwork  destinationNetwork;

    /**
     * An IP protocol number to match.
     */
    @XmlElement
    private Short  protocol;

    /**
     * A DSCP field value to match.
     */
    @XmlElement
    private Short  dscp;

    /**
     * Construct a new instance from the given {@link InetMatch} instance.
     *
     * @param imatch  An {@link InetMatch} instance.
     * @return  A {@link VTNInetMatch} instance which represents the given
     *          {@link InetMatch} instance. Note that {@code null} is returned
     *          if {@code imatch} is {@code null}.
     * @throws RpcException
     *    An invalid instance is specified to {@code imatch}.
     */
    public static final VTNInetMatch create(InetMatch imatch)
        throws RpcException {
        if (imatch == null) {
            return null;
        }
        if (imatch instanceof Inet4Match) {
            return new VTNInet4Match((Inet4Match)imatch);
        }

        // This should never hanppen.
        String msg = "Unexpected inet match instance: " + imatch;
        throw RpcException.getBadArgumentException(msg);
    }

    /**
     * Construct a new instance from the given pair of {@link VTNEtherMatch}
     * and {@link VtnInetMatch} instances.
     *
     * @param eth  A {@link VTNEtherMatch} instance.
     * @param ip   A {@link VtnInetMatch} instance.
     * @return  A {@link VTNInetMatch} instance or {@code null}.
     * @throws RpcException
     *    The given condition is invalid.
     */
    public static final VTNInetMatch create(VTNEtherMatch eth, VtnInetMatch ip)
        throws RpcException {
        // Currently only IPv4 is supported.
        return (ip == null) ? null : new VTNInet4Match(ip);
    }

    /**
     * Create a new instance from the given MD-SAL flow match.
     *
     * @param match  A MD-SAL flow match.
     * @return  A {@link VTNInetMatch} instance or {@code null}.
     * @throws NullPointerException
     *    {@code match} is {@code null}.
     * @throws RpcException
     *    {@code match} contains invalid condition against IP header.
     */
    public static VTNInetMatch create(Match match) throws RpcException {
        IpMatchFields ip = match.getIpMatch();
        Layer3Match l3 = match.getLayer3Match();
        if (l3 == null) {
            if (ip == null) {
                return null;
            }
        } else if (!(l3 instanceof Ipv4MatchFields)) {
            String msg = "Unsupported layer 3 match: " + l3;
            throw RpcException.getBadArgumentException(msg);
        }

        return new VTNInet4Match(ip, (Ipv4MatchFields)l3);
    }

    /**
     * Construct a new instance that matches every IP packet.
     */
    VTNInetMatch() {
    }

    /**
     * Construct a new instance that matches the given IP protocol.
     *
     * @param proto  A {@link Short} instance which represents the IP protocol
     *               number to match against packets.
     *               {@code null} matches every IP protocol.
     */
    VTNInetMatch(Short proto) {
        protocol = proto;
    }

    /**
     * Construct a new instance.
     *
     * @param src    A {@link IpNetwork} instance which specifies the source
     *               IP address to match.
     *               {@code null} matches every source IP address.
     * @param dst    A {@link IpNetwork} instance which specifies the
     *               destination IP address to match.
     *               {@code null} matches every destination IP address.
     * @param proto  A {@link Short} instance which represents the IP protocol
     *               number to match against packets.
     *               {@code null} matches every IP protocol.
     * @param d      A DSCP field value to match.
     *               {@code null} matches every DSCP field value.
     * @throws RpcException
     *    The specified condition is invalid.
     */
    VTNInetMatch(IpNetwork src, IpNetwork dst, Short proto, Short d)
        throws RpcException {
        sourceNetwork = src;
        destinationNetwork = dst;
        protocol = proto;
        dscp = d;
        verify();
    }

    /**
     * Construct a new instance from the given {@link InetMatch} instance.
     *
     * @param imatch  An {@link InetMatch} instance.
     * @throws NullPointerException
     *    {@code imatch} is {@code null}.
     * @throws RpcException
     *    {@code imatch} contains invalid value.
     */
    VTNInetMatch(InetMatch imatch) throws RpcException {
        Status st = imatch.getValidationStatus();
        if (st != null) {
            throw new RpcException(RpcErrorTag.BAD_ELEMENT, st);
        }

        Class<? extends IpNetwork> itype = getIpNetworkType();
        sourceNetwork = getIpNetwork(itype, imatch.getSourceAddress(),
                                     imatch.getSourceSuffix(),
                                     VTNMatch.DESC_SRC);
        destinationNetwork = getIpNetwork(
            itype, imatch.getDestinationAddress(),
            imatch.getDestinationSuffix(), VTNMatch.DESC_DST);
        protocol = imatch.getProtocol();
        dscp = NumberUtils.toShort(imatch.getDscp());
        verify();
    }

    /**
     * Construct a new instance from the given {@link VtnInetMatchFields}
     * instance.
     *
     * @param vimatch  A {@link VtnInetMatchFields} instance.
     * @throws NullPointerException
     *    {@code vimatch} is {@code null}.
     * @throws RpcException
     *    {@code vimatch} contains invalid value.
     */
    VTNInetMatch(VtnInetMatchFields vimatch) throws RpcException {
        Class<? extends IpNetwork> itype = getIpNetworkType();
        sourceNetwork = getIpNetwork(itype, vimatch.getSourceNetwork(),
                                     VTNMatch.DESC_SRC);
        destinationNetwork = getIpNetwork(
            itype, vimatch.getDestinationNetwork(), VTNMatch.DESC_DST);

        // Below restriction check can be removed when RESTCONF implements the
        // restriction check.
        protocol = vimatch.getProtocol();
        if (protocol != null) {
            ProtocolUtils.checkIpProtocol(protocol.shortValue());
        }

        dscp = ProtocolUtils.getIpDscp(vimatch.getDscp());
    }

    /**
     * Construct a new instance from the given {@link IpMatchFields} instance.
     *
     * @param imatch  An {@link IpMatchFields} instance.
     * @throws RpcException
     *    {@code imatch} contains invalid value.
     */
    VTNInetMatch(IpMatchFields imatch) throws RpcException {
        if (imatch != null) {
            // Below restriction check can be removed when RESTCONF implements
            // the restriction check.
            protocol = imatch.getIpProtocol();
            if (protocol != null) {
                ProtocolUtils.checkIpProtocol(protocol.shortValue());
            }
            dscp = ProtocolUtils.getIpDscp(imatch.getIpDscp());
        }
    }

    /**
     * Return the source IP network to match against packets.
     *
     * @return  An {@link IpNetwork} instance if the source IP network is
     *          specified. {@code null} if not specified.
     */
    public final IpNetwork getSourceNetwork() {
        return sourceNetwork;
    }

    /**
     * Return the destination IP network to match against packets.
     *
     * @return  An {@link IpNetwork} instance if the destination IP network is
     *          specified. {@code null} if not specified.
     */
    public final IpNetwork getDestinationNetwork() {
        return destinationNetwork;
    }

    /**
     * Return the IP protocol number to match against packets.
     *
     * @return  A {@link Short} instance if the IP protocol number is
     *          specified. {@code null} if not specified.
     */
    public final Short getProtocol() {
        return protocol;
    }

    /**
     * Set the IP protocol number to match against packets.
     *
     * @param proto  A {@link Short} instance which represents the IP protocol
     *               number to match against packets.
     *               {@code null} matches every IP protocol.
     * @throws RpcException
     *    The IP protocol number different from the given number is already
     *    configured in this instance.
     */
    public final void setProtocol(Short proto) throws RpcException {
        if (protocol != null && !protocol.equals(proto)) {
            String msg = new StringBuilder("IP protocol conflict: proto=").
                append(protocol).append(", expected=").append(proto).
                toString();
            throw RpcException.getBadArgumentException(msg);
        }
        protocol = proto;
    }

    /**
     * Return the IP DSCP field value to match against packets.
     *
     * @return  A {@link Short} instance if the IP DSCP value is specified.
     *          {@code null} if not specified.
     */
    public final Short getDscp() {
        return dscp;
    }

    /**
     * Return a {@link VtnInetMatchBuilder} instance which contains the flow
     * condition configured in this instance.
     *
     * @return  A {@link VtnInetMatchBuilder} instance.
     */
    public final VtnInetMatchBuilder toVtnInetMatchBuilder() {
        VtnInetMatchBuilder builder = new VtnInetMatchBuilder();
        if (sourceNetwork != null) {
            builder.setSourceNetwork(sourceNetwork.getIpPrefix());
        }
        if (destinationNetwork != null) {
            builder.setDestinationNetwork(destinationNetwork.getIpPrefix());
        }
        if (dscp != null) {
            builder.setDscp(new Dscp(dscp));
        }

        return builder.setProtocol(protocol);
    }

    /**
     * Configure the condition represented by this instance into the given
     * MD-SAL flow match builder.
     *
     * @param builder  A {@link MatchBuilder} instance.
     */
    public void setMatch(MatchBuilder builder) {
        IpMatchBuilder imatch;
        if (protocol == null) {
            imatch = null;
        } else {
            imatch = create((IpMatchBuilder)null).setIpProtocol(protocol);
        }
        if (dscp != null) {
            imatch = create(imatch).setIpDscp(new Dscp(dscp));
        }

        if (imatch != null) {
            builder.setIpMatch(imatch.build());
        }
    }

    /**
     * Determine whether the given IP header matches the condition configured
     * in this instance.
     *
     * <p>
     *   The caller has to guarantee that the IP protocol version of the
     *   IP header matches this instance.
     * </p>
     *
     * @param ctx    A {@link FlowMatchContext} instance.
     * @return  {@code true} only if the given packet header matches all
     *          the conditions configured in this instance.
     */
    public final boolean match(FlowMatchContext ctx) {
        boolean result = false;
        InetHeader iph = ctx.getInetHeader();

        // Check the source and destination IP addresses.
        if (iph != null && matchAddress(ctx, iph)) {
            // Check the IP protocol number.
            if (protocol != null) {
                ctx.addMatchField(FlowMatchType.IP_PROTO);
                if (protocol.shortValue() != iph.getProtocol()) {
                    return false;
                }
            }

            // Check the IP DSCP value.
            if (dscp != null) {
                ctx.addMatchField(FlowMatchType.IP_DSCP);
                if (dscp.shortValue() != iph.getDscp()) {
                    return false;
                }
            }

            result = true;
        }

        return result;
    }

    /**
     * Store strings used to construct flow condition key.
     *
     * @param builder  A {@link StringBuilder} instance which contains strings
     *                 used to construct flow condition key.
     */
    public final void setConditionKey(StringBuilder builder) {
        String sep = (builder.length() == 0)
            ? "" : VTNMatch.COND_KEY_SEPARATOR;

        if (sourceNetwork != null) {
            builder.append(sep).append(FlowMatchType.IP_SRC).append('=').
                append(sourceNetwork.getText());
            sep = VTNMatch.COND_KEY_SEPARATOR;
        }

        if (destinationNetwork != null) {
            builder.append(sep).append(FlowMatchType.IP_DST).append('=').
                append(destinationNetwork.getText());
            sep = VTNMatch.COND_KEY_SEPARATOR;
        }

        if (protocol != null) {
            builder.append(sep).append(FlowMatchType.IP_PROTO).append('=').
                append(protocol);
            sep = VTNMatch.COND_KEY_SEPARATOR;
        }

        if (dscp != null) {
            builder.append(sep).append(FlowMatchType.IP_DSCP).append('=').
                append(dscp);
        }
    }

    /**
     * Verify the contents of this instance.
     *
     * @throws RpcException  Verification failed.
     */
    public final void verify() throws RpcException {
        if (protocol != null) {
            ProtocolUtils.checkIpProtocol(protocol.shortValue());
        }
        if (dscp != null) {
            ProtocolUtils.checkIpDscp(dscp.shortValue());
        }
    }

    /**
     * Determine whether this instance does not specify any IP header field
     * or not.
     *
     * @return  {@code true} only if this instance is empty.
     */
    public final boolean isEmpty() {
        return (sourceNetwork == null && destinationNetwork == null &&
                protocol == null && dscp == null);
    }

    /**
     * Set the source IP network to match against packets.
     *
     * @param ipn  An {@link IpNetwork} instance which represents the source
     *             IP network to match against packets.
     *             {@code null} matches every source IP address.
     * @throws IllegalArgumentException
     *    The given IP network does not match the IP protocol version.
     */
    final void setSourceNetwork(IpNetwork ipn) {
        checkIpNetwork(ipn);
        sourceNetwork = ipn;
    }

    /**
     * Set the destination IP network to match against packets.
     *
     * @param ipn  An {@link IpNetwork} instance which represents the
     *             destination IP network to match against packets.
     *             {@code null} matches every destination IP address.
     * @throws IllegalArgumentException
     *    The given IP network does not match the IP protocol version.
     */
    final void setDestinationNetwork(IpNetwork ipn) {
        checkIpNetwork(ipn);
        destinationNetwork = ipn;
    }

    /**
     * Return an {@link RpcException} instance which notifies an invalid
     * IP address or CIDR prefix length.
     *
     * @param obj   An object which contains invalid value.
     * @param desc  A brief description about the IP address.
     * @return  An {@link RpcException} instance.
     */
    final RpcException invalidInetAddress(Object obj, String desc) {
        StringBuilder builder = new StringBuilder("Invalid ").
            append(desc).append(" IP address: ").append(obj);
        return RpcException.getBadArgumentException(builder.toString());
    }

    /**
     * Construct a new {@link IpNetwork} instance which represents the IP
     * network specified by a pair of IP address and CIDR prefix length.
     *
     * @param type    A class which specifies the type of {@link IpNetwork}
     *                instance.
     * @param iaddr   An {@link InetAddress} instance.
     * @param length  CIDR prefix length.
     * @param desc    A brief description about the specified IP network.
     * @return  An {@link IpNetwork} instance or {@code null}.
     * @throws RpcException
     *    The given parameter is invalid.
     */
    private IpNetwork getIpNetwork(Class<? extends IpNetwork> type,
                                   InetAddress iaddr, Short length,
                                   String desc) throws RpcException {
        int prefix = 0;
        boolean zero = false;
        try {
            if (length != null) {
                prefix = length.intValue();
                zero = (prefix == 0);
            }

            IpNetwork ipn = IpNetwork.create(iaddr, prefix);
            if (ipn != null) {
                if (!type.isInstance(ipn)) {
                    throw new IllegalArgumentException(
                        "Unexpected IP address type");
                }
                if (zero) {
                    // Reject zero prefix length to keep backward
                    // compatibility.
                    throw new IllegalArgumentException(
                        "Invalid prefix length: " + prefix);
                }
            }

            return ipn;
        } catch (RuntimeException e) {
            // iaddr should be a non-null value.
            String msg = new StringBuilder(iaddr.getHostAddress()).
                append(IpNetwork.CIDR_SEPARATOR).append(prefix).append(": ").
                append(e.getMessage()).toString();
            RpcException re = invalidInetAddress(msg, desc);
            re.initCause(e);
            throw re;
        }
    }

    /**
     * Construct a new {@link IpNetwork} instance which represents the IP
     * network specified by the given {@link IpPrefix} instance.
     *
     * @param type    A class which specifies the type of {@link IpNetwork}
     *                instance.
     * @param ipp     An {@link IpPrefix} instance.
     * @param desc    A brief description about the specified IP network.
     * @return  An {@link IpNetwork} instance or {@code null}.
     * @throws RpcException
     *    The given parameter is invalid.
     */
    private IpNetwork getIpNetwork(Class<? extends IpNetwork> type,
                                   IpPrefix ipp, String desc)
        throws RpcException {
        try {
            IpNetwork ipn = IpNetwork.create(ipp);
            if (ipn != null && !type.isInstance(ipn)) {
                throw new IllegalArgumentException(
                    "Unexpected IP prefix type");
            }

            return ipn;
        } catch (RuntimeException e) {
            String msg = MiscUtils.joinColon(ipp, e.getMessage());
            RpcException re = invalidInetAddress(msg, desc);
            re.initCause(e);
            throw re;
        }
    }

    /**
     * Verify that the given {@link IpNetwork} instance matches the IP protocol
     * version corresponding to this instance.
     *
     * @param ipn  An {@link IpNetwork} instance.
     * @throws IllegalArgumentException
     *    Unexpected type of IP network is specified.
     */
    private void checkIpNetwork(IpNetwork ipn) {
        if (ipn != null && !getIpNetworkType().isInstance(ipn)) {
            throw new IllegalArgumentException(
                "Unexpected IP network type: " + ipn);
        }
    }

    /**
     * Create an {@link IpMatchBuilder} instance.
     *
     * @param imatch  An {@link IpMatchBuilder} instance.
     *                {@code null} must be specified on the first call.
     * @return  If {@code imatch} is {@code null}, this method creates a new
     *          {@link IpMatchBuilder} instance and returns it.
     *          Otherwise a {@link IpMatchBuilder} instance passed to
     *          {@code imatch} is returned.
     */
    private IpMatchBuilder create(IpMatchBuilder imatch) {
        return (imatch == null) ? new IpMatchBuilder() : imatch;
    }

    /**
     * Determine whether the given IP header matches the condition for
     * IP address configured in this instance.
     *
     * @param ctx  A {@link FlowMatchContext} instance.
     * @param iph  An {@link InetHeader} instance.
     * @return  {@code true} only if the given IP header matches all the
     *          conditions for IP address configured in this instance.
     */
    private boolean matchAddress(FlowMatchContext ctx, InetHeader iph) {
        // Check the source IP address.
        if (sourceNetwork != null) {
            ctx.addMatchField(FlowMatchType.IP_SRC);
            if (!sourceNetwork.contains(iph.getSourceAddress())) {
                return false;
            }
        }

        // Check the destination IP address.
        if (destinationNetwork != null) {
            ctx.addMatchField(FlowMatchType.IP_DST);
            if (!destinationNetwork.contains(iph.getDestinationAddress())) {
                return false;
            }
        }

        return true;
    }

    /**
     * Return an Ethernet type assigned to this IP protocol version.
     *
     * @return  An Ethernet type.
     */
    public abstract int getEtherType();

    /**
     * Return a class to represent an IP address.
     *
     * @return  A class to represent an IP address.
     */
    public abstract Class<? extends IpNetwork> getIpNetworkType();

    /**
     * Return an {@link InetMatch} instance which represents this condition.
     *
     * @return  An {@link InetMatch} instance.
     */
    public abstract InetMatch toInetMatch();

    /**
     * Return An {@link IpVersion} instance which describes the IP version.
     *
     * @return  An {@link IpVersion} instance.
     */
    public abstract IpVersion getIpVersion();

    // Object

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

        VTNInetMatch im = (VTNInetMatch)o;
        return (Objects.equals(sourceNetwork, im.sourceNetwork) &&
                Objects.equals(destinationNetwork, im.destinationNetwork) &&
                Objects.equals(protocol, im.protocol) &&
                Objects.equals(dscp, im.dscp));
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public final int hashCode() {
        return Objects.hash(getClass().getName(), sourceNetwork,
                            destinationNetwork, protocol, dscp);
    }
}
