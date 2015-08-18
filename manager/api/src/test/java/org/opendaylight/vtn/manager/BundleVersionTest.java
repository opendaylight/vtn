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

import org.junit.Test;

/**
 * JUnit test for {@link BundleVersion}.
 */
public class BundleVersionTest extends TestBase {
    /**
     * Root XML element name associated with {@link BundleVersion} class.
     */
    private static final String  XML_ROOT = "bundleVersion";

    /**
     * Test case for getter methods.
     */
    @Test
    public void testGetter() {
        for (String qf: createStrings("SNAPSHOT")) {
            for (Integer maj: createIntegers(0, 3, false)) {
                for (Integer min: createIntegers(0, 3, false)) {
                    for (Integer micro: createIntegers(0, 3, false)) {
                        BundleVersion bv =
                            new BundleVersion(maj, min, micro, qf);
                        assertEquals(maj.intValue(), bv.getMajor());
                        assertEquals(min.intValue(), bv.getMinor());
                        assertEquals(micro.intValue(), bv.getMicro());
                        if (qf == null || qf.isEmpty()) {
                            assertNull(bv.getQualifier());
                        } else {
                            assertEquals(qf, bv.getQualifier());
                        }
                    }
                }
            }
        }
    }

    /**
     * Test case for {@link BundleVersion#equals(Object)} and
     * {@link BundleVersion#hashCode()}.
     */
    @Test
    public void testEquals() {
        HashSet<Object> set = new HashSet<Object>();
        List<String> qualifiers = createStrings("SNAPSHOT", false);
        List<Integer> majors = createIntegers(0, 3, false);
        List<Integer> minors = createIntegers(0, 3, false);
        List<Integer> micros = createIntegers(0, 3, false);
        for (String qf: qualifiers) {
            for (Integer maj: majors) {
                for (Integer min: minors) {
                    for (Integer micro: micros) {
                        BundleVersion bv1 =
                            new BundleVersion(maj, min, micro, qf);
                        BundleVersion bv2 =
                            new BundleVersion(maj, min, micro, copy(qf));
                        testEquals(set, bv1, bv2);
                    }
                }
            }
        }

        int required = qualifiers.size() * majors.size() * minors.size() *
            micros.size();
        assertEquals(required, set.size());
    }

    /**
     * Test case for {@link BundleVersion#toString()}.
     */
    @Test
    public void testToString() {
        for (String qf: createStrings("SNAPSHOT", false)) {
            for (Integer maj: createIntegers(0, 3, false)) {
                for (Integer min: createIntegers(0, 3, false)) {
                    for (Integer micro: createIntegers(0, 3, false)) {
                        BundleVersion bv =
                            new BundleVersion(maj, min, micro, qf);

                        StringBuilder builder = new StringBuilder();
                        builder.append(maj).append('.').append(min).
                            append('.').append(micro);
                        if (qf != null && !qf.isEmpty()) {
                            builder.append('.').append(qf);
                        }

                        assertEquals(builder.toString(), bv.toString());
                    }
                }
            }
        }
    }

    /**
     * Ensure that {@link BundleVersion} is serializable.
     */
    @Test
    public void testSerialize() {
        for (String qf: createStrings("SNAPSHOT", false)) {
            for (Integer maj: createIntegers(0, 3, false)) {
                for (Integer min: createIntegers(0, 3, false)) {
                    for (Integer micro: createIntegers(0, 3, false)) {
                        BundleVersion bv =
                            new BundleVersion(maj, min, micro, qf);
                        serializeTest(bv);
                    }
                }
            }
        }
    }

    /**
     * Ensure that {@link BundleVersion} is mapped to XML root element.
     */
    @Test
    public void testJAXB() {
        for (String qf: createStrings("SNAPSHOT", false)) {
            for (Integer maj: createIntegers(0, 3, false)) {
                for (Integer min: createIntegers(0, 3, false)) {
                    for (Integer micro: createIntegers(0, 3, false)) {
                        BundleVersion bv =
                            new BundleVersion(maj, min, micro, qf);
                        jaxbTest(bv, BundleVersion.class, "bundleVersion");
                    }
                }
            }
        }

        // Ensure that broken values in XML can be detected.
        jaxbErrorTest(BundleVersion.class,
                      new XmlAttributeType(XML_ROOT, "major", int.class),
                      new XmlAttributeType(XML_ROOT, "minor", int.class),
                      new XmlAttributeType(XML_ROOT, "micro", int.class));
    }
}
