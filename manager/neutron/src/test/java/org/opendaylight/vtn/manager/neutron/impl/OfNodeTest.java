/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.neutron.impl;

import java.math.BigInteger;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Locale;
import java.util.Map.Entry;
import java.util.Map;
import java.util.Set;

import org.junit.Test;

import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.NodeId;

import org.opendaylight.yang.gen.v1.urn.opendaylight.params.xml.ns.yang.ovsdb.rev150105.DatapathId;

/**
 * JUnit test for {@link OfNode}.
 */
public class OfNodeTest extends TestBase {
    /**
     * Test case for {@link OfNode#getNodeNumber()}.
     */
    @Test
    public void testGetNodeNumber() {
        Map<String, BigInteger> cases = new HashMap<>();
        cases.put("00:00:00:00:00:00:00:00", BigInteger.ZERO);
        cases.put("00:00:00:00:00:00:00:02", BigInteger.valueOf(2L));
        cases.put("00:00:00:11:22:33:aa:bb", new BigInteger("112233aabb", 16));
        cases.put("7f:ab:cd:ef:11:01:35:99",
                  new BigInteger("7fabcdef11013599", 16));
        cases.put("7f:ff:ff:ff:ff:ff:ff:ff",
                  new BigInteger("7fffffffffffffff", 16));
        cases.put("80:00:00:00:00:00:00:00",
                  new BigInteger("8000000000000000", 16));
        cases.put("80:00:00:00:00:00:00:01",
                  new BigInteger("8000000000000001", 16));
        cases.put("ff:11:22:33:ab:cd:ef:98",
                  new BigInteger("ff112233abcdef98", 16));
        cases.put("ff:ff:ff:ff:ff:ff:ff:ff",
                  new BigInteger("ffffffffffffffff", 16));

        for (Entry<String, BigInteger> entry: cases.entrySet()) {
            DatapathId dpid = new DatapathId(entry.getKey());
            OfNode on = new OfNode(dpid);
            assertEquals(entry.getValue(), on.getNodeNumber());
        }
    }

    /**
     * Test case for {@link OfNode#getNodeId()}.
     */
    @Test
    public void testGetNodeId() {
        Map<String, String> cases = new HashMap<>();
        cases.put("00:00:00:00:00:00:00:00", "openflow:0");
        cases.put("00:00:00:00:00:00:00:02", "openflow:2");
        cases.put("00:00:00:11:22:33:aa:bb", "openflow:73588255419");
        cases.put("7f:ff:ff:ff:ff:ff:ff:ff", "openflow:9223372036854775807");
        cases.put("80:00:00:00:00:00:00:00", "openflow:9223372036854775808");
        cases.put("80:00:00:00:00:00:00:01", "openflow:9223372036854775809");
        cases.put("ff:11:22:33:ab:cd:ef:98", "openflow:18379509159596781464");
        cases.put("ff:ff:ff:ff:ff:ff:ff:ff", "openflow:18446744073709551615");

        for (Entry<String, String> entry: cases.entrySet()) {
            DatapathId dpid = new DatapathId(entry.getKey());
            OfNode on = new OfNode(dpid);
            NodeId expected = new NodeId(entry.getValue());
            assertEquals(expected, on.getNodeId());
        }
    }

    /**
     * Test case for {@link OfNode#equals(Object)} and
     * {@link OfNode#hashCode()}.
     */
    @Test
    public void testEquals() {
        Set<Object> set = new HashSet<>();
        DatapathId[] dpids = {
            new DatapathId("00:00:00:00:00:00:00:00"),
            new DatapathId("00:00:00:00:00:00:00:02"),
            new DatapathId("00:00:00:11:22:33:aa:bb"),
            new DatapathId("7f:ff:ff:ff:ff:ff:ff:ff"),
            new DatapathId("80:00:00:00:00:00:00:00"),
            new DatapathId("80:00:00:00:00:00:00:01"),
            new DatapathId("ff:11:22:33:ab:cd:ef:98"),
            new DatapathId("ff:ff:ff:ff:ff:ff:ff:ff"),
        };

        for (DatapathId dpid: dpids) {
            OfNode on1 = new OfNode(dpid);
            String upper = dpid.getValue().toUpperCase(Locale.ENGLISH);
            OfNode on2 = new OfNode(new DatapathId(upper));
            testEquals(set, on1, on2);
        }

        assertEquals(dpids.length, set.size());

        for (DatapathId dpid: dpids) {
            OfNode on = new OfNode(dpid);
            assertEquals(true, set.remove(on));
        }
        assertEquals(true, set.isEmpty());
    }

    /**
     * Test case for {@link OfNode#toString()}.
     */
    @Test
    public void testToString() {
        Map<String, String> cases = new HashMap<>();
        cases.put("00:00:00:00:00:00:00:00", "openflow:0");
        cases.put("00:00:00:00:00:00:00:02", "openflow:2");
        cases.put("00:00:00:11:22:33:aa:bb", "openflow:73588255419");
        cases.put("7f:ff:ff:ff:ff:ff:ff:ff", "openflow:9223372036854775807");
        cases.put("80:00:00:00:00:00:00:00", "openflow:9223372036854775808");
        cases.put("80:00:00:00:00:00:00:01", "openflow:9223372036854775809");
        cases.put("ff:11:22:33:ab:cd:ef:98", "openflow:18379509159596781464");
        cases.put("ff:ff:ff:ff:ff:ff:ff:ff", "openflow:18446744073709551615");

        for (Entry<String, String> entry: cases.entrySet()) {
            DatapathId dpid = new DatapathId(entry.getKey());
            OfNode on = new OfNode(dpid);
            String str = on.toString();
            assertEquals(entry.getValue(), str);

            // Result should be cached.
            for (int i = 0; i < 10; i++) {
                assertSame(str, on.toString());
            }
        }
    }
}
