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
import org.opendaylight.vtn.manager.internal.util.vnode.VBridgeIdentifier;
import org.opendaylight.vtn.manager.internal.util.vnode.VBridgeIfIdentifier;
import org.opendaylight.vtn.manager.internal.util.vnode.VInterfaceIdentifier;
import org.opendaylight.vtn.manager.internal.util.vnode.VNodeIdentifier;
import org.opendaylight.vtn.manager.internal.util.vnode.VTenantIdentifier;
import org.opendaylight.vtn.manager.internal.util.vnode.VTerminalIdentifier;
import org.opendaylight.vtn.manager.internal.util.vnode.VTerminalIfIdentifier;

import org.opendaylight.vtn.manager.internal.TestBase;
import org.opendaylight.vtn.manager.internal.XmlDataType;
import org.opendaylight.vtn.manager.internal.XmlNode;
import org.opendaylight.vtn.manager.internal.util.vnode.VBridgeIfIdentifierTest;
import org.opendaylight.vtn.manager.internal.util.vnode.VTerminalIfIdentifierTest;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.flow.action.list.VtnFlowAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.flow.action.list.VtnFlowActionBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.VtnFlowFilterConfig;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.flow.filter.list.VtnFlowFilter;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.flow.filter.list.VtnFlowFilterBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.flow.filter.type.fields.VtnFlowFilterType;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.flow.filter.type.fields.vtn.flow.filter.type.VtnRedirectFilterCase;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.flow.filter.type.fields.vtn.flow.filter.type.VtnRedirectFilterCaseBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.flow.filter.type.fields.vtn.flow.filter.type.vtn.redirect.filter._case.VtnRedirectFilter;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.flow.filter.type.fields.vtn.flow.filter.type.vtn.redirect.filter._case.VtnRedirectFilterBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.redirect.filter.config.RedirectDestination;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.redirect.filter.config.RedirectDestinationBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeName;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnErrorTag;

/**
 * JUnit test for {@link VTNRedirectFilter}.
 */
public class VTNRedirectFilterTest extends TestBase {
    /**
     * Root XML element name associated with {@link VTNRedirectFilter} class.
     */
    private static final String  XML_ROOT = "vtn-redirect-filter";

    /**
     * Return a list of {@link XmlDataType} instances that specifies XML node
     * types mapped to a {@link VTNRedirectFilter} instance.
     *
     * @param name    The name of the target node.
     * @param parent  Path to the parent node.
     * @return  A list of {@link XmlDataType} instances.
     */
    public static List<XmlDataType> getXmlDataTypes(String name,
                                                    String ... parent) {
        List<XmlDataType> dlist =
            VTNFlowFilterTest.getXmlDataTypes(name, parent);
        String[] p = XmlDataType.addPath(name, parent);
        dlist.addAll(VBridgeIfIdentifierTest.getXmlDataTypes("vbridge-if", p));
        dlist.addAll(VTerminalIfIdentifierTest.
                     getXmlDataTypes("vterminal-if", p));

        // "output" field does not need to be tested.
        return dlist;
    }

    /**
     * Test case for
     * {@link VTNRedirectFilter#VTNRedirectFilter(VtnFlowFilterConfig, VtnRedirectFilterCase)}
     * and the followings.
     *
     * <ul>
     *   <li>{@link VTNFlowFilter#getIdentifier()}</li>
     *   <li>{@link VTNFlowFilter#getCondition()}</li>
     *   <li>{@link VTNFlowFilter#toVtnFlowFilter()}</li>
     *   <li>{@link VTNRedirectFilter#getVtnFlowFilterType()}</li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testConstructor() throws Exception {
        Integer[] indices = {1, 2, 3456, 55555, 65534, 65535};
        String[] conditions = {"cond_1", "cond_2"};
        Boolean[] bools = {null, Boolean.TRUE, Boolean.FALSE};
        VInterfaceIdentifier[] dests = {
            new VBridgeIfIdentifier(
                new VnodeName("vtn"), new VnodeName("vbr1"),
                new VnodeName("if1")),
            new VTerminalIfIdentifier(
                null, new VnodeName("vterm1"), new VnodeName("if1")),
            new VBridgeIfIdentifier(
                null, new VnodeName("vbridge_1"), new VnodeName("if_2")),
            new VTerminalIfIdentifier(
                new VnodeName("vtn1"), new VnodeName("vbr1"),
                new VnodeName("if1")),
        };

        EtherAddress dlSrc = new EtherAddress(0x100101010101L);
        EtherAddress dlDst = new EtherAddress(0x00aabbccddeeL);
        Ip4Network nwSrc = new Ip4Network("10.44.55.66");
        Ip4Network nwDst = new Ip4Network("192.168.200.254");
        int pcp = 3;
        int dscp = 0;
        int portSrc = 8765;
        int portDst = 123;
        int icmpType = 127;
        int icmpCode = 0;

        List<VtnFlowAction> empty = Collections.<VtnFlowAction>emptyList();
        List<VtnFlowAction> actions = new ArrayList<>();
        Collections.addAll(
            actions,
            createVtnSetDlDstAction(77, dlDst.getMacAddress()),
            createVtnSetDlSrcAction(99, dlSrc.getMacAddress()),
            createVtnSetPortDstAction(33, portDst),
            createVtnSetPortSrcAction(55, portSrc),
            createVtnSetIcmpTypeAction(88, icmpType),
            createVtnSetIcmpCodeAction(100, icmpCode),
            createVtnSetInetDscpAction(22, dscp),
            createVtnSetInetDstAction(44, nwDst.getMdAddress()),
            createVtnSetInetSrcAction(11, nwSrc.getMdAddress()),
            createVtnSetVlanPcpAction(66, pcp));

        // Flow actions should be sorted by action order value.
        List<FlowFilterAction> factions = new ArrayList<>();
        Collections.addAll(
            factions,
            new VTNSetInetSrcAction(nwSrc, 11),
            new VTNSetInetDscpAction((short)dscp, 22),
            new VTNSetPortDstAction(portDst, 33),
            new VTNSetInetDstAction(nwDst, 44),
            new VTNSetPortSrcAction(portSrc, 55),
            new VTNSetVlanPcpAction((short)pcp, 66),
            new VTNSetDlDstAction(dlDst, 77),
            new VTNSetIcmpTypeAction((short)icmpType, 88),
            new VTNSetDlSrcAction(dlSrc, 99),
            new VTNSetIcmpCodeAction((short)icmpCode, 100));

        for (Integer index: indices) {
            for (String cond: conditions) {
                VnodeName vcond = new VnodeName(cond);
                for (VInterfaceIdentifier dst: dests) {
                    RedirectDestination rd = dst.toRedirectDestination();
                    for (Boolean output: bools) {
                        VtnRedirectFilter vrf = new VtnRedirectFilterBuilder().
                            setRedirectDestination(rd).
                            setOutput(output).
                            build();
                        VtnRedirectFilterCase redirect =
                            new VtnRedirectFilterCaseBuilder().
                            setVtnRedirectFilter(vrf).
                            build();

                        // Null action.
                        VtnFlowFilter vff = new VtnFlowFilterBuilder().
                            setIndex(index).
                            setCondition(vcond).
                            setVtnFlowFilterType(redirect).
                            build();
                        VTNRedirectFilter vredirect =
                            new VTNRedirectFilter(vff, redirect);
                        assertEquals(index, vredirect.getIdentifier());
                        assertEquals(cond, vredirect.getCondition());
                        assertEquals(true, vredirect.getActions().isEmpty());

                        RedirectDestinationBuilder rdb =
                            new RedirectDestinationBuilder().
                            setInterfaceName(dst.getInterfaceNameString());
                        if (dst instanceof VBridgeIfIdentifier) {
                            rdb.setBridgeName(dst.getBridgeNameString());
                        } else {
                            rdb.setTerminalName(dst.getBridgeNameString());
                        }

                        VtnRedirectFilter exVrf =
                            new VtnRedirectFilterBuilder().
                            setOutput(Boolean.TRUE.equals(output)).
                            setRedirectDestination(rdb.build()).
                            build();
                        VtnRedirectFilterCase exCase =
                            new VtnRedirectFilterCaseBuilder().
                            setVtnRedirectFilter(exVrf).
                            build();
                        assertEquals(exCase, vredirect.getVtnFlowFilterType());

                        VtnFlowFilter exVff = new VtnFlowFilterBuilder(vff).
                            setVtnFlowFilterType(exCase).
                            build();
                        assertEqualsVtnFlowFilter(exVff,
                                                  vredirect.toVtnFlowFilter());

                        // Empty action.
                        VtnFlowFilterConfig vffc = new VtnFlowFilterBuilder().
                            setIndex(index).
                            setCondition(vcond).
                            setVtnFlowAction(empty).
                            setVtnFlowFilterType(redirect).
                            build();
                        vredirect = new VTNRedirectFilter(vffc, redirect);
                        assertEquals(index, vredirect.getIdentifier());
                        assertEquals(cond, vredirect.getCondition());
                        assertEquals(true, vredirect.getActions().isEmpty());
                        assertEquals(exCase, vredirect.getVtnFlowFilterType());
                        assertEqualsVtnFlowFilter(exVff,
                                                  vredirect.toVtnFlowFilter());

                        // All supported actions.
                        vff = new VtnFlowFilterBuilder().
                            setIndex(index).
                            setCondition(vcond).
                            setVtnFlowAction(actions).
                            setVtnFlowFilterType(redirect).
                            build();
                        exVff = new VtnFlowFilterBuilder(vff).
                            setVtnFlowFilterType(exCase).
                            build();
                        vredirect = new VTNRedirectFilter(vff, redirect);
                        assertEquals(index, vredirect.getIdentifier());
                        assertEquals(cond, vredirect.getCondition());
                        assertEquals(factions,
                                     new ArrayList<>(vredirect.getActions()));
                        assertEquals(exCase, vredirect.getVtnFlowFilterType());
                        assertEqualsVtnFlowFilter(exVff,
                                                  vredirect.toVtnFlowFilter());
                    }
                }
            }
        }

        RpcErrorTag etag = RpcErrorTag.MISSING_ELEMENT;
        VtnErrorTag vtag = VtnErrorTag.BADREQUEST;
        VnodeName vcond = new VnodeName("condition");
        Map<VtnFlowFilter, String> missingCases = new HashMap<>();
        RedirectDestination rdst = new RedirectDestinationBuilder().
            setBridgeName("vbridge_1").
            setInterfaceName("if_1").
            build();
        VtnRedirectFilter vrf = new VtnRedirectFilterBuilder().
            setRedirectDestination(rdst).
            setOutput(true).
            build();
        VtnRedirectFilterCase redirect = new VtnRedirectFilterCaseBuilder().
            setVtnRedirectFilter(vrf).
            build();

        // No filter index.
        VtnFlowFilter vfilter = new VtnFlowFilterBuilder().
            setVtnFlowFilterType(redirect).build();
        missingCases.put(vfilter, "Filter index cannot be null");

        // No flow condition name.
        vfilter = new VtnFlowFilterBuilder().
            setIndex(1).setVtnFlowFilterType(redirect).build();
        missingCases.put(vfilter, "Flow condition name cannot be null");

        // No vtn-redirect-filter container.
        vfilter = new VtnFlowFilterBuilder().
            setIndex(1).setCondition(vcond).
            setVtnFlowFilterType(new VtnRedirectFilterCaseBuilder().build()).
            build();
        missingCases.put(vfilter, "vtn-redirect-filter cannot be null");

        // No redirect-destination container.
        VtnRedirectFilterCase badCase = new VtnRedirectFilterCaseBuilder().
            setVtnRedirectFilter(new VtnRedirectFilterBuilder().build()).
            build();
        vfilter = new VtnFlowFilterBuilder().
            setIndex(1).setCondition(vcond).
            setVtnFlowFilterType(badCase).
            build();
        missingCases.put(vfilter, "redirect-destination cannot be null");

        // Null in action list.
        actions = new ArrayList<>();
        actions.add(null);
        vfilter = new VtnFlowFilterBuilder().
            setIndex(1).setCondition(vcond).
            setVtnFlowFilterType(redirect).setVtnFlowAction(actions).
            build();
        missingCases.put(vfilter, "vtn-flow-action cannot be null");

        // Null vtn-action.
        actions = new ArrayList<>();
        actions.add(new VtnFlowActionBuilder().setOrder(0).build());
        vfilter = new VtnFlowFilterBuilder().
            setIndex(1).setCondition(vcond).
            setVtnFlowFilterType(redirect).setVtnFlowAction(actions).
            build();
        missingCases.put(vfilter, "vtn-action cannot be null");

        // No action order.
        actions = new ArrayList<>();
        Collections.addAll(
            actions,
            createVtnSetInetDscpAction(1, 4),
            createVtnSetPortSrcAction(2, 100),
            createVtnSetDlDstAction(null, dlDst.getMacAddress()));
        vfilter = new VtnFlowFilterBuilder().
            setIndex(1).setCondition(vcond).
            setVtnFlowFilterType(redirect).setVtnFlowAction(actions).
            build();
        missingCases.put(vfilter,
                         "VTNSetDlDstAction: Action order cannot be null");

        for (Map.Entry<VtnFlowFilter, String> entry: missingCases.entrySet()) {
            VtnFlowFilter vff = entry.getKey();
            VtnFlowFilterType vftype = vff.getVtnFlowFilterType();
            assertTrue(vftype instanceof VtnRedirectFilterCase);
            VtnRedirectFilterCase vrfc = (VtnRedirectFilterCase)vftype;
            try {
                new VTNRedirectFilter(vff, vrfc);
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
                setVtnFlowFilterType(redirect).setVtnFlowAction(act).
                build();
            Class<?> type = vfa.getVtnAction().getImplementedInterface();
            String msg = "Unsupported vtn-action: " + type.getName();
            try {
                new VTNRedirectFilter(vff, redirect);
                unexpected();
            } catch (RpcException e) {
                assertEquals(etag, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                assertEquals(msg, e.getMessage());
            }
        }

        // Duplicate action order.
        actions = new ArrayList<>();
        int dup = 12345;
        Collections.addAll(
            actions,
            createVtnSetDlDstAction(5, dlDst.getMacAddress()),
            createVtnSetDlSrcAction(dup, dlSrc.getMacAddress()),
            createVtnSetPortDstAction(4, portDst),
            createVtnSetPortSrcAction(12344, portSrc),
            createVtnSetIcmpTypeAction(dup, icmpType),
            createVtnSetIcmpCodeAction(11, icmpCode),
            createVtnSetInetDscpAction(100, dscp),
            createVtnSetInetDstAction(33, nwDst.getMdAddress()),
            createVtnSetInetSrcAction(3, nwSrc.getMdAddress()),
            createVtnSetVlanPcpAction(101, pcp));

        String msg = "Duplicate action order: " + dup;
        try {
            VtnFlowFilter vff = new VtnFlowFilterBuilder().
                setIndex(1).setCondition(vcond).setVtnFlowAction(actions).
                build();
            new VTNRedirectFilter(vff, redirect);
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            assertEquals(vtag, e.getVtnErrorTag());
            assertEquals(msg, e.getMessage());
        }
    }

    /**
     * Test case for {@link VTNRedirectFilter#canSet(VNodeIdentifier)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testCanSet() throws Exception {
        String vtnName = "vtn_1";
        String bridgeName = "bridge_1";
        String ifName = "if_1";
        VInterfaceIdentifier[] dests = {
            new VBridgeIfIdentifier(
                null, new VnodeName(bridgeName), new VnodeName(ifName)),
            new VTerminalIfIdentifier(
                null, new VnodeName(bridgeName), new VnodeName(ifName)),
        };

        VnodeName[] tnames = {
            new VnodeName(vtnName),
            new VnodeName("vtn_2"),
            new VnodeName("vtn_3"),
        };
        VnodeName[] bnames = {
            new VnodeName(bridgeName),
            new VnodeName("bridge_2"),
            new VnodeName("bridge_3"),
        };
        VnodeName[] inames = {
            new VnodeName(ifName),
            new VnodeName("if_2"),
            new VnodeName("vif_1"),
        };

        VnodeName vcond = new VnodeName("cond");
        RpcErrorTag etag = RpcErrorTag.BAD_ELEMENT;
        VtnErrorTag vtag = VtnErrorTag.BADREQUEST;

        for (VInterfaceIdentifier dst: dests) {
            VtnRedirectFilter vrf = new VtnRedirectFilterBuilder().
                setRedirectDestination(dst.toRedirectDestination()).
                build();
            VtnRedirectFilterCase redirect = new VtnRedirectFilterCaseBuilder().
                setVtnRedirectFilter(vrf).
                build();
            VtnFlowFilter vff = new VtnFlowFilterBuilder().
                setIndex(1).
                setCondition(vcond).
                setVtnFlowFilterType(redirect).
                build();
            VTNRedirectFilter vredirect = new VTNRedirectFilter(vff, redirect);

            for (VnodeName tname: tnames) {
                // VTN flow filter can contain redirection to any interface.
                VNodeIdentifier<?> vid = new VTenantIdentifier(tname);
                vredirect.canSet(vid);

                for (VnodeName bname: bnames) {
                    // vBridge flow filter can contain redirection to any
                    // interface.
                    vid = new VBridgeIdentifier(tname, bname);
                    vredirect.canSet(vid);

                    // Although vTerminal has no flow filter, canSet() should
                    // never throw exception.
                    vid = new VTerminalIdentifier(tname, bname);
                    vredirect.canSet(vid);

                    for (VnodeName iname: inames) {
                        VInterfaceIdentifier[] ifIds = {
                            new VBridgeIfIdentifier(tname, bname, iname),
                            new VTerminalIfIdentifier(tname, bname, iname),
                        };
                        for (VInterfaceIdentifier ifId: ifIds) {
                            boolean bad = (dst.getType() == ifId.getType() &&
                                           dst.getBridgeName().equals(bname) &&
                                           dst.getInterfaceName().
                                           equals(iname));
                            try {
                                vredirect.canSet(ifId);
                                assertFalse("dst=" + dst + ", id=" + ifId, bad);
                            } catch (RpcException e) {
                                assertTrue(bad);
                                assertEquals(etag, e.getErrorTag());
                                assertEquals(vtag, e.getVtnErrorTag());
                                String msg =
                                    "Self redirection is not allowed: " + ifId;
                                assertEquals(msg, e.getMessage());
                            }
                        }
                    }
                }
            }
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

        Integer[] indices = {1, 2, 65534, 65535};
        String[] conditions = {"cond_1", "cond_2", "cond_3"};
        Boolean[] bools = {Boolean.TRUE, Boolean.FALSE};
        VInterfaceIdentifier[] dests = {
            new VBridgeIfIdentifier(
                new VnodeName("vtn"), new VnodeName("vbr1"),
                new VnodeName("if1")),
            new VTerminalIfIdentifier(
                new VnodeName("vtn1"), new VnodeName("vbr1"),
                new VnodeName("if1")),
        };

        List<List<VtnFlowAction>> actionLists = new ArrayList<>();
        List<VtnFlowAction> actions = Collections.<VtnFlowAction>emptyList();
        actionLists.add(actions);

        EtherAddress dlSrc = new EtherAddress(0x001122334455L);
        EtherAddress dlDst = new EtherAddress(0xc0abcd987654L);
        Ip4Network nwSrc = new Ip4Network("192.168.111.222");
        Ip4Network nwDst = new Ip4Network("10.20.30.40");

        actions = Collections.singletonList(createVtnSetInetDscpAction(0, 45));
        actionLists.add(actions);

        actions = new ArrayList<>();
        Collections.addAll(
            actions,
            createVtnSetDlDstAction(10, dlDst.getMacAddress()),
            createVtnSetInetSrcAction(0, nwSrc.getMdAddress()),
            createVtnSetIcmpTypeAction(3, 0));
        actionLists.add(actions);

        actions = new ArrayList<>();
        Collections.addAll(
            actions,
            createVtnSetPortDstAction(90, 80),
            createVtnSetDlDstAction(80, dlDst.getMacAddress()),
            createVtnSetIcmpCodeAction(70, 1),
            createVtnSetDlSrcAction(10, dlSrc.getMacAddress()),
            createVtnSetVlanPcpAction(60, 0),
            createVtnSetPortSrcAction(0, 55555),
            createVtnSetInetDstAction(20, nwDst.getMdAddress()),
            createVtnSetInetDscpAction(50, 0),
            createVtnSetInetSrcAction(30, nwSrc.getMdAddress()),
            createVtnSetIcmpTypeAction(40, 0));
        actionLists.add(actions);

        for (String cond: conditions) {
            VnodeName vcond1 = new VnodeName(cond);
            VnodeName vcond2 = copy(vcond1);
            for (Integer index: indices) {
                for (VInterfaceIdentifier dst: dests) {
                    RedirectDestination rd = dst.toRedirectDestination();
                    for (Boolean output: bools) {
                        for (List<VtnFlowAction> acts: actionLists) {
                            VtnRedirectFilter vrf1 =
                                new VtnRedirectFilterBuilder().
                                setRedirectDestination(rd).
                                setOutput(output).
                                build();
                            VtnRedirectFilter vrf2 =
                                new VtnRedirectFilterBuilder().
                                setRedirectDestination(rd).
                                setOutput(output).
                                build();
                            VtnRedirectFilterCase redirect1 =
                                new VtnRedirectFilterCaseBuilder().
                                setVtnRedirectFilter(vrf1).
                                build();
                            VtnRedirectFilterCase redirect2 =
                                new VtnRedirectFilterCaseBuilder().
                                setVtnRedirectFilter(vrf2).
                                build();

                            VtnFlowFilter vff1 = new VtnFlowFilterBuilder().
                                setIndex(index).
                                setCondition(vcond1).
                                setVtnFlowFilterType(redirect1).
                                setVtnFlowAction(acts).
                                build();
                            VtnFlowFilter vff2 = new VtnFlowFilterBuilder().
                                setIndex(copy(index)).
                                setCondition(vcond2).
                                setVtnFlowFilterType(redirect2).
                                setVtnFlowAction(Collections.
                                                 unmodifiableList(acts)).
                                build();
                            testEquals(set, vff1, vff2);
                        }
                    }
                }
            }
        }

        int expected = conditions.length * indices.length *
            actionLists.size() * bools.length * dests.length;
        assertEquals(expected, set.size());
    }

    /**
     * Test case for {@link VTNRedirectFilter#getLogger()}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testGetLogger() throws Exception {
        RedirectDestination rdst = new RedirectDestinationBuilder().
            setBridgeName("vbridge_1").
            setInterfaceName("if_1").
            build();
        VtnRedirectFilter vrf = new VtnRedirectFilterBuilder().
            setRedirectDestination(rdst).
            build();
        VtnRedirectFilterCase redirect = new VtnRedirectFilterCaseBuilder().
            setVtnRedirectFilter(vrf).
            build();
        VtnFlowFilter vff = new VtnFlowFilterBuilder().
            setIndex(1).
            setCondition(new VnodeName("cond")).
            setVtnFlowFilterType(redirect).
            build();
        VTNRedirectFilter vredirect = new VTNRedirectFilter(vff, redirect);
        Logger logger = vredirect.getLogger();
        assertEquals(VTNRedirectFilter.class.getName(), logger.getName());
    }

    /**
     * Ensure that {@link VTNRedirectFilter} is mapped to XML root element.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testJAXB() throws Exception {
        List<Class<?>> jaxbClasses = new ArrayList<>();
        Collections.addAll(jaxbClasses, VTNRedirectFilter.class,
                           VTNFlowFilter.class);

        Integer[] indices = {1, 100, 9999, 32768, 45678, 65535};
        String[] conditions = {"cond_1", "cond_2", "condition", "cond_3"};
        Boolean[] bools = {Boolean.TRUE, Boolean.FALSE};
        VInterfaceIdentifier[] dests = {
            new VBridgeIfIdentifier(
                null, new VnodeName("vbr1"), new VnodeName("if1")),
            new VTerminalIfIdentifier(
                null, new VnodeName("vterm1"), new VnodeName("if1")),
            new VBridgeIfIdentifier(
                null, new VnodeName("vbridge_1"), new VnodeName("if_2")),
            new VTerminalIfIdentifier(
                null, new VnodeName("vbr1"), new VnodeName("if1")),
        };

        List<List<VtnFlowAction>> actionLists = new ArrayList<>();
        actionLists.add(null);

        EtherAddress dlSrc = new EtherAddress(0xe012345abcdeL);
        EtherAddress dlDst = new EtherAddress(0xf098765fedcbL);
        Ip4Network nwSrc = new Ip4Network("192.168.1.2");
        Ip4Network nwDst = new Ip4Network("10.2.3.4");

        List<VtnFlowAction> actions = Collections.singletonList(
            createVtnSetInetSrcAction(0, nwSrc.getMdAddress()));
        actionLists.add(actions);

        actions = new ArrayList<>();
        Collections.addAll(
            actions,
            createVtnSetInetSrcAction(1, nwSrc.getMdAddress()),
            createVtnSetInetDstAction(0, nwDst.getMdAddress()));
        actionLists.add(actions);

        actions = new ArrayList<>();
        Collections.addAll(
            actions,
            createVtnSetVlanPcpAction(0, 5),
            createVtnSetDlSrcAction(1, dlSrc.getMacAddress()),
            createVtnSetInetDscpAction(2, 60),
            createVtnSetPortSrcAction(3, 12345),
            createVtnSetPortDstAction(4, 55555),
            createVtnSetIcmpTypeAction(5, 123),
            createVtnSetDlDstAction(6, dlDst.getMacAddress()),
            createVtnSetInetDstAction(7, nwDst.getMdAddress()),
            createVtnSetInetSrcAction(8, nwSrc.getMdAddress()),
            createVtnSetIcmpCodeAction(9, 200));
        actionLists.add(actions);

        int xindex = 65535;
        String xcond = "xcond2";
        String xdstbr = "vbridge_1";
        String xdstif = "if_4";
        boolean xoutput = true;
        EtherAddress xdlSrc = new EtherAddress(0x901234abcdefL);
        EtherAddress xdlDst = new EtherAddress(0x001000000000L);
        Ip4Network xnwSrc = new Ip4Network("33.44.55.66");
        Ip4Network xnwDst = new Ip4Network("192.168.254.254");
        int xpcp = 2;
        int xdscp = 0;
        int xportSrc = 32768;
        int xportDst = 4433;
        int xicmpType = 127;
        int xicmpCode = 9;

        int xordDlSrc = 5;
        int xordDlDst = 1;
        int xordNwSrc = 2;
        int xordNwDst = 3;
        int xordDscp = 6;
        int xordPortSrc = 8;
        int xordPortDst = 4;
        int xordIcmpType = 10;
        int xordIcmpCode = 7;
        int xordPcp = 9;

        String sortTestXml = new XmlNode(XML_ROOT).
            add(new XmlNode("index", xindex)).
            add(new XmlNode("condition", xcond)).
            add(new XmlNode("vbridge-if").
                add(new XmlNode("bridge-name", xdstbr)).
                add(new XmlNode("interface-name", xdstif))).
            add(new XmlNode("output", xoutput)).
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

        Class<VTNRedirectFilter> type = VTNRedirectFilter.class;
        List<XmlDataType> dlist = getXmlDataTypes(XML_ROOT);
        for (Class<?> cls: jaxbClasses) {
            Marshaller m = createMarshaller(cls);
            Unmarshaller um = createUnmarshaller(cls);

            for (String cond: conditions) {
                VnodeName vcond = new VnodeName(cond);
                for (Integer index: indices) {
                    for (VInterfaceIdentifier dst: dests) {
                        RedirectDestination rd = dst.toRedirectDestination();
                        for (Boolean output: bools) {
                            for (List<VtnFlowAction> acts: actionLists) {
                                VtnRedirectFilter vrf =
                                    new VtnRedirectFilterBuilder().
                                    setRedirectDestination(rd).
                                    setOutput(output).
                                    build();
                                VtnRedirectFilterCase redirect =
                                    new VtnRedirectFilterCaseBuilder().
                                    setVtnRedirectFilter(vrf).
                                    build();

                                VtnFlowFilter vff = new VtnFlowFilterBuilder().
                                    setIndex(index).
                                    setCondition(vcond).
                                    setVtnFlowFilterType(redirect).
                                    setVtnFlowAction(acts).
                                    build();
                                VTNRedirectFilter vredirect =
                                    new VTNRedirectFilter(vff, redirect);
                                VTNRedirectFilter d =
                                    jaxbTest(vredirect, type, m, um, XML_ROOT);
                                d.verify();
                            }
                        }
                    }
                }
            }

            // Ensure that broken values in XML can be detected.
            jaxbErrorTest(um, type, dlist);

            // Ensure that flow actions are sorted in ascending order of
            // action order.
            VTNRedirectFilter vredirect = unmarshal(um, sortTestXml, type);
            vredirect.verify();
            assertEquals(xindex, vredirect.getIdentifier().intValue());
            assertEquals(xcond, vredirect.getCondition());
            VtnRedirectFilter  xvrf = vredirect.getVtnFlowFilterType().
                getVtnRedirectFilter();
            RedirectDestination xrdest = xvrf.getRedirectDestination();
            assertEquals(xdstbr, xrdest.getBridgeName());
            assertEquals(xdstif, xrdest.getInterfaceName());
            assertEquals(Boolean.valueOf(xoutput), xvrf.isOutput());

            Iterator<Map.Entry<Integer, VtnFlowAction>> it =
                actMap.entrySet().iterator();
            for (FlowFilterAction ffact: vredirect.getActions()) {
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

        // No vtn-redirect-filter container.
        xml = new XmlNode(XML_ROOT).
            add(new XmlNode("index", 1)).
            add(new XmlNode("condition", "cond")).
            toString();
        missingCases.put(xml, "redirect-destination cannot be null");

        // No vBridge name.
        xml = new XmlNode(XML_ROOT).
            add(new XmlNode("index", 1)).
            add(new XmlNode("condition", "cond")).
            add(new XmlNode("vbridge-if").
                add(new XmlNode("interface-name", "if_1"))).
            toString();
        missingCases.put(xml, "vBridge name cannot be null");

        // No vTerminal name.
        xml = new XmlNode(XML_ROOT).
            add(new XmlNode("index", 1)).
            add(new XmlNode("condition", "cond")).
            add(new XmlNode("vterminal-if").
                add(new XmlNode("interface-name", "if_1"))).
            toString();
        missingCases.put(xml, "vTerminal name cannot be null");

        // No interface name.
        String ifMissing = "vInterface name cannot be null";
        xml = new XmlNode(XML_ROOT).
            add(new XmlNode("index", 1)).
            add(new XmlNode("condition", "cond")).
            add(new XmlNode("vbridge-if").
                add(new XmlNode("bridge-name", "vbr1"))).
            toString();
        missingCases.put(xml, ifMissing);

        xml = new XmlNode(XML_ROOT).
            add(new XmlNode("index", 1)).
            add(new XmlNode("condition", "cond")).
            add(new XmlNode("vterminal-if").
                add(new XmlNode("bridge-name", "vterm1"))).
            toString();
        missingCases.put(xml, ifMissing);

        // No action order.
        xml = new XmlNode(XML_ROOT).
            add(new XmlNode("index", 1)).
            add(new XmlNode("condition", "cond")).
            add(new XmlNode("vbridge-if").
                add(new XmlNode("bridge-name", "vbr1")).
                add(new XmlNode("interface-name", "if1"))).
            add(new XmlNode("actions").
                add(new XmlNode("vtn-set-icmp-code").
                    add(new XmlNode("code", 0)))).
            toString();
        missingCases.put(xml,
                         "VTNSetIcmpCodeAction: Action order cannot be null");

        for (Map.Entry<String, String> entry: missingCases.entrySet()) {
            String x = entry.getKey();
            VTNRedirectFilter vredirect = unmarshal(um, x, type);
            try {
                vredirect.verify();
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
                add(new XmlNode("vbridge-if").
                    add(new XmlNode("bridge-name", "vbr1")).
                    add(new XmlNode("interface-name", "if1"))).
                toString();
            String msg = "Invalid filter index: " + index;
            VTNRedirectFilter vredirect = unmarshal(um, xml, type);
            try {
                vredirect.verify();
                unexpected();
            } catch (RpcException e) {
                assertEquals(etag, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                assertEquals(msg, e.getMessage());
            }
        }

        Map<String, String> broken = new HashMap<>();

        // Duplicate action order.
        int dup = 33333;
        xml = new XmlNode(XML_ROOT).
            add(new XmlNode("index", 1)).
            add(new XmlNode("condition", "cond")).
            add(new XmlNode("vbridge-if").
                add(new XmlNode("bridge-name", "vbr1")).
                add(new XmlNode("interface-name", "if1"))).
            add(new XmlNode("actions").
                add(new XmlNode("vtn-set-icmp-code").
                    add(new XmlNode("order", dup)).
                    add(new XmlNode("code", 1))).
                add(new XmlNode("vtn-set-port-dst").
                    add(new XmlNode("order", 1)).
                    add(new XmlNode("port", 999))).
                add(new XmlNode("vtn-set-dl-dst").
                    add(new XmlNode("order", dup)).
                    add(new XmlNode("address", "00:11:22:33:44:55")))).
            toString();
        broken.put(xml, "Duplicate action order: " + dup);

        // Invalid flow action.
        short pcp = 8;
        xml = new XmlNode(XML_ROOT).
            add(new XmlNode("index", 1)).
            add(new XmlNode("condition", "cond")).
            add(new XmlNode("actions").
                add(new XmlNode("vtn-set-vlan-pcp").
                    add(new XmlNode("order", 1)).
                    add(new XmlNode("priority", pcp)))).
            toString();
        broken.put(xml,
                   "VTNSetVlanPcpAction: Invalid VLAN priority: " + pcp);

        for (Map.Entry<String, String> entry: broken.entrySet()) {
            VTNRedirectFilter vredirect = unmarshal(um, entry.getKey(), type);
            try {
                vredirect.verify();
                unexpected();
            } catch (RpcException e) {
                assertEquals(etag, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                assertEquals(entry.getValue(), e.getMessage());
            }
        }
    }
}
