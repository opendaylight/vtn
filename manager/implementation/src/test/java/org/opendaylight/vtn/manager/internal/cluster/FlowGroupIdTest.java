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
import org.opendaylight.vtn.manager.internal.TestBase;

/**
 * JUnit Test case for {@link FlowGroupId}.
 */
public class FlowGroupIdTest extends TestBase {

    /**
     * Test case for getter methods.
     */
    @Test
    public void testGetter() {
        ClusterEventId.setLocalAddress(null);
        InetAddress loopback = InetAddress.getLoopbackAddress();

        FlowGroupId cev = new FlowGroupId("vtn");
        assertTrue(cev.isLocal());
        assertEquals(loopback, cev.getControllerAddress());

        for (String tname : createStrings("tenant")) {
            ClusterEventId.setLocalAddress(null);

            // create object by FlowGroupId(String).
            cev = new FlowGroupId(tname);
            long eventid = cev.getEventId();
            assertEquals(loopback, cev.getControllerAddress());

            cev = new FlowGroupId(tname);
            assertEquals(eventid + 1, cev.getEventId());
            assertEquals(loopback, cev.getControllerAddress());
            assertEquals(tname, cev.getTenantName());

            // create object by FlowGroupId(InetAddress, long, String).
            long[] values = new long[] {0, Long.MIN_VALUE, Long.MAX_VALUE};
            for (long id : values) {
                for (Set<InetAddress> iset : createInetAddresses(false)) {
                    for (InetAddress ipaddr : iset) {
                        String emsg = "(id)" + id + ",(ipaddr)"
                                + ipaddr.toString();
                        cev = new FlowGroupId(ipaddr, id, tname);
                        assertEquals(emsg, id, cev.getEventId());
                        assertEquals(emsg, ipaddr, cev.getControllerAddress());
                        assertEquals(emsg, tname, cev.getTenantName());

                        if (ipaddr != null) {
                            ClusterEventId.setLocalAddress(ipaddr);
                            assertTrue(emsg, cev.isLocal());
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
        ClusterEventId.setLocalAddress(null);
        Set<Object> cevSet = new HashSet<Object>();
        Set<InetAddress> ipSet = new HashSet<InetAddress>();

        int numSet = 0;
        for (String tname : createStrings("tenant", false)) {
            // create object by FlowGroupId(String).
            FlowGroupId cev1 = new FlowGroupId(tname);
            FlowGroupId cev2 = new FlowGroupId(cev1.getControllerAddress(),
                                               cev1.getEventId(), tname);
            testEquals(cevSet, cev1, cev2);
            numSet++;

            // create object by FlowGroupId(InetAddress, long, String).
            long[] values = new long[] {0, Long.MIN_VALUE, Long.MAX_VALUE};
            for (long id : values) {
                for (Set<InetAddress> iset : createInetAddresses(false)) {
                    for (InetAddress ipaddr : iset) {
                        if (!ipSet.add(ipaddr)) {
                            continue;
                        }

                        cev1 = new FlowGroupId(ipaddr, id, tname);
                        cev2 = new FlowGroupId(ipaddr, id, tname);

                        testEquals(cevSet, cev1, cev2);
                        numSet++;
                    }
                }
            }
        }

        assertEquals(numSet, cevSet.size());
    }

    /*
     * Test method for {@link FlowGroupId#toString()}.
     * */
    @Test
    public void testToString() {
        ClusterEventId.setLocalAddress(null);
        InetAddress loopback = InetAddress.getLoopbackAddress();

        for (String tname : createStrings("tenant")) {
            // create object by FlowGroupId(String).
            FlowGroupId cev = new FlowGroupId(tname);

            String required = "vtn:" + tname + "-" + loopback.getHostAddress() + "-"
                              + cev.getEventId();
            assertEquals(required, cev.toString());

            // create object by FlowGroupId(InetAddress, long, String).
            long[] values = new long[] {0, Long.MIN_VALUE, Long.MAX_VALUE};
            for (long id : values) {
                for (Set<InetAddress> iset : createInetAddresses(false)) {
                    for (InetAddress ipaddr : iset) {
                        cev = new FlowGroupId(ipaddr, id, tname);
                        required = "vtn:" + tname + "-" + ipaddr.getHostAddress()
                                + "-" + cev.getEventId();
                        assertEquals(required, cev.toString());
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
        ClusterEventId.setLocalAddress(null);

        for (String tname : createStrings("tenant", false)) {
            // create object by FlowGroupId(String).
            FlowGroupId cev = new FlowGroupId(tname);
            serializeTest(cev);

            // create object by FlowGroupId(InetAddress, long, String).
            long[] values = new long[] {0, Long.MIN_VALUE, Long.MAX_VALUE};
            for (long id : values) {
                for (Set<InetAddress> iset : createInetAddresses(false)) {
                    for (InetAddress ipaddr : iset) {
                        cev = new FlowGroupId(ipaddr, id, tname);
                        serializeTest(cev);
                    }
                }
            }
        }
    }
}
