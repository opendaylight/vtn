/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.util.flow.match;

import static org.junit.Assert.assertEquals;

import org.opendaylight.vtn.manager.util.IpNetwork;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.vtn.match.fields.VtnInetMatch;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.vtn.match.fields.VtnInetMatchBuilder;

import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.inet.types.rev100924.Dscp;
import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.inet.types.rev100924.IpVersion;

/**
 * {@code VTNInetMatch} describes the condition for IP header to match against
 * packets.
 */
public abstract class VTNInetMatch implements Cloneable {
    /**
     * The source IP network to match.
     */
    private IpNetwork  sourceNetwork;

    /**
     * The destination IP network to match.
     */
    private IpNetwork  destinationNetwork;

    /**
     * An IP protocol number to match.
     */
    private Short  protocol;

    /**
     * A DSCP field value to match.
     */
    private Short  dscp;

    /**
     * Construct an empty instance.
     */
    VTNInetMatch() {
    }

    /**
     * Construct a new instance.
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
     */
    VTNInetMatch(IpNetwork src, IpNetwork dst, Short proto, Short d) {
        sourceNetwork = src;
        destinationNetwork = dst;
        protocol = proto;
        dscp = d;
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
     * Return the IP DSCP field value to match against packets.
     *
     * @return  A {@link Short} instance if the IP DSCP value is specified.
     *          {@code null} if not specified.
     */
    public final Short getDscp() {
        return dscp;
    }

    /**
     * Complete match conditions.
     *
     * @param proto  An expected value of IP protocol.
     * @return  A {@link VTNInetMatch} instance that contains completed
     *          match conditions.
     */
    public final VTNInetMatch complete(Short proto) {
        VTNInetMatch imatch;

        if (proto == null) {
            imatch = this;
        } else if (protocol == null) {
            imatch = clone();
            imatch.protocol = proto;
        } else {
            assertEquals(proto, protocol);
            imatch = this;
        }

        return imatch;
    }

    /**
     * Verify the given vtn-inet-match.
     *
     * @param vimatch  A {@link VtnInetMatch} instance.
     */
    public final void verify(VtnInetMatch vimatch) {
        assertEquals(sourceNetwork,
                     IpNetwork.create(vimatch.getSourceNetwork()));
        assertEquals(destinationNetwork,
                     IpNetwork.create(vimatch.getDestinationNetwork()));
        assertEquals(protocol, vimatch.getProtocol());

        if (dscp == null) {
            assertEquals(null, vimatch.getDscp());
        } else {
            assertEquals(dscp, vimatch.getDscp().getValue());
        }
    }

    /**
     * Return a {@link VtnInetMatch} instance which contains the condition
     * represented by this instance.
     *
     * @return  A {@link VtnInetMatch} instance.
     */
    public final VtnInetMatch toVtnInetMatch() {
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

        return builder.setProtocol(protocol).build();
    }

    /**
     * Determine whether this instance is empty or not.
     *
     * @return  {@code true} if this instance is empty.
     *          {@code false} otherwise.
     */
    public final boolean isEmpty() {
        boolean ret = (sourceNetwork == null && destinationNetwork == null);
        if (ret) {
            ret = (protocol == null && dscp == null);
        }

        return ret;
    }

    /**
     * Set the source IP network to match against packets.
     *
     * @param ipn  A {@link IpNetwork} instance which specifies the source
     *             IP address to match.
     *             {@code null} matches every source IP address.
     */
    protected final void setSource(IpNetwork ipn) {
        sourceNetwork = ipn;
    }

    /**
     * Set the destination IP network to match against packets.
     *
     * @param ipn  A {@link IpNetwork} instance which specifies the destination
     *             IP address to match.
     *             {@code null} matches every destination IP address.
     */
    protected final void setDestination(IpNetwork ipn) {
        destinationNetwork = ipn;
    }

    /**
     * Set the IP protocol number to match against packets.
     *
     * @param proto  A {@link Short} instance that indicates the IP protocol
     *               number to match against packets.
     *               {@code null} matches every IP protocol.
     */
    protected final void setIpProto(Short proto) {
        protocol = proto;
    }

    /**
     * Set the IP DSCP field value to match against packets.
     *
     * @param d  A {@link Short} instance that indicates the IP DSCP field
     *           value to match against packets.
     *           {@code null} matches every DSCP field value.
     */
    protected final void setIpDscp(Short d) {
        dscp = d;
    }

    /**
     * Return an Ethernet type assigned to this IP protocol version.
     *
     * @return  An Ethernet type.
     */
    public abstract int getEtherType();

    /**
     * Return An {@link IpVersion} instance which describes the IP version.
     *
     * @return  An {@link IpVersion} instance.
     */
    public abstract IpVersion getIpVersion();

    // Object

    /**
     * Create a copy of this instance.
     *
     * @return  A copy of this instance.
     */
    @Override
    public VTNInetMatch clone() {
        try {
            return (VTNInetMatch)super.clone();
        } catch (CloneNotSupportedException e) {
            throw new IllegalStateException("Unable to clone.", e);
        }
    }
}
