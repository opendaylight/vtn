/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.util.xml.adapters;

import org.junit.Test;

import org.opendaylight.vtn.manager.TestBase;

import org.opendaylight.yang.gen.v1.urn.opendaylight.l2.types.rev130827.VlanId;

/**
 * JUnit test for {@link VlanIdAdapter}.
 */
public class VlanIdAdapterTest extends TestBase {
    /**
     * Test case for {@link VlanIdAdapter#marshal(VlanId)}.
     */
    @Test
    public void testMarshal() {
        VlanIdAdapter adapter = new VlanIdAdapter();
        assertEquals(null, adapter.marshal((VlanId)null));

        Integer[] vids = {
            0, 1, 2, 30, 100, 1000, 2345, 3456, 4094, 4095,
        };

        for (Integer vid: vids) {
            VlanId vlanId = new VlanId(vid);
            assertEquals(vid.toString(), adapter.marshal(vlanId));
        }
    }

    /**
     * Test case for {@link VlanIdAdapter#unmarshal(String)}.
     */
    @Test
    public void testUnmarshal() {
        VlanIdAdapter adapter = new VlanIdAdapter();
        assertEquals(null, adapter.unmarshal((String)null));

        Integer[] vids = {
            0, 1, 2, 30, 100, 1000, 2345, 3456, 4094, 4095,
        };

        for (Integer vid: vids) {
            VlanId vlanId = new VlanId(vid);
            assertEquals(vlanId, adapter.unmarshal(vid.toString()));
        }

        String[] invalid = {
            // Invalid format.
            "",
            " ",
            "bad vlan ID",

            // Out of valid range.
            "-999999999999999999999999999",
            "9999999999999999999999999999",
            "-12345",
            "-1",
            "4096",
            "4097",
            "100000",
        };

        for (String str: invalid) {
            try {
                adapter.unmarshal(str);
                unexpected();
            } catch (IllegalArgumentException e) {
            }
        }
    }
}
