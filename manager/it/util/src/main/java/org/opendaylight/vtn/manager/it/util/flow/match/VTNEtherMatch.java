/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.util.flow.match;

import static org.junit.Assert.assertEquals;

import org.opendaylight.vtn.manager.util.EtherAddress;
import org.opendaylight.vtn.manager.util.NumberUtils;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.vtn.match.fields.VtnEtherMatch;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.vtn.match.fields.VtnEtherMatchBuilder;

import org.opendaylight.yang.gen.v1.urn.opendaylight.l2.types.rev130827.EtherType;
import org.opendaylight.yang.gen.v1.urn.opendaylight.l2.types.rev130827.VlanId;
import org.opendaylight.yang.gen.v1.urn.opendaylight.l2.types.rev130827.VlanPcp;

/**
 * {@code VTNEtherMatch} describes the condition for ethernet header to match
 * against packets.
 */
public final class VTNEtherMatch implements Cloneable {
    /**
     * The source MAC address to match.
     */
    private EtherAddress  sourceAddress;

    /**
     * The destination MAC address to match.
     */
    private EtherAddress  destinationAddress;

    /**
     * The ethernet type to match.
     */
    private Integer  etherType;

    /**
     * The VLAN ID to match.
     */
    private Integer  vlanId;

    /**
     * The VLAN priority to match.
     */
    private Short  vlanPriority;

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
     * @param vid   The VLAN ID to match. Zero matches untagged frame.
     *              {@code null} matches every VLAN frame, including untagged
     *              frame.
     * @param pcp   The VLAN priority to match. {@code null} matches every
     *              VLAN priority.
     */
    public VTNEtherMatch(EtherAddress src, EtherAddress dst, Integer type,
                         Integer vid, Short pcp) {
        sourceAddress = src;
        destinationAddress = dst;
        etherType = type;
        vlanId = vid;
        vlanPriority = pcp;
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
     * Set the source MAC address to match against packets.
     *
     * @param eaddr  An {@link EtherAddress} instance that specifies the
     *               source MAC address to match.
     *               {@code null} matches every source MAC address.
     * @return  This instance.
     */
    public VTNEtherMatch setSourceAddress(EtherAddress eaddr) {
        sourceAddress = eaddr;
        return this;
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
     * Set the destination MAC address to match against packets.
     *
     * @param eaddr  An {@link EtherAddress} instance that specifies the
     *               destination MAC address to match.
     *               {@code null} matches every destination MAC address.
     * @return  This instance.
     */
    public VTNEtherMatch setDestinationAddress(EtherAddress eaddr) {
        destinationAddress = eaddr;
        return this;
    }

    /**
     * Return the Ethernet type to match against packets.
     *
     * @return  An {@link Integer} instance if the ethernet type is specified.
     *          {@code null} if not specified.
     */
    public Integer getEtherType() {
        return etherType;
    }

    /**
     * Set the Ethernet type to match against packets.
     *
     * @param type  An {@link Integer} instance that specifies the Ethernet
     *              type to match. {@code null} matches every Ethernet type.
     * @return  This instance.
     */
    public VTNEtherMatch setEtherType(Integer type) {
        etherType = type;
        return this;
    }

    /**
     * Return the VLAN ID to match against packets.
     *
     * @return  A {@link Integer} instance if the VLAN ID is specified.
     *          Note that zero matches packets that have no VLAN tag.
     *          {@code null} if the VLAN ID is not specified.
     */
    public Integer getVlanId() {
        return vlanId;
    }

    /**
     * Set the VLAN ID to match against packets.
     *
     * @param vid  An {@link Integer} instance that specifies the VLAN ID
     *             to match. Zero matches untagged frame.
     *             {@code null} matches every VLAN, including untagged frame.
     * @return  This instance.
     */
    public VTNEtherMatch setVlanId(Integer vid) {
        vlanId = vid;
        return this;
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
     * Set the VLAN priority to match against packets.
     *
     * @param pri  A {@link Short} instance that specifies the VLAN priority
     *             to match. {@code null} matches every VLAN priority.
     * @return  This instance.
     */
    public VTNEtherMatch setVlanPriority(Short pri) {
        vlanPriority = pri;
        return this;
    }

    /**
     * Return a {@link VtnEtherMatch} instance which contains the flow
     * conditions configured in this instance.
     *
     * @return  A {@link VtnEtherMatch} instance.
     */
    public VtnEtherMatch toVtnEtherMatch() {
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

        return builder.build();
    }

    /**
     * Determine whether this instance is empty or not.
     *
     * @return  {@code true} if this instance is empty.
     *          {@code false} otherwise.
     */
    public boolean isEmpty() {
        boolean ret = (sourceAddress == null && destinationAddress == null);
        if (ret) {
            ret = (etherType == null && vlanId == null &&
                   vlanPriority == null);
        }

        return ret;
    }

    /**
     * Complete match conditions.
     *
     * @param etype  An expected value of Ethernet type.
     * @return  A {@link VTNEtherMatch} instance that contains completed
     *          match conditions.
     */
    public VTNEtherMatch complete(Integer etype) {
        VTNEtherMatch ematch;

        if (etype == null) {
            ematch = this;
        } else if (etherType == null) {
            ematch = clone();
            ematch.etherType = etype;
        } else {
            assertEquals(etype, etherType);
            ematch = this;
        }

        return ematch;
    }

    /**
     * Verify the given vtn-ether-match.
     *
     * @param vematch  A {@link VtnEtherMatch} instance.
     */
    public void verify(VtnEtherMatch vematch) {
        assertEquals(sourceAddress,
                     EtherAddress.create(vematch.getSourceAddress()));
        assertEquals(destinationAddress,
                     EtherAddress.create(vematch.getDestinationAddress()));

        Integer etype = etherType;
        if (etype == null) {
            assertEquals(null, vematch.getEtherType());
        } else {
            assertEquals(etype.longValue(),
                         vematch.getEtherType().getValue().longValue());
        }

        if (vlanId == null) {
            assertEquals(null, vematch.getVlanId());
            assertEquals(null, vematch.getVlanPcp());
        } else {
            assertEquals(vlanId, vematch.getVlanId().getValue());
            if (vlanPriority == null) {
                assertEquals(null, vematch.getVlanPcp());
            } else {
                assertEquals(vlanPriority, vematch.getVlanPcp().getValue());
            }
        }
    }

    // Object

    /**
     * Create a copy of this instance.
     *
     * @return  A copy of this instance.
     */
    @Override
    public VTNEtherMatch clone() {
        try {
            return (VTNEtherMatch)super.clone();
        } catch (CloneNotSupportedException e) {
            throw new IllegalStateException("Unable to clone.", e);
        }
    }
}
