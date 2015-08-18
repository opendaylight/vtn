/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.flow.match;

import org.opendaylight.vtn.manager.util.EtherAddress;
import org.opendaylight.vtn.manager.util.NumberUtils;

import org.opendaylight.vtn.manager.internal.util.packet.EtherHeader;

import org.opendaylight.vtn.manager.internal.TestBase;
import org.opendaylight.vtn.manager.internal.XmlNode;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.vtn.match.fields.VtnEtherMatch;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.vtn.match.fields.VtnEtherMatchBuilder;

import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.rev131026.flow.MatchBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.model.match.types.rev131026.ethernet.match.fields.EthernetDestination;
import org.opendaylight.yang.gen.v1.urn.opendaylight.model.match.types.rev131026.ethernet.match.fields.EthernetSource;
import org.opendaylight.yang.gen.v1.urn.opendaylight.model.match.types.rev131026.ethernet.match.fields.EthernetType;
import org.opendaylight.yang.gen.v1.urn.opendaylight.model.match.types.rev131026.match.EthernetMatch;
import org.opendaylight.yang.gen.v1.urn.opendaylight.model.match.types.rev131026.match.VlanMatch;

import org.opendaylight.yang.gen.v1.urn.opendaylight.l2.types.rev130827.EtherType;
import org.opendaylight.yang.gen.v1.urn.opendaylight.l2.types.rev130827.VlanId;
import org.opendaylight.yang.gen.v1.urn.opendaylight.l2.types.rev130827.VlanPcp;

/**
 * {@code EtherMatchParams} describes parameters for conditions to match
 * against Ethernet header.
 */
public final class EtherMatchParams extends TestBase
    implements EtherHeader, Cloneable {
    /**
     * The source MAC address.
     */
    private EtherAddress  sourceAddress;

    /**
     * The destination MAC address.
     */
    private EtherAddress  destinationAddress;

    /**
     * Ethernet type.
     */
    private Integer  etherType;

    /**
     * VLAN ID.
     */
    private Integer  vlanId;

    /**
     * Vlan priority.
     */
    private Short  vlanPriority;

    /**
     * Set the source MAC address.
     *
     * @param src  A long integer value which represents the source MAC
     *             address.
     * @return  This instance.
     */
    public EtherMatchParams setSourceAddress(long src) {
        sourceAddress = new EtherAddress(src);
        return this;
    }

    /**
     * Set the source MAC address.
     *
     * @param src  A byte array which represents the source MAC address.
     * @return  This instance.
     */
    public EtherMatchParams setSourceAddress(byte[] src) {
        sourceAddress = new EtherAddress(src);
        return this;
    }

    /**
     * Set the destination MAC address.
     *
     * @param dst  A long integer value which represents the destination MAC
     *             address.
     * @return  This instance.
     */
    public EtherMatchParams setDestinationAddress(long dst) {
        destinationAddress = new EtherAddress(dst);
        return this;
    }

    /**
     * Set the destination MAC address.
     *
     * @param dst  A byte array which represents the destination MAC address.
     * @return  This instance.
     */
    public EtherMatchParams setDestinationAddress(byte[] dst) {
        destinationAddress = new EtherAddress(dst);
        return this;
    }

    /**
     * Set the Ethernet type.
     *
     * @param type  Ethernet type.
     * @return  This instance.
     */
    public EtherMatchParams setEtherType(Integer type) {
        etherType = type;
        return this;
    }

    /**
     * Set the VLAN ID.
     *
     * @param vid  VLAN ID.
     * @return  This instance.
     */
    public EtherMatchParams setVlanId(Integer vid) {
        vlanId = vid;
        return this;
    }

    /**
     * Set the VLAN ID.
     *
     * @param vid  VLAN ID.
     * @return  This instance.
     */
    public EtherMatchParams setVlanId(Short vid) {
        vlanId = NumberUtils.toInteger(vid);
        return this;
    }

    /**
     * Set the VLAN priority.
     *
     * @param pcp  VLAN priority.
     * @return  This instance.
     */
    public EtherMatchParams setVlanPriority(Short pcp) {
        vlanPriority = pcp;
        return this;
    }

    /**
     * Set the VLAN priority.
     *
     * @param pcp  VLAN priority.
     * @return  This instance.
     */
    public EtherMatchParams setVlanPriority(Byte pcp) {
        vlanPriority = NumberUtils.toShort(pcp);
        return this;
    }

    /**
     * Reset to the initial state.
     *
     * @return  This instance.
     */
    public EtherMatchParams reset() {
        sourceAddress = null;
        destinationAddress = null;
        etherType = null;
        vlanId = null;
        vlanPriority = null;
        return this;
    }

    /**
     * Construct an
     * {@link org.opendaylight.vtn.manager.flow.cond.EthernetMatch} instance.
     *
     * @return  An {@link org.opendaylight.vtn.manager.flow.cond.EthernetMatch}
     *          instance.
     */
    public org.opendaylight.vtn.manager.flow.cond.EthernetMatch toEthernetMatch() {
        Short vid = NumberUtils.toShort(vlanId);
        Byte pcp = NumberUtils.toByte(vlanPriority);
        return new org.opendaylight.vtn.manager.flow.cond.EthernetMatch(
            sourceAddress, destinationAddress, etherType, vid, pcp);
    }

    /**
     * Construct a {@link VtnEtherMatch} instance.
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
            Long et = NumberUtils.toLong(etherType);
            builder.setEtherType(new EtherType(et));
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
     * Construct a new {@link VTNEtherMatch} instance.
     *
     * @return  A {@link VTNEtherMatch} instance.
     * @throws Exception  An error occurred.
     */
    public VTNEtherMatch toVTNEtherMatch() throws Exception {
        return new VTNEtherMatch(toVtnEtherMatch());
    }

    /**
     * Return a {@link XmlNode} instance which represents this instance.
     *
     * @param name  The name of the root node.
     * @return  A {@link XmlNode} instance.
     */
    public XmlNode toXmlNode(String name) {
        XmlNode root = new XmlNode(name);
        if (sourceAddress != null) {
            root.add(new XmlNode("source-address", sourceAddress.getText()));
        }

        if (destinationAddress != null) {
            root.add(new XmlNode("destination-address",
                                 destinationAddress.getText()));
        }

        if (etherType != null) {
            root.add(new XmlNode("ether-type", etherType));
        }

        if (vlanId != null) {
            root.add(new XmlNode("vlan-id", vlanId));
        }

        if (vlanPriority != null) {
            root.add(new XmlNode("vlan-pcp", vlanPriority));
        }

        return root;
    }

    /**
     * Ensure that the given {@link VTNEtherMatch} instance contains the same
     * Ethernet conditions as this instance.
     *
     * @param ematch  A {@link VTNEtherMatch} instance.
     */
    public void verifyValues(VTNEtherMatch ematch) {
        assertEquals(sourceAddress, ematch.getSourceAddress());
        assertEquals(destinationAddress, ematch.getDestinationAddress());
        assertEquals(etherType, ematch.getEtherType());
        assertEquals(vlanId, ematch.getVlanId());
        assertEquals(vlanPriority, ematch.getVlanPriority());
        assertEquals(isEmpty(), ematch.isEmpty());
    }

    /**
     * Ensure that the given {@link VTNEtherMatch} instance contains the same
     * Ethernet conditions as this instance.
     *
     * @param ematch  A {@link VTNEtherMatch} instance.
     * @throws Exception  An error occurred.
     */
    public void verify(VTNEtherMatch ematch) throws Exception {
        verifyValues(ematch);

        org.opendaylight.vtn.manager.flow.cond.EthernetMatch em =
            ematch.toEthernetMatch();
        assertEquals(sourceAddress, em.getSourceEtherAddress());
        assertEquals(destinationAddress, em.getDestinationEtherAddress());
        assertEquals(etherType, em.getType());
        assertEquals(NumberUtils.toShort(vlanId), em.getVlan());
        assertEquals(NumberUtils.toByte(vlanPriority), em.getVlanPriority());

        MatchBuilder mb = new MatchBuilder();
        ematch.setMatch(mb);
        boolean hasValue = verify(mb);

        VTNEtherMatch ematch1 = VTNEtherMatch.create(mb.build());
        assertEquals((hasValue) ? ematch : null, ematch1);
    }

    /**
     * Ensure that the given {@link MatchBuilder} instance contains the same
     * Ethernet conditions as this instance.
     *
     * @param builder  A {@link MatchBuilder} instance.
     * @return  {@code true} only if the given match contains the condition for
     *          the Ethernet header.
     */
    public boolean verify(MatchBuilder builder) {
        boolean isEther = false;
        boolean isVlan = false;
        EthernetMatch ematch = builder.getEthernetMatch();
        if (ematch == null) {
            assertEquals(null, sourceAddress);
            assertEquals(null, destinationAddress);
            assertEquals(null, etherType);
        } else {
            if (sourceAddress == null) {
                assertEquals(null, ematch.getEthernetSource());
            } else {
                EthernetSource src = ematch.getEthernetSource();
                assertEquals(sourceAddress.getMacAddress(), src.getAddress());
                assertEquals(null, src.getMask());
                isEther = true;
            }

            if (destinationAddress == null) {
                assertEquals(null, ematch.getEthernetDestination());
            } else {
                EthernetDestination dst = ematch.getEthernetDestination();
                assertEquals(destinationAddress.getMacAddress(),
                             dst.getAddress());
                assertEquals(null, dst.getMask());
                isEther = true;
            }

            if (etherType == null) {
                assertEquals(null, ematch.getEthernetType());
            } else {
                EthernetType etype = ematch.getEthernetType();
                assertEquals(etype.getType().getValue(),
                             NumberUtils.toLong(etherType));
                isEther = true;
            }
            assertTrue(isEther);
        }

        VlanMatch vmatch = builder.getVlanMatch();
        if (vmatch == null) {
            assertEquals(null, vlanId);
            assertEquals(null, vlanPriority);
        } else {
            if (vlanId == null) {
                assertEquals(null, vmatch.getVlanId());
            } else {
                org.opendaylight.yang.gen.v1.urn.opendaylight.model.match.types.rev131026.vlan.match.fields.VlanId tag =
                    vmatch.getVlanId();
                int vid = vlanId.intValue();
                boolean present = (vid != VLAN_NONE);
                assertEquals(vlanId, tag.getVlanId().getValue());
                assertEquals(Boolean.valueOf(present), tag.isVlanIdPresent());
                isVlan = true;
            }

            if (vlanPriority == null) {
                assertEquals(null, vmatch.getVlanPcp());
            } else {
                VlanPcp pcp = vmatch.getVlanPcp();
                assertEquals(vlanPriority, pcp.getValue());
                isVlan = true;
            }
            assertTrue(isVlan);
        }

        return (isEther || isVlan);
    }

    /**
     * Determine whether this instance does not specify any Ethernet header
     * field or not.
     *
     * @return  {@code true} only if this instance is empty.
     */
    public boolean isEmpty() {
        return (sourceAddress == null && destinationAddress == null &&
                etherType == null && vlanId == null && vlanPriority == null);
    }

    // EtherHeader

    /**
     * {@inheritDoc}
     */
    @Override
    public EtherAddress getSourceAddress() {
        return sourceAddress;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void setSourceAddress(EtherAddress mac) {
        sourceAddress = mac;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public EtherAddress getDestinationAddress() {
        return destinationAddress;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void setDestinationAddress(EtherAddress mac) {
        destinationAddress = mac;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public int getEtherType() {
        return etherType.intValue();
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public int getVlanId() {
        return (vlanId == null) ? VLAN_NONE : vlanId.intValue();
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void setVlanId(int vid) {
        vlanId = Integer.valueOf(vid);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public short getVlanPriority() {
        return (vlanPriority == null) ? -1 : vlanPriority.shortValue();
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void setVlanPriority(short pcp) {
        vlanPriority = Short.valueOf(pcp);
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
    public EtherMatchParams clone() {
        try {
            return (EtherMatchParams)super.clone();
        } catch (CloneNotSupportedException e) {
            // This should never happen.
            throw new IllegalStateException("clone() failed.", e);
        }
    }
}
