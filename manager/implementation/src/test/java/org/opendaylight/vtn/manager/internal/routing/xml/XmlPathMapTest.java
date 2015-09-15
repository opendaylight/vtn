/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.routing.xml;

import static org.opendaylight.vtn.manager.internal.util.pathpolicy.PathPolicyUtilsTest.PATH_POLICY_MIN;
import static org.opendaylight.vtn.manager.internal.util.pathpolicy.PathPolicyUtilsTest.PATH_POLICY_MAX;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashSet;
import java.util.List;

import javax.xml.bind.Unmarshaller;

import org.junit.Test;

import org.opendaylight.vtn.manager.internal.TestBase;
import org.opendaylight.vtn.manager.internal.XmlDataType;
import org.opendaylight.vtn.manager.internal.XmlNode;
import org.opendaylight.vtn.manager.internal.XmlValueType;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathmap.rev150328.VtnPathMapConfig;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathmap.rev150328.vtn.path.map.list.VtnPathMap;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathmap.rev150328.vtn.path.map.list.VtnPathMapBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeName;

/**
 * JUnit test for {@link XmlPathMap}.
 */
public class XmlPathMapTest extends TestBase {
    /**
     * Root XML element name associated with {@link XmlPathMap} class.
     */
    private static final String  XML_ROOT = "vtn-path-map";

    /**
     * Return a list of {@link XmlDataType} instances that specifies XML node
     * types mapped to a {@link XmlPathMap} instance.
     *
     * @param name    The name of the target node.
     * @param parent  Path to the parent node.
     * @return  A list of {@link XmlDataType} instances.
     */
    public static List<XmlDataType> getXmlDataTypes(String name,
                                                    String ... parent) {
        List<XmlDataType> dlist = new ArrayList<>();
        Collections.addAll(
            dlist,
            new XmlValueType("index", Integer.class).add(name).prepend(parent),
            new XmlValueType("condition", VnodeName.class).add(name).
            prepend(parent),
            new XmlValueType("policy", Integer.class).add(name).prepend(parent),
            new XmlValueType("idle-timeout", Integer.class).add(name).
            prepend(parent),
            new XmlValueType("hard-timeout", Integer.class).add(name).
            prepend(parent));

        return dlist;
    }

    /**
     * Test case for {@link XmlPathMap#getImplementedInterface()}.
     */
    @Test
    public void testGetImplementedInterface() {
        XmlPathMap xpm = new XmlPathMap(new VtnPathMapBuilder().build());
        for (int i = 0; i < 10; i++) {
            assertEquals(VtnPathMapConfig.class,
                         xpm.getImplementedInterface());
        }
    }

    /**
     * Test case for {@link XmlPathMap#getIndex()}.
     */
    @Test
    public void testGetIndex() {
        Integer[] indices = {
            null, 1, 2, 10, 333, 5555, 10000, 32767, 32768, 40000, 55555,
            65534, 65535,
        };

        for (Integer index: indices) {
            VtnPathMap vpm = new VtnPathMapBuilder().
                setIndex(index).build();
            XmlPathMap xpm = new XmlPathMap(vpm);
            assertEquals(index, xpm.getIndex());
            assertEquals(null, xpm.getCondition());
            assertEquals(null, xpm.getPolicy());
            assertEquals(null, xpm.getIdleTimeout());
            assertEquals(null, xpm.getHardTimeout());
        }
    }

    /**
     * Test case for {@link XmlPathMap#getCondition()}.
     */
    @Test
    public void testGetCondition() {
        String[] conds = {
            null,
            "a",
            "AB",
            "abcde",
            "0",
            "01",
            "0123456789",
            "a123456789B123456789c123456789D",
            "cond_1",
            "cond_2",
            "Condition_3",
        };

        for (String cond: conds) {
            VnodeName vcond = (cond == null) ? null : new VnodeName(cond);
            VtnPathMap vpm = new VtnPathMapBuilder().
                setCondition(vcond).build();
            XmlPathMap xpm = new XmlPathMap(vpm);
            assertEquals(null, xpm.getIndex());
            assertEquals(vcond, xpm.getCondition());
            assertEquals(null, xpm.getPolicy());
            assertEquals(null, xpm.getIdleTimeout());
            assertEquals(null, xpm.getHardTimeout());
        }
    }

    /**
     * Test case for {@link XmlPathMap#getPolicy()}.
     */
    @Test
    public void testGetPolicy() {
        List<Integer> ids = new ArrayList<>();
        ids.add(null);
        for (int id = PATH_POLICY_MIN; id <= PATH_POLICY_MAX; id++) {
            ids.add(id);
        }

        for (Integer pid: ids) {
            VtnPathMap vpm = new VtnPathMapBuilder().
                setPolicy(pid).build();
            XmlPathMap xpm = new XmlPathMap(vpm);
            assertEquals(null, xpm.getIndex());
            assertEquals(null, xpm.getCondition());
            assertEquals(pid, xpm.getPolicy());
            assertEquals(null, xpm.getIdleTimeout());
            assertEquals(null, xpm.getHardTimeout());
        }
    }

    /**
     * Test case for {@link XmlPathMap#getIdleTimeout()}.
     */
    @Test
    public void testGetIdleTimeout() {
        Integer[] timeouts = {
            null, 0, 1, 2, 10, 222, 3333, 10000, 32767, 32768, 44444,
            55555, 60000, 65534, 65535,
        };

        for (Integer timeout: timeouts) {
            VtnPathMap vpm = new VtnPathMapBuilder().
                setIdleTimeout(timeout).build();
            XmlPathMap xpm = new XmlPathMap(vpm);
            assertEquals(null, xpm.getIndex());
            assertEquals(null, xpm.getCondition());
            assertEquals(null, xpm.getPolicy());
            assertEquals(timeout, xpm.getIdleTimeout());
            assertEquals(null, xpm.getHardTimeout());
        }
    }

    /**
     * Test case for {@link XmlPathMap#getHardTimeout()}.
     */
    @Test
    public void testGetHardTimeout() {
        Integer[] timeouts = {
            null, 0, 1, 2, 10, 222, 3333, 10000, 32767, 32768, 44444,
            55555, 60000, 65534, 65535,
        };

        for (Integer timeout: timeouts) {
            VtnPathMap vpm = new VtnPathMapBuilder().
                setHardTimeout(timeout).build();
            XmlPathMap xpm = new XmlPathMap(vpm);
            assertEquals(null, xpm.getIndex());
            assertEquals(null, xpm.getCondition());
            assertEquals(null, xpm.getPolicy());
            assertEquals(null, xpm.getIdleTimeout());
            assertEquals(timeout, xpm.getHardTimeout());
        }
    }

    /**
     * Test case for {@link XmlPathMap#equals(Object)} and
     * {@link XmlPathMap#hashCode()}.
     */
    @Test
    public void testEquals() {
        HashSet<Object> set = new HashSet<>();

        Integer[] indices = {
            null, 333, 65535,
        };
        String[] conds = {
            null, "cond", "cond_1",
        };
        Integer[] timeouts = {
            null, 123, 65535,
        };

        List<Integer> ids = new ArrayList<>();
        ids.add(null);
        for (int id = PATH_POLICY_MIN; id <= PATH_POLICY_MAX; id++) {
            ids.add(id);
        }

        VtnPathMapBuilder bld1 = new VtnPathMapBuilder();
        VtnPathMapBuilder bld2 = new VtnPathMapBuilder();
        for (String cond: conds) {
            if (cond == null) {
                bld1.setCondition(null);
                bld2.setCondition(null);
            } else {
                bld1.setCondition(new VnodeName(cond));
                bld2.setCondition(new VnodeName(copy(cond)));
            }

            for (Integer index: indices) {
                bld1.setIndex(index);
                bld2.setIndex(copy(index));
                for (Integer pid: ids) {
                    bld1.setPolicy(pid);
                    bld2.setPolicy(copy(pid));
                    for (Integer idle: timeouts) {
                        bld1.setIdleTimeout(idle);
                        bld2.setIdleTimeout(copy(idle));
                        for (Integer hard: timeouts) {
                            VtnPathMap vpm1 =
                                bld1.setHardTimeout(hard).build();
                            VtnPathMap vpm2 =
                                bld2.setHardTimeout(copy(hard)).build();
                            XmlPathMap xpm1 = new XmlPathMap(vpm1);
                            XmlPathMap xpm2 = new XmlPathMap(vpm2);
                            testEquals(set, xpm1, xpm2);
                        }
                    }
                }
            }
        }

        int expected = indices.length * conds.length * timeouts.length *
            timeouts.length * ids.size();
        assertEquals(expected, set.size());
    }

    /**
     * Test case for {@link XmlPathMap#toString()}.
     */
    @Test
    public void testToString() {
        Integer[] indices = {
            null, 11, 65535,
        };
        String[] conds = {
            null, "cond", "cond_1",
        };
        Integer[] idles = {
            null, 234, 65535,
        };
        Integer[] hards = {
            null, 0, 33333,
        };

        List<Integer> ids = new ArrayList<>();
        ids.add(null);
        for (int id = PATH_POLICY_MIN; id <= PATH_POLICY_MAX; id++) {
            ids.add(id);
        }

        VtnPathMapBuilder bld = new VtnPathMapBuilder();
        for (String cond: conds) {
            VnodeName vcond = (cond == null) ? null : new VnodeName(cond);
            bld.setCondition(vcond);
            for (Integer index: indices) {
                bld.setIndex(index);
                for (Integer pid: ids) {
                    bld.setPolicy(pid);
                    for (Integer idle: idles) {
                        bld.setIdleTimeout(idle);
                        for (Integer hard: hards) {
                            VtnPathMap vpm = bld.setHardTimeout(hard).build();
                            XmlPathMap xpm = new XmlPathMap(vpm);
                            String expected = "vtn-path-map[index=" + index +
                                ", condition=" + cond + ", policy=" + pid +
                                ", idle-timeout=" + idle +
                                ", hard-timeout=" + hard + "]";
                            assertEquals(expected, xpm.toString());
                        }
                    }
                }
            }
        }
    }

    /**
     * Test case for XML binding.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testJAXB() throws Exception {
        Class<XmlPathMap> type = XmlPathMap.class;
        Unmarshaller um = createUnmarshaller(type);

        // Normal case.
        Integer[] indices = {
            null, 11, 65535,
        };
        String[] conds = {
            null, "cond", "cond_1",
        };
        Integer[] idles = {
            null, 234, 65535,
        };
        Integer[] hards = {
            null, 0, 33333,
        };

        List<Integer> ids = new ArrayList<>();
        ids.add(null);
        for (int id = PATH_POLICY_MIN; id <= PATH_POLICY_MAX; id++) {
            ids.add(id);
        }

        for (String cond: conds) {
            VnodeName vcond = (cond == null) ? null : new VnodeName(cond);
            for (Integer index: indices) {
                for (Integer pid: ids) {
                    for (Integer idle: idles) {
                        for (Integer hard: hards) {
                            XmlNode xnode = new XmlNode(XML_ROOT);
                            if (index != null) {
                                xnode.add(new XmlNode("index", index));
                            }
                            if (cond != null) {
                                xnode.add(new XmlNode("condition", cond));
                            }
                            if (pid != null) {
                                xnode.add(new XmlNode("policy", pid));
                            }
                            if (idle != null) {
                                xnode.add(new XmlNode("idle-timeout", idle));
                            }
                            if (hard != null) {
                                xnode.add(new XmlNode("hard-timeout", hard));
                            }

                            String xml = xnode.toString();
                            XmlPathMap xpm = unmarshal(um, xml, type);
                            assertEquals(index, xpm.getIndex());
                            assertEquals(vcond, xpm.getCondition());
                            assertEquals(pid, xpm.getPolicy());
                            assertEquals(idle, xpm.getIdleTimeout());
                            assertEquals(hard, xpm.getHardTimeout());
                            jaxbTest(xpm, type, XML_ROOT);
                        }
                    }
                }
            }
        }

        // Ensure that broken values in XML can be detected.
        jaxbErrorTest(um, type, getXmlDataTypes(XML_ROOT));
    }
}
