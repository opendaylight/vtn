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

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnPortDesc;

/**
 * JUnit test for {@link VtnPortDescAdapter}.
 */
public class VtnPortDescAdapterTest extends TestBase {
    /**
     * Test case for {@link VtnPortDescAdapter#marshal(VtnPortDesc)}.
     */
    @Test
    public void testMarshal() {
        VtnPortDescAdapter adapter = new VtnPortDescAdapter();
        assertEquals(null, adapter.marshal((VtnPortDesc)null));

        String[] descs = {
            "openflow:1,2,eth2",
            "openflow:1,2,",
            "openflow:1,,",
            "openflow:9999:3,,eth3",
            "unknown:1,abc,port-A",
            "unknown:1,abc,",
            "unknown:1,,port-A",
            "unknown:1,,",
        };

        for (String desc: descs) {
            VtnPortDesc vpdesc = new VtnPortDesc(desc);
            assertEquals(desc, adapter.marshal(vpdesc));
        }
    }

    /**
     * Test case for {@link VtnPortDescAdapter#unmarshal(String)}.
     */
    @Test
    public void testUnmarshal() {
        VtnPortDescAdapter adapter = new VtnPortDescAdapter();
        assertEquals(null, adapter.unmarshal((String)null));

        String[] descs = {
            "openflow:1,2,eth2",
            "openflow:1,2,",
            "openflow:1,,",
            "openflow:9999:3,,eth3",
            "unknown:1,abc,port-A",
            "unknown:1,abc,",
            "unknown:1,,port-A",
            "unknown:1,,",
        };

        for (String desc: descs) {
            VtnPortDesc vpdesc = new VtnPortDesc(desc);
            assertEquals(vpdesc, adapter.unmarshal(desc));
        }

        String[] invalid = {
            // Invalid format.
            "",
            "bad port desc",
            "openflow:1:1,",
            "openflow:1:2",
            ",1,eth1",
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
