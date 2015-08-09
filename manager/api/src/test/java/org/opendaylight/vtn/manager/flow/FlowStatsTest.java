/*
 * Copyright (c) 2014, 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.flow;

import java.util.HashSet;

import org.junit.Test;

import org.opendaylight.vtn.manager.TestBase;
import org.opendaylight.vtn.manager.XmlAttributeType;

/**
 * JUnit test for {@link FlowStats}.
 */
public class FlowStatsTest extends TestBase {
    /**
     * Root XML element name associated with {@link FlowStats} class.
     */
    private static final String  XML_ROOT = "flowstats";

    /**
     * Test case for getter methods.
     */
    @Test
    public void testGetter() {
        long[] packets = {0L, 1000L, 10000000000L};
        long[] bytes = {0L, 12345L, 123456789012L};
        long[] duration = {0L, 8888888L, 9999999999999L};
        for (long p: packets) {
            for (long b: bytes) {
                for (long d: duration) {
                    FlowStats fst = new FlowStats(p, b, d);
                    assertEquals(p, fst.getPacketCount());
                    assertEquals(b, fst.getByteCount());
                    assertEquals(d, fst.getDuration());
                }
            }
        }
    }

    /**
     * Test case for {@link FlowStats#equals(Object)} and
     * {@link FlowStats#hashCode()}.
     */
    @Test
    public void testEquals() {
        HashSet<Object> set = new HashSet<Object>();
        long[] packets = {0L, 1000L, 10000000000L};
        long[] bytes = {0L, 12345L, 123456789012L};
        long[] duration = {0L, 8888888L, 9999999999999L};
        for (long p: packets) {
            for (long b: bytes) {
                for (long d: duration) {
                    FlowStats fst1 = new FlowStats(p, b, d);
                    FlowStats fst2 = new FlowStats(p, b, d);
                    testEquals(set, fst1, fst2);
                }
            }
        }

        assertEquals(packets.length * bytes.length * duration.length,
                     set.size());
    }

    /**
     * Test case for {@link FlowStats#toString()}.
     */
    @Test
    public void testToString() {
        long[] packets = {0L, 1000L, 10000000000L};
        long[] bytes = {0L, 12345L, 123456789012L};
        long[] duration = {0L, 8888888L, 9999999999999L};
        for (long p: packets) {
            for (long b: bytes) {
                for (long d: duration) {
                    FlowStats fst = new FlowStats(p, b, d);
                    String required = "FlowStats[packets=" + p +
                        ",bytes=" + b + ",duration=" + d + "]";
                    assertEquals(required, fst.toString());
                }
            }
        }
    }

    /**
     * Ensure that {@link FlowStats} is serializable.
     */
    @Test
    public void testSerialize() {
        long[] packets = {0L, 1000L, 10000000000L};
        long[] bytes = {0L, 12345L, 123456789012L};
        long[] duration = {0L, 8888888L, 9999999999999L};
        for (long p: packets) {
            for (long b: bytes) {
                for (long d: duration) {
                    FlowStats fst = new FlowStats(p, b, d);
                    serializeTest(fst);
                }
            }
        }
    }

    /**
     * Ensure that {@link FlowStats} is mapped to XML root element.
     */
    @Test
    public void testJAXB() {
        long[] packets = {0L, 1000L, 10000000000L};
        long[] bytes = {0L, 12345L, 123456789012L};
        long[] duration = {0L, 8888888L, 9999999999999L};
        for (long p: packets) {
            for (long b: bytes) {
                for (long d: duration) {
                    FlowStats fst = new FlowStats(p, b, d);
                    jaxbTest(fst, FlowStats.class, XML_ROOT);
                }
            }
        }

        // Ensure that broken values in XML can be detected.
        jaxbErrorTest(FlowStats.class,
                      new XmlAttributeType(XML_ROOT, "packets", long.class),
                      new XmlAttributeType(XML_ROOT, "bytes", long.class),
                      new XmlAttributeType(XML_ROOT, "duration", long.class));
    }

    /**
     * Ensure that {@link FlowStats} is mapped to JSON object.
     */
    @Test
    public void testJSON() {
        long[] packets = {0L, 1000L, 10000000000L};
        long[] bytes = {0L, 12345L, 123456789012L};
        long[] duration = {0L, 8888888L, 9999999999999L};
        for (long p: packets) {
            for (long b: bytes) {
                for (long d: duration) {
                    FlowStats fst = new FlowStats(p, b, d);
                    jsonTest(fst, FlowStats.class);
                }
            }
        }
    }
}
