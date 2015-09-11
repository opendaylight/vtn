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

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeName;

/**
 * JUnit test for {@link VnodeNameAdapter}.
 */
public class VnodeNameAdapterTest extends TestBase {
    /**
     * Test case for {@link VnodeNameAdapter#marshal(VnodeName)}.
     */
    @Test
    public void testMarshal() {
        VnodeNameAdapter adapter = new VnodeNameAdapter();
        assertEquals(null, adapter.marshal((VnodeName)null));

        String[] names = {
            "a",
            "AB",
            "abcde",
            "0",
            "01",
            "012",
            "0123456789",
            "a123456789B123456789c123456789D",
            "vtn_1",
            "vbridge_2",
            "vif_3",
        };

        for (String name: names) {
            VnodeName vname = new VnodeName(name);
            assertEquals(name, adapter.marshal(vname));
        }
    }

    /**
     * Test case for {@link VnodeNameAdapter#unmarshal(String)}.
     */
    @Test
    public void testUnmarshal() {
        VnodeNameAdapter adapter = new VnodeNameAdapter();
        assertEquals(null, adapter.unmarshal((String)null));

        String[] names = {
            "a",
            "AB",
            "abcde",
            "0",
            "01",
            "0123456789",
            "a123456789B123456789c123456789D",
            "vtn_1",
            "vbridge_2",
            "vif_3",
        };

        for (String name: names) {
            VnodeName vname = new VnodeName(name);
            assertEquals(vname, adapter.unmarshal(name));
        }

        String[] invalid = {
            // Invalid format.
            "",
            "bad name",
            "vtn-1",
            "@vtn",
            "_vtn",
            "/vbridge",
            "a123456789B123456789c123456789D1",
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
