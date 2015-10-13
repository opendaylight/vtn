/*
 * Copyright (c) 2014, 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.flow.cond;

import java.net.InetAddress;
import java.util.ArrayList;
import java.util.List;

import org.junit.Test;

import org.opendaylight.vtn.manager.util.EtherAddress;
import org.opendaylight.vtn.manager.util.InetProtocols;

import org.opendaylight.vtn.manager.TestBase;

import org.opendaylight.controller.sal.packet.address.EthernetAddress;

/**
 * JUnit test for {@link FlowMatchBuilder}.
 */
public class FlowMatchBuilderTest extends TestBase {
    /**
     * Test case for
     * {@link FlowMatchBuilder#setSourceMacAddress(EthernetAddress)} and
     * {@link FlowMatchBuilder#setSourceMacAddress(EtherAddress)} and
     */
    @Test
    public void testSetSourceMacAddress() {
        FlowMatchBuilder fmbuilder = new FlowMatchBuilder();
        int index = 0;
        for (EthernetAddress eaddr: createEthernetAddresses()) {
            assertSame(fmbuilder, fmbuilder.setSourceMacAddress(eaddr));
            FlowMatch fm = fmbuilder.build(index);
            EthernetMatch em = (eaddr == null)
                ? null
                : new EthernetMatch(eaddr, null, null, null, null);
            assertEquals(Integer.valueOf(index), fm.getIndex());
            assertEquals(em, fm.getEthernetMatch());
            assertEquals(null, fm.getInetMatch());
            assertEquals(null, fm.getLayer4Match());
            assertEquals(-1, fm.getInetProtocol());

            EtherAddress ea = EtherAddress.create(eaddr);
            assertSame(fmbuilder, fmbuilder.setSourceMacAddress(ea));
            fm = fmbuilder.build(index);
            em = (ea == null) ? null
                : new EthernetMatch(eaddr, null, null, null, null);
            assertEquals(Integer.valueOf(index), fm.getIndex());
            assertEquals(em, fm.getEthernetMatch());
            assertEquals(null, fm.getInetMatch());
            assertEquals(null, fm.getLayer4Match());
            assertEquals(-1, fm.getInetProtocol());

            fm = fmbuilder.build();
            assertEquals(null, fm.getIndex());
            assertEquals(em, fm.getEthernetMatch());
            assertEquals(null, fm.getInetMatch());
            assertEquals(null, fm.getLayer4Match());
            assertEquals(-1, fm.getInetProtocol());

            index++;
        }
    }

    /**
     * Test case for
     * {@link FlowMatchBuilder#setDestinationMacAddress(EthernetAddress)} and
     * {@link FlowMatchBuilder#setDestinationMacAddress(EtherAddress)}.
     */
    @Test
    public void testSetDestinationMacAddress() {
        FlowMatchBuilder fmbuilder = new FlowMatchBuilder();
        int index = 0;
        for (EthernetAddress eaddr: createEthernetAddresses()) {
            assertSame(fmbuilder, fmbuilder.setDestinationMacAddress(eaddr));
            FlowMatch fm = fmbuilder.build(index);
            EthernetMatch em = (eaddr == null)
                ? null
                : new EthernetMatch(null, eaddr, null, null, null);
            assertEquals(Integer.valueOf(index), fm.getIndex());
            assertEquals(em, fm.getEthernetMatch());
            assertEquals(null, fm.getInetMatch());
            assertEquals(null, fm.getLayer4Match());
            assertEquals(-1, fm.getInetProtocol());

            EtherAddress ea = EtherAddress.create(eaddr);
            assertSame(fmbuilder, fmbuilder.setDestinationMacAddress(ea));
            fm = fmbuilder.build(index);
            em = (ea == null) ? null
                : new EthernetMatch(null, eaddr, null, null, null);
            assertEquals(Integer.valueOf(index), fm.getIndex());
            assertEquals(em, fm.getEthernetMatch());
            assertEquals(null, fm.getInetMatch());
            assertEquals(null, fm.getLayer4Match());
            assertEquals(-1, fm.getInetProtocol());

            fm = fmbuilder.build();
            assertEquals(null, fm.getIndex());
            assertEquals(em, fm.getEthernetMatch());
            assertEquals(null, fm.getInetMatch());
            assertEquals(null, fm.getLayer4Match());
            assertEquals(-1, fm.getInetProtocol());

            index++;
        }
    }

    /**
     * Test case for {@link FlowMatchBuilder#setEtherType(Integer)}.
     */
    @Test
    public void testSetEtherType() {
        FlowMatchBuilder fmbuilder = new FlowMatchBuilder();
        int index = 0;
        Integer[] types = {
            null, 0, 1, 0x800, 0x86dd, 0xffff,
        };
        EtherAddress nullMac = null;

        for (Integer type: types) {
            assertSame(fmbuilder, fmbuilder.setEtherType(type));
            FlowMatch fm = fmbuilder.build(index);
            EthernetMatch em = (type == null)
                ? null
                : new EthernetMatch(nullMac, nullMac, type, null, null);
            assertEquals(Integer.valueOf(index), fm.getIndex());
            assertEquals(em, fm.getEthernetMatch());
            assertEquals(null, fm.getInetMatch());
            assertEquals(null, fm.getLayer4Match());
            assertEquals(-1, fm.getInetProtocol());

            fm = fmbuilder.build();
            assertEquals(null, fm.getIndex());
            assertEquals(em, fm.getEthernetMatch());
            assertEquals(null, fm.getInetMatch());
            assertEquals(null, fm.getLayer4Match());
            assertEquals(-1, fm.getInetProtocol());

            index++;
        }
    }

    /**
     * Test case for {@link FlowMatchBuilder#setVlanId(Short)}.
     */
    @Test
    public void testSetVlanId() {
        FlowMatchBuilder fmbuilder = new FlowMatchBuilder();
        int index = 0;
        Short[] vlans = {
            null, 0, 1, 100, 3000, 4095,
        };
        EtherAddress nullMac = null;

        for (Short vlan: vlans) {
            assertSame(fmbuilder, fmbuilder.setVlanId(vlan));
            FlowMatch fm = fmbuilder.build(index);
            EthernetMatch em = (vlan == null)
                ? null
                : new EthernetMatch(nullMac, nullMac, null, vlan, null);
            assertEquals(Integer.valueOf(index), fm.getIndex());
            assertEquals(em, fm.getEthernetMatch());
            assertEquals(null, fm.getInetMatch());
            assertEquals(null, fm.getLayer4Match());
            assertEquals(-1, fm.getInetProtocol());

            fm = fmbuilder.build();
            assertEquals(null, fm.getIndex());
            assertEquals(em, fm.getEthernetMatch());
            assertEquals(null, fm.getInetMatch());
            assertEquals(null, fm.getLayer4Match());
            assertEquals(-1, fm.getInetProtocol());

            index++;
        }
    }

    /**
     * Test case for {@link FlowMatchBuilder#setVlanPriority(Byte)}.
     */
    @Test
    public void testSetVlanPriority() {
        FlowMatchBuilder fmbuilder = new FlowMatchBuilder();
        int index = 0;
        EtherAddress nullMac = null;
        List<Byte> priorities = new ArrayList<>();
        priorities.add(null);
        for (byte pri = 0; pri <= 7; pri++) {
            priorities.add(Byte.valueOf(pri));
        }

        for (Byte pri: priorities) {
            assertSame(fmbuilder, fmbuilder.setVlanPriority(pri));
            FlowMatch fm = fmbuilder.build(index);
            EthernetMatch em = (pri == null)
                ? null
                : new EthernetMatch(nullMac, nullMac, null, null, pri);
            assertEquals(Integer.valueOf(index), fm.getIndex());
            assertEquals(em, fm.getEthernetMatch());
            assertEquals(null, fm.getInetMatch());
            assertEquals(null, fm.getLayer4Match());
            assertEquals(-1, fm.getInetProtocol());

            fm = fmbuilder.build();
            assertEquals(null, fm.getIndex());
            assertEquals(em, fm.getEthernetMatch());
            assertEquals(null, fm.getInetMatch());
            assertEquals(null, fm.getLayer4Match());
            assertEquals(-1, fm.getInetProtocol());
            index++;
        }
    }

    /**
     * Test case for
     * {@link FlowMatchBuilder#setSourceInetAddress(InetAddress)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testSetSourceInetAddress() throws Exception {
        FlowMatchBuilder fmbuilder = new FlowMatchBuilder();
        int index = 0;
        for (InetAddress iaddr: createInet4Addresses()) {
            assertSame(fmbuilder, fmbuilder.setSourceInetAddress(iaddr));
            FlowMatch fm = fmbuilder.build(index);
            InetMatch im = (iaddr == null)
                ? null
                : new Inet4Match(iaddr, null, null, null, null, null);
            assertEquals(Integer.valueOf(index), fm.getIndex());
            assertEquals(null, fm.getEthernetMatch());
            assertEquals(im, fm.getInetMatch());
            assertEquals(null, fm.getLayer4Match());
            assertEquals(-1, fm.getInetProtocol());

            fm = fmbuilder.build();
            assertEquals(null, fm.getIndex());
            assertEquals(null, fm.getEthernetMatch());
            assertEquals(im, fm.getInetMatch());
            assertEquals(null, fm.getLayer4Match());
            assertEquals(-1, fm.getInetProtocol());
            index++;
        }

        // Specifying invalid IP address.
        InetAddress v6 = InetAddress.getByName("::1");
        try {
            fmbuilder.setSourceInetAddress(v6);
            unexpected();
        } catch (IllegalArgumentException e) {
            String msg = "Unsupported IP address type: " + v6;
            assertEquals(msg, e.getMessage());
        }
    }

    /**
     * Test case for
     * {@link FlowMatchBuilder#setDestinationInetAddress(InetAddress)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testSetDestinationInetAddress() throws Exception {
        FlowMatchBuilder fmbuilder = new FlowMatchBuilder();
        int index = 0;
        for (InetAddress iaddr: createInet4Addresses()) {
            assertSame(fmbuilder, fmbuilder.setDestinationInetAddress(iaddr));
            FlowMatch fm = fmbuilder.build(index);
            InetMatch im = (iaddr == null)
                ? null
                : new Inet4Match(null, null, iaddr, null, null, null);
            assertEquals(Integer.valueOf(index), fm.getIndex());
            assertEquals(null, fm.getEthernetMatch());
            assertEquals(im, fm.getInetMatch());
            assertEquals(null, fm.getLayer4Match());
            assertEquals(-1, fm.getInetProtocol());

            fm = fmbuilder.build();
            assertEquals(null, fm.getIndex());
            assertEquals(null, fm.getEthernetMatch());
            assertEquals(im, fm.getInetMatch());
            assertEquals(null, fm.getLayer4Match());
            assertEquals(-1, fm.getInetProtocol());
            index++;
        }

        // Specifying invalid IP address.
        InetAddress v6 = InetAddress.getByName("::1");
        try {
            fmbuilder.setSourceInetAddress(v6);
            unexpected();
        } catch (IllegalArgumentException e) {
            String msg = "Unsupported IP address type: " + v6;
            assertEquals(msg, e.getMessage());
        }
    }

    /**
     * Test case for {@link FlowMatchBuilder#setSourceInetSuffix(Short)}.
     */
    @Test
    public void testSetSourceInetSuffix() {
        FlowMatchBuilder fmbuilder = new FlowMatchBuilder();
        int index = 0;
        List<Short> suffixes = new ArrayList<>();
        suffixes.add(null);
        for (short suffix = 1; suffix <= 31; suffix++) {
            suffixes.add(Short.valueOf(suffix));
        }
        for (Short suffix: suffixes) {
            assertSame(fmbuilder, fmbuilder.setSourceInetSuffix(suffix));
            FlowMatch fm = fmbuilder.build(index);
            InetMatch im = (suffix == null)
                ? null
                : new Inet4Match(null, suffix, null, null, null, null);
            assertEquals(Integer.valueOf(index), fm.getIndex());
            assertEquals(null, fm.getEthernetMatch());
            assertEquals(im, fm.getInetMatch());
            assertEquals(null, fm.getLayer4Match());
            assertEquals(-1, fm.getInetProtocol());

            fm = fmbuilder.build();
            assertEquals(null, fm.getIndex());
            assertEquals(null, fm.getEthernetMatch());
            assertEquals(im, fm.getInetMatch());
            assertEquals(null, fm.getLayer4Match());
            assertEquals(-1, fm.getInetProtocol());
            index++;
        }
    }

    /**
     * Test case for {@link FlowMatchBuilder#setDestinationInetSuffix(Short)}.
     */
    @Test
    public void testSetDestinationInetSuffix() {
        FlowMatchBuilder fmbuilder = new FlowMatchBuilder();
        int index = 0;
        List<Short> suffixes = new ArrayList<>();
        suffixes.add(null);
        for (short suffix = 1; suffix <= 31; suffix++) {
            suffixes.add(Short.valueOf(suffix));
        }
        for (Short suffix: suffixes) {
            assertSame(fmbuilder, fmbuilder.setDestinationInetSuffix(suffix));
            FlowMatch fm = fmbuilder.build(index);
            InetMatch im = (suffix == null)
                ? null
                : new Inet4Match(null, null, null, suffix, null, null);
            assertEquals(Integer.valueOf(index), fm.getIndex());
            assertEquals(null, fm.getEthernetMatch());
            assertEquals(im, fm.getInetMatch());
            assertEquals(null, fm.getLayer4Match());
            assertEquals(-1, fm.getInetProtocol());

            fm = fmbuilder.build();
            assertEquals(null, fm.getIndex());
            assertEquals(null, fm.getEthernetMatch());
            assertEquals(im, fm.getInetMatch());
            assertEquals(null, fm.getLayer4Match());
            assertEquals(-1, fm.getInetProtocol());
            index++;
        }
    }

    /**
     * Test case for {@link FlowMatchBuilder#setInetProtocol(Short)}.
     */
    @Test
    public void testSetInetProtocol() {
        FlowMatchBuilder fmbuilder = new FlowMatchBuilder();
        int index = 0;
        Short[] protocols = {
            null, Short.valueOf((short)0), Short.valueOf((short)6),
            Short.valueOf((short)17), Short.valueOf((short)50),
            Short.valueOf((short)120), Short.valueOf((short)250),
        };
        for (Short proto: protocols) {
            assertSame(fmbuilder, fmbuilder.setInetProtocol(proto));
            FlowMatch fm = fmbuilder.build(index);
            InetMatch im = (proto == null)
                ? null
                : new Inet4Match(null, null, null, null, proto, null);
            assertEquals(Integer.valueOf(index), fm.getIndex());
            assertEquals(null, fm.getEthernetMatch());
            assertEquals(im, fm.getInetMatch());
            assertEquals(null, fm.getLayer4Match());
            int ipproto = (proto == null) ? -1 : proto.intValue();
            assertEquals(ipproto, fm.getInetProtocol());

            fm = fmbuilder.build();
            assertEquals(null, fm.getIndex());
            assertEquals(null, fm.getEthernetMatch());
            assertEquals(im, fm.getInetMatch());
            assertEquals(null, fm.getLayer4Match());
            assertEquals(ipproto, fm.getInetProtocol());
            index++;
        }
    }

    /**
     * Test case for {@link FlowMatchBuilder#setInetDscp(Byte)}.
     */
    @Test
    public void testSetInetDscp() {
        FlowMatchBuilder fmbuilder = new FlowMatchBuilder();
        int index = 0;
        Byte[] dscps = {
            null, Byte.valueOf((byte)0), Byte.valueOf((byte)1),
            Byte.valueOf((byte)10), Byte.valueOf((byte)33),
            Byte.valueOf((byte)50), Byte.valueOf((byte)63),
        };
        for (Byte dscp: dscps) {
            assertSame(fmbuilder, fmbuilder.setInetDscp(dscp));
            FlowMatch fm = fmbuilder.build(index);
            InetMatch im = (dscp == null)
                ? null
                : new Inet4Match(null, null, null, null, null, dscp);
            assertEquals(Integer.valueOf(index), fm.getIndex());
            assertEquals(null, fm.getEthernetMatch());
            assertEquals(im, fm.getInetMatch());
            assertEquals(null, fm.getLayer4Match());
            assertEquals(-1, fm.getInetProtocol());

            fm = fmbuilder.build();
            assertEquals(null, fm.getIndex());
            assertEquals(null, fm.getEthernetMatch());
            assertEquals(im, fm.getInetMatch());
            assertEquals(null, fm.getLayer4Match());
            assertEquals(-1, fm.getInetProtocol());
            index++;
        }
    }

    /**
     * Test case for {@link FlowMatchBuilder#setTcpSourcePort(Integer)}.
     */
    @Test
    public void testSetTcpSourcePort1() {
        FlowMatchBuilder fmbuilder = new FlowMatchBuilder();
        int index = 0;
        Integer[] ports = {
            null, 0, 1, 10, 60, 100, 5000, 33333, 65535,
        };

        // L4 protocol condition other than TCP should be cleared.
        fmbuilder.setUdpSourcePort(1).setUdpDestinationPort(2);

        for (Integer port: ports) {
            assertSame(fmbuilder, fmbuilder.setTcpSourcePort(port));
            FlowMatch fm = fmbuilder.build(index);
            L4Match lm = (port == null)
                ? null : new TcpMatch(new PortMatch(port), null);
            assertEquals(Integer.valueOf(index), fm.getIndex());
            assertEquals(null, fm.getEthernetMatch());
            assertEquals(null, fm.getInetMatch());
            assertEquals(lm, fm.getLayer4Match());
            assertEquals(-1, fm.getInetProtocol());

            fm = fmbuilder.build();
            assertEquals(null, fm.getIndex());
            assertEquals(null, fm.getEthernetMatch());
            assertEquals(null, fm.getInetMatch());
            assertEquals(lm, fm.getLayer4Match());
            assertEquals(-1, fm.getInetProtocol());
            index++;
        }
    }

    /**
     * Test case for {@link FlowMatchBuilder#setTcpSourcePort(int, int)}.
     */
    @Test
    public void testSetTcpSourcePort2() {
        FlowMatchBuilder fmbuilder = new FlowMatchBuilder();
        int index = 0;
        int[] ports = {
            3, 35, 63, 250, 7777, 65534,
        };

        // L4 protocol condition other than TCP should be cleared.
        fmbuilder.setUdpSourcePort(1).setUdpDestinationPort(2);

        for (int from: ports) {
            int to = from + 1;
            assertSame(fmbuilder, fmbuilder.setTcpSourcePort(from, to));
            FlowMatch fm = fmbuilder.build(index);
            L4Match lm = new TcpMatch(new PortMatch(from, to), null);
            assertEquals(Integer.valueOf(index), fm.getIndex());
            assertEquals(null, fm.getEthernetMatch());
            assertEquals(null, fm.getInetMatch());
            assertEquals(lm, fm.getLayer4Match());
            assertEquals(-1, fm.getInetProtocol());
            index++;
        }
    }

    /**
     * Test case for {@link FlowMatchBuilder#setTcpDestinationPort(Integer)}.
     */
    @Test
    public void testSetTcpDestinationPort1() {
        FlowMatchBuilder fmbuilder = new FlowMatchBuilder();
        int index = 0;
        Integer[] ports = {
            null, 0, 1, 10, 60, 100, 5000, 33333, 65535,
        };

        // L4 protocol condition other than TCP should be cleared.
        fmbuilder.setUdpSourcePort(1).setUdpDestinationPort(2);

        for (Integer port: ports) {
            assertSame(fmbuilder, fmbuilder.setTcpDestinationPort(port));
            FlowMatch fm = fmbuilder.build(index);
            L4Match lm = (port == null)
                ? null : new TcpMatch(null, new PortMatch(port));
            assertEquals(Integer.valueOf(index), fm.getIndex());
            assertEquals(null, fm.getEthernetMatch());
            assertEquals(null, fm.getInetMatch());
            assertEquals(lm, fm.getLayer4Match());
            assertEquals(-1, fm.getInetProtocol());

            fm = fmbuilder.build();
            assertEquals(null, fm.getIndex());
            assertEquals(null, fm.getEthernetMatch());
            assertEquals(null, fm.getInetMatch());
            assertEquals(lm, fm.getLayer4Match());
            assertEquals(-1, fm.getInetProtocol());
            index++;
        }
    }

    /**
     * Test case for {@link FlowMatchBuilder#setTcpDestinationPort(int, int)}.
     */
    @Test
    public void testSetTcpDestinationPort2() {
        FlowMatchBuilder fmbuilder = new FlowMatchBuilder();
        int index = 0;
        int[] ports = {
            3, 35, 63, 250, 7777, 65534,
        };

        // L4 protocol condition other than TCP should be cleared.
        fmbuilder.setUdpSourcePort(1).setUdpDestinationPort(2);

        for (int from: ports) {
            int to = from + 1;
            assertSame(fmbuilder, fmbuilder.setTcpDestinationPort(from, to));
            FlowMatch fm = fmbuilder.build(index);
            L4Match lm = new TcpMatch(null, new PortMatch(from, to));
            assertEquals(Integer.valueOf(index), fm.getIndex());
            assertEquals(null, fm.getEthernetMatch());
            assertEquals(null, fm.getInetMatch());
            assertEquals(lm, fm.getLayer4Match());
            assertEquals(-1, fm.getInetProtocol());
            index++;
        }
    }

    /**
     * Test case for {@link FlowMatchBuilder#setUdpSourcePort(Integer)}.
     */
    @Test
    public void testSetUdpSourcePort1() {
        FlowMatchBuilder fmbuilder = new FlowMatchBuilder();
        int index = 0;
        Integer[] ports = {
            null, 0, 1, 10, 60, 100, 5000, 33333, 65535,
        };

        // L4 protocol condition other than UDP should be cleared.
        fmbuilder.setIcmpType((short)10).setIcmpCode((short)20);

        for (Integer port: ports) {
            assertSame(fmbuilder, fmbuilder.setUdpSourcePort(port));
            FlowMatch fm = fmbuilder.build(index);
            L4Match lm = (port == null)
                ? null : new UdpMatch(new PortMatch(port), null);
            assertEquals(Integer.valueOf(index), fm.getIndex());
            assertEquals(null, fm.getEthernetMatch());
            assertEquals(null, fm.getInetMatch());
            assertEquals(lm, fm.getLayer4Match());
            assertEquals(-1, fm.getInetProtocol());

            fm = fmbuilder.build();
            assertEquals(null, fm.getIndex());
            assertEquals(null, fm.getEthernetMatch());
            assertEquals(null, fm.getInetMatch());
            assertEquals(lm, fm.getLayer4Match());
            assertEquals(-1, fm.getInetProtocol());
            index++;
        }
    }

    /**
     * Test case for {@link FlowMatchBuilder#setUdpSourcePort(int, int)}.
     */
    @Test
    public void testSetUdpSourcePort2() {
        FlowMatchBuilder fmbuilder = new FlowMatchBuilder();
        int index = 0;
        int[] ports = {
            3, 35, 63, 250, 7777, 65534,
        };

        // L4 protocol condition other than UDP should be cleared.
        fmbuilder.setIcmpType((short)10).setIcmpCode((short)20);

        for (int from: ports) {
            int to = from + 1;
            assertSame(fmbuilder, fmbuilder.setUdpSourcePort(from, to));
            FlowMatch fm = fmbuilder.build(index);
            L4Match lm = new UdpMatch(new PortMatch(from, to), null);
            assertEquals(Integer.valueOf(index), fm.getIndex());
            assertEquals(null, fm.getEthernetMatch());
            assertEquals(null, fm.getInetMatch());
            assertEquals(lm, fm.getLayer4Match());
            assertEquals(-1, fm.getInetProtocol());
            index++;
        }
    }


    /**
     * Test case for {@link FlowMatchBuilder#setUdpDestinationPort(Integer)}.
     */
    @Test
    public void testSetUdpDestinationPort1() {
        FlowMatchBuilder fmbuilder = new FlowMatchBuilder();
        int index = 0;
        Integer[] ports = {
            null, 0, 1, 10, 60, 100, 5000, 33333, 65535,
        };

        // L4 protocol condition other than UDP should be cleared.
        fmbuilder.setIcmpType((short)10).setIcmpCode((short)20);

        for (Integer port: ports) {
            assertSame(fmbuilder, fmbuilder.setUdpDestinationPort(port));
            FlowMatch fm = fmbuilder.build(index);
            L4Match lm = (port == null)
                ? null : new UdpMatch(null, new PortMatch(port));
            assertEquals(Integer.valueOf(index), fm.getIndex());
            assertEquals(null, fm.getEthernetMatch());
            assertEquals(null, fm.getInetMatch());
            assertEquals(lm, fm.getLayer4Match());
            assertEquals(-1, fm.getInetProtocol());

            fm = fmbuilder.build();
            assertEquals(null, fm.getIndex());
            assertEquals(null, fm.getEthernetMatch());
            assertEquals(null, fm.getInetMatch());
            assertEquals(lm, fm.getLayer4Match());
            assertEquals(-1, fm.getInetProtocol());
            index++;
        }
    }

    /**
     * Test case for {@link FlowMatchBuilder#setUdpDestinationPort(int, int)}.
     */
    @Test
    public void testSetUdpDestinationPort2() {
        FlowMatchBuilder fmbuilder = new FlowMatchBuilder();
        int index = 0;
        int[] ports = {
            3, 35, 63, 250, 7777, 65534,
        };

        // L4 protocol condition other than UDP should be cleared.
        fmbuilder.setIcmpType((short)10).setIcmpCode((short)20);

        for (int from: ports) {
            int to = from + 1;
            assertSame(fmbuilder, fmbuilder.setUdpDestinationPort(from, to));
            FlowMatch fm = fmbuilder.build(index);
            L4Match lm = new UdpMatch(null, new PortMatch(from, to));
            assertEquals(Integer.valueOf(index), fm.getIndex());
            assertEquals(null, fm.getEthernetMatch());
            assertEquals(null, fm.getInetMatch());
            assertEquals(lm, fm.getLayer4Match());
            assertEquals(-1, fm.getInetProtocol());
            index++;
        }
    }

    /**
     * Test case for {@link FlowMatchBuilder#setIcmpType(Short)}.
     */
    @Test
    public void testSetIcmpType() {
        FlowMatchBuilder fmbuilder = new FlowMatchBuilder();
        int index = 0;
        Short[] types = {
            null, 0, 1, 20, 55, 120, 230, 255,
        };

        // L4 protocol condition other than ICMP should be cleared.
        fmbuilder.setTcpSourcePort(10).setTcpDestinationPort(20);

        for (Short type: types) {
            assertSame(fmbuilder, fmbuilder.setIcmpType(type));
            FlowMatch fm = fmbuilder.build(index);
            L4Match lm = (type == null)
                ? null : new IcmpMatch(type, null);
            assertEquals(Integer.valueOf(index), fm.getIndex());
            assertEquals(null, fm.getEthernetMatch());
            assertEquals(null, fm.getInetMatch());
            assertEquals(lm, fm.getLayer4Match());
            assertEquals(-1, fm.getInetProtocol());

            fm = fmbuilder.build();
            assertEquals(null, fm.getIndex());
            assertEquals(null, fm.getEthernetMatch());
            assertEquals(null, fm.getInetMatch());
            assertEquals(lm, fm.getLayer4Match());
            assertEquals(-1, fm.getInetProtocol());
            index++;
        }
    }

    /**
     * Test case for {@link FlowMatchBuilder#setIcmpCode(Short)}.
     */
    @Test
    public void testSetIcmpCode() {
        FlowMatchBuilder fmbuilder = new FlowMatchBuilder();
        int index = 0;
        Short[] codes = {
            null, 0, 3, 31, 67, 133, 219, 255,
        };

        // L4 protocol condition other than ICMP should be cleared.
        fmbuilder.setTcpSourcePort(10).setTcpDestinationPort(20);

        for (Short code: codes) {
            assertSame(fmbuilder, fmbuilder.setIcmpCode(code));
            FlowMatch fm = fmbuilder.build(index);
            L4Match lm = (code == null)
                ? null : new IcmpMatch(null, code);
            assertEquals(Integer.valueOf(index), fm.getIndex());
            assertEquals(null, fm.getEthernetMatch());
            assertEquals(null, fm.getInetMatch());
            assertEquals(lm, fm.getLayer4Match());
            assertEquals(-1, fm.getInetProtocol());

            fm = fmbuilder.build();
            assertEquals(null, fm.getIndex());
            assertEquals(null, fm.getEthernetMatch());
            assertEquals(null, fm.getInetMatch());
            assertEquals(lm, fm.getLayer4Match());
            assertEquals(-1, fm.getInetProtocol());
            index++;
        }
    }

    /**
     * Test case for {@link FlowMatchBuilder#build(int)} and
     * {@link FlowMatchBuilder#reset()}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testBuild() throws Exception {
        int index = 1;
        FlowMatchBuilder fmbuilder = new FlowMatchBuilder();
        FlowMatch fm = fmbuilder.build(index);
        assertEquals(Integer.valueOf(index), fm.getIndex());
        assertEquals(null, fm.getEthernetMatch());
        assertEquals(null, fm.getInetMatch());
        assertEquals(null, fm.getLayer4Match());
        assertEquals(-1, fm.getInetProtocol());

        FlowMatch fm1 = fmbuilder.build(index);
        assertEquals(fm, fm1);
        assertNotSame(fm, fm1);
        index++;

        // L2 conditions.
        byte[] srcMac = {
            (byte)0x00, (byte)0x01, (byte)0x02,
            (byte)0x03, (byte)0x04, (byte)0x05,
        };
        byte[] dstMac = {
            (byte)0x1a, (byte)0x2b, (byte)0x3c,
            (byte)0x4d, (byte)0x5e, (byte)0x6f,
        };
        EthernetAddress ethSrc = new EthernetAddress(srcMac);
        EthernetAddress ethDst = new EthernetAddress(dstMac);
        Integer ethType = Integer.valueOf(0x806);
        Short ethVlan = Short.valueOf((short)4095);
        Byte ethPri = Byte.valueOf((byte)7);
        EthernetMatch em = new EthernetMatch(ethSrc, ethDst, ethType, ethVlan,
                                             ethPri);
        fmbuilder.setSourceMacAddress(ethSrc).
            setDestinationMacAddress(ethDst).
            setEtherType(ethType.intValue()).
            setVlanId(ethVlan.shortValue()).
            setVlanPriority(ethPri.byteValue());
        fm = fmbuilder.build(index);
        assertEquals(Integer.valueOf(index), fm.getIndex());
        assertEquals(em, fm.getEthernetMatch());
        assertEquals(null, fm.getInetMatch());
        assertEquals(null, fm.getLayer4Match());
        assertEquals(-1, fm.getInetProtocol());

        fm1 = fmbuilder.build(index);
        assertEquals(fm, fm1);
        assertNotSame(fm, fm1);
        index++;

        // L3 conditions.
        InetAddress inetSrc = InetAddress.getByName("192.168.100.2");
        InetAddress inetDst = InetAddress.getByName("10.250.132.10");
        Short inetSrcSuff = Short.valueOf((short)24);
        Short inetDstSuff = Short.valueOf((short)31);
        Short inetProto = Short.valueOf((short)100);
        Byte inetDscp = Byte.valueOf((byte)15);
        InetMatch im = new Inet4Match(inetSrc, inetSrcSuff, inetDst,
                                      inetDstSuff, inetProto, inetDscp);
        ethType = Integer.valueOf(0x800);
        em = new EthernetMatch(ethSrc, ethDst, ethType, ethVlan, ethPri);

        fmbuilder.setEtherType(ethType.intValue()).
            setSourceInetAddress(inetSrc).
            setSourceInetSuffix(inetSrcSuff.shortValue()).
            setDestinationInetAddress(inetDst).
            setDestinationInetSuffix(inetDstSuff.shortValue()).
            setInetProtocol(inetProto.shortValue()).
            setInetDscp(inetDscp.byteValue());

        fm = fmbuilder.build(index);
        assertEquals(Integer.valueOf(index), fm.getIndex());
        assertEquals(em, fm.getEthernetMatch());
        assertEquals(im, fm.getInetMatch());
        assertEquals(null, fm.getLayer4Match());
        assertEquals(inetProto.intValue(), fm.getInetProtocol());

        fm1 = fmbuilder.build(index);
        assertEquals(fm, fm1);
        assertNotSame(fm, fm1);
        index++;

        // L4 conditions.
        InetProtocols[] ipprotos = {
            InetProtocols.TCP, InetProtocols.UDP, InetProtocols.ICMP,
        };
        for (InetProtocols ipproto: ipprotos) {
            short proto = ipproto.shortValue();
            im = new Inet4Match(inetSrc, inetSrcSuff, inetDst, inetDstSuff,
                                Short.valueOf(proto), inetDscp);
            fmbuilder.setInetProtocol(proto);

            L4Match lm;
            switch (ipproto) {
            case TCP:
                lm = new TcpMatch(new PortMatch(100), new PortMatch(60000));
                fmbuilder.setTcpSourcePort(100).setTcpDestinationPort(60000);
                break;

            case UDP:
                lm = new UdpMatch(new PortMatch(10, 500),
                                  new PortMatch(30000, 45000));
                fmbuilder.setUdpSourcePort(10, 500).
                    setUdpDestinationPort(30000, 45000);
                break;

            default:
                short type = 10;
                short code = 30;
                lm = new IcmpMatch(Short.valueOf(type), Short.valueOf(code));
                fmbuilder.setIcmpType(type).setIcmpCode(code);
                break;
            }

            fm = fmbuilder.build(index);
            assertEquals(Integer.valueOf(index), fm.getIndex());
            assertEquals(em, fm.getEthernetMatch());
            assertEquals(im, fm.getInetMatch());
            assertEquals(lm, fm.getLayer4Match());
            assertEquals(ipproto.intValue(), fm.getInetProtocol());

            fm1 = fmbuilder.build(index);
            assertEquals(fm, fm1);
            assertNotSame(fm, fm1);
            index++;
        }

        fmbuilder.reset();
        fm = fmbuilder.build(index);
        assertEquals(Integer.valueOf(index), fm.getIndex());
        assertEquals(null, fm.getEthernetMatch());
        assertEquals(null, fm.getInetMatch());
        assertEquals(null, fm.getLayer4Match());
        assertEquals(-1, fm.getInetProtocol());

        fm1 = fmbuilder.build(index);
        assertEquals(fm, fm1);
        assertNotSame(fm, fm1);
        index++;

        for (InetProtocols ipproto: ipprotos) {
            fmbuilder.setSourceMacAddress(ethSrc).
                setDestinationMacAddress(ethDst).
                setEtherType(ethType.intValue()).
                setVlanId(ethVlan.shortValue()).
                setVlanPriority(ethPri.byteValue());

            short proto = ipproto.shortValue();
            im = new Inet4Match(inetSrc, inetSrcSuff, inetDst, inetDstSuff,
                                Short.valueOf(proto), inetDscp);
            fmbuilder.setSourceInetAddress(inetSrc).
                setSourceInetSuffix(inetSrcSuff.shortValue()).
                setDestinationInetAddress(inetDst).
                setDestinationInetSuffix(inetDstSuff.shortValue()).
                setInetProtocol(proto).
                setInetDscp(inetDscp.byteValue());

            L4Match lm;
            switch (ipproto) {
            case TCP:
                lm = new TcpMatch(new PortMatch(33, 45),
                                  new PortMatch(12300, 45600));
                fmbuilder.setTcpSourcePort(33, 45).
                    setTcpDestinationPort(12300, 45600);
                break;

            case UDP:
                lm = new UdpMatch(new PortMatch(32800), new PortMatch(65500));
                fmbuilder.setUdpSourcePort(32800).setUdpDestinationPort(65500);
                break;

            default:
                short type = 0;
                short code = 8;
                lm = new IcmpMatch(Short.valueOf(type), Short.valueOf(code));
                fmbuilder.setIcmpType(type).setIcmpCode(code);
                break;
            }

            fm = fmbuilder.build(index);
            assertEquals(Integer.valueOf(index), fm.getIndex());
            assertEquals(em, fm.getEthernetMatch());
            assertEquals(im, fm.getInetMatch());
            assertEquals(lm, fm.getLayer4Match());
            assertEquals(ipproto.intValue(), fm.getInetProtocol());

            fm1 = fmbuilder.build(index);
            assertEquals(fm, fm1);
            assertNotSame(fm, fm1);
            index++;

            fmbuilder.reset();
            fm = fmbuilder.build(index);
            assertEquals(Integer.valueOf(index), fm.getIndex());
            assertEquals(null, fm.getEthernetMatch());
            assertEquals(null, fm.getInetMatch());
            assertEquals(null, fm.getLayer4Match());
            assertEquals(-1, fm.getInetProtocol());

            fm1 = fmbuilder.build(index);
            assertEquals(fm, fm1);
            assertNotSame(fm, fm1);
            index++;
        }
    }
}
