/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.inventory;

import java.util.HashMap;
import java.util.Map;

import org.junit.Test;

import org.opendaylight.vtn.manager.internal.TestBase;

/**
 * JUnit test for {@link SalNodeAdapter}.
 */
public class SalNodeAdapterTest extends TestBase {
    /**
     * Test case for {@link SalNodeAdapter#marshal(SalNode)}.
     */
    @Test
    public void testMarshal() {
        SalNodeAdapter adapter = new SalNodeAdapter();
        assertEquals(null, adapter.marshal((SalNode)null));

        SalNode[] values = {
            new SalNode(1L),
            new SalNode(2L),
            new SalNode(1000L),
            new SalNode(-1L),
        };

        for (SalNode snode: values) {
            assertEquals(snode.toString(), adapter.marshal(snode));
        }
    }

    /**
     * Test case for {@link SalNodeAdapter#unmarshal(String)}.
     */
    @Test
    public void testUnmarshal() {
        SalNodeAdapter adapter = new SalNodeAdapter();
        Map<String, SalNode> cases = new HashMap<>();
        cases.put(null, null);
        cases.put("openflow:1", new SalNode(1L));
        cases.put("openflow:2", new SalNode(2L));
        cases.put("openflow:1234567890123", new SalNode(1234567890123L));
        cases.put("openflow:18446744073709551615", new SalNode(-1L));
        for (Map.Entry<String, SalNode> entry: cases.entrySet()) {
            assertEquals(entry.getValue(), adapter.unmarshal(entry.getKey()));
        }

        String[] badValues = {
            "",
            "a",
            "123",
            "openflow",
            "openflow:",
            "openflow:abcde",
            "unknown:1",
        };

        for (String bad: badValues) {
            try {
                adapter.unmarshal(bad);
                unexpected();
            } catch (IllegalArgumentException e) {
                String msg = "Unsupported SAL node identifier: " + bad;
                assertEquals(msg, e.getMessage());
            }
        }
    }
}
