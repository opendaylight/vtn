/*
 * Copyright (c) 2015, 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.util;

import static org.opendaylight.vtn.manager.util.NumberUtils.MASK_BYTE;
import static org.opendaylight.vtn.manager.util.NumberUtils.MASK_SHORT;

import java.math.BigInteger;
import java.util.ArrayList;
import java.util.List;
import java.util.Random;
import java.util.Set;

import com.google.common.collect.ImmutableList;

import org.junit.Assert;

import org.osgi.framework.Bundle;
import org.osgi.framework.BundleContext;

import org.opendaylight.vtn.manager.util.ByteUtils;
import org.opendaylight.vtn.manager.util.EtherAddress;
import org.opendaylight.vtn.manager.util.Ip4Network;
import org.opendaylight.vtn.manager.util.IpNetwork;
import org.opendaylight.vtn.manager.util.NumberUtils;

import org.opendaylight.vtn.manager.it.ofmock.OfMockService;
import org.opendaylight.vtn.manager.it.util.packet.ArpFactory;
import org.opendaylight.vtn.manager.it.util.packet.EthernetFactory;
import org.opendaylight.vtn.manager.it.util.vnode.mac.MacEntry;

/**
 * Abstract base class for integration tests using JUnit.
 */
public abstract class TestBase extends Assert {
    /**
     * The number of milliseconds to wait for OSGi service.
     */
    public static final long  OSGI_TIMEOUT = 600000L;

    /**
     * The number of milliseconds to sleep for short delay.
     */
    public static final long  SHORT_DELAY = 100L;

    /**
     * The number of milliseconds to wait for background tasks to complete.
     */
    public static final long  BGTASK_DELAY = 3000L;

    /**
     * The number of seconds to wait for completion of asynchronous task.
     */
    public static final long  TASK_TIMEOUT = 10L;

    /**
     * The symbolic name of the manager.implementation bundle.
     */
    public static final String  BUNDLE_VTN_MANAGER_IMPL =
        "org.opendaylight.vtn.manager.implementation";

    /**
     * Zero MAC address.
     */
    public static final EtherAddress MAC_ZERO = new EtherAddress(0L);

    /**
     * Dummy MAC address.
     */
    public static final EtherAddress  MAC_DUMMY =
        new EtherAddress(0x00deadbeef11L);

    /**
     * Zero IPv4 address.
     */
    public static final Ip4Network  IPV4_ZERO = new Ip4Network(0);

    /**
     * IPv4 address used to send ARP request.
     */
    public static final Ip4Network  IPV4_DUMMY = new Ip4Network("10.20.30.40");

    /**
     * The minimum value of vtn-index.
     */
    public static final int  VTN_INDEX_MIN = 1;

    /**
     * The maximum value of vtn-index.
     */
    public static final int  VTN_INDEX_MAX = 65535;

    /**
     * The minimum value of path policy ID.
     */
    public static final int  PATH_POLICY_ID_MIN = 1;

    /**
     * The maximum value of path policy ID.
     */
    public static final int  PATH_POLICY_ID_MAX = 3;

    /**
     * A mask value which represents valid bits in a VLAN ID.
     */
    public static final int  MASK_VLAN_ID = 0xfff;

    /**
     * A mask value which represents valid bits in a VLAN priority.
     */
    public static final int  MASK_VLAN_PRI = 0x7;

    /**
     * A mask value which represents valid bits in a DSCP value.
     */
    public static final int  MASK_IP_DSCP = 0x3f;

    /**
     * The maximum number of network elements to be added random add operation.
     */
    public static final int  RANDOM_ADD_MAX = 10;

    /**
     * A list of invalid virtual node names.
     */
    public static final List<String>  INVALID_VNODE_NAMES;

    /**
     * Initialize static field.
     */
    static {
        INVALID_VNODE_NAMES = ImmutableList.<String>builder().
            add("").
            add("01234567890123456789012345678901").
            add("abcABC_0123_XXXXXXXXXXXXXXXXXXXX").
            add("_vnode_1").
            add("VNODE 1").
            add("vNode%1").
            add("_").
            add(" ").
            add("\u3042").
            build();
    }

    /**
     * Throw an error which indicates the test code should never reach here.
     */
    public static final void unexpected() {
        fail("Should never reach here.");
    }

    /**
     * Throw an error which indicates an unexpected throwable is caught.
     *
     * @param t  A throwable.
     */
    public static final void unexpected(Throwable t) {
        throw new AssertionError("Unexpected throwable: " + t, t);
    }

    /**
     * Block the calling thread for the specified number of milliseconds.
     *
     * @param millis  The number of milliseconds to sleep.
     */
    public static final void sleep(long millis) {
        try {
            Thread.sleep(millis);
        } catch (InterruptedException e) {
            unexpected(e);
        }
    }

    /**
     * Return the OSGi bundle instance specified by the symbolic name.
     *
     * @param bc    A {@link BundleContext} instance.
     * @param name  The symbolic name of the OSGi bundle.
     * @return  {@link Bundle} instance if found.
     *          {@code null} if not found.
     */
    public static final Bundle getBundle(BundleContext bc, String name) {
        for (Bundle b: bc.getBundles()) {
            if (name.equals(b.getSymbolicName())) {
                return b;
            }
        }

        return null;
    }

    /**
     * Return the OSGi bundle instance associated with the
     * manager.implementation bundle.
     *
     * @param bc    A {@link BundleContext} instance.
     * @return  {@link Bundle} instance if found.
     *          {@code null} if not found.
     */
    public static final Bundle getManagerBundle(BundleContext bc) {
        return getBundle(bc, BUNDLE_VTN_MANAGER_IMPL);
    }

    /**
     * Create a list of boolean values and a {@code null}.
     *
     * @return A list of boolean values.
     */
    public static final List<Boolean> createBooleans() {
        return createBooleans(true);
    }

    /**
     * Create a list of boolean values.
     *
     * @param setNull  Set {@code null} to returned list if {@code true}.
     * @return A list of boolean values.
     */
    public static final List<Boolean> createBooleans(boolean setNull) {
        ArrayList<Boolean> list = new ArrayList<Boolean>();
        if (setNull) {
            list.add(null);
        }

        list.add(Boolean.TRUE);
        list.add(Boolean.FALSE);
        return list;
    }

    /**
     * Send a broadcast ARP packet.
     *
     * @param ofmock   openflowplugin mock-up service.
     * @param mac      The source MAC address.
     * @param ip       The source IPv4 address.
     * @param vid      VLAN ID for a ARP packet.
     *                 Zero or a negative value indicates untagged.
     * @param ingress  The MD-SAL node connector identifier which specifies
     *                 the ingress port.
     * @return  A raw byte image of the ARP packet.
     * @throws Exception  An error occurred.
     */
    public static final byte[] sendBroadcast(
        OfMockService ofmock, EtherAddress mac, byte[] ip, int vid,
        String ingress) throws Exception {
        EthernetFactory efc = new EthernetFactory(mac, EtherAddress.BROADCAST).
            setVlanId(vid);
        ArpFactory afc = ArpFactory.newInstance(efc);
        afc.setSenderHardwareAddress(mac.getBytes()).
            setTargetHardwareAddress(MAC_ZERO.getBytes()).
            setSenderProtocolAddress(ip).
            setTargetProtocolAddress(IPV4_DUMMY.getBytes());
        byte[] payload = efc.create();
        ofmock.sendPacketIn(ingress, payload);
        return payload;
    }

    /**
     * Send a broadcast ARP packet.
     *
     * @param ofmock  openflowplugin mock-up service.
     * @param host    The source host of the ARP packet.
     * @return  A raw byte image ot the ARP packet.
     * @throws Exception  An error occurred.
     */
    public static final byte[] sendBroadcast(OfMockService ofmock,
                                             TestHost host) throws Exception {
        return sendBroadcast(ofmock, host.getEtherAddress(),
                             host.getRawInetAddress(), host.getVlanId(),
                             host.getPortIdentifier());
    }

    /**
     * Send a broadcast ARP packet.
     *
     * @param ofmock  openflowplugin mock-up service.
     * @param ment    The source host of the ARP packet.
     * @return  A raw byte image ot the ARP packet.
     * @throws Exception  An error occurred.
     */
    public static final byte[] sendBroadcast(OfMockService ofmock,
                                             MacEntry ment) throws Exception {
        EtherAddress mac = ment.getMacAddress();
        Set<IpNetwork> ipaddrs = ment.getIpAddresses();
        assertEquals(1, ipaddrs.size());
        IpNetwork ipaddr = ipaddrs.iterator().next();
        assertTrue(ipaddr instanceof Ip4Network);

        String pid = ment.getPortIdentifier();

        return sendBroadcast(ofmock, mac, ipaddr.getBytes(), ment.getVlanId(),
                             pid);
    }

    /**
     * Convert the given long number into a hex string with ":" inserted.
     *
     * @param value  A long number.
     * @return  A hex string.
     */
    public static final String toHexString(long value) {
        String sep = "";
        StringBuilder builder = new StringBuilder();
        for (int nshift = Long.SIZE - Byte.SIZE; nshift >= 0;
             nshift -= Byte.SIZE) {
            int octet = (int)(value >>> nshift) & NumberUtils.MASK_BYTE;
            builder.append(sep).append(String.format("%02x", octet));
            sep = ByteUtils.HEX_SEPARATOR;
        }

        return builder.toString();
    }

    /**
     * Convert a hex string with ":" inserted into a long number.
     *
     * @param hex  A hex string.
     * @return  A long number.
     */
    public static final long toLong(String hex) {
        String str = hex.replaceAll(ByteUtils.HEX_SEPARATOR, "");
        BigInteger bi = new BigInteger(str, ByteUtils.HEX_RADIX);
        return bi.longValue();
    }

    /**
     * Create a vtn-index value using the given random generator.
     *
     * @param rand  A pseudo random generator.
     * @param set   A set of vtn-index values to store generated values.
     * @return  An unique vtn-index value.
     */
    public static final Integer createVtnIndex(Random rand, Set<Integer> set) {
        Integer value;
        int upper = VTN_INDEX_MAX - VTN_INDEX_MIN;
        do {
            value = rand.nextInt(upper) + VTN_INDEX_MIN;
        } while (!set.add(value));

        return value;
    }

    /**
     * Create a unique integer using the given random generator.
     *
     * @param rand  A pseudo random generator.
     * @param set   A set of integer values to store generated values.
     * @return  An unique integer.
     */
    public static final Integer createInteger(Random rand, Set<Integer> set) {
        Integer value;
        do {
            value = rand.nextInt();
        } while (!set.add(value));

        return value;
    }

    /**
     * Create an unicast MAC address using the given random generator.
     *
     * @param rand  A pseudo random generator.
     * @return  An {@link EtherAddress} instance.
     */
    public static final EtherAddress createEtherAddress(Random rand) {
        long addr = rand.nextLong() & EtherAddress.BROADCAST.getAddress();
        addr &= ~EtherAddress.MASK_MULTICAST;
        return new EtherAddress(addr);
    }

    /**
     * Create an IPv4 address using the given random generator.
     *
     * @param rand  A pseudo random generator.
     * @return  An {@link Ip4Network} instance.
     */
    public static final Ip4Network createIp4Network(Random rand) {
        int addr;
        do {
            addr = rand.nextInt();
        } while (addr == 0);

        return new Ip4Network(addr);
    }

    /**
     * Create a VLAN ID using the given random generator.
     *
     * @param rand  A pseudo random generator.
     * @return  A VLAN ID.
     */
    public static final int createVlanId(Random rand) {
        return (rand.nextInt() & MASK_VLAN_ID);
    }

    /**
     * Create a VLAN PCP value using the given random generator.
     *
     * @param rand  A pseudo random generator.
     * @return  A VLAN PCP value.
     */
    public static final short createVlanPcp(Random rand) {
        return (short)(rand.nextInt() & MASK_VLAN_PRI);
    }

    /**
     * Create an IP DSCP value using the given random generator.
     *
     * @param rand  A pseudo random generator.
     * @return  An IP DSCP value.
     */
    public static final short createDscp(Random rand) {
        return (short)(rand.nextInt() & MASK_IP_DSCP);
    }

    /**
     * Create an unsigned short value using the given random generator.
     *
     * @param rand  A pseudo random generator.
     * @return  An unsigned short value.
     */
    public static final int createUnsignedShort(Random rand) {
        return rand.nextInt() & MASK_SHORT;
    }

    /**
     * Create an unsigned byte value using the given random generator.
     *
     * @param rand  A pseudo random generator.
     * @return  An unsigned byte value.
     */
    public static final short createUnsignedByte(Random rand) {
        return (short)(rand.nextInt() & MASK_BYTE);
    }

    /**
     * Create a random positive long number using the given random generator.
     *
     * @param rand  A pseudo random generator.
     * @return  A positive long number, including zero.
     */
    public static final long createPositiveLong(Random rand) {
        return (rand.nextLong() & Long.MAX_VALUE);
    }

    /**
     * Create a random natural long number using the given random generator.
     *
     * @param rand  A pseudo random generator.
     * @return  A natural long number.
     */
    public static final long createNaturalLong(Random rand) {
        long l;
        do {
            l = (rand.nextLong() & Long.MAX_VALUE);
        } while (l == 0L);
        return l;
    }

    /**
     * Create a random reference to the path policy using the given random
     * generator.
     *
     * @param rand  A pseudo random generator.
     * @return  An path policy ID.
     */
    public static final int createPathPolicyReference(Random rand) {
        return rand.nextInt(PATH_POLICY_ID_MAX + 1);
    }

    /**
     * Create a random virtual node name using the given random generator.
     *
     * @param prefix  The prefix of the virtual node name.
     * @param rand    A pseudo random generator.
     * @return  A virtual node name.
     */
    public static final String createVnodeName(String prefix, Random rand) {
        return prefix + Integer.toHexString(rand.nextInt());
    }
}
