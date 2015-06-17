/*
 * Copyright (c) 2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.flow.match;

import java.util.Objects;

import javax.xml.bind.annotation.XmlAccessType;
import javax.xml.bind.annotation.XmlAccessorType;
import javax.xml.bind.annotation.XmlElement;
import javax.xml.bind.annotation.XmlRootElement;

import org.opendaylight.vtn.manager.flow.cond.IcmpMatch;

import org.opendaylight.vtn.manager.internal.util.ProtocolUtils;
import org.opendaylight.vtn.manager.internal.util.packet.IcmpHeader;
import org.opendaylight.vtn.manager.internal.util.packet.Layer4Header;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;

import org.opendaylight.controller.sal.utils.IPProtocols;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.VtnIcmpMatchFields;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.vtn.match.fields.vtn.layer4.match.VtnIcmpMatch;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.vtn.match.fields.vtn.layer4.match.VtnIcmpMatchBuilder;

import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.rev131026.flow.MatchBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.model.match.types.rev131026.Icmpv4MatchFields;
import org.opendaylight.yang.gen.v1.urn.opendaylight.model.match.types.rev131026.match.Icmpv4MatchBuilder;

import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.inet.types.rev100924.IpVersion;

/**
 * {@code VTNIcmpMatch} describes the condition for ICMP header to match
 * against packets.
 *
 * <p>
 *   Note that this class is not synchronized.
 * </p>
 */
@XmlRootElement(name = "vtn-icmp-match")
@XmlAccessorType(XmlAccessType.NONE)
public final class VTNIcmpMatch extends VTNLayer4Match {
    /**
     * The ICMP type to match against packets.
     */
    @XmlElement(name = "type")
    private Short  icmpType;

    /**
     * The ICMP code to match against packets.
     */
    @XmlElement(name = "code")
    private Short  icmpCode;

    /**
     * Construct a new instance that matches every ICMP packet.
     */
    public VTNIcmpMatch() {
    }

    /**
     * Construct a new instance.
     *
     * @param type  The ICMP type to match.
     *              {@code null} matches every ICMP type.
     * @param code  The ICMP code to match.
     *              {@code null} matches every ICMP code.
     * @throws RpcException
     *    The specified condition is invalid.
     */
    public VTNIcmpMatch(Short type, Short code) throws RpcException {
        icmpType = type;
        icmpCode = code;
        verify();
    }

    /**
     * Create a new instance from the given {@link IcmpMatch} instance.
     *
     * @param imatch  An {@link IcmpMatch} instance.
     * @throws NullPointerException
     *    {@code imatch} is {@code null}.
     * @throws RpcException
     *    {@code imatch} contains invalid value.
     */
    public VTNIcmpMatch(IcmpMatch imatch) throws RpcException {
        icmpType = imatch.getType();
        icmpCode = imatch.getCode();
        verify();
    }

    /**
     * Create a new instance from the given {@link VtnIcmpMatchFields}
     * instance.
     *
     * @param vimatch  An {@link VtnIcmpMatchFields} instance.
     * @throws NullPointerException
     *    {@code vimatch} is {@code null}.
     * @throws RpcException
     *    {@code vimatch} contains invalid value.
     */
    public VTNIcmpMatch(VtnIcmpMatchFields vimatch) throws RpcException {
        icmpType = vimatch.getIcmpType();
        icmpCode = vimatch.getIcmpCode();

        // Below restriction check can be removed when RESTCONF implements the
        // restriction check.
        verify();
    }

    /**
     * Create a new instance from the given {@link Icmpv4MatchFields} instance.
     *
     * @param imatch  An {@link Icmpv4MatchFields} instance.
     * @throws NullPointerException
     *    {@code imatch} is {@code null}.
     * @throws RpcException
     *    {@code imatch} contains invalid value.
     */
    public VTNIcmpMatch(Icmpv4MatchFields imatch) throws RpcException {
        icmpType = imatch.getIcmpv4Type();
        icmpCode = imatch.getIcmpv4Code();

        // Below restriction check can be removed when RESTCONF implements the
        // restriction check.
        verify();
    }

    /**
     * Return the ICMP type to match against packets.
     *
     * @return  A {@link Short} instance which indicates the ICMP type to
     *          match. {@code null} if the ICMP type is not specified.
     */
    public Short getIcmpType() {
        return icmpType;
    }

    /**
     * Return the ICMP code to match against packets.
     *
     * @return  A {@link Short} instance which indicates the ICMP code to
     *          match. {@code null} if the ICMP code is not specified.
     */
    public Short getIcmpCode() {
        return icmpCode;
    }

    /**
     * Create an {@link VtnIcmpMatchBuilder} instance.
     *
     * @param vimatch  An {@link VtnIcmpMatchBuilder} instance.
     *                 {@code null} must be specified on the first call.
     * @return  If {@code vimatch} is {@code null}, this method creates a new
     *          {@link VtnIcmpMatchBuilder} instance and returns it.
     *          Otherwise a {@link VtnIcmpMatchBuilder} instance passed to
     *          {@code vimatch} is returned.
     */
    private VtnIcmpMatchBuilder create(VtnIcmpMatchBuilder vimatch) {
        return (vimatch == null) ? new VtnIcmpMatchBuilder() : vimatch;
    }

    /**
     * Create an {@link Icmpv4MatchBuilder} instance.
     *
     * @param imatch  An {@link Icmpv4MatchBuilder} instance.
     *                {@code null} must be specified on the first call.
     * @return  If {@code imatch} is {@code null}, this method creates a new
     *          {@link Icmpv4MatchBuilder} instance and returns it.
     *          Otherwise a {@link Icmpv4MatchBuilder} instance passed to
     *          {@code imatch} is returned.
     */
    private Icmpv4MatchBuilder create(Icmpv4MatchBuilder imatch) {
        return (imatch == null) ? new Icmpv4MatchBuilder() : imatch;
    }

    /**
     * Ensure that the given IP version is supported.
     *
     * @param ver  An {@link IpVersion} instance which describes the
     *             IP version.
     * @throws IllegalStateException
     *    {@code ver} is not {@link IpVersion#Ipv4}.
     */
    private void checkIpVersion(IpVersion ver) {
        if (IpVersion.Ipv4 != ver) {
            // This should never happen.
            throw new IllegalStateException("Unsupported IP version: " + ver);
        }
    }

    // VTNLayer4Match

    /**
     * {@inheritDoc}
     */
    @Override
    public void verify() throws RpcException {
        ProtocolUtils.checkIcmpValue(icmpType, "type");
        ProtocolUtils.checkIcmpValue(icmpCode, "code");
    }

    /**
     * Return an IP protocol number assigned to this protocol.
     *
     * @param ver  An {@link IpVersion} instance which describes the
     *             IP version.
     * @return  An IP protocol number.
     * @throws IllegalStateException
     *    {@code ver} is not {@link IpVersion#Ipv4}.
     */
    @Override
    public short getInetProtocol(IpVersion ver) {
        checkIpVersion(ver);
        return IPProtocols.ICMP.shortValue();
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public IcmpMatch toL4Match() {
        return new IcmpMatch(icmpType, icmpCode);
    }

    /**
     * Return a {@link VtnIcmpMatch} instance which contains the condition
     * represented by this instance.
     *
     * @return  A {@link VtnIcmpMatch} instance if this instance contains
     *          the condition. {@code null} if this instance does not contain
     *          any condition.
     */
    @Override
    public VtnIcmpMatch toVtnLayer4Match() {
        VtnIcmpMatchBuilder vimatch = null;
        if (icmpType != null) {
            vimatch = create(vimatch).setIcmpType(icmpType);
        }

        if (icmpCode != null) {
            vimatch = create(vimatch).setIcmpCode(icmpCode);
        }

        return (vimatch == null) ? null : vimatch.build();
    }

    /**
     * Configure the condition represented by this instance into the given
     * MD-SAL flow match builder.
     *
     * @param builder  A {@link MatchBuilder} instance.
     * @param ver      An {@link IpVersion} instance which describes the
     *                 IP version.
     * @throws IllegalStateException
     *    {@code ver} is not {@link IpVersion#Ipv4}.
     */
    @Override
    public void setMatch(MatchBuilder builder, IpVersion ver) {
        checkIpVersion(ver);
        Icmpv4MatchBuilder imatch = null;
        if (icmpType != null) {
            imatch = create(imatch).setIcmpv4Type(icmpType);
        }

        if (icmpCode != null) {
            imatch = create(imatch).setIcmpv4Code(icmpCode);
        }

        if (imatch != null) {
            builder.setIcmpv4Match(imatch.build());
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public boolean match(FlowMatchContext ctx) {
        Layer4Header l4 = ctx.getLayer4Header();
        if (!(l4 instanceof IcmpHeader)) {
            return false;
        }

        IcmpHeader icmp = (IcmpHeader)l4;
        if (icmpType != null) {
            ctx.addMatchField(FlowMatchType.ICMP_TYPE);
            if (icmpType.shortValue() != icmp.getIcmpType()) {
                return false;
            }
        }

        if (icmpCode != null) {
            ctx.addMatchField(FlowMatchType.ICMP_CODE);
            if (icmpCode.shortValue() != icmp.getIcmpCode()) {
                return false;
            }
        }

        return true;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void setConditionKey(StringBuilder builder) {
        String sep = (builder.length() == 0)
            ? "" : VTNMatch.COND_KEY_SEPARATOR;

        if (icmpType != null) {
            builder.append(sep).append(FlowMatchType.ICMP_TYPE).append('=').
                append(icmpType);
            sep = VTNMatch.COND_KEY_SEPARATOR;
        }

        if (icmpCode != null) {
            builder.append(sep).append(FlowMatchType.ICMP_CODE).append('=').
                append(icmpCode);
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public boolean isEmpty() {
        return (icmpType == null && icmpCode == null);
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
        if (o == null || !getClass().equals(o.getClass())) {
            return false;
        }

        VTNIcmpMatch im = (VTNIcmpMatch)o;
        return (Objects.equals(icmpType, im.icmpType) &&
                Objects.equals(icmpCode, im.icmpCode));
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        return Objects.hash(icmpType, icmpCode);
    }
}
