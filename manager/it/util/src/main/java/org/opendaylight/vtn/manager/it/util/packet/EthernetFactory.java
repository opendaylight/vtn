/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.util.packet;

import static org.junit.Assert.assertArrayEquals;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertTrue;

import java.net.InetAddress;
import java.util.Collections;
import java.util.HashSet;
import java.util.Set;

import org.opendaylight.vtn.manager.it.ofmock.OfMockService;
import org.opendaylight.vtn.manager.it.util.ModelDrivenTestBase;
import org.opendaylight.vtn.manager.it.util.TestBase;
import org.opendaylight.vtn.manager.it.util.match.FlowMatchType;

import org.opendaylight.controller.sal.packet.Ethernet;
import org.opendaylight.controller.sal.packet.IEEE8021Q;
import org.opendaylight.controller.sal.packet.Packet;
import org.opendaylight.controller.sal.utils.EtherTypes;

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
    private byte[]  sourceAddress;

    /**
     * The destination MAC address.
     */
    private byte[]  destinationAddress;

    /**
     * The ethernet type.
     */
    private short  etherType;

    /**
     * The VLAN ID.
     */
    private short  vlanId;

    /**
     * The VLAN priority.
     */
    private byte  vlanPcp;

    /**
     * IP address to be probed.
     */
    private InetAddress  probeAddress;

    /**
     * VLAN ID used for IP address probe.
     */
    private short  probeVlan = -1;

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
    public EthernetFactory(byte[] src, byte[] dst) {
        setSourceAddress(src);
        setDestinationAddress(dst);
    }

    /**
     * Return the source MAC address.
     *
     * @return  A byte array that represents the source MAC address or
     *          {@code null}.
     */
    public byte[] getSourceAddress() {
        return getBytes(sourceAddress);
    }

    /**
     * Return the destination MAC address.
     *
     * @return  A byte array that represents the destination MAC address or
     *          {@code null}.
     */
    public byte[] getDestinationAddress() {
        return getBytes(destinationAddress);
    }

    /**
     * Return the ethernet type.
     *
     * @return  Ethernet type.
     */
    public short getEtherType() {
        return etherType;
    }

    /**
     * Return the VLAN ID.
     *
     * @return  VLAN ID. Zero means an untagged frame.
     */
    public short getVlanId() {
        return vlanId;
    }

    /**
     * Return the VLAN priority.
     *
     * @return  VLAN priority.
     */
    public byte getVlanPcp() {
        return vlanPcp;
    }

    /**
     * Return an IP address to be probed.
     *
     * @return  An {@link InetAddress} instance or {@code null}.
     */
    public InetAddress getProbeAddress() {
        return probeAddress;
    }

    /**
     * Return a VLAN ID used for IP address probe.
     *
     * @return  VLAN ID used for IP address probe.
     */
    public short getProbeVlan() {
        return probeVlan;
    }

    /**
     * Set the source MAC address.
     *
     * @param b  A byte array that represents the source MAC address.
     * @return  This instance.
     */
    public EthernetFactory setSourceAddress(byte[] b) {
        sourceAddress = getBytes(b);
        return this;
    }

    /**
     * Set the destination MAC address.
     *
     * @param b  A byte array that represents the destination MAC address.
     * @return  This instance.
     */
    public EthernetFactory setDestinationAddress(byte[] b) {
        destinationAddress = getBytes(b);
        return this;
    }

    /**
     * Set the ethernet type.
     *
     * @param type  Ethernet type.
     * @return  This instance.
     */
    public EthernetFactory setEtherType(short type) {
        etherType = type;
        return this;
    }

    /**
     * Set the VLAN ID.
     *
     * @param vid  VLAN ID. Zero means an untagged frame.
     * @return  This instance.
     */
    public EthernetFactory setVlanId(short vid) {
        vlanId = vid;
        return this;
    }

    /**
     * Set the VLAN priority.
     *
     * @param pcp  VLAN priority.
     * @return  This instance.
     */
    public EthernetFactory setVlanPcp(byte pcp) {
        vlanPcp = pcp;
        return this;
    }

    /**
     * Set a pair of IP address and VLAN ID for IP address probe.
     *
     * @param ip   An {@link InetAddress} instance.
     * @param vid  VLAN ID used for IP address probe.
     * @return  This instance.
     */
    public EthernetFactory setProbe(InetAddress ip, short vid) {
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
        Set<Short> vlanIds = Collections.singleton(Short.valueOf(vlanId));
        Set<Short> vids = verify(ofmock, bytes, vlanIds);
        assertEquals(Collections.<Short>emptySet(), vids);
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
    public Set<Short> verify(OfMockService ofmock, byte[] bytes,
                             Set<Short> vlanIds) throws Exception {
        assertNotNull(bytes);

        Ethernet eth = new Ethernet();
        eth.deserialize(bytes, 0, bytes.length * Byte.SIZE);

        short vlan;
        short type = eth.getEtherType();
        Packet payload = eth.getPayload();
        Packet parent;
        if (type == EtherTypes.VLANTAGGED.shortValue()) {
            assertTrue(payload instanceof IEEE8021Q);
            IEEE8021Q vtag = (IEEE8021Q)payload;
            vlan = vtag.getVid();
            assertEquals((byte)0, vtag.getCfi());
            assertEquals(vlanPcp, vtag.getPcp());
            type = vtag.getEtherType();
            payload = vtag.getPayload();
            parent = vtag;
        } else {
            vlan = 0;
            parent = eth;
        }

        if (vlan == probeVlan) {
            // The given packet should be an ARP request.
            byte[] ctlrMac = ofmock.getControllerMacAddress();
            EthernetFactory efc = new EthernetFactory(ctlrMac, sourceAddress).
                setVlanId(vlan);
            ArpFactory afc = ArpFactory.newInstance(efc);
            afc.setSenderHardwareAddress(ctlrMac).
                setTargetHardwareAddress(sourceAddress).
                setSenderProtocolAddress(TestBase.IPV4_ZERO).
                setTargetProtocolAddress(probeAddress.getAddress());
            return efc.verify(ofmock, bytes, vlanIds);
        }

        Set<Short> vids = new HashSet<>(vlanIds);
        assertTrue(vids.remove(Short.valueOf(vlan)));
        assertArrayEquals(sourceAddress, eth.getSourceMACAddress());
        assertArrayEquals(destinationAddress, eth.getDestinationMACAddress());
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
        Ethernet eth = new Ethernet().setSourceMACAddress(sourceAddress).
            setDestinationMACAddress(destinationAddress);
        if (vlanId == 0) {
            eth.setEtherType(etherType);
        } else {
            IEEE8021Q vtag = new IEEE8021Q().setCfi((byte)0).setPcp(vlanPcp).
                setVid(vlanId).setEtherType(etherType);
            eth.setEtherType(EtherTypes.VLANTAGGED.shortValue()).
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
            etype = (int)etherType;
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
            pcp = toShort(vlanPcp);
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
        EthernetFactory c = (EthernetFactory)super.clone();
        c.sourceAddress = getBytes(sourceAddress);
        c.destinationAddress = getBytes(destinationAddress);
        return c;
    }
}
