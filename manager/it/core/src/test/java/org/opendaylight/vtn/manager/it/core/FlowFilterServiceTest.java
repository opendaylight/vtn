/*
 * Copyright (c) 2015, 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.core;

import static org.opendaylight.vtn.manager.it.core.VTNManagerIT.LOG;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map.Entry;
import java.util.Map;
import java.util.Random;
import java.util.Set;

import org.opendaylight.vtn.manager.util.EtherAddress;

import org.opendaylight.vtn.manager.it.util.RpcErrorTag;
import org.opendaylight.vtn.manager.it.util.VirtualNetwork;
import org.opendaylight.vtn.manager.it.util.flow.action.FlowAction;
import org.opendaylight.vtn.manager.it.util.flow.action.VTNDropAction;
import org.opendaylight.vtn.manager.it.util.flow.action.VTNPopVlanAction;
import org.opendaylight.vtn.manager.it.util.flow.action.VTNPushVlanAction;
import org.opendaylight.vtn.manager.it.util.flow.action.VTNSetDlDstAction;
import org.opendaylight.vtn.manager.it.util.flow.action.VTNSetDlSrcAction;
import org.opendaylight.vtn.manager.it.util.flow.action.VTNSetIcmpTypeAction;
import org.opendaylight.vtn.manager.it.util.flow.action.VTNSetInetDscpAction;
import org.opendaylight.vtn.manager.it.util.flow.action.VTNSetVlanIdAction;
import org.opendaylight.vtn.manager.it.util.flow.action.VTNSetVlanPcpAction;
import org.opendaylight.vtn.manager.it.util.flow.filter.DropFilter;
import org.opendaylight.vtn.manager.it.util.flow.filter.FlowFilter;
import org.opendaylight.vtn.manager.it.util.flow.filter.FlowFilterList;
import org.opendaylight.vtn.manager.it.util.flow.filter.PassFilter;
import org.opendaylight.vtn.manager.it.util.flow.filter.RedirectFilter;
import org.opendaylight.vtn.manager.it.util.vnode.FlowFilterNode;
import org.opendaylight.vtn.manager.it.util.vnode.VBridgeIdentifier;
import org.opendaylight.vtn.manager.it.util.vnode.VInterfaceIdentifier;
import org.opendaylight.vtn.manager.it.util.vnode.VNodeIdentifier;
import org.opendaylight.vtn.manager.it.util.vnode.VNodeType;
import org.opendaylight.vtn.manager.it.util.vnode.VTenantIdentifier;
import org.opendaylight.vtn.manager.it.util.vnode.VTerminalIfIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.RemoveFlowFilterInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.RemoveFlowFilterInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.SetFlowFilterInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.SetFlowFilterInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.VtnFlowFilterService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.flow.filter.list.VtnFlowFilter;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.flow.filter.list.VtnFlowFilterBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.virtual.route.info.VirtualNodePath;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.virtual.route.info.VirtualNodePathBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeName;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnErrorTag;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;

/**
 * Test case for {@link VtnFlowFilterService}.
 */
public final class FlowFilterServiceTest extends TestMethodBase {
    /**
     * Construct a new instance.
     *
     * @param vit  A {@link VTNManagerIT} instance.
     */
    public FlowFilterServiceTest(VTNManagerIT vit) {
        super(vit);
    }

    /**
     * Test case for {@link VtnFlowFilterService}.
     *
     * @param ffSrv   The vtn-flow-filter service.
     * @param vnet    A {@link VirtualNetwork} instance.
     * @param rand    A pseudo random generator.
     * @param ident   The identifier for the target virtual node.
     * @param out     {@code true} indicates the flow filter for outgoing
     *                packets.
     * @throws Exception  An error occurred.
     */
    private void testFlowFilterService(
        VtnFlowFilterService ffSrv, VirtualNetwork vnet, Random rand,
        VNodeIdentifier<?> ident, boolean out) throws Exception {
        LOG.debug("testFlowFilterService(): node={}, output={}", ident, out);

        // Create PASS flow filter with all supported actions.
        FlowFilterList flist = vnet.getFlowFilterList(ident, out);
        Set<Integer> idxSet = new HashSet<>();
        Integer passIdx = createVtnIndex(rand, idxSet);
        PassFilter pass = new PassFilter(passIdx, "cond_1");
        pass.getFlowActions().addAll(rand);
        flist.add(pass);
        assertEquals(VtnUpdateType.CREATED, pass.update(ffSrv, ident, out));
        vnet.verify();

        assertEquals(null, pass.update(ffSrv, ident, out));
        vnet.verify();

        // Create one more PASS filter with 3 actions.
        Boolean b = (out) ? Boolean.TRUE : null;
        Integer passIdx1 = createVtnIndex(rand, idxSet);
        PassFilter pass1 = new PassFilter(passIdx1, "cond_2");
        pass1.getFlowActions().
            add(new VTNSetDlSrcAction(1, createEtherAddress(rand))).
            add(new VTNSetVlanPcpAction(2, createVlanPcp(rand))).
            add(new VTNSetIcmpTypeAction(3, createUnsignedByte(rand)));
        flist.add(pass1);
        assertEquals(VtnUpdateType.CREATED, pass1.update(ffSrv, ident, b));
        vnet.verify();

        // Create DROP action without action.
        Integer dropIdx = createVtnIndex(rand, idxSet);
        DropFilter drop = new DropFilter(dropIdx, "cond_3");
        flist.add(drop);

        Map<Integer, VtnUpdateType> resMap = new HashMap<>();
        resMap.put(passIdx, null);
        resMap.put(passIdx1, null);
        resMap.put(dropIdx, VtnUpdateType.CREATED);
        assertEquals(resMap, flist.update(ffSrv, ident, out));
        vnet.verify();

        // Append 2 actions into the DROP filter.
        drop.getFlowActions().
            add(new VTNSetInetDscpAction(10, createDscp(rand))).
            add(new VTNSetDlSrcAction(20, createEtherAddress(rand)));
        assertEquals(VtnUpdateType.CHANGED, drop.update(ffSrv, ident, out));
        vnet.verify();

        // Create 3 REDIRECT filters.
        // VTN name in destination should be always ignored.
        FlowFilterList flist1 = new FlowFilterList();
        Integer redirectIdx1 = createVtnIndex(rand, idxSet);
        VirtualNodePath dst1 =
            newBridgePath("vtn_12345678", "vbridge_12345", "if_1");
        RedirectFilter redirect1 = new RedirectFilter(
            redirectIdx1, "cond_4", dst1, null);
        flist.add(redirect1);
        flist1.add(redirect1);

        Integer redirectIdx2 = createVtnIndex(rand, idxSet);
        VirtualNodePath dst2 = newTerminalPath(null, "vterm_12", "if_123");
        RedirectFilter redirect2 = new RedirectFilter(
            redirectIdx2, "cond_5", dst2, Boolean.TRUE);
        flist.add(redirect2);
        flist1.add(redirect2);

        Integer redirectIdx3 = createVtnIndex(rand, idxSet);
        VirtualNodePath dst3 = newBridgePath(null, "vbridge_12", "if_12345");
        RedirectFilter redirect3 = new RedirectFilter(
            redirectIdx3, "cond_6", dst3, Boolean.FALSE);
        flist.add(redirect3);
        flist1.add(redirect3);

        resMap.clear();
        resMap.put(redirectIdx1, VtnUpdateType.CREATED);
        resMap.put(redirectIdx2, VtnUpdateType.CREATED);
        resMap.put(redirectIdx3, VtnUpdateType.CREATED);
        assertEquals(resMap, flist1.update(ffSrv, ident, out));
        vnet.verify();

        // Update destination of redirect2.
        VirtualNodePath dst4 = newTerminalPath(null, "vterm_999", "if_200");
        redirect1.setDestination(dst4);
        resMap.put(redirectIdx1, VtnUpdateType.CHANGED);
        resMap.put(redirectIdx2, null);
        resMap.put(redirectIdx3, null);
        assertEquals(resMap, flist1.update(ffSrv, ident, out));
        vnet.verify();

        resMap.put(redirectIdx1, null);
        assertEquals(resMap, flist1.update(ffSrv, ident, out));
        vnet.verify();

        // Error tests.

        // Invalid index.
        resMap.clear();
        List<Integer> badIndices = Arrays.asList(-1, 0, 65536, 65537, 100000);
        for (Integer idx: badIndices) {
            resMap.put(idx, null);
        }
        assertEquals(resMap, removeFlowFilter(ident, out, badIndices));

        // No flow filter list.
        final VirtualNodePath vpath = ident.getVirtualNodePath();
        SetFlowFilterInputBuilder builder = new SetFlowFilterInputBuilder().
            setOutput(b);
        builder.fieldsFrom(vpath);
        SetFlowFilterInput input = builder.build();
        checkRpcError(ffSrv.setFlowFilter(input),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);

        // Empty flow filter list.
        builder = new SetFlowFilterInputBuilder().
            setVtnFlowFilter(Collections.<VtnFlowFilter>emptyList()).
            setOutput(b);
        builder.fieldsFrom(vpath);
        input = builder.build();
        checkRpcError(ffSrv.setFlowFilter(input),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);

        // Duplicate index.
        String condition = "cond";
        List<VtnFlowFilter> dupFilters = new ArrayList<>();
        Collections.addAll(
            dupFilters,
            new PassFilter(passIdx, condition).toVtnFlowFilter(),
            new PassFilter(passIdx1, condition).toVtnFlowFilter(),
            new PassFilter(dropIdx, condition).toVtnFlowFilter(),
            new PassFilter(redirectIdx1, condition).toVtnFlowFilter(),
            new PassFilter(passIdx, condition).toVtnFlowFilter());
        builder = new SetFlowFilterInputBuilder().
            setVtnFlowFilter(dupFilters).
            setOutput(b);
        builder.fieldsFrom(vpath);
        input = builder.build();
        checkRpcError(ffSrv.setFlowFilter(input),
                      RpcErrorTag.BAD_ELEMENT, VtnErrorTag.BADREQUEST);

        // No flow condition name.
        PassFilter pf = new PassFilter(passIdx, null);
        input = pf.newInputBuilder(ident, out).build();
        checkRpcError(ffSrv.setFlowFilter(input),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);

        // No flow filter type.
        VnodeName vcond = new VnodeName(condition);
        VtnFlowFilter vff = new VtnFlowFilterBuilder().
            setIndex(passIdx).
            setCondition(vcond).
            build();
        builder = new SetFlowFilterInputBuilder().
            setVtnFlowFilter(Collections.singletonList(vff)).
            setOutput(b);
        builder.fieldsFrom(vpath);
        input = builder.build();
        checkRpcError(ffSrv.setFlowFilter(input),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);

        // Invalid action.
        Integer actIdx = 1;
        List<FlowAction<?>> badActions = new ArrayList<>();
        Collections.addAll(
            badActions,

            // Zero MAC address.
            new VTNSetDlSrcAction(actIdx, new EtherAddress(0L)),

            // Broadcast address.
            new VTNSetDlDstAction(actIdx, EtherAddress.BROADCAST),

            // Multicast address.
            new VTNSetDlDstAction(actIdx, new EtherAddress(0x010000000000L)),

            // Unsupported action.
            new VTNDropAction(actIdx),
            new VTNPushVlanAction(actIdx),
            new VTNPopVlanAction(actIdx),
            new VTNSetVlanIdAction(actIdx, 10));

        for (FlowAction<?> act: badActions) {
            PassFilter bad = new PassFilter(passIdx, condition);
            bad.getFlowActions().add(act);
            input = bad.newInputBuilder(ident, out).build();
            checkRpcError(ffSrv.setFlowFilter(input),
                          RpcErrorTag.BAD_ELEMENT, VtnErrorTag.BADREQUEST);
        }

        // Invalid destination of REDIRECT filter.
        String bname = "bridge";
        String iname = "vif";
        Map<VirtualNodePath, RpcErrorTag> badDests = new HashMap<>();
        assertNull(badDests.put(null, RpcErrorTag.MISSING_ELEMENT));
        assertNull(badDests.put(new VirtualNodePathBuilder().build(),
                                RpcErrorTag.BAD_ELEMENT));

        // No interface name.
        assertNull(badDests.put(newBridgePath(null, bname, null),
                                RpcErrorTag.MISSING_ELEMENT));
        assertNull(badDests.put(newTerminalPath(null, bname, null),
                                RpcErrorTag.MISSING_ELEMENT));

        // No bridge name.
        assertNull(badDests.put(newBridgePath(null, null, iname),
                                RpcErrorTag.BAD_ELEMENT));

        String[] badNames = {
            "_badname",
            "",
            "12345678901234567890123456789012",
        };
        for (String name: badNames) {
            assertNull(badDests.put(newBridgePath(null, name, iname),
                                    RpcErrorTag.BAD_ELEMENT));
            assertNull(badDests.put(newBridgePath(null, bname, name),
                                    RpcErrorTag.BAD_ELEMENT));
            assertNull(badDests.put(newTerminalPath(null, name, iname),
                                    RpcErrorTag.BAD_ELEMENT));
            assertNull(badDests.put(newTerminalPath(null, bname, name),
                                    RpcErrorTag.BAD_ELEMENT));
        }

        if (ident.getType().isInterface()) {
            // Self redirection.
            assertNull(badDests.put(ident.getVirtualNodePath(),
                                    RpcErrorTag.BAD_ELEMENT));
        }

        Boolean[] bools = {null, Boolean.TRUE, Boolean.FALSE};
        for (Entry<VirtualNodePath, RpcErrorTag> entry:
                 badDests.entrySet()) {
            VirtualNodePath vp = entry.getKey();
            RpcErrorTag etag = entry.getValue();
            for (Boolean output: bools) {
                RedirectFilter rf =
                    new RedirectFilter(passIdx, condition, vp, output);
                input = rf.newInputBuilder(ident, b).build();
                checkRpcError(ffSrv.setFlowFilter(input), etag,
                              VtnErrorTag.BADREQUEST);
            }
        }

        // Errors should never affect flow filters.
        vnet.verify();

        // Remove actions in pass1.
        pass1.getFlowActions().clear();
        assertEquals(VtnUpdateType.CHANGED, pass1.update(ffSrv, ident, b));
        vnet.verify();

        // Remove pass1.
        resMap.clear();
        List<Integer> indices = new ArrayList<>();
        for (int i = 0; i < 5; i++) {
            Integer idx = createVtnIndex(rand, idxSet);
            indices.add(idx);
            resMap.put(idx, null);
        }
        resMap.put(passIdx1, VtnUpdateType.REMOVED);
        indices.add(passIdx1);
        assertEquals(resMap, removeFlowFilter(ident, out, indices));
        flist.remove(passIdx1);
        vnet.verify();

        resMap.put(passIdx1, null);
        assertEquals(resMap, removeFlowFilter(ident, out, indices));
        vnet.verify();
    }

    /**
     * Ensure that flow filter APIs returns an error if the specified virtual
     * node path is invalid.
     *
     * @param ffSrv  The vtn-flow-filter service.
     * @param vpath  A {@link VirtualNodePath} instance that specifies the
     *               target virtual node.
     * @param etag   The {@link RpcErrorTag} to be returned by flow filter
     *               APIs.
     * @param vtag   The {@link VtnErrorTag} to be returned by flow filter
     *               APIs.
     */
    private void testInvalidNodeFlowFilter(
        VtnFlowFilterService ffSrv, VirtualNodePath vpath, RpcErrorTag etag,
        VtnErrorTag vtag) {
        Boolean[] bools = {null, Boolean.TRUE, Boolean.FALSE};
        String condition = "cond";
        Integer idx = 1;
        List<Integer> indices = Collections.singletonList(idx);

        for (Boolean out: bools) {
            PassFilter pf = new PassFilter(idx, condition);
            SetFlowFilterInputBuilder ib = pf.newInputBuilder(null, out);
            if (vpath != null) {
                ib.fieldsFrom(vpath);
            }
            SetFlowFilterInput input = ib.build();
            checkRpcError(ffSrv.setFlowFilter(input), etag, vtag);

            RemoveFlowFilterInputBuilder rib =
                new RemoveFlowFilterInputBuilder();
            if (vpath != null) {
                rib.fieldsFrom(vpath);
            }
            RemoveFlowFilterInput rin = rib.
                setOutput(out).
                setIndices(indices).
                build();
            checkRpcError(ffSrv.removeFlowFilter(rin), etag, vtag);
        }
    }

    /**
     * Restore flow filters.
     *
     * @param ffSrv    The vtn-flow-filter service.
     * @param vnet     A {@link VirtualNetwork} instance.
     * @param filters  A map that keeps flow filter lists to be restored.
     * @param out      {@code true} indicates the flow filter list for
     *                 outgoing packets.
     */
    private void restoreFlowFilters(
        VtnFlowFilterService ffSrv, VirtualNetwork vnet,
        Map<VNodeIdentifier<?>, FlowFilterList> filters, boolean out) {
        for (Entry<VNodeIdentifier<?>, FlowFilterList> entry:
                 filters.entrySet()) {
            VNodeIdentifier<?> ident = entry.getKey();
            FlowFilterList saved = entry.getValue();
            FlowFilterList flist = vnet.getFlowFilterList(ident, out);
            Map<Integer, VtnUpdateType> resMap = new HashMap<>();
            for (FlowFilter<?> ff: saved.getFlowFilters()) {
                flist.add(ff);
                assertNull(resMap.put(ff.getIndex(), VtnUpdateType.CREATED));
            }
            assertEquals(resMap, flist.update(ffSrv, ident, out));
            vnet.verify();

            for (FlowFilter<?> ff: saved.getFlowFilters()) {
                assertEquals(VtnUpdateType.CREATED,
                             resMap.put(ff.getIndex(), null));
            }
            assertEquals(resMap, flist.update(ffSrv, ident, out));
            vnet.verify();
        }
    }

    /**
     * Clear the specified flow filter list.
     *
     * @param vnet   A {@link VirtualNetwork} instance.
     * @param ident  The identifier for the target virtual node.
     * @param out    {@code true} indicates the flow filter for outgoing
     *               packets.
     * @param empty  If {@code true}, specifies an empty list instead of
     *               {@code null} to remove-flow-filter input.
     */
    private void clearFlowFilter(
        VirtualNetwork vnet, VNodeIdentifier<?> ident, boolean out,
        boolean empty) {
        FlowFilterList flist = vnet.getFlowFilterList(ident, out);
        Map<Integer, VtnUpdateType> resMap = new HashMap<>();
        for (FlowFilter<?> ff: flist.getFlowFilters()) {
            assertNull(resMap.put(ff.getIndex(), VtnUpdateType.REMOVED));
        }
        assertEquals(false, resMap.isEmpty());

        List<Integer> indices = (empty)
            ? Collections.<Integer>emptyList() : null;
        assertEquals(resMap,
                     removeFlowFilter(ident, out, indices));
        flist.clear();
        vnet.verify();

        assertEquals(null,
                     removeFlowFilter(ident, out, indices));
        vnet.verify();
    }

    // TestMethodBase

    /**
     * Run the test.
     *
     * @throws Exception  An error occurred.
     */
    @Override
    protected void runTest() throws Exception {
        VTNManagerIT vit = getTest();
        VtnFlowFilterService ffSrv = vit.getFlowFilterService();

        // Create virtual nodes.
        VirtualNetwork vnet = getVirtualNetwork();
        String tname1 = "vtn_1";
        String tname2 = "vtn_2";
        String bname1 = "node_1";
        String bname2 = "node_2";
        String iname1 = "if_1";
        String iname2 = "if_2";
        VTenantIdentifier vtnId1 = new VTenantIdentifier(tname1);
        VTenantIdentifier vtnId2 = new VTenantIdentifier(tname2);
        VBridgeIdentifier vbrId1 = new VBridgeIdentifier(tname1, bname1);
        VBridgeIdentifier vbrId2 = new VBridgeIdentifier(tname1, bname2);
        VInterfaceIdentifier<?> ifId1 = vbrId1.childInterface(iname1);
        VInterfaceIdentifier<?> ifId2 = vbrId1.childInterface(iname2);
        VInterfaceIdentifier<?> ifId3 =
            new VTerminalIfIdentifier(tname1, bname1, iname1);
        VInterfaceIdentifier<?> ifId4 =
            new VTerminalIfIdentifier(tname2, bname1, iname1);
        vnet.addBridge(vbrId1, vbrId2).
            addInterface(ifId1, ifId2, ifId3, ifId4).
            apply().
            verify();

        List<VNodeIdentifier<?>> identifiers = new ArrayList<>();
        Collections.addAll(identifiers,
                           vbrId1, vbrId2,
                           ifId1, ifId2, ifId3, ifId4);

        // VTN input filter test.
        // VTN filter is always treated as input even if "output" is true.
        Random rand = new Random(22360679L);
        testFlowFilterService(ffSrv, vnet, rand, vtnId1, false);
        testFlowFilterService(ffSrv, vnet, rand, vtnId2, true);

        // vBridge/vInterface filter test.
        boolean[] bools = {true, false};
        for (VNodeIdentifier<?> ident: identifiers) {
            for (boolean output: bools) {
                testFlowFilterService(ffSrv, vnet, rand, ident, output);
            }
        }
        Collections.addAll(identifiers, vtnId1, vtnId2);

        // Error tests.

        // Null input.
        checkRpcError(ffSrv.setFlowFilter(null),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);
        checkRpcError(ffSrv.removeFlowFilter(null),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);

        // Missing path component.
        List<VirtualNodePath> badPaths = new ArrayList<>();
        Collections.addAll(
            badPaths,
            null,
            new VirtualNodePathBuilder().build(),

            // No VTN name.
            newBridgePath(null, bname1, iname1),
            newTerminalPath(null, bname1, iname1),

            // No bridge name.
            newBridgePath(tname1, null, iname1));

        for (VirtualNodePath vpath: badPaths) {
            testInvalidNodeFlowFilter(ffSrv, vpath, RpcErrorTag.MISSING_ELEMENT,
                                      VtnErrorTag.BADREQUEST);
        }

        // The target virtual node is not present.
        String unknown = "unknown_name";
        badPaths.clear();
        Collections.addAll(
            badPaths,
            newBridgePath(unknown, null, null),
            newBridgePath(tname1, unknown, null),
            newBridgePath(tname1, bname1, unknown),
            newTerminalPath(tname1, bname1, unknown));

        String[] badNames = {
            "_badname",
            "",
            "12345678901234567890123456789012",
        };
        for (String name: badNames) {
            Collections.addAll(
                badPaths,
                newBridgePath(name, null, null),
                newBridgePath(tname1, name, null),
                newBridgePath(tname1, bname1, name),
                newTerminalPath(tname1, bname1, name));
        }

        for (VirtualNodePath vpath: badPaths) {
            testInvalidNodeFlowFilter(ffSrv, vpath, RpcErrorTag.DATA_MISSING,
                                      VtnErrorTag.NOTFOUND);
        }

        // Specifying vTerminal as the target virtual node.
        badPaths.clear();
        Collections.addAll(
            badPaths,
            newTerminalPath(tname1, unknown, null),
            newTerminalPath(tname1, bname1, null),
            newTerminalPath(tname2, bname1, null));

        for (VirtualNodePath vpath: badPaths) {
            testInvalidNodeFlowFilter(ffSrv, vpath, RpcErrorTag.BAD_ELEMENT,
                                      VtnErrorTag.BADREQUEST);
        }

        // Errors should never affect flow filters.
        vnet.verify();

        // Remove all flow filters.
        Map<VNodeIdentifier<?>, FlowFilterList> savedInputFilters =
            new HashMap<>();
        Map<VNodeIdentifier<?>, FlowFilterList> savedOutputFilters =
            new HashMap<>();
        for (VNodeIdentifier<?> ident: identifiers) {
            for (boolean output: bools) {
                if (output && ident.getType().equals(VNodeType.VTN)) {
                    continue;
                }

                FlowFilterNode fnode = vnet.getFlowFilterNode(ident);
                FlowFilterList flist = (output)
                    ? fnode.getOutputFilter()
                    : fnode.getInputFilter();
                FlowFilterList saved = flist.clone();
                List<Integer> indices1 = new ArrayList<>();
                List<Integer> indices2 = new ArrayList<>();
                List<Integer> indicesAll = new ArrayList<>();
                Map<Integer, VtnUpdateType> resMap1 = new HashMap<>();
                Map<Integer, VtnUpdateType> resMap2 = new HashMap<>();
                Map<Integer, VtnUpdateType> resMapAll = new HashMap<>();
                int count = 0;
                for (FlowFilter<?> ff: saved.getFlowFilters()) {
                    count++;
                    List<Integer> indices;
                    Map<Integer, VtnUpdateType> resMap;
                    if (count <= 2) {
                        indices = indices1;
                        resMap = resMap1;
                    } else {
                        indices = indices2;
                        resMap = resMap2;
                    }

                    Integer idx = ff.getIndex();
                    indices.add(idx);
                    indicesAll.add(idx);
                    assertNull(resMap.put(idx, VtnUpdateType.REMOVED));
                    assertNull(resMapAll.put(idx, null));
                }

                assertEquals(resMap1,
                             removeFlowFilter(ident, output, indices1));
                flist.remove(indices1);
                vnet.verify();

                assertEquals(resMap2,
                             removeFlowFilter(ident, output, indices2));
                flist.clear();
                vnet.verify();

                assertEquals(resMapAll,
                             removeFlowFilter(ident, output, indicesAll));
                vnet.verify();

                Map<VNodeIdentifier<?>, FlowFilterList> savedMap = (output)
                    ? savedOutputFilters : savedInputFilters;
                assertNull(savedMap.put(ident, saved));
            }
        }

        for (boolean empty: bools) {
            // Restore all flow filters.
            restoreFlowFilters(ffSrv, vnet, savedInputFilters, false);
            restoreFlowFilters(ffSrv, vnet, savedOutputFilters, true);

            // Clear the flow filter list.
            for (VNodeIdentifier<?> ident: savedInputFilters.keySet()) {
                clearFlowFilter(vnet, ident, false, empty);
            }
            for (VNodeIdentifier<?> ident: savedOutputFilters.keySet()) {
                clearFlowFilter(vnet, ident, true, empty);
            }
        }

        // Remove VTNs.
        removeVtn(tname1);
        removeVtn(tname2);
        vnet.removeTenant(tname1).
            removeTenant(tname2).
            verify();
    }
}
