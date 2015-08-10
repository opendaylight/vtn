/*
 * Copyright (c) 2013, 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.northbound;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;

import org.junit.Test;
import org.opendaylight.vtn.manager.BundleVersion;

/**
 * JUnit test for {@link ManagerVersion}.
 */
public class ManagerVersionTest extends TestBase {
    /**
     * Root XML element name associated with {@link ManagerVersion} class.
     */
    private static final String  XML_ROOT = "version";

    /**
     * Test case for getter methods.
     */
    @Test
    public void testGetter() {
        for (int api = 0; api < 3; api++) {
            for (BundleVersion bv: createBundleVersions()) {
                ManagerVersion mv = new ManagerVersion(api, bv);
                assertEquals(api, mv.getApiVersion());
                if (mv == null) {
                    assertNull(mv.getBundleVersion());
                } else {
                    assertEquals(bv, mv.getBundleVersion());
                }
            }
        }
    }

    /**
     * test case for {@link ManagerVersion#equals(Object)} and
     * {@link ManagerVersion#hashCode()}
     */
    @Test
    public void testEquals() {
        int[] apis = {0, 1, 2, 10};
        List<BundleVersion> bundles = createBundleVersions();
        HashSet<Object> set = new HashSet<Object>();

        for (int api: apis) {
            for (BundleVersion bv: bundles) {
                ManagerVersion mv1 = new ManagerVersion(api, bv);
                ManagerVersion mv2 = new ManagerVersion(api, copy(bv));
                testEquals(set, mv1, mv2);
            }
        }

        assertEquals(apis.length * bundles.size(), set.size());
    }

    /**
     * Ensure that {@link ManagerVersion} is mapped to both XML root element
     * and JSON object.
     */
    @Test
    public void testJAXB() {
        for (int api = 0; api < 3; api++) {
            for (BundleVersion bv: createBundleVersions()) {
                ManagerVersion mv = new ManagerVersion(api, bv);
                jaxbTest(mv, ManagerVersion.class, XML_ROOT);
                jsonTest(mv, ManagerVersion.class);
            }
        }

        // Ensure that broken values in XML can be detected.
        jaxbErrorTest(ManagerVersion.class,
                      new XmlAttributeType(XML_ROOT, "api", int.class),

                      // BundleVersion
                      new XmlAttributeType("bundle", "major", int.class).
                      add(XML_ROOT),
                      new XmlAttributeType("bundle", "minor", int.class).
                      add(XML_ROOT),
                      new XmlAttributeType("bundle", "micro", int.class).
                      add(XML_ROOT));
    }

    /**
     * Create a list of {@code BundleVersion} for test.
     *
     * @return  A list of {@code BundleVersion}, including a {@code null}.
     */
    private List<BundleVersion> createBundleVersions() {
        List<BundleVersion> list = new ArrayList<BundleVersion>();
        String[] qualifiers = {null, "SNAPSHOT"};
        for (String qf: qualifiers) {
            for (int maj = 0; maj < 3; maj++) {
                for (int min = 10; min < 13; min++) {
                    for (int micro = 20; micro < 23; micro++) {
                        BundleVersion bv =
                            new BundleVersion(maj, min, micro, qf);
                        list.add(bv);
                    }
                }
            }
        }

        list.add(null);
        return list;
    }

    /**
     * Return a deep copy of the given bundle version object.
     *
     * @param bv  A {@code BundleVersion} object.
     * @return  A deep copy of the given {@code BundleVersion} object.
     */
    private BundleVersion copy(BundleVersion bv) {
        if (bv == null) {
            return null;
        }
        return new BundleVersion(bv.getMajor(), bv.getMinor(), bv.getMicro(),
                                 copy(bv.getQualifier()));
    }
}
