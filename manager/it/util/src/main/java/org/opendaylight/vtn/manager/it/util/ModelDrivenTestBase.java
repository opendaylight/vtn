/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.util;

import java.net.Inet4Address;
import java.net.InetAddress;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.ListIterator;

import org.opendaylight.vtn.manager.util.EtherAddress;
import org.opendaylight.vtn.manager.util.EtherTypes;
import org.opendaylight.vtn.manager.util.NumberUtils;

import org.opendaylight.vtn.manager.it.ofmock.OfMockUtils;

import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.DropActionCase;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.OutputActionCase;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.PopVlanActionCase;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.PushVlanActionCase;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.SetVlanIdActionCase;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.drop.action._case.DropAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.output.action._case.OutputAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.pop.vlan.action._case.PopVlanAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.push.vlan.action._case.PushVlanAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.set.vlan.id.action._case.SetVlanIdAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.list.Action;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.rev131026.flow.Instructions;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.rev131026.flow.Match;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.rev131026.flow.MatchBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.rev131026.instruction.instruction.ApplyActionsCase;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.rev131026.instruction.instruction.apply.actions._case.ApplyActions;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.rev131026.instruction.list.Instruction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.NodeConnectorId;
import org.opendaylight.yang.gen.v1.urn.opendaylight.l2.types.rev130827.VlanId;
import org.opendaylight.yang.gen.v1.urn.opendaylight.model.match.types.rev131026.ethernet.match.fields.EthernetDestination;
import org.opendaylight.yang.gen.v1.urn.opendaylight.model.match.types.rev131026.ethernet.match.fields.EthernetDestinationBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.model.match.types.rev131026.ethernet.match.fields.EthernetSource;
import org.opendaylight.yang.gen.v1.urn.opendaylight.model.match.types.rev131026.ethernet.match.fields.EthernetSourceBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.model.match.types.rev131026.ethernet.match.fields.EthernetTypeBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.model.match.types.rev131026.match.EthernetMatch;
import org.opendaylight.yang.gen.v1.urn.opendaylight.model.match.types.rev131026.match.EthernetMatchBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.model.match.types.rev131026.match.VlanMatch;
import org.opendaylight.yang.gen.v1.urn.opendaylight.model.match.types.rev131026.match.VlanMatchBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.model.match.types.rev131026.vlan.match.fields.VlanIdBuilder;

import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.inet.types.rev100924.Ipv4Prefix;
import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.inet.types.rev100924.PortNumber;
import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.yang.types.rev100924.MacAddress;
import org.opendaylight.yang.gen.v1.urn.opendaylight.l2.types.rev130827.EtherType;
import org.opendaylight.yang.gen.v1.urn.opendaylight.l2.types.rev130827.VlanPcp;

/**
 * Abstract base class for integration tests using JUnit.
 *
 * <p>
 *   This class provides utilities for MD-SAL.
 * </p>
 */
public abstract class ModelDrivenTestBase extends TestBase {
    /**
     * The expected number of flow instructions.
     */
    private static final int  FLOW_INST_SIZE = 1;

    /**
     * Convert the given byte array into MD-SAL MAC address.
     *
     * @param mac  A byte array which represents the MAC address.
     * @return  A {@link MacAddress} instance.
     */
    public static MacAddress toMacAddress(byte[] mac) {
        return new EtherAddress(mac).getMacAddress();
    }

    /**
     * Create ethernet match builder.
     *
     * @param src  The source MAC address.
     * @param dst  The destination MAC address.
     * @return  A {@link EthernetMatchBuilder} instance.
     */
    public static EthernetMatchBuilder createEthernetMatch(
        byte[] src, byte[] dst) {
        EthernetMatchBuilder builder = new EthernetMatchBuilder();
        if (src != null) {
            EthernetSourceBuilder sb = new EthernetSourceBuilder().
                setAddress(toMacAddress(src));
            builder.setEthernetSource(sb.build());
        }
        if (dst != null) {
            EthernetDestinationBuilder db = new EthernetDestinationBuilder().
                setAddress(toMacAddress(dst));
            builder.setEthernetDestination(db.build());
        }

        return builder;
    }

    /**
     * Create ethernet match builder.
     *
     * @param src   The source MAC address.
     * @param dst   The destination MAC address.
     * @param type  The ethernet type. Zero means an undefined type.
     * @return  A {@link EthernetMatchBuilder} instance.
     */
    public static EthernetMatchBuilder createEthernetMatch(
        byte[] src, byte[] dst, int type) {
        EthernetMatchBuilder builder = createEthernetMatch(src, dst);
        if (type != 0) {
            EthernetTypeBuilder etb = new EthernetTypeBuilder().
                setType(new EtherType(Long.valueOf((long)type)));
            builder.setEthernetType(etb.build());
        }

        return builder;
    }

    /**
     * Create VLAN match builder.
     *
     * @param vlan  VLAN ID. Zero means untagged ethernet frame.
     * @return  A {@link VlanMatchBuilder} instance.
     */
    public static VlanMatchBuilder createVlanMatch(short vlan) {
        return createVlanMatch(vlan, null);
    }

    /**
     * Create VLAN match builder.
     *
     * @param vlan  VLAN ID. Zero means untagged ethernet frame.
     * @param pcp   VLAN priority. {@code null} means undefined.
     * @return  A {@link VlanMatchBuilder} instance.
     */
    public static VlanMatchBuilder createVlanMatch(short vlan, Short pcp) {
        VlanIdBuilder ib = new VlanIdBuilder();
        boolean present = (vlan != 0);
        VlanId vid = new VlanId(NumberUtils.getUnsigned(vlan));
        ib.setVlanId(vid).setVlanIdPresent(present);
        VlanMatchBuilder builder = new VlanMatchBuilder().
            setVlanId(ib.build());
        if (pcp != null) {
            builder.setVlanPcp(new VlanPcp(pcp));
        }

        return builder;
    }

    /**
     * Create flow match builder to match packet from the given port.
     *
     * @param ingress  The ingress port identifier.
     * @param vlan     VLAN ID. Zero means untagged ethernet frame.
     * @return  A {@link MatchBuilder} instance.
     */
    public static MatchBuilder createMatch(String ingress, short vlan) {
        MatchBuilder builder = new MatchBuilder();
        if (vlan >= 0) {
            builder.setVlanMatch(createVlanMatch(vlan).build());
        }
        if (ingress != null) {
            builder.setInPort(new NodeConnectorId(ingress));
        }

        return builder;
    }

    /**
     * Create flow match builder to match the given packet.
     *
     * @param ingress  The ingress port identifier.
     * @param vlan     VLAN ID. Zero means untagged ethernet frame.
     * @param src      The source MAC address.
     * @param dst      The destination MAC address.
     * @return  A {@link MatchBuilder} instance.
     */
    public static MatchBuilder createMatch(String ingress, short vlan,
                                           byte[] src, byte[] dst) {
        MatchBuilder builder = createMatch(ingress, vlan);
        if (src != null || dst != null) {
            builder.setEthernetMatch(createEthernetMatch(src, dst).build());
        }

        return builder;
    }

    /**
     * Convert the given {@link InetAddress} instance into a {@link Ipv4Prefix}
     * instance.
     *
     * @param ip  An {@link InetAddress} instance.
     * @return  A {@link Ipv4Prefix} instance.
     */
    public static Ipv4Prefix toIpv4Prefix(InetAddress ip) {
        return toIpv4Prefix(ip, Integer.SIZE);
    }

    /**
     * Convert the given {@link InetAddress} instance into a {@link Ipv4Prefix}
     * instance.
     *
     * @param ip      An {@link InetAddress} instance.
     * @param prefix  CIDR prefix length.
     * @return  A {@link Ipv4Prefix} instance.
     */
    public static Ipv4Prefix toIpv4Prefix(InetAddress ip, int prefix) {
        assertTrue(ip instanceof Inet4Address);
        String value = String.format("%s/%d", ip.getHostAddress(), prefix);
        return new Ipv4Prefix(value);
    }

    /**
     * Convert the given short value into a {@link PortNumber} instance.
     *
     * @param port  A short value.
     * @return  A {@link PortNumber} instance.
     */
    public static PortNumber toPortNumber(short port) {
        Integer value = Integer.valueOf((int)(port & OfMockUtils.MASK_SHORT));
        return new PortNumber(value);
    }

    /**
     * Verify flow action.
     *
     * @param it   Action list iterator.
     * @param cls  Expected flow action class.
     * @param <T>  The type of flow action.
     * @return  Flow action instance.
     */
    public static <T> T verifyAction(ListIterator<Action> it, Class<T> cls) {
        if (!it.hasNext()) {
            String msg = String.format("action[%d]: Expected %s action.",
                                       it.nextIndex(), cls.getSimpleName());
            fail(msg);
        }

        Action act = it.next();
        Object a = act.getAction();
        if (!cls.isInstance(a)) {
            String msg = String.format("action[%d]: %s is expected, but %s.",
                                       it.previousIndex(), cls.getSimpleName(),
                                       a);
            fail(msg);
        }

        return cls.cast(a);
    }

    /**
     * Vefiry POP_VLAN action.
     *
     * @param it  Action list iterator.
     */
    public static void verifyPopVlanAction(ListIterator<Action> it) {
        PopVlanActionCase act = verifyAction(it, PopVlanActionCase.class);
        PopVlanAction pva = act.getPopVlanAction();
        assertNotNull(pva);
    }

    /**
     * Verify PUSH_VLAN action.
     *
     * @param it       Action list iterator.
     * @param ethType  Expected ethernet type.
     */
    public static void verifyPushVlanAction(ListIterator<Action> it,
                                            int ethType) {
        PushVlanActionCase act = verifyAction(it, PushVlanActionCase.class);
        PushVlanAction pva = act.getPushVlanAction();
        assertNotNull(pva);
        assertEquals(Integer.valueOf(ethType), pva.getEthernetType());
    }

    /**
     * Verify SET_VLAN_ID action.
     *
     * @param it    Action list iterator.
     * @param vlan  Expected VLAN ID.
     */
    public static void verifySetVlanIdAction(ListIterator<Action> it,
                                             short vlan) {
        SetVlanIdActionCase act = verifyAction(it, SetVlanIdActionCase.class);
        SetVlanIdAction sva = act.getSetVlanIdAction();
        VlanId vid = new VlanId(NumberUtils.getUnsigned(vlan));
        assertEquals(vid, sva.getVlanId());
    }

    /**
     * Verify the flow action that specifies the VLAN for the outgoing packet.
     *
     * @param it       A list iterator associated with a list of flow actions.
     * @param inVlan   The VLAN ID for incoming packet.
     * @param outVlan  The VLAN ID for outgoing packet.
     */
    public static void verifyVlanAction(ListIterator<Action> it, short inVlan,
                                        short outVlan) {
        if (inVlan != outVlan) {
            if (outVlan == 0) {
                verifyPopVlanAction(it);
            } else {
                if (inVlan == 0) {
                    int ethType = EtherTypes.VLAN.intValue();
                    verifyPushVlanAction(it, ethType);
                }
                verifySetVlanIdAction(it, outVlan);
            }
        }
    }

    /**
     * Verify DROP action.
     *
     * @param it  Action list iterator.
     */
    public static void verifyDropAction(ListIterator<Action> it) {
        DropActionCase act = verifyAction(it, DropActionCase.class);
        DropAction da = act.getDropAction();
        assertNotNull(da);
    }

    /**
     * Verify OUTPUT action.
     *
     * @param it      Action list iterator.
     * @param egress  Expected egress port identifier.
     */
    public static void verifyOutputAction(ListIterator<Action> it,
                                          String egress) {
        OutputActionCase act = verifyAction(it, OutputActionCase.class);
        OutputAction oa = act.getOutputAction();
        assertEquals(egress, oa.getOutputNodeConnector().getValue());
    }

    /**
     * Verify that the given flow instruction forwards the packet to the
     * specified network.
     *
     * @param it       A list iterator associated with a list of flow actions.
     * @param egress   The egress port identifier.
     * @param inVlan   The VLAN ID for incoming packet.
     * @param outVlan  The VLAN ID for outgoing packet.
     */
    public static void verifyOutputAction(ListIterator<Action> it,
                                          String egress, short inVlan,
                                          short outVlan) {
        verifyVlanAction(it, inVlan, outVlan);
        verifyOutputAction(it, egress);
    }

    /**
     * Return a list of actions configured in the given flow instruction.
     *
     * @param inst  A flow instructions.
     * @return  A list of flow actions.
     */
    public static List<Action> getActionList(Instructions inst) {
        List<Instruction> instList = inst.getInstruction();
        assertNotNull("Instruction is null.", instList);

        assertEquals(FLOW_INST_SIZE, instList.size());

        org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.rev131026.instruction.Instruction instCase =
            instList.get(0).getInstruction();
        if (!(instCase instanceof ApplyActionsCase)) {
            fail("Unexpected instruction: " + instCase);
        }

        ApplyActionsCase apCase = (ApplyActionsCase)instCase;
        ApplyActions apActions = apCase.getApplyActions();
        assertNotNull("ApplyActions is null.", apActions);

        List<Action> actList = apActions.getAction();
        assertNotNull("Action list is null.", actList);
        assertFalse("Action list is empty.", actList.isEmpty());

        List<Action> actions = new ArrayList<>();
        actions.addAll(actList);
        Collections.sort(actions, new OrderedComparator());

        return actions;
    }

    /**
     * Verify that the given flow instruction forwards the packet to the
     * specified network.
     *
     * @param inst     A flow instructions.
     * @param egress   The egress port identifier.
     * @param inVlan   The VLAN ID for incoming packet.
     * @param outVlan  The VLAN ID for outgoing packet.
     */
    public static void verifyOutputFlow(Instructions inst, String egress,
                                        short inVlan, short outVlan) {
        ListIterator<Action> it = getActionList(inst).listIterator();
        verifyOutputAction(it, egress, inVlan, outVlan);
        assertFalse(it.hasNext());
    }

    /**
     * Verify that the given flow instruction drops the packet.
     *
     * @param inst     A flow instructions.
     */
    public static void verifyDropFlow(Instructions inst) {
        ListIterator<Action> it = getActionList(inst).listIterator();
        verifyDropAction(it);
        assertFalse(it.hasNext());
    }

    /**
     * Return the VLAN ID configured in the given flow match.
     *
     * @param match  A {@link Match} instance.
     * @return  VLAN ID configured in the given match.
     *          Zero is returned if the given instance matches untagged
     *          network.
     */
    public static short getVlanMatch(Match match) {
        VlanMatch vmatch = match.getVlanMatch();
        org.opendaylight.yang.gen.v1.urn.opendaylight.model.match.types.rev131026.vlan.match.fields.VlanId vid =
            vmatch.getVlanId();

        Boolean present = vid.isVlanIdPresent();
        assertNotNull(present);
        if (Boolean.FALSE.equals(present)) {
            return 0;
        }

        return vid.getVlanId().getValue().shortValue();
    }

    /**
     * Return the source MAC address configured in the given flow match.
     *
     * @param match  A {@link Match} instance.
     * @return  A byte array which represents the source MAC address
     *          configured in the given match.
     *          {@code null} is returned if the source MAC address is not
     *          configured.
     */
    public static byte[] getEthernetSourceMatch(Match match) {
        EthernetMatch ematch = match.getEthernetMatch();
        if (ematch == null) {
            return null;
        }

        EthernetSource esrc = ematch.getEthernetSource();
        if (esrc == null) {
            return null;
        }

        MacAddress maddr = esrc.getAddress();
        return (maddr == null) ? null : OfMockUtils.getMacAddress(maddr);
    }

    /**
     * Return the destination MAC address configured in the given flow match.
     *
     * @param match  A {@link Match} instance.
     * @return  A byte array which represents the destination MAC address
     *          configured in the given match.
     *          {@code null} is returned if the destination MAC address is not
     *          configured.
     */
    public static byte[] getEthernetDestinationMatch(Match match) {
        EthernetMatch ematch = match.getEthernetMatch();
        if (ematch == null) {
            return null;
        }

        EthernetDestination edst = ematch.getEthernetDestination();
        if (edst == null) {
            return null;
        }

        MacAddress maddr = edst.getAddress();
        return (maddr == null) ? null : OfMockUtils.getMacAddress(maddr);
    }

    /**
     * Return the ingress port identifier configured in the given flow match.
     *
     * @param match  A {@link Match} instance.
     * @return  The ingress port identifier if found.
     *          {@code null} if not found.
     */
    public static String getInPortMatch(Match match) {
        NodeConnectorId ncId = match.getInPort();
        return (ncId == null) ? null : ncId.getValue();
    }

    /**
     * Determine whether the given flow instructions forwards packets to
     * the given network or not.
     *
     * @param inst    Flow instructions.
     * @param pid     The egress port identifier.
     * @param vlan    VLAN ID of the egress network.
     * @param inVlan  VLAN ID configured in the incoming packet.
     * @return  {@code true} only if the given flow instructions forwards
     *          packets to the given network.
     */
    public static boolean hasOutput(Instructions inst, String pid,
                                    short vlan, short inVlan) {
        short vid = inVlan;
        for (Action act: getActionList(inst)) {
            Object a = act.getAction();
            if (a instanceof PopVlanActionCase) {
                vid = 0;
            } else if (a instanceof SetVlanIdActionCase) {
                SetVlanIdActionCase s = (SetVlanIdActionCase)a;
                SetVlanIdAction sva = s.getSetVlanIdAction();
                VlanId vlanId = sva.getVlanId();
                if (vlanId != null) {
                    Integer value = vlanId.getValue();
                    if (value != null) {
                        vid = value.shortValue();
                    }
                }
            } else if (a instanceof OutputActionCase) {
                OutputActionCase o = (OutputActionCase)a;
                OutputAction oa = o.getOutputAction();
                if (oa != null) {
                    String out = oa.getOutputNodeConnector().getValue();
                    if (pid.equals(out) && vlan == vid) {
                        return true;
                    }
                }
            }
        }

        return false;
    }
}
