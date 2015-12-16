/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.util.flow.match;

import org.opendaylight.vtn.manager.util.EtherTypes;
import org.opendaylight.vtn.manager.util.Ip4Network;

import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.inet.types.rev100924.IpVersion;

/**
 * {@code VTNInet4Match} describes the condition for IPv4 header to match
 * against packets.
 */
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
        super(null, null, proto, null);
    }

    /**
     * Construct a new instance.
     *
     * @param src    A {@link Ip4Network} instance which specifies the source
     *               IP address to match.
     *               {@code null} matches every source IP address.
     * @param dst    A {@link Ip4Network} instance which specifies the
     *               destination IP address to match.
     *               {@code null} matches every destination IP address.
     * @param proto  A {@link Short} instance which represents the IP protocol
     *               number to match against packets.
     *               {@code null} matches every IP protocol.
     * @param d      A DSCP field value to match.
     *               {@code null} matches every DSCP field value.
     */
    public VTNInet4Match(Ip4Network src, Ip4Network dst, Short proto,
                         Short d) {
        super(src, dst, proto, d);
    }

    /**
     * Set the source IP network to match against packets.
     *
     * @param ipn  A {@link Ip4Network} instance which specifies the source
     *             IP address to match.
     *             {@code null} matches every source IP address.
     * @return  This instance.
     */
    public VTNInet4Match setSourceNetwork(Ip4Network ipn) {
        setSource(ipn);
        return this;
    }

    /**
     * Set the destination IP network to match against packets.
     *
     * @param ipn  A {@link Ip4Network} instance which specifies the
     *             destination IP address to match.
     *             {@code null} matches every destination IP address.
     * @return  This instance.
     */
    public VTNInet4Match setDestinationNetwork(Ip4Network ipn) {
        setDestination(ipn);
        return this;
    }

    /**
     * Set the IP protocol number to match against packets.
     *
     * @param proto  A {@link Short} instance that indicates the IP protocol
     *               number to match against packets.
     *               {@code null} matches every IP protocol.
     * @return  This instance.
     */
    public VTNInet4Match setProtocol(Short proto) {
        setIpProto(proto);
        return this;
    }

    /**
     * Set the IP DSCP field value to match against packets.
     *
     * @param d  A {@link Short} instance that indicates the IP DSCP field
     *           value to match against packets.
     *           {@code null} matches every DSCP field value.
     * @return  This instance.
     */
    public VTNInet4Match setDscp(Short d) {
        setIpDscp(d);
        return this;
    }

    // VTNInetMatch

    /**
     * {@inheritDoc}
     */
    @Override
    public int getEtherType() {
        return EtherTypes.IPV4.intValue();
    }

    /**
     * Return An {@link IpVersion} instance which describes the IP version.
     *
     * @return  {@link IpVersion#Ipv4}.
     */
    @Override
    public IpVersion getIpVersion() {
        return IpVersion.Ipv4;
    }

    // Object

    /**
     * Create a copy of this instance.
     *
     * @return  A copy of this instance.
     */
    @Override
    public VTNInet4Match clone() {
        return (VTNInet4Match)super.clone();
    }
}
