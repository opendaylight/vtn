/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.flow.match;

import java.net.InetAddress;

import javax.xml.bind.annotation.XmlAccessType;
import javax.xml.bind.annotation.XmlAccessorType;
import javax.xml.bind.annotation.XmlRootElement;

import org.opendaylight.vtn.manager.flow.cond.Inet4Match;
import org.opendaylight.vtn.manager.util.Ip4Network;
import org.opendaylight.vtn.manager.util.IpNetwork;
import org.opendaylight.vtn.manager.util.NumberUtils;

import org.opendaylight.vtn.manager.internal.util.MiscUtils;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;

import org.opendaylight.controller.sal.utils.EtherTypes;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.VtnInetMatchFields;

import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.rev131026.flow.MatchBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.model.match.types.rev131026.IpMatchFields;
import org.opendaylight.yang.gen.v1.urn.opendaylight.model.match.types.rev131026.Ipv4MatchFields;
import org.opendaylight.yang.gen.v1.urn.opendaylight.model.match.types.rev131026.match.layer._3.match.Ipv4MatchBuilder;

import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.inet.types.rev100924.IpPrefix;
import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.inet.types.rev100924.IpVersion;
import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.inet.types.rev100924.Ipv4Prefix;

/**
 * {@code VTNInet4Match} describes the condition for IPv4 header to match
 * against packets.
 *
 * <p>
 *   Note that this class is not synchronized.
 * </p>
 */
@XmlRootElement(name = "vtn-inet4-match")
@XmlAccessorType(XmlAccessType.NONE)
public final class VTNInet4Match extends VTNInetMatch {
    /**
     * Construct a new instance that matches every IPv4 packet.
     */
    public VTNInet4Match() {
    }

    /**
     * Construct a new instance that matches the given IP protocol.
     *
     * @param proto  A {@link Short} instance which represents the IP protocol
     *               number to match against packets.
     *               {@code null} matches every IP protocol.
     */
    public VTNInet4Match(Short proto) {
        super(proto);
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
    public VTNInet4Match(IpNetwork src, IpNetwork dst, Short proto, Short d)
        throws RpcException {
        super(src, dst, proto, d);
        verify();
    }

    /**
     * Construct a new instance from the given {@link Inet4Match} instance.
     *
     * @param imatch  An {@link Inet4Match} instance.
     * @throws NullPointerException
     *    {@code imatch} is {@code null}.
     * @throws RpcException
     *    {@code imatch} contains invalid value.
     */
    public VTNInet4Match(Inet4Match imatch) throws RpcException {
        super(imatch);
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
    public VTNInet4Match(VtnInetMatchFields vimatch) throws RpcException {
        super(vimatch);
    }

    /**
     * Construct a new instance from the given {@link IpMatchFields} and
     * {@link Ipv4MatchFields} instances.
     *
     * @param imatch  An {@link IpMatchFields} instance.
     * @param i4match  An {@link Ipv4MatchFields} instance.
     * @throws RpcException
     *    {@code imatch} or {@code i4match} contains invalid value.
     */
    public VTNInet4Match(IpMatchFields imatch, Ipv4MatchFields i4match)
        throws RpcException {
        super(imatch);

        if (i4match != null) {
            Ip4Network ipn4 = getIp4Network(i4match.getIpv4Source(),
                                            VTNMatch.DESC_SRC);
            setSourceNetwork(ipn4);
            ipn4 = getIp4Network(i4match.getIpv4Destination(),
                                VTNMatch.DESC_DST);
            setDestinationNetwork(ipn4);
        }
    }

    /**
     * Construct a new {@link Ip4Network} instance which represents the IPv4
     * network specified by the given {@link Ipv4Prefix} instance.
     *
     * @param ipp4    An {@link Ipv4Prefix} instance.
     * @param desc    A brief description about the specified IP network.
     * @return  An {@link Ip4Network} instance or {@code null}.
     * @throws RpcException
     *    The given parameter is invalid.
     */
    private Ip4Network getIp4Network(Ipv4Prefix ipp4, String desc)
        throws RpcException {
        if (ipp4 == null) {
            return null;
        }
        String value = ipp4.getValue();
        if (value == null) {
            return null;
        }

        try {
            return new Ip4Network(value);
        } catch (RuntimeException e) {
            String msg = MiscUtils.joinColon(ipp4, e.getMessage());
            RpcException re = invalidInetAddress(msg, desc);
            re.initCause(e);
            throw re;
        }
    }

    /**
     * Create an {@link Ipv4MatchBuilder} instance.
     *
     * @param imatch  An {@link Ipv4MatchBuilder} instance.
     *                {@code null} must be specified on the first call.
     * @return  If {@code imatch} is {@code null}, this method creates a new
     *          {@link Ipv4MatchBuilder} instance and returns it.
     *          Otherwise a {@link Ipv4MatchBuilder} instance passed to
     *          {@code imatch} is returned.
     */
    private Ipv4MatchBuilder create(Ipv4MatchBuilder imatch) {
        return (imatch == null) ? new Ipv4MatchBuilder() : imatch;
    }

    // VTNInetMatch

    /**
     * {@inheritDoc}
     */
    @Override
    public void setMatch(MatchBuilder builder) {
        super.setMatch(builder);

        Ipv4MatchBuilder imatch = null;
        IpNetwork ipn = getSourceNetwork();
        if (ipn != null) {
            IpPrefix ipp = ipn.getIpPrefix();
            imatch = create(imatch).setIpv4Source(ipp.getIpv4Prefix());
        }

        ipn = getDestinationNetwork();
        if (ipn != null) {
            IpPrefix ipp = ipn.getIpPrefix();
            imatch = create(imatch).setIpv4Destination(ipp.getIpv4Prefix());
        }

        if (imatch != null) {
            builder.setLayer3Match(imatch.build());
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public int getEtherType() {
        return EtherTypes.IPv4.intValue();
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public Class<Ip4Network> getIpNetworkType() {
        return Ip4Network.class;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public Inet4Match toInetMatch() {
        InetAddress srcAddr;
        Short srcSuff;
        IpNetwork ipn = getSourceNetwork();
        if (ipn == null) {
            srcAddr = null;
            srcSuff = null;
        } else {
            srcAddr = ipn.getInetAddress();
            srcSuff = (ipn.isAddress())
                ? null
                : Short.valueOf((short)ipn.getPrefixLength());
        }

        InetAddress dstAddr;
        Short dstSuff;
        ipn = getDestinationNetwork();
        if (ipn == null) {
            dstAddr = null;
            dstSuff = null;
        } else {
            dstAddr = ipn.getInetAddress();
            dstSuff = (ipn.isAddress())
                ? null
                : Short.valueOf((short)ipn.getPrefixLength());
        }

        Short proto = getProtocol();
        Byte d = NumberUtils.toByte(getDscp());
        return new Inet4Match(srcAddr, srcSuff, dstAddr, dstSuff, proto, d);
    }

    /**
     * Return An {@link IpVersion} instance which describes the IP version.
     *
     * @return  {@link IpVersion#Ipv4}.
     */
    public IpVersion getIpVersion() {
        return IpVersion.Ipv4;
    }
}
