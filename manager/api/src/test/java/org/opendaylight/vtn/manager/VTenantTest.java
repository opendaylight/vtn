/*
 * Copyright (c) 2013, 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager;

import java.util.HashSet;
import java.util.List;

import javax.xml.bind.Unmarshaller;

import org.junit.Test;

/**
 * JUnit test for {@link VTenant}.
 */
public class VTenantTest extends TestBase {
    /**
     * Root XML element name associated with {@link VTenant} class.
     */
    private static final String  XML_ROOT = "vtn";

    /**
     * Test case for getter methods.
     */
    @Test
    public void testGetter() {
        for (String name: createStrings("name")) {
            for (String desc: createStrings("desc")) {
                for (Integer iv: createIntegers(-2, 5)) {
                    for (Integer hv: createIntegers(-2, 5)) {
                        String emsg = "(name)" + name + "(desc)" + desc +
                            ",(iv)" + ((iv == null) ? "null" : iv.intValue()) +
                            ",(hv)" + ((hv == null) ? "null" : hv.intValue());

                        VTenantConfig tconf =
                            createVTenantConfig(desc, iv, hv);
                        VTenant vtenant = new VTenant(name, tconf);
                        assertEquals(emsg, name, vtenant.getName());
                        assertEquals(emsg, desc, vtenant.getDescription());
                        if (iv == null || iv.intValue() < 0) {
                            assertEquals(emsg, -1, vtenant.getIdleTimeout());
                            assertNull(emsg, vtenant.getIdleTimeoutValue());
                        } else {
                            assertEquals(emsg, iv.intValue(),
                                         vtenant.getIdleTimeout());
                            assertEquals(emsg, iv, vtenant.getIdleTimeoutValue());
                        }
                        if (hv == null || hv.intValue() < 0) {
                            assertEquals(emsg, -1, vtenant.getHardTimeout());
                            assertNull(emsg, vtenant.getHardTimeoutValue());
                        } else {
                            assertEquals(emsg, hv.intValue(),
                                         vtenant.getHardTimeout());
                            assertEquals(emsg, hv, vtenant.getHardTimeoutValue());
                        }
                    }
                }
            }
        }
    }

    /**
     * Test case for {@link VTenant#equals(Object)} and
     * {@link VTenant#hashCode()}.
     */
    @Test
    public void testEquals() {
        HashSet<Object> set = new HashSet<Object>();
        List<String> names = createStrings("name");
        List<String> descs = createStrings("desc");
        List<Integer> idles = createIntegers(0, 3);
        List<Integer> hards = createIntegers(0, 3);
        for (String name: names) {
            for (String desc: descs) {
                for (Integer iv: idles) {
                    for (Integer hv: hards) {
                        VTenantConfig tconf =
                            createVTenantConfig(desc, iv, hv);
                        VTenant t1 = new VTenant(name, tconf);
                        tconf = createVTenantConfig(copy(desc), iv, hv);
                        VTenant t2 = new VTenant(copy(name), tconf);
                        testEquals(set, t1, t2);
                    }
                }
            }
        }

        int required = names.size() * descs.size() * idles.size() *
            hards.size();
        assertEquals(required, set.size());
    }

    /**
     * Test case for {@link VTenant#toString()}.
     */
    @Test
    public void testToString() {
        String prefix = "VTenant[";
        String suffix = "]";
        for (String name: createStrings("name")) {
            for (String desc: createStrings("desc")) {
                for (Integer iv: createIntegers(0, 3)) {
                    for (Integer hv: createIntegers(0, 3)) {
                        VTenantConfig tconf =
                            createVTenantConfig(desc, iv, hv);
                        VTenant vtenant = new VTenant(name, tconf);
                        if (name != null) {
                            name = "name=" + name;
                        }
                        if (desc != null) {
                            desc = "desc=" + desc;
                        }
                        String is = (iv == null) ? null : "idleTimeout=" + iv;
                        String hs = (hv == null) ? null : "hardTimeout=" + hv;
                        String required =
                            joinStrings(prefix, suffix, ",", name, desc,
                                        is, hs);
                        assertEquals(required, vtenant.toString());
                    }
                }
            }
        }
    }

    /**
     * Ensure that {@link VTenant} is serializable.
     */
    @Test
    public void testSerialize() {
        for (String name: createStrings("name")) {
            for (String desc: createStrings("desc")) {
                for (Integer iv: createIntegers(0, 3)) {
                    for (Integer hv: createIntegers(0, 3)) {
                        VTenantConfig tconf =
                            createVTenantConfig(desc, iv, hv);
                        VTenant vtenant = new VTenant(name, tconf);
                        serializeTest(vtenant);
                    }
                }
            }
        }
    }

    /**
     * Ensure that {@link VTenant} is mapped to XML root element.
     */
    @Test
    public void testJAXB() {
        for (String name: createStrings("name")) {
            for (String desc: createStrings("desc")) {
                for (Integer iv: createIntegers(10, 3, false)) {
                    for (Integer hv: createIntegers(10, 3, false)) {
                        VTenantConfig tconf =
                            createVTenantConfig(desc, iv, hv);
                        VTenant vtenant = new VTenant(name, tconf);
                        jaxbTest(vtenant, VTenant.class, XML_ROOT);
                    }
                }

                // Ensure that negative timeouts are ignored.
                for (Integer iv: createIntegers(-10, 3)) {
                    for (Integer hv: createIntegers(-20, 3)) {
                        jaxbTimeoutTest(name, desc, iv, hv);
                    }
                }
            }
        }

        // Ensure that broken values in XML can be detected.
        jaxbErrorTest(VTenant.class,
                      VTenantConfigTest.getXmlDataTypes(XML_ROOT));
    }

    /**
     * JAXB test for "idleTimeout" and "hardTimeout" attributes.
     *
     * @param name      The name of the tenant.
     * @param desc      Description about the tenant.
     * @param idle      {@code idle_timeout} value for flow entries.
     * @param hard      {@code hard_timeout} value for flow entries.
     */
    private void jaxbTimeoutTest(String name, String desc, Integer idle,
                                 Integer hard) {
        StringBuilder builder = new StringBuilder(XML_DECLARATION);
        builder.append('<').append(XML_ROOT);
        if (name != null) {
            builder.append(" name=\"").append(name).append('"');
        }
        if (desc != null) {
            builder.append(" description=\"").append(desc).append('"');
        }
        if (idle != null) {
            builder.append(" idleTimeout=\"").append(idle.toString()).
                append('"');
        }
        if (hard != null) {
            builder.append(" hardTimeout=\"").append(hard.toString()).
                append('"');
        }


        String xml = builder.append("/>").toString();
        Unmarshaller um = createUnmarshaller(VTenant.class);
        VTenant vtenant = null;
        try {
            vtenant = unmarshal(um, xml, VTenant.class);
        } catch (Exception e) {
            unexpected(e);
        }

        assertEquals(name, vtenant.getName());
        assertEquals(desc, vtenant.getDescription());
        if (idle == null || idle.intValue() < 0) {
            assertEquals(-1, vtenant.getIdleTimeout());
            assertNull(vtenant.getIdleTimeoutValue());
        } else {
            assertEquals(idle.intValue(), vtenant.getIdleTimeout());
            assertEquals(idle, vtenant.getIdleTimeoutValue());
        }
        if (hard == null || hard.intValue() < 0) {
            assertEquals(-1, vtenant.getHardTimeout());
            assertNull(vtenant.getHardTimeoutValue());
        } else {
            assertEquals(hard.intValue(), vtenant.getHardTimeout());
            assertEquals(hard, vtenant.getHardTimeoutValue());
        }
    }
}
