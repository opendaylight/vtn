/*
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal;

import java.net.InetAddress;
import java.net.Inet4Address;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Random;

import org.junit.Test;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.controller.sal.match.MatchType;
import org.opendaylight.controller.sal.packet.Ethernet;
import org.opendaylight.controller.sal.packet.ICMP;
import org.opendaylight.controller.sal.packet.IEEE8021Q;
import org.opendaylight.controller.sal.packet.IPv4;
import org.opendaylight.controller.sal.packet.Packet;
import org.opendaylight.controller.sal.packet.TCP;
import org.opendaylight.controller.sal.packet.UDP;
import org.opendaylight.controller.sal.utils.EtherTypes;
import org.opendaylight.controller.sal.utils.IPProtocols;
import org.opendaylight.controller.sal.utils.Status;
import org.opendaylight.controller.sal.utils.StatusCode;

/**
 * JUnit test for {@link MiscUtils}
 */
public class MiscUtilsTest extends TestBase {
    /**
     * Verify static constants.
     */
    @Test
    public void testConstants() {
        assertEquals(0xff, MiscUtils.MASK_BYTE);
        assertEquals(0xffff, MiscUtils.MASK_SHORT);
    }

    /**
     * Test case for {@link MiscUtils#hashCode(long)}.
     */
    @Test
    public void testHashCode() {
        Random rand = new Random();
        for (int i = 0; i < 50; i++) {
            int hi = rand.nextInt();
            int lo = rand.nextInt();
            long l = ((long)hi << Integer.SIZE) | (long)(lo & 0xffffffffL);
            assertEquals(hi ^ lo, MiscUtils.hashCode(l));
        }
    }

    /**
     * Test case for {@link MiscUtils#formatMacAddress(long mac)}.
     */
    @Test
    public void testFormatMacAddress() {
        assertEquals("00:00:00:00:00:00", MiscUtils.formatMacAddress(0L));
        assertEquals("00:00:00:00:00:01", MiscUtils.formatMacAddress(1L));
        assertEquals("00:00:00:00:00:ff", MiscUtils.formatMacAddress(0xffL));
        assertEquals("00:00:00:00:a0:ff", MiscUtils.formatMacAddress(0xa0ffL));
        assertEquals("00:00:00:12:34:56",
                     MiscUtils.formatMacAddress(0x123456L));
        assertEquals("aa:bb:cc:dd:ee:ff",
                     MiscUtils.formatMacAddress(0xaabbccddeeffL));
    }

    /**
     * Test case for {@link MiscUtils#checkName(String, String)}.
     */
    @Test
    public void testCheckName() {
        String[] descriptions = {"desc-1", "desc-2", "desc-3"};

        String[] good = {
            "a",
            "1",
            "12",
            "name",
            "NaMe_",
            "GooD_NamE1",
            "1234567890123456789012345678901",
        };

        Map<String, String> badRequests = new HashMap<String, String>();
        badRequests.put(null, "%s name cannot be null");
        badRequests.put("", "%s name cannot be empty");
        badRequests.put("12345678901234567890123456789012",
                        "%s name is too long");
        badRequests.put("_name", "%s name contains invalid character");

        List<Character> badCharacters = new ArrayList<Character>();
        for (char c = 0; c < 0x80; c++) {
            if (c == '_') {
                continue;
            }

            int type = Character.getType(c);
            if (type != Character.DECIMAL_DIGIT_NUMBER &&
                type != Character.UPPERCASE_LETTER &&
                type != Character.LOWERCASE_LETTER) {
                badCharacters.add(c);
            }
        }

        // Unicode fullwidth digit zero.
        badCharacters.add((char)0xff10);

        // Unicode hiragana letter A.
        badCharacters.add((char)0x3042);

        List<String> badNames = new ArrayList<String>();
        for (char bad: badCharacters) {
            StringBuilder builder = new StringBuilder();
            badNames.add(builder.append(bad).append("name").toString());

            builder = new StringBuilder("na");
            badNames.add(builder.append(bad).append("me").toString());

            builder = new StringBuilder("name");
            badNames.add(builder.append(bad).toString());
        }

        for (String desc: descriptions) {
            for (String name: good) {
                try {
                    MiscUtils.checkName(desc, name);
                } catch (Exception e) {
                    unexpected(e);
                }
            }

            for (Map.Entry<String, String> entry: badRequests.entrySet()) {
                String name = entry.getKey();
                String msg = String.format(entry.getValue(), desc);

                try {
                    MiscUtils.checkName(desc, name);
                    unexpected();
                } catch (VTNException e) {
                    checkException(e, msg);
                }
            }

            String msg = desc + " name contains invalid character";
            for (String name: badNames) {
                try {
                    MiscUtils.checkName(desc, name);
                    unexpected();
                } catch (VTNException e) {
                    checkException(e, msg);
                }
            }
        }
    }

    /**
     * Test case for {@link MiscUtils#checkVlan(short)}.
     */
    @Test
    public void testCheckVlan() {
        for (short vid = 0; vid <= 4095; vid++) {
            try {
                MiscUtils.checkVlan(vid);
            } catch (Exception e) {
                unexpected(e);
            }
        }

        short[] badVids = {
            Short.MIN_VALUE, -30000, -20000, -10000, -5000, -100, -23, -2, -1,
            4096, 4097, 6000, 8000, 10000, 15000, 20000, 30000, 32000,
            Short.MAX_VALUE,
        };
        for (short vid: badVids) {
            try {
                MiscUtils.checkVlan(vid);
                unexpected();
            } catch (VTNException e) {
                checkException(e, "Invalid VLAN ID: " + vid);
            }
        }
    }

    /**
     * Test case for {@link MiscUtils#isVlanPriorityValid(byte)}.
     */
    @Test
    public void testIsVlanPriorityValid() {
        for (int i = 0; i <= 0xff; i++) {
            byte pri = (byte)i;
            assertEquals(pri >= 0 && pri <= 7,
                         MiscUtils.isVlanPriorityValid(pri));
        }
    }

    /**
     * Test case for {@link MiscUtils#isDscpValid(byte)}.
     */
    @Test
    public void testIsDscpValid() {
        for (int i = 0; i <= 0xff; i++) {
            byte dscp = (byte)i;
            assertEquals(dscp >= 0 && dscp <= 63, MiscUtils.isDscpValid(dscp));
        }
    }

    /**
     * Test case for {@link MiscUtils#isIcmpValueValid(short)}.
     */
    @Test
    public void testIsIcmpValueValid() {
        for (short v = 0; v <= 0xff; v++) {
            assertTrue(MiscUtils.isIcmpValueValid(v));
        }

        short[] badValues = {
            Short.MIN_VALUE, -30000, -20000, -10000, -5000, -100, -14, -2, -1,
            256, 257, 300, 2000, 8000, 12000, 28000, 30000, Short.MAX_VALUE,
        };
        for (short v: badValues) {
            assertFalse(MiscUtils.isIcmpValueValid(v));
        }
    }

    /**
     * Test case for {@link MiscUtils#isPortNumberValid(int)}.
     */
    @Test
    public void testIsPortNumberValid() {
        int[] validPorts = {
            0, 1, 44, 555, 1000, 4000, 10000, 13000, 20000, 30000, 40000,
            50000, 65000, 65534, 65535,
        };
        for (int port: validPorts) {
            assertTrue(MiscUtils.isPortNumberValid(port));
        }

        int[] badValues = {
            Integer.MIN_VALUE, (int)0xfff00000, (int)0xf0000000, -10000,
            -100, -3, -2, -1,
            0x10000, 0x10001, 0x11000, 0x100000, 0x333333,
            0x444444, 0x500000, 0x1000000, 0xa000000, 0x10000000,
            0x50000000, 0x7f000000, Integer.MAX_VALUE,
        };
        for (int port: badValues) {
            assertFalse(MiscUtils.isPortNumberValid(port));
        }
    }

    /**
     * Test case for {@link MiscUtils#argumentIsNull(String)}.
     */
    @Test
    public void testArgumentIsNull() {
        String[] descriptions = {"desc-1", "desc-2", "desc-3"};
        for (String desc: descriptions) {
            Status st = MiscUtils.argumentIsNull(desc);
            checkStatus(st, desc + " cannot be null");
        }
    }

    /**
     * Test case for {@link MiscUtils#toInetAddress(int)} and
     * {@link MiscUtils#toInteger(InetAddress)}.
     */
    @Test
    public void testToInetAddress() {
        Random rand = new Random();
        for (int i = 0; i < 100; i++) {
            int v = rand.nextInt();
            InetAddress iaddr = MiscUtils.toInetAddress(v);
            assertTrue(iaddr instanceof Inet4Address);

            byte[] raw = iaddr.getAddress();
            assertEquals(4, raw.length);
            for (int j = 0; j < raw.length; j++) {
                byte b = (byte)(v >>> ((3 - j) * Byte.SIZE));
                assertEquals(b, raw[j]);
            }

            assertEquals(v, MiscUtils.toInteger(iaddr));
        }

        List<InetAddress> invalid = new ArrayList<InetAddress>();
        invalid.add(null);
        try {
            invalid.add(InetAddress.getByName("::1"));
        } catch (Exception e) {
            unexpected(e);
        }

        for (InetAddress iaddr: invalid) {
            try {
                MiscUtils.toInteger(iaddr);
                unexpected();
            } catch (IllegalStateException e) {
            }
        }
    }

    /**
     * Test case for {@link MiscUtils#copy(Packet, Packet)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testCopyPacket() throws Exception {
        // Specifying null.
        try {
            MiscUtils.copy((Packet)null, (Packet)null);
            unexpected();
        } catch (VTNException e) {
            checkException(e, "Failed to copy the packet.",
                           StatusCode.INTERNALERROR);
            assertTrue(e.getCause() instanceof NullPointerException);
        }

        byte[] raw = {
            (byte)0x7c, (byte)0xe8, (byte)0x07, (byte)0xde,
            (byte)0xc5, (byte)0x1b, (byte)0x40, (byte)0x86,
            (byte)0xc0, (byte)0xa1, (byte)0xe3, (byte)0xe0,
            (byte)0x90, (byte)0xdd, (byte)0x0c, (byte)0xe6,
        };

        // Ensure that Ethernet instance with raw payload can be copied.
        byte[] srcMac = {
            (byte)0x00, (byte)0x11, (byte)0x22,
            (byte)0x33, (byte)0x44, (byte)0x55,
        };
        byte[] dstMac = {
            (byte)0xf0, (byte)0xfa, (byte)0xfb,
            (byte)0xfc, (byte)0xfd, (byte)0xfe,
        };
        short etype = 0xa00;
        short vid = MatchType.DL_VLAN_NONE;
        byte pcp = 1;
        for (int loop = 0; loop < 5; loop++) {
            Ethernet org = createEthernet(srcMac, dstMac, etype, vid, pcp, raw);
            Ethernet copy = new Ethernet();
            assertSame(copy, MiscUtils.copy(org, copy));
            checkCopy(org, copy, srcMac, dstMac, etype, vid, pcp, raw);
            etype++;
            vid++;
            srcMac[3]++;
            dstMac[4]++;
        }

        // Ensure that IPv4 instance with raw payload can be copied.
        int srcIp = (int)0x0a010203;
        int dstIp = (int)0xc0a864c8;
        short ipproto = 30;
        byte dscp = 0;
        for (int loop = 0; loop < 5; loop++) {
            IPv4 org = createIPv4(srcIp, dstIp, ipproto, dscp);
            org.setRawPayload(raw);

            // Copy the original in order to fix up header fields, such as
            // checksum.
            org = MiscUtils.copy(org, new IPv4());

            IPv4 copy = new IPv4();
            assertSame(copy, MiscUtils.copy(org, copy));
            checkCopy(org, copy, srcIp, dstIp, ipproto, dscp, raw);
            srcIp++;
            dstIp++;
            ipproto++;
            dscp++;
        }

        // Ensure that TCP instance with raw payload can be copied.
        int seq = 0x3333;
        int ack = 0x4567;
        byte dataOff = 0;
        byte resv = 0;
        short win = (short)0xfc00;
        short urp = (short)0x5678;
        short cksum = (short)0x1234;
        short flags = 0x18;
        short srcPort = 17;
        short dstPort = 30000;
        for (int loop = 0; loop < 5; loop++) {
            TCP org = new TCP();
            org.setSourcePort(srcPort).setDestinationPort(dstPort).
                setSequenceNumber(seq).setAckNumber(ack).
                setDataOffset(dataOff).setHeaderLenFlags(flags).
                setReserved(resv).setWindowSize(win).setChecksum(cksum).
                setUrgentPointer(urp).setRawPayload(raw);
            TCP copy = new TCP();
            assertSame(copy, MiscUtils.copy(org, copy));
            checkCopy(org, copy, srcPort, dstPort, cksum, raw);

            seq++;
            ack++;
            dataOff = (byte)((dataOff + 1) & 0xf);
            cksum += 7;
            srcPort++;
            dstPort++;
        }

        // Ensure that UDP instance with raw payload can be copied.
        srcPort = (short)45678;
        dstPort = 133;
        cksum = (short)0xf1a0;
        short udpLen = (short)(raw.length + 8);
        for (int loop = 0; loop < 5; loop++) {
            UDP org = new UDP();
            org.setSourcePort(srcPort).setDestinationPort(dstPort).
                setLength(udpLen).setChecksum(cksum).setRawPayload(raw);
            UDP copy = new UDP();
            assertSame(copy, MiscUtils.copy(org, copy));
            checkCopy(org, copy, srcPort, dstPort, cksum, raw);

            srcPort++;
            dstPort++;
            cksum++;
        }

        // Ensure that ICMP instance with raw payload can be copied.
        short icmpSeq = 0x13a9;
        short icmpId = 0x7c;
        byte icmpType = 0;
        byte icmpCode = 6;
        for (int loop = 0; loop < 5; loop++) {
            ICMP org = new ICMP();
            org.setType(icmpType).setCode(icmpCode).setIdentifier(icmpId).
                setSequenceNumber(icmpSeq).setRawPayload(raw);

            // Copy the original in order to fix up header fields, such as
            // checksum.
            org = MiscUtils.copy(org, new ICMP());

            ICMP copy = new ICMP();
            assertSame(copy, MiscUtils.copy(org, copy));
            checkCopy(org, copy, icmpType, icmpCode, icmpId, icmpSeq, raw);

            icmpSeq++;
            icmpId++;
            icmpType++;
            icmpCode++;
        }

        // Ensure that nested Etherner frame can be copied.
        IPProtocols[] protos = {
            IPProtocols.TCP,
            IPProtocols.UDP,
            IPProtocols.ICMP,
        };
        int protoIndex = 0;
        vid = MatchType.DL_VLAN_NONE;
        etype = EtherTypes.IPv4.shortValue();
        pcp = 0;
        dscp = 0;
        srcPort = 128;
        dstPort = 12345;
        cksum = (short)0xed03;
        for (int loop = 0; loop < protos.length * 5; loop++) {
            IPProtocols proto = protos[protoIndex];
            protoIndex++;
            if (protoIndex >= protos.length) {
                protoIndex = 0;
            }

            Packet l4 = null;
            switch (proto) {
            case TCP:
                l4 = new TCP().setSourcePort(srcPort).
                    setDestinationPort(dstPort).setSequenceNumber(seq).
                    setAckNumber(ack).setDataOffset(dataOff).
                    setHeaderLenFlags(flags).setReserved(resv).
                    setWindowSize(win).setChecksum(cksum).
                    setUrgentPointer(urp);
                break;

            case UDP:
                l4 = new UDP().setSourcePort(srcPort).
                    setDestinationPort(dstPort).setLength(udpLen).
                    setChecksum(cksum);
                break;

            case ICMP:
                l4 = new ICMP().setType(icmpType).setCode(icmpCode).
                    setIdentifier(icmpId).setSequenceNumber(icmpSeq);
                break;

            default:
                unexpected();
                break;
            }

            l4.setRawPayload(raw);

            ipproto = proto.byteValue();
            IPv4 ipv4 = createIPv4(srcIp, dstIp, ipproto, dscp);
            ipv4.setPayload(l4);
            Ethernet ether = createEthernet(srcMac, dstMac, etype, vid, pcp,
                                            ipv4);

            // Copy the original in order to fix up header fields, such as
            // IP checksum.
            ether = MiscUtils.copy(ether, new Ethernet());

            Ethernet copy = new Ethernet();
            assertSame(copy, MiscUtils.copy(ether, copy));
            checkCopy(ether, copy, srcMac, dstMac, etype, vid, pcp, null);

            IPv4 ipCopy;
            if (vid == MatchType.DL_VLAN_NONE) {
                ipv4 = (IPv4)ether.getPayload();
                ipCopy = (IPv4)copy.getPayload();
            } else {
                Packet pkt = ether.getPayload();
                ipv4 = (IPv4)pkt.getPayload();
                pkt = copy.getPayload();
                ipCopy = (IPv4)pkt.getPayload();
            }
            vid = (short)((vid + 1) & 0xfff);
            srcMac[5]++;
            dstMac[2]++;

            // Check IPv4 packet.
            checkCopy(ipv4, ipCopy, srcIp, dstIp, ipproto, dscp, null);
            srcIp++;
            dstIp++;
            dscp = (byte)((dscp + 1) & 0x3f);

            switch (proto) {
            case TCP:
                // Check TCP packet.
                TCP tcp = (TCP)ipv4.getPayload();
                TCP tcpCopy = (TCP)ipCopy.getPayload();
                checkCopy(tcp, tcpCopy, srcPort, dstPort, cksum, raw);
                seq++;
                ack++;
                dataOff = (byte)((dataOff + 1) & 0xf);
                cksum += 13;
                srcPort++;
                dstPort++;
                break;

            case UDP:
                // Check UDP packet.
                UDP udp = (UDP)ipv4.getPayload();
                UDP udpCopy = (UDP)ipCopy.getPayload();
                checkCopy(udp, udpCopy, srcPort, dstPort, cksum, raw);
                srcPort++;
                dstPort++;
                cksum += 23;
                break;

            case ICMP:
                // Check ICMP packet.
                ICMP icmp = (ICMP)ipv4.getPayload();
                ICMP icmpCopy = (ICMP)ipCopy.getPayload();
                checkCopy(icmp, icmpCopy, icmpType, icmpCode, icmpId, icmpSeq,
                          raw);
                icmpSeq++;
                icmpId++;
                icmpType++;
                icmpCode++;
                break;

            default:
                unexpected();
                break;
            }
        }
    }

    /**
     * Test case for {@link MiscUtils#setInt(byte[], int, int)}.
     */
    @Test
    public void testSetInt() {
        int size = 32;
        int value = 0xa1b2c3d4;
        byte[] bytes = {(byte)0xa1, (byte)0xb2, (byte)0xc3, (byte)0xd4};
        for (int off = 0; off <= size - 4; off++) {
            byte[] buffer = new byte[size];
            MiscUtils.setInt(buffer, off, value);
            for (int i = 0; i < buffer.length; i++) {
                if (i >= off && i < off + 4) {
                    assertEquals(bytes[i - off], buffer[i]);
                } else {
                    assertEquals((byte)0, buffer[i]);
                }
            }
        }
    }

    /**
     * Test case for {@link MiscUtils#setShort(byte[], int, short)}.
     */
    @Test
    public void testSetShort() {
        int size = 32;
        short value = (short)0xa1b2;
        byte[] bytes = {(byte)0xa1, (byte)0xb2};
        for (int off = 0; off <= size - 2; off++) {
            byte[] buffer = new byte[size];
            MiscUtils.setShort(buffer, off, value);
            for (int i = 0; i < buffer.length; i++) {
                if (i >= off && i < off + 2) {
                    assertEquals(bytes[i - off], buffer[i]);
                } else {
                    assertEquals((byte)0, buffer[i]);
                }
            }
        }
    }

    /**
     * Verify the contents of the given {@link VTNException}.
     *
     * <p>
     *   This method expects that {@link StatusCode#BADREQUEST} is configured
     *   in the given exception.
     * </p>
     *
     * @param e     A {@link VTNException} to be tested.
     * @param desc  An expected description.
     */
    private void checkException(VTNException e, String desc) {
        checkException(e, desc, StatusCode.BADREQUEST);
    }

    /**
     * Verify the contents of the given {@link VTNException}.
     *
     * @param e     A {@link VTNException} to be tested.
     * @param desc  An expected description.
     * @param code  An expected status code.
     */
    private void checkException(VTNException e, String desc, StatusCode code) {
        checkStatus(e.getStatus(), desc, code);
    }

    /**
     * Verify the contents of the given {@link Status}.
     *
     * <p>
     *   This method expects that {@link StatusCode#BADREQUEST} is configured
     *   in the given status.
     * </p>
     *
     * @param st    A {@link Status} instance to be tested.
     * @param desc  An expected description.
     */
    private void checkStatus(Status st, String desc) {
        checkStatus(st, desc, StatusCode.BADREQUEST);
    }

    /**
     * Verify the contents of the given {@link Status}.
     *
     * @param st    A {@link Status} instance to be tested.
     * @param desc  An expected description.
     * @param code  An expected status code.
     */
    private void checkStatus(Status st, String desc, StatusCode code) {
        assertEquals(desc, st.getDescription());
        assertEquals(code, st.getCode());
    }

    /**
     * Ensure that the raw payload of the packet was copied successfully.
     *
     * @param org   Original{@link Packet} instance.
     * @param copy  A copy of {@code org}.
     * @param raw    Expected raw payload.
     */
    private void checkRawPayloadCopy(Packet org, Packet copy, byte[] raw) {
        byte[] rawPayload = copy.getRawPayload();
        byte[] orgPayload = org.getRawPayload();
        if (raw == null) {
            assertEquals(null, rawPayload);
            assertEquals(null, orgPayload);
        } else {
            assertEquals(null, org.getPayload());
            assertEquals(null, copy.getPayload());
            assertNotSame(raw, rawPayload);
            assertNotSame(orgPayload, rawPayload);
            assertArrayEquals(raw, rawPayload);
            assertArrayEquals(raw, orgPayload);
        }
    }

    /**
     * Ensure that an {@link Ethernet} instance was copied successfully.
     *
     * @param org    Original {@link Ethernet} instance.
     * @param copy   A copy of {@code org}.
     * @param src    Expected source MAC address.
     * @param dst    Expected destination MAC address.
     * @param etype  Expected ethernet type.
     * @param vid    Expected VLAN ID.
     * @param pcp    Expected VLAN priority.
     * @param raw    Expected raw payload.
     */
    private void checkCopy(Ethernet org, Ethernet copy, byte[] src, byte[] dst,
                           short etype, short vid, byte pcp, byte[] raw) {
        assertNotSame(copy, org);
        assertEquals(copy, org);
        assertArrayEquals(src, org.getSourceMACAddress());
        assertArrayEquals(dst, org.getDestinationMACAddress());
        assertArrayEquals(src, copy.getSourceMACAddress());
        assertArrayEquals(dst, copy.getDestinationMACAddress());
        Packet parent;
        Packet orgParent;
        if (vid == MatchType.DL_VLAN_NONE) {
            assertEquals(etype, org.getEtherType());
            assertEquals(etype, copy.getEtherType());
            parent = copy;
            orgParent = org;
        } else {
            assertEquals(EtherTypes.VLANTAGGED.shortValue(),
                         org.getEtherType());
            assertEquals(EtherTypes.VLANTAGGED.shortValue(),
                         copy.getEtherType());
            IEEE8021Q orgTag = (IEEE8021Q)org.getPayload();
            IEEE8021Q vlanTag = (IEEE8021Q)copy.getPayload();
            assertNotNull(vlanTag);
            assertNotSame(orgTag, vlanTag);
            assertEquals(orgTag, vlanTag);
            assertEquals(etype, orgTag.getEtherType());
            assertEquals(vid, orgTag.getVid());
            assertEquals(pcp, orgTag.getPcp());
            assertEquals((byte)0, orgTag.getCfi());
            assertEquals(etype, vlanTag.getEtherType());
            assertEquals(vid, vlanTag.getVid());
            assertEquals(pcp, vlanTag.getPcp());
            assertEquals((byte)0, vlanTag.getCfi());
            parent = vlanTag;
            orgParent = orgTag;
        }

        checkRawPayloadCopy(parent, orgParent, raw);
    }

    /**
     * Ensure that an {@link IPv4} instance was copied successfully.
     *
     * @param org    Original {@link IPv4} instance.
     * @param copy   A copy of {@code org}.
     * @param src    Expected source IP address.
     * @param dst    Expected destination IP address.
     * @param proto  Expected IP protocol number.
     * @param dscp   Expected DSCP field value.
     * @param raw    Expected raw payload.
     */
    private void checkCopy(IPv4 org, IPv4 copy, int src, int dst, short proto,
                           byte dscp, byte[] raw) {
        assertNotSame(copy, org);
        assertEquals(copy, org);
        for (IPv4 ipv4: new IPv4[]{org, copy}) {
            assertEquals(src, ipv4.getSourceAddress());
            assertEquals(dst, ipv4.getDestinationAddress());
            assertEquals((byte)proto, ipv4.getProtocol());
            assertEquals(dscp, ipv4.getDiffServ());
        }

        checkRawPayloadCopy(org, copy, raw);
    }

    /**
     * Ensure that an {@link TCP} instance was copied successfully.
     *
     * @param org    Original {@link TCP} instance.
     * @param copy   A copy of {@code org}.
     * @param src    Expected source port number.
     * @param dst    Expected destination port number.
     * @param cksum  Expected TCP checksum.
     * @param raw    Expected raw payload.
     */
    private void checkCopy(TCP org, TCP copy, short src, short dst,
                           short cksum, byte[] raw) {
        assertNotSame(copy, org);
        assertEquals(copy, org);
        for (TCP tcp: new TCP[]{org, copy}) {
            assertEquals(src, tcp.getSourcePort());
            assertEquals(dst, tcp.getDestinationPort());
            assertEquals(cksum, tcp.getChecksum());
        }

        checkRawPayloadCopy(org, copy, raw);
    }

    /**
     * Ensure that an {@link UDP} instance was copied successfully.
     *
     * @param org    Original {@link UDP} instance.
     * @param copy   A copy of {@code org}.
     * @param src    Expected source port number.
     * @param dst    Expected destination port number.
     * @param cksum  Expected UDP checksum.
     * @param raw    Expected raw payload.
     */
    private void checkCopy(UDP org, UDP copy, short src, short dst,
                           short cksum, byte[] raw) {
        assertNotSame(copy, org);
        assertEquals(copy, org);
        for (UDP udp: new UDP[]{org, copy}) {
            assertEquals(src, udp.getSourcePort());
            assertEquals(dst, udp.getDestinationPort());
            assertEquals(cksum, udp.getChecksum());
        }

        checkRawPayloadCopy(org, copy, raw);
    }

    /**
     * Ensure that an {@link ICMP} instance was copied successfully.
     *
     * @param org    Original {@link UDP} instance.
     * @param copy   A copy of {@code org}.
     * @param type   Expected ICMP type.
     * @param code   Expected ICMP code.
     * @param id     Expected identifier.
     * @param seq    Expected sequence number.
     * @param raw    Expected raw payload.
     */
    private void checkCopy(ICMP org, ICMP copy, byte type, byte code,
                           short id, short seq, byte[] raw) {
        assertNotSame(copy, org);
        assertEquals(copy, org);
        for (ICMP icmp: new ICMP[]{org, copy}) {
            assertEquals(type, icmp.getType());
            assertEquals(code, icmp.getCode());
            assertEquals(id, icmp.getIdentifier());
            assertEquals(seq, icmp.getSequenceNumber());
        }

        checkRawPayloadCopy(org, copy, raw);
    }
}
