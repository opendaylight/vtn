/*
 * Copyright (c) 2013, 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.cluster;

import java.net.InetAddress;
import java.util.HashSet;
import java.util.Set;

import org.junit.Test;
import org.opendaylight.vtn.manager.internal.TestBase;

/**
 * JUnit test for {@link ClusterEventId}
 */
public class ClusterEventIdTest extends TestBase {

    /**
     * Test case for getter methods.
     */
    @Test
    public void testGetter() {
        ClusterEventId.setLocalAddress(null);

        // create object by ClusterEvendId().
        InetAddress loopback = InetAddress.getLoopbackAddress();

        ClusterEventId cev = new ClusterEventId();

        long eventid = cev.getEventId();
        assertEquals(loopback, cev.getControllerAddress());

        cev = new ClusterEventId();
        assertEquals(eventid + 1, cev.getEventId());
        assertEquals(loopback, cev.getControllerAddress());

        assertTrue(cev.isLocal());

        // create object by ClusterEvendId(long, InetAddress).
        long[] values = new long[] {0, Long.MIN_VALUE, Long.MAX_VALUE};
        for (long id : values) {
            for (Set<InetAddress> iset : createInetAddresses(false)) {
                for (InetAddress ipaddr : iset) {
                    String emsg = "(id)" + id
                                + ",(InetAddress)" + ipaddr.toString();
                    cev = new ClusterEventId(ipaddr, id);
                    assertEquals(emsg, id, cev.getEventId());
                    assertEquals(emsg, ipaddr, cev.getControllerAddress());

                    if (ipaddr != null) {
                        ClusterEventId.setLocalAddress(ipaddr);
                        assertTrue(emsg, cev.isLocal());
                    }
                }
            }
        }
    }

    /**
     * Test method for
     * {@link ClusterEventId#hashCode()},
     * {@link ClusterEventId#equals(java.lang.Object)}.
     */
    @Test
    public void testEquals() {
        ClusterEventId.setLocalAddress(null);

        // create object by ClusterEvendId().
        Set<Object> cevSet = new HashSet<Object>();
        Set<InetAddress> ipSet = new HashSet<InetAddress>();

        ClusterEventId cev1 = new ClusterEventId();
        ClusterEventId cev2 = new ClusterEventId(cev1.getControllerAddress(),
                                                 cev1.getEventId());
        testEquals(cevSet, cev1, cev2);
        assertEquals(1, cevSet.size());

        // create object by ClusterEvendId(long, InetAddress).
        long[] values = new long[] {-1L, Long.MIN_VALUE, Long.MAX_VALUE};
        int numSet = 1;
        for (long id : values) {
            for (Set<InetAddress> iset : createInetAddresses(false)) {
                for (InetAddress ipaddr : iset) {
                    if (!ipSet.add(ipaddr)) {
                        continue;
                    }
                    cev1 = new ClusterEventId(ipaddr, id);
                    cev2 = new ClusterEventId(ipaddr, id);

                    testEquals(cevSet, cev1, cev2);
                    numSet++;
                }
            }
            ipSet.clear();
        }

        assertEquals(numSet, cevSet.size());
    }

    /**
     * Test method for {@link ClusterEventId#toString()}.
     */
    @Test
    public void testToString() {
        ClusterEventId.setLocalAddress(null);

        // create object by ClusterEvendId().
        InetAddress loopback = InetAddress.getLoopbackAddress();
        ClusterEventId cev = new ClusterEventId();

        String required = loopback.getHostAddress() + "-" + cev.getEventId();
        assertEquals(required, cev.toString());

        // create object by ClusterEvendId(long, InetAddress).
        long[] values = new long[] {0, Long.MIN_VALUE, Long.MAX_VALUE};
        for (long id : values) {
            for (Set<InetAddress> iset : createInetAddresses(false)) {
                for (InetAddress ipaddr : iset) {
                    cev = new ClusterEventId(ipaddr, id);
                    required = ipaddr.getHostAddress() + "-"
                            + cev.getEventId();
                    assertEquals(required, cev.toString());
                }
            }
        }
    }

    /**
     * Ensure that {@link ClusterEventId} is serializable.
     */
    @Test
    public void testSerialize() {
        ClusterEventId.setLocalAddress(null);

        // create object by ClusterEvendId().
        ClusterEventId cev = new ClusterEventId();
        serializeTest(cev);

        // create object by ClusterEvendId(long, InetAddress).
        long[] values = new long[] {0, Long.MIN_VALUE, Long.MAX_VALUE};
        for (long id : values) {
            for (Set<InetAddress> iset : createInetAddresses(false)) {
                for (InetAddress ipaddr : iset) {
                    cev = new ClusterEventId(ipaddr, id);
                    serializeTest(cev);
                }
            }
        }
    }
}
