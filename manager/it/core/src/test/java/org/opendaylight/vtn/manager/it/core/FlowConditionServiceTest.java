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
import java.util.Collection;
import java.util.Collections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Random;
import java.util.Set;

import com.google.common.collect.ImmutableList;

import org.opendaylight.vtn.manager.util.EtherTypes;
import org.opendaylight.vtn.manager.util.InetProtocols;
import org.opendaylight.vtn.manager.util.Ip4Network;

import org.opendaylight.vtn.manager.it.util.RpcErrorTag;
import org.opendaylight.vtn.manager.it.util.VirtualNetwork;
import org.opendaylight.vtn.manager.it.util.flow.cond.FlowCondSet;
import org.opendaylight.vtn.manager.it.util.flow.cond.FlowCondition;
import org.opendaylight.vtn.manager.it.util.flow.cond.FlowMatch;
import org.opendaylight.vtn.manager.it.util.flow.match.PortRange;
import org.opendaylight.vtn.manager.it.util.flow.match.VTNEtherMatch;
import org.opendaylight.vtn.manager.it.util.flow.match.VTNIcmpMatch;
import org.opendaylight.vtn.manager.it.util.flow.match.VTNInet4Match;
import org.opendaylight.vtn.manager.it.util.flow.match.VTNTcpMatch;
import org.opendaylight.vtn.manager.it.util.flow.match.VTNUdpMatch;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.RemoveFlowConditionInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.RemoveFlowConditionInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.RemoveFlowConditionMatchInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.RemoveFlowConditionMatchInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.SetFlowConditionInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.SetFlowConditionInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.SetFlowConditionMatchInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.SetFlowConditionMatchInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.VtnFlowConditionService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.set.flow.condition.match.input.FlowMatchList;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.vtn.flow.cond.config.VtnFlowMatch;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeName;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnErrorTag;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateOperationType;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;

/**
 * Test case for {@link VtnFlowConditionService}.
 */
public final class FlowConditionServiceTest extends TestMethodBase {
    /**
     * A list of invalid flow matches.
     */
    private static final List<FlowMatch>  INVALID_MATCHES;

    /**
     * Initialize static fields.
     */
    static {
        // Construct a list of invalid flow matches.
        ImmutableList.Builder<FlowMatch> builder =
            ImmutableList.<FlowMatch>builder();
        Integer index = 1;

        // Specifying VLAN priority without specifying VLAN ID.
        VTNEtherMatch ether = new VTNEtherMatch(
            null, null, null, null, (short)1);
        builder.add(new FlowMatch(index).setEtherMatch(ether));

        // Specifying VLAN priority for untagged frame.
        ether = new VTNEtherMatch(
            null, null, null, 0, (short)1);
        builder.add(new FlowMatch(index).setEtherMatch(ether));

        // Inconsistent Ethernet type.
        VTNTcpMatch tcp = new VTNTcpMatch();
        VTNUdpMatch udp = new VTNUdpMatch();
        VTNIcmpMatch icmp = new VTNIcmpMatch();
        VTNInet4Match inet4 = new VTNInet4Match();
        for (EtherTypes etype: EtherTypes.values()) {
            if (etype != EtherTypes.IPV4) {
                Integer type = etype.intValue();
                ether = new VTNEtherMatch(type);
                builder.add(new FlowMatch(index, ether, inet4, null));

                ether = new VTNEtherMatch(type);
                builder.add(new FlowMatch(index, ether, null, tcp));

                ether = new VTNEtherMatch(type);
                builder.add(new FlowMatch(index, ether, null, udp));

                ether = new VTNEtherMatch(type);
                builder.add(new FlowMatch(index, ether, null, icmp));
            }
        }

        // Inconsistent IP protocol.
        for (InetProtocols proto: InetProtocols.values()) {
            inet4 = new VTNInet4Match(proto.shortValue());
            if (proto != InetProtocols.TCP) {
                builder.add(new FlowMatch(index, null, inet4, tcp));
            }
            if (proto != InetProtocols.UDP) {
                builder.add(new FlowMatch(index, null, inet4, udp));
            }
            if (proto != InetProtocols.ICMP) {
                builder.add(new FlowMatch(index, null, inet4, icmp));
            }
        }

        INVALID_MATCHES = builder.build();
    }

    /**
     * Construct a new instance.
     *
     * @param vit  A {@link VTNManagerIT} instance.
     */
    public FlowConditionServiceTest(VTNManagerIT vit) {
        super(vit);
    }

    /**
     * Test case for {@link VtnFlowConditionService}.
     *
     * @param fcSrv  vtn-flow-condition service.
     * @param rand   A pseudo random generator.
     * @param name   The name of the target flow condition.
     * @throws Exception  An error occurred.
     */
    private void testFlowConditionSevice(VtnFlowConditionService fcSrv,
                                         Random rand, String name)
        throws Exception {
        LOG.debug("testFlowConditionSevice: name={}", name);

        // Ensure that flow condition RPCs return NOTFOUND error if the
        // specified flow condition is not present.
        notFoundTest(fcSrv, name, true);

        // Create an empty flow condition.
        VirtualNetwork vnet = getVirtualNetwork();
        FlowCondSet fcSet = vnet.getFlowConditions();
        FlowCondition fc = new FlowCondition(name);
        VtnUpdateOperationType opType = VtnUpdateOperationType.ADD;
        assertEquals(VtnUpdateType.CREATED, fc.update(fcSrv, opType, null));
        fcSet.add(fc);
        vnet.verify();

        // Configure Ethernet match.
        Set<Integer> idxSet = new HashSet<>();
        VTNEtherMatch ether = new VTNEtherMatch(
            createEtherAddress(rand), null, null, null, null);
        FlowMatch fm1 = new FlowMatch(VTN_INDEX_MIN).
            setEtherMatch(ether);
        assertEquals(true, idxSet.add(fm1.getIndex()));
        assertEquals(VtnUpdateType.CREATED, fc.addMatch(fcSrv, fm1));
        vnet.verify();
        assertEquals(null, fc.addMatch(fcSrv, fm1));

        ether = new VTNEtherMatch(
            createEtherAddress(rand), createEtherAddress(rand),
            0x86dd, 4095, (short)7);
        FlowMatch fm2 = new FlowMatch(createVtnIndex(rand, idxSet)).
            setEtherMatch(ether);
        Map<Integer, VtnUpdateType> fmResult = new HashMap<>();
        assertEquals(null, fmResult.put(fm1.getIndex(), null));
        assertEquals(null,
                     fmResult.put(fm2.getIndex(), VtnUpdateType.CREATED));
        assertEquals(fmResult, fc.addMatches(fcSrv, fm1, fm2));
        vnet.verify();
        assertEquals(VtnUpdateType.CREATED,
                     fmResult.put(fm2.getIndex(), null));
        assertEquals(fmResult, fc.addMatches(fcSrv, fm1, fm2));

        // Configure IPv4 match.
        // This will also configure Ethernet match for Ethernet type.
        VTNInet4Match inet4 = new VTNInet4Match(
            null, createIp4Network(rand), null, (short)63);
        FlowMatch fm3 = new FlowMatch(VTN_INDEX_MAX).
            setInetMatch(inet4);
        assertEquals(true, idxSet.add(fm3.getIndex()));
        assertEquals(VtnUpdateType.CREATED, fc.addMatch(fcSrv, fm3));
        vnet.verify();
        assertEquals(null, fc.addMatch(fcSrv, fm3));

        Ip4Network ip4Src =
            new Ip4Network(createIp4Network(rand).getAddress(), 24);
        Ip4Network ip4Dst =
            new Ip4Network(createIp4Network(rand).getAddress(), 29);
        inet4 = new VTNInet4Match(ip4Src, ip4Dst, (short)255, (short)0);
        FlowMatch fm4 = new FlowMatch(createVtnIndex(rand, idxSet)).
            setInetMatch(inet4);
        fmResult.clear();
        assertEquals(null, fmResult.put(fm1.getIndex(), null));
        assertEquals(null, fmResult.put(fm2.getIndex(), null));
        assertEquals(null, fmResult.put(fm3.getIndex(), null));
        assertEquals(null,
                     fmResult.put(fm4.getIndex(), VtnUpdateType.CREATED));
        assertEquals(fmResult, fc.addMatches(fcSrv, fm1, fm2, fm3, fm4));
        vnet.verify();
        assertEquals(VtnUpdateType.CREATED,
                     fmResult.put(fm4.getIndex(), null));
        assertEquals(fmResult, fc.addMatches(fcSrv, fm1, fm2, fm3, fm4));

        // Configure TCP and UDP matches.
        VTNTcpMatch tcp = new VTNTcpMatch((Integer)null, 65535);
        FlowMatch fm5 = new FlowMatch(createVtnIndex(rand, idxSet)).
            setLayer4Match(tcp);
        tcp = new VTNTcpMatch(
            new PortRange(32768, 65535), new PortRange(0, 1000));
        FlowMatch fm6 = new FlowMatch(createVtnIndex(rand, idxSet)).
            setLayer4Match(tcp);

        VTNUdpMatch udp = new VTNUdpMatch(0, createUnsignedShort(rand));
        FlowMatch fm7 = new FlowMatch(createVtnIndex(rand, idxSet)).
            setLayer4Match(udp);
        int portFrom = createUnsignedShort(rand);
        int portTo = Math.max(portFrom, createUnsignedShort(rand));
        udp = new VTNUdpMatch(
            new PortRange(12345), new PortRange(portFrom, portTo));
        FlowMatch fm8 = new FlowMatch(createVtnIndex(rand, idxSet)).
            setLayer4Match(udp);

        List<FlowMatch> matches = Arrays.asList(fm5, fm6, fm7, fm8);
        fmResult.clear();
        for (FlowMatch fm: matches) {
            assertEquals(null,
                         fmResult.put(fm.getIndex(), VtnUpdateType.CREATED));
        }
        assertEquals(fmResult, fc.addMatches(fcSrv, matches));
        vnet.verify();

        fmResult.clear();
        for (FlowMatch fm: matches) {
            assertEquals(null, fmResult.put(fm.getIndex(), null));
        }
        assertEquals(fmResult, fc.addMatches(fcSrv, matches));

        // Configure ICMP match.
        VTNIcmpMatch icmp = new VTNIcmpMatch((short)0, (Short)null);
        FlowMatch fm9 = new FlowMatch(createVtnIndex(rand, idxSet)).
            setLayer4Match(icmp);
        icmp = new VTNIcmpMatch((Short)null, (short)255);
        FlowMatch fm10 = new FlowMatch(createVtnIndex(rand, idxSet)).
            setLayer4Match(icmp);
        icmp = new VTNIcmpMatch(
            createUnsignedByte(rand), createUnsignedByte(rand));
        FlowMatch fm11 = new FlowMatch(createVtnIndex(rand, idxSet)).
            setLayer4Match(icmp);

        // Change condition in fm2.
        ether = new VTNEtherMatch(
            null, createEtherAddress(rand), 0xffff, 0, null);
        fm2.setEtherMatch(ether);

        // Make fm3 empty.
        fm3.setInetMatch(null);

        // Create an empty match.
        FlowMatch fm12 = new FlowMatch(createVtnIndex(rand, idxSet));

        fmResult.clear();
        matches = new ArrayList<>();
        Collections.addAll(matches, fm2, fm3, fm9, fm10, fm11, fm12);
        for (FlowMatch fm: matches) {
            Integer idx = fm.getIndex();
            VtnUpdateType utype = (fc.get(idx) == null)
                ? VtnUpdateType.CREATED : VtnUpdateType.CHANGED;
            assertEquals(null, fmResult.put(idx, utype));
        }

        for (FlowMatch fm: Arrays.asList(fm1, fm4, fm5)) {
            assertEquals(null, fmResult.put(fm.getIndex(), null));
            matches.add(fm);
        }

        assertEquals(fmResult, fc.addMatches(fcSrv, matches));
        vnet.verify();

        fmResult.clear();
        for (FlowMatch fm: matches) {
            assertEquals(null, fmResult.put(fm.getIndex(), null));
        }
        assertEquals(fmResult, fc.addMatches(fcSrv, matches));

        // Remove fm3.
        assertEquals(VtnUpdateType.REMOVED, fc.removeMatch(fcSrv, fm3));
        vnet.verify();
        assertEquals(null, fc.removeMatch(fcSrv, fm3));

        // Remove fm1, fm5, fm7, and fm11.
        fmResult.clear();
        fc.remove(fm1, fm5, fm7, fm11);
        List<Integer> indices = new ArrayList<>();
        Collections.addAll(indices, fm1.getIndex(), fm5.getIndex(),
                           fm7.getIndex(), fm11.getIndex());
        for (Integer idx: indices) {
            assertEquals(null, fmResult.put(idx, VtnUpdateType.REMOVED));
        }

        // remove-flow-condition-match should ignore invalid indices.
        Integer[] invalidIndices = {
            Integer.MIN_VALUE, -1, 0, 65536, 65537, 100000, Integer.MAX_VALUE,
        };
        for (Integer idx: invalidIndices) {
            indices.add(idx);
            assertEquals(null, fmResult.put(idx, null));
        }

        // Duplicate indices should be ignored.
        Collections.addAll(indices, fm1.getIndex(), fm11.getIndex());
        assertEquals(fmResult, fc.removeMatches(fcSrv, indices));
        vnet.verify();

        fmResult.clear();
        for (Integer idx: indices) {
            fmResult.put(idx, null);
        }
        assertEquals(fmResult, fc.removeMatches(fcSrv, indices));

        // Error tests.

        // Invalid operation in set-flow-condition input.
        FlowCondition tmpfc = new FlowCondition(name);
        SetFlowConditionInput input =
            tmpfc.newInput(VtnUpdateOperationType.REMOVE, null);
        checkRpcError(fcSrv.setFlowCondition(input),
                      RpcErrorTag.BAD_ELEMENT, VtnErrorTag.BADREQUEST);

        // Null vtn-flow-condition.
        FlowMatch tmpfm = new FlowMatch(1);
        VnodeName vname = new VnodeName(name);
        List<VtnFlowMatch> vfmList = new ArrayList<>();
        Collections.addAll(vfmList, tmpfm.toVtnFlowMatch(), null);
        input = new SetFlowConditionInputBuilder().
            setName(vname).
            setVtnFlowMatch(vfmList).
            build();
        checkRpcError(fcSrv.setFlowCondition(input),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);

        List<FlowMatchList> fml = new ArrayList<>();
        fml.add(null);
        SetFlowConditionMatchInput minput =
            new SetFlowConditionMatchInputBuilder().
            setName(name).
            setFlowMatchList(fml).
            build();
        checkRpcError(fcSrv.setFlowConditionMatch(minput),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);

        // No flow match index in vtn-flow-condition.
        tmpfm = new FlowMatch(null);
        vfmList.clear();
        vfmList.add(tmpfm.toVtnFlowMatch());
        input = new SetFlowConditionInputBuilder().
            setName(vname).
            setVtnFlowMatch(vfmList).
            build();
        checkRpcError(fcSrv.setFlowCondition(input),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);

        minput = tmpfc.newSetMatchInput(tmpfm);
        checkRpcError(fcSrv.setFlowConditionMatch(minput),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);

        // No flow match in set-flow-condition-match input.
        minput = tmpfc.newSetMatchInput((Collection<FlowMatch>)null);
        checkRpcError(fcSrv.setFlowConditionMatch(minput),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);
        minput = tmpfc.newSetMatchInput(Collections.<FlowMatch>emptyList());
        checkRpcError(fcSrv.setFlowConditionMatch(minput),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);

        // No flow match index in remove-flow-condition-match input.
        RemoveFlowConditionMatchInput rminput =
            new RemoveFlowConditionMatchInputBuilder().
            setName(name).
            build();
        checkRpcError(fcSrv.removeFlowConditionMatch(rminput),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);
        rminput = new RemoveFlowConditionMatchInputBuilder().
            setName(name).
            setMatchIndex(Collections.<Integer>emptyList()).
            build();
        checkRpcError(fcSrv.removeFlowConditionMatch(rminput),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);

        // Null match index in remove-flow-condition-match input.
        indices.clear();
        Collections.addAll(indices, 1, 2, null, 3);
        rminput = new RemoveFlowConditionMatchInputBuilder().
            setName(name).
            setMatchIndex(Collections.<Integer>emptyList()).
            build();
        checkRpcError(fcSrv.removeFlowConditionMatch(rminput),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);

        // Duplicate match index.
        FlowMatch dup = null;
        Set<Integer> tmpSet = new HashSet<>();
        List<FlowMatch> dupList = new ArrayList<>();
        for (int i = 0; i < 20; i++) {
            dup = new FlowMatch(createVtnIndex(rand, tmpSet));
            dupList.add(dup);
        }

        vfmList.clear();
        for (FlowMatch fm: dupList) {
            vfmList.add(fm.toVtnFlowMatch());
        }
        vfmList.add(dup.toVtnFlowMatch());
        input = new SetFlowConditionInputBuilder().
            setName(vname).
            setVtnFlowMatch(vfmList).
            build();
        checkRpcError(fcSrv.setFlowCondition(input),
                      RpcErrorTag.BAD_ELEMENT, VtnErrorTag.BADREQUEST);

        fml.clear();
        for (FlowMatch fm: dupList) {
            fml.add(fm.toFlowMatchList());
        }
        fml.add(dup.toFlowMatchList());
        minput = new SetFlowConditionMatchInputBuilder().
            setName(name).
            setFlowMatchList(fml).
            build();
        checkRpcError(fcSrv.setFlowConditionMatch(minput),
                      RpcErrorTag.BAD_ELEMENT, VtnErrorTag.BADREQUEST);

        // Invalid flow match.
        for (FlowMatch fm: INVALID_MATCHES) {
            vfmList = Collections.singletonList(fm.toVtnFlowMatch());
            input = new SetFlowConditionInputBuilder().
                setName(vname).
                setVtnFlowMatch(vfmList).
                build();
            checkRpcError(fcSrv.setFlowCondition(input),
                          RpcErrorTag.BAD_ELEMENT, VtnErrorTag.BADREQUEST);

            fml = Collections.singletonList(fm.toFlowMatchList());
            minput = new SetFlowConditionMatchInputBuilder().
                setName(name).
                setFlowMatchList(fml).
                build();
            checkRpcError(fcSrv.setFlowConditionMatch(minput),
                          RpcErrorTag.BAD_ELEMENT, VtnErrorTag.BADREQUEST);
        }

        // Errors should never affect path policies.
        vnet.verify();

        // Redefine flow matches using SET operation.
        opType = VtnUpdateOperationType.SET;
        fc.clear();
        idxSet.clear();
        fc.add(new FlowMatch(createVtnIndex(rand, idxSet)));

        ether = new VTNEtherMatch();
        fc.add(new FlowMatch(createVtnIndex(rand, idxSet)).
               setEtherMatch(ether));

        ether = new VTNEtherMatch(
            null, createEtherAddress(rand), null, null, null);
        fc.add(new FlowMatch(createVtnIndex(rand, idxSet)).
               setEtherMatch(ether));

        ether = new VTNEtherMatch(EtherTypes.LLDP.intValue());
        fc.add(new FlowMatch(createVtnIndex(rand, idxSet)).
               setEtherMatch(ether));

        ether = new VTNEtherMatch(
            createEtherAddress(rand), createEtherAddress(rand),
            createUnsignedShort(rand), 0, null);
        fc.add(new FlowMatch(createVtnIndex(rand, idxSet)).
               setEtherMatch(ether));

        ether = new VTNEtherMatch(
            createEtherAddress(rand), createEtherAddress(rand),
            createUnsignedShort(rand), 100, (short)5);
        fc.add(new FlowMatch(createVtnIndex(rand, idxSet)).
               setEtherMatch(ether));

        inet4 = new VTNInet4Match();
        fc.add(new FlowMatch(createVtnIndex(rand, idxSet)).
               setInetMatch(inet4));

        inet4 = new VTNInet4Match(InetProtocols.ICMP.shortValue());
        ether = new VTNEtherMatch(EtherTypes.IPV4.intValue());
        fc.add(new FlowMatch(createVtnIndex(rand, idxSet)).
               setEtherMatch(ether).setInetMatch(inet4));

        ip4Dst = new Ip4Network(createIp4Network(rand).getAddress(), 31);
        inet4 = new VTNInet4Match(
            createIp4Network(rand), ip4Dst, createUnsignedByte(rand), null);
        fc.add(new FlowMatch(createVtnIndex(rand, idxSet)).
               setInetMatch(inet4));

        ip4Src = new Ip4Network(createIp4Network(rand).getAddress(), 16);
        inet4 = new VTNInet4Match(
            ip4Src, createIp4Network(rand), createUnsignedByte(rand),
            (short)35);
        fc.add(new FlowMatch(createVtnIndex(rand, idxSet)).
               setInetMatch(inet4));

        tcp = new VTNTcpMatch();
        fc.add(new FlowMatch(createVtnIndex(rand, idxSet)).
               setLayer4Match(tcp));

        tcp = new VTNTcpMatch(65535, (Integer)null);
        inet4 = new VTNInet4Match(InetProtocols.TCP.shortValue());
        fc.add(new FlowMatch(createVtnIndex(rand, idxSet)).
               setInetMatch(inet4).setLayer4Match(tcp));

        tcp = new VTNTcpMatch(new PortRange(9999, 19999), null);
        fc.add(new FlowMatch(createVtnIndex(rand, idxSet)).
               setLayer4Match(tcp));

        udp = new VTNUdpMatch();
        inet4 = new VTNInet4Match(InetProtocols.UDP.shortValue());
        fc.add(new FlowMatch(createVtnIndex(rand, idxSet)).
               setInetMatch(inet4).setLayer4Match(udp));

        udp = new VTNUdpMatch(12345, 54321);
        fc.add(new FlowMatch(createVtnIndex(rand, idxSet)).
               setLayer4Match(udp));

        udp = new VTNUdpMatch(null, new PortRange(4096, 60000));
        fc.add(new FlowMatch(createVtnIndex(rand, idxSet)).
               setLayer4Match(udp));

        icmp = new VTNIcmpMatch();
        fc.add(new FlowMatch(createVtnIndex(rand, idxSet)).
               setLayer4Match(icmp));

        icmp = new VTNIcmpMatch((short)255, (short)0);
        inet4 = new VTNInet4Match(InetProtocols.ICMP.shortValue());
        fc.add(new FlowMatch(createVtnIndex(rand, idxSet)).
               setInetMatch(inet4).setLayer4Match(icmp));

        assertEquals(VtnUpdateType.CHANGED, fc.update(fcSrv, opType, true));
        vnet.verify();
        Boolean[] bools = {null, Boolean.TRUE, Boolean.FALSE};
        for (VtnUpdateOperationType op: MODIFY_OPERATIONS) {
            for (Boolean present: bools) {
                assertEquals(null, fc.update(fcSrv, op, present));
            }
        }

        // Make the flow condition empty using SET operation.
        tmpfc = new FlowCondition(name);
        fcSet.add(tmpfc);
        assertEquals(VtnUpdateType.CHANGED, tmpfc.update(fcSrv, opType, false));
        vnet.verify();
        for (VtnUpdateOperationType op: MODIFY_OPERATIONS) {
            for (Boolean present: bools) {
                assertEquals(null, tmpfc.update(fcSrv, op, present));
            }
        }

        // Restore flow matches using set-flow-condition-match RPC.
        fcSet.add(fc);
        fc.restore(fcSrv);
        vnet.verify();
        for (VtnUpdateOperationType op: MODIFY_OPERATIONS) {
            for (Boolean present: bools) {
                assertEquals(null, fc.update(fcSrv, op, present));
            }
        }
    }

    /**
     * Ensure that flow condition RPCs return NOTFOUND error if the specified
     * flow condition is not present.
     *
     * @param fcSrv  vtn-flow-condition service.
     * @param name   The name of the target flow condition.
     * @param valid  {@code true} indicates that the given name is valid.
     * @throws Exception  An error occurred.
     */
    private void notFoundTest(VtnFlowConditionService fcSrv, String name,
                              boolean valid)
        throws Exception {
        List<Integer> idxList = Collections.singletonList(Integer.valueOf(1));

        FlowCondition fc = new FlowCondition(name);
        FlowMatch fm = new FlowMatch(1);
        for (VtnUpdateOperationType op: MODIFY_OPERATIONS) {
            if (valid) {
                SetFlowConditionInput input = fc.newInput(op, true);
                checkRpcError(fcSrv.setFlowCondition(input),
                              RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);
            }

            RemoveFlowConditionInput rinput =
                new RemoveFlowConditionInputBuilder().
                setName(name).
                build();
            checkRpcError(fcSrv.removeFlowCondition(rinput),
                          RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);

            SetFlowConditionMatchInput minput = fc.newSetMatchInput(fm);
            checkRpcError(fcSrv.setFlowConditionMatch(minput),
                          RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);

            RemoveFlowConditionMatchInput rminput =
                new RemoveFlowConditionMatchInputBuilder().
                setName(name).
                setMatchIndex(idxList).
                build();
            checkRpcError(fcSrv.removeFlowConditionMatch(rminput),
                          RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);
        }
    }

    // TestMethodBase

    /**
     * Run the test.
     *
     * @throws Exception  An error occurred.
     */
    @Override
    protected void runTest() throws Exception {
        Random rand = new Random(0xabcdef1234567L);
        VTNManagerIT vit = getTest();
        VtnFlowConditionService fcSrv = vit.getFlowConditionService();

        String[] names = {
            "flow_cond_1",
            "a",
            "1234567890123456789012345678901",
            "flow_cond_2",
            "fc3",
        };
        for (String name: names) {
            testFlowConditionSevice(fcSrv, rand, name);
        }

        // Error tests.

        // Null input.
        checkRpcError(fcSrv.setFlowCondition(null),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);
        checkRpcError(fcSrv.removeFlowCondition(null),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);
        checkRpcError(fcSrv.setFlowConditionMatch(null),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);
        checkRpcError(fcSrv.removeFlowConditionMatch(null),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);

        // No flow condition name.
        FlowCondition tmpfc = new FlowCondition(null);
        Boolean[] bools = {null, Boolean.TRUE, Boolean.FALSE};
        for (VtnUpdateOperationType op: MODIFY_OPERATIONS) {
            for (Boolean present: bools) {
                SetFlowConditionInput input = tmpfc.newInput(op, present);
                checkRpcError(fcSrv.setFlowCondition(input),
                              RpcErrorTag.MISSING_ELEMENT,
                              VtnErrorTag.BADREQUEST);
            }
        }

        RemoveFlowConditionInput rinput =
            new RemoveFlowConditionInputBuilder().
            build();
        checkRpcError(fcSrv.removeFlowCondition(rinput),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);

        SetFlowConditionMatchInput minput =
            FlowCondition.newSetMatchInput((String)null, new FlowMatch(1));
        checkRpcError(fcSrv.setFlowConditionMatch(minput),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);

        List<Integer> idxList = Collections.singletonList(1);
        RemoveFlowConditionMatchInput rminput =
            new RemoveFlowConditionMatchInputBuilder().
            setMatchIndex(idxList).
            build();
        checkRpcError(fcSrv.removeFlowConditionMatch(rminput),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);

        // Invalid flow condition name.
        for (String name: INVALID_VNODE_NAMES) {
            notFoundTest(fcSrv, name, false);
        }

        // Errors should never affect path policies.
        VirtualNetwork vnet = getVirtualNetwork();
        vnet.verify();

        // Remove all the flow conditions using remove-flow-condition.
        FlowCondSet fcSet = vnet.getFlowConditions();
        FlowCondSet savedSet = fcSet.clone();
        for (FlowCondition fc: savedSet.getFlowConditions()) {
            String name = fc.getName();
            removeFlowCondition(name);
            fcSet.remove(name);
            vnet.verify();
        }

        // Restore all the flow conditions using set-flow-condition RPC.
        for (FlowCondition fc: savedSet.getFlowConditions()) {
            assertEquals(VtnUpdateType.CREATED, fc.update(fcSrv, null, false));
            fcSet.add(fc);
            vnet.verify();
            assertEquals(null, fc.update(fcSrv, null, false));
        }

        // Remove all the flow conditions using clear-flow-condition.
        assertEquals(VtnUpdateType.REMOVED,
                     getRpcResult(fcSrv.clearFlowCondition()));
        fcSet.clear();
        vnet.verify();

        assertEquals(null, getRpcResult(fcSrv.clearFlowCondition()));
        vnet.verify();
    }
}
