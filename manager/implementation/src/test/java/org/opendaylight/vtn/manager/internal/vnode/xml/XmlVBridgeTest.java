/*
 * Copyright (c) 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.vnode.xml;

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

import org.opendaylight.vtn.manager.internal.util.inventory.MacVlan;
import org.opendaylight.vtn.manager.internal.util.inventory.SalNode;
import org.opendaylight.vtn.manager.internal.util.log.VTNLogLevel;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcErrorTag;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;
import org.opendaylight.vtn.manager.internal.util.vnode.BridgeIdentifier;
import org.opendaylight.vtn.manager.internal.util.vnode.VBridgeIdentifier;
import org.opendaylight.vtn.manager.internal.util.vnode.VTerminalIfIdentifier;

import org.opendaylight.vtn.manager.internal.TestBase;
import org.opendaylight.vtn.manager.internal.XmlDataType;
import org.opendaylight.vtn.manager.internal.XmlNode;
import org.opendaylight.vtn.manager.internal.XmlValueType;
import org.opendaylight.vtn.manager.internal.util.flow.filter.FlowFilterListTest;
import org.opendaylight.vtn.manager.internal.util.vnode.VTNMacMapConfigTest;
import org.opendaylight.vtn.manager.internal.util.vnode.VTNVlanMapConfigTest;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.flow.filter.list.VtnFlowFilter;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeName;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnErrorTag;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.VtnVbridgeInfo;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.vbridge.info.VbridgeConfig;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.vbridge.info.VbridgeInputFilter;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.vbridge.info.VbridgeInputFilterBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.vbridge.info.VbridgeOutputFilter;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.vbridge.info.VbridgeOutputFilterBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.vbridge.list.Vbridge;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.vbridge.list.VbridgeBuilder;

/**
 * JUnit test for {@link XmlVBridge}.
 */
public class XmlVBridgeTest extends TestBase {
    /**
     * Root XML element name associated with {@link XmlVBridge} class.
     */
    public static final String  XML_ROOT = "vbridge";

    /**
     * Return a list of {@link XmlDataType} instances that specifies XML node
     * types mapped to a {@link XmlVBridge} instance.
     *
     * @param name    The name of the target node.
     * @param parent  Path to the parent node.
     * @return  A list of {@link XmlDataType} instances.
     */
    public static List<XmlDataType> getXmlDataTypes(String name,
                                                    String ... parent) {
        List<XmlDataType> dlist = new ArrayList<>();
        dlist.add(new XmlValueType("age-interval", Integer.class).
                  add(name).prepend(parent));

        String[] path = XmlDataType.addPath(name, parent);
        String[] p = XmlDataType.addPath("vinterfaces", path);
        dlist.addAll(XmlVInterfaceTest.getXmlDataTypes("vinterface", p));

        p = XmlDataType.addPath("vlan-maps", path);
        dlist.addAll(VTNVlanMapConfigTest.getXmlDataTypes("vlan-map", p));

        dlist.addAll(VTNMacMapConfigTest.getXmlDataTypes("mac-map", path));
        dlist.addAll(FlowFilterListTest.
                     getXmlDataTypes("input-filters", path));
        dlist.addAll(FlowFilterListTest.
                     getXmlDataTypes("output-filters", path));
        return dlist;
    }

    /**
     * Test case for {@link XmlVBridge#XmlVBridge(VtnVbridgeInfo)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testConstructor() throws Exception {
        // Test case for name, description, and age-interval.
        int[] intervals = {10, 333, 1000000};
        for (int i = 0; i < 5; i++) {
            String name = "vbr_" + i;
            String desc = (i == 0) ? null : "vBridge " + i;
            for (int age: intervals) {
                VBridgeConfig bconf = new VBridgeConfig(name, desc).
                    setAgeInterval(age);
                VtnVbridgeInfo vbr = bconf.toVbridge();
                XmlVBridge xvbr = new XmlVBridge(vbr);
                bconf.verify(xvbr, false);
            }
        }
    }

    /**
     * Test case for {@link XmlAbstractBridge#getInterfaces()}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testGetInterfaces() throws Exception {
        String name = "vbr";
        String desc = "vBridge";
        int age = 300;
        VBridgeConfig bconf = new VBridgeConfig(name, desc).
            setAgeInterval(age);

        Random rand = new Random(0xabcdef12345L);
        int[] counts = {0, 1, 20};
        for (int count: counts) {
            VInterfaceConfigList iconfList =
                new VInterfaceConfigList(rand, count);
            bconf.setInterfaces(iconfList);
            Vbridge vbr = bconf.toVbridge();
            XmlVBridge xvbr = new XmlVBridge(vbr);
            bconf.verify(xvbr, false);
        }
    }

    /**
     * Test case for {@link XmlVBridge#getVlanMaps()}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testGetVlanMaps() throws Exception {
        String name = "vbr";
        String desc = "vBridge";
        int age = 300;
        VBridgeConfig bconf = new VBridgeConfig(name, desc).
            setAgeInterval(age);

        Random rand = new Random(223620679L);
        int[] counts = {0, 1, 20};
        for (int count: counts) {
            XmlVlanMapConfigList xvmaps =
                new XmlVlanMapConfigList(rand, count);
            Vbridge vbr = bconf.setVlanMaps(xvmaps).toVbridge();
            XmlVBridge xvbr = new XmlVBridge(vbr);
            bconf.verify(xvbr, false);
        }
    }

    /**
     * Test case for {@link XmlVBridge#getMacMap()}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testGetMacMap() throws Exception {
        String name = "vbr";
        String desc = "vBridge";
        int age = 300;
        VBridgeConfig bconf = new VBridgeConfig(name, desc).
            setAgeInterval(age);

        Random rand = new Random(17320508L);
        for (int i = 0; i < 5; i++) {
            XmlMacMapConfig xmmc = new XmlMacMapConfig(rand);
            Vbridge vbr = bconf.setMacMap(xmmc).toVbridge();
            XmlVBridge xvbr = new XmlVBridge(vbr);
            bconf.verify(xvbr, false);
        }
    }

    /**
     * Test case for {@link XmlVBridge#getInputFilters()} and
     * {@link XmlVBridge#getOutputFilters()}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testGetFlowFilters() throws Exception {
        String name = "vbr";
        VnodeName vname = new VnodeName(name);
        String desc = "vBridge";
        int age = 300;
        VBridgeConfig bconf = new VBridgeConfig(name, desc).
            setAgeInterval(age);
        VbridgeConfig vbrc = bconf.toVbridgeConfig();

        // Test case for null/empty flow filter list.
        List<List<VtnFlowFilter>> cases = new ArrayList<>();
        Collections.addAll(
            cases,
            null,
            Collections.<VtnFlowFilter>emptyList());
        for (List<VtnFlowFilter> vfilters: cases) {
            VbridgeInputFilter input = new VbridgeInputFilterBuilder().
                setVtnFlowFilter(vfilters).build();
            VbridgeOutputFilter output = new VbridgeOutputFilterBuilder().
                setVtnFlowFilter(vfilters).build();
            Vbridge vbr = new VbridgeBuilder().
                setName(vname).
                setVbridgeConfig(vbrc).
                setVbridgeInputFilter(input).
                setVbridgeOutputFilter(output).
                build();
            XmlVBridge xvbr = new XmlVBridge(vbr);
            assertEquals(vname, xvbr.getName());
            assertEquals(desc, xvbr.getDescription());
            assertEquals(age, VBridgeConfig.getAgeInterval(xvbr));
            assertEquals(null, xvbr.getInterfaces());
            assertEquals(null, xvbr.getVlanMaps());
            assertEquals(null, xvbr.getMacMap());
            assertEquals(null, xvbr.getInputFilters());
            assertEquals(null, xvbr.getOutputFilters());
        }

        // Input filter test.
        Random rand = new Random(0x1234abcd5678L);
        XmlFlowActionList xactions = new XmlFlowActionList().addAll(rand);
        XmlFlowFilter xpass = new XmlPassFilter(1, "cond_pass").
            setFlowActions(xactions);
        XmlFlowFilterList xfilters = new XmlFlowFilterList().
            add(xpass).
            addAll(rand);
        Vbridge vbr = bconf.setInputFilters(xfilters).toVbridge();
        XmlVBridge xvbr = new XmlVBridge(vbr);
        bconf.verify(xvbr, false);

        // Output filter test.
        xactions = new XmlFlowActionList().addAll(rand);
        VTerminalIfIdentifier dest = new VTerminalIfIdentifier(
            null, new VnodeName("dest_vterm"), new VnodeName("dest_if"));
        XmlFlowFilter xredirect =
            new XmlRedirectFilter(1, "cond", dest, true).
            setFlowActions(xactions);
        xfilters = new XmlFlowFilterList().
            add(xredirect).
            addAll(rand);
        vbr = bconf.setOutputFilters(xfilters).toVbridge();
        xvbr = new XmlVBridge(vbr);
        bconf.verify(xvbr, false);
    }

    /**
     * Test case for {@link XmlVBridge#equals(Object)} and
     * {@link XmlVBridge#hashCode()}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testEquals() throws Exception {
        Set<Object> set = new HashSet<>();

        String[] names = {
            "vbr_1", "vbr_2", "vbridge_3",
        };
        String[] descs = {
            null, "desc 1", "desciption 2",
        };
        int[] intervals = {10, 4567, 1000000};
        int count = 0;
        for (String name1: names) {
            String name2 = new String(name1);
            for (String desc1: descs) {
                String desc2 = (desc1 == null) ? null : new String(desc1);
                for (int age: intervals) {
                    VBridgeConfig bconf1 = new VBridgeConfig(name1, desc1).
                        setAgeInterval(age);
                    VBridgeConfig bconf2 = new VBridgeConfig(name2, desc2).
                        setAgeInterval(age);
                    Vbridge vbr1 = bconf1.toVbridge();
                    Vbridge vbr2 = bconf2.toVbridge();
                    XmlVBridge xvbr1 = new XmlVBridge(vbr1);
                    XmlVBridge xvbr2 = new XmlVBridge(vbr2);
                    testEquals(set, xvbr1, xvbr2);
                    count++;
                }
            }
        }

        String name1 = names[0];
        String desc1 = descs[1];
        String name2 = new String(name1);
        String desc2 = new String(desc1);
        int age = intervals[1];
        VBridgeConfig bconf1 = new VBridgeConfig(name1, desc1).
            setAgeInterval(age);
        VBridgeConfig bconf2 = new VBridgeConfig(name2, desc2).
            setAgeInterval(age);

        XmlVlanMapConfig xvmc1 = new XmlVlanMapConfig();
        XmlVlanMapConfig xvmc2 = new XmlVlanMapConfig();
        XmlVlanMapConfigList xvmaps1 = new XmlVlanMapConfigList().
            add(xvmc1);
        XmlVlanMapConfigList xvmaps2 = new XmlVlanMapConfigList().
            add(xvmc2);
        Vbridge vbr1 = bconf1.setVlanMaps(xvmaps1).toVbridge();
        Vbridge vbr2 = bconf2.setVlanMaps(xvmaps2).toVbridge();
        XmlVBridge xvbr1 = new XmlVBridge(vbr1);
        XmlVBridge xvbr2 = new XmlVBridge(vbr2);
        testEquals(set, xvbr1, xvbr2);
        count++;

        SalNode snode = new SalNode(12345L);
        int vid = 45;
        xvmc1 = new XmlVlanMapConfig(snode, vid);
        xvmc2 = new XmlVlanMapConfig(snode, vid);
        xvmaps1.add(xvmc1);
        xvmaps2.add(xvmc2);
        vbr1 = bconf1.toVbridge();
        vbr2 = bconf2.toVbridge();
        xvbr1 = new XmlVBridge(vbr1);
        xvbr2 = new XmlVBridge(vbr2);
        testEquals(set, xvbr1, xvbr2);
        count++;

        XmlMacMapConfig xmmc1 = new XmlMacMapConfig().
            addAllowedHosts(new MacVlan(0x001122334455L, 0));
        XmlMacMapConfig xmmc2 = new XmlMacMapConfig().
            addAllowedHosts(new MacVlan(0x001122334455L, 0));
        vbr1 = bconf1.setMacMap(xmmc1).toVbridge();
        vbr2 = bconf2.setMacMap(xmmc2).toVbridge();
        xvbr1 = new XmlVBridge(vbr1);
        xvbr2 = new XmlVBridge(vbr2);
        testEquals(set, xvbr1, xvbr2);
        count++;

        Random rand = new Random(666666666L);
        VInterfaceConfigList iconfList = new VInterfaceConfigList(rand, 20);
        VInterfaceConfigList ilist1 = new VInterfaceConfigList();
        VInterfaceConfigList ilist2 = new VInterfaceConfigList();
        bconf1.setInterfaces(ilist1);
        bconf2.setInterfaces(ilist2);
        for (VInterfaceConfig iconf: iconfList.getAll()) {
            ilist1.add(iconf);
            ilist2.add(iconf);
            vbr1 = bconf1.toVbridge();
            vbr2 = bconf2.toVbridge();
            xvbr1 = new XmlVBridge(vbr1);
            xvbr2 = new XmlVBridge(vbr2);
            testEquals(set, xvbr1, xvbr2);
            count++;
        }

        XmlPassFilter xpass = new XmlPassFilter(12345, "cond1");
        XmlFlowFilterList xinput1 = new XmlFlowFilterList().
            add(xpass).
            addRandom(rand);
        XmlFlowFilterList xinput2 = xinput1.clone();
        vbr1 = bconf1.setInputFilters(xinput1).toVbridge();
        vbr2 = bconf2.setInputFilters(xinput2).toVbridge();
        xvbr1 = new XmlVBridge(vbr1);
        xvbr2 = new XmlVBridge(vbr2);
        testEquals(set, xvbr1, xvbr2);
        count++;

        XmlDropFilter xdrop = new XmlDropFilter(999, "cond2");
        XmlFlowFilterList xoutput1 = new XmlFlowFilterList().
            add(xdrop).
            addRandom(rand);
        XmlFlowFilterList xoutput2 = xoutput1.clone();
        vbr1 = bconf1.setOutputFilters(xoutput1).toVbridge();
        vbr2 = bconf2.setOutputFilters(xoutput2).toVbridge();
        xvbr1 = new XmlVBridge(vbr1);
        xvbr2 = new XmlVBridge(vbr2);
        testEquals(set, xvbr1, xvbr2);
        count++;

        assertEquals(count, set.size());
    }

    /**
     * Test case for {@link XmlVBridge#toVbridgeBuilder(XmlLogger,BridgeIdentifier)} and XML binding.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testJAXB() throws Exception {
        Class<XmlVBridge> type = XmlVBridge.class;
        Unmarshaller um = createUnmarshaller(type);

        String[] names = {"vbr_1", "vbridge_1"};
        String[] descs = {null, "vbr desc", "vBridge 1"};
        int[] intervals = {10, 556677, 1000000};
        String tname = "vtn";
        for (String name: names) {
            BridgeIdentifier<Vbridge> vbrId =
                VBridgeIdentifier.create(tname, name, false);
            for (String desc: descs) {
                for (int age: intervals) {
                    VBridgeConfig bconf = new VBridgeConfig(name, desc).
                        setAgeInterval(age);
                    XmlNode xnode = bconf.toXmlNode();
                    XmlVBridge xvbr = unmarshal(um, xnode.toString(), type);
                    bconf.verify(xvbr, true);
                    jaxbTest(xvbr, type, XML_ROOT);
                    bconf.testToVbridgeBuilder(xvbr);
                }
            }
        }

        // Test for vinterface.
        Random rand = new Random(777777777L);
        String name = "vbr";
        String desc = "vBridge";
        int age = 60000;
        VBridgeConfig bconf = new VBridgeConfig(name, desc).
            setAgeInterval(age);
        VInterfaceConfigList iconfList = new VInterfaceConfigList(rand, 10);
        bconf.setInterfaces(iconfList);
        XmlNode xnode = bconf.toXmlNode();
        XmlVBridge xvbr = unmarshal(um, xnode.toString(), type);
        bconf.verify(xvbr, true);
        jaxbTest(xvbr, type, XML_ROOT);
        bconf.testToVbridgeBuilder(xvbr);

        // Test for vlan-map.
        XmlVlanMapConfigList xvmaps = new XmlVlanMapConfigList(rand, 20);
        xnode = bconf.setVlanMaps(xvmaps).toXmlNode();
        xvbr = unmarshal(um, xnode.toString(), type);
        bconf.verify(xvbr, true);
        jaxbTest(xvbr, type, XML_ROOT);
        bconf.testToVbridgeBuilder(xvbr);

        // Test for mac-map.
        XmlMacMapConfig xmmc = new XmlMacMapConfig().
            addAllowedHosts(new MacVlan(0x001122334455L, 0),
                            new MacVlan(0xa0b0c0d0e0f0L, 4095),
                            new MacVlan(MacVlan.UNDEFINED, 33)).
            addDeniedHosts(new MacVlan(0x00aabbccddeeL, 33),
                           new MacVlan(0xf0f1f2f3f4f5L, 999));
        xnode = bconf.setMacMap(xmmc).toXmlNode();
        xvbr = unmarshal(um, xnode.toString(), type);
        bconf.verify(xvbr, true);
        jaxbTest(xvbr, type, XML_ROOT);
        bconf.testToVbridgeBuilder(xvbr);

        // Test for input-filters.
        XmlDropFilter xdrop = new XmlDropFilter(111, "drop1");
        XmlPassFilter xpass = new XmlPassFilter(222, "pass1");
        XmlFlowFilterList xinput = new XmlFlowFilterList().
            add(xdrop).
            add(xpass).
            addAll(rand);
        xnode = bconf.setInputFilters(xinput).toXmlNode();
        xvbr = unmarshal(um, xnode.toString(), type);
        bconf.verify(xvbr, true);
        jaxbTest(xvbr, type, XML_ROOT);
        bconf.testToVbridgeBuilder(xvbr);

        // Test for output-filters.
        xdrop = new XmlDropFilter(65534, "drop2");
        xpass = new XmlPassFilter(65535, "pass2");
        XmlFlowFilterList xoutput = new XmlFlowFilterList().
            add(xdrop).
            add(xpass).
            addAll(rand);
        xnode = bconf.setOutputFilters(xoutput).toXmlNode();
        System.out.printf("xml=%s\n", xnode);
        xvbr = unmarshal(um, xnode.toString(), type);
        bconf.verify(xvbr, true);
        jaxbTest(xvbr, type, XML_ROOT);
        bconf.testToVbridgeBuilder(xvbr);

        // Broken input filter should be ignored.
        XmlFlowFilterList xbadIn = new XmlFlowFilterList().
            add(new XmlDropFilter(12345, null));
        xnode = bconf.setInputFilters(xbadIn).toXmlNode();
        xvbr = unmarshal(um, xnode.toString(), type);
        assertEquals(name, xvbr.getName().getValue());
        assertEquals(desc, xvbr.getDescription());
        assertEquals(age, VBridgeConfig.getAgeInterval(xvbr));
        iconfList.verify(xvbr.getInterfaces(), true);
        xvmaps.verify(xvbr.getVlanMaps());
        xmmc.verify(xvbr.getMacMap());
        xoutput.verify(xvbr.getOutputFilters(), true);
        jaxbTest(xvbr, type, XML_ROOT);

        XmlLogger xlogger = mock(XmlLogger.class);
        BridgeIdentifier<Vbridge> vbrId =
            VBridgeIdentifier.create("vtn", name, false);
        VbridgeBuilder builder = xvbr.toVbridgeBuilder(xlogger, vbrId);
        assertEquals(name, builder.getName().getValue());
        VbridgeConfig vbrc = builder.getVbridgeConfig();
        assertEquals(desc, vbrc.getDescription());
        assertEquals(age, vbrc.getAgeInterval().intValue());
        assertEquals(null, builder.getBridgeStatus());
        assertEquals(null, builder.getVinterface());
        assertEquals(null, builder.getVlanMap());
        assertEquals(null, builder.getMacMap());
        assertEquals(null, builder.getVbridgeInputFilter());
        xoutput.verify(builder.getVbridgeOutputFilter());

        String inMsg = "IN";
        String outMsg = "OUT";
        String logfmt = "{}: {} flow filters have been loaded.";
        String badfmt = "%s: %s: Ignore broken flow filters.";
        verify(xlogger).
            log(eq(VTNLogLevel.WARN), any(RpcException.class), eq(badfmt),
                eq(vbrId), eq(inMsg));
        verify(xlogger).log(VTNLogLevel.DEBUG, logfmt, vbrId, outMsg);
        verifyNoMoreInteractions(xlogger);

        // Broken output filter should be ignored.
        XmlFlowFilterList xbadOut = new XmlFlowFilterList().
            add(new XmlPassFilter(9999, null));
        xnode = bconf.setInputFilters(xinput).
            setOutputFilters(xbadOut).
            toXmlNode();
        xvbr = unmarshal(um, xnode.toString(), type);
        assertEquals(name, xvbr.getName().getValue());
        assertEquals(desc, xvbr.getDescription());
        assertEquals(age, VBridgeConfig.getAgeInterval(xvbr));
        iconfList.verify(xvbr.getInterfaces(), true);
        xvmaps.verify(xvbr.getVlanMaps());
        xmmc.verify(xvbr.getMacMap());
        xinput.verify(xvbr.getInputFilters(), true);
        jaxbTest(xvbr, type, XML_ROOT);

        xlogger = mock(XmlLogger.class);
        builder = xvbr.toVbridgeBuilder(xlogger, vbrId);
        assertEquals(name, builder.getName().getValue());
        vbrc = builder.getVbridgeConfig();
        assertEquals(desc, vbrc.getDescription());
        assertEquals(age, vbrc.getAgeInterval().intValue());
        assertEquals(null, builder.getBridgeStatus());
        assertEquals(null, builder.getVinterface());
        assertEquals(null, builder.getVlanMap());
        assertEquals(null, builder.getMacMap());
        assertEquals(null, builder.getVbridgeOutputFilter());
        xinput.verify(builder.getVbridgeInputFilter());

        verify(xlogger).
            log(eq(VTNLogLevel.WARN), any(RpcException.class), eq(badfmt),
                eq(vbrId), eq(outMsg));
        verify(xlogger).log(VTNLogLevel.DEBUG, logfmt, vbrId, inMsg);
        verifyNoMoreInteractions(xlogger);

        // Ensure that broken values in XML can be detected.
        jaxbErrorTest(um, type, getXmlDataTypes(XML_ROOT));

        // Node name is missing.
        xnode = new XmlNode(XML_ROOT).
            add(new XmlNode("description", desc)).
            add(new XmlNode("age-interval", age));
        xvbr = unmarshal(um, xnode.toString(), type);
        xlogger = mock(XmlLogger.class);
        RpcErrorTag etag = RpcErrorTag.MISSING_ELEMENT;
        VtnErrorTag vtag = VtnErrorTag.BADREQUEST;
        String msg = "vBridge name cannot be null";
        try {
            xvbr.toVbridgeBuilder(xlogger, vbrId);
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            assertEquals(vtag, e.getVtnErrorTag());
            assertEquals(msg, e.getMessage());
        }
        verifyNoMoreInteractions(xlogger);

        // age-interval is missing.
        etag = RpcErrorTag.BAD_ELEMENT;
        msg = vbrId.toString() + ": age-interval is missing";
        xnode = new XmlNode(XML_ROOT).
            add(new XmlNode("name", name)).
            add(new XmlNode("description", desc));
        xvbr = unmarshal(um, xnode.toString(), type);
        try {
            xvbr.toVbridgeBuilder(xlogger, vbrId);
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            assertEquals(vtag, e.getVtnErrorTag());
            assertEquals(msg, e.getMessage());
        }
        verifyNoMoreInteractions(xlogger);
    }
}
