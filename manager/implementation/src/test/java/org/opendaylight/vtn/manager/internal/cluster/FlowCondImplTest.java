/*
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.cluster;

import java.io.File;
import java.net.InetAddress;
import java.util.ArrayList;
import java.util.HashSet;
import java.util.LinkedList;
import java.util.Map;
import java.util.TreeMap;

import org.junit.Test;

import org.opendaylight.vtn.manager.VTNException;
import org.opendaylight.vtn.manager.flow.cond.EthernetMatch;
import org.opendaylight.vtn.manager.flow.cond.FlowCondition;
import org.opendaylight.vtn.manager.flow.cond.FlowMatch;
import org.opendaylight.vtn.manager.flow.cond.Inet4Match;
import org.opendaylight.vtn.manager.flow.cond.TcpMatch;

import org.opendaylight.vtn.manager.internal.ContainerConfig;
import org.opendaylight.vtn.manager.internal.VTNManagerImpl;

import org.opendaylight.vtn.manager.internal.DataGenerator;
import org.opendaylight.vtn.manager.internal.TestBase;

import org.opendaylight.controller.sal.core.UpdateType;
import org.opendaylight.controller.sal.match.MatchType;
import org.opendaylight.controller.sal.packet.Ethernet;
import org.opendaylight.controller.sal.packet.ICMP;
import org.opendaylight.controller.sal.packet.IPv4;
import org.opendaylight.controller.sal.packet.TCP;
import org.opendaylight.controller.sal.packet.UDP;
import org.opendaylight.controller.sal.utils.EtherTypes;
import org.opendaylight.controller.sal.utils.GlobalConstants;
import org.opendaylight.controller.sal.utils.IPProtocols;
import org.opendaylight.controller.sal.utils.Status;
import org.opendaylight.controller.sal.utils.StatusCode;

/**
 * JUnit test for {@link FlowCondImpl}.
 */
public class FlowCondImplTest extends TestBase {
    /**
     * Test case for basic methods.
     *
     * @throws VTNException  An error occurred.
     */
    @Test
    public void testBasic() throws VTNException {
        VTNManagerImpl mgr = getVTNManager();
        ContainerConfig cfg = new ContainerConfig(mgr.getContainerName());
        cfg.init();

        // Create an empty flow condition.
        String name = "flow_condition";
        FlowCondImpl fci = new FlowCondImpl(name, null);
        FlowCondition fc = new FlowCondition(name, null);
        assertEquals(name, fci.getName());
        assertEquals(fc, fci.getFlowCondition());
        serializeTest(fci);
        for (int idx = 0; idx <= 10; idx++) {
            assertEquals(null, fci.getMatch(idx));
            assertEquals(null, fci.removeMatch(idx));

            FlowCondImpl copy = fci.clone();
            assertNotSame(fci, copy);
            assertNotSame(copy, fci.clone());
            assertEquals(fci, copy);
            assertEquals(name, copy.getName());
            assertEquals(fc, copy.getFlowCondition());
            assertEquals(null, copy.getMatch(idx));
            assertEquals(null, copy.removeMatch(idx));
        }

        File path = getPath(name);
        Status st = fci.saveConfig(mgr);
        assertTrue(st.isSuccess());
        assertTrue(path.isFile());
        assertEquals(fci, cfg.load(ContainerConfig.Type.FLOWCOND, name));
        fci.destroy(mgr);
        assertFalse(path.isFile());

        // Configure FlowMatch.
        Map<FlowMatch, FlowMatch> map = FlowMatchImplTest.createFlowMatches();
        int index = 1;
        boolean first = true;
        FlowMatchImpl fmi = null;
        for (Map.Entry<FlowMatch, FlowMatch> entry: map.entrySet()) {
            FlowMatch key = entry.getKey();
            FlowMatch value = entry.getValue();
            FlowMatch fm = key.assignIndex(index);
            FlowMatch expected = value.assignIndex(index);
            fmi = new FlowMatchImpl(expected);
            UpdateType result = fci.setMatch(fm);
            if (first) {
                assertEquals(UpdateType.ADDED, result);
            } else {
                assertEquals(UpdateType.CHANGED, result);
            }
            first = false;

            assertEquals(null, fci.setMatch(fm));

            assertEquals(expected, fci.getMatch(index));
            ArrayList<FlowMatch> matches = new ArrayList<FlowMatch>();
            matches.add(expected);
            fc = new FlowCondition(name, matches);
            assertEquals(fc, fci.getFlowCondition());
            serializeTest(fci);

            FlowCondImpl copy = fci.clone();
            assertNotSame(fci, copy);
            assertNotSame(copy, fci.clone());
            assertEquals(fci, copy);
            assertEquals(name, copy.getName());
            assertEquals(fc, copy.getFlowCondition());
            assertEquals(expected, copy.getMatch(index));

            st = fci.saveConfig(mgr);
            assertTrue(st.isSuccess());
            assertTrue(path.isFile());
            assertEquals(fci, cfg.load(ContainerConfig.Type.FLOWCOND, name));
            fci.destroy(mgr);
            assertFalse(path.isFile());
        }

        assertEquals(fmi, fci.removeMatch(index));

        // Configure all matches into one flow condition.
        LinkedList<FlowMatch> matchList = new LinkedList<FlowMatch>();
        ArrayList<FlowMatch> expectedList = new ArrayList<FlowMatch>();
        for (Map.Entry<FlowMatch, FlowMatch> entry: map.entrySet()) {
            FlowMatch key = entry.getKey();
            FlowMatch value = entry.getValue();
            FlowMatch fm = key.assignIndex(index);
            FlowMatch expected = value.assignIndex(index);
            expectedList.add(expected);
            assertEquals(UpdateType.ADDED, fci.setMatch(fm));
            assertEquals(null, fci.setMatch(fm));
            index += 13;

            // Reverse order of match list.
            matchList.addFirst(fm);
        }

        FlowCondition expectedCond = new FlowCondition(name, expectedList);
        assertEquals(expectedCond, fci.getFlowCondition());
        serializeTest(fci);

        FlowCondImpl copy = fci.clone();
        assertNotSame(fci, copy);
        assertNotSame(copy, fci.clone());
        assertEquals(fci, copy);
        assertEquals(name, copy.getName());
        assertEquals(expectedCond, copy.getFlowCondition());

        st = fci.saveConfig(mgr);
        assertTrue(st.isSuccess());
        assertTrue(path.isFile());
        assertEquals(fci, cfg.load(ContainerConfig.Type.FLOWCOND, name));
        fci.destroy(mgr);
        assertFalse(path.isFile());

        for (FlowMatch fm: expectedList) {
            int idx = fm.getIndex().intValue();
            assertEquals(fm, fci.getMatch(idx));
        }
        for (int i = 1; i <= 10; i++) {
            int idx = index + i;
            assertEquals(null, fci.getMatch(idx));
            assertEquals(null, fci.removeMatch(idx));

            idx = 1 - i;
            assertEquals(null, fci.getMatch(idx));
            assertEquals(null, fci.removeMatch(idx));
        }

        // Create another flow condition that has the same contents.
        fc = new FlowCondition(null, matchList);
        FlowCondImpl fci1 = new FlowCondImpl(name, fc);
        assertEquals(name, fci1.getName());
        assertEquals(expectedCond, fci1.getFlowCondition());
        assertEquals(fci, fci1);
        assertFalse(fci1.setMatches(matchList));
        assertFalse(fci1.setMatches(expectedList));

        // Remove all flow matches in fci1.
        for (FlowMatch fm: expectedList) {
            int idx = fm.getIndex().intValue();
            fmi = new FlowMatchImpl(fm);
            assertEquals(fm, fci1.getMatch(idx));
            assertEquals(fmi, fci1.removeMatch(idx));
            assertEquals(null, fci1.removeMatch(idx));
            assertEquals(null, fci1.getMatch(idx));
        }

        // Restore all flow matches.
        assertTrue(fci1.setMatches(matchList));
        assertFalse(fci1.setMatches(expectedList));
        assertEquals(name, fci1.getName());
        assertEquals(expectedCond, fci1.getFlowCondition());
        assertEquals(fci, fci1);

        // Specify duplicated index.
        LinkedList<FlowMatch> badList = new LinkedList<FlowMatch>(matchList);
        FlowMatch dup1 = badList.getFirst();
        int duplicate = dup1.getIndex().intValue();
        FlowMatch dup2 = badList.removeLast();
        assertNotSame(dup1, dup2);
        badList.addLast(dup2.assignIndex(duplicate));
        try {
            new FlowCondImpl(name, new FlowCondition(null, badList));
            unexpected();
        } catch (VTNException e) {
            st = e.getStatus();
            assertEquals(StatusCode.BADREQUEST, st.getCode());
            assertEquals("Duplicate match index: " + duplicate,
                         st.getDescription());
        }

        try {
            fci1.setMatches(badList);
            unexpected();
        } catch (VTNException e) {
            st = e.getStatus();
            assertEquals(StatusCode.BADREQUEST, st.getCode());
            assertEquals("Duplicate match index: " + duplicate,
                         st.getDescription());
        }

        assertEquals(name, fci1.getName());
        assertEquals(expectedCond, fci1.getFlowCondition());
        assertEquals(fci, fci1);
    }

    /**
     * Test case for {@link FlowCondImpl#equals(Object)} and
     * {@link FlowCondImpl#hashCode()}.
     *
     * @throws VTNException  An error occurred.
     */
    @Test
    public void testEquals() throws VTNException {
        HashSet<Object> set = new HashSet<Object>();

        String[] names = {"name_1", "name_2", "cond_1"};
        Map<FlowMatch, FlowMatch> map = FlowMatchImplTest.createFlowMatches();
        ArrayList<FlowMatch> matchList = new ArrayList<FlowMatch>();
        int index = 1;
        int count = 0;
        for (FlowMatch key: map.keySet()) {
            FlowMatch fm = key.assignIndex(index);
            index++;
            ArrayList<FlowMatch> l = new ArrayList<FlowMatch>();
            l.add(fm);
            matchList.add(fm);
            for (String name: names) {
                FlowCondition fc = new FlowCondition(null, l);
                FlowCondImpl fci1 = new FlowCondImpl(name, fc);
                FlowCondImpl fci2 = new FlowCondImpl(copy(name), fc);
                testEquals(set, fci1, fci2);
                count++;

                if (matchList.size() > 1) {
                    fc = new FlowCondition(null, matchList);
                    fci1 = new FlowCondImpl(name, fc);
                    fci2 = new FlowCondImpl(copy(name), fc);
                    testEquals(set, fci1, fci2);
                    count++;
                }
            }
        }

        for (String name: names) {
            FlowCondition fc = new FlowCondition(null, null);
            FlowCondImpl fci1 = new FlowCondImpl(name, fc);
            FlowCondImpl fci2 = new FlowCondImpl(copy(name), fc);
            testEquals(set, fci1, fci2);
            count++;
        }

        assertEquals(count, set.size());
    }

    /**
     * Test case for {@link FlowCondImpl#toString()}.
     *
     * @throws VTNException  An error occurred.
     */
    @Test
    public void testToString() throws VTNException {
        String prefix = "FlowCondImpl[";
        String suffix = "]";

        String[] names = {"name_1", "name_2", "cond_1"};
        Map<FlowMatch, FlowMatch> map = FlowMatchImplTest.createFlowMatches();
        for (String name: names) {
            TreeMap<Integer, FlowMatchImpl> matches =
                new TreeMap<Integer, FlowMatchImpl>();
            String n = "name=" + name;
            String m = "matches=" + matches.values();
            FlowCondImpl fci = new FlowCondImpl(name, null);
            String required = joinStrings(prefix, suffix, ",", n, m);
            assertEquals(required, fci.toString());

            int index = 1;
            for (Map.Entry<FlowMatch, FlowMatch> entry: map.entrySet()) {
                FlowMatch key = entry.getKey();
                FlowMatch value = entry.getValue();
                FlowMatch fm = key.assignIndex(index);
                FlowMatch expected = value.assignIndex(index);
                assertEquals(UpdateType.ADDED, fci.setMatch(fm));
                matches.put(index, new FlowMatchImpl(expected));
                m = "matches=" + matches.values();
                required = joinStrings(prefix, suffix, ",", n, m);
                assertEquals(required, fci.toString());
                index++;
            }
        }
    }

    /**
     * Test case for
     * {@link FlowCondImpl#match(VTNManagerImpl mgr, PacketContext pctx)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testMatch() throws Exception {
        // Create an empty flow condition.
        FlowCondImpl empty = new FlowCondImpl("empty", null);

        // Create a flow condition.
        FlowCondImpl fci = new FlowCondImpl("empty", null);

        // 10000: IP DSCP == 35
        byte dscp = 35;
        Inet4MatchImplBuilder inet4Builder = new Inet4MatchImplBuilder();
        Inet4Match im = inet4Builder.setDscp(dscp).getInet4Match();
        FlowMatch fm = new FlowMatch(10000, null, im, null);
        assertEquals(UpdateType.ADDED, fci.setMatch(fm));

        // 10: ether type == 0x1234
        int ethType = 0x1234;
        EthernetMatchImplBuilder ethBuilder = new EthernetMatchImplBuilder();
        EthernetMatch em = ethBuilder.reset().setEtherType(ethType).
            getEthernetMatch();
        fm = new FlowMatch(10, em, null, null);
        assertEquals(UpdateType.ADDED, fci.setMatch(fm));

        // 1500: TCP dst port 1000-2000
        TcpMatchImplBuilder tcpBuilder = new TcpMatchImplBuilder();
        short dstFrom = 1000;
        short dstTo = 2000;
        TcpMatch tm = tcpBuilder.setDestinationPortFrom(dstFrom).
            setDestinationPortTo(dstTo).getTcpMatch();
        fm = new FlowMatch(1500, null, null, tm);
        assertEquals(UpdateType.ADDED, fci.setMatch(fm));

        // 300: Source MAC == mac1 && VLAN ID == 3
        DataGenerator gen = new DataGenerator();
        byte[] mac1 = gen.getMacAddress();
        short vlan = 3;
        em = ethBuilder.reset().setSourceAddress(mac1).setVlanId(vlan).
            getEthernetMatch();
        fm = new FlowMatch(300, em, null, null);
        assertEquals(UpdateType.ADDED, fci.setMatch(fm));

        // 150: Source IP address in 192.168.233.0/24
        im = inet4Builder.reset().setSourceAddress("192.168.233.0").
            setSourceSuffix((short)24).getInet4Match();
        fm = new FlowMatch(150, null, im, null);
        assertEquals(UpdateType.ADDED, fci.setMatch(fm));

        // Create an untagged Ethernet frame with type 0x5555.
        byte[] payload = {
            (byte)0xf4, (byte)0xec, (byte)0x8b, (byte)0x72,
            (byte)0x67, (byte)0x43, (byte)0x5e, (byte)0x20,
            (byte)0x28, (byte)0xc9, (byte)0x0f, (byte)0x7c,
        };

        byte[] mac2 = gen.getMacAddress();
        byte[] mac3 = gen.getMacAddress();
        Ethernet ether = createEthernet(mac2, mac3, 0x5555,
                                        MatchType.DL_VLAN_NONE, (byte)0,
                                        payload);

        // Empty flow condition should match every packet.
        PacketMatchTest test = new PacketMatchTest();
        VTNManagerImpl mgr = getVTNManager();
        assertEquals(true, test.run(mgr, empty, ether));

        // This packet should not be matched.
        test.setMatchType(MatchType.DL_TYPE, MatchType.DL_SRC);
        assertEquals(false, test.run(mgr, fci, ether));

        // Create an Ethernet frame with VLAN tag(vid=10) and type 0x1234.
        // FlowMatch at index 10 should match this packet.
        ether = createEthernet(mac2, mac3, ethType, (short)10, (byte)0,
                               payload);
        assertEquals(true, test.reset().run(mgr, empty, ether));
        test.setMatchType(MatchType.DL_TYPE);
        assertEquals(true, test.run(mgr, fci, ether));

        // Create an Ethernet frame to be matched by FlowMatch at index 300.
        ether = createEthernet(mac1, mac3, 0x3333, vlan, (byte)0, payload);
        assertEquals(true, test.reset().run(mgr, empty, ether));
        test.setMatchType(MatchType.DL_TYPE, MatchType.DL_SRC);
        assertEquals(true, test.run(mgr, fci, ether));

        // Create an UDP packet to be matched by FlowMatch at index 10000.
        UDP udp = new UDP();
        udp.setSourcePort((short)gen.getPort()).
            setDestinationPort((short)gen.getPort()).
            setLength((short)123).setChecksum((short)0x3333).
            setRawPayload(payload);
        InetAddress ipaddr1 = gen.getInetAddress();
        InetAddress ipaddr2 = gen.getInetAddress();
        IPv4 ipv4 = createIPv4(ipaddr1, ipaddr2, IPProtocols.UDP.shortValue(),
                               dscp);
        ipv4.setPayload(udp);
        ether = createEthernet(mac2, mac3, EtherTypes.IPv4.intValue(),
                               (short)4095, (byte)2, ipv4);
        assertEquals(true, test.reset().run(mgr, empty, ether));
        test.setMatchType(MatchType.DL_TYPE, MatchType.DL_SRC,
                          MatchType.NW_SRC, MatchType.NW_PROTO,
                          MatchType.NW_TOS);
        assertEquals(true, test.run(mgr, fci, ether));

        // Change DSCP so that the packet is not be matched.
        ipv4 = createIPv4(ipaddr1, ipaddr2, IPProtocols.UDP.shortValue(),
                          (byte)(dscp + 1));
        ipv4.setPayload(udp);
        ether = createEthernet(mac2, mac3, EtherTypes.IPv4.intValue(),
                               (short)4095, (byte)2, ipv4);
        assertEquals(true, test.reset().run(mgr, empty, ether));
        test.setMatchType(MatchType.DL_TYPE, MatchType.DL_SRC,
                          MatchType.NW_SRC, MatchType.NW_PROTO,
                          MatchType.NW_TOS);
        assertEquals(false, test.run(mgr, fci, ether));

        // Create an ICMP packet to be matched by FlowMatch at index 150.
        InetAddress inetSrc = InetAddress.getByName("192.168.233.98");
        ICMP icmp = new ICMP();
        icmp.setType((byte)gen.getPort()).setCode((byte)gen.getPort()).
            setIdentifier((short)0x1234).setSequenceNumber((short)0x5678).
            setRawPayload(payload);
        ipv4 = createIPv4(inetSrc, ipaddr2, IPProtocols.ICMP.shortValue(),
                          dscp);
        ipv4.setPayload(icmp);
        ether = createEthernet(mac2, mac3, EtherTypes.IPv4.intValue(),
                               (short)30, (byte)1, ipv4);
        assertEquals(true, test.reset().run(mgr, empty, ether));
        test.setMatchType(MatchType.DL_TYPE, MatchType.NW_SRC);
        assertEquals(true, test.run(mgr, fci, ether));

        // Create a TCP packet.
        short[] ports = {
            (short)(dstFrom - 1),
            dstFrom,
            (short)(dstFrom + ((dstTo - dstFrom) >> 1)),
            dstTo,
            (short)(dstTo + 1),
        };
        for (short port: ports) {
            TCP tcp = new TCP();
            tcp.setSourcePort((short)gen.getPort()).setDestinationPort(port).
                setSequenceNumber(0x12345).setAckNumber(0x23456).
                setDataOffset((byte)1).setHeaderLenFlags((short)0x18).
                setReserved((byte)0).setWindowSize((short)1500).
                setChecksum((short)0x9999).setRawPayload(payload);
            ipv4 = createIPv4(ipaddr1, ipaddr2, IPProtocols.TCP.shortValue(),
                              dscp);
            ipv4.setPayload(tcp);
            ether = createEthernet(mac2, mac3, EtherTypes.IPv4.intValue(),
                                   (short)30, (byte)1, ipv4);
            assertEquals(true, test.reset().run(mgr, empty, ether));

            test.setMatchType(MatchType.DL_TYPE, MatchType.DL_SRC,
                              MatchType.NW_SRC, MatchType.NW_PROTO,
                              MatchType.TP_DST);

            // If the destination port is in range [1000, 2000], the FlowMatch
            // at index 1500 should match the packet.
            // Otherwise the last FlowMatch should match the packet.
            if (port < dstFrom || port > dstTo) {
                test.setMatchType(MatchType.NW_TOS);
            }
            assertEquals(true, test.run(mgr, fci, ether));
        }

        // Create a TCP packet that should not be mached.
        TCP tcp = new TCP();
        tcp.setSourcePort((short)gen.getPort()).
            setDestinationPort((short)(dstTo + 1)).
            setSequenceNumber(0x12345).setAckNumber(0x23456).
            setDataOffset((byte)1).setHeaderLenFlags((short)0x18).
            setReserved((byte)0).setWindowSize((short)1500).
            setChecksum((short)0x9999).setRawPayload(payload);
        ipv4 = createIPv4(ipaddr1, ipaddr2, IPProtocols.TCP.shortValue(),
                          (byte)(dscp + 1));
        ipv4.setPayload(tcp);
        ether = createEthernet(mac2, mac3, EtherTypes.IPv4.intValue(),
                               (short)30, (byte)1, ipv4);
        assertEquals(true, test.reset().run(mgr, empty, ether));

        test.setMatchType(MatchType.DL_TYPE, MatchType.DL_SRC,
                          MatchType.NW_SRC, MatchType.NW_PROTO,
                          MatchType.NW_TOS, MatchType.TP_DST);
        assertEquals(false, test.run(mgr, fci, ether));
    }

    /**
     * Create VTN Manager instance for test.
     *
     * @return {@link VTNManagerImpl} instance.
     */
    private VTNManagerImpl getVTNManager() {
        return new VTNManagerImpl() {
            @Override
            public String getContainerName() {
                return GlobalConstants.DEFAULT.toString();
            }
        };
    }

    /**
     * Return a path to the configuration file corresponding to the
     * specified flow condition.
     *
     * @param name  The name of the flow condition.
     * @return  A {@link File} instance associated with the configuration file
     *          of the flow condition.
     */
    private File getPath(String name) {
        String base = GlobalConstants.STARTUPHOME.toString();
        File parent = new File(base, GlobalConstants.DEFAULT.toString());
        parent = new File(parent, "vtn");
        parent = new File(parent, "FLOWCOND");
        return new File(parent, name + ".conf");
    }
}
