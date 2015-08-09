/*
 * Copyright (c) 2014, 2015 NEC Corporation. All rights reserved.
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
 * JUnit test for {@link PathMap}.
 */
public class PathMapTest extends TestBase {
    /**
     * Root XML element name associated with {@link PathMap} class.
     */
    private static final String  XML_ROOT = "pathmap";

    /**
     * Test case for getter methods and {@link PathMap#clone()}.
     */
    @Test
    public void testGetter() {
        int[] policyIds = {Integer.MIN_VALUE, 1, 10, Integer.MAX_VALUE};
        Integer[] indices = {
            null, Integer.valueOf(Integer.MIN_VALUE),
            Integer.valueOf(1), Integer.valueOf(512),
            Integer.valueOf(Integer.MAX_VALUE),
        };
        int[] idles = {0, 200, 33333};
        int[] hards = {0, 159, 12345678};

        for (Integer idx: indices) {
            for (String cond: createStrings("cond")) {
                for (int policyId: policyIds) {
                    PathMap pmap = (idx == null)
                        ? new PathMap(cond, policyId)
                        : new PathMap(idx.intValue(), cond, policyId);
                    assertEquals(idx, pmap.getIndex());
                    assertEquals(cond, pmap.getFlowConditionName());
                    assertEquals(policyId, pmap.getPathPolicyId());
                    assertEquals(null, pmap.getIdleTimeout());
                    assertEquals(null, pmap.getHardTimeout());

                    PathMap cloned = pmap.clone();
                    assertNotSame(pmap, cloned);
                    assertEquals(pmap, cloned);
                    assertEquals(idx, cloned.getIndex());
                    assertEquals(cond, cloned.getFlowConditionName());
                    assertEquals(policyId, cloned.getPathPolicyId());
                    assertEquals(null, cloned.getIdleTimeout());
                    assertEquals(null, cloned.getHardTimeout());

                    for (int idle: idles) {
                        for (int hard: hards) {
                            pmap = (idx == null)
                                ? new PathMap(cond, policyId, idle, hard)
                                : new PathMap(idx.intValue(), cond, policyId,
                                              idle, hard);
                            assertEquals(idx, pmap.getIndex());
                            assertEquals(cond, pmap.getFlowConditionName());
                            assertEquals(policyId, pmap.getPathPolicyId());
                            assertEquals(Integer.valueOf(idle),
                                         pmap.getIdleTimeout());
                            assertEquals(Integer.valueOf(hard),
                                         pmap.getHardTimeout());

                            cloned = pmap.clone();
                            assertNotSame(pmap, cloned);
                            assertEquals(pmap, cloned);
                            assertEquals(idx, cloned.getIndex());
                            assertEquals(cond, cloned.getFlowConditionName());
                            assertEquals(policyId, cloned.getPathPolicyId());
                            assertEquals(Integer.valueOf(idle),
                                         cloned.getIdleTimeout());
                            assertEquals(Integer.valueOf(hard),
                                         cloned.getHardTimeout());
                        }
                    }
                }
            }
        }
    }

    /**
     * Test case for {@link PathMap#equals(Object)} and
     * {@link PathMap#hashCode()}.
     */
    @Test
    public void testEquals() {
        HashSet<Object> set = new HashSet<Object>();
        int[] policyIds = {1, 23456};
        List<String> conditions = createStrings("cd");
        Integer[] indices = {null, Integer.valueOf(1), Integer.valueOf(10)};
        Integer[] idles = {null, Integer.valueOf(6000)};
        Integer[] hards = {null, Integer.valueOf(234000)};

        for (Integer idx: indices) {
            for (String cond: conditions) {
                for (int policy: policyIds) {
                    for (Integer idle: idles) {
                        for (Integer hard: hards) {
                            PathMap pmap1 = createPathMap(idx, cond, policy,
                                                          idle, hard);
                            PathMap pmap2 = createPathMap(idx, cond, policy,
                                                          idle, hard);
                            testEquals(set, pmap1, pmap2);
                        }
                    }
                }
            }
        }

        int required = indices.length * conditions.size() * policyIds.length *
            idles.length * hards.length;
        assertEquals(required, set.size());
    }

    /**
     * Test case for {@link PathMap#toString()}.
     */
    @Test
    public void testToString() {
        String prefix = "PathMap[";
        String suffix = "]";
        int[] policyIds = {1, 10, Integer.MAX_VALUE};
        Integer[] indices = {
            null, Integer.valueOf(1), Integer.valueOf(237),
        };
        Integer[] idles = {
            null, Integer.valueOf(0), Integer.valueOf(3000),
        };
        Integer[] hards = {
            null, Integer.valueOf(0), Integer.valueOf(15000),
        };

        for (Integer idx: indices) {
            String i = (idx == null) ? null : "index=" + idx;
            for (String cond: createStrings("cd")) {
                String c = (cond == null) ? null : "cond=" + cond;
                for (int policy: policyIds) {
                    String p = "policy=" + policy;
                    for (Integer idle: idles) {
                        String it = (idle == null) ? null : "idle=" + idle;
                        for (Integer hard: hards) {
                            PathMap pmap = createPathMap(idx, cond, policy,
                                                         idle, hard);
                            String ht = (hard == null) ? null : "hard=" + hard;
                            String required =
                                joinStrings(prefix, suffix, ",", i, c, p,
                                            it, ht);
                            assertEquals(required, pmap.toString());
                        }
                    }
                }
            }
        }
    }

    /**
     * Ensure that {@link PathMap} is serializable.
     */
    @Test
    public void testSerialize() {
        int[] policyIds = {1, 10, Integer.MAX_VALUE};
        Integer[] indices = {
            null, Integer.valueOf(1), Integer.valueOf(237),
        };
        Integer[] idles = {null, Integer.valueOf(2000)};
        Integer[] hards = {null, Integer.valueOf(15000)};

        for (Integer idx: indices) {
            for (String cond: createStrings("cnd")) {
                for (int policy: policyIds) {
                    for (Integer idle: idles) {
                        for (Integer hard: hards) {
                            PathMap pmap = createPathMap(idx, cond, policy,
                                                         idle, hard);
                            serializeTest(pmap);
                        }
                    }
                }
            }
        }
    }

    /**
     * Ensure that {@link PathMap} is mapped to XML root element.
     */
    @Test
    public void testJAXB() {
        int[] policyIds = {1, 4567};
        Integer[] indices = {null, Integer.valueOf(133)};
        Integer[] idles = {null, Integer.valueOf(5000)};
        Integer[] hards = {null, Integer.valueOf(13000)};
        String[] conditions = {null, "cond"};

        for (Integer idx: indices) {
            for (String cond: conditions) {
                for (int policy: policyIds) {
                    for (Integer idle: idles) {
                        for (Integer hard: hards) {
                            PathMap pmap = createPathMap(idx, cond, policy,
                                                         idle, hard);
                            jaxbTest(pmap, PathMap.class, XML_ROOT);
                        }
                    }
                }
            }
        }

        // Ensure that broken values in XML can be detected.
        jaxbErrorTest(PathMap.class,
                      new XmlAttributeType(XML_ROOT, "index", Integer.class),
                      new XmlAttributeType(XML_ROOT, "policy", int.class),
                      new XmlAttributeType(XML_ROOT, "idleTimeout",
                                           Integer.class),
                      new XmlAttributeType(XML_ROOT, "hardTimeout",
                                           Integer.class));
    }

    /**
     * Ensure that {@link PathMap} is mapped to JSON object.
     */
    @Test
    public void testJSON() {
        int[] policyIds = {1, 4567};
        Integer[] indices = {null, Integer.valueOf(133)};
        Integer[] idles = {null, Integer.valueOf(5000)};
        Integer[] hards = {null, Integer.valueOf(13000)};
        String[] conditions = {null, "cond"};

        for (Integer idx: indices) {
            for (String cond: conditions) {
                for (int policy: policyIds) {
                    for (Integer idle: idles) {
                        for (Integer hard: hards) {
                            PathMap pmap = createPathMap(idx, cond, policy,
                                                         idle, hard);
                            jsonTest(pmap, PathMap.class);
                        }
                    }
                }
            }
        }
    }

    /**
     * Construct an instance of {@link PathMap} using JAXB.
     *
     * @param index   An {@link Integer} instance that represents an index
     *                value.
     * @param cond    The name of the flow condition.
     * @param policy  The identifier of the path policy.
     * @param idle    An {@link Integer} instance that represents idle timeout.
     * @param hard    An {@link Integer} instance that represents hard timeout.
     * @return  A {@link PathMap} instance.
     */
    private PathMap createPathMap(Integer index, String cond, int policy,
                                  Integer idle, Integer hard) {
        StringBuilder builder = new StringBuilder(XML_DECLARATION);
        builder.append('<').append(XML_ROOT);
        if (index != null) {
            builder.append(" index=\"").append(index).append("\"");
        }
        if (cond != null) {
            builder.append(" condition=\"").append(cond).append("\"");
        }
        builder.append(" policy=\"").append(policy).append("\"");
        if (idle != null) {
            builder.append(" idleTimeout=\"").append(idle).append("\"");
        }
        if (hard != null) {
            builder.append(" hardTimeout=\"").append(hard).append("\"");
        }

        String xml = builder.append(" />").toString();
        Unmarshaller um = createUnmarshaller(PathMap.class);
        PathMap pmap = null;
        try {
            pmap = unmarshal(um, xml, PathMap.class);
            assertEquals(index, pmap.getIndex());
            assertEquals(cond, pmap.getFlowConditionName());
            assertEquals(policy, pmap.getPathPolicyId());
            assertEquals(idle, pmap.getIdleTimeout());
            assertEquals(hard, pmap.getHardTimeout());
        } catch (Exception e) {
            unexpected(e);
        }

        return pmap;
    }
}
