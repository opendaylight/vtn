/*
 * Copyright (c) 2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */
package org.opendaylight.vtn.manager.internal.cluster;

import static org.junit.Assert.*;

import java.net.InetAddress;
import java.util.HashSet;
import java.util.Set;

import org.junit.Test;
import org.opendaylight.controller.sal.packet.address.EthernetAddress;
import org.opendaylight.controller.sal.utils.HexEncode;
import org.opendaylight.controller.sal.utils.NetUtils;
import org.opendaylight.vtn.manager.VBridgePath;
import org.opendaylight.vtn.manager.internal.TestBase;

/**
 * JUnit Test for {@link MacTableEntryId}.
 */
public class MacTableEntryIdTest extends TestBase{

    /**
     * Test case for getter methods.
     */
    @Test
    public void testGetter() {
        long[] values = new long[] {0, Long.MIN_VALUE, Long.MAX_VALUE};
        InetAddress loopback = InetAddress.getLoopbackAddress();
        ClusterEventId.setLocalAddress(null);

        MacTableEntryId cev = new MacTableEntryId(new VBridgePath("vtn", "vbr"),
                                                  0L);
        assertTrue(cev.isLocal());

        for (String tname : createStrings("tenant")) {
            for (String bname : createStrings("vbr")) {
                VBridgePath bpath = new VBridgePath(tname, bname);

                for (EthernetAddress ea : createEthernetAddresses(false)) {
                    ClusterEventId.setLocalAddress(null);
                    long mac = NetUtils.byteArray6ToLong(ea.getValue());
                    String emsg = "(VBridgePath)" + bpath.toString()
                            + ",(EthernetAddress)" + ea.toString();

                    // create object by MacTableEntryId(VBridgePath, long).
                    cev = new MacTableEntryId(bpath, mac);
                    long eventid = cev.getEventId();
                    assertEquals(emsg, loopback, cev.getControllerAddress());
                    assertEquals(emsg, bpath, cev.getBridgePath());
                    assertEquals(emsg, mac, cev.getMacAddress());

                    cev = new MacTableEntryId(bpath, mac);
                    assertEquals(emsg, eventid + 1, cev.getEventId());
                    assertEquals(emsg, loopback, cev.getControllerAddress());
                    assertEquals(emsg, bpath, cev.getBridgePath());
                    assertEquals(emsg, mac, cev.getMacAddress());

                    // create object by
                    // MacTableEntryId(InetAddress, long, VBridgePath, long).
                    for (long id : values) {
                        for (Set<InetAddress> iset : createInetAddresses(false)) {
                            for (InetAddress ipaddr : iset) {
                                String emsgl = emsg + ",(id)" + id
                                        + ",(InetAddress)" + ipaddr.toString();
                                cev = new MacTableEntryId(ipaddr, id, bpath, mac);
                                assertEquals(emsgl, id, cev.getEventId());
                                assertEquals(emsgl,
                                             ipaddr, cev.getControllerAddress());
                                assertEquals(emsgl, bpath, cev.getBridgePath());
                                assertEquals(emsgl, mac, cev.getMacAddress());

                                if (ipaddr != null) {
                                    ClusterEventId.setLocalAddress(ipaddr);
                                    assertTrue(emsgl, cev.isLocal());
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    /**
     * Test method for
     * {@link FlowGroupId#hashCode()},
     * {@link FlowGroupId#equals(Object)}.
     */
    @Test
    public void testEquals() {
        long[] values = new long[] {0, Long.MIN_VALUE, Long.MAX_VALUE};
        Set<Object> cevSet = new HashSet<Object>();
        Set<InetAddress> ipSet = new HashSet<InetAddress>();

        ClusterEventId.setLocalAddress(null);

        int numSet = 0;
        for (String tname : createStrings("tenant")) {
            for (String bname : createStrings("vbr")) {
                VBridgePath bpath = new VBridgePath(tname, bname);

                for (EthernetAddress ea : createEthernetAddresses(false)) {
                    long mac = NetUtils.byteArray6ToLong(ea.getValue());

                 // create object by MacTableEntryId(VBridgePath, long).
                    MacTableEntryId cev1 = new MacTableEntryId(bpath, mac);
                    MacTableEntryId cev2 = new MacTableEntryId(
                                            cev1.getControllerAddress(),
                                            cev1.getEventId(), bpath, mac);
                    testEquals(cevSet, cev1, cev2);
                    numSet++;

                    // create object by
                    // MacTableEntryId(InetAddress, long, VBridgePath, long).
                    for (long id : values) {
                        for (Set<InetAddress> iset : createInetAddresses(false)) {
                            for (InetAddress ipaddr : iset) {
                                if (!ipSet.add(ipaddr)) {
                                    continue;
                                }

                                cev1 = new MacTableEntryId(ipaddr, id, bpath,
                                                           mac);
                                cev2 = new MacTableEntryId(ipaddr, id, bpath,
                                                           mac);

                                testEquals(cevSet, cev1, cev2);
                                numSet++;
                            }
                        }
                    }
                }
            }
        }

        assertEquals(numSet, cevSet.size());
    }

    /**
     * Test method for {@link FlowGroupId#toString()}.
     */
    @Test
    public void testToString() {
        long[] values = new long[] {0, Long.MIN_VALUE, Long.MAX_VALUE};
        InetAddress loopback = InetAddress.getLoopbackAddress();

        for (String tname : createStrings("tenant")) {
            for (String bname : createStrings("vbr")) {
                VBridgePath bpath = new VBridgePath(tname, bname);

                for (EthernetAddress ea : createEthernetAddresses(false)) {
                    long mac = NetUtils.byteArray6ToLong(ea.getValue());
                    String macStr = HexEncode.bytesToHexStringFormat(ea.getValue());
                    ClusterEventId.setLocalAddress(null);

                    // create object by MacTableEntryId(VBridgePath, long).
                    MacTableEntryId cev = new MacTableEntryId(bpath, mac);

                    String required = bpath.toString() + "-" + macStr + "-"
                            + loopback.getHostAddress() + "-" + cev.getEventId();
                    assertEquals(required, cev.toString());

                    // create object by
                    // MacTableEntryId(InetAddress, long, VBridgePath, long).
                    for (long id : values) {
                        for (Set<InetAddress> iset : createInetAddresses(false)) {
                            for (InetAddress ipaddr : iset) {
                                cev = new MacTableEntryId(ipaddr, id, bpath,
                                                          mac);
                                required = bpath.toString() + "-" + macStr + "-"
                                        + ipaddr.getHostAddress() + "-"
                                        + cev.getEventId();
                                assertEquals(required, cev.toString());
                            }
                        }
                    }
                }
            }
        }
    }

    /**
     * Ensure that {@link FlowGroupId} is serializable.
     */
    @Test
    public void testSerialize() {
        long[] values = new long[] {0, Long.MIN_VALUE, Long.MAX_VALUE};

        for (String tname : createStrings("tenant")) {
            for (String bname : createStrings("vbr")) {
                VBridgePath bpath = new VBridgePath(tname, bname);

                for (EthernetAddress ea : createEthernetAddresses(false)) {
                    long mac = NetUtils.byteArray6ToLong(ea.getValue());

                    // create object by MacTableEntryId(VBridgePath, long).
                    MacTableEntryId cev = new MacTableEntryId(bpath, mac);
                    serializeTest(cev);

                    // create object by
                    // MacTableEntryId(InetAddress, long, VBridgePath, long).
                    for (long id : values) {
                        for (Set<InetAddress> iset : createInetAddresses(false)) {
                            for (InetAddress ipaddr : iset) {
                                cev = new MacTableEntryId(ipaddr, id, bpath,
                                                          mac);
                                serializeTest(cev);
                            }
                        }
                    }
                }
            }
        }
    }
}
