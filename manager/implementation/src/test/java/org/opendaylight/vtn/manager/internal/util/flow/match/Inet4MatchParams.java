/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.flow.match;

import java.net.InetAddress;

import org.opendaylight.vtn.manager.flow.cond.Inet4Match;
import org.opendaylight.vtn.manager.flow.cond.InetMatch;

import org.opendaylight.vtn.manager.util.IpNetwork;
import org.opendaylight.vtn.manager.util.NumberUtils;

import org.opendaylight.vtn.manager.internal.util.packet.InetHeader;

import org.opendaylight.vtn.manager.internal.TestBase;
import org.opendaylight.vtn.manager.internal.XmlNode;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.vtn.match.fields.VtnInetMatch;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.vtn.match.fields.VtnInetMatchBuilder;

import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.rev131026.flow.MatchBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.model.match.types.rev131026.match.IpMatch;
import org.opendaylight.yang.gen.v1.urn.opendaylight.model.match.types.rev131026.match.Layer3Match;
import org.opendaylight.yang.gen.v1.urn.opendaylight.model.match.types.rev131026.match.layer._3.match.Ipv4Match;

import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.inet.types.rev100924.Dscp;
import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.inet.types.rev100924.IpPrefix;
import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.inet.types.rev100924.Ipv4Prefix;

/**
 * {@code Inet4MatchParams} describes parameters for conditions to match
 * against IPv4 header.
 */
public final class Inet4MatchParams extends TestBase
    implements InetHeader, Cloneable {
    /**
     * The source IP address.
     */
    private IpNetwork  sourceAddress;

    /**
     * The destination IP address.
     */
    private IpNetwork  destinationAddress;

    /**
     * Prefix length for the source IP network.
     */
    private Short  sourcePrefix;

    /**
     * Prefix length for the destination IP network.
     */
    private Short  destinationPrefix;

    /**
     * The IP protocol number.
     */
    private Short  protocol;

    /**
     * The IP DSCP field value.
     */
    private Short  dscp;

    /**
     * Set the IP network to match against the source IP address.
     *
     * @param ipn  An {@link IpNetwork} instance.
     * @return  This instance.
     */
    public Inet4MatchParams setSourceNetwork(IpNetwork ipn) {
        if (ipn == null) {
            sourceAddress = null;
            sourcePrefix = null;
        } else {
            sourceAddress = IpNetwork.create(ipn.getInetAddress());
            sourcePrefix = (ipn.isAddress())
                ? null : Short.valueOf((short)ipn.getPrefixLength());
        }

        return this;
    }

    /**
     * Set the IP address to match against the source IP address.
     *
     * @param ip  An {@link InetAddress} instance.
     * @return  This instance.
     */
    public Inet4MatchParams setSourceAddress(InetAddress ip) {
        sourceAddress = IpNetwork.create(ip);
        return this;
    }

    /**
     * Set the IP network to match against the destination IP address.
     *
     * @param ipn  An {@link IpNetwork} instance.
     * @return  This instance.
     */
    public Inet4MatchParams setDestinationNetwork(IpNetwork ipn) {
        if (ipn == null) {
            destinationAddress = null;
            destinationPrefix = null;
        } else {
            destinationAddress = IpNetwork.create(ipn.getInetAddress());
            destinationPrefix = (ipn.isAddress())
                ? null : Short.valueOf((short)ipn.getPrefixLength());
        }

        return this;
    }

    /**
     * Set the IP address to match against the destination IP address.
     *
     * @param ip  An {@link InetAddress} instance.
     * @return  This instance.
     */
    public Inet4MatchParams setDestinationAddress(InetAddress ip) {
        destinationAddress = IpNetwork.create(ip);
        return this;
    }

    /**
     * Set the prefix length for the source IP network.
     *
     * @param len  The prefix length.
     * @return  This instance.
     */
    public Inet4MatchParams setSourcePrefix(Short len) {
        sourcePrefix = len;
        return this;
    }

    /**
     * Set the prefix length for the destination IP network.
     *
     * @param len  The prefix length.
     * @return  This instance.
     */
    public Inet4MatchParams setDestinationPrefix(Short len) {
        destinationPrefix = len;
        return this;
    }

    /**
     * Set the IP protocol number.
     *
     * @param proto  The IP protocol number.
     * @return  This instance.
     */
    public Inet4MatchParams setProtocol(Short proto) {
        protocol = proto;
        return this;
    }

    /**
     * Set the IP DSCP field value.
     *
     * @param value  The DSCP field value.
     * @return  This instance.
     */
    public Inet4MatchParams setDscp(Short value) {
        dscp = value;
        return this;
    }

    /**
     * Reset to the initial state.
     *
     * @return  This instance.
     */
    public Inet4MatchParams reset() {
        sourceAddress = null;
        destinationAddress = null;
        sourcePrefix = null;
        destinationPrefix = null;
        protocol = null;
        dscp = null;
        return this;
    }

    /**
     * Construct an {@link Inet4Match} instance.
     *
     * @return  An {@link Inet4Match} instance.
     */
    public Inet4Match toInet4Match() {
        InetAddress src = (sourceAddress == null)
            ? null : sourceAddress.getInetAddress();
        InetAddress dst = (destinationAddress == null)
            ? null : destinationAddress.getInetAddress();
        Byte d = NumberUtils.toByte(dscp);
        return new Inet4Match(src, sourcePrefix, dst, destinationPrefix,
                              protocol, d);
    }

    /**
     * Construct a {@link VtnInetMatch} instance.
     *
     * @return  A {@link VtnInetMatch} instance.
     */
    public VtnInetMatch toVtnInetMatch() {
        return toVtnInetMatch(false);
    }

    /**
     * Construct a {@link VtnInetMatch} instance.
     *
     * @param noZero  Avoid zero prefix if {@code true}.
     * @return  A {@link VtnInetMatch} instance.
     */
    public VtnInetMatch toVtnInetMatch(boolean noZero) {
        VtnInetMatchBuilder builder = new VtnInetMatchBuilder();

        IpPrefix src = getIpPrefix(sourceAddress, sourcePrefix, noZero);
        if (src != null) {
            builder.setSourceNetwork(src);
        }

        IpPrefix dst = getIpPrefix(destinationAddress, destinationPrefix,
                                   noZero);
        if (dst != null) {
            builder.setDestinationNetwork(dst);
        }

        if (protocol != null) {
            builder.setProtocol(protocol);
        }

        if (dscp != null) {
            builder.setDscp(new Dscp(dscp));
        }

        return builder.build();
    }

    /**
     * Construct a new {@link VTNInet4Match} instance.
     *
     * @return  A {@link VTNInet4Match} instance.
     * @throws Exception  An error occurred.
     */
    public VTNInet4Match toVTNInet4Match() throws Exception {
        return new VTNInet4Match(toInet4Match());
    }

    /**
     * Return a {@link XmlNode} instance which represents this instance.
     *
     * @param name  The name of the root node.
     * @return  A {@link XmlNode} instance.
     */
    public XmlNode toXmlNode(String name) {
        XmlNode root = new XmlNode(name);
        IpNetwork src = getIpNetwork(sourceAddress, sourcePrefix);
        if (src != null) {
            root.add(new XmlNode("source-network-v4", src.getText()));
        }

        IpNetwork dst = getIpNetwork(destinationAddress, destinationPrefix);
        if (dst != null) {
            root.add(new XmlNode("destination-network-v4", dst.getText()));
        }

        if (protocol != null) {
            root.add(new XmlNode("protocol", protocol));
        }

        if (dscp != null) {
            root.add(new XmlNode("dscp", dscp));
        }

        return root;
    }

    /**
     * Ensure that the given {@link VTNInet4Match} instance contains the same
     * IP conditions as this instance.
     *
     * @param imatch  A {@link VTNInet4Match} instance.
     * @param src     An {@link IpNetwork} instance which represents the
     *                source IP network configured in this instance.
     * @param dst     An {@link IpNetwork} instance which represents the
     *                destniation IP network configured in this instance.
     */
    public void verifyValues(VTNInet4Match imatch, IpNetwork src,
                             IpNetwork dst) {
        assertEquals(src, imatch.getSourceNetwork());
        assertEquals(dst, imatch.getDestinationNetwork());
        assertEquals(protocol, imatch.getProtocol());
        assertEquals(dscp, imatch.getDscp());
    }

    /**
     * Ensure that the given {@link VTNInet4Match} instance contains the same
     * IP conditions as this instance.
     *
     * @param imatch  A {@link VTNInet4Match} instance.
     */
    public void verifyValues(VTNInet4Match imatch) {
        IpNetwork src = getIpNetwork(sourceAddress, sourcePrefix);
        IpNetwork dst = getIpNetwork(destinationAddress, destinationPrefix);
        verifyValues(imatch, src, dst);
        assertEquals(isEmpty(), imatch.isEmpty());
    }

    /**
     * Ensure that the given {@link VTNInet4Match} instance contains the same
     * IP conditions as this instance.
     *
     * @param imatch  A {@link VTNInet4Match} instance.
     * @throws Exception  An error occurred.
     */
    public void verify(VTNInet4Match imatch) throws Exception {
        IpNetwork src = getIpNetwork(sourceAddress, sourcePrefix);
        IpNetwork dst = getIpNetwork(destinationAddress, destinationPrefix);
        verifyValues(imatch, src, dst);

        InetMatch im = imatch.toInetMatch();
        assertTrue(im instanceof Inet4Match);
        InetAddress srcAddr = im.getSourceAddress();
        Short srcSuff = im.getSourceSuffix();
        InetAddress dstAddr = im.getDestinationAddress();
        Short dstSuff = im.getDestinationSuffix();
        assertEquals(src, getIpNetwork(srcAddr, srcSuff));
        assertEquals(dst, getIpNetwork(dstAddr, dstSuff));
        assertEquals(protocol, im.getProtocol());
        assertEquals(NumberUtils.toByte(dscp), im.getDscp());

        VtnInetMatch vim = imatch.toVtnInetMatchBuilder().build();
        assertEquals(src, IpNetwork.create(vim.getSourceNetwork()));
        assertEquals(dst, IpNetwork.create(vim.getDestinationNetwork()));
        assertEquals(protocol, vim.getProtocol());
        if (dscp == null) {
            assertEquals(null, vim.getDscp());
        } else {
            assertEquals(dscp, vim.getDscp().getValue());
        }

        MatchBuilder mb = new MatchBuilder();
        imatch.setMatch(mb);
        boolean hasValue = verify(mb);

        VTNInetMatch imatch1 = VTNInetMatch.create(mb.build());
        assertEquals((hasValue) ? imatch : null, imatch1);
    }

    /**
     * Ensure that the given {@link MatchBuilder} instance contains the same
     * IP conditions as this instance.
     *
     * @param builder  A {@link MatchBuilder} instance.
     * @return  {@code true} only if the given match contains the condition
     *          for the IP header.
     */
    public boolean verify(MatchBuilder builder) {
        boolean isIp = false;
        boolean isIpv4 = false;
        IpMatch imatch = builder.getIpMatch();
        if (imatch == null) {
            assertEquals(null, protocol);
            assertEquals(null, dscp);
        } else {
            assertEquals(protocol, imatch.getIpProtocol());
            if (protocol != null) {
                isIp = true;
            }

            if (dscp == null) {
                assertEquals(null, imatch.getIpDscp());
            } else {
                assertEquals(dscp, imatch.getIpDscp().getValue());
                isIp = true;
            }
            assertTrue(isIp);
        }

        Layer3Match l3 = builder.getLayer3Match();
        if (l3 == null) {
            assertEquals(null, sourceAddress);
            assertEquals(null, destinationAddress);
        } else {
            assertTrue(l3 instanceof Ipv4Match);
            Ipv4Match i4match = (Ipv4Match)l3;
            if (sourceAddress == null) {
                assertEquals(null, i4match.getIpv4Source());
            } else {
                Ipv4Prefix pfx = i4match.getIpv4Source();
                IpNetwork src = getIpNetwork(sourceAddress, sourcePrefix);
                IpPrefix ipp = src.getIpPrefix();
                assertEquals(pfx, ipp.getIpv4Prefix());
                assertEquals(null, ipp.getIpv6Prefix());
                isIpv4 = true;
            }

            if (destinationAddress == null) {
                assertEquals(null, i4match.getIpv4Destination());
            } else {
                Ipv4Prefix pfx = i4match.getIpv4Destination();
                IpNetwork dst =
                    getIpNetwork(destinationAddress, destinationPrefix);
                IpPrefix ipp = dst.getIpPrefix();
                assertEquals(pfx, ipp.getIpv4Prefix());
                assertEquals(null, ipp.getIpv6Prefix());
                isIpv4 = true;
            }
            assertTrue(isIpv4);
        }

        return (isIp || isIpv4);
    }

    /**
     * Determine whether this instance does not specify any IP header field
     * or not.
     *
     * @return  {@code true} only if this instance is empty.
     */
    public boolean isEmpty() {
        return (sourceAddress == null && destinationAddress == null &&
                protocol == null && dscp == null);
    }

    /**
     * Create an {@link IpNetwork}.
     *
     * @param addr    An {@link IpNetwork} instance which represents the
     *                IP address.
     * @param prefix  The network prefix length.
     * @return  A {@link IpNetwork} or {@code null}.
     */
    private IpNetwork getIpNetwork(IpNetwork addr, Short prefix) {
        if (addr == null) {
            return null;
        }

        int len = (prefix == null) ? 0 : prefix.intValue();
        return IpNetwork.create(addr.getInetAddress(), len);
    }

    /**
     * Create an {@link IpNetwork}.
     *
     * @param addr    An {@link InetAddress} instance which represents the
     *                IP address.
     * @param prefix  The network prefix length.
     * @return  A {@link IpNetwork} or {@code null}.
     */
    private IpNetwork getIpNetwork(InetAddress addr, Short prefix) {
        if (addr == null) {
            return null;
        }

        int len = (prefix == null) ? 0 : prefix.intValue();
        return IpNetwork.create(addr, len);
    }

    /**
     * Create an {@link IpPrefix}.
     *
     * @param addr    An {@link IpNetwork} instance which represents the
     *                IP address.
     * @param prefix  The network prefix length.
     * @param noZero  Avoid zero prefix if {@code true}.
     * @return  A {@link IpPrefix} or {@code null}.
     */
    private IpPrefix getIpPrefix(IpNetwork addr, Short prefix,
                                 boolean noZero) {
        if (addr == null) {
            return null;
        }

        int len = (prefix == null) ? 0 : prefix.intValue();
        if (len == 0 && noZero) {
            len = addr.getMaxPrefix();
        }
        StringBuilder builder = new StringBuilder(addr.getText()).
            append('/').append(len);
        Ipv4Prefix v4 = new Ipv4Prefix(builder.toString());
        return new IpPrefix(v4);
    }

    // InetHeader

    /**
     * {@inheritDoc}
     */
    @Override
    public IpNetwork getSourceAddress() {
        return sourceAddress;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public boolean setSourceAddress(IpNetwork ipn) {
        sourceAddress = ipn;
        return true;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public IpNetwork getDestinationAddress() {
        return destinationAddress;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public boolean setDestinationAddress(IpNetwork ipn) {
        destinationAddress = ipn;
        return true;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public short getProtocol() {
        return protocol.shortValue();
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public short getDscp() {
        return dscp.shortValue();
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void setDscp(short value) {
        dscp = Short.valueOf(value);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void setDescription(StringBuilder builder) {
    }

    // Cloneable

    /**
     * Return a shallow copy of this instance.
     *
     * @return  A shallow copy of this instance.
     */
    @Override
    public Inet4MatchParams clone() {
        try {
            return (Inet4MatchParams)super.clone();
        } catch (CloneNotSupportedException e) {
            // This should never happen.
            throw new IllegalStateException("clone() failed.", e);
        }
    }
}
