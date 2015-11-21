/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.flow.filter;

import static org.opendaylight.vtn.manager.internal.util.flow.action.FlowActionUtilsTest.createVtnSetInetDscpAction;
import static org.opendaylight.vtn.manager.internal.util.flow.action.FlowActionUtilsTest.createVtnSetInetSrcAction;
import static org.opendaylight.vtn.manager.internal.util.flow.action.FlowActionUtilsTest.createVtnSetVlanPcpAction;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

import org.junit.Test;

import org.opendaylight.vtn.manager.util.Ip4Network;

import org.opendaylight.vtn.manager.internal.util.rpc.RpcErrorTag;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;
import org.opendaylight.vtn.manager.internal.util.vnode.VBridgeIdentifier;
import org.opendaylight.vtn.manager.internal.util.vnode.VBridgeIfIdentifier;
import org.opendaylight.vtn.manager.internal.util.vnode.VInterfaceIdentifier;
import org.opendaylight.vtn.manager.internal.util.vnode.VNodeIdentifier;
import org.opendaylight.vtn.manager.internal.util.vnode.VTenantIdentifier;
import org.opendaylight.vtn.manager.internal.util.vnode.VTerminalIfIdentifier;

import org.opendaylight.vtn.manager.internal.TestBase;
import org.opendaylight.vtn.manager.internal.XmlDataType;
import org.opendaylight.vtn.manager.internal.XmlValueType;
import org.opendaylight.vtn.manager.internal.util.flow.action.VTNSetDlDstActionTest;
import org.opendaylight.vtn.manager.internal.util.flow.action.VTNSetDlSrcActionTest;
import org.opendaylight.vtn.manager.internal.util.flow.action.VTNSetIcmpCodeActionTest;
import org.opendaylight.vtn.manager.internal.util.flow.action.VTNSetIcmpTypeActionTest;
import org.opendaylight.vtn.manager.internal.util.flow.action.VTNSetInetDscpActionTest;
import org.opendaylight.vtn.manager.internal.util.flow.action.VTNSetInetDstActionTest;
import org.opendaylight.vtn.manager.internal.util.flow.action.VTNSetInetSrcActionTest;
import org.opendaylight.vtn.manager.internal.util.flow.action.VTNSetPortDstActionTest;
import org.opendaylight.vtn.manager.internal.util.flow.action.VTNSetPortSrcActionTest;
import org.opendaylight.vtn.manager.internal.util.flow.action.VTNSetVlanPcpActionTest;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;
import org.opendaylight.yangtools.yang.binding.InstanceIdentifier.InstanceIdentifierBuilder;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.flow.action.list.VtnFlowAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.VtnFlowFilterConfig;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.flow.filter.list.VtnFlowFilter;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.flow.filter.list.VtnFlowFilterBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.flow.filter.list.VtnFlowFilterKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.flow.filter.type.fields.vtn.flow.filter.type.VtnDropFilterCase;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.flow.filter.type.fields.vtn.flow.filter.type.VtnDropFilterCaseBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.flow.filter.type.fields.vtn.flow.filter.type.VtnPassFilterCase;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.flow.filter.type.fields.vtn.flow.filter.type.VtnPassFilterCaseBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.flow.filter.type.fields.vtn.flow.filter.type.VtnRedirectFilterCase;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.flow.filter.type.fields.vtn.flow.filter.type.VtnRedirectFilterCaseBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.flow.filter.type.fields.vtn.flow.filter.type.vtn.drop.filter._case.VtnDropFilterBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.flow.filter.type.fields.vtn.flow.filter.type.vtn.pass.filter._case.VtnPassFilterBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.flow.filter.type.fields.vtn.flow.filter.type.vtn.redirect.filter._case.VtnRedirectFilter;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.flow.filter.type.fields.vtn.flow.filter.type.vtn.redirect.filter._case.VtnRedirectFilterBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.redirect.filter.config.RedirectDestination;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.redirect.filter.config.RedirectDestinationBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.Vtns;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.vtn.info.VtnInputFilter;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.vtns.Vtn;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.vtns.VtnKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeName;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnErrorTag;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.vbridge.info.VbridgeInputFilter;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.vbridge.info.VbridgeOutputFilter;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.vbridge.list.Vbridge;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.vbridge.list.VbridgeKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.vtn.mappable.vinterface.list.Vinterface;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.vtn.mappable.vinterface.list.VinterfaceKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.vtn.vinterface.info.VinterfaceInputFilter;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.vtn.vinterface.info.VinterfaceOutputFilter;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vterminal.rev150907.vtn.vterminal.list.Vterminal;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vterminal.rev150907.vtn.vterminal.list.VterminalKey;

/**
 * JUnit test for {@link VTNFlowFilter}.
 */
public class VTNFlowFilterTest extends TestBase {
    /**
     * Return a list of {@link XmlDataType} instances that specifies XML node
     * types mapped to a {@link VTNFlowFilter} instance.
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
            new XmlValueType("condition",
                             VnodeName.class).add(name).prepend(parent));

        String[] p = XmlDataType.addPath(
            "actions", XmlDataType.addPath(name, parent));
        dlist.addAll(VTNSetDlSrcActionTest.
                     getXmlDataTypes("vtn-set-dl-src", p));
        dlist.addAll(VTNSetDlDstActionTest.
                     getXmlDataTypes("vtn-set-dl-dst", p));
        dlist.addAll(VTNSetInetSrcActionTest.
                     getXmlDataTypes("vtn-set-inet-src", p));
        dlist.addAll(VTNSetInetDstActionTest.
                     getXmlDataTypes("vtn-set-inet-dst", p));
        dlist.addAll(VTNSetInetDscpActionTest.
                     getXmlDataTypes("vtn-set-inet-dscp", p));
        dlist.addAll(VTNSetPortSrcActionTest.
                     getXmlDataTypes("vtn-set-port-src", p));
        dlist.addAll(VTNSetPortDstActionTest.
                     getXmlDataTypes("vtn-set-port-dst", p));
        dlist.addAll(VTNSetIcmpTypeActionTest.
                     getXmlDataTypes("vtn-set-icmp-type", p));
        dlist.addAll(VTNSetIcmpCodeActionTest.
                     getXmlDataTypes("vtn-set-icmp-code", p));
        dlist.addAll(VTNSetVlanPcpActionTest.
                     getXmlDataTypes("vtn-set-vlan-pcp", p));

        return dlist;
    }

    /**
     * Test case for {@link VTNFlowFilter#isOutput(InstanceIdentifier)}.
     */
    @Test
    public void testIsOutput() {
        VtnKey vtnKey = new VtnKey(new VnodeName("vtn_1"));
        VbridgeKey vbrKey = new VbridgeKey(new VnodeName("vbridge_1"));
        VterminalKey vtmKey = new VterminalKey(new VnodeName("vterm_1"));
        VinterfaceKey vifKey = new VinterfaceKey(new VnodeName("if_1"));
        VtnFlowFilterKey key = new VtnFlowFilterKey(123);

        InstanceIdentifierBuilder<Vtn> vtn = InstanceIdentifier.
            builder(Vtns.class).
            child(Vtn.class, vtnKey);
        InstanceIdentifierBuilder<Vbridge> vbr =
            vtn.child(Vbridge.class, vbrKey);
        InstanceIdentifierBuilder<Vterminal> vtm =
            vtn.child(Vterminal.class, vtmKey);
        InstanceIdentifierBuilder<Vinterface> vbrIf =
            vbr.child(Vinterface.class, vifKey);
        InstanceIdentifierBuilder<Vinterface> vtmIf =
            vtm.child(Vinterface.class, vifKey);

        // Input filters.
        Class<VtnFlowFilter> type = VtnFlowFilter.class;
        List<InstanceIdentifier<VtnFlowFilter>> paths = new ArrayList<>();
        Collections.addAll(
            paths,
            vtn.child(VtnInputFilter.class).child(type, key).build(),
            vbr.child(VbridgeInputFilter.class).child(type, key).build(),
            vbrIf.child(VinterfaceInputFilter.class).child(type, key).build(),
            vtmIf.child(VinterfaceInputFilter.class).child(type, key).build());
        for (InstanceIdentifier<VtnFlowFilter> path: paths) {
            assertEquals(false, VTNFlowFilter.isOutput(path));
        }

        // Output filters.
        paths.clear();
        Collections.addAll(
            paths,
            vbr.child(VbridgeOutputFilter.class).child(type, key).build(),
            vbrIf.child(VinterfaceOutputFilter.class).child(type, key).build(),
            vtmIf.child(VinterfaceOutputFilter.class).child(type, key).build());
        for (InstanceIdentifier<VtnFlowFilter> path: paths) {
            assertEquals(true, VTNFlowFilter.isOutput(path));
        }
    }

    /**
     * Test case for
     * {@link VTNFlowFilter#getTypeDescription(VtnFlowFilterConfig)}.
     */
    @Test
    public void testGetTypeDescription() {
        VtnFlowFilter vff = new VtnFlowFilterBuilder().build();
        assertEquals("UNKNOWN", VTNFlowFilter.getTypeDescription(vff));

        VtnPassFilterCase pass = new VtnPassFilterCaseBuilder().
            setVtnPassFilter(new VtnPassFilterBuilder().build()).
            build();
        vff = new VtnFlowFilterBuilder().
            setVtnFlowFilterType(pass).
            build();
        assertEquals("PASS", VTNFlowFilter.getTypeDescription(vff));

        VtnDropFilterCase drop = new VtnDropFilterCaseBuilder().
            setVtnDropFilter(new VtnDropFilterBuilder().build()).
            build();
        vff = new VtnFlowFilterBuilder().
            setVtnFlowFilterType(drop).
            build();
        assertEquals("DROP", VTNFlowFilter.getTypeDescription(vff));

        VtnRedirectFilterCase redirect = new VtnRedirectFilterCaseBuilder().
            setVtnRedirectFilter(new VtnRedirectFilterBuilder().build()).
            build();
        vff = new VtnFlowFilterBuilder().
            setVtnFlowFilterType(redirect).
            build();
        assertEquals("REDIRECT", VTNFlowFilter.getTypeDescription(vff));
    }

    /**
     * Test case for {@link VTNFlowFilter#create(VtnFlowFilterConfig)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testCreate() throws Exception {
        // Null flow filter.
        RpcErrorTag etag = RpcErrorTag.MISSING_ELEMENT;
        VtnErrorTag vtag = VtnErrorTag.BADREQUEST;
        String msg = "vtn-flow-filter cannot be null";
        try {
            VtnFlowFilterConfig vffc = null;
            VTNFlowFilter.create(vffc);
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            assertEquals(vtag, e.getVtnErrorTag());
            assertEquals(msg, e.getMessage());
        }

        // No flow filter type.
        msg = "vtn-flow-filter-type cannot be null";
        try {
            VtnFlowFilter vff = new VtnFlowFilterBuilder().build();
            VTNFlowFilter.create(vff);
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            assertEquals(vtag, e.getVtnErrorTag());
            assertEquals(msg, e.getMessage());
        }

        // Unsupported flow filter type.
        UnknownFlowFilterCase unknown = new UnknownFlowFilterCase();
        etag = RpcErrorTag.BAD_ELEMENT;
        msg = "Unexpected flow filter type: " + unknown;
        try {
            VtnFlowFilter vff = new VtnFlowFilterBuilder().
                setVtnFlowFilterType(unknown).
                build();
            VTNFlowFilter.create(vff);
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            assertEquals(vtag, e.getVtnErrorTag());
            assertEquals(msg, e.getMessage());
        }

        int pcp = 7;
        int dscp = 32;
        int pcpOrder = 10;
        int dscpOrder = 99;
        List<VtnFlowAction> actions = new ArrayList<>();
        Collections.addAll(
            actions,
            createVtnSetInetDscpAction(dscpOrder, dscp),
            createVtnSetVlanPcpAction(pcpOrder, pcp));
        VnodeName vcond = new VnodeName("cond");
        int index = 10;

        // Drop filter.
        VtnDropFilterCase drop = new VtnDropFilterCaseBuilder().
            setVtnDropFilter(new VtnDropFilterBuilder().build()).
            build();
        VtnFlowFilter vff = new VtnFlowFilterBuilder().
            setIndex(index).
            setCondition(vcond).
            setVtnFlowFilterType(drop).
            setVtnFlowAction(actions).
            build();
        VTNFlowFilter expected = new VTNDropFilter(vff, drop);
        assertEquals(expected, VTNFlowFilter.create(vff));

        // Pass filter.
        VtnPassFilterCase pass = new VtnPassFilterCaseBuilder().
            setVtnPassFilter(new VtnPassFilterBuilder().build()).
            build();
        vff = new VtnFlowFilterBuilder().
            setIndex(index).
            setCondition(vcond).
            setVtnFlowFilterType(pass).
            build();
        expected = new VTNPassFilter(vff, pass);
        assertEquals(expected, VTNFlowFilter.create(vff));

        // Redirect filter.
        String bname = "bridge";
        String iname = "if";
        RedirectDestination[] rdests = {
            new RedirectDestinationBuilder().setBridgeName(bname).
            setInterfaceName(iname).build(),
            new RedirectDestinationBuilder().setTerminalName(bname).
            setInterfaceName(iname).build(),
        };
        for (RedirectDestination rd: rdests) {
            VtnRedirectFilterCase redirect = new VtnRedirectFilterCaseBuilder().
                setVtnRedirectFilter(new VtnRedirectFilterBuilder().
                                     setRedirectDestination(rd).build()).
                build();
            vff = new VtnFlowFilterBuilder().
                setIndex(index).
                setCondition(vcond).
                setVtnFlowFilterType(redirect).
                build();
            expected = new VTNRedirectFilter(vff, redirect);
            assertEquals(expected, VTNFlowFilter.create(vff));
        }
    }

    /**
     * Test case for {@link VTNFlowFilter#checkIndexNotNull(Integer)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testCheckIndexNotNull() throws Exception {
        Integer[] indices = {1, 10, 40, 33333, 65535};
        for (Integer index: indices) {
            assertSame(index, VTNFlowFilter.checkIndexNotNull(index));
        }

        RpcErrorTag etag = RpcErrorTag.MISSING_ELEMENT;
        VtnErrorTag vtag = VtnErrorTag.BADREQUEST;
        String msg = "Filter index cannot be null";
        try {
            VTNFlowFilter.checkIndexNotNull((Integer)null);
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            assertEquals(vtag, e.getVtnErrorTag());
            assertEquals(msg, e.getMessage());
        }
    }

    /**
     * Test case for {@link VTNFlowFilter#getPathString(FlowFilterListId)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testGetPathString() throws Exception {
        VtnDropFilterCase drop = new VtnDropFilterCaseBuilder().
            setVtnDropFilter(new VtnDropFilterBuilder().build()).
            build();
        List<VNodeIdentifier<?>> identifiers = new ArrayList<>();
        Collections.addAll(
            identifiers,
            VTenantIdentifier.create("vtn_1", false),
            VBridgeIdentifier.create("vtn_1", "vbr_1", false),
            VBridgeIfIdentifier.create("vtn_2", "vbr_2", "if_1", false),
            VTerminalIfIdentifier.create("vtn_2", "vterm_2", "if_2", false));
        boolean[] bools = {true, false};
        int[] indices = {1, 3, 100, 4567, 12345, 40000, 65535};
        VnodeName vcond = new VnodeName("cond");

        for (int index: indices) {
            VtnFlowFilter vff = new VtnFlowFilterBuilder().
                setIndex(index).
                setCondition(vcond).
                setVtnFlowFilterType(drop).
                build();
            VTNDropFilter vdrop = new VTNDropFilter(vff, drop);
            for (boolean out: bools) {
                String direction = (out) ? "OUT" : "IN";
                for (VNodeIdentifier<?> ident: identifiers) {
                    String expected = ident.toString() + "%" + direction +
                        "." + index;
                    FlowFilterListId flid = new FlowFilterListId(ident, out);
                    assertEquals(expected, vdrop.getPathString(flid));
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
        Set<VTNFlowFilter> set = new HashSet<>();
        List<VTNFlowFilter> filters = new ArrayList<>();
        assertEquals(true, set.add(null));

        Integer[] indices = {1, 30000, 65535};
        String[] conditions = {"cond_1", "cond_2", "cond_3"};
        Boolean[] bools = {Boolean.TRUE, Boolean.FALSE};

        VInterfaceIdentifier[] dests = {
            new VBridgeIfIdentifier(
                null, new VnodeName("vbr1"), new VnodeName("if1")),
            new VBridgeIfIdentifier(
                null, new VnodeName("vbr2"), new VnodeName("if1")),
            new VTerminalIfIdentifier(
                null, new VnodeName("vbr1"), new VnodeName("if1")),
        };

        List<List<VtnFlowAction>> actionLists = new ArrayList<>();
        actionLists.add(null);

        Ip4Network nwSrc = new Ip4Network("192.168.111.222");
        List<VtnFlowAction> actions = new ArrayList<>();
        Collections.addAll(
            actions,
            createVtnSetVlanPcpAction(0, 2),
            createVtnSetInetDscpAction(1, 51),
            createVtnSetInetSrcAction(2, nwSrc.getMdAddress()));
        actionLists.add(actions);

        VtnDropFilterCase drop = new VtnDropFilterCaseBuilder().
            setVtnDropFilter(new VtnDropFilterBuilder().build()).
            build();
        VtnPassFilterCase pass = new VtnPassFilterCaseBuilder().
            setVtnPassFilter(new VtnPassFilterBuilder().build()).
            build();

        for (String cond: conditions) {
            VnodeName vcond = new VnodeName(cond);
            for (Integer index: indices) {
                for (List<VtnFlowAction> acts: actionLists) {
                    VtnFlowFilter vff = new VtnFlowFilterBuilder().
                        setIndex(index).
                        setCondition(vcond).
                        setVtnFlowAction(acts).
                        setVtnFlowFilterType(drop).
                        build();
                    VTNFlowFilter ff = VTNFlowFilter.create(vff);
                    assertTrue(ff instanceof VTNDropFilter);
                    assertEquals(true, set.add(ff));
                    filters.add(VTNFlowFilter.create(vff));

                    vff = new VtnFlowFilterBuilder().
                        setIndex(index).
                        setCondition(vcond).
                        setVtnFlowAction(acts).
                        setVtnFlowFilterType(pass).
                        build();
                    ff = VTNFlowFilter.create(vff);
                    assertTrue(ff instanceof VTNPassFilter);
                    assertEquals(true, set.add(ff));
                    filters.add(VTNFlowFilter.create(vff));

                    for (VInterfaceIdentifier dst: dests) {
                        RedirectDestination rd = dst.toRedirectDestination();
                        for (Boolean output: bools) {
                            VtnRedirectFilter vrf =
                                new VtnRedirectFilterBuilder().
                                setRedirectDestination(rd).
                                setOutput(output).
                                build();
                            VtnRedirectFilterCase redirect =
                                new VtnRedirectFilterCaseBuilder().
                                setVtnRedirectFilter(vrf).
                                build();
                            vff = new VtnFlowFilterBuilder().
                                setIndex(index).
                                setCondition(vcond).
                                setVtnFlowAction(acts).
                                setVtnFlowFilterType(redirect).
                                build();
                            ff = VTNFlowFilter.create(vff);
                            assertTrue(ff instanceof VTNRedirectFilter);
                            assertEquals(true, set.add(ff));
                            filters.add(VTNFlowFilter.create(vff));
                        }
                    }
                }
            }
        }

        assertEquals(filters.size() + 1, set.size());
        assertEquals(true, set.remove(null));
        for (VTNFlowFilter ff: filters) {
            assertEquals(true, set.remove(ff));
        }

        assertTrue(set.isEmpty());
    }
}
