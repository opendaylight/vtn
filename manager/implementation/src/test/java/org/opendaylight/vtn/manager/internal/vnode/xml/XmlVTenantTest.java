/*
 * Copyright (c) 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.vnode.xml;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertSame;

import static org.mockito.Mockito.any;
import static org.mockito.Mockito.eq;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.verifyNoMoreInteractions;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashSet;
import java.util.List;
import java.util.Random;
import java.util.Set;

import javax.xml.bind.Unmarshaller;

import org.junit.Test;

import org.opendaylight.vtn.manager.internal.util.log.VTNLogLevel;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcErrorTag;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;
import org.opendaylight.vtn.manager.internal.util.vnode.VTenantIdentifier;

import org.opendaylight.vtn.manager.internal.TestBase;
import org.opendaylight.vtn.manager.internal.XmlDataType;
import org.opendaylight.vtn.manager.internal.XmlNode;
import org.opendaylight.vtn.manager.internal.XmlValueType;
import org.opendaylight.vtn.manager.internal.routing.xml.XmlPathMapTest;
import org.opendaylight.vtn.manager.internal.util.flow.filter.FlowFilterListTest;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.flow.filter.list.VtnFlowFilter;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.VtnInfo;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.vtn.info.VtenantConfig;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.vtn.info.VtnInputFilter;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.vtn.info.VtnInputFilterBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.vtn.info.VtnPathMapsBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.vtns.Vtn;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.vtns.VtnBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeName;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnErrorTag;

/**
 * JUnit test for {@link XmlVTenant}.
 */
public class XmlVTenantTest extends TestBase {
    /**
     * Root XML element name associated with {@link XmlVTenant} class.
     */
    public static final String  XML_ROOT = "vtn";

    /**
     * Return a list of {@link XmlDataType} instances that specifies XML node
     * types mapped to a {@link XmlVTenant} instance.
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
            new XmlValueType("idle-timeout",
                             Integer.class).add(name).prepend(parent),
            new XmlValueType("hard-timeout",
                             Integer.class).add(name).prepend(parent));

        String[] path = XmlDataType.addPath(name, parent);
        String[] p = XmlDataType.addPath("vbridges", path);
        dlist.addAll(XmlVBridgeTest.getXmlDataTypes("vbridge", p));

        p = XmlDataType.addPath("vterminals", path);
        dlist.addAll(XmlVTerminalTest.getXmlDataTypes("vterminal", p));

        p = XmlDataType.addPath("vtn-path-maps", path);
        dlist.addAll(XmlPathMapTest.getXmlDataTypes("vtn-path-map", p));

        dlist.addAll(FlowFilterListTest.
                     getXmlDataTypes("input-filters", path));
        return dlist;
    }

    /**
     * Test case for {@link XmlVTenant#XmlVTenant(VtnInfo)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testConstructor() throws Exception {
        // Test case for name and description.
        for (int i = 0; i < 5; i++) {
            String name = "vtn_" + i;
            String desc = (i == 0) ? null : "VTN " + i;
            VTenantConfig tconf = new VTenantConfig(name, desc);
            VtnInfo vtn = tconf.toVtn();
            XmlVTenant xvtn = new XmlVTenant(vtn);
            tconf.verify(xvtn, false);
        }

        // Test case for idle-timeout.
        Integer[] timeouts = {0, 1, 123, 4567, 65534, 65535};
        String name = "vtn";
        String desc = "VTN";
        for (Integer idle: timeouts) {
            VTenantConfig tconf = new VTenantConfig(name, desc).
                setIdleTimeout(idle).
                setHardTimeout(0);
            VtnInfo vtn = tconf.toVtn();
            XmlVTenant xvtn = new XmlVTenant(vtn);
            tconf.verify(xvtn, false);
        }

        // Test case for hard-timeout.
        for (Integer hard: timeouts) {
            VTenantConfig tconf = new VTenantConfig(name, desc).
                setIdleTimeout(0).
                setHardTimeout(hard);
            VtnInfo vtn = tconf.toVtn();
            XmlVTenant xvtn = new XmlVTenant(vtn);
            tconf.verify(xvtn, false);
        }
    }

    /**
     * Test case for {@link XmlVTenant#getIdentifier()}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testGetIdentifier() throws Exception {
        String[] names = {"vtn_1", "vtn_2", "vtenant_1234567"};
        for (String name: names) {
            VTenantConfig tconf = new VTenantConfig(name);
            VtnInfo vtn = tconf.toVtn();
            XmlVTenant xvtn = new XmlVTenant(vtn);
            tconf.verify(xvtn, false);
            VTenantIdentifier expected = VTenantIdentifier.create(name, false);
            VTenantIdentifier vtnId = xvtn.getIdentifier();
            assertEquals(expected, vtnId);

            // Identifier should be cached.
            for (int i = 0; i < 10; i++) {
                assertSame(vtnId, xvtn.getIdentifier());
            }
        }
    }

    /**
     * Test cast for {@link XmlVTenant#getBridges()}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testGetBridges() throws Exception {
        String name = "vtn";
        String desc = "VTN";
        Integer idle = 300;
        Integer hard = 600;
        VTenantConfig tconf = new VTenantConfig(name, desc).
            setIdleTimeout(idle).
            setHardTimeout(hard);

        Random rand = new Random(0xffff000012345L);
        int[] counts = {0, 1, 20};
        for (int count: counts) {
            VBridgeConfigList bconfList = new VBridgeConfigList(rand, count);
            tconf.setBridges(bconfList);
            Vtn vtn = tconf.toVtn();
            XmlVTenant xvtn = new XmlVTenant(vtn);
            tconf.verify(xvtn, false);
        }
    }

    /**
     * Test cast for {@link XmlVTenant#getTerminals()}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testGetTerminals() throws Exception {
        String name = "vtn";
        String desc = "VTN";
        Integer idle = 300;
        Integer hard = 600;
        VTenantConfig tconf = new VTenantConfig(name, desc).
            setIdleTimeout(idle).
            setHardTimeout(hard);

        Random rand = new Random(0x987612345abcL);
        int[] counts = {0, 1, 20};
        for (int count: counts) {
            VTerminalConfigList vtconfList =
                new VTerminalConfigList(rand, count);
            tconf.setTerminals(vtconfList);
            Vtn vtn = tconf.toVtn();
            XmlVTenant xvtn = new XmlVTenant(vtn);
            tconf.verify(xvtn, false);
        }
    }

    /**
     * Test cast for {@link XmlVTenant#getPathMaps()}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testGetPathMaps() throws Exception {
        String name = "vtn";
        String desc = "VTN";
        Integer idle = 300;
        Integer hard = 600;
        VTenantConfig tconf = new VTenantConfig(name, desc).
            setIdleTimeout(idle).
            setHardTimeout(hard);

        // Test case for null vtn-path-map list.
        Vtn vtn = new VtnBuilder(tconf.toVtn()).
            setVtnPathMaps(new VtnPathMapsBuilder().build()).
            build();
        XmlVTenant xvtn = new XmlVTenant(vtn);
        tconf.verify(xvtn, false);

        Random rand = new Random(0x19283746L);
        int[] counts = {0, 1, 20};
        for (int count: counts) {
            PathMapConfigList pmaps = new PathMapConfigList(rand, count);
            tconf.setPathMaps(pmaps);
            vtn = tconf.toVtn();
            xvtn = new XmlVTenant(vtn);
            tconf.verify(xvtn, false);
        }
    }

    /**
     * Test case for {@link XmlVTenant#getInputFilters()}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testGetInputFilters() throws Exception {
        String name = "vtn";
        VnodeName vname = new VnodeName(name);
        String desc = "VTN";
        Integer idle = 300;
        Integer hard = 600;
        VTenantConfig tconf = new VTenantConfig(name, desc).
            setIdleTimeout(idle).
            setHardTimeout(hard);
        VtenantConfig vtnc = tconf.toVtenantConfig();

        // Test case for null/empty flow filter list.
        List<List<VtnFlowFilter>> cases = new ArrayList<>();
        Collections.addAll(
            cases,
            null,
            Collections.<VtnFlowFilter>emptyList());
        for (List<VtnFlowFilter> vfilters: cases) {
            VtnInputFilter input = new VtnInputFilterBuilder().
                setVtnFlowFilter(vfilters).build();
            Vtn vtn = new VtnBuilder().
                setName(vname).
                setVtenantConfig(vtnc).
                setVtnInputFilter(input).
                build();
            XmlVTenant xvtn = new XmlVTenant(vtn);
            assertEquals(vname, xvtn.getName());
            assertEquals(desc, xvtn.getDescription());
            assertEquals(idle, VTenantConfig.getIdleTimeout(xvtn));
            assertEquals(hard, VTenantConfig.getHardTimeout(xvtn));
            assertEquals(null, xvtn.getBridges());
            assertEquals(null, xvtn.getTerminals());
            assertEquals(null, xvtn.getPathMaps());
            assertEquals(null, xvtn.getInputFilters());
        }

        // Input filter test.
        Random rand = new Random(0x5647382910L);
        XmlFlowActionList xactions = new XmlFlowActionList().addAll(rand);
        XmlFlowFilter xpass = new XmlPassFilter(111, "cond_pass").
            setFlowActions(xactions);
        XmlFlowFilter xdrop = new XmlDropFilter(222, "cond_drop");
        XmlFlowFilterList xfilters = new XmlFlowFilterList().
            add(xpass).
            add(xdrop).
            addAll(rand);
        Vtn vtn = tconf.setInputFilters(xfilters).toVtn();
        XmlVTenant xvtn = new XmlVTenant(vtn);
        tconf.verify(xvtn, false);
    }

    /**
     * Test case for {@link XmlVTenant#equals(Object)} and
     * {@link XmlVTenant#hashCode()}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testEquals() throws Exception {
        Set<Object> set = new HashSet<>();

        String[] names = {
            "vtn_1", "vtn_2", "vtenant_3",
        };
        String[] descs = {
            null, "desc 1", "desciption 2",
        };
        int[][] timeouts = {
            new int[]{0, 0},
            new int[]{0, 65535},
            new int[]{65535, 0},
            new int[]{1000, 1001},
        };
        int count = 0;
        for (String name1: names) {
            String name2 = new String(name1);
            for (String desc1: descs) {
                String desc2 = (desc1 == null) ? null : new String(desc1);
                for (int[] timeout: timeouts) {
                    Integer idle1 = timeout[0];
                    Integer idle2 = timeout[0];
                    Integer hard1 = timeout[1];
                    Integer hard2 = timeout[1];
                    VTenantConfig tconf1 = new VTenantConfig(name1, desc1).
                        setIdleTimeout(idle1).
                        setHardTimeout(hard1);
                    VTenantConfig tconf2 = new VTenantConfig(name2, desc2).
                        setIdleTimeout(idle2).
                        setHardTimeout(hard2);
                    Vtn vtn1 = tconf1.toVtn();
                    Vtn vtn2 = tconf2.toVtn();
                    XmlVTenant xvtn1 = new XmlVTenant(vtn1);
                    XmlVTenant xvtn2 = new XmlVTenant(vtn2);
                    testEquals(set, xvtn1, xvtn2);
                    count++;
                }
            }
        }

        String name1 = names[0];
        String desc1 = descs[1];
        String name2 = new String(name1);
        String desc2 = new String(desc1);
        Integer idle1 = 300;
        Integer idle2 = 300;
        Integer hard1 = 600;
        Integer hard2 = 600;
        VTenantConfig tconf1 = new VTenantConfig(name1, desc1).
            setIdleTimeout(idle1).
            setHardTimeout(hard1);
        VTenantConfig tconf2 = new VTenantConfig(name2, desc2).
            setIdleTimeout(idle2).
            setHardTimeout(hard2);

        Random rand = new Random(0xdeadbeefL);
        VBridgeConfigList[] bconfs = {
            new VBridgeConfigList(rand, 1),
            new VBridgeConfigList(rand, 10),
        };
        for (VBridgeConfigList bconfList1: bconfs) {
            VBridgeConfigList bconfList2 = bconfList1.clone();
            tconf1.setBridges(bconfList1);
            tconf2.setBridges(bconfList2);
            Vtn vtn1 = tconf1.toVtn();
            Vtn vtn2 = tconf2.toVtn();
            XmlVTenant xvtn1 = new XmlVTenant(vtn1);
            XmlVTenant xvtn2 = new XmlVTenant(vtn2);
            testEquals(set, xvtn1, xvtn2);
            count++;
        }

        VTerminalConfigList[] vtconfs = {
            new VTerminalConfigList(rand, 1),
            new VTerminalConfigList(rand, 10),
        };
        for (VTerminalConfigList vtconfList1: vtconfs) {
            VTerminalConfigList vtconfList2 = vtconfList1.clone();
            tconf1.setTerminals(vtconfList1);
            tconf2.setTerminals(vtconfList2);
            Vtn vtn1 = tconf1.toVtn();
            Vtn vtn2 = tconf2.toVtn();
            XmlVTenant xvtn1 = new XmlVTenant(vtn1);
            XmlVTenant xvtn2 = new XmlVTenant(vtn2);
            testEquals(set, xvtn1, xvtn2);
            count++;
        }

        PathMapConfigList[] pmaps = {
            new PathMapConfigList(rand, 1),
            new PathMapConfigList(rand, 20),
        };
        for (PathMapConfigList pmaps1: pmaps) {
            PathMapConfigList pmaps2 = pmaps1.clone();
            tconf1.setPathMaps(pmaps1);
            tconf2.setPathMaps(pmaps2);
            Vtn vtn1 = tconf1.toVtn();
            Vtn vtn2 = tconf2.toVtn();
            XmlVTenant xvtn1 = new XmlVTenant(vtn1);
            XmlVTenant xvtn2 = new XmlVTenant(vtn2);
            testEquals(set, xvtn1, xvtn2);
            count++;
        }

        XmlPassFilter xpass = new XmlPassFilter(12345, "cond1");
        XmlDropFilter xdrop = new XmlDropFilter(999, "cond2");
        XmlFlowFilterList[] xfilters = {
            new XmlFlowFilterList().add(xpass).add(xdrop),
            new XmlFlowFilterList().addRandom(rand, 10),
        };
        for (XmlFlowFilterList xinput1: xfilters) {
            XmlFlowFilterList xinput2 = xinput1.clone();
            tconf1.setInputFilters(xinput1);
            tconf2.setInputFilters(xinput2);
            Vtn vtn1 = tconf1.toVtn();
            Vtn vtn2 = tconf2.toVtn();
            XmlVTenant xvtn1 = new XmlVTenant(vtn1);
            XmlVTenant xvtn2 = new XmlVTenant(vtn2);
            testEquals(set, xvtn1, xvtn2);
            count++;
        }

        assertEquals(count, set.size());
    }

    /**
     * Test case for {@link XmlVTenant#toVtnBuilder(XmlLogger, String)} and
     * XML binding.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testJAXB() throws Exception {
        Class<XmlVTenant> type = XmlVTenant.class;
        Unmarshaller um = createUnmarshaller(type);

        String[] names = {"vtn_1", "vtn_2"};
        String[] descs = {null, "desc 1", "VTN 2"};
        Integer[][] timeouts = {
            new Integer[]{0, 0},
            new Integer[]{0, 65535},
            new Integer[]{65535, 0},
            new Integer[]{1000, 1001},
        };

        for (String name: names) {
            for (String desc: descs) {
                for (Integer[] timeout: timeouts) {
                    VTenantConfig tconf = new VTenantConfig(name, desc).
                        setIdleTimeout(timeout[0]).
                        setHardTimeout(timeout[1]);
                    XmlNode xnode = tconf.toXmlNode();
                    XmlVTenant xvtn = unmarshal(um, xnode.toString(), type);
                    tconf.verify(xvtn, true);
                    jaxbTest(xvtn, type, XML_ROOT);
                    tconf.testToVtnBuilder(xvtn);
                }
            }
        }

        // Test for vbridge.
        Random rand = new Random(0xa0b0c0d0e0f0L);
        String name = "vtn";
        String desc = "Virtual Tenant Network";
        Integer idle = 300;
        Integer hard = 600;
        VTenantConfig tconf = new VTenantConfig(name, desc).
            setIdleTimeout(idle).
            setHardTimeout(hard);
        VBridgeConfigList bconfList = new VBridgeConfigList(rand, 10);
        XmlNode xnode = tconf.setBridges(bconfList).toXmlNode();
        XmlVTenant xvtn = unmarshal(um, xnode.toString(), type);
        tconf.verify(xvtn, true);
        jaxbTest(xvtn, type, XML_ROOT);
        tconf.testToVtnBuilder(xvtn);

        // Test for vterminal.
        VTerminalConfigList vtconfList = new VTerminalConfigList(rand, 10);
        xnode = tconf.setTerminals(vtconfList).toXmlNode();
        xvtn = unmarshal(um, xnode.toString(), type);
        tconf.verify(xvtn, true);
        jaxbTest(xvtn, type, XML_ROOT);
        tconf.testToVtnBuilder(xvtn);

        // Test for vtn-path-map.
        PathMapConfigList pmaps = new PathMapConfigList(rand, 10);
        xnode = tconf.setPathMaps(pmaps).toXmlNode();
        xvtn = unmarshal(um, xnode.toString(), type);
        tconf.verify(xvtn, true);
        jaxbTest(xvtn, type, XML_ROOT);
        tconf.testToVtnBuilder(xvtn);

        // Test for input-filters.
        XmlDropFilter xdrop = new XmlDropFilter(12345, "drop1");
        XmlPassFilter xpass = new XmlPassFilter(6789, "pass1");
        XmlFlowFilterList xinput = new XmlFlowFilterList().
            add(xdrop).
            add(xpass).
            addAll(rand);
        xnode = tconf.setInputFilters(xinput).toXmlNode();
        xvtn = unmarshal(um, xnode.toString(), type);
        tconf.verify(xvtn, true);
        jaxbTest(xvtn, type, XML_ROOT);
        tconf.testToVtnBuilder(xvtn);

        // Broken input filter should be ignored.
        XmlFlowFilterList xbadIn = new XmlFlowFilterList().
            add(new XmlDropFilter(333, null));
        xnode = tconf.setInputFilters(xbadIn).toXmlNode();
        xvtn = unmarshal(um, xnode.toString(), type);
        assertEquals(name, xvtn.getName().getValue());
        assertEquals(desc, xvtn.getDescription());
        assertEquals(idle, VTenantConfig.getIdleTimeout(xvtn));
        assertEquals(hard, VTenantConfig.getHardTimeout(xvtn));
        bconfList.verify(xvtn.getBridges(), true);
        vtconfList.verify(xvtn.getTerminals(), true);
        pmaps.verify(xvtn.getPathMaps(), true);
        jaxbTest(xvtn, type, XML_ROOT);

        XmlLogger xlogger = mock(XmlLogger.class);
        VtnBuilder builder = xvtn.toVtnBuilder(xlogger, name);
        assertEquals(name, builder.getName().getValue());
        VtenantConfig vtnc = builder.getVtenantConfig();
        assertEquals(desc, vtnc.getDescription());
        assertEquals(idle, vtnc.getIdleTimeout());
        assertEquals(hard, vtnc.getHardTimeout());
        assertEquals(null, builder.getVbridge());
        assertEquals(null, builder.getVterminal());
        assertEquals(null, builder.getVtnPathMaps());
        assertEquals(null, builder.getVtnInputFilter());

        VTenantIdentifier vtnId = VTenantIdentifier.create(name, false);
        String inMsg = "IN";
        String badfmt = "%s: %s: Ignore broken flow filters.";
        verify(xlogger).
            log(eq(VTNLogLevel.WARN), any(RpcException.class), eq(badfmt),
                eq(vtnId), eq(inMsg));
        verifyNoMoreInteractions(xlogger);

        // Ensure that broken values in XML can be detected.
        jaxbErrorTest(um, type, getXmlDataTypes(XML_ROOT));

        // Node name is missing.
        xnode = new XmlNode(XML_ROOT).
            add(new XmlNode("description", desc)).
            add(new XmlNode("idle-timeout", idle)).
            add(new XmlNode("hard-timeout", hard));
        xvtn = unmarshal(um, xnode.toString(), type);
        xlogger = mock(XmlLogger.class);
        RpcErrorTag etag = RpcErrorTag.MISSING_ELEMENT;
        VtnErrorTag vtag = VtnErrorTag.BADREQUEST;
        String msg = "VTN name cannot be null";
        try {
            xvtn.toVtnBuilder(xlogger, name);
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            assertEquals(vtag, e.getVtnErrorTag());
            assertEquals(msg, e.getMessage());
        }
        verifyNoMoreInteractions(xlogger);

        // idle-timeout is missing.
        xnode = new XmlNode(XML_ROOT).
            add(new XmlNode("name", name)).
            add(new XmlNode("description", desc)).
            add(new XmlNode("hard-timeout", hard));
        xvtn = unmarshal(um, xnode.toString(), type);
        msg = "idle-timeout cannot be null";
        try {
            xvtn.toVtnBuilder(xlogger, name);
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            assertEquals(vtag, e.getVtnErrorTag());
            assertEquals(msg, e.getMessage());
        }
        verifyNoMoreInteractions(xlogger);

        // hard-timeout is missing.
        xnode = new XmlNode(XML_ROOT).
            add(new XmlNode("name", name)).
            add(new XmlNode("description", desc)).
            add(new XmlNode("idle-timeout", idle));
        xvtn = unmarshal(um, xnode.toString(), type);
        msg = "hard-timeout cannot be null";
        try {
            xvtn.toVtnBuilder(xlogger, name);
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            assertEquals(vtag, e.getVtnErrorTag());
            assertEquals(msg, e.getMessage());
        }
        verifyNoMoreInteractions(xlogger);

        // Unexpected VTN name.
        String badName = "vtn1";
        xnode = new XmlNode(XML_ROOT).
            add(new XmlNode("name", name)).
            add(new XmlNode("description", desc)).
            add(new XmlNode("idle-timeout", idle)).
            add(new XmlNode("hard-timeout", hard));
        xvtn = unmarshal(um, xnode.toString(), type);
        etag = RpcErrorTag.BAD_ELEMENT;
        msg = badName + ": Unexpected VTN name: " + name;
        try {
            xvtn.toVtnBuilder(xlogger, badName);
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            assertEquals(vtag, e.getVtnErrorTag());
            assertEquals(msg, e.getMessage());
        }
        verifyNoMoreInteractions(xlogger);

        // idle-timeout is greater than or equal to idle-timeout.
        idle = 100;
        Integer[] hards = {1, 99, 100};
        for (Integer value: hards) {
            xnode = new XmlNode(XML_ROOT).
                add(new XmlNode("name", name)).
                add(new XmlNode("description", desc)).
                add(new XmlNode("idle-timeout", idle)).
                add(new XmlNode("hard-timeout", value));
            xvtn = unmarshal(um, xnode.toString(), type);
            msg = "idle-timeout must be less than hard-timeout.";
            try {
                xvtn.toVtnBuilder(xlogger, name);
                unexpected();
            } catch (RpcException e) {
                assertEquals(etag, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                assertEquals(msg, e.getMessage());
            }
            verifyNoMoreInteractions(xlogger);
        }
    }
}
