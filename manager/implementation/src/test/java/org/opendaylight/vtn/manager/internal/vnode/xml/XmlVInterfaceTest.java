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

import org.opendaylight.vtn.manager.internal.util.inventory.SalNode;
import org.opendaylight.vtn.manager.internal.util.log.VTNLogLevel;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcErrorTag;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;
import org.opendaylight.vtn.manager.internal.util.vnode.VBridgeIfIdentifier;
import org.opendaylight.vtn.manager.internal.util.vnode.VInterfaceIdentifier;

import org.opendaylight.vtn.manager.internal.TestBase;
import org.opendaylight.vtn.manager.internal.XmlDataType;
import org.opendaylight.vtn.manager.internal.XmlNode;
import org.opendaylight.vtn.manager.internal.util.flow.filter.FlowFilterListTest;
import org.opendaylight.vtn.manager.internal.util.vnode.VTNPortMapConfigTest;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.flow.filter.list.VtnFlowFilter;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeName;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnErrorTag;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.vbridge.list.Vbridge;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.VtnMappableVinterface;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.vtn.mappable.vinterface.list.Vinterface;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.vtn.mappable.vinterface.list.VinterfaceBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.vtn.vinterface.info.VinterfaceConfig;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.vtn.vinterface.info.VinterfaceConfigBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.vtn.vinterface.info.VinterfaceInputFilter;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.vtn.vinterface.info.VinterfaceInputFilterBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.vtn.vinterface.info.VinterfaceOutputFilter;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.vtn.vinterface.info.VinterfaceOutputFilterBuilder;

/**
 * JUnit test for {@link XmlVInterface}.
 */
public class XmlVInterfaceTest extends TestBase {
    /**
     * Root XML element name associated with {@link XmlVInterface} class.
     */
    public static final String  XML_ROOT = "vinterface";

    /**
     * Return a list of {@link XmlDataType} instances that specifies XML node
     * types mapped to a {@link XmlVInterface} instance.
     *
     * @param name    The name of the target node.
     * @param parent  Path to the parent node.
     * @return  A list of {@link XmlDataType} instances.
     */
    public static List<XmlDataType> getXmlDataTypes(String name,
                                                    String ... parent) {
        List<XmlDataType> dlist = new ArrayList<>();
        String[] p = XmlDataType.addPath(name, parent);
        dlist.addAll(VTNPortMapConfigTest.getXmlDataTypes("port-map", p));
        dlist.addAll(FlowFilterListTest.getXmlDataTypes("input-filters", p));
        dlist.addAll(FlowFilterListTest.getXmlDataTypes("output-filters", p));
        return dlist;
    }

    /**
     * Test case for {@link XmlVInterface#toList(List)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testToList() throws Exception {
        List<Vinterface> list = null;
        assertEquals(null, XmlVInterface.toList(list));
        list = Collections.<Vinterface>emptyList();
        assertEquals(null, XmlVInterface.toList(list));

        list = new ArrayList<>();
        List<XmlVInterface> xlist = new ArrayList<>();
        for (int i = 0; i < 5; i++) {
            VnodeName vname = new VnodeName("vif_" + i);
            String desc = "vInterface " + i;
            VinterfaceConfig vifc = new VinterfaceConfigBuilder().
                setDescription(desc).
                build();
            Vinterface vif = new VinterfaceBuilder().
                setName(vname).
                setVinterfaceConfig(vifc).
                build();
            list.add(vif);
            xlist.add(new XmlVInterface(vif));
        }

        assertEquals(xlist, XmlVInterface.toList(list));
    }

    /**
     * Test case for {@link XmlVInterface#XmlVInterface(VtnMappableVinterface)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testConstructor() throws Exception {
        // Test case for name, description, and enabled.
        Boolean[] bools = {null, Boolean.TRUE, Boolean.FALSE};
        for (int i = 0; i < 5; i++) {
            String name = "vif_" + i;
            String desc = (i == 0) ? null : "vInterface " + i;
            for (Boolean enabled: bools) {
                VInterfaceConfig iconf = new VInterfaceConfig(name, desc).
                    setEnabled(enabled);
                VtnMappableVinterface vif = iconf.toVinterface();
                XmlVInterface xvif = new XmlVInterface(vif);
                iconf.verify(xvif, false);
            }
        }
    }

    /**
     * Test case for {@link XmlVInterface#getPortMap()}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testGetPortMap() throws Exception {
        VInterfaceConfig iconf = new VInterfaceConfig("vif", "vInterface").
            setEnabled(Boolean.TRUE);
        SalNode snode = new SalNode(12345L);
        XmlPortMapConfig xpmc = new XmlPortMapConfig(snode).
            setPortName("port-3").
            setPortId("3").
            setVlanId(4095);
        iconf.setPortMap(xpmc);
        Vinterface vif = iconf.toVinterface();
        XmlVInterface xvif = new XmlVInterface(vif);
        iconf.verify(xvif, false);

        xpmc = new XmlPortMapConfig(snode).
            setPortName("port-123");
        iconf.setPortMap(xpmc);
        vif = iconf.toVinterface();
        xvif = new XmlVInterface(vif);
        iconf.verify(xvif, false);

        xpmc = new XmlPortMapConfig(snode).
            setPortId("99").
            setVlanId(1);
        iconf.setPortMap(xpmc);
        vif = iconf.toVinterface();
        xvif = new XmlVInterface(vif);
        iconf.verify(xvif, false);
    }

    /**
     * Test case for {@link XmlVInterface#getInputFilters()} and
     * {@link XmlVInterface#getOutputFilters()}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testGetFlowFilters() throws Exception {
        String name = "vif";
        VnodeName vname = new VnodeName(name);
        String desc = "vInterface";
        Boolean enabled = Boolean.TRUE;
        VInterfaceConfig iconf = new VInterfaceConfig(name, desc).
            setEnabled(enabled);
        VinterfaceConfig vifc = iconf.toVinterfaceConfig();

        // Test case for null/empty flow filter list.
        List<List<VtnFlowFilter>> cases = new ArrayList<>();
        Collections.addAll(
            cases,
            null,
            Collections.<VtnFlowFilter>emptyList());
        for (List<VtnFlowFilter> vfilters: cases) {
            VinterfaceInputFilter input = new VinterfaceInputFilterBuilder().
                setVtnFlowFilter(vfilters).build();
            VinterfaceOutputFilter output = new VinterfaceOutputFilterBuilder().
                setVtnFlowFilter(vfilters).build();
            Vinterface vif = new VinterfaceBuilder().
                setName(vname).
                setVinterfaceConfig(vifc).
                setVinterfaceInputFilter(input).
                setVinterfaceOutputFilter(output).
                build();
            XmlVInterface xvif = new XmlVInterface(vif);
            assertEquals(vname, xvif.getName());
            assertEquals(desc, xvif.getDescription());
            assertEquals(enabled, VInterfaceConfig.isEnabled(xvif));
            assertEquals(null, xvif.getPortMap());
            assertEquals(null, xvif.getInputFilters());
            assertEquals(null, xvif.getOutputFilters());
        }

        // Input filter test.
        Random rand = new Random(0x123456789aL);
        XmlFlowActionList xactions = new XmlFlowActionList().addAll(rand);
        XmlFlowFilter xpass = new XmlPassFilter(1, "cond_pass").
            setFlowActions(xactions);
        XmlFlowFilterList xfilters = new XmlFlowFilterList().
            add(xpass).
            addAll(rand);
        Vinterface vif = iconf.setInputFilters(xfilters).toVinterface();
        XmlVInterface xvif = new XmlVInterface(vif);
        iconf.verify(xvif, false);

        // Output filter test.
        xactions = new XmlFlowActionList().addAll(rand);
        VBridgeIfIdentifier dest = new VBridgeIfIdentifier(
            null, new VnodeName("dest_bridge"), new VnodeName("dest_if"));
        XmlFlowFilter xredirect =
            new XmlRedirectFilter(65535, "cond", dest, null).
            setFlowActions(xactions);
        xfilters = new XmlFlowFilterList().
            add(xredirect).
            addAll(rand);
        vif = iconf.setOutputFilters(xfilters).toVinterface();
        xvif = new XmlVInterface(vif);
        iconf.verify(xvif, false);
    }

    /**
     * Test case for {@link XmlVInterface#equals(Object)} and
     * {@link XmlVInterface#hashCode()}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testEquals() throws Exception {
        Set<Object> set = new HashSet<>();

        String[] names = {
            "vif_1", "vif_2", "vinterface_3",
        };
        String[] descs = {
            null, "desc 1", "desciption 2",
        };
        Boolean[] bools = {Boolean.TRUE, Boolean.FALSE};
        int count = 0;
        for (String name1: names) {
            String name2 = new String(name1);
            for (String desc1: descs) {
                String desc2 = (desc1 == null) ? null : new String(desc1);
                for (Boolean enabled: bools) {
                    VInterfaceConfig iconf1 =
                        new VInterfaceConfig(name1, desc1).
                        setEnabled(enabled);
                    VInterfaceConfig iconf2 =
                        new VInterfaceConfig(name2, desc2).
                        setEnabled(enabled);
                    Vinterface vif1 = iconf1.toVinterface();
                    Vinterface vif2 = iconf2.toVinterface();
                    XmlVInterface xvif1 = new XmlVInterface(vif1);
                    XmlVInterface xvif2 = new XmlVInterface(vif2);
                    testEquals(set, xvif1, xvif2);
                    count++;
                }
            }
        }

        String name1 = names[0];
        String desc1 = descs[1];
        String name2 = new String(name1);
        String desc2 = new String(desc1);
        VInterfaceConfig iconf1 = new VInterfaceConfig(name1, desc1).
            setEnabled(Boolean.TRUE);
        VInterfaceConfig iconf2 = new VInterfaceConfig(name2, desc2).
            setEnabled(Boolean.TRUE);

        SalNode snode = new SalNode(1L);
        String portId = "1";
        XmlPortMapConfig xpmc1 = new XmlPortMapConfig(snode).
            setPortId(portId);
        XmlPortMapConfig xpmc2 = new XmlPortMapConfig(snode).
            setPortId(portId);
        iconf1.setPortMap(xpmc1);
        iconf2.setPortMap(xpmc2);
        Vinterface vif1 = iconf1.toVinterface();
        Vinterface vif2 = iconf2.toVinterface();
        XmlVInterface xvif1 = new XmlVInterface(vif1);
        XmlVInterface xvif2 = new XmlVInterface(vif2);
        testEquals(set, xvif1, xvif2);
        count++;

        String portName = "eth0";
        int vid = 3333;
        xpmc1.setPortName(portName).setVlanId(vid);
        xpmc2.setPortName(portName).setVlanId(vid);
        vif1 = iconf1.toVinterface();
        vif2 = iconf2.toVinterface();
        xvif1 = new XmlVInterface(vif1);
        xvif2 = new XmlVInterface(vif2);
        testEquals(set, xvif1, xvif2);
        count++;

        Random rand = new Random(0x333344445555L);
        XmlDropFilter xdrop = new XmlDropFilter(65535, "cond_drop");
        XmlFlowFilterList xinput1 = new XmlFlowFilterList().
            add(xdrop).
            addRandom(rand);
        XmlFlowFilterList xinput2 = xinput1.clone();
        vif1 = iconf1.setInputFilters(xinput1).toVinterface();
        vif2 = iconf2.setInputFilters(xinput2).toVinterface();
        xvif1 = new XmlVInterface(vif1);
        xvif2 = new XmlVInterface(vif2);
        testEquals(set, xvif1, xvif2);
        count++;

        XmlPassFilter xpass = new XmlPassFilter(1, "cond_pass");
        XmlFlowFilterList xoutput1 = new XmlFlowFilterList().
            add(xpass).
            addRandom(rand);
        XmlFlowFilterList xoutput2 = xoutput1.clone();
        vif1 = iconf1.setOutputFilters(xinput1).toVinterface();
        vif2 = iconf2.setOutputFilters(xinput2).toVinterface();
        xvif1 = new XmlVInterface(vif1);
        xvif2 = new XmlVInterface(vif2);
        testEquals(set, xvif1, xvif2);
        count++;

        assertEquals(count, set.size());
    }

    /**
     * Test case for {@link XmlVInterface#toVinterfaceBuilder(XmlLogger, VInterfaceIdentifier)}
     * and XML binding.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testJAXB() throws Exception {
        Class<XmlVInterface> type = XmlVInterface.class;
        Unmarshaller um = createUnmarshaller(type);

        String[] names = {"vif_1", "vinterface_1"};
        String[] descs = {null, "vif desc", "Virtual Interface 1"};
        Boolean[] bools = {null, Boolean.TRUE, Boolean.FALSE};
        String tname = "vtn";
        String bname = "vbridge";
        for (String name: names) {
            VInterfaceIdentifier<Vbridge> ifId =
                VBridgeIfIdentifier.create(tname, bname, name, false);
            for (String desc: descs) {
                for (Boolean bool: bools) {
                    VInterfaceConfig iconf = new VInterfaceConfig(name, desc).
                        setEnabled(bool);
                    XmlNode xnode = iconf.toXmlNode();
                    XmlVInterface xvif = unmarshal(um, xnode.toString(), type);
                    iconf.verify(xvif, true);
                    jaxbTest(xvif, type, XML_ROOT);
                    iconf.testToVinterfaceBuilder(xvif);
                }
            }
        }

        // Test for port-map.
        SalNode snode = new SalNode(334455L);
        XmlPortMapConfig xpmc = new XmlPortMapConfig(snode).
            setPortId("12").
            setPortName("port-12").
            setVlanId(45);
        String name = "vif";
        String desc = "virtual interface";
        Boolean enabled = Boolean.TRUE;
        VInterfaceConfig iconf = new VInterfaceConfig(name, desc).
            setEnabled(enabled).
            setPortMap(xpmc);
        XmlNode xnode = iconf.toXmlNode();
        XmlVInterface xvif = unmarshal(um, xnode.toString(), type);
        iconf.verify(xvif, true);
        jaxbTest(xvif, type, XML_ROOT);
        iconf.testToVinterfaceBuilder(xvif);

        // Test for input-filters.
        Random rand = new Random(0xaabbccddL);
        XmlDropFilter xdrop = new XmlDropFilter(345, "drop1");
        XmlPassFilter xpass = new XmlPassFilter(123, "pass1");
        XmlFlowFilterList xinput = new XmlFlowFilterList().
            add(xdrop).
            add(xpass).
            addAll(rand);
        xnode = iconf.setInputFilters(xinput).toXmlNode();
        xvif = unmarshal(um, xnode.toString(), type);
        iconf.verify(xvif, true);
        jaxbTest(xvif, type, XML_ROOT);
        iconf.testToVinterfaceBuilder(xvif);

        // Test for output-filters.
        xdrop = new XmlDropFilter(1, "drop2");
        xpass = new XmlPassFilter(2, "pass2");
        XmlFlowFilterList xoutput = new XmlFlowFilterList().
            add(xdrop).
            add(xpass).
            addAll(rand);
        xnode = iconf.setOutputFilters(xoutput).toXmlNode();
        xvif = unmarshal(um, xnode.toString(), type);
        iconf.verify(xvif, true);
        jaxbTest(xvif, type, XML_ROOT);
        iconf.testToVinterfaceBuilder(xvif);

        // Broken input filter should be ignored.
        XmlFlowFilterList xbadIn = new XmlFlowFilterList().
            add(new XmlDropFilter(null, "cond"));
        xnode = iconf.setInputFilters(xbadIn).toXmlNode();
        xvif = unmarshal(um, xnode.toString(), type);
        assertEquals(name, xvif.getName().getValue());
        assertEquals(desc, xvif.getDescription());
        assertEquals(enabled, VInterfaceConfig.isEnabled(xvif));
        xpmc.verify(xvif.getPortMap());
        xoutput.verify(xvif.getOutputFilters(), true);
        jaxbTest(xvif, type, XML_ROOT);

        XmlLogger xlogger = mock(XmlLogger.class);
        VInterfaceIdentifier<Vbridge> ifId =
            VBridgeIfIdentifier.create("vtn", "vbr", name, false);
        VinterfaceBuilder builder = xvif.toVinterfaceBuilder(xlogger, ifId);
        assertEquals(name, builder.getName().getValue());
        VinterfaceConfig vifc = builder.getVinterfaceConfig();
        assertEquals(desc, vifc.getDescription());
        assertEquals(enabled, vifc.isEnabled());
        assertEquals(null, builder.getVinterfaceStatus());
        xpmc.verify(builder.getPortMapConfig());
        assertEquals(null, builder.getVinterfaceInputFilter());
        xoutput.verify(builder.getVinterfaceOutputFilter());

        String inMsg = "IN";
        String outMsg = "OUT";
        String logfmt = "{}: {} flow filters have been loaded.";
        String badfmt = "%s: %s: Ignore broken flow filters.";
        verify(xlogger).
            log(eq(VTNLogLevel.WARN), any(RpcException.class), eq(badfmt),
                eq(ifId), eq(inMsg));
        verify(xlogger).log(VTNLogLevel.DEBUG, logfmt, ifId, outMsg);
        verifyNoMoreInteractions(xlogger);

        // Broken output filter should be ignored.
        XmlFlowFilterList xbadOut = new XmlFlowFilterList().
            add(new XmlPassFilter(333, null));
        xnode = iconf.setInputFilters(xinput).
            setOutputFilters(xbadOut).
            toXmlNode();
        xvif = unmarshal(um, xnode.toString(), type);

        assertEquals(name, xvif.getName().getValue());
        assertEquals(desc, xvif.getDescription());
        assertEquals(enabled, VInterfaceConfig.isEnabled(xvif));
        xpmc.verify(xvif.getPortMap());
        xinput.verify(xvif.getInputFilters(), true);
        jaxbTest(xvif, type, XML_ROOT);

        xlogger = mock(XmlLogger.class);
        builder = xvif.toVinterfaceBuilder(xlogger, ifId);
        assertEquals(name, builder.getName().getValue());
        vifc = builder.getVinterfaceConfig();
        assertEquals(desc, vifc.getDescription());
        assertEquals(enabled, vifc.isEnabled());
        assertEquals(null, builder.getVinterfaceStatus());
        xpmc.verify(builder.getPortMapConfig());
        xinput.verify(builder.getVinterfaceInputFilter());
        assertEquals(null, builder.getVinterfaceOutputFilter());

        verify(xlogger).log(VTNLogLevel.DEBUG, logfmt, ifId, inMsg);
        verify(xlogger).
            log(eq(VTNLogLevel.WARN), any(RpcException.class), eq(badfmt),
                eq(ifId), eq(outMsg));
        verifyNoMoreInteractions(xlogger);

        // Ensure that broken values in XML can be detected.
        jaxbErrorTest(um, type, getXmlDataTypes(XML_ROOT));

        // Node name is missing.
        xnode = new XmlNode(XML_ROOT).
            add(new XmlNode("description", desc));
        xvif = unmarshal(um, xnode.toString(), type);
        xlogger = mock(XmlLogger.class);
        RpcErrorTag etag = RpcErrorTag.MISSING_ELEMENT;
        VtnErrorTag vtag = VtnErrorTag.BADREQUEST;
        String msg = "vInterface name cannot be null";
        try {
            xvif.toVinterfaceBuilder(xlogger, ifId);
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            assertEquals(vtag, e.getVtnErrorTag());
            assertEquals(msg, e.getMessage());
        }
        verifyNoMoreInteractions(xlogger);
    }
}
