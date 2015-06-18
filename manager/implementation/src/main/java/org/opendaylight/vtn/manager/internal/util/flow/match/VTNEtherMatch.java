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

import org.opendaylight.vtn.manager.flow.cond.EthernetMatch;
import org.opendaylight.vtn.manager.util.EtherAddress;
import org.opendaylight.vtn.manager.util.NumberUtils;

import org.opendaylight.vtn.manager.internal.util.MiscUtils;
import org.opendaylight.vtn.manager.internal.util.ProtocolUtils;
import org.opendaylight.vtn.manager.internal.util.packet.EtherHeader;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcErrorTag;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;

import org.opendaylight.controller.sal.packet.address.EthernetAddress;
import org.opendaylight.controller.sal.utils.Status;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.VtnEtherMatchFields;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.vtn.match.fields.VtnEtherMatchBuilder;

import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.rev131026.flow.MatchBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.model.match.types.rev131026.EthernetMatchFields;
import org.opendaylight.yang.gen.v1.urn.opendaylight.model.match.types.rev131026.MacAddressFilter;
import org.opendaylight.yang.gen.v1.urn.opendaylight.model.match.types.rev131026.Match;
import org.opendaylight.yang.gen.v1.urn.opendaylight.model.match.types.rev131026.VlanMatchFields;
import org.opendaylight.yang.gen.v1.urn.opendaylight.model.match.types.rev131026.ethernet.match.fields.EthernetDestination;
import org.opendaylight.yang.gen.v1.urn.opendaylight.model.match.types.rev131026.ethernet.match.fields.EthernetDestinationBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.model.match.types.rev131026.ethernet.match.fields.EthernetSource;
import org.opendaylight.yang.gen.v1.urn.opendaylight.model.match.types.rev131026.ethernet.match.fields.EthernetSourceBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.model.match.types.rev131026.ethernet.match.fields.EthernetType;
import org.opendaylight.yang.gen.v1.urn.opendaylight.model.match.types.rev131026.ethernet.match.fields.EthernetTypeBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.model.match.types.rev131026.match.EthernetMatchBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.model.match.types.rev131026.match.VlanMatchBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.model.match.types.rev131026.vlan.match.fields.VlanIdBuilder;

import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.yang.types.rev100924.MacAddress;
import org.opendaylight.yang.gen.v1.urn.opendaylight.l2.types.rev130827.EtherType;
import org.opendaylight.yang.gen.v1.urn.opendaylight.l2.types.rev130827.VlanId;
import org.opendaylight.yang.gen.v1.urn.opendaylight.l2.types.rev130827.VlanPcp;

/**
 * {@code VTNEtherMatch} describes the condition for ethernet header to match
 * against packets.
 *
 * <p>
 *   Note that this class is not synchronized.
 * </p>
 */
@XmlRootElement(name = "vtn-ether-match")
@XmlAccessorType(XmlAccessType.NONE)
public final class VTNEtherMatch {
    /**
     * The source MAC address to match.
     */
    @XmlElement(name = "source-address")
    private EtherAddress  sourceAddress;

    /**
     * The destination MAC address to match.
     */
    @XmlElement(name = "destination-address")
    private EtherAddress  destinationAddress;

    /**
     * The ethernet type to match.
     */
    @XmlElement(name = "ether-type")
    private Integer  etherType;

    /**
     * The VLAN ID to match.
     */
    @XmlElement(name = "vlan-id")
    private Integer  vlanId;

    /**
     * The VLAN priority to match.
     */
    @XmlElement(name = "vlan-pcp")
    private Short  vlanPriority;

    /**
     * Create a new instance from the given MD-SAL flow match.
     *
     * @param match  A MD-SAL flow match.
     * @return  A {@link VTNEtherMatch} instance or {@code null}.
     * @throws NullPointerException
     *    {@code match} is {@code null}.
     * @throws RpcException
     *    {@code match} contains invalid condition against Ethernet header.
     */
    public static VTNEtherMatch create(Match match) throws RpcException {
        EthernetMatchFields eth = match.getEthernetMatch();
        VlanMatchFields vlan = match.getVlanMatch();
        return (eth != null || vlan != null)
            ? new VTNEtherMatch(eth, vlan) : null;
    }

    /**
     * Construct a new instance that matches every packet.
     */
    public VTNEtherMatch() {
    }

    /**
     * Construct a new instance that matches the given Ethernet type.
     *
     * @param  type  An {@link Integer} instance whcih represents the ethernet
     *               type to match against packets.
     *               {@code null} matches every ethernet type.
     */
    public VTNEtherMatch(Integer type) {
        etherType = type;
    }

    /**
     * Construct a new instance.
     *
     * @param src   An {@link EtherAddress} instance which specifies the
     *              source MAC address. {@code null} matches every source
     *              MAC address.
     * @param dst   An {@link EtherAddress} instance which specifies the
     *              destination MAC address. {@code null} matches every
     *              destination MAC address.
     * @param type  An {@link Integer} instance whcih specifies the ethernet
     *              type. {@code null} matches every ethernet type.
     * @param vid   The VLAN ID to match. {@link EtherHeader#VLAN_NONE} matches
     *              untagged frame. {@code null} matches every VLAN frame,
     *              including untagged frame.
     * @param pcp   The VLAN priority to match. {@code null} matches every
     *              VLAN priority.
     * @throws RpcException
     *    The specified condition is invalid.
     */
    public VTNEtherMatch(EtherAddress src, EtherAddress dst, Integer type,
                         Integer vid, Short pcp) throws RpcException {
        sourceAddress = src;
        destinationAddress = dst;
        etherType = type;
        vlanId = vid;
        vlanPriority = pcp;
        verify();
    }

    /**
     * Construct a new instance from the given {@link EthernetMatch} instance.
     *
     * @param ematch  An {@link EthernetMatch} instance.
     * @throws NullPointerException
     *    {@code ematch} is {@code null}.
     * @throws RpcException
     *    {@code ematch} contains invalid value.
     */
    public VTNEtherMatch(EthernetMatch ematch) throws RpcException {
        Status st = ematch.getValidationStatus();
        if (st != null) {
            throw new RpcException(RpcErrorTag.BAD_ELEMENT, st);
        }

        sourceAddress = EtherAddress.create(ematch.getSourceAddress());
        destinationAddress =
            EtherAddress.create(ematch.getDestinationAddress());
        etherType = ematch.getType();
        vlanId = NumberUtils.toInteger(ematch.getVlan());
        vlanPriority = NumberUtils.toShort(ematch.getVlanPriority());
        verify();
    }

    /**
     * Construct a new instance from the given {@link VtnEtherMatchFields}.
     *
     * @param vematch  A {@link VtnEtherMatchFields} instance.
     * @throws NullPointerException
     *    {@code vematch} is {@code null}.
     * @throws RpcException
     *    {@code vematch} contains invalid value.
     */
    public VTNEtherMatch(VtnEtherMatchFields vematch) throws RpcException {
        sourceAddress = getMacAddress(vematch.getSourceAddress(),
                                      VTNMatch.DESC_SRC);
        destinationAddress = getMacAddress(vematch.getDestinationAddress(),
                                           VTNMatch.DESC_DST);
        etherType = ProtocolUtils.getEtherType(vematch.getEtherType());
        vlanId = ProtocolUtils.getVlanId(vematch.getVlanId());
        vlanPriority = ProtocolUtils.getVlanPriority(vematch.getVlanPcp());
        if (vlanPriority != null) {
            checkVlanPriority();
        }
    }

    /**
     * Construct a new instance from the given {@link EthernetMatchFields} and
     * {@link VlanMatchFields} instances.
     *
     * @param ematch  A {@link EthernetMatchFields} instance.
     * @param vmatch  A {@link VlanMatchFields} instance.
     * @throws RpcException
     *    {@code ematch} or {@code vmatch} contains invalid value.
     */
    public VTNEtherMatch(EthernetMatchFields ematch, VlanMatchFields vmatch)
        throws RpcException {
        if (ematch != null) {
            sourceAddress = getMacAddress(ematch.getEthernetSource(),
                                          VTNMatch.DESC_SRC);
            destinationAddress = getMacAddress(ematch.getEthernetDestination(),
                                               VTNMatch.DESC_DST);
            etherType = ProtocolUtils.getEtherType(ematch.getEthernetType());
        }

        if (vmatch != null) {
            initVlanId(vmatch.getVlanId());
            vlanPriority = ProtocolUtils.getVlanPriority(vmatch.getVlanPcp());
            if (vlanPriority != null) {
                checkVlanPriority();
            }
        }
    }

    /**
     * Return the source MAC address to match against packets.
     *
     * @return  An {@link EtherAddress} instance if the source MAC address
     *          is specified. {@code null} if not specified.
     */
    public EtherAddress getSourceAddress() {
        return sourceAddress;
    }

    /**
     * Return the destination MAC address to match against packets.
     *
     * @return  An {@link EtherAddress} instance if the destination MAC address
     *          is specified. {@code null} if not specified.
     */
    public EtherAddress getDestinationAddress() {
        return destinationAddress;
    }

    /**
     * Return the ethernet type to match against packets.
     *
     * @return  An {@link Integer} instance if the ethernet type is specified.
     *          {@code null} if not specified.
     */
    public Integer getEtherType() {
        return etherType;
    }

    /**
     * Set the ethernet type to match against packets.
     *
     * @param  type  An {@link Integer} instance whcih represents the ethernet
     *               type to match against packets.
     *               {@code null} matches every ethernet type.
     * @throws RpcException
     *    The Ethernet type different from the given type is already configured
     *    in this instance.
     */
    public void setEtherType(Integer type) throws RpcException {
        if (etherType != null && !etherType.equals(type)) {
            StringBuilder builder =
                new StringBuilder("Ethernet type conflict: type=0x");
            builder.append(Integer.toHexString(etherType.intValue())).
                append(", expected=0x").
                append(Integer.toHexString(type.intValue()));
            String msg = builder.toString();
            throw RpcException.getBadArgumentException(msg);
        }
        etherType = type;
    }

    /**
     * Return the VLAN ID to match against packets.
     *
     * @return  A {@link Integer} instance if the VLAN ID is specified.
     *          Note that {@link EtherHeader#VLAN_NONE} matches packets
     *          that have no VLAN tag.
     *          {@code null} if the VLAN ID is not specified.
     */
    public Integer getVlanId() {
        return vlanId;
    }

    /**
     * Return the VLAN priority to match against packets.
     *
     * @return  A {@link Short} instance if the VLAN priority is specified.
     *          {@code null} if the VLAN priority is not specified.
     */
    public Short getVlanPriority() {
        return vlanPriority;
    }

    /**
     * Return an {@link EthernetMatch} instance which represents this
     * condition.
     *
     * @return  An {@link EthernetMatch} instance.
     */
    public EthernetMatch toEthernetMatch() {
        EthernetAddress src = (sourceAddress == null)
            ? null : sourceAddress.getEthernetAddress();
        EthernetAddress dst = (destinationAddress == null)
            ? null : destinationAddress.getEthernetAddress();
        Short vid = NumberUtils.toShort(vlanId);
        Byte pri = NumberUtils.toByte(vlanPriority);

        return new EthernetMatch(src, dst, etherType, vid, pri);
    }

    /**
     * Return a {@link VtnEtherMatchBuilder} instance which contains the flow
     * conditions configured in this instance.
     *
     * @return  A {@link VtnEtherMatchBuilder} instance.
     */
    public VtnEtherMatchBuilder toVtnEtherMatchBuilder() {
        VtnEtherMatchBuilder builder = new VtnEtherMatchBuilder();
        if (sourceAddress != null) {
            builder.setSourceAddress(sourceAddress.getMacAddress());
        }
        if (destinationAddress != null) {
            builder.setDestinationAddress(destinationAddress.getMacAddress());
        }
        if (etherType != null) {
            Long type = NumberUtils.toLong(etherType);
            builder.setEtherType(new EtherType(type));
        }
        if (vlanId != null) {
            builder.setVlanId(new VlanId(vlanId));
        }
        if (vlanPriority != null) {
            builder.setVlanPcp(new VlanPcp(vlanPriority));
        }

        return builder;
    }

    /**
     * Configure the condition represented by this instance into the given
     * MD-SAL flow match builder.
     *
     * @param builder  A {@link MatchBuilder} instance.
     */
    public void setMatch(MatchBuilder builder) {
        EthernetMatchBuilder ematch = null;
        if (sourceAddress != null) {
            EthernetSource src = new EthernetSourceBuilder().
                setAddress(sourceAddress.getMacAddress()).build();
            ematch = create(ematch).setEthernetSource(src);
        }
        if (destinationAddress != null) {
            EthernetDestination dst = new EthernetDestinationBuilder().
                setAddress(destinationAddress.getMacAddress()).build();
            ematch = create(ematch).setEthernetDestination(dst);
        }
        if (etherType != null) {
            EthernetType etype = new EthernetTypeBuilder().
                setType(new EtherType(NumberUtils.toLong(etherType))).build();
            ematch = create(ematch).setEthernetType(etype);
        }
        if (ematch != null) {
            builder.setEthernetMatch(ematch.build());
        }

        VlanMatchBuilder vmatch = null;
        if (vlanId != null) {
            VlanIdBuilder vidBuilder = new VlanIdBuilder();
            boolean present = (vlanId.intValue() != EtherHeader.VLAN_NONE);
            vidBuilder.setVlanIdPresent(present).setVlanId(new VlanId(vlanId));
            vmatch = create(vmatch).setVlanId(vidBuilder.build());
        }
        if (vlanPriority != null) {
            vmatch = create(vmatch).setVlanPcp(new VlanPcp(vlanPriority));
        }
        if (vmatch != null) {
            builder.setVlanMatch(vmatch.build());
        }
    }

    /**
     * Determine whether the given Ethernet header matches the condition
     * configured in this instance.
     *
     * @param ctx   A {@link FlowMatchContext} instance.
     * @return  {@code true} only if the given Ethernet header matches all
     *          the conditions configured in this instance.
     */
    public boolean match(FlowMatchContext ctx) {
        EtherHeader eth = ctx.getEtherHeader();

        // Check the source and destination MAC addresses.
        if (matchAddress(ctx, eth)) {
            // Check the ether type.
            if (etherType != null) {
                ctx.addMatchField(FlowMatchType.DL_TYPE);
                if (etherType.intValue() != eth.getEtherType()) {
                    return false;
                }
            }

            // Check VLAN ID and priority.
            return matchVlan(ctx, eth);
        }

        return false;
    }

    /**
     * Store strings used to construct flow condition key.
     *
     * @param builder  A {@link StringBuilder} instance which contains strings
     *                 used to construct flow condition key.
     */
    public void setConditionKey(StringBuilder builder) {
        String sep = (builder.length() == 0)
            ? "" : VTNMatch.COND_KEY_SEPARATOR;

        if (sourceAddress != null) {
            builder.append(sep).append(FlowMatchType.DL_SRC).append('=').
                append(sourceAddress.getText());
            sep = VTNMatch.COND_KEY_SEPARATOR;
        }

        if (destinationAddress != null) {
            builder.append(sep).append(FlowMatchType.DL_DST).append('=').
                append(destinationAddress.getText());
            sep = VTNMatch.COND_KEY_SEPARATOR;
        }

        if (etherType != null) {
            builder.append(sep).append(FlowMatchType.DL_TYPE).append('=').
                append(etherType);
            sep = VTNMatch.COND_KEY_SEPARATOR;
        }

        if (vlanId != null) {
            builder.append(sep).append(FlowMatchType.DL_VLAN).append('=').
                append(vlanId);
            sep = VTNMatch.COND_KEY_SEPARATOR;
        }

        if (vlanPriority != null) {
            builder.append(sep).append(FlowMatchType.DL_VLAN_PCP).append('=').
                append(vlanPriority);
        }
    }

    /**
     * Verify the contents of this instance.
     *
     * @throws RpcException  Verifycation failed.
     */
    public void verify() throws RpcException {
        if (etherType != null) {
            ProtocolUtils.checkEtherType(etherType.intValue());
        }
        if (vlanId != null) {
            ProtocolUtils.checkVlan(vlanId.intValue());
        }
        if (vlanPriority != null) {
            ProtocolUtils.checkVlanPriority(vlanPriority.shortValue());
            checkVlanPriority();
        }
    }

    /**
     * Determine whether this instance does not specify any Ethernet header
     * field or not.
     *
     * @return  {@code true} only if this instance is empty.
     */
    public boolean isEmpty() {
        if (sourceAddress != null) {
            return false;
        }

        if (destinationAddress != null) {
            return false;
        }

        if (etherType != null) {
            return false;
        }

        if (vlanId != null) {
            return false;
        }

        return (vlanPriority == null);
    }

    /**
     * Return the MAC address in the given {@link MacAddress} instance.
     *
     * @param mac   A {@link MacAddress} instance.
     * @param desc  A brief description about the given MAC address.
     * @return  An {@link EtherAddress} instance.
     * @throws RpcException
     *    The given MAC address is invalid.
     */
    private EtherAddress getMacAddress(MacAddress mac, String desc)
        throws RpcException {
        // Exception check can be removed when RESTCONF implements the
        // restriction check.
        try {
            return EtherAddress.create(mac);
        } catch (RuntimeException e) {
            String msg = MiscUtils.joinColon(mac, e.getMessage());
            RpcException re = invalidMacAddress(msg, desc);
            re.initCause(e);
            throw re;
        }
    }

    /**
     * Return the MAC address in the given {@link MacAddressFilter} instance.
     *
     * @param mf   A {@link MacAddressFilter} instance.
     * @param desc  A brief description about the given MAC address.
     * @return  An {@link EtherAddress} instance.
     * @throws RpcException
     *    The given MAC address is invalid.
     */
    private EtherAddress getMacAddress(MacAddressFilter mf, String desc)
        throws RpcException {
        // Exception check can be removed when RESTCONF implements the
        // restriction check.
        try {
            return EtherAddress.create(mf);
        } catch (RuntimeException e) {
            String msg = MiscUtils.joinColon(mf, e.getMessage());
            RpcException re = invalidMacAddress(msg, desc);
            re.initCause(e);
            throw re;
        }
    }

    /**
     * Return an {@link RpcException} instance which notifies an invalid MAC
     * address.
     *
     * @param obj   An object which contains invalid MAC address.
     * @param desc  A brief description about the MAC address.
     * @return  An {@link RpcException} instance.
     */
    private RpcException invalidMacAddress(Object obj, String desc) {
        StringBuilder builder = new StringBuilder("Invalid ").
            append(desc).append(" MAC address: ").append(obj);
        return RpcException.getBadArgumentException(builder.toString());
    }

    /**
     * Initialize the condition for VLAN ID from the given MD-SAL
     * VLAN ID instance.
     *
     * @param vid  A MD-SAL VLAN ID instance.
     * @throws RpcException
     *    The given VLAN ID condition is invalid.
     */
    private void initVlanId(org.opendaylight.yang.gen.v1.urn.opendaylight.model.match.types.rev131026.vlan.match.fields.VlanId vid)
        throws RpcException {
        if (vid != null) {
            if (Boolean.TRUE.equals(vid.isVlanIdPresent())) {
                vlanId = ProtocolUtils.getVlanId(vid.getVlanId());
                if (vlanId == null ||
                    vlanId.intValue() == EtherHeader.VLAN_NONE) {
                    String msg = "Unsupported VLAN ID match: " + vid;
                    throw RpcException.getBadArgumentException(msg);
                }
            } else {
                vlanId = Integer.valueOf(EtherHeader.VLAN_NONE);
            }
        }
    }

    /**
     * Verify the VLAN priority value configured in this instance.
     *
     * @throws RpcException  This instance contains invalid value.
     */
    private void checkVlanPriority() throws RpcException {
        if (vlanId == null || vlanId.intValue() <= EtherHeader.VLAN_NONE) {
            String msg = "VLAN priority requires a valid VLAN ID.";
            throw RpcException.getBadArgumentException(msg);
        }
    }

    /**
     * Create an {@link EthernetMatchBuilder} instance.
     *
     * @param ematch  An {@link EthernetMatchBuilder} instance.
     *                {@code null} must be specified on the first call.
     * @return  If {@code ematch} is {@code null}, this method creates a new
     *          {@link EthernetMatchBuilder} instance and returns it.
     *          Otherwise a {@link EthernetMatchBuilder} instance passed to
     *          {@code ematch} is returned.
     */
    private EthernetMatchBuilder create(EthernetMatchBuilder ematch) {
        return (ematch == null) ? new EthernetMatchBuilder() : ematch;
    }

    /**
     * Create a {@link VlanMatchBuilder} instance.
     *
     * @param vmatch  An {@link VlanMatchBuilder} instance.
     *                {@code null} must be specified on the first call.
     * @return  If {@code vmatch} is {@code null}, this method creates a new
     *          {@link VlanMatchBuilder} instance and returns it.
     *          Otherwise a {@link VlanMatchBuilder} instance passed to
     *          {@code vmatch} is returned.
     */
    private VlanMatchBuilder create(VlanMatchBuilder vmatch) {
        return (vmatch == null) ? new VlanMatchBuilder() : vmatch;
    }

    /**
     * Determine whether the given ethernet match contains the same condition
     * for MAC address.
     *
     * @param em  The ethernet match object to be compared.
     * @return  {@code true} only if the given object contains the same
     *          condition for MAC address.
     */
    private boolean equalsAddress(VTNEtherMatch em) {
        return (Objects.equals(sourceAddress, em.sourceAddress) &&
                Objects.equals(destinationAddress, em.destinationAddress));
    }

    /**
     * Determine whether the given ethernet match contains the same condition
     * for VLAN.
     *
     * @param em  The ethernet match object to be compared.
     * @return  {@code true} only if the given object contains the same
     *          condition for VLAN.
     */
    private boolean equalsVlan(VTNEtherMatch em) {
        return (Objects.equals(vlanId, em.vlanId) &&
                Objects.equals(vlanPriority, em.vlanPriority));
    }

    /**
     * Determine whether the given Ethernet header matches the condition
     * for MAC address configured in this instance.
     *
     * @param ctx  A {@link FlowMatchContext} instance.
     * @param eth  An {@link EtherHeader} instance.
     * @return  {@code true} only if the given Ethernet header matches all
     *          the conditions for MAC address configured in this instance.
     */
    private boolean matchAddress(FlowMatchContext ctx, EtherHeader eth) {
        if (sourceAddress != null) {
            ctx.addMatchField(FlowMatchType.DL_SRC);
            if (sourceAddress.getAddress() !=
                eth.getSourceAddress().getAddress()) {
                return false;
            }
        }

        // Check the destination MAC address.
        if (destinationAddress != null) {
            ctx.addMatchField(FlowMatchType.DL_DST);
            if (destinationAddress.getAddress() !=
                eth.getDestinationAddress().getAddress()) {
                return false;
            }
        }

        return true;
    }

    /**
     * Determine whether the given Ethernet header matches the condition
     * for VLAN configured in this instance.
     *
     * @param ctx  A {@link FlowMatchContext} instance.
     * @param eth  An {@link EtherHeader} instance.
     * @return  {@code true} only if the given Ethernet header matches all
     *          the conditions for VLAN configured in this instance.
     */
    private boolean matchVlan(FlowMatchContext ctx, EtherHeader eth) {
        // We don't need to set DL_VLAN into the FlowMatchContext because
        // it is mandatory.
        if (vlanId != null) {
            if (vlanId.intValue() != eth.getVlanId()) {
                return false;
            }

            // Check the VLAN priority only if a VLAN ID is specified.
            if (vlanPriority != null) {
                ctx.addMatchField(FlowMatchType.DL_VLAN_PCP);
                if (vlanPriority.shortValue() != eth.getVlanPriority()) {
                    return false;
                }
            }
        }

        return true;
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

        VTNEtherMatch em = (VTNEtherMatch)o;
        return (equalsAddress(em) && equalsVlan(em) &&
                Objects.equals(etherType, em.etherType));
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        return Objects.hash(sourceAddress, destinationAddress, etherType,
                            vlanId, vlanPriority);
    }
}
