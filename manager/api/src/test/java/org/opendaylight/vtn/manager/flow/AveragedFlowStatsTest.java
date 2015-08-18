/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
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
 * JUnit test for {@link AveragedFlowStats}.
 */
public class AveragedFlowStatsTest extends TestBase {
    /**
     * Root XML element name associated with {@link FlowStats} class.
     */
    private static final String  XML_ROOT = "averagedflowstats";

    /**
     * Test case for getter methods.
     */
    @Test
    public void testGetter() {
        double[] packets = {0D, 123D, 1234.5678D};
        double[] bytes = {0D, 3333D, 34567.8901D};
        long[] starts = {0L, 123456L, 1234567890L};
        long[] ends = {0L, 3333333L, 9999999999999L};

        final double delta = 0.000001d;
        for (double p: packets) {
            for (double b: bytes) {
                for (long s: starts) {
                    for (long e: ends) {
                        AveragedFlowStats stats =
                            new AveragedFlowStats(p, b, s, e);
                        assertEquals(p, stats.getPacketCount(), delta);
                        assertEquals(b, stats.getByteCount(), delta);
                        assertEquals(s, stats.getStartTime());
                        assertEquals(e, stats.getEndTime());
                    }
                }
            }
        }
    }

    /**
     * Test case for {@link AveragedFlowStats#equals(Object)} and
     * {@link AveragedFlowStats#hashCode()}.
     */
    @Test
    public void testEquals() {
        HashSet<Object> set = new HashSet<Object>();
        double[] packets = {0D, 123D, 1234.5678D};
        double[] bytes = {0D, 3333D, 34567.8901D};
        long[] starts = {0L, 123456L, 1234567890L};
        long[] ends = {0L, 3333333L, 9999999999999L};

        for (double p: packets) {
            for (double b: bytes) {
                for (long s: starts) {
                    for (long e: ends) {
                        AveragedFlowStats s1 =
                            new AveragedFlowStats(p, b, s, e);
                        AveragedFlowStats s2 =
                            new AveragedFlowStats(p, b, s, e);
                        testEquals(set, s1, s2);
                    }
                }
            }
        }

        int expected = packets.length * bytes.length * starts.length *
            ends.length;
        assertEquals(expected, set.size());
    }

    /**
     * Test case for {@link AveragedFlowStats#toString()}.
     */
    @Test
    public void testToString() {
        double[] packets = {0D, 123D, 1234.5678D};
        double[] bytes = {0D, 3333D, 34567.8901D};
        long[] starts = {0L, 123456L, 1234567890L};
        long[] ends = {0L, 3333333L, 9999999999999L};

        for (double p: packets) {
            for (double b: bytes) {
                for (long s: starts) {
                    for (long e: ends) {
                        AveragedFlowStats stats =
                            new AveragedFlowStats(p, b, s, e);
                        String required = "AveragedFlowStats[packets=" + p +
                            ",bytes=" + b + ",start=" + s + ",end=" + e + "]";
                        assertEquals(required, stats.toString());
                    }
                }
            }
        }
    }

    /**
     * Ensure that {@link AveragedFlowStats} is serializable.
     */
    @Test
    public void testSerialize() {
        double[] packets = {0D, 123D, 1234.5678D};
        double[] bytes = {0D, 3333D, 34567.8901D};
        long[] starts = {0L, 123456L, 1234567890L};
        long[] ends = {0L, 3333333L, 9999999999999L};

        for (double p: packets) {
            for (double b: bytes) {
                for (long s: starts) {
                    for (long e: ends) {
                        AveragedFlowStats stats =
                            new AveragedFlowStats(p, b, s, e);
                        serializeTest(stats);
                    }
                }
            }
        }
    }

    /**
     * Ensure that {@link AveragedFlowStats} is mapped to XML root element.
     */
    @Test
    public void testJAXB() {
        double[] packets = {0D, 123D, 1234.5678D};
        double[] bytes = {0D, 3333D, 34567.8901D};
        long[] starts = {0L, 123456L, 1234567890L};
        long[] ends = {0L, 3333333L, 9999999999999L};

        for (double p: packets) {
            for (double b: bytes) {
                for (long s: starts) {
                    for (long e: ends) {
                        AveragedFlowStats stats =
                            new AveragedFlowStats(p, b, s, e);
                        jaxbTest(stats, AveragedFlowStats.class, XML_ROOT);
                    }
                }
            }
        }

        // Ensure that broken values in XML can be detected.
        jaxbErrorTest(AveragedFlowStats.class,
                      new XmlAttributeType(XML_ROOT, "packets", double.class),
                      new XmlAttributeType(XML_ROOT, "bytes", double.class),
                      new XmlAttributeType(XML_ROOT, "start", long.class),
                      new XmlAttributeType(XML_ROOT, "end", long.class));
    }

    /**
     * Ensure that {@link AveragedFlowStats} is mapped to JSON object.
     */
    @Test
    public void testJSON() {
        double[] packets = {0D, 123D, 1234.5678D};
        double[] bytes = {0D, 3333D, 34567.8901D};
        long[] starts = {0L, 123456L, 1234567890L};
        long[] ends = {0L, 3333333L, 9999999999999L};

        for (double p: packets) {
            for (double b: bytes) {
                for (long s: starts) {
                    for (long e: ends) {
                        AveragedFlowStats stats =
                            new AveragedFlowStats(p, b, s, e);
                        jsonTest(stats, AveragedFlowStats.class);
                    }
                }
            }
        }
    }
}
