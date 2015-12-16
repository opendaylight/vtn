/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.util.packet;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertTrue;

import java.util.Collections;
import java.util.HashSet;
import java.util.Set;

import org.opendaylight.vtn.manager.packet.Ethernet;
import org.opendaylight.vtn.manager.packet.IEEE8021Q;
import org.opendaylight.vtn.manager.packet.Packet;
import org.opendaylight.vtn.manager.util.EtherAddress;
import org.opendaylight.vtn.manager.util.EtherTypes;
import org.opendaylight.vtn.manager.util.IpNetwork;

import org.opendaylight.vtn.manager.it.ofmock.OfMockService;
import org.opendaylight.vtn.manager.it.util.ModelDrivenTestBase;
import org.opendaylight.vtn.manager.it.util.TestBase;
import org.opendaylight.vtn.manager.it.util.match.FlowMatchType;

import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.rev131026.flow.MatchBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.model.match.types.rev131026.match.EthernetMatchBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.model.match.types.rev131026.match.VlanMatchBuilder;

/**
 * {@code EthernetFactory} is a utility class used to create or to verify
 * an Ethernet frame.
 */
public final class EthernetFactory extends PacketFactory {
    /**
     * The source MAC address.
     */
    private EtherAddress  sourceAddress;

    /**
     * The destination MAC address.
     */
    private EtherAddress  destinationAddress;

    /**
     * The ethernet type.
     */
    private int  etherType;

    /**
     * The VLAN ID.
     */
    private int  vlanId;

    /**
     * The VLAN priority.
     */
    private short  vlanPcp;

    /**
     * IP address to be probed.
     */
    private IpNetwork  probeAddress;

    /**
     * VLAN ID used for IP address probe.
     */
    private int  probeVlan = -1;

    /**
     * Construct a new instance.
     */
    public EthernetFactory() {}

    /**
     * Construct a new instance.
     *
     * @param src  The source MAC address.
     * @param dst  The destination MAC address.
     */
    public EthernetFactory(EtherAddress src, EtherAddress dst) {
        setSourceAddress(src);
        setDestinationAddress(dst);
    }

    /**
     * Return the source MAC address.
     *
     * @return  An {@link EtherAddress} instance that represents the
     *          source MAC address or {@code null}.
     */
    public EtherAddress getSourceAddress() {
        return sourceAddress;
    }

    /**
     * Return the destination MAC address.
     *
     * @return  An {@link EtherAddress} instance that represents the
     *          destination MAC address or {@code null}.
     */
    public EtherAddress getDestinationAddress() {
        return destinationAddress;
    }

    /**
     * Return the ethernet type.
     *
     * @return  Ethernet type.
     */
    public int getEtherType() {
        return etherType;
    }

    /**
     * Return the VLAN ID.
     *
     * @return  VLAN ID. Zero means an untagged frame.
     */
    public int getVlanId() {
        return vlanId;
    }

    /**
     * Return the VLAN priority.
     *
     * @return  VLAN priority.
     */
    public short getVlanPcp() {
        return vlanPcp;
    }

    /**
     * Return an IP address to be probed.
     *
     * @return  An {@link IpNetwork} instance or {@code null}.
     */
    public IpNetwork getProbeAddress() {
        return probeAddress;
    }

    /**
     * Return a VLAN ID used for IP address probe.
     *
     * @return  VLAN ID used for IP address probe.
     */
    public int getProbeVlan() {
        return probeVlan;
    }

    /**
     * Set the source MAC address.
     *
     * @param mac  An {@link EtherAddress} instance that represents the
     *             source MAC address.
     * @return  This instance.
     */
    public EthernetFactory setSourceAddress(EtherAddress mac) {
        sourceAddress = mac;
        return this;
    }

    /**
     * Set the destination MAC address.
     *
     * @param mac  An {@link EtherAddress} instance that represents the
     *             destination MAC address.
     * @return  This instance.
     */
    public EthernetFactory setDestinationAddress(EtherAddress mac) {
        destinationAddress = mac;
        return this;
    }

    /**
     * Set the ethernet type.
     *
     * @param type  Ethernet type.
     * @return  This instance.
     */
    public EthernetFactory setEtherType(int type) {
        etherType = type;
        return this;
    }

    /**
     * Set the VLAN ID.
     *
     * @param vid  VLAN ID. Zero means an untagged frame.
     * @return  This instance.
     */
    public EthernetFactory setVlanId(int vid) {
        vlanId = vid;
        return this;
    }

    /**
     * Set the VLAN priority.
     *
     * @param pcp  VLAN priority.
     * @return  This instance.
     */
    public EthernetFactory setVlanPcp(short pcp) {
        vlanPcp = pcp;
        return this;
    }

    /**
     * Set a pair of IP address and VLAN ID for IP address probe.
     *
     * @param ip   An {@link IpNetwork} instance.
     * @param vid  VLAN ID used for IP address probe.
     * @return  This instance.
     */
    public EthernetFactory setProbe(IpNetwork ip, int vid) {
        probeAddress = ip;
        probeVlan = vid;
        return this;
    }

    /**
     * Verify the packet.
     *
     * @param ofmock  openflowplugin mock-up service.
     * @param bytes    A byte array which represents the raw packet image.
     * @return  This instance.
     * @throws Exception  An error occurred.
     */
    public EthernetFactory verify(OfMockService ofmock, byte[] bytes)
        throws Exception {
        Set<Integer> vlanIds = Collections.singleton(vlanId);
        Set<Integer> vids = verify(ofmock, bytes, vlanIds);
        assertEquals(Collections.<Integer>emptySet(), vids);
        return this;
    }

    /**
     * Verify the packet.
     *
     * @param ofmock   openflowplugin mock-up service.
     * @param bytes    A byte array which represents the raw packet image.
     * @param vlanIds  A set of VLAN IDs expected to be configured in the
     *                 packet.
     * @return  A set of VLAN IDs passed to {@code vlanIds} eliminating VLAN ID
     *          detected in {@code bytes}.
     * @throws Exception  An error occurred.
     */
    public Set<Integer> verify(OfMockService ofmock, byte[] bytes,
                               Set<Integer> vlanIds) throws Exception {
        assertNotNull(bytes);

        Ethernet eth = new Ethernet();
        eth.deserialize(bytes, 0, bytes.length * Byte.SIZE);

        int vid;
        int type = (int)eth.getEtherType();
        Packet payload = eth.getPayload();
        Packet parent;
        if (type == EtherTypes.VLAN.shortValue()) {
            assertTrue(payload instanceof IEEE8021Q);
            IEEE8021Q vtag = (IEEE8021Q)payload;
            vid = (int)vtag.getVid();
            assertEquals((byte)0, vtag.getCfi());
            assertEquals((byte)vlanPcp, vtag.getPcp());
            type = (int)vtag.getEtherType();
            payload = vtag.getPayload();
            parent = vtag;
        } else {
            vid = 0;
            parent = eth;
        }

        if (vid == probeVlan) {
            // The given packet should be an ARP request.
            EtherAddress ctlrMac = ofmock.getControllerMacAddress();
            EthernetFactory efc = new EthernetFactory(ctlrMac, sourceAddress).
                setVlanId(vid);
            ArpFactory afc = ArpFactory.newInstance(efc);
            afc.setSenderHardwareAddress(ctlrMac.getBytes()).
                setTargetHardwareAddress(sourceAddress.getBytes()).
                setSenderProtocolAddress(TestBase.IPV4_ZERO.getBytes()).
                setTargetProtocolAddress(probeAddress.getBytes());
            return efc.verify(ofmock, bytes, vlanIds);
        }

        Set<Integer> vids = new HashSet<>(vlanIds);
        assertTrue(vids.remove(vid));
        assertEquals(sourceAddress,
                     EtherAddress.create(eth.getSourceMACAddress()));
        assertEquals(destinationAddress,
                     EtherAddress.create(eth.getDestinationMACAddress()));
        assertEquals(etherType, type);

        PacketFactory f = this;
        while (payload != null) {
            f = f.getNextFactory();
            assertNotNull(f);
            f.verify(payload);
            parent = payload;
            payload = payload.getPayload();
        }

        assertEquals(null, f.getNextFactory());
        f.assertRawPayload(parent.getRawPayload());

        return vids;
    }

    // PacketFactory

    /**
     * {@inheritDoc}
     */
    @Override
    Packet createPacket() {
        Ethernet eth = new Ethernet().
            setSourceMACAddress(sourceAddress.getBytes()).
            setDestinationMACAddress(destinationAddress.getBytes());
        if (vlanId == 0) {
            eth.setEtherType((short)etherType);
        } else {
            IEEE8021Q vtag = new IEEE8021Q().setCfi((byte)0).
                setPcp((byte)vlanPcp).setVid((short)vlanId).
                setEtherType((short)etherType);
            eth.setEtherType(EtherTypes.VLAN.shortValue()).
                setPayload(vtag);
        }

        return eth;
    }

    /**
     * This method should never be called.
     *
     * @param packet  Unused.
     * @throws IllegalStateException  Always thrown.
     */
    @Override
    void verify(Packet packet) {
        throw new IllegalStateException("Should never be called.");
    }

    /**
     * {@inheritDoc}
     */
    @Override
    int initMatch(MatchBuilder builder, Set<FlowMatchType> types) {
        // Source and destination MAC address are mandatory.
        int etype;
        int count;
        if (types.contains(FlowMatchType.ETH_TYPE)) {
            etype = etherType;
            count = 1;
        } else {
            etype = 0;
            count = 0;
        }

        EthernetMatchBuilder emb = ModelDrivenTestBase.
            createEthernetMatch(sourceAddress, destinationAddress, etype);

        // VLAN ID is mandatory.
        Short pcp;
        if (types.contains(FlowMatchType.VLAN_PCP)) {
            pcp = vlanPcp;
            count++;
        } else {
            pcp = null;
        }
        VlanMatchBuilder vmb =
            ModelDrivenTestBase.createVlanMatch(vlanId, pcp);
        builder.setEthernetMatch(emb.build()).setVlanMatch(vmb.build());

        return count;
    }

    // Object

    /**
     * {@inheritDoc}
     */
    @Override
    public EthernetFactory clone() {
        return (EthernetFactory)super.clone();
    }
}
