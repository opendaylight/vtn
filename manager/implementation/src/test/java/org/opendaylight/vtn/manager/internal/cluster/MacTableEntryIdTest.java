/*
 * Copyright (c) 2013, 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.cluster;

import java.net.InetAddress;
import java.net.Inet4Address;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

import org.junit.Test;

import org.opendaylight.vtn.manager.VBridgePath;
import org.opendaylight.vtn.manager.internal.util.MiscUtils;

import org.opendaylight.vtn.manager.internal.TestBase;

/**
 * JUnit Test for {@link MacTableEntryId}.
 */
public class MacTableEntryIdTest extends TestBase {
    /**
     * A list of {@link VBridgePath} instances for test.
     */
    private final List<VBridgePath>  bridgePaths;

    /**
     * Construct a new instance.
     */
    public MacTableEntryIdTest() {
        bridgePaths = createVBridgePaths(true);
    }

    /**
     * Test case for getter methods.
     */
    @Test
    public void testGetter() {
        long[] values = new long[] {0, Long.MIN_VALUE, Long.MAX_VALUE};
        long[] macAddrs = {
            0x000000000001L,
            0x004455667788L,
            0xf0abcdef1234L,
        };

        InetAddress loopback = InetAddress.getLoopbackAddress();
        ClusterEventId.setLocalAddress(null);

        MacTableEntryId cev = new MacTableEntryId(new VBridgePath("vtn", "vbr"),
                                                  0L);
        assertTrue(cev.isLocal());

        for (VBridgePath path: bridgePaths) {
            VBridgePath bpath = new VBridgePath(path.getTenantName(),
                                                path.getBridgeName());
            for (long mac: macAddrs) {
                ClusterEventId.setLocalAddress(null);
                String emsg = "(VBridgePath)" + path.toString()
                    + ",(MAC Address)" + MiscUtils.formatMacAddress(mac);

                // Create object by MacTableEntryId(VBridgePath, long).
                cev = new MacTableEntryId(path, mac);
                long eventid = cev.getEventId();
                assertEquals(emsg, loopback, cev.getControllerAddress());
                assertEquals(emsg, bpath, cev.getBridgePath());
                assertEquals(emsg, path, cev.getMapPath());
                assertEquals(emsg, mac, cev.getMacAddress());

                cev = new MacTableEntryId(path, mac);
                assertEquals(emsg, eventid + 1, cev.getEventId());
                assertEquals(emsg, bpath, cev.getBridgePath());
                assertEquals(emsg, path, cev.getMapPath());
                assertEquals(emsg, mac, cev.getMacAddress());

                // Create object by
                // MacTableEntryId(InetAddress, long, VBridgePath, long).
                for (long id : values) {
                    for (Set<InetAddress> iset : createInetAddresses(false)) {
                        for (InetAddress ipaddr : iset) {
                            String emsgl = emsg + ",(id)" + id
                                + ",(InetAddress)" + ipaddr.toString();
                            cev = new MacTableEntryId(ipaddr, id, path, mac);
                            assertEquals(emsgl, id, cev.getEventId());
                            assertEquals(emsgl,
                                         ipaddr, cev.getControllerAddress());
                            assertEquals(emsgl, bpath, cev.getBridgePath());
                            assertEquals(emsgl, path, cev.getMapPath());
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

    /**
     * Test method for
     * {@link MacTableEntryId#hashCode()},
     * {@link MacTableEntryId#equals(Object)}.
     */
    @Test
    public void testEquals() {
        long[] values = new long[] {-1L, Long.MIN_VALUE, Long.MAX_VALUE};
        long[] macAddrs = {
            0x000000000001L,
            0x0abcdef12345L,
        };

        Set<InetAddress> ipSet = new HashSet<InetAddress>();
        int ipv4 = 0;
        for (Set<InetAddress> iset: createInetAddresses(false)) {
            for (InetAddress iaddr: iset) {
                if (iaddr instanceof Inet4Address) {
                    if (ipv4 >= 2) {
                        continue;
                    }
                    ipv4++;
                }
                ipSet.add(iaddr);
            }
        }

        ClusterEventId.setLocalAddress(null);

        int expected = bridgePaths.size() *
            (macAddrs.length * (1 + values.length * ipSet.size()));
        Set<Object> cevSet = new HashSet<Object>(expected);
        for (VBridgePath path: bridgePaths) {
            for (long mac: macAddrs) {
                // Create object by MacTableEntryId(VBridgePath, long).
                MacTableEntryId cev1 = new MacTableEntryId(path, mac);
                MacTableEntryId cev2 =
                    new MacTableEntryId(cev1.getControllerAddress(),
                                        cev1.getEventId(), path, mac);
                testEquals(cevSet, cev1, cev2);

                // Create object by
                // MacTableEntryId(InetAddress, long, VBridgePath, long).
                for (long id: values) {
                    for (InetAddress ipaddr: ipSet) {
                        cev1 = new MacTableEntryId(ipaddr, id, path, mac);
                        cev2 = new MacTableEntryId(ipaddr, id, path, mac);
                        testEquals(cevSet, cev1, cev2);
                    }
                }
            }
        }

        assertEquals(expected, cevSet.size());
    }

    /**
     * Test method for {@link MacTableEntryId#toString()}.
     */
    @Test
    public void testToString() {
        long[] values = new long[] {0, Long.MIN_VALUE, Long.MAX_VALUE};
        long[] macAddrs = {
            0x000000000001L,
            0x000000112233L,
            0x004455667788L,
            0x0abcdef12345L,
        };
        InetAddress loopback = InetAddress.getLoopbackAddress();

        ClusterEventId.setLocalAddress(null);

        for (VBridgePath path: bridgePaths) {
            for (long mac: macAddrs) {
                String macStr = MiscUtils.formatMacAddress(mac);

                // Create object by MacTableEntryId(VBridgePath, long).
                MacTableEntryId cev = new MacTableEntryId(path, mac);

                String required = path.toString() + "-" + macStr + "-" +
                    loopback.getHostAddress() + "-" + cev.getEventId();
                assertEquals(required, cev.toString());

                // Create object by
                // MacTableEntryId(InetAddress, long, VBridgePath, long).
                for (long id: values) {
                    for (Set<InetAddress> iset: createInetAddresses(false)) {
                        for (InetAddress ipaddr: iset) {
                            cev = new MacTableEntryId(ipaddr, id, path, mac);
                            required = path.toString() + "-" + macStr + "-" +
                                ipaddr.getHostAddress() + "-" +
                                cev.getEventId();
                            assertEquals(required, cev.toString());
                        }
                    }
                }
            }
        }
    }

    /**
     * Ensure that {@link MacTableEntryId} is serializable.
     */
    @Test
    public void testSerialize() {
        long[] values = new long[] {0, Long.MIN_VALUE, Long.MAX_VALUE};
        long[] macAddrs = {
            0x000000000001L,
            0x000000112233L,
            0x004455667788L,
            0x0abcdef12345L,
        };

        for (VBridgePath path: bridgePaths) {
            for (long mac: macAddrs) {
                // Create object by MacTableEntryId(VBridgePath, long).
                MacTableEntryId cev = new MacTableEntryId(path, mac);
                serializeTest(cev);

                // Create object by
                // MacTableEntryId(InetAddress, long, VBridgePath, long).
                for (long id: values) {
                    for (Set<InetAddress> iset: createInetAddresses(false)) {
                        for (InetAddress ipaddr: iset) {
                            cev = new MacTableEntryId(ipaddr, id, path, mac);
                            serializeTest(cev);
                        }
                    }
                }
            }
        }
    }
}
