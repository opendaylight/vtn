/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.util.packet;

import static org.junit.Assert.assertArrayEquals;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;

import java.util.Set;

import org.opendaylight.vtn.manager.packet.ARP;
import org.opendaylight.vtn.manager.packet.Packet;
import org.opendaylight.vtn.manager.util.EtherTypes;

import org.opendaylight.vtn.manager.it.util.flow.match.FlowMatchType;

import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.rev131026.flow.MatchBuilder;

/**
 * {@code ArpFactory} is a utility class used to create or to verify an ARP
 * packet.
 */
public final class ArpFactory extends PacketFactory {
    /**
     * The sender hardware address.
     */
    private byte[]  senderHardwareAddress;

    /**
     * The target hardware address.
     */
    private byte[]  targetHardwareAddress;

    /**
     * The sender protocol address.
     */
    private byte[]  senderProtocolAddress;

    /**
     * The target protocol address.
     */
    private byte[]  targetProtocolAddress;

    /**
     * ARP operation code.
     */
    private short  operation = ARP.REQUEST;

    /**
     * Construct a new instance.
     *
     * @param efc  An {@link EthernetFactory} instance.
     * @return  An {@link ArpFactory} instance.
     */
    public static ArpFactory newInstance(EthernetFactory efc) {
        ArpFactory afc = new ArpFactory();
        efc.setEtherType(EtherTypes.ARP.shortValue()).setNextFactory(afc);

        return afc;
    }

    /**
     * Construct a new instance that indicates an ARP request.
     */
    ArpFactory() {}

    /**
     * Return the sender hardware address.
     *
     * @return  A byte array that represents the sender hardware address or
     *          {@code null}.
     */
    public byte[] getSenderHardwareAddress() {
        return getBytes(senderHardwareAddress);
    }

    /**
     * Return the sender protocol address.
     *
     * @return  A byte array that represents the sender protocol address or
     *          {@code null}.
     */
    public byte[] getSenderProtocolAddress() {
        return getBytes(senderProtocolAddress);
    }

    /**
     * Return the target hardware address.
     *
     * @return  A byte array that represents the target hardware address or
     *          {@code null}.
     */
    public byte[] getTargetHardwareAddress() {
        return getBytes(targetHardwareAddress);
    }

    /**
     * Return the target protocol address.
     *
     * @return  A byte array that represents the target protocol address or
     *          {@code null}.
     */
    public byte[] getTargetProtocolAddress() {
        return getBytes(targetProtocolAddress);
    }

    /**
     * Return the ARP operation code.
     *
     * @return  ARP operation code.
     */
    public short getOperation() {
        return operation;
    }

    /**
     * Set the sender hardware address.
     *
     * @param b  A byte array that represents the sender hardware address.
     * @return   This instance.
     */
    public ArpFactory setSenderHardwareAddress(byte[] b) {
        senderHardwareAddress = getBytes(b);
        return this;
    }

    /**
     * Set the sender protocol address.
     *
     * @param b  A byte array that represents the sender protocol address.
     * @return   This instance.
     */
    public ArpFactory setSenderProtocolAddress(byte[] b) {
        senderProtocolAddress = getBytes(b);
        return this;
    }

    /**
     * Set the target hardware address.
     *
     * @param b  A byte array that represents the target hardware address.
     * @return   This instance.
     */
    public ArpFactory setTargetHardwareAddress(byte[] b) {
        targetHardwareAddress = getBytes(b);
        return this;
    }

    /**
     * Set the target protocol address.
     *
     * @param b  A byte array that represents the target protocol address.
     * @return   This instance.
     */
    public ArpFactory setTargetProtocolAddress(byte[] b) {
        targetProtocolAddress = getBytes(b);
        return this;
    }

    /**
     * Set the ARP operation code.
     *
     * @param op  ARP operation code.
     * @return  This instance.
     */
    public ArpFactory setOperation(short op) {
        operation = op;
        return this;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    Packet createPacket() {
        ARP arp = new ARP().
            setHardwareType(ARP.HW_TYPE_ETHERNET).
            setProtocolType(EtherTypes.IPV4.shortValue()).
            setHardwareAddressLength((byte)senderHardwareAddress.length).
            setProtocolAddressLength((byte)targetProtocolAddress.length).
            setOpCode(operation).
            setSenderHardwareAddress(senderHardwareAddress).
            setSenderProtocolAddress(senderProtocolAddress).
            setTargetHardwareAddress(targetHardwareAddress).
            setTargetProtocolAddress(targetProtocolAddress);

        return arp;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    void verify(Packet packet) {
        assertTrue(packet instanceof ARP);
        ARP arp = (ARP)packet;

        assertEquals(ARP.HW_TYPE_ETHERNET, arp.getHardwareType());
        assertEquals(EtherTypes.IPV4.shortValue(), arp.getProtocolType());
        assertEquals((byte)senderHardwareAddress.length,
                     arp.getHardwareAddressLength());
        assertEquals((byte)senderProtocolAddress.length,
                     arp.getProtocolAddressLength());
        assertEquals(operation, arp.getOpCode());
        assertArrayEquals(senderHardwareAddress,
                          arp.getSenderHardwareAddress());
        assertArrayEquals(targetHardwareAddress,
                          arp.getTargetHardwareAddress());
        assertArrayEquals(senderProtocolAddress,
                          arp.getSenderProtocolAddress());
        assertArrayEquals(targetProtocolAddress,
                          arp.getTargetProtocolAddress());
    }

    /**
     * {@inheritDoc}
     */
    @Override
    int initMatch(MatchBuilder builder, Set<FlowMatchType> types) {
        return 0;
    }

    // Object

    /**
     * {@inheritDoc}
     */
    @Override
    public ArpFactory clone() {
        ArpFactory c = (ArpFactory)super.clone();
        c.senderHardwareAddress = getBytes(senderHardwareAddress);
        c.targetHardwareAddress = getBytes(targetHardwareAddress);
        c.senderProtocolAddress = getBytes(senderProtocolAddress);
        c.targetProtocolAddress = getBytes(targetProtocolAddress);
        return c;
    }
}
