/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.routing.xml;

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

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.VtnPathCostConfig;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.vtn.path.policy.config.VtnPathCost;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.vtn.path.policy.config.VtnPathCostBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnPortDesc;

/**
 * JUnit test for {@link XmlPathCost}.
 */
public class XmlPathCostTest extends TestBase {
    /**
     * Root XML element name associated with {@link XmlPathCost} class.
     */
    private static final String  XML_ROOT = "vtn-path-cost";

    /**
     * Return a list of {@link XmlDataType} instances that specifies XML node
     * types mapped to a {@link XmlPathCost} instance.
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
            new XmlValueType("port-desc", VtnPortDesc.class).add(name).
            prepend(parent),
            new XmlValueType("cost", Long.class).add(name).prepend(parent));

        return dlist;
    }

    /**
     * Test case for {@link XmlPathCost#getImplementedInterface()}.
     */
    @Test
    public void testGetImplementedInterface() {
        XmlPathCost xpm = new XmlPathCost(new VtnPathCostBuilder().build());
        for (int i = 0; i < 10; i++) {
            assertEquals(VtnPathCostConfig.class,
                         xpm.getImplementedInterface());
        }
    }

    /**
     * Test case for {@link XmlPathCost#getPortDesc()}.
     */
    @Test
    public void testGetPortDesc() {
        String[] descs = {
            null,
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
            VtnPortDesc vpdesc = (desc == null) ? null : new VtnPortDesc(desc);
            VtnPathCost vpc = new VtnPathCostBuilder().
                setPortDesc(vpdesc).build();
            XmlPathCost xpc = new XmlPathCost(vpc);
            assertEquals(vpdesc, xpc.getPortDesc());
            assertEquals(null, xpc.getCost());
        }
    }

    /**
     * Test case for {@link XmlPathCost#getCost()}.
     */
    @Test
    public void testGetCost() {
        Long[] costs = {
            null, 1L, 2L, 10L, 222L, 3333L, 5555555L, 1234567890123L,
            99999999999999L, Long.MAX_VALUE,
        };

        for (Long cost: costs) {
            VtnPathCost vpc = new VtnPathCostBuilder().
                setCost(cost).build();
            XmlPathCost xpc = new XmlPathCost(vpc);
            assertEquals(null, xpc.getPortDesc());
            assertEquals(cost, xpc.getCost());
        }
    }

    /**
     * Test case for {@link XmlPathCost#equals(Object)} and
     * {@link XmlPathCost#hashCode()}.
     */
    @Test
    public void testEquals() {
        HashSet<Object> set = new HashSet<>();

        String[] descs = {
            null,
            "openflow:1,2,eth2",
            "openflow:1,2,",
            "openflow:1,,",
            "openflow:9999:3,,eth3",
            "unknown:1,abc,port-A",
            "unknown:1,abc,",
            "unknown:1,,port-A",
            "unknown:1,,",
        };
        Long[] costs = {
            null, 1L, 2L, 10L, 999L, 12345L, 3141692L, 1234567890123L,
            77777777777777L, Long.MAX_VALUE,
        };

        VtnPathCostBuilder bld1 = new VtnPathCostBuilder();
        VtnPathCostBuilder bld2 = new VtnPathCostBuilder();
        for (String desc: descs) {
            if (desc == null) {
                bld1.setPortDesc(null);
                bld2.setPortDesc(null);
            } else {
                bld1.setPortDesc(new VtnPortDesc(desc));
                bld2.setPortDesc(new VtnPortDesc(copy(desc)));
            }

            for (Long cost: costs) {
                VtnPathCost vpc1 = bld1.setCost(cost).build();
                VtnPathCost vpc2 = bld2.setCost(copy(cost)).build();
                XmlPathCost xpc1 = new XmlPathCost(vpc1);
                XmlPathCost xpc2 = new XmlPathCost(vpc2);
                testEquals(set, xpc1, xpc2);
            }
        }

        int expected = descs.length * costs.length;
        assertEquals(expected, set.size());
    }

    /**
     * Test case for {@link XmlPathCost#toString()}.
     */
    @Test
    public void testToString() {
        String[] descs = {
            null,
            "openflow:1,2,eth2",
            "openflow:1,2,",
            "openflow:1,,",
            "openflow:9999:3,,eth3",
            "unknown:1,abc,port-A",
            "unknown:1,abc,",
            "unknown:1,,port-A",
            "unknown:1,,",
        };
        Long[] costs = {
            null, 1L, 2L, 10L, 999L, 12345L, 3141692L, 1234567890123L,
            77777777777777L, Long.MAX_VALUE,
        };

        VtnPathCostBuilder bld = new VtnPathCostBuilder();
        for (String desc: descs) {
            VtnPortDesc vpdesc = (desc == null) ? null : new VtnPortDesc(desc);
            bld.setPortDesc(vpdesc);
            for (Long cost: costs) {
                VtnPathCost vpc = bld.setCost(cost).build();
                XmlPathCost xpc = new XmlPathCost(vpc);
                String expected = "vtn-path-cost[port=" + desc +
                    ", cost=" + cost + "]";
                assertEquals(expected, xpc.toString());
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
        Class<XmlPathCost> type = XmlPathCost.class;
        Unmarshaller um = createUnmarshaller(type);

        // Normal case.
        String[] descs = {
            null,
            "openflow:1,2,eth2",
            "openflow:1,2,",
            "openflow:1,,",
            "openflow:9999:3,,eth3",
            "unknown:1,abc,port-A",
            "unknown:1,abc,",
            "unknown:1,,port-A",
            "unknown:1,,",
        };
        Long[] costs = {
            null, Long.MIN_VALUE, -123456789012345L, -333333333333L,
            0L, 1L, 2L, 10L, 999L, 12345L, 3141692L, 1234567890123L,
            77777777777777L, Long.MAX_VALUE,
        };

        for (String desc: descs) {
            VtnPortDesc vpdesc = (desc == null) ? null : new VtnPortDesc(desc);
            for (Long cost: costs) {
                XmlNode xnode = new XmlNode(XML_ROOT);
                if (desc != null) {
                    xnode.add(new XmlNode("port-desc", desc));
                }
                if (cost != null) {
                    xnode.add(new XmlNode("cost", cost));
                }

                String xml = xnode.toString();
                XmlPathCost xpc = unmarshal(um, xml, type);
                assertEquals(vpdesc, xpc.getPortDesc());
                assertEquals(cost, xpc.getCost());
            }
        }

        // Ensure that broken values in XML can be detected.
        jaxbErrorTest(um, type, getXmlDataTypes(XML_ROOT));
    }
}
