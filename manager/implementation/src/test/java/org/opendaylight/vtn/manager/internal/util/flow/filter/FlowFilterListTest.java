/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.flow.filter;

import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.when;

import static org.opendaylight.vtn.manager.internal.util.flow.action.FlowActionUtilsTest.createVtnSetDlDstAction;
import static org.opendaylight.vtn.manager.internal.util.flow.action.FlowActionUtilsTest.createVtnSetDlSrcAction;
import static org.opendaylight.vtn.manager.internal.util.flow.action.FlowActionUtilsTest.createVtnSetIcmpCodeAction;
import static org.opendaylight.vtn.manager.internal.util.flow.action.FlowActionUtilsTest.createVtnSetIcmpTypeAction;
import static org.opendaylight.vtn.manager.internal.util.flow.action.FlowActionUtilsTest.createVtnSetInetDscpAction;
import static org.opendaylight.vtn.manager.internal.util.flow.action.FlowActionUtilsTest.createVtnSetInetDstAction;
import static org.opendaylight.vtn.manager.internal.util.flow.action.FlowActionUtilsTest.createVtnSetInetSrcAction;
import static org.opendaylight.vtn.manager.internal.util.flow.action.FlowActionUtilsTest.createVtnSetPortDstAction;
import static org.opendaylight.vtn.manager.internal.util.flow.action.FlowActionUtilsTest.createVtnSetPortSrcAction;
import static org.opendaylight.vtn.manager.internal.util.flow.action.FlowActionUtilsTest.createVtnSetVlanPcpAction;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Set;

import javax.xml.bind.Marshaller;
import javax.xml.bind.Unmarshaller;

import org.junit.Test;

import org.opendaylight.vtn.manager.util.EtherAddress;
import org.opendaylight.vtn.manager.util.Ip4Network;

import org.opendaylight.vtn.manager.internal.util.rpc.RpcErrorTag;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;
import org.opendaylight.vtn.manager.internal.util.vnode.VBridgeIdentifier;
import org.opendaylight.vtn.manager.internal.util.vnode.VNodeIdentifier;
import org.opendaylight.vtn.manager.internal.util.vnode.VTenantIdentifier;
import org.opendaylight.vtn.manager.internal.util.vnode.VTerminalIfIdentifier;

import org.opendaylight.vtn.manager.internal.TestBase;
import org.opendaylight.vtn.manager.internal.XmlDataType;
import org.opendaylight.vtn.manager.internal.XmlNode;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.flow.action.list.VtnFlowAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.VtnFlowFilterList;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.flow.filter.list.VtnFlowFilter;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.flow.filter.list.VtnFlowFilterBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.flow.filter.type.fields.vtn.flow.filter.type.VtnDropFilterCase;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.flow.filter.type.fields.vtn.flow.filter.type.VtnDropFilterCaseBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.flow.filter.type.fields.vtn.flow.filter.type.VtnPassFilterCase;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.flow.filter.type.fields.vtn.flow.filter.type.VtnPassFilterCaseBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.flow.filter.type.fields.vtn.flow.filter.type.VtnRedirectFilterCase;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.flow.filter.type.fields.vtn.flow.filter.type.VtnRedirectFilterCaseBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.flow.filter.type.fields.vtn.flow.filter.type.vtn.drop.filter._case.VtnDropFilterBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.flow.filter.type.fields.vtn.flow.filter.type.vtn.pass.filter._case.VtnPassFilterBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.flow.filter.type.fields.vtn.flow.filter.type.vtn.redirect.filter._case.VtnRedirectFilterBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.redirect.filter.config.RedirectDestination;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.redirect.filter.config.RedirectDestinationBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.vtn.info.VtnInputFilterBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeName;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnErrorTag;

/**
 * JUnit test for {@link FlowFilterList}.
 */
public class FlowFilterListTest extends TestBase {
    /**
     * Root XML element name associated with {@link FlowFilterList} class.
     */
    private static final String  XML_ROOT = "vtn-flow-filter-list";

    /**
     * Return a list of {@link XmlDataType} instances that specifies XML node
     * types mapped to a {@link FlowFilterList} instance.
     *
     * @param name    The name of the target node.
     * @param parent  Path to the parent node.
     * @return  A list of {@link XmlDataType} instances.
     */
    public static List<XmlDataType> getXmlDataTypes(String name,
                                                    String ... parent) {
        List<XmlDataType> dlist = new ArrayList<>();
        String[] p = XmlDataType.addPath(name, parent);
        dlist.addAll(VTNPassFilterTest.getXmlDataTypes("pass-filter", p));
        dlist.addAll(VTNDropFilterTest.getXmlDataTypes("drop-filter", p));
        dlist.addAll(VTNRedirectFilterTest.
                     getXmlDataTypes("redirect-filter", p));

        return dlist;
    }

    /**
     * Test case for
     * {@link FlowFilterList#create(VtnFlowFilterList, boolean)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testCreate() throws Exception {
        // Null flow filter list.
        VtnFlowFilterList vflist = null;
        FlowFilterList ffl = FlowFilterList.create(vflist, false);
        assertEquals(true, ffl.isEmpty());
        assertEquals(null, ffl.getListId());
        assertEquals(null, FlowFilterList.create(vflist, true));

        // Null flow filter list in the container.
        vflist = new VtnInputFilterBuilder().build();
        ffl = FlowFilterList.create(vflist, false);
        assertEquals(true, ffl.isEmpty());
        assertEquals(null, ffl.getListId());
        assertEquals(null, FlowFilterList.create(vflist, true));

        // An empty list in the container.
        List<VtnFlowFilter> vfilters = new ArrayList<>();
        vflist = new VtnInputFilterBuilder().setVtnFlowFilter(vfilters).
            build();
        ffl = FlowFilterList.create(vflist, false);
        assertEquals(true, ffl.isEmpty());
        assertEquals(null, ffl.getListId());
        assertEquals(null, FlowFilterList.create(vflist, true));

        // Drop filter.
        VtnDropFilterCase drop = new VtnDropFilterCaseBuilder().
            setVtnDropFilter(new VtnDropFilterBuilder().build()).
            build();
        VtnFlowFilter vdrop16 = new VtnFlowFilterBuilder().
            setIndex(16).
            setCondition(new VnodeName("cond_1")).
            setVtnFlowFilterType(drop).
            build();
        vfilters.add(vdrop16);
        VTNFlowFilter drop16 = new VTNDropFilter(vdrop16, drop);
        VtnFlowFilter vdrop4 = new VtnFlowFilterBuilder().
            setIndex(4).
            setCondition(new VnodeName("cond_2")).
            setVtnFlowFilterType(drop).
            build();
        vfilters.add(vdrop4);
        VTNFlowFilter drop4 = new VTNDropFilter(vdrop4, drop);

        // Pass filter.
        List<VtnFlowAction> actions = new ArrayList<>();
        EtherAddress dlSrc = new EtherAddress(0x000000000010L);
        EtherAddress dlDst = new EtherAddress(0x00123456789aL);
        Ip4Network nwSrc = new Ip4Network("192.168.11.2");
        Ip4Network nwDst = new Ip4Network("10.12.34.56");
        Collections.addAll(
            actions,
            createVtnSetDlDstAction(3, dlDst.getMacAddress()),
            createVtnSetDlSrcAction(5, dlSrc.getMacAddress()),
            createVtnSetPortDstAction(9, 999),
            createVtnSetPortSrcAction(11, 33),
            createVtnSetIcmpTypeAction(15, 4),
            createVtnSetIcmpCodeAction(1, 1),
            createVtnSetInetDscpAction(4, 34),
            createVtnSetInetDstAction(2, nwDst.getMdAddress()),
            createVtnSetInetSrcAction(7, nwSrc.getMdAddress()),
            createVtnSetVlanPcpAction(8, 7));

        VtnPassFilterCase pass = new VtnPassFilterCaseBuilder().
            setVtnPassFilter(new VtnPassFilterBuilder().build()).
            build();
        VtnFlowFilter vpass115 = new VtnFlowFilterBuilder().
            setIndex(115).
            setCondition(new VnodeName("cond_3")).
            setVtnFlowFilterType(pass).
            build();
        vfilters.add(vpass115);
        VTNFlowFilter pass115 = new VTNPassFilter(vpass115, pass);

        VtnFlowFilter vpass3 = new VtnFlowFilterBuilder().
            setIndex(3).
            setCondition(new VnodeName("cond_4")).
            setVtnFlowFilterType(pass).
            setVtnFlowAction(actions).
            build();
        vfilters.add(vpass3);
        VTNFlowFilter pass3 = new VTNPassFilter(vpass3, pass);

        // Redirect filter.
        dlSrc = new EtherAddress(0x000011112222L);
        dlDst = new EtherAddress(0x00aabbccddeeL);
        nwSrc = new Ip4Network("10.20.30.40");
        nwDst = new Ip4Network("50.60.70.80");
        actions = new ArrayList<>();
        Collections.addAll(
            actions,
            createVtnSetDlDstAction(10, dlDst.getMacAddress()),
            createVtnSetDlSrcAction(9, dlSrc.getMacAddress()),
            createVtnSetPortDstAction(8, 65535),
            createVtnSetPortSrcAction(7, 1),
            createVtnSetIcmpTypeAction(6, 0),
            createVtnSetIcmpCodeAction(5, 127),
            createVtnSetInetDscpAction(4, 63),
            createVtnSetInetDstAction(3, nwDst.getMdAddress()),
            createVtnSetInetSrcAction(2, nwSrc.getMdAddress()),
            createVtnSetVlanPcpAction(1, 5));

        String bname = "vbridge_1";
        String iname = "if_1";
        RedirectDestination rdest19 = new RedirectDestinationBuilder().
            setBridgeName(bname).setInterfaceName(iname).build();
        VtnRedirectFilterCase redirect = new VtnRedirectFilterCaseBuilder().
            setVtnRedirectFilter(new VtnRedirectFilterBuilder().
                                 setRedirectDestination(rdest19).
                                 setOutput(false).build()).
            build();
        VtnFlowFilter vredirect19 = new VtnFlowFilterBuilder().
            setIndex(19).
            setCondition(new VnodeName("cond_5")).
            setVtnFlowFilterType(redirect).
            build();
        vfilters.add(vredirect19);
        VTNFlowFilter redirect19 = new VTNRedirectFilter(vredirect19, redirect);

        String vtname = "vterm_1";
        RedirectDestination rdest9 = new RedirectDestinationBuilder().
            setTerminalName(vtname).setInterfaceName(iname).build();
        redirect = new VtnRedirectFilterCaseBuilder().
            setVtnRedirectFilter(new VtnRedirectFilterBuilder().
                                 setRedirectDestination(rdest9).
                                 setOutput(true).build()).
            build();
        VtnFlowFilter vredirect9 = new VtnFlowFilterBuilder().
            setIndex(9).
            setCondition(new VnodeName("cond_6")).
            setVtnFlowFilterType(redirect).
            setVtnFlowAction(actions).
            build();
        vfilters.add(vredirect9);
        VTNFlowFilter redirect9 =
            new VTNRedirectFilter(vredirect9, redirect);

        vflist = new VtnInputFilterBuilder().setVtnFlowFilter(vfilters).
            build();
        List<VTNFlowFilter> expected = Arrays.asList(
            pass3, drop4, redirect9, drop16, redirect19, pass115);
        boolean[] bools = {true, false};
        for (boolean nillable: bools) {
            ffl = FlowFilterList.create(vflist, nillable);
            assertEquals(expected, ffl.getFlowFilters());
        }

        // Specifying invalid flow filter.
        VtnFlowFilter badFilter = new VtnFlowFilterBuilder().build();
        vfilters = Collections.singletonList(badFilter);
        vflist = new VtnInputFilterBuilder().setVtnFlowFilter(vfilters).
            build();
        RpcErrorTag etag = RpcErrorTag.MISSING_ELEMENT;
        VtnErrorTag vtag = VtnErrorTag.BADREQUEST;
        String msg = "vtn-flow-filter-type cannot be null";
        for (boolean nillable: bools) {
            try {
                FlowFilterList.create(vflist, nillable);
                unexpected();
            } catch (RpcException e) {
                assertEquals(etag, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                assertEquals(msg, e.getMessage());
            }
        }
    }

    /**
     * Test case for {@link FlowFilterList#setListId(FlowFilterListId)}.
     */
    @Test
    public void testSetListId() {
        VnodeName vtnName = new VnodeName("vtn1");
        VTenantIdentifier vtnId = new VTenantIdentifier(vtnName);
        FlowFilterListId flid = new FlowFilterListId(vtnId, false);
        FlowFilterList ffl = new FlowFilterList();
        assertTrue(ffl.isEmpty());
        assertEquals(null, ffl.getListId());
        ffl.setListId(flid);
        assertEquals(flid, ffl.getListId());

        VnodeName vbrName = new VnodeName("vbridge_1");
        VBridgeIdentifier vbrId = new VBridgeIdentifier(vtnName, vbrName);
        boolean[] bools = {true, false};
        for (boolean out: bools) {
            flid = new FlowFilterListId(vbrId, out);
            ffl.setListId(flid);
            assertEquals(flid, ffl.getListId());
        }
    }

    /**
     * Test case for {@link FlowFilterList#isEmpty(VtnFlowFilterList)}.
     */
    @Test
    public void testIsEmpty() {
        VtnFlowFilterList vflist = null;
        assertEquals(true, FlowFilterList.isEmpty(vflist));

        List<VtnFlowFilter> none = null;
        List<VtnFlowFilter> empty = Collections.<VtnFlowFilter>emptyList();
        List<VtnFlowFilter> list = new ArrayList<>();
        list.add(new VtnFlowFilterBuilder().build());

        vflist = mock(VtnFlowFilterList.class);
        when(vflist.getVtnFlowFilter()).
            thenReturn(none).
            thenReturn(empty).
            thenReturn(list);

        assertEquals(true, FlowFilterList.isEmpty(vflist));
        assertEquals(true, FlowFilterList.isEmpty(vflist));
        assertEquals(false, FlowFilterList.isEmpty(vflist));
    }

    /**
     * Test case for {@link FlowFilterList#equals(Object)} and
     * {@link FlowFilterList#hashCode()}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testEquals() throws Exception {
        Set<Object> set = new HashSet<>();

        List<VtnFlowFilter> list = new ArrayList<>();
        VtnDropFilterCase drop = new VtnDropFilterCaseBuilder().
            setVtnDropFilter(new VtnDropFilterBuilder().build()).
            build();
        VnodeName vcond1 = new VnodeName("cond_1");
        VnodeName vcond2 = new VnodeName("cond_2");
        VtnFlowFilter vff = new VtnFlowFilterBuilder().
            setIndex(1).
            setCondition(vcond1).
            setVtnFlowFilterType(drop).
            build();
        list.add(vff);

        vff = new VtnFlowFilterBuilder().
            setIndex(12345).
            setCondition(vcond2).
            setVtnFlowFilterType(drop).
            build();
        list.add(vff);

        VtnPassFilterCase pass = new VtnPassFilterCaseBuilder().
            setVtnPassFilter(new VtnPassFilterBuilder().build()).
            build();
        vff = new VtnFlowFilterBuilder().
            setIndex(99).
            setCondition(vcond1).
            setVtnFlowFilterType(pass).
            build();
        list.add(vff);

        vff = new VtnFlowFilterBuilder().
            setIndex(9384).
            setCondition(vcond2).
            setVtnFlowFilterType(pass).
            build();
        list.add(vff);

        RedirectDestination rdest = new RedirectDestinationBuilder().
            setBridgeName("vbr1").setInterfaceName("if1").build();
        VtnRedirectFilterCase redirect = new VtnRedirectFilterCaseBuilder().
            setVtnRedirectFilter(new VtnRedirectFilterBuilder().
                                 setRedirectDestination(rdest).
                                 setOutput(true).build()).
            build();
        vff = new VtnFlowFilterBuilder().
            setIndex(65535).
            setCondition(vcond1).
            setVtnFlowFilterType(redirect).
            build();
        list.add(vff);

        rdest = new RedirectDestinationBuilder().
            setTerminalName("vtm1").setInterfaceName("if2").build();
        redirect = new VtnRedirectFilterCaseBuilder().
            setVtnRedirectFilter(new VtnRedirectFilterBuilder().
                                 setRedirectDestination(rdest).
                                 setOutput(false).build()).
            build();
        vff = new VtnFlowFilterBuilder().
            setIndex(123).
            setCondition(vcond2).
            setVtnFlowFilterType(redirect).
            build();
        list.add(vff);

        List<VtnFlowFilter> vfilters = new ArrayList<>();
        for (VtnFlowFilter vf: list) {
            vfilters.add(vf);
            VtnFlowFilterList vflist = mock(VtnFlowFilterList.class);
            when(vflist.getVtnFlowFilter()).thenReturn(vfilters);
            FlowFilterList ffl1 = FlowFilterList.create(vflist, true);
            FlowFilterList ffl2 = FlowFilterList.create(vflist, true);
            testEquals(set, ffl1, ffl2);
        }

        FlowFilterList ffl1 = FlowFilterList.create(null, false);
        FlowFilterList ffl2 = FlowFilterList.create(null, false);
        testEquals(set, ffl1, ffl2);
        assertEquals(list.size() + 1, set.size());
    }

    /**
     * Ensure that {@link FlowFilterList} is mapped to XML root element.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testJAXB() throws Exception {
        List<VtnFlowFilter> list = new ArrayList<>();
        VtnDropFilterCase drop = new VtnDropFilterCaseBuilder().
            setVtnDropFilter(new VtnDropFilterBuilder().build()).
            build();
        VnodeName vcond1 = new VnodeName("cond_1");
        VnodeName vcond2 = new VnodeName("cond_2");
        VtnFlowFilter drop1 = new VtnFlowFilterBuilder().
            setIndex(1).
            setCondition(vcond1).
            setVtnFlowFilterType(drop).
            build();
        list.add(drop1);

        List<VtnFlowAction> actions = new ArrayList<>();
        EtherAddress dlSrc = new EtherAddress(0x000000000010L);
        EtherAddress dlDst = new EtherAddress(0x00123456789aL);
        Ip4Network nwSrc = new Ip4Network("192.168.11.2");
        Ip4Network nwDst = new Ip4Network("10.12.34.56");
        Collections.addAll(
            actions,
            createVtnSetDlDstAction(3, dlDst.getMacAddress()),
            createVtnSetDlSrcAction(5, dlSrc.getMacAddress()),
            createVtnSetPortDstAction(9, 999),
            createVtnSetPortSrcAction(11, 33),
            createVtnSetIcmpTypeAction(15, 4),
            createVtnSetIcmpCodeAction(1, 1),
            createVtnSetInetDscpAction(4, 34),
            createVtnSetInetDstAction(2, nwDst.getMdAddress()),
            createVtnSetInetSrcAction(7, nwSrc.getMdAddress()),
            createVtnSetVlanPcpAction(8, 7));

        VtnFlowFilter drop12345 = new VtnFlowFilterBuilder().
            setIndex(12345).
            setCondition(vcond2).
            setVtnFlowFilterType(drop).
            setVtnFlowAction(actions).
            build();
        list.add(drop12345);

        VtnPassFilterCase pass = new VtnPassFilterCaseBuilder().
            setVtnPassFilter(new VtnPassFilterBuilder().build()).
            build();
        VtnFlowFilter pass99 = new VtnFlowFilterBuilder().
            setIndex(99).
            setCondition(vcond1).
            setVtnFlowFilterType(pass).
            build();
        list.add(pass99);

        actions = new ArrayList<>();
        Collections.addAll(
            actions,
            createVtnSetPortSrcAction(11, 60000),
            createVtnSetInetDscpAction(4, 42));
        VtnFlowFilter pass9384 = new VtnFlowFilterBuilder().
            setIndex(9384).
            setCondition(vcond2).
            setVtnFlowFilterType(pass).
            setVtnFlowAction(actions).
            build();
        list.add(pass9384);

        RedirectDestination rdest = new RedirectDestinationBuilder().
            setBridgeName("vbr1").setInterfaceName("if1").build();
        VtnRedirectFilterCase redirect = new VtnRedirectFilterCaseBuilder().
            setVtnRedirectFilter(new VtnRedirectFilterBuilder().
                                 setRedirectDestination(rdest).
                                 setOutput(true).build()).
            build();
        VtnFlowFilter redirectMax = new VtnFlowFilterBuilder().
            setIndex(65535).
            setCondition(vcond1).
            setVtnFlowFilterType(redirect).
            build();
        list.add(redirectMax);

        rdest = new RedirectDestinationBuilder().
            setTerminalName("vtm1").setInterfaceName("if2").build();
        redirect = new VtnRedirectFilterCaseBuilder().
            setVtnRedirectFilter(new VtnRedirectFilterBuilder().
                                 setRedirectDestination(rdest).
                                 setOutput(false).build()).
            build();
        actions = new ArrayList<>();
        Collections.addAll(
            actions,
            createVtnSetVlanPcpAction(10, 3),
            createVtnSetIcmpTypeAction(15, 200),
            createVtnSetIcmpCodeAction(1, 190));
        VtnFlowFilter redirect123 = new VtnFlowFilterBuilder().
            setIndex(123).
            setCondition(vcond2).
            setVtnFlowFilterType(redirect).
            setVtnFlowAction(actions).
            build();
        list.add(redirect123);

        List<VtnFlowFilter> expected = Arrays.asList(
            drop1, pass99, redirect123, pass9384, drop12345, redirectMax);

        VtnFlowFilterList vflist = mock(VtnFlowFilterList.class);
        when(vflist.getVtnFlowFilter()).thenReturn(list);

        Class<FlowFilterList> type = FlowFilterList.class;
        Marshaller m = createMarshaller(type);
        Unmarshaller um = createUnmarshaller(type);
        FlowFilterList ffl = FlowFilterList.create(vflist, true);
        FlowFilterList ffl1 = jaxbTest(ffl, type, m, um, XML_ROOT);
        VnodeName vtnName = new VnodeName("vtn1");
        VTenantIdentifier vtnId = new VTenantIdentifier(vtnName);

        List<VtnFlowFilter> vfilters = ffl.toVtnFlowFilterList(vtnId);
        Iterator<VtnFlowFilter> it = expected.iterator();
        for (VtnFlowFilter vff: vfilters) {
            assertTrue(it.hasNext());
            assertEqualsVtnFlowFilter(it.next(), vff);
        }
        assertFalse(it.hasNext());

        ffl = FlowFilterList.create(null, false);
        ffl1 = jaxbTest(ffl, type, m, um, XML_ROOT);
        assertEquals(null, ffl.toVtnFlowFilterList(vtnId));

        // Ensure that broken values in XML can be detected.
        jaxbErrorTest(um, type, getXmlDataTypes(XML_ROOT));

        RpcErrorTag etag = RpcErrorTag.MISSING_ELEMENT;
        VtnErrorTag vtag = VtnErrorTag.BADREQUEST;
        Map<String, String> missingCases = new HashMap<>();

        // No filter index.
        String xml = new XmlNode(XML_ROOT).
            add(new XmlNode("pass-filter").
                add(new XmlNode("condition", "cond"))).
            toString();
        missingCases.put(xml, "Filter index cannot be null");

        // No flow condition name.
        xml = new XmlNode(XML_ROOT).
            add(new XmlNode("drop-filter").
                add(new XmlNode("index", 1))).
            toString();
        missingCases.put(xml, "Flow condition name cannot be null");

        // No vtn-redirect-filter container.
        xml = new XmlNode(XML_ROOT).
            add(new XmlNode("redirect-filter").
                add(new XmlNode("index", 1)).
                add(new XmlNode("condition", "cond"))).
            toString();
        missingCases.put(xml, "redirect-destination cannot be null");

        for (Map.Entry<String, String> entry: missingCases.entrySet()) {
            String x = entry.getKey();
            ffl = unmarshal(um, x, type);
            try {
                ffl.toVtnFlowFilterList(vtnId);
                unexpected();
            } catch (RpcException e) {
                assertEquals(etag, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                assertEquals(entry.getValue(), e.getMessage());
            }
        }

        etag = RpcErrorTag.BAD_ELEMENT;
        Map<String, String> badCases = new HashMap<>();

        // Duplicate filter index.
        int dup = 12345;
        xml = new XmlNode(XML_ROOT).
            add(new XmlNode("pass-filter").
                add(new XmlNode("index", dup)).
                add(new XmlNode("condition", "cond1"))).
            add(new XmlNode("redirect-filter").
                add(new XmlNode("index", 12344)).
                add(new XmlNode("condition", "cond2")).
                add(new XmlNode("vbridge-if").
                    add(new XmlNode("bridge-name", "vbr1")).
                    add(new XmlNode("interface-name", "if1")))).
            add(new XmlNode("drop-filter").
                add(new XmlNode("index", dup)).
                add(new XmlNode("condition", "cond3"))).
            toString();
        badCases.put(xml, "Duplicate flow filter index: " + dup);

        // Invalid filter index.
        int badIndex = Integer.MAX_VALUE;
        xml = new XmlNode(XML_ROOT).
            add(new XmlNode("pass-filter").
                add(new XmlNode("index", 1)).
                add(new XmlNode("condition", "cond1"))).
            add(new XmlNode("redirect-filter").
                add(new XmlNode("index", 12344)).
                add(new XmlNode("condition", "cond2")).
                add(new XmlNode("vbridge-if").
                    add(new XmlNode("bridge-name", "vbr1")).
                    add(new XmlNode("interface-name", "if1")))).
            add(new XmlNode("drop-filter").
                add(new XmlNode("index", badIndex)).
                add(new XmlNode("condition", "cond3"))).
            toString();
        badCases.put(xml, "Invalid filter index: " + badIndex);

        for (Map.Entry<String, String> entry: badCases.entrySet()) {
            String x = entry.getKey();
            ffl = unmarshal(um, x, type);
            try {
                ffl.toVtnFlowFilterList(vtnId);
                unexpected();
            } catch (RpcException e) {
                assertEquals(etag, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                assertEquals(entry.getValue(), e.getMessage());
            }
        }

        // Self redirection.
        String vtname = "vterm_1";
        String iname = "if_2";
        xml = new XmlNode(XML_ROOT).
            add(new XmlNode("redirect-filter").
                add(new XmlNode("index", 1)).
                add(new XmlNode("condition", "cond1")).
                add(new XmlNode("vterminal-if").
                    add(new XmlNode("bridge-name", vtname)).
                    add(new XmlNode("interface-name", iname)))).
            toString();

        ffl = unmarshal(um, xml, type);
        VNodeIdentifier<?> ident = new VTerminalIfIdentifier(
            vtnName, new VnodeName(vtname), new VnodeName(iname));
        try {
            ffl.toVtnFlowFilterList(ident);
            unexpected();
        } catch (RpcException e) {
            String msg = "Self redirection is not allowed: " + ident;
            assertEquals(etag, e.getErrorTag());
            assertEquals(vtag, e.getVtnErrorTag());
            assertEquals(msg, e.getMessage());
        }
    }
}
