/*
 * Copyright (c) 2013-2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager;

import java.io.ByteArrayInputStream;
import java.util.HashSet;
import java.util.List;

import javax.xml.bind.JAXB;

import org.junit.Test;

/**
 * JUnit test for {@link VTenantConfig}.
 */
public class VTenantConfigTest extends TestBase {
    /**
     * Root XML element name associated with {@link VTenantConfig} class.
     */
    private static final String  XML_ROOT = "vtnconf";

    /**
     * Test case for getter methods.
     */
    @Test
    public void testGetter() {
        for (String desc: createStrings("description")) {
            for (Integer iv: createIntegers(-2, 5)) {
                for (Integer hv: createIntegers(-2, 5)) {
                    String emsg = "(desc)" + desc
                            + ",(iv)" + ((iv == null) ? "null" : iv.intValue())
                            + ",(hv)" + ((hv == null) ? "null" : hv.intValue());

                    VTenantConfig[] configs = {
                        createVTenantConfig(desc, iv, hv),
                        createVTenantConfigJAXB(desc, iv, hv),
                    };
                    for (VTenantConfig tconf: configs) {
                        assertEquals(emsg, desc, tconf.getDescription());
                        if (iv == null || iv.intValue() < 0) {
                            assertEquals(emsg, -1, tconf.getIdleTimeout());
                            assertNull(emsg, tconf.getIdleTimeoutValue());
                        } else {
                            assertEquals(emsg, iv.intValue(),
                                         tconf.getIdleTimeout());
                            assertEquals(emsg, iv, tconf.getIdleTimeoutValue());
                        }
                        if (hv == null || hv.intValue() < 0) {
                            assertEquals(emsg, -1, tconf.getHardTimeout());
                            assertNull(emsg, tconf.getHardTimeoutValue());
                        } else {
                            assertEquals(emsg, hv.intValue(),
                                         tconf.getHardTimeout());
                            assertEquals(emsg, hv,
                                         tconf.getHardTimeoutValue());
                        }
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
            for (Integer iv: createIntegers(-1, 5)) {
                for (Integer hv: createIntegers(-1, 5)) {
                    VTenantConfig tconf = createVTenantConfig(desc, iv, hv);
                    if (desc != null) {
                        desc = "desc=" + desc;
                    }
                    String is = (iv == null || iv < 0)
                            ? null : "idleTimeout=" + iv;
                    String hs = (hv == null || hv < 0)
                            ? null : "hardTimeout=" + hv;
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
            for (Integer iv: createIntegers(-1, 5)) {
                for (Integer hv: createIntegers(-1, 5)) {
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
            for (Integer iv: createIntegers(-1, 5)) {
                for (Integer hv: createIntegers(-1, 5)) {
                    VTenantConfig tconf = createVTenantConfig(desc, iv, hv);
                    jaxbTest(tconf, "vtnconf");
                }
            }
        }
    }

    /**
     * Construct a {@link VTenantConfig} instance using JAXB.
     *
     * @param desc  A brief description about the VTN.
     * @param idle  Idle timeout for flow entry.
     * @param hard  Hard timeout for flow entry.
     * @return  A {@link VBridgeConfig} instance.
     */
    private VTenantConfig createVTenantConfigJAXB(String desc, Integer idle,
                                                  Integer hard) {
        StringBuilder builder = new StringBuilder(XML_DECLARATION);
        builder.append('<').append(XML_ROOT);
        if (desc != null) {
            builder.append(" description=\"").append(desc).append("\"");
        }
        if (idle != null) {
            builder.append(" idleTimeout=\"").append(idle).append("\"");
        }
        if (hard != null) {
            builder.append(" hardTimeout=\"").append(hard).append("\"");
        }

        String xml = builder.append(" />").toString();
        VTenantConfig tconf = null;
        try {
            ByteArrayInputStream in = new ByteArrayInputStream(xml.getBytes());
            tconf = JAXB.unmarshal(in, VTenantConfig.class);
        } catch (Exception e) {
            unexpected(e);
        }

        return tconf;
    }
}
