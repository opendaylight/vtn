/*
 * Copyright (c) 2015, 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.routing.xml;

import static org.opendaylight.vtn.manager.internal.util.pathpolicy.PathPolicyUtilsTest.PATH_POLICY_MIN;
import static org.opendaylight.vtn.manager.internal.util.pathpolicy.PathPolicyUtilsTest.PATH_POLICY_MAX;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.HashSet;
import java.util.List;

import javax.xml.bind.Unmarshaller;

import org.junit.Test;

import org.opendaylight.vtn.manager.internal.TestBase;
import org.opendaylight.vtn.manager.internal.XmlDataType;
import org.opendaylight.vtn.manager.internal.XmlNode;
import org.opendaylight.vtn.manager.internal.XmlValueType;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.VtnPathPolicyConfig;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.vtn.path.policies.VtnPathPolicy;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.vtn.path.policies.VtnPathPolicyBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.vtn.path.policy.config.VtnPathCost;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.vtn.path.policy.config.VtnPathCostBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnPortDesc;

/**
 * JUnit test for {@link XmlPathPolicy}.
 */
public class XmlPathPolicyTest extends TestBase {
    /**
     * Root XML element name associated with {@link XmlPathPolicy} class.
     */
    private static final String  XML_ROOT = "vtn-path-policy";

    /**
     * Return a list of {@link XmlDataType} instances that specifies XML node
     * types mapped to a {@link XmlPathPolicy} instance.
     *
     * @param name    The name of the target node.
     * @param parent  Path to the parent node.
     * @return  A list of {@link XmlDataType} instances.
     */
    public static List<XmlDataType> getXmlDataTypes(String name,
                                                    String ... parent) {
        String[] p = XmlDataType.addPath(
            "vtn-path-costs", XmlDataType.addPath(name, parent));
        List<XmlDataType> dlist = new ArrayList<>();
        Collections.addAll(
            dlist,
            new XmlValueType("id", Integer.class).add(name).prepend(parent),
            new XmlValueType("default", Long.class).add(name).prepend(parent));
        dlist.addAll(XmlPathCostTest.getXmlDataTypes("vtn-path-cost", p));

        return dlist;
    }

    /**
     * Test case for {@link XmlPathPolicy#getImplementedInterface()}.
     */
    @Test
    public void testGetImplementedInterface() {
        XmlPathPolicy xpp = new XmlPathPolicy(
            new VtnPathPolicyBuilder().build());
        for (int i = 0; i < 10; i++) {
            assertEquals(VtnPathPolicyConfig.class,
                         xpp.getImplementedInterface());
        }
    }

    /**
     * Test case for {@link XmlPathPolicy#getId()}.
     */
    @Test
    public void testGetId() {
        List<Integer> ids = new ArrayList<>();
        ids.add(null);
        for (int id = PATH_POLICY_MIN; id <= PATH_POLICY_MAX; id++) {
            ids.add(id);
        }

        for (Integer id: ids) {
            VtnPathPolicy vpp = new VtnPathPolicyBuilder().
                setId(id).build();
            XmlPathPolicy xpp = new XmlPathPolicy(vpp);
            assertEquals(id, xpp.getId());
            assertEquals(null, xpp.getDefaultCost());
            assertEquals(null, xpp.getVtnPathCost());
        }
    }

    /**
     * Test case for {@link XmlPathPolicy#getDefaultCost()}.
     */
    @Test
    public void testGetDefaultCost() {
        Long[] costs = {
            null, 0L, 1L, 2L, 10L, 222L, 3333L, 5555555L, 1234567890123L,
            99999999999999L, Long.MAX_VALUE,
        };

        for (Long cost: costs) {
            VtnPathPolicy vpp = new VtnPathPolicyBuilder().
                setDefaultCost(cost).build();
            XmlPathPolicy xpp = new XmlPathPolicy(vpp);
            assertEquals(null, xpp.getId());
            assertEquals(cost, xpp.getDefaultCost());
            assertEquals(null, xpp.getVtnPathCost());
        }
    }

    /**
     * Test case for {@link XmlPathPolicy#getVtnPathCost()}.
     */
    @Test
    public void testGetVtnPathCost() {
        // Null.
        XmlPathPolicy xpp = new XmlPathPolicy(
            new VtnPathPolicyBuilder().build());
        assertEquals(null, xpp.getId());
        assertEquals(null, xpp.getDefaultCost());
        assertEquals(null, xpp.getVtnPathCost());

        // Empty list.
        List<VtnPathCost> vpcosts = new ArrayList<>();
        xpp = new XmlPathPolicy(
            new VtnPathPolicyBuilder().setVtnPathCost(vpcosts).build());
        assertEquals(null, xpp.getId());
        assertEquals(null, xpp.getDefaultCost());
        assertEquals(null, xpp.getVtnPathCost());

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

        VtnPathPolicyBuilder bld = new VtnPathPolicyBuilder();
        VtnPathCostBuilder cbld = new VtnPathCostBuilder();
        for (String desc: descs) {
            VtnPortDesc vpdesc = (desc == null) ? null : new VtnPortDesc(desc);
            cbld.setPortDesc(vpdesc);
            for (Long cost: costs) {
                vpcosts.add(cbld.setCost(cost).build());
                VtnPathPolicy vpp = bld.setVtnPathCost(vpcosts).build();
                xpp = new XmlPathPolicy(vpp);
                assertEquals(null, xpp.getId());
                assertEquals(null, xpp.getDefaultCost());
                assertEquals(vpcosts, xpp.getVtnPathCost());
            }
        }
    }

    /**
     * Test case for {@link XmlPathPolicy#equals(Object)} and
     * {@link XmlPathPolicy#hashCode()}.
     */
    @Test
    public void testEquals() {
        HashSet<Object> set = new HashSet<>();

        List<Integer> ids = new ArrayList<>();
        ids.add(null);
        for (int id = PATH_POLICY_MIN; id <= PATH_POLICY_MAX; id++) {
            ids.add(id);
        }

        Long[] defCosts = {
            null, 0L, 10L, 5555555L, 1234567890123L, Long.MAX_VALUE,
        };
        String[] descs = {
            null,
            "openflow:1,2,eth2",
            "openflow:1,,",
            "unknown:1,,port-1",
        };
        Long[] costs = {
            null, 1L, 12345678L, Long.MAX_VALUE,
        };

        List<VtnPathCost> vpcosts = new ArrayList<>();
        VtnPathCostBuilder cbld = new VtnPathCostBuilder();
        for (String desc: descs) {
            VtnPortDesc vpdesc = (desc == null) ? null : new VtnPortDesc(desc);
            cbld.setPortDesc(vpdesc);
            for (Long cost: costs) {
                vpcosts.add(cbld.setCost(cost).build());
            }
        }
        List<List<VtnPathCost>> vpcLists = Arrays.asList(
            (List<VtnPathCost>)null,
            Collections.singletonList(vpcosts.get(0)),
            vpcosts);

        VtnPathPolicyBuilder bld1 = new VtnPathPolicyBuilder();
        VtnPathPolicyBuilder bld2 = new VtnPathPolicyBuilder();
        for (Integer id: ids) {
            bld1.setId(id);
            bld2.setId(copy(id));
            for (Long cost: defCosts) {
                bld1.setDefaultCost(cost);
                bld2.setDefaultCost(copy(cost));
                for (List<VtnPathCost> vpcs1: vpcLists) {
                    List<VtnPathCost> vpcs2 = (vpcs1 == null)
                        ? null
                        : new ArrayList<>(vpcs1);
                    VtnPathPolicy vpp1 = bld1.setVtnPathCost(vpcs1).build();
                    VtnPathPolicy vpp2 = bld2.setVtnPathCost(vpcs2).build();
                    XmlPathPolicy xpp1 = new XmlPathPolicy(vpp1);
                    XmlPathPolicy xpp2 = new XmlPathPolicy(vpp2);
                    testEquals(set, xpp1, xpp2);
                }
            }
        }

        int expected = ids.size() * defCosts.length * vpcLists.size();
        assertEquals(expected, set.size());
    }

    /**
     * Test case for {@link XmlPathPolicy#toString()}.
     */
    @Test
    public void testToString() {
        List<Integer> ids = new ArrayList<>();
        ids.add(null);
        for (int id = PATH_POLICY_MIN; id <= PATH_POLICY_MAX; id++) {
            ids.add(id);
        }

        Long[] defCosts = {
            null, 0L, 10L, 5555555L, 1234567890123L, Long.MAX_VALUE,
        };
        String[] descs = {
            null,
            "openflow:1,2,eth2",
            "openflow:1,,",
            "openflow:9999:3,,eth3",
            "unknown:1,abc,",
            "unknown:1,,",
        };
        Long[] costs = {
            null, 1L, 11L, 8888888L, 12345678901234L, Long.MAX_VALUE,
        };

        List<VtnPathCost> vpcosts = new ArrayList<>();
        VtnPathCostBuilder cbld = new VtnPathCostBuilder();
        for (String desc: descs) {
            VtnPortDesc vpdesc = (desc == null) ? null : new VtnPortDesc(desc);
            cbld.setPortDesc(vpdesc);
            for (Long cost: costs) {
                vpcosts.add(cbld.setCost(cost).build());
            }
        }

        VtnPathPolicyBuilder bld = new VtnPathPolicyBuilder();
        List<VtnPathCost> vpcs = new ArrayList<>();
        List<XmlPathCost> xpcs = new ArrayList<>();
        for (Integer id: ids) {
            bld.setId(id);
            for (Long cost: defCosts) {
                bld.setDefaultCost(cost);
                for (VtnPathCost vpc: vpcosts) {
                    vpcs.add(vpc);
                    xpcs.add(new XmlPathCost(vpc));
                    VtnPathPolicy vpp = bld.setVtnPathCost(vpcs).build();
                    XmlPathPolicy xpp = new XmlPathPolicy(vpp);
                    String expected = "vtn-path-policy[id=" + id +
                        ", default=" + cost + ", costs=" + xpcs + "]";
                    assertEquals(expected, xpp.toString());
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
        Class<XmlPathPolicy> type = XmlPathPolicy.class;
        Unmarshaller um = createUnmarshaller(type);

        // Normal case.
        List<Integer> ids = new ArrayList<>();
        ids.add(null);
        for (int id = PATH_POLICY_MIN; id <= PATH_POLICY_MAX; id++) {
            ids.add(id);
        }

        Long[] defCosts = {
            null, 0L, 10L, 5555555L, 1234567890123L, Long.MAX_VALUE,
        };
        String[] descs = {
            null,
            "openflow:2,10,eth10",
            "openflow:5,,",
            "openflow:9999:1,,eth1",
            "unknown:33,abc,",
            "unknown:1,,",
        };
        Long[] costs = {
            null, 1L, 11L, 8888888L, 12345678901234L, Long.MAX_VALUE,
        };

        List<VtnPathCost> vpcosts = new ArrayList<>();
        VtnPathCostBuilder cbld = new VtnPathCostBuilder();
        for (String desc: descs) {
            VtnPortDesc vpdesc = (desc == null) ? null : new VtnPortDesc(desc);
            cbld.setPortDesc(vpdesc);
            for (Long cost: costs) {
                vpcosts.add(cbld.setCost(cost).build());
            }
        }

        List<List<VtnPathCost>> vpcLists = Arrays.asList(
            (List<VtnPathCost>)null, Collections.<VtnPathCost>emptyList(),
            vpcosts);

        for (Integer id: ids) {
            for (Long cost: defCosts) {
                for (List<VtnPathCost> vpcs: vpcLists) {
                    XmlNode xnode = new XmlNode(XML_ROOT);
                    if (id != null) {
                        xnode.add(new XmlNode("id", id));
                    }
                    if (cost != null) {
                        xnode.add(new XmlNode("default", cost));
                    }
                    if (vpcs != null) {
                        XmlNode xlist = new XmlNode("vtn-path-costs");
                        for (VtnPathCost vpc: vpcs) {
                            XmlNode xcost = new XmlNode("vtn-path-cost");
                            VtnPortDesc vpdesc = vpc.getPortDesc();
                            if (vpdesc != null) {
                                String desc = vpdesc.getValue();
                                xcost.add(new XmlNode("port-desc", desc));
                            }

                            Long c = vpc.getCost();
                            if (c != null) {
                                xcost.add(new XmlNode("cost", c));
                            }
                            xlist.add(xcost);
                        }

                        xnode.add(xlist);
                    }

                    String xml = xnode.toString();
                    XmlPathPolicy xpp = unmarshal(um, xml, type);
                    assertEquals(id, xpp.getId());
                    assertEquals(cost, xpp.getDefaultCost());
                    assertEquals(vpcs, xpp.getVtnPathCost());
                    jaxbTest(xpp, type, XML_ROOT);
                }
            }
        }

        // Ensure that broken values in XML can be detected.
        jaxbErrorTest(um, type, getXmlDataTypes(XML_ROOT));
    }
}
