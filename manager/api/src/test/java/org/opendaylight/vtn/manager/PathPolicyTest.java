/*
 * Copyright (c) 2014, 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashSet;
import java.util.List;

import org.junit.Test;

import org.opendaylight.controller.sal.core.Node;

/**
 * JUnit test for {@link PathPolicy}.
 */
public class PathPolicyTest extends TestBase {
    /**
     * Root XML element name associated with {@link PathPolicy} class.
     */
    private static final String  XML_ROOT = "pathpolicy";

    /**
     * Test case for getter methods.
     */
    @Test
    public void testGetter() {
        int[] ids = {0, 1, 30};
        long[] defCosts = {0L, 333L, Long.MAX_VALUE};
        for (List<PathCost> costs: createPathCostLists()) {
            for (long def: defCosts) {
                PathPolicy policy = new PathPolicy(def, costs);
                assertEquals(null, policy.getPolicyId());
                assertEquals(def, policy.getDefaultCost());
                List<PathCost> l = policy.getPathCosts();
                assertNotSame(l, costs);
                assertEquals(l, costs);

                for (int id: ids) {
                    policy = new PathPolicy(id, def, costs);
                    assertEquals(Integer.valueOf(id), policy.getPolicyId());
                    assertEquals(def, policy.getDefaultCost());
                    l = policy.getPathCosts();
                    assertNotSame(l, costs);
                    assertEquals(l, costs);

                    Integer pid = (id == 0) ? null : Integer.valueOf(id);
                    policy = new PathPolicy(pid, def, costs);
                    assertEquals(pid, policy.getPolicyId());
                    assertEquals(def, policy.getDefaultCost());
                    l = policy.getPathCosts();
                    assertNotSame(l, costs);
                    assertEquals(l, costs);
                }
            }
        }

        long def = 10L;
        PathPolicy policy = new PathPolicy(def, null);
        assertEquals(null, policy.getPolicyId());
        assertEquals(def, policy.getDefaultCost());
        assertEquals(0, policy.getPathCosts().size());

        int id = 1;
        policy = new PathPolicy(id, def, null);
        assertEquals(Integer.valueOf(id), policy.getPolicyId());
        assertEquals(def, policy.getDefaultCost());
        assertEquals(0, policy.getPathCosts().size());
    }

    /**
     * Test case for {@link PathPolicy#equals(Object)} and
     * {@link PathPolicy#hashCode()}.
     */
    @Test
    public void testEquals() {
        HashSet<Object> set = new HashSet<Object>();
        int[] ids = {0, 1, 30};
        long[] defCosts = {0L, 333L, Long.MAX_VALUE};
        List<List<PathCost>> costLists1 = createPathCostLists();
        List<List<PathCost>> costLists2 = createPathCostLists();
        for (int costIdx = 0; costIdx < costLists1.size(); costIdx++) {
            List<PathCost> costs1 = costLists1.get(costIdx);
            List<PathCost> costs2 = costLists2.get(costIdx);
            for (long def: defCosts) {
                PathPolicy policy1 = new PathPolicy(def, costs1);
                PathPolicy policy2 = new PathPolicy(def, costs2);
                testEquals(set, policy1, policy2);

                for (int id: ids) {
                    policy1 = new PathPolicy(id, def, costs1);
                    policy2 = new PathPolicy(id, def, costs2);
                    testEquals(set, policy1, policy2);
                }
            }
        }

        int ncosts = costLists1.size() * defCosts.length;
        int required = ncosts + (ncosts * ids.length);
        assertEquals(required, set.size());
    }

    /**
     * Test case for {@link PathPolicy#toString()}.
     */
    @Test
    public void testToString() {
        String prefix = "PathPolicy[";
        String suffix = "]";
        int[] ids = {0, 1, 30};
        long[] defCosts = {0L, 333L, Long.MAX_VALUE};
        for (long def: defCosts) {
            String d = "default=" + def;
            for (List<PathCost> costs: createPathCostLists()) {
                PathPolicy policy = new PathPolicy(def, costs);
                String c = (costs.isEmpty()) ? null : "costs=" + costs;
                String required = joinStrings(prefix, suffix, ",", d, c);
                assertEquals(required, policy.toString());

                for (int id: ids) {
                    policy = new PathPolicy(id, def, costs);
                    String i = "id=" + id;
                    required = joinStrings(prefix, suffix, ",", i, d, c);
                    assertEquals(required, policy.toString());
                }
            }
        }
    }

    /**
     * Ensure that {@link PathPolicy} is serializable.
     */
    @Test
    public void testSerialize() {
        int[] ids = {0, 1, 30};
        long[] defCosts = {0L, 333L, Long.MAX_VALUE};
        for (long def: defCosts) {
            for (List<PathCost> costs: createPathCostLists()) {
                PathPolicy policy = new PathPolicy(def, costs);
                serializeTest(policy);

                for (int id: ids) {
                    policy = new PathPolicy(id, def, costs);
                    serializeTest(policy);
                }
            }
        }
    }

    /**
     * Ensure that {@link PathPolicy} is mapped to XML root element.
     */
    @Test
    public void testJAXB() {
        int[] ids = {0, 1, 30};
        long[] defCosts = {0L, 333L, Long.MAX_VALUE};
        for (long def: defCosts) {
            for (List<PathCost> costs: createPathCostLists()) {
                PathPolicy policy = new PathPolicy(def, costs);
                jaxbTest(policy, PathPolicy.class, XML_ROOT);

                for (int id: ids) {
                    policy = new PathPolicy(id, def, costs);
                    jaxbTest(policy, PathPolicy.class, XML_ROOT);
                }
            }
        }

        // Ensure that broken values in XML can be detected.
        List<XmlDataType> dlist = PathCostTest.
            getXmlDataTypes("cost", XML_ROOT, "costs");
        Collections.addAll(dlist,
                           new XmlAttributeType(XML_ROOT, "id", Integer.class),
                           new XmlAttributeType(XML_ROOT, "default",
                                                long.class));
        jaxbErrorTest(PathPolicy.class, dlist);
    }

    /**
     * Ensure that {@link PathCost} is mapped to JSON object.
     */
    @Test
    public void testJSON() {
        int[] ids = {0, 1, 30};
        long[] defCosts = {0L, 333L, Long.MAX_VALUE};
        for (long def: defCosts) {
            for (List<PathCost> costs: createPathCostLists()) {
                PathPolicy policy = new PathPolicy(def, costs);
                jsonTest(policy, PathPolicy.class);

                for (int id: ids) {
                    policy = new PathPolicy(id, def, costs);
                    jsonTest(policy, PathPolicy.class);
                }
            }
        }
    }

    /**
     * Create lists of {@link PathCost} instances for test.
     *
     * @return  A list of {@link PathCost} lists.
     */
    private List<List<PathCost>> createPathCostLists() {
        ArrayList<List<PathCost>> list = new ArrayList<List<PathCost>>();
        list.add(new ArrayList<PathCost>());

        long[] costs = {0L, 12345678L};
        ArrayList<PathCost> l = new ArrayList<PathCost>();
        for (Node node: createNodes(2)) {
            for (SwitchPort port: createSwitchPorts(2)) {
                for (long c: costs) {
                    PortLocation ploc = new PortLocation(node, port);
                    PathCost pc = new PathCost(ploc, c);
                    l.add(pc);
                    list.add(new ArrayList<PathCost>(l));
                }
            }
        }

        return list;
    }
}
