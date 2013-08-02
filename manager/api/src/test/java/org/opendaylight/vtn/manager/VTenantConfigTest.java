/*
 * Copyright (c) 2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager;

import java.util.HashSet;
import java.util.List;

import org.junit.Test;

/**
 * JUnit test for {@link VTenantConfig}.
 */
public class VTenantConfigTest extends TestBase {
    /**
     * Test case for getter methods.
     */
    @Test
    public void testGetter() {
        for (String desc: createStrings("description")) {
            for (Integer iv: createIntegers(-2, 5)) {
                for (Integer hv: createIntegers(-2, 5)) {
                    VTenantConfig tconf = createVTenantConfig(desc, iv, hv);
                    assertEquals(desc, tconf.getDescription());
                    if (iv == null || iv.intValue() < 0) {
                        assertEquals(-1, tconf.getIdleTimeout());
                        assertNull(tconf.getIdleTimeoutValue());
                    } else {
                        assertEquals(iv.intValue(), tconf.getIdleTimeout());
                        assertEquals(iv, tconf.getIdleTimeoutValue());
                    }
                    if (hv == null || hv.intValue() < 0) {
                        assertEquals(-1, tconf.getHardTimeout());
                        assertNull(tconf.getHardTimeoutValue());
                    } else {
                        assertEquals(hv.intValue(), tconf.getHardTimeout());
                        assertEquals(hv, tconf.getHardTimeoutValue());
                    }
                }
            }
        }
    }

    /**
     * Test case for {@link VTenantConfig#equals(Object)} and
     * {@link VTenantConfig#hashCode()}.
     */
    @Test
    public void testEquals() {
        HashSet<Object> set = new HashSet<Object>();
        List<String> descs = createStrings("description");
        List<Integer> idles = createIntegers(0, 5);
        List<Integer> hards = createIntegers(0, 5);
        for (String desc: descs) {
            for (Integer iv: idles) {
                for (Integer hv: hards) {
                    VTenantConfig tc1 = createVTenantConfig(desc, iv, hv);
                    VTenantConfig tc2 =
                        createVTenantConfig(copy(desc), iv, hv);
                    testEquals(set, tc1, tc2);
                }
            }
        }

        assertEquals(descs.size() * idles.size() * hards.size(), set.size());
    }

    /**
     * Test case for {@link VTenantConfig#toString()}.
     */
    @Test
    public void testToString() {
        String prefix = "VTenantConfig[";
        String suffix = "]";
        for (String desc: createStrings("description")) {
            for (Integer iv: createIntegers(0, 5)) {
                for (Integer hv: createIntegers(0, 5)) {
                    VTenantConfig tconf = createVTenantConfig(desc, iv, hv);
                    if (desc != null) {
                        desc = "desc=" + desc;
                    }
                    String is = (iv == null) ? null : "idleTimeout=" + iv;
                    String hs = (hv == null) ? null : "hardTimeout=" + hv;
                    String required =
                        joinStrings(prefix, suffix, ",", desc, is, hs);
                    assertEquals(required, tconf.toString());
                }
            }
        }
    }

    /**
     * Ensure that {@link VTenantConfig} is serializable.
     */
    @Test
    public void testSerialize() {
        for (String desc: createStrings("description")) {
            for (Integer iv: createIntegers(0, 5)) {
                for (Integer hv: createIntegers(0, 5)) {
                    VTenantConfig tconf = createVTenantConfig(desc, iv, hv);
                    serializeTest(tconf);
                }
            }
        }
    }

    /**
     * Ensure that {@link VTenantConfig} is mapped to XML root element.
     */
    @Test
    public void testJAXB() {
        for (String desc: createStrings("description")) {
            for (Integer iv: createIntegers(0, 5)) {
                for (Integer hv: createIntegers(0, 5)) {
                    VTenantConfig tconf = createVTenantConfig(desc, iv, hv);
                    jaxbTest(tconf, "vtnconf");
                }
            }
        }
    }
}
