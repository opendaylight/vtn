/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.flow.filter;

import static org.opendaylight.vtn.manager.internal.util.flow.action.FlowActionUtilsTest.createVtnDropAction;
import static org.opendaylight.vtn.manager.internal.util.flow.action.FlowActionUtilsTest.createVtnPopVlanAction;
import static org.opendaylight.vtn.manager.internal.util.flow.action.FlowActionUtilsTest.createVtnPushVlanAction;
import static org.opendaylight.vtn.manager.internal.util.flow.action.FlowActionUtilsTest.createVtnSetDlDstAction;
import static org.opendaylight.vtn.manager.internal.util.flow.action.FlowActionUtilsTest.createVtnSetDlSrcAction;
import static org.opendaylight.vtn.manager.internal.util.flow.action.FlowActionUtilsTest.createVtnSetIcmpCodeAction;
import static org.opendaylight.vtn.manager.internal.util.flow.action.FlowActionUtilsTest.createVtnSetIcmpTypeAction;
import static org.opendaylight.vtn.manager.internal.util.flow.action.FlowActionUtilsTest.createVtnSetInetDscpAction;
import static org.opendaylight.vtn.manager.internal.util.flow.action.FlowActionUtilsTest.createVtnSetInetDstAction;
import static org.opendaylight.vtn.manager.internal.util.flow.action.FlowActionUtilsTest.createVtnSetInetSrcAction;
import static org.opendaylight.vtn.manager.internal.util.flow.action.FlowActionUtilsTest.createVtnSetPortDstAction;
import static org.opendaylight.vtn.manager.internal.util.flow.action.FlowActionUtilsTest.createVtnSetPortSrcAction;
import static org.opendaylight.vtn.manager.internal.util.flow.action.FlowActionUtilsTest.createVtnSetVlanIdAction;
import static org.opendaylight.vtn.manager.internal.util.flow.action.FlowActionUtilsTest.createVtnSetVlanPcpAction;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.TreeMap;

import javax.xml.bind.Marshaller;
import javax.xml.bind.Unmarshaller;

import org.slf4j.Logger;

import org.junit.Test;

import org.opendaylight.vtn.manager.util.EtherAddress;
import org.opendaylight.vtn.manager.util.Ip4Network;

import org.opendaylight.vtn.manager.internal.util.flow.action.FlowFilterAction;
import org.opendaylight.vtn.manager.internal.util.flow.action.VTNSetDlDstAction;
import org.opendaylight.vtn.manager.internal.util.flow.action.VTNSetDlSrcAction;
import org.opendaylight.vtn.manager.internal.util.flow.action.VTNSetIcmpCodeAction;
import org.opendaylight.vtn.manager.internal.util.flow.action.VTNSetIcmpTypeAction;
import org.opendaylight.vtn.manager.internal.util.flow.action.VTNSetInetDscpAction;
import org.opendaylight.vtn.manager.internal.util.flow.action.VTNSetInetDstAction;
import org.opendaylight.vtn.manager.internal.util.flow.action.VTNSetInetSrcAction;
import org.opendaylight.vtn.manager.internal.util.flow.action.VTNSetPortDstAction;
import org.opendaylight.vtn.manager.internal.util.flow.action.VTNSetPortSrcAction;
import org.opendaylight.vtn.manager.internal.util.flow.action.VTNSetVlanPcpAction;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcErrorTag;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;
import org.opendaylight.vtn.manager.internal.util.vnode.VNodeIdentifier;

import org.opendaylight.vtn.manager.internal.TestBase;
import org.opendaylight.vtn.manager.internal.XmlDataType;
import org.opendaylight.vtn.manager.internal.XmlNode;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.flow.action.list.VtnFlowAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.flow.action.list.VtnFlowActionBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.VtnFlowFilterConfig;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.flow.filter.list.VtnFlowFilter;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.flow.filter.list.VtnFlowFilterBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.flow.filter.type.fields.VtnFlowFilterType;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.flow.filter.type.fields.vtn.flow.filter.type.VtnDropFilterCase;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.flow.filter.type.fields.vtn.flow.filter.type.VtnDropFilterCaseBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.flow.filter.type.fields.vtn.flow.filter.type.vtn.drop.filter._case.VtnDropFilterBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeName;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnErrorTag;

/**
 * JUnit test for {@link VTNDropFilter}.
 */
public class VTNDropFilterTest extends TestBase {
    /**
     * Root XML element name associated with {@link VTNDropFilter} class.
     */
    private static final String  XML_ROOT = "vtn-drop-filter";

    /**
     * Return a list of {@link XmlDataType} instances that specifies XML node
     * types mapped to a {@link VTNDropFilter} instance.
     *
     * @param name    The name of the target node.
     * @param parent  Path to the parent node.
     * @return  A list of {@link XmlDataType} instances.
     */
    public static List<XmlDataType> getXmlDataTypes(String name,
                                                    String ... parent) {
        return VTNFlowFilterTest.getXmlDataTypes(name, parent);
    }

    /**
     * Test case for
     * {@link VTNDropFilter#VTNDropFilter(VtnFlowFilterConfig, VtnDropFilterCase)}
     * and the followings.
     *
     * <ul>
     *   <li>{@link VTNFlowFilter#getIdentifier()}</li>
     *   <li>{@link VTNFlowFilter#getCondition()}</li>
     *   <li>{@link VTNFlowFilter#toVtnFlowFilter()}</li>
     *   <li>{@link VTNDropFilter#getVtnFlowFilterType()}</li>
     *   <li>{@link VTNDropFilter#canSet(VNodeIdentifier)}</li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testConstructor() throws Exception {
        Integer[] indices = {1, 2, 1000, 30000, 65534, 65535};
        String[] conditions = {"cond_1", "cond_2"};

        EtherAddress dlSrc = new EtherAddress(0x123456789L);
        EtherAddress dlDst = new EtherAddress(0xaabbccddee11L);
        Ip4Network nwSrc = new Ip4Network("192.168.100.200");
        Ip4Network nwDst = new Ip4Network("127.0.0.1");
        int pcp = 5;
        int dscp = 45;
        int portSrc = 33333;
        int portDst = 100;
        int icmpType = 129;
        int icmpCode = 38;

        // VTNDropFilter.canSet() does nothing.
        VNodeIdentifier ident = null;

        List<VtnFlowAction> empty = Collections.<VtnFlowAction>emptyList();
        List<VtnFlowAction> actions = new ArrayList<>();
        Collections.addAll(
            actions,
            createVtnSetDlDstAction(5, dlDst.getMacAddress()),
            createVtnSetDlSrcAction(2, dlSrc.getMacAddress()),
            createVtnSetPortDstAction(4, portDst),
            createVtnSetPortSrcAction(10, portSrc),
            createVtnSetIcmpTypeAction(0, icmpType),
            createVtnSetIcmpCodeAction(11, icmpCode),
            createVtnSetInetDscpAction(1, dscp),
            createVtnSetInetDstAction(13, nwDst.getMdAddress()),
            createVtnSetInetSrcAction(3, nwSrc.getMdAddress()),
            createVtnSetVlanPcpAction(12, pcp));

        // Flow actions should be sorted by action order value.
        List<FlowFilterAction> factions = new ArrayList<>();
        Collections.addAll(
            factions,
            new VTNSetIcmpTypeAction((short)icmpType, 0),
            new VTNSetInetDscpAction((short)dscp, 1),
            new VTNSetDlSrcAction(dlSrc, 2),
            new VTNSetInetSrcAction(nwSrc, 3),
            new VTNSetPortDstAction(portDst, 4),
            new VTNSetDlDstAction(dlDst, 5),
            new VTNSetPortSrcAction(portSrc, 10),
            new VTNSetIcmpCodeAction((short)icmpCode, 11),
            new VTNSetVlanPcpAction((short)pcp, 12),
            new VTNSetInetDstAction(nwDst, 13));

        VtnDropFilterCase drop = new VtnDropFilterCaseBuilder().
            setVtnDropFilter(new VtnDropFilterBuilder().build()).
            build();

        for (Integer index: indices) {
            for (String cond: conditions) {
                // Null action.
                VnodeName vcond = new VnodeName(cond);
                VtnFlowFilter vff = new VtnFlowFilterBuilder().
                    setIndex(index).
                    setCondition(vcond).
                    setVtnFlowFilterType(drop).
                    build();
                VTNDropFilter vdrop = new VTNDropFilter(vff, drop);
                assertEquals(index, vdrop.getIdentifier());
                assertEquals(cond, vdrop.getCondition());
                assertEquals(true, vdrop.getActions().isEmpty());
                assertEquals(drop, vdrop.getVtnFlowFilterType());
                assertEqualsVtnFlowFilter(vff, vdrop.toVtnFlowFilter());
                vdrop.canSet(ident);

                // Empty action.
                VtnFlowFilterConfig vffc = new VtnFlowFilterBuilder().
                    setIndex(index).
                    setCondition(vcond).
                    setVtnFlowAction(empty).
                    setVtnFlowFilterType(drop).
                    build();
                vdrop = new VTNDropFilter(vffc, drop);
                assertEquals(index, vdrop.getIdentifier());
                assertEquals(cond, vdrop.getCondition());
                assertEquals(true, vdrop.getActions().isEmpty());
                assertEquals(drop, vdrop.getVtnFlowFilterType());
                assertEqualsVtnFlowFilter(vff, vdrop.toVtnFlowFilter());
                vdrop.canSet(ident);

                // All supported actions.
                vff = new VtnFlowFilterBuilder().
                    setIndex(index).
                    setCondition(vcond).
                    setVtnFlowAction(actions).
                    setVtnFlowFilterType(drop).
                    build();
                vdrop = new VTNDropFilter(vff, drop);
                assertEquals(index, vdrop.getIdentifier());
                assertEquals(cond, vdrop.getCondition());
                assertEquals(factions, new ArrayList<>(vdrop.getActions()));
                assertEquals(drop, vdrop.getVtnFlowFilterType());
                assertEqualsVtnFlowFilter(vff, vdrop.toVtnFlowFilter());
                vdrop.canSet(ident);
            }
        }

        RpcErrorTag etag = RpcErrorTag.MISSING_ELEMENT;
        VtnErrorTag vtag = VtnErrorTag.BADREQUEST;
        VnodeName vcond = new VnodeName("condition");
        Map<VtnFlowFilter, String> missingCases = new HashMap<>();

        // No filter index.
        VtnFlowFilter vfilter = new VtnFlowFilterBuilder().
            setVtnFlowFilterType(drop).build();
        missingCases.put(vfilter, "Filter index cannot be null");

        // No flow condition name.
        vfilter = new VtnFlowFilterBuilder().
            setIndex(1).setVtnFlowFilterType(drop).build();
        missingCases.put(vfilter, "Flow condition name cannot be null");

        // No vtn-drop-filter container.
        vfilter = new VtnFlowFilterBuilder().
            setIndex(1).setCondition(vcond).
            setVtnFlowFilterType(new VtnDropFilterCaseBuilder().build()).
            build();
        missingCases.put(vfilter, "vtn-drop-filter cannot be null");

        // Null in action list.
        actions = new ArrayList<>();
        actions.add(null);
        vfilter = new VtnFlowFilterBuilder().
            setIndex(1).setCondition(vcond).
            setVtnFlowFilterType(drop).setVtnFlowAction(actions).
            build();
        missingCases.put(vfilter, "vtn-flow-action cannot be null");

        // Null vtn-action.
        actions = new ArrayList<>();
        actions.add(new VtnFlowActionBuilder().setOrder(0).build());
        vfilter = new VtnFlowFilterBuilder().
            setIndex(1).setCondition(vcond).
            setVtnFlowFilterType(drop).setVtnFlowAction(actions).
            build();
        missingCases.put(vfilter, "vtn-action cannot be null");

        // No action order.
        actions = new ArrayList<>();
        Collections.addAll(
            actions,
            createVtnSetIcmpCodeAction(1, 12),
            createVtnSetInetDscpAction(2, 3),
            createVtnSetVlanPcpAction(null, 5));
        vfilter = new VtnFlowFilterBuilder().
            setIndex(1).setCondition(vcond).
            setVtnFlowFilterType(drop).setVtnFlowAction(actions).
            build();
        missingCases.put(vfilter,
                         "VTNSetVlanPcpAction: Action order cannot be null");

        for (Map.Entry<VtnFlowFilter, String> entry: missingCases.entrySet()) {
            VtnFlowFilter vff = entry.getKey();
            VtnFlowFilterType vftype = vff.getVtnFlowFilterType();
            assertTrue(vftype instanceof VtnDropFilterCase);
            VtnDropFilterCase vdfc = (VtnDropFilterCase)vftype;
            try {
                new VTNDropFilter(vff, vdfc);
                unexpected();
            } catch (RpcException e) {
                assertEquals(etag, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                assertEquals(entry.getValue(), e.getMessage());
            }
        }

        // Unsupported flow action.
        etag = RpcErrorTag.BAD_ELEMENT;
        List<VtnFlowAction> unsupported = new ArrayList<>();
        Collections.addAll(
            unsupported,
            createVtnDropAction(1),
            createVtnPopVlanAction(1),
            createVtnPushVlanAction(1),
            createVtnSetVlanIdAction(1, 1));
        for (VtnFlowAction vfa: unsupported) {
            List<VtnFlowAction> act = Collections.singletonList(vfa);
            VtnFlowFilter vff = new VtnFlowFilterBuilder().
                setIndex(1).setCondition(vcond).
                setVtnFlowFilterType(drop).setVtnFlowAction(act).
                build();
            Class<?> type = vfa.getVtnAction().getImplementedInterface();
            String msg = "Unsupported vtn-action: " + type.getName();
            try {
                new VTNDropFilter(vff, drop);
                unexpected();
            } catch (RpcException e) {
                assertEquals(etag, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                assertEquals(msg, e.getMessage());
            }
        }

        // Duplicate action order.
        actions = new ArrayList<>();
        int dup = 1;
        Collections.addAll(
            actions,
            createVtnSetDlDstAction(5, dlDst.getMacAddress()),
            createVtnSetDlSrcAction(2, dlSrc.getMacAddress()),
            createVtnSetPortDstAction(4, portDst),
            createVtnSetPortSrcAction(10, portSrc),
            createVtnSetIcmpTypeAction(0, icmpType),
            createVtnSetIcmpCodeAction(11, icmpCode),
            createVtnSetInetDscpAction(dup, dscp),
            createVtnSetInetDstAction(13, nwDst.getMdAddress()),
            createVtnSetInetSrcAction(3, nwSrc.getMdAddress()),
            createVtnSetVlanPcpAction(dup, pcp));

        String msg = "Duplicate action order: " + dup;
        try {
            VtnFlowFilter vff = new VtnFlowFilterBuilder().
                setIndex(1).setCondition(vcond).setVtnFlowAction(actions).
                build();
            new VTNDropFilter(vff, drop);
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            assertEquals(vtag, e.getVtnErrorTag());
            assertEquals(msg, e.getMessage());
        }
    }

    /**
     * Test case for {@link VTNFlowFilter#equals(Object)} and
     * {@link VTNFlowFilter#hashCode()}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testEquals() throws Exception {
        Set<Object> set = new HashSet<>();

        Integer[] indices = {1, 2, 1234, 32768, 65534, 65535};
        String[] conditions = {"cond_1", "cond_2", "condition", "cond_3"};
        List<List<VtnFlowAction>> actionLists = new ArrayList<>();
        List<VtnFlowAction> actions = Collections.<VtnFlowAction>emptyList();
        actionLists.add(actions);

        EtherAddress dlSrc = new EtherAddress(0x001122334455L);
        EtherAddress dlDst = new EtherAddress(0xc0abcd987654L);
        Ip4Network nwSrc = new Ip4Network("192.168.111.222");
        Ip4Network nwDst = new Ip4Network("10.20.30.40");

        actions = Collections.singletonList(
            createVtnSetDlSrcAction(0, dlSrc.getMacAddress()));
        actionLists.add(actions);

        actions = new ArrayList<>();
        Collections.addAll(
            actions,
            createVtnSetDlDstAction(10, dlDst.getMacAddress()),
            createVtnSetDlSrcAction(0, dlSrc.getMacAddress()));
        actionLists.add(actions);

        actions = new ArrayList<>();
        Collections.addAll(
            actions,
            createVtnSetDlDstAction(0, dlDst.getMacAddress()),
            createVtnSetDlSrcAction(1, dlSrc.getMacAddress()),
            createVtnSetPortDstAction(2, 40000),
            createVtnSetPortSrcAction(3, 999),
            createVtnSetIcmpTypeAction(4, 0),
            createVtnSetIcmpCodeAction(5, 1),
            createVtnSetInetDscpAction(6, 0),
            createVtnSetInetDstAction(7, nwDst.getMdAddress()),
            createVtnSetInetSrcAction(8, nwSrc.getMdAddress()),
            createVtnSetVlanPcpAction(9, 0));
        actionLists.add(actions);

        for (String cond: conditions) {
            VnodeName vcond1 = new VnodeName(cond);
            VnodeName vcond2 = copy(vcond1);
            for (Integer index: indices) {
                for (List<VtnFlowAction> acts: actionLists) {
                    VtnDropFilterCase drop1 = new VtnDropFilterCaseBuilder().
                        setVtnDropFilter(new VtnDropFilterBuilder().build()).
                        build();
                    VtnDropFilterCase drop2 = new VtnDropFilterCaseBuilder().
                        setVtnDropFilter(new VtnDropFilterBuilder().build()).
                        build();
                    VtnFlowFilter vff1 = new VtnFlowFilterBuilder().
                        setIndex(index).
                        setCondition(vcond1).
                        setVtnFlowFilterType(drop1).
                        setVtnFlowAction(acts).
                        build();
                    VtnFlowFilter vff2 = new VtnFlowFilterBuilder().
                        setIndex(copy(index)).
                        setCondition(vcond2).
                        setVtnFlowFilterType(drop2).
                        setVtnFlowAction(Collections.unmodifiableList(acts)).
                        build();
                    testEquals(set, vff1, vff2);
                }
            }
        }

        int expected = conditions.length * indices.length * actionLists.size();
        assertEquals(expected, set.size());
    }

    /**
     * Test case for {@link VTNDropFilter#getLogger()}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testGetLogger() throws Exception {
        VtnDropFilterCase drop = new VtnDropFilterCaseBuilder().
            setVtnDropFilter(new VtnDropFilterBuilder().build()).
            build();
        VtnFlowFilter vff = new VtnFlowFilterBuilder().
            setIndex(1).
            setCondition(new VnodeName("cond")).
            setVtnFlowFilterType(drop).
            build();
        VTNDropFilter vdrop = new VTNDropFilter(vff, drop);
        Logger logger = vdrop.getLogger();
        assertEquals(VTNDropFilter.class.getName(), logger.getName());
    }

    /**
     * Ensure that {@link VTNDropFilter} is mapped to XML root element.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testJAXB() throws Exception {
        List<Class<?>> jaxbClasses = new ArrayList<>();
        Collections.addAll(jaxbClasses, VTNDropFilter.class,
                           VTNFlowFilter.class);

        Integer[] indices = {1, 2, 1234, 32768, 65534, 65535};
        String[] conditions = {"cond_1", "cond_2", "condition", "cond_3"};

        List<List<VtnFlowAction>> actionLists = new ArrayList<>();
        actionLists.add(null);

        EtherAddress dlSrc = new EtherAddress(0x001122334455L);
        EtherAddress dlDst = new EtherAddress(0xc0abcd987654L);
        Ip4Network nwSrc = new Ip4Network("192.168.111.222");
        Ip4Network nwDst = new Ip4Network("10.20.30.40");

        List<VtnFlowAction> actions = Collections.singletonList(
            createVtnSetDlSrcAction(0, dlSrc.getMacAddress()));
        actionLists.add(actions);

        actions = new ArrayList<>();
        Collections.addAll(
            actions,
            createVtnSetDlDstAction(10, dlDst.getMacAddress()),
            createVtnSetDlSrcAction(0, dlSrc.getMacAddress()));
        actionLists.add(actions);

        actions = new ArrayList<>();
        Collections.addAll(
            actions,
            createVtnSetDlDstAction(0, dlDst.getMacAddress()),
            createVtnSetDlSrcAction(1, dlSrc.getMacAddress()),
            createVtnSetPortDstAction(2, 40000),
            createVtnSetPortSrcAction(3, 999),
            createVtnSetIcmpTypeAction(4, 0),
            createVtnSetIcmpCodeAction(5, 1),
            createVtnSetInetDscpAction(6, 0),
            createVtnSetInetDstAction(7, nwDst.getMdAddress()),
            createVtnSetInetSrcAction(8, nwSrc.getMdAddress()),
            createVtnSetVlanPcpAction(9, 0));
        actionLists.add(actions);

        VtnDropFilterCase drop = new VtnDropFilterCaseBuilder().
            setVtnDropFilter(new VtnDropFilterBuilder().build()).
            build();

        int xindex = 1;
        String xcond = "xcond";
        EtherAddress xdlSrc = new EtherAddress(0x00123456789aL);
        EtherAddress xdlDst = new EtherAddress(0xa0b0c0d0e0f0L);
        Ip4Network xnwSrc = new Ip4Network("192.168.0.1");
        Ip4Network xnwDst = new Ip4Network("11.22.33.44");
        int xpcp = 5;
        int xdscp = 60;
        int xportSrc = 93;
        int xportDst = 6633;
        int xicmpType = 1;
        int xicmpCode = 45;

        int xordDlSrc = 8;
        int xordDlDst = 10;
        int xordNwSrc = 9;
        int xordNwDst = 3;
        int xordDscp = 4;
        int xordPortSrc = 2;
        int xordPortDst = 5;
        int xordIcmpType = 6;
        int xordIcmpCode = 1;
        int xordPcp = 7;

        String sortTestXml = new XmlNode(XML_ROOT).
            add(new XmlNode("index", xindex)).
            add(new XmlNode("condition", xcond)).
            add(new XmlNode("actions").
                add(new XmlNode("vtn-set-dl-src").
                    add(new XmlNode("order", xordDlSrc)).
                    add(new XmlNode("address", xdlSrc.getText()))).
                add(new XmlNode("vtn-set-dl-dst").
                    add(new XmlNode("order", xordDlDst)).
                    add(new XmlNode("address", xdlDst.getText()))).
                add(new XmlNode("vtn-set-inet-src").
                    add(new XmlNode("order", xordNwSrc)).
                    add(new XmlNode("ipv4-address", xnwSrc.getText()))).
                add(new XmlNode("vtn-set-inet-dst").
                    add(new XmlNode("order", xordNwDst)).
                    add(new XmlNode("ipv4-address", xnwDst.getText()))).
                add(new XmlNode("vtn-set-inet-dscp").
                    add(new XmlNode("order", xordDscp)).
                    add(new XmlNode("dscp", xdscp))).
                add(new XmlNode("vtn-set-port-src").
                    add(new XmlNode("order", xordPortSrc)).
                    add(new XmlNode("port", xportSrc))).
                add(new XmlNode("vtn-set-port-dst").
                    add(new XmlNode("order", xordPortDst)).
                    add(new XmlNode("port", xportDst))).
                add(new XmlNode("vtn-set-icmp-type").
                    add(new XmlNode("order", xordIcmpType)).
                    add(new XmlNode("type", xicmpType))).
                add(new XmlNode("vtn-set-icmp-code").
                    add(new XmlNode("order", xordIcmpCode)).
                    add(new XmlNode("code", xicmpCode))).
                add(new XmlNode("vtn-set-vlan-pcp").
                    add(new XmlNode("order", xordPcp)).
                    add(new XmlNode("priority", xpcp)))).
            toString();
        List<VtnFlowAction> xactions = new ArrayList<>();
        Collections.addAll(
            xactions,
            createVtnSetDlSrcAction(xordDlSrc, xdlSrc.getMacAddress()),
            createVtnSetDlDstAction(xordDlDst, xdlDst.getMacAddress()),
            createVtnSetInetSrcAction(xordNwSrc, xnwSrc.getMdAddress()),
            createVtnSetInetDstAction(xordNwDst, xnwDst.getMdAddress()),
            createVtnSetInetDscpAction(xordDscp, xdscp),
            createVtnSetPortSrcAction(xordPortSrc, xportSrc),
            createVtnSetPortDstAction(xordPortDst, xportDst),
            createVtnSetIcmpTypeAction(xordIcmpType, xicmpType),
            createVtnSetIcmpCodeAction(xordIcmpCode, xicmpCode),
            createVtnSetVlanPcpAction(xordPcp, xpcp));

        Map<Integer, VtnFlowAction> actMap = new TreeMap<>();
        for (VtnFlowAction vfact: xactions) {
            assertEquals(null, actMap.put(vfact.getOrder(), vfact));
        }

        Class<VTNDropFilter> type = VTNDropFilter.class;
        List<XmlDataType> dlist = getXmlDataTypes(XML_ROOT);
        for (Class<?> cls: jaxbClasses) {
            Marshaller m = createMarshaller(cls);
            Unmarshaller um = createUnmarshaller(cls);

            for (String cond: conditions) {
                VnodeName vcond = new VnodeName(cond);
                for (Integer index: indices) {
                    for (List<VtnFlowAction> acts: actionLists) {
                        VtnFlowFilter vff = new VtnFlowFilterBuilder().
                            setIndex(index).
                            setCondition(vcond).
                            setVtnFlowFilterType(drop).
                            setVtnFlowAction(acts).
                            build();
                        VTNDropFilter vdrop = new VTNDropFilter(vff, drop);
                        VTNDropFilter d =
                            jaxbTest(vdrop, type, m, um, XML_ROOT);
                        d.verify();
                    }
                }
            }

            // Ensure that broken values in XML can be detected.
            jaxbErrorTest(um, type, dlist);

            // Ensure that flow actions are sorted in ascending order of
            // action order.
            VTNDropFilter vdrop = unmarshal(um, sortTestXml, type);
            vdrop.verify();
            assertEquals(xindex, vdrop.getIdentifier().intValue());
            assertEquals(xcond, vdrop.getCondition());
            assertEquals(drop, vdrop.getVtnFlowFilterType());
            Iterator<Map.Entry<Integer, VtnFlowAction>> it =
                actMap.entrySet().iterator();
            for (FlowFilterAction ffact: vdrop.getActions()) {
                Map.Entry<Integer, VtnFlowAction> ent = it.next();
                assertEquals(ent.getKey(), ffact.getIdentifier());
                assertEquals(ent.getValue(), ffact.toVtnFlowAction());
            }
            assertEquals(false, it.hasNext());
        }

        Unmarshaller um = createUnmarshaller(type);
        RpcErrorTag etag = RpcErrorTag.MISSING_ELEMENT;
        VtnErrorTag vtag = VtnErrorTag.BADREQUEST;
        Map<String, String> missingCases = new HashMap<>();

        // No filter index.
        String xml = new XmlNode(XML_ROOT).toString();
        missingCases.put(xml, "Filter index cannot be null");

        // No flow condition name.
        xml = new XmlNode(XML_ROOT).
            add(new XmlNode("index", 1)).
            toString();
        missingCases.put(xml, "Flow condition name cannot be null");

        // No action order.
        xml = new XmlNode(XML_ROOT).
            add(new XmlNode("index", 1)).
            add(new XmlNode("condition", "cond")).
            add(new XmlNode("actions").
                add(new XmlNode("vtn-set-icmp-type").
                    add(new XmlNode("type", 1)))).
            toString();
        missingCases.put(xml,
                         "VTNSetIcmpTypeAction: Action order cannot be null");

        for (Map.Entry<String, String> entry: missingCases.entrySet()) {
            String x = entry.getKey();
            VTNDropFilter vdrop = unmarshal(um, x, type);
            try {
                vdrop.verify();
                unexpected();
            } catch (RpcException e) {
                assertEquals(etag, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                assertEquals(entry.getValue(), e.getMessage());
            }
        }

        // Invalid filter index.
        etag = RpcErrorTag.BAD_ELEMENT;
        int[] badIndices = {
            Integer.MIN_VALUE, -1000000, -2, -1, 0,
            65536, 65537, 1000000, Integer.MAX_VALUE,
        };
        for (int index: badIndices) {
            xml = new XmlNode(XML_ROOT).
                add(new XmlNode("index", index)).
                add(new XmlNode("condition", "cond")).
                toString();
            String msg = "Invalid filter index: " + index;
            VTNDropFilter vdrop = unmarshal(um, xml, type);
            try {
                vdrop.verify();
                unexpected();
            } catch (RpcException e) {
                assertEquals(etag, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                assertEquals(msg, e.getMessage());
            }
        }

        Map<String, String> broken = new HashMap<>();

        // Duplicate action order.
        int dup = 99;
        xml = new XmlNode(XML_ROOT).
            add(new XmlNode("index", 1)).
            add(new XmlNode("condition", "cond")).
            add(new XmlNode("actions").
                add(new XmlNode("vtn-set-icmp-type").
                    add(new XmlNode("order", dup)).
                    add(new XmlNode("type", 1))).
                add(new XmlNode("vtn-set-inet-dscp").
                    add(new XmlNode("order", 1)).
                    add(new XmlNode("dscp", 1))).
                add(new XmlNode("vtn-set-dl-src").
                    add(new XmlNode("order", dup)).
                    add(new XmlNode("address", "00:11:22:33:44:55")))).
            toString();
        broken.put(xml, "Duplicate action order: " + dup);

        // Invalid flow action.
        short icmpType = Short.MAX_VALUE;
        xml = new XmlNode(XML_ROOT).
            add(new XmlNode("index", 1)).
            add(new XmlNode("condition", "cond")).
            add(new XmlNode("actions").
                add(new XmlNode("vtn-set-icmp-type").
                    add(new XmlNode("order", 1)).
                    add(new XmlNode("type", icmpType)))).
            toString();
        broken.put(xml,
                   "VTNSetIcmpTypeAction: Invalid ICMP type: " + icmpType);

        for (Map.Entry<String, String> entry: broken.entrySet()) {
            VTNDropFilter vdrop = unmarshal(um, entry.getKey(), type);
            try {
                vdrop.verify();
                unexpected();
            } catch (RpcException e) {
                assertEquals(etag, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                assertEquals(entry.getValue(), e.getMessage());
            }
        }
    }
}
