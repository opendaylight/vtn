/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.flow.action;

import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import org.junit.Test;

import org.opendaylight.vtn.manager.util.EtherAddress;
import org.opendaylight.vtn.manager.util.InetProtocols;
import org.opendaylight.vtn.manager.util.Ip4Network;

import org.opendaylight.vtn.manager.internal.util.OrderedComparator;
import org.opendaylight.vtn.manager.internal.util.inventory.L2Host;
import org.opendaylight.vtn.manager.internal.util.inventory.SalPort;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcErrorTag;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;

import org.opendaylight.vtn.manager.internal.TestBase;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.VtnOrderedFlowAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.VtnAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.flow.action.list.VtnFlowAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.flow.action.list.VtnFlowActionBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VlanType;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnErrorTag;

import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.Ordered;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.DropActionCaseBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.OutputActionCaseBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.PopPbbActionCaseBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.PopVlanActionCaseBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.PushVlanActionCaseBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.SetDlDstActionCaseBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.SetDlSrcActionCaseBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.SetNwDstActionCaseBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.SetNwSrcActionCaseBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.SetNwTosActionCaseBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.SetTpDstActionCaseBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.SetTpSrcActionCaseBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.SetVlanIdActionCaseBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.SetVlanPcpActionCaseBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.drop.action._case.DropActionBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.output.action._case.OutputActionBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.pop.pbb.action._case.PopPbbActionBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.pop.vlan.action._case.PopVlanActionBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.push.vlan.action._case.PushVlanActionBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.set.dl.dst.action._case.SetDlDstActionBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.set.dl.src.action._case.SetDlSrcActionBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.set.nw.dst.action._case.SetNwDstActionBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.set.nw.src.action._case.SetNwSrcActionBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.set.nw.tos.action._case.SetNwTosActionBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.set.tp.dst.action._case.SetTpDstActionBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.set.tp.src.action._case.SetTpSrcActionBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.set.vlan.id.action._case.SetVlanIdActionBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.set.vlan.pcp.action._case.SetVlanPcpActionBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.list.Action;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.list.ActionBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.address.Address;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.rev131026.InstructionList;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.rev131026.flow.InstructionsBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.rev131026.instruction.instruction.ApplyActionsCaseBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.rev131026.instruction.instruction.WriteActionsCaseBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.rev131026.instruction.instruction.apply.actions._case.ApplyActionsBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.rev131026.instruction.list.Instruction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.rev131026.instruction.list.InstructionBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.NodeConnectorId;

import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.inet.types.rev100924.PortNumber;
import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.yang.types.rev100924.MacAddress;
import org.opendaylight.yang.gen.v1.urn.opendaylight.l2.types.rev130827.VlanId;
import org.opendaylight.yang.gen.v1.urn.opendaylight.l2.types.rev130827.VlanPcp;

/**
 * JUnit test for {@link FlowActionUtils} and {@link FlowActionConverter}.
 */
public class FlowActionUtilsTest extends TestBase {
    /**
     * Create a MD-SAL DROP action.
     *
     * @param order  Action order in the action list.
     * @return  A MD-SAL DROP action.
     */
    public static Action createDropAction(Integer order) {
        return new ActionBuilder().
            setOrder(order).
            setAction(new DropActionCaseBuilder().
                      setDropAction(new DropActionBuilder().build()).
                      build()).
            build();
    }

    /**
     * Create a MD-SAL POP_VLAN action.
     *
     * @param order  Action order in the action list.
     * @return  A MD-SAL POP_VLAN action.
     */
    public static Action createPopVlanAction(Integer order) {
        return new ActionBuilder().
            setOrder(order).
            setAction(new PopVlanActionCaseBuilder().
                      setPopVlanAction(new PopVlanActionBuilder().build()).
                      build()).
            build();
    }

    /**
     * Create a MD-SAL PUSH_VLAN action.
     *
     * @param order  Action order in the action list.
     * @return  A MD-SAL PUSH_VLAN action.
     */
    public static Action createPushVlanAction(Integer order) {
        PushVlanActionBuilder ab = new PushVlanActionBuilder().
            setEthernetType(VlanType.VLAN.getIntValue());
        return new ActionBuilder().
            setOrder(order).
            setAction(new PushVlanActionCaseBuilder().
                      setPushVlanAction(ab.build()).build()).
            build();
    }

    /**
     * Create a MD-SAL OUTPUT action.
     *
     * @param order  Action order in the action list.
     * @param port   A MD-SAL port identifier.
     * @return  A MD-SAL OUTPUT action.
     */
    public static Action createOutputAction(Integer order, String port) {
        OutputActionBuilder ab = new OutputActionBuilder().
            setMaxLength(0xffff);
        if (port != null) {
            ab.setOutputNodeConnector(new NodeConnectorId(port));
        }

        return new ActionBuilder().
            setOrder(order).
            setAction(new OutputActionCaseBuilder().
                      setOutputAction(ab.build()).build()).
            build();
    }

    /**
     * Create a MD-SAL SET_DL_DST action.
     *
     * @param order  Action order in the action list.
     * @param mac    A MAC address.
     * @return  A MD-SAL SET_DL_DST action.
     */
    public static Action createSetDlDstAction(Integer order, MacAddress mac) {
        return new ActionBuilder().
            setOrder(order).
            setAction(new SetDlDstActionCaseBuilder().
                      setSetDlDstAction(
                          new SetDlDstActionBuilder().setAddress(mac).build()).
                      build()).
            build();
    }

    /**
     * Create a MD-SAL SET_DL_SRC action.
     *
     * @param order  Action order in the action list.
     * @param mac    A MAC address.
     * @return  A MD-SAL SET_DL_SRC action.
     */
    public static Action createSetDlSrcAction(Integer order, MacAddress mac) {
        return new ActionBuilder().
            setOrder(order).
            setAction(new SetDlSrcActionCaseBuilder().
                      setSetDlSrcAction(
                          new SetDlSrcActionBuilder().setAddress(mac).build()).
                      build()).
            build();
    }

    /**
     * Create a MD-SAL SET_TP_DST action.
     *
     * @param order  Action order in the action list.
     * @param port   A port number.
     * @return  A MD-SAL SET_TP_DST action.
     */
    public static Action createSetTpDstAction(Integer order, int port) {
        return new ActionBuilder().
            setOrder(order).
            setAction(new SetTpDstActionCaseBuilder().
                      setSetTpDstAction(
                          new SetTpDstActionBuilder().
                          setPort(new PortNumber(port)).build()).
                      build()).
            build();
    }

    /**
     * Create a MD-SAL SET_TP_SRC action.
     *
     * @param order  Action order in the action list.
     * @param port   A port number.
     * @return  A MD-SAL SET_TP_SRC action.
     */
    public static Action createSetTpSrcAction(Integer order, int port) {
        return new ActionBuilder().
            setOrder(order).
            setAction(new SetTpSrcActionCaseBuilder().
                      setSetTpSrcAction(
                          new SetTpSrcActionBuilder().
                          setPort(new PortNumber(port)).build()).
                      build()).
            build();
    }

    /**
     * Create a MD-SAL SET_NW_TOS action.
     *
     * @param order  Action order in the action list.
     * @param dscp   A DSCP value.
     * @return  A MD-SAL SET_NW_TOS action.
     */
    public static Action createSetNwTosAction(Integer order, int dscp) {
        Integer tos = Integer.valueOf(dscp << 2);
        return new ActionBuilder().
            setOrder(order).
            setAction(new SetNwTosActionCaseBuilder().
                      setSetNwTosAction(
                          new SetNwTosActionBuilder().setTos(tos).build()).
                      build()).
            build();
    }

    /**
     * Create a MD-SAL SET_NW_DST action.
     *
     * @param order  Action order in the action list.
     * @param addr   A MD-SAL IP address.
     * @return  A MD-SAL SET_NW_DST action.
     */
    public static Action createSetNwDstAction(Integer order, Address addr) {
        return new ActionBuilder().
            setOrder(order).
            setAction(new SetNwDstActionCaseBuilder().
                      setSetNwDstAction(
                          new SetNwDstActionBuilder().
                          setAddress(addr).build()).
                      build()).
            build();
    }

    /**
     * Create a MD-SAL SET_NW_SRC action.
     *
     * @param order  Action order in the action list.
     * @param addr   A MD-SAL IP address.
     * @return  A MD-SAL SET_NW_SRC action.
     */
    public static Action createSetNwSrcAction(Integer order, Address addr) {
        return new ActionBuilder().
            setOrder(order).
            setAction(new SetNwSrcActionCaseBuilder().
                      setSetNwSrcAction(
                          new SetNwSrcActionBuilder().
                          setAddress(addr).build()).
                      build()).
            build();
    }

    /**
     * Create a MD-SAL SET_VLAN_ID action.
     *
     * @param order  Action order in the action list.
     * @param vid    A VLAN ID.
     * @return  A MD-SAL SET_VLAN_ID action.
     */
    public static Action createSetVlanIdAction(Integer order, int vid) {
        return new ActionBuilder().
            setOrder(order).
            setAction(new SetVlanIdActionCaseBuilder().
                      setSetVlanIdAction(
                          new SetVlanIdActionBuilder().
                          setVlanId(new VlanId(vid)).build()).
                      build()).
            build();
    }

    /**
     * Create a MD-SAL SET_VLAN_PCP action.
     *
     * @param order  Action order in the action list.
     * @param pri    A VLAN priority.
     * @return  A MD-SAL SET_VLAN_ID action.
     */
    public static Action createSetVlanPcpAction(Integer order, int pri) {
        return new ActionBuilder().
            setOrder(order).
            setAction(new SetVlanPcpActionCaseBuilder().
                      setSetVlanPcpAction(
                          new SetVlanPcpActionBuilder().
                          setVlanPcp(new VlanPcp((short)pri)).build()).
                      build()).
            build();
    }

    /**
     * Create a VTN DROP action builder.
     *
     * @param order  Action order in the action list.
     * @return  A VTN DROP action builder.
     */
    public static VtnFlowActionBuilder createVtnDropActionBuilder(
        Integer order) {
        return new VtnFlowActionBuilder().
            setOrder(order).
            setVtnAction(VTNDropAction.newVtnAction());
    }

    /**
     * Create a VTN DROP action.
     *
     * @param order  Action order in the action list.
     * @return  A VTN DROP action.
     */
    public static VtnFlowAction createVtnDropAction(Integer order) {
        return createVtnDropActionBuilder(order).build();
    }

    /**
     * Create a VTN POP_VLAN action builder.
     *
     * @param order  Action order in the action list.
     * @return  A VTN POP_VLAN action builder.
     */
    public static VtnFlowActionBuilder createVtnPopVlanActionBuilder(
        Integer order) {
        return new VtnFlowActionBuilder().
            setOrder(order).
            setVtnAction(VTNPopVlanAction.newVtnAction());
    }

    /**
     * Create a VTN POP_VLAN action.
     *
     * @param order  Action order in the action list.
     * @return  A VTN POP_VLAN action.
     */
    public static VtnFlowAction createVtnPopVlanAction(Integer order) {
        return createVtnPopVlanActionBuilder(order).build();
    }

    /**
     * Create a VTN PUSH_VLAN action.
     *
     * @param order  Action order in the action list.
     * @return  A VTN PUSH_VLAN action.
     */
    public static VtnFlowAction createVtnPushVlanAction(Integer order) {
        return new VtnFlowActionBuilder().
            setOrder(order).
            setVtnAction(VTNPushVlanAction.newVtnAction(VlanType.VLAN)).
            build();
    }

    /**
     * Create a VTN SET_DL_DST action.
     *
     * @param order  Action order in the action list.
     * @param mac    A MAC address.
     * @return  A VTN SET_DL_DST action.
     */
    public static VtnFlowAction createVtnSetDlDstAction(Integer order,
                                                        MacAddress mac) {
        return new VtnFlowActionBuilder().
            setOrder(order).
            setVtnAction(VTNSetDlDstAction.newVtnAction(mac)).
            build();
    }

    /**
     * Create a VTN SET_DL_SRC action.
     *
     * @param order  Action order in the action list.
     * @param mac    A MAC address.
     * @return  A VTN SET_DL_SRC action.
     */
    public static VtnFlowAction createVtnSetDlSrcAction(Integer order,
                                                        MacAddress mac) {
        return new VtnFlowActionBuilder().
            setOrder(order).
            setVtnAction(VTNSetDlSrcAction.newVtnAction(mac)).
            build();
    }

    /**
     * Create a VTN SET_ICMP_CODE action.
     *
     * @param order  Action order in the action list.
     * @param code   An ICMP code.
     * @return  A VTN SET_ICMP_CODE action.
     */
    public static VtnFlowAction createVtnSetIcmpCodeAction(Integer order,
                                                           int code) {
        return new VtnFlowActionBuilder().
            setOrder(order).
            setVtnAction(VTNSetIcmpCodeAction.newVtnAction((short)code)).
            build();
    }


    /**
     * Create a VTN SET_ICMP_TYPE action.
     *
     * @param order  Action order in the action list.
     * @param type   An ICMP type.
     * @return  A VTN SET_ICMP_TYPE action.
     */
    public static VtnFlowAction createVtnSetIcmpTypeAction(Integer order,
                                                           int type) {
        return new VtnFlowActionBuilder().
            setOrder(order).
            setVtnAction(VTNSetIcmpTypeAction.newVtnAction((short)type)).
            build();
    }

    /**
     * Create a VTN SET_INET_DSCP action.
     *
     * @param order  Action order in the action list.
     * @param dscp   A DSCP value.
     * @return  A VTN SET_INET_DSCP action.
     */
    public static VtnFlowAction createVtnSetInetDscpAction(Integer order,
                                                           int dscp) {
        return new VtnFlowActionBuilder().
            setOrder(order).
            setVtnAction(VTNSetInetDscpAction.newVtnAction((short)dscp)).
            build();
    }

    /**
     * Create a VTN SET_INET_DST action.
     *
     * @param order  Action order in the action list.
     * @param addr   A MD-SAL IP address.
     * @return  A VTN SET_INET_DST action.
     */
    public static VtnFlowAction createVtnSetInetDstAction(Integer order,
                                                          Address addr) {
        return new VtnFlowActionBuilder().
            setOrder(order).
            setVtnAction(VTNSetInetDstAction.newVtnAction(addr)).
            build();
    }

    /**
     * Create a VTN SET_INET_SRC action.
     *
     * @param order  Action order in the action list.
     * @param addr   A MD-SAL IP address.
     * @return  A VTN SET_INET_SRC action.
     */
    public static VtnFlowAction createVtnSetInetSrcAction(Integer order,
                                                          Address addr) {
        return new VtnFlowActionBuilder().
            setOrder(order).
            setVtnAction(VTNSetInetSrcAction.newVtnAction(addr)).
            build();
    }

    /**
     * Create a VTN SET_TP_DST action builder.
     *
     * @param order  Action order in the action list.
     * @param port   A port number.
     * @return  A VTN SET_TP_DST action builder.
     */
    public static VtnFlowActionBuilder createVtnSetPortDstActionBuilder(
        Integer order, int port) {
        PortNumber pnum = new PortNumber(port);
        return new VtnFlowActionBuilder().
            setOrder(order).
            setVtnAction(VTNSetPortDstAction.newVtnAction(pnum));
    }

    /**
     * Create a VTN SET_TP_DST action.
     *
     * @param order  Action order in the action list.
     * @param port   A port number.
     * @return  A VTN SET_TP_DST action.
     */
    public static VtnFlowAction createVtnSetPortDstAction(Integer order,
                                                          int port) {
        return createVtnSetPortDstActionBuilder(order, port).build();
    }

    /**
     * Create a VTN SET_TP_SRC action builder.
     *
     * @param order  Action order in the action list.
     * @param port   A port number.
     * @return  A VTN SET_TP_SRC action builder.
     */
    public static VtnFlowActionBuilder createVtnSetPortSrcActionBuilder(
        Integer order, int port) {
        PortNumber pnum = new PortNumber(port);
        return new VtnFlowActionBuilder().
            setOrder(order).
            setVtnAction(VTNSetPortSrcAction.newVtnAction(pnum));
    }

    /**
     * Create a VTN SET_TP_SRC action.
     *
     * @param order  Action order in the action list.
     * @param port   A port number.
     * @return  A VTN SET_TP_SRC action.
     */
    public static VtnFlowAction createVtnSetPortSrcAction(Integer order,
                                                          int port) {
        return createVtnSetPortSrcActionBuilder(order, port).build();
    }

    /**
     * Create a VTN SET_VLAN_ID action.
     *
     * @param order  Action order in the action list.
     * @param vid    A VLAN ID.
     * @return  A VTN SET_VLAN_ID action.
     */
    public static VtnFlowAction createVtnSetVlanIdAction(Integer order,
                                                         int vid) {
        return new VtnFlowActionBuilder().
            setOrder(order).
            setVtnAction(VTNSetVlanIdAction.newVtnAction(vid)).
            build();
    }

    /**
     * Create a VTN SET_VLAN_PCP action.
     *
     * @param order  Action order in the action list.
     * @param pri    A VLAN priority.
     * @return  A VTN SET_VLAN_PCP action.
     */
    public static VtnFlowAction createVtnSetVlanPcpAction(Integer order,
                                                          int pri) {
        VlanPcp pcp = new VlanPcp((short)pri);
        return new VtnFlowActionBuilder().
            setOrder(order).
            setVtnAction(VTNSetVlanPcpAction.newVtnAction(pcp)).
            build();
    }

    /**
     * Test case for {@link FlowActionUtils#getOutputPort(List)}.
     */
    @Test
    public void testGetOutputPort() {
        List<Action> actions = new ArrayList<>();
        assertEquals(null, FlowActionUtils.getOutputPort(actions));

        EtherAddress dlSrc = new EtherAddress(0x123456789L);
        EtherAddress dlDst = new EtherAddress(0xaabbccddee11L);
        Ip4Network nwSrc = new Ip4Network("192.168.100.200");
        Ip4Network nwDst = new Ip4Network("127.0.0.1");

        // No OUTPUT action.
        int order = 0;
        Collections.addAll(
            actions,
            createDropAction(order++),
            createPopVlanAction(order++),
            createPushVlanAction(order++),
            createSetDlDstAction(order++, dlDst.getMacAddress()),
            createSetDlSrcAction(order++, dlSrc.getMacAddress()),
            createSetTpDstAction(order++, 100),
            createSetTpSrcAction(order++, 33333),
            createSetNwTosAction(order++, 45),
            createSetNwDstAction(order++, nwDst.getMdAddress()),
            createSetNwSrcAction(order++, nwSrc.getMdAddress()),
            createSetVlanIdAction(order++, 1000),
            createSetVlanPcpAction(order++, 5));
        assertEquals(null, FlowActionUtils.getOutputPort(actions));

        // Put OUTPUT action at the head of the list.
        List<Action> outActions = new ArrayList<>();
        String port = "openflow:1:2";
        outActions.add(createOutputAction(0, port));
        outActions.addAll(actions);
        assertEquals(port, FlowActionUtils.getOutputPort(outActions));

        // Put OUTPUT action at the tail of the list.
        port = "openflow:333:444";
        outActions = new ArrayList<Action>(actions);
        outActions.add(createOutputAction(0, port));
        assertEquals(port, FlowActionUtils.getOutputPort(outActions));

        // Put OUTPUT action at the middle of the list.
        port = "openflow:12345:678";
        outActions = new ArrayList<Action>(actions);
        for (int i = 0; i < actions.size(); i++) {
            outActions.add(actions.get(i));
            if (i == 4) {
                outActions.add(createOutputAction(0, port));
            }
        }
        assertEquals(port, FlowActionUtils.getOutputPort(outActions));

        // Append a broken OUTPUT action.
        List<Action> brokenList = new ArrayList<>(outActions);
        Action broken = new ActionBuilder().
            setOrder(0).
            setAction(new OutputActionCaseBuilder().build()).
            build();
        brokenList.add(broken);
        assertEquals(null, FlowActionUtils.getOutputPort(brokenList));

        brokenList = new ArrayList<Action>(outActions);
        broken = new ActionBuilder().
            setOrder(0).
            setAction(new OutputActionCaseBuilder().
                      setOutputAction(new OutputActionBuilder().build()).
                      build()).
            build();
        brokenList.add(broken);
        assertEquals(null, FlowActionUtils.getOutputPort(brokenList));
    }

    /**
     * Test case for
     * {@link FlowActionUtils#getDestinationHost(List,MacAddress,int)}.
     */
    @Test
    public void testGetDestinationHost() {
        List<Action> actions = new ArrayList<>();
        EtherAddress dlDst = new EtherAddress(0xaabbccddee11L);
        MacAddress dstMac = dlDst.getMacAddress();
        int vlan = 1;

        // Empty action.
        L2Host expected = null;
        L2Host lh = FlowActionUtils.getDestinationHost(actions, dstMac, vlan);
        assertEquals(expected, lh);

        // No OUTPUT action.
        EtherAddress newDst = new EtherAddress(0xdeadbeef123L);
        MacAddress newMac = newDst.getMacAddress();
        int newVlan = 10;
        int order = 0;
        Collections.addAll(
            actions,
            createSetVlanIdAction(order++, newVlan),
            createSetDlDstAction(order++, newMac),
            createSetVlanPcpAction(order++, 3));
        lh = FlowActionUtils.getDestinationHost(actions, dstMac, vlan);
        assertEquals(expected, lh);

        order = 0;
        actions.clear();
        Collections.addAll(
            actions,
            createSetVlanIdAction(order++, newVlan),
            createSetDlDstAction(order++, newMac),
            createDropAction(order++));
        lh = FlowActionUtils.getDestinationHost(actions, dstMac, vlan);
        assertEquals(expected, lh);

        // Null output port.
        String outPort = null;
        order = 0;
        actions.clear();
        Collections.addAll(
            actions,
            createSetVlanIdAction(order++, newVlan),
            createSetDlDstAction(order++, newMac),
            createSetVlanPcpAction(order++, 3),
            createOutputAction(order++, outPort));
        lh = FlowActionUtils.getDestinationHost(actions, dstMac, vlan);
        assertEquals(expected, lh);

        // Unsupported output port.
        outPort = "unknown:10:1";
        order = 0;
        actions.clear();
        Collections.addAll(
            actions,
            createSetVlanIdAction(order++, newVlan),
            createSetDlDstAction(order++, newMac),
            createSetVlanPcpAction(order++, 3),
            createOutputAction(order++, outPort));
        lh = FlowActionUtils.getDestinationHost(actions, dstMac, vlan);
        assertEquals(expected, lh);

        // SetDlDstAction is null.
        Action act = new ActionBuilder().
            setOrder(1).
            setAction(new SetDlDstActionCaseBuilder().build()).
            build();
        outPort = "openflow:3:5";
        SalPort sport = SalPort.create(outPort);
        assertNotNull(sport);
        actions.clear();
        Collections.addAll(
            actions,
            createSetVlanIdAction(0, newVlan),
            act,
            createSetVlanPcpAction(2, 3),
            createOutputAction(3, outPort));
        lh = FlowActionUtils.getDestinationHost(actions, dstMac, vlan);
        expected = new L2Host(null, newVlan, sport);
        assertEquals(expected, lh);

        // Null in SetDlDstAction.
        order = 0;
        actions.clear();
        Collections.addAll(
            actions,
            createSetVlanIdAction(order++, newVlan),
            createSetDlDstAction(order++, null),
            createSetVlanPcpAction(order++, 3),
            createOutputAction(order++, outPort));
        lh = FlowActionUtils.getDestinationHost(actions, dstMac, vlan);
        assertEquals(expected, lh);

        // Both MAC address and VLAN ID are changed.
        order = 0;
        actions.clear();
        Collections.addAll(
            actions,
            createSetVlanIdAction(order++, newVlan),
            createSetDlDstAction(order++, newMac),
            createSetVlanPcpAction(order++, 3),
            createOutputAction(order++, outPort));
        lh = FlowActionUtils.getDestinationHost(actions, dstMac, vlan);
        expected = new L2Host(newMac, newVlan, sport);
        assertEquals(expected, lh);

        // SetVlanIdAction is null.
        act = new ActionBuilder().
            setOrder(0).
            setAction(new SetVlanIdActionCaseBuilder().build()).
            build();
        actions.clear();
        Collections.addAll(
            actions,
            act,
            createSetDlDstAction(1, newMac),
            createSetVlanPcpAction(2, 3),
            createOutputAction(3, outPort));
        lh = FlowActionUtils.getDestinationHost(actions, dstMac, vlan);
        expected = new L2Host(newMac, vlan, sport);
        assertEquals(expected, lh);

        // Null in SetVlanIdAction.
        act = new ActionBuilder().
            setOrder(0).
            setAction(new SetVlanIdActionCaseBuilder().
                      setSetVlanIdAction(new SetVlanIdActionBuilder().build()).
                      build()).
            build();
        actions.clear();
        Collections.addAll(
            actions,
            act,
            createSetDlDstAction(1, newMac),
            createSetVlanPcpAction(2, 3),
            createOutputAction(3, outPort));
        lh = FlowActionUtils.getDestinationHost(actions, dstMac, vlan);
        assertEquals(expected, lh);

        // Destination MAC address is not changed.
        order = 0;
        actions.clear();
        Collections.addAll(
            actions,
            createSetVlanIdAction(order++, newVlan),
            createSetVlanPcpAction(order++, 3),
            createOutputAction(order++, outPort));
        lh = FlowActionUtils.getDestinationHost(actions, dstMac, vlan);
        expected = new L2Host(dstMac, newVlan, sport);
        assertEquals(expected, lh);

        // VLAN ID is not changed.
        order = 0;
        actions.clear();
        Collections.addAll(
            actions,
            createSetVlanPcpAction(order++, 3),
            createSetDlDstAction(order++, newMac),
            createOutputAction(order++, outPort));
        lh = FlowActionUtils.getDestinationHost(actions, dstMac, vlan);
        expected = new L2Host(newMac, vlan, sport);
        assertEquals(expected, lh);

        // VLAN tag is stripped.
        order = 0;
        actions.clear();
        Collections.addAll(
            actions,
            createSetVlanPcpAction(order++, 3),
            createPopVlanAction(order++),
            createSetDlDstAction(order++, newMac),
            createOutputAction(order++, outPort));
        lh = FlowActionUtils.getDestinationHost(actions, dstMac, vlan);
        expected = new L2Host(newMac, 0, sport);
        assertEquals(expected, lh);

        // VLAN tag is pushed.
        order = 0;
        actions.clear();
        Collections.addAll(
            actions,
            createPushVlanAction(order++),
            createSetVlanPcpAction(order++, 7),
            createSetVlanIdAction(order++, newVlan),
            createSetDlDstAction(order++, newMac),
            createOutputAction(order++, outPort));
        lh = FlowActionUtils.getDestinationHost(actions, dstMac, 0);
        expected = new L2Host(newMac, newVlan, sport);
        assertEquals(expected, lh);

        // Neither destination MAC address nor VLAN ID are changed.
        Ip4Network nwSrc = new Ip4Network("192.168.100.200");
        Ip4Network nwDst = new Ip4Network("127.0.0.1");
        order = 0;
        actions.clear();
        Collections.addAll(
            actions,
            createSetVlanPcpAction(order++, 7),
            createSetDlSrcAction(order++, newMac),
            createSetNwTosAction(order++, 45),
            createSetNwDstAction(order++, nwDst.getMdAddress()),
            createSetNwSrcAction(order++, nwSrc.getMdAddress()),
            createOutputAction(order++, outPort));
        lh = FlowActionUtils.getDestinationHost(actions, dstMac, vlan);
        expected = new L2Host(dstMac, vlan, sport);
        assertEquals(expected, lh);
    }

    /**
     * Test case for
     * {@link FlowActionUtils#getActions(InstructionList,Comparator)}.
     */
    @Test
    public void testGetActions() {
        List<Action> empty = Collections.<Action>emptyList();
        Comparator<Ordered> comp = null;

        // Empty instructions.
        InstructionList insts = null;
        assertEquals(empty, FlowActionUtils.getActions(insts, comp));

        // Null instruction list.
        insts = new InstructionsBuilder().build();
        assertEquals(empty, FlowActionUtils.getActions(insts, comp));

        // Empty instruction list.
        List<Instruction> ilist = new ArrayList<>();
        insts = new InstructionsBuilder().setInstruction(ilist).build();
        assertEquals(empty, FlowActionUtils.getActions(insts, comp));

        // WRITE actions.
        Instruction write = new InstructionBuilder().
            setInstruction(new WriteActionsCaseBuilder().build()).
            build();
        ilist.add(write);
        insts = new InstructionsBuilder().setInstruction(ilist).build();
        assertEquals(empty, FlowActionUtils.getActions(insts, comp));

        // Empty APPLY action.
        Instruction apply = new InstructionBuilder().
            setInstruction(new ApplyActionsCaseBuilder().build()).
            build();
        ilist.add(apply);
        insts = new InstructionsBuilder().setInstruction(ilist).build();
        assertEquals(empty, FlowActionUtils.getActions(insts, comp));

        // APPLY action.
        EtherAddress dlSrc = new EtherAddress(0x123456789L);
        EtherAddress dlDst = new EtherAddress(0xaabbccddee11L);
        Ip4Network nwSrc = new Ip4Network("192.168.100.200");
        Ip4Network nwDst = new Ip4Network("127.0.0.1");
        List<Action> actions = new ArrayList<>();
        Collections.addAll(
            actions,
            createDropAction(6),
            createPopVlanAction(7),
            createPushVlanAction(2),
            createSetDlDstAction(5, dlDst.getMacAddress()),
            createOutputAction(12, "openflow:2:5"),
            createSetDlSrcAction(8, dlSrc.getMacAddress()),
            createSetTpDstAction(0, 100),
            createSetTpSrcAction(10, 33333),
            createSetNwTosAction(9, 45),
            createSetNwDstAction(1, nwDst.getMdAddress()),
            createSetNwSrcAction(11, nwSrc.getMdAddress()),
            createSetVlanIdAction(3, 1000),
            createSetVlanPcpAction(4, 5));
        List<Action> original = new ArrayList<>(actions);
        Map<Integer, Action> actionMap = new HashMap<>();
        for (Action act: actions) {
            assertEquals(null, actionMap.put(act.getOrder(), act));
        }
        assertEquals(actions.size(), actionMap.size());

        apply = new InstructionBuilder().
            setOrder(0).
            setInstruction(new ApplyActionsCaseBuilder().
                           setApplyActions(
                               new ApplyActionsBuilder().
                               setAction(actions).build()).
                           build()).
            build();
        ilist = new ArrayList<Instruction>();
        ilist.add(apply);
        insts = new InstructionsBuilder().setInstruction(ilist).build();

        // Action list should be returned unchanged if no comparator is
        // specified.
        assertSame(actions, FlowActionUtils.getActions(insts, comp));
        assertEquals(original, actions);

        // Action list should be sorted in ascending order or "order" field
        // if OrderedComparator is specified.
        comp = new OrderedComparator();
        List<Action> result = FlowActionUtils.getActions(insts, comp);
        assertNotSame(actions, result);
        assertEquals(original.size(), result.size());

        int order = 0;
        for (Action act: result) {
            Integer ord = act.getOrder();
            assertEquals(order, ord.intValue());
            assertEquals(actionMap.get(ord), act);
            order++;
        }

        // Original action list should be unchanged.
        assertEquals(original, actions);

        // Sort actions in descending order.
        comp = Collections.reverseOrder(comp);
        result = FlowActionUtils.getActions(insts, comp);
        assertNotSame(actions, result);
        assertEquals(original.size(), result.size());

        order = original.size() - 1;
        for (Action act: result) {
            Integer ord = act.getOrder();
            assertEquals(order, ord.intValue());
            assertEquals(actionMap.get(ord), act);
            order--;
        }

        assertEquals(-1, order);
        assertEquals(original, actions);
    }

    /**
     * Test case for
     * {@link FlowActionConverter#toFlowFilterAction(VtnOrderedFlowAction)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testVtnFlowActionToFlowFilterAction() throws Exception {
        FlowActionConverter converter = FlowActionConverter.getInstance();
        VtnOrderedFlowAction vaction = null;
        String emsg = "vtn-flow-action cannot be null";
        RpcErrorTag etag = RpcErrorTag.MISSING_ELEMENT;
        VtnErrorTag vtag = VtnErrorTag.BADREQUEST;
        try {
            converter.toFlowFilterAction(vaction);
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            assertEquals(vtag, e.getVtnErrorTag());
            assertEquals(emsg, e.getMessage());
        }

        vaction = new VtnFlowActionBuilder().build();
        emsg = "vtn-action cannot be null";
        try {
            converter.toFlowFilterAction(vaction);
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            assertEquals(vtag, e.getVtnErrorTag());
            assertEquals(emsg, e.getMessage());
        }

        // Unsupported actions.
        List<VtnFlowAction> actions = new ArrayList<>();
        Collections.addAll(
            actions,
            createVtnDropAction(1),
            createVtnPopVlanAction(1),
            createVtnPushVlanAction(1),
            createVtnSetVlanIdAction(1, 1));
        etag = RpcErrorTag.BAD_ELEMENT;
        for (VtnFlowAction vfa: actions) {
            VtnAction vact = vfa.getVtnAction();
            emsg = "Unsupported vtn-action: " +
                vact.getImplementedInterface().getName();
            try {
                converter.toFlowFilterAction(vfa);
                unexpected();
            } catch (RpcException e) {
                assertEquals(etag, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                assertEquals(emsg, e.getMessage());
            }
        }

        Integer[] orders = {
            null, Integer.MIN_VALUE, -1000, -1,
            0, 1, 2, 32000, Integer.MAX_VALUE,
        };

        EtherAddress dlSrc = new EtherAddress(0x123456789L);
        EtherAddress dlDst = new EtherAddress(0xaabbccddee11L);
        Ip4Network nwSrc = new Ip4Network("192.168.100.200");
        Ip4Network nwDst = new Ip4Network("127.0.0.1");
        short dscp = 45;
        int portSrc = 1;
        int portDst = 60000;
        short icmpType = 35;
        short icmpCode = 7;
        short vlanPcp = 4;
        etag = RpcErrorTag.MISSING_ELEMENT;
        for (Integer order: orders) {
            FlowFilterAction ffa;
            try {
                vaction = createVtnSetDlSrcAction(order, dlSrc.getMacAddress());
                ffa = converter.toFlowFilterAction(vaction);
                assertNotNull(order);
                assertEquals(order, ffa.getOrder());
                assertTrue(ffa instanceof VTNSetDlSrcAction);
                assertEquals(dlSrc, ((VTNSetDlSrcAction)ffa).getAddress());
            } catch (RpcException e) {
                assertEquals(null, order);
                assertEquals(etag, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                emsg = "VTNSetDlSrcAction: Action order cannot be null";
                assertEquals(emsg, e.getMessage());
            }

            try {
                vaction = createVtnSetDlDstAction(order, dlDst.getMacAddress());
                ffa = converter.toFlowFilterAction(vaction);
                assertNotNull(order);
                assertEquals(order, ffa.getOrder());
                assertTrue(ffa instanceof VTNSetDlDstAction);
                assertEquals(dlDst, ((VTNSetDlDstAction)ffa).getAddress());
            } catch (RpcException e) {
                assertEquals(null, order);
                assertEquals(etag, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                emsg = "VTNSetDlDstAction: Action order cannot be null";
                assertEquals(emsg, e.getMessage());
            }

            try {
                vaction = createVtnSetInetSrcAction(order,
                                                    nwSrc.getMdAddress());
                ffa = converter.toFlowFilterAction(vaction);
                assertNotNull(order);
                assertEquals(order, ffa.getOrder());
                assertTrue(ffa instanceof VTNSetInetSrcAction);
                assertEquals(nwSrc, ((VTNSetInetSrcAction)ffa).getAddress());
            } catch (RpcException e) {
                assertEquals(null, order);
                assertEquals(etag, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                emsg = "VTNSetInetSrcAction: Action order cannot be null";
                assertEquals(emsg, e.getMessage());
            }

            try {
                vaction = createVtnSetInetDstAction(order,
                                                    nwDst.getMdAddress());
                ffa = converter.toFlowFilterAction(vaction);
                assertNotNull(order);
                assertEquals(order, ffa.getOrder());
                assertTrue(ffa instanceof VTNSetInetDstAction);
                assertEquals(nwDst, ((VTNSetInetDstAction)ffa).getAddress());
            } catch (RpcException e) {
                assertEquals(null, order);
                assertEquals(etag, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                emsg = "VTNSetInetDstAction: Action order cannot be null";
                assertEquals(emsg, e.getMessage());
            }

            try {
                vaction = createVtnSetInetDscpAction(order, dscp);
                ffa = converter.toFlowFilterAction(vaction);
                assertNotNull(order);
                assertEquals(order, ffa.getOrder());
                assertTrue(ffa instanceof VTNSetInetDscpAction);
                assertEquals(dscp, ((VTNSetInetDscpAction)ffa).getDscp());
            } catch (RpcException e) {
                assertEquals(null, order);
                assertEquals(etag, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                emsg = "VTNSetInetDscpAction: Action order cannot be null";
                assertEquals(emsg, e.getMessage());
            }

            try {
                vaction = createVtnSetPortSrcAction(order, portSrc);
                ffa = converter.toFlowFilterAction(vaction);
                assertNotNull(order);
                assertEquals(order, ffa.getOrder());
                assertTrue(ffa instanceof VTNSetPortSrcAction);
                assertEquals(portSrc, ((VTNSetPortSrcAction)ffa).getPort());
            } catch (RpcException e) {
                assertEquals(null, order);
                assertEquals(etag, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                emsg = "VTNSetPortSrcAction: Action order cannot be null";
                assertEquals(emsg, e.getMessage());
            }

            try {
                vaction = createVtnSetPortDstAction(order, portDst);
                ffa = converter.toFlowFilterAction(vaction);
                assertNotNull(order);
                assertEquals(order, ffa.getOrder());
                assertTrue(ffa instanceof VTNSetPortDstAction);
                assertEquals(portDst, ((VTNSetPortDstAction)ffa).getPort());
            } catch (RpcException e) {
                assertEquals(null, order);
                assertEquals(etag, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                emsg = "VTNSetPortDstAction: Action order cannot be null";
                assertEquals(emsg, e.getMessage());
            }

            try {
                vaction = createVtnSetIcmpTypeAction(order, icmpType);
                ffa = converter.toFlowFilterAction(vaction);
                assertNotNull(order);
                assertEquals(order, ffa.getOrder());
                assertTrue(ffa instanceof VTNSetIcmpTypeAction);
                assertEquals(icmpType, ((VTNSetIcmpTypeAction)ffa).getType());
            } catch (RpcException e) {
                assertEquals(null, order);
                assertEquals(etag, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                emsg = "VTNSetIcmpTypeAction: Action order cannot be null";
                assertEquals(emsg, e.getMessage());
            }

            try {
                vaction = createVtnSetIcmpCodeAction(order, icmpCode);
                ffa = converter.toFlowFilterAction(vaction);
                assertNotNull(order);
                assertEquals(order, ffa.getOrder());
                assertTrue(ffa instanceof VTNSetIcmpCodeAction);
                assertEquals(icmpCode, ((VTNSetIcmpCodeAction)ffa).getCode());
            } catch (RpcException e) {
                assertEquals(null, order);
                assertEquals(etag, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                emsg = "VTNSetIcmpCodeAction: Action order cannot be null";
                assertEquals(emsg, e.getMessage());
            }

            try {
                vaction = createVtnSetVlanPcpAction(order, vlanPcp);
                ffa = converter.toFlowFilterAction(vaction);
                assertNotNull(order);
                assertEquals(order, ffa.getOrder());
                assertTrue(ffa instanceof VTNSetVlanPcpAction);
                assertEquals(vlanPcp, ((VTNSetVlanPcpAction)ffa).getPriority());
            } catch (RpcException e) {
                assertEquals(null, order);
                assertEquals(etag, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                emsg = "VTNSetVlanPcpAction: Action order cannot be null";
                assertEquals(emsg, e.getMessage());
            }
        }
    }

    /**
     * Test case for the following methods.
     *
     * <ul>
     *   <li>{@link FlowActionUtils#toVtnFlowActions(List,Comparator,Short)}</li>
     *   <li>{@link FlowActionConverter#toVtnAction(org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.Action,Short)}</li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testToDataFlowAcitons() throws Exception {
        List<Action> actions = null;
        Comparator<Ordered> comp = new OrderedComparator();
        Comparator<Ordered> rcomp = Collections.reverseOrder(comp);
        List<VtnFlowAction> drop = Collections.
            singletonList(createVtnDropAction(0));
        assertEquals(drop,
                     FlowActionUtils.toVtnFlowActions(actions, comp, null));
        actions = new ArrayList<>();
        assertEquals(drop,
                     FlowActionUtils.toVtnFlowActions(actions, comp, null));

        // Construct a list that contains all the supported MD-SAL actions.
        EtherAddress dlSrc = new EtherAddress(0x123456789L);
        EtherAddress dlDst = new EtherAddress(0xaabbccddee11L);
        Ip4Network nwSrc = new Ip4Network("192.168.100.200");
        Ip4Network nwDst = new Ip4Network("127.0.0.1");
        actions = new ArrayList<>();
        Collections.addAll(
            actions,
            createDropAction(7),
            createPopVlanAction(8),
            createPushVlanAction(2),
            createSetDlDstAction(5, dlDst.getMacAddress()),
            createOutputAction(14, "openflow:2:5"),
            createSetDlSrcAction(9, dlSrc.getMacAddress()),
            createSetTpDstAction(0, 203),
            createSetTpSrcAction(12, 9),
            createSetNwTosAction(11, 45),
            createSetNwDstAction(1, nwDst.getMdAddress()),
            createSetNwSrcAction(13, nwSrc.getMdAddress()),
            createSetVlanIdAction(3, 1000),
            createSetVlanPcpAction(4, 5),

            // Insert broken MD-SAL action.
            new ActionBuilder().setOrder(10).build(),

            // Insert unsupported MD-SAL action.
            new ActionBuilder().setOrder(6).setAction(
                new PopPbbActionCaseBuilder().setPopPbbAction(
                    new PopPbbActionBuilder().build()).build()).
            build());

        List<Action> original = new ArrayList<>(actions);

        // In case of unknown IP protocol.
        Short[] protocols = {null, 123};
        List<VtnFlowAction> expected = new ArrayList<>();
        Collections.addAll(
            expected,
            createVtnSetInetDstAction(0, nwDst.getMdAddress()),
            createVtnPushVlanAction(1),
            createVtnSetVlanIdAction(2, 1000),
            createVtnSetVlanPcpAction(3, 5),
            createVtnSetDlDstAction(4, dlDst.getMacAddress()),
            createVtnDropAction(5),
            createVtnPopVlanAction(6),
            createVtnSetDlSrcAction(7, dlSrc.getMacAddress()),
            createVtnSetInetDscpAction(8, 45),
            createVtnSetInetSrcAction(9, nwSrc.getMdAddress()));
        List<VtnFlowAction> reversed = new ArrayList<>(expected);
        Collections.reverse(reversed);

        for (Short ipproto: protocols) {
            List<VtnFlowAction> results = FlowActionUtils.
                toVtnFlowActions(actions, comp, ipproto);
            assertEquals(expected, results);

            // The specified list should not be changed.
            assertEquals(original, actions);

            // Sort actions in descending order.
            results = FlowActionUtils.
                toVtnFlowActions(actions, rcomp, ipproto);
            assertEquals(expected.size(), results.size());
            for (int order = 0; order < results.size(); order++) {
                VtnAction ex = reversed.get(order).getVtnAction();
                VtnFlowAction vfa = results.get(order);
                assertEquals(order, vfa.getOrder().intValue());
                assertEquals(ex, vfa.getVtnAction());
            }
            assertEquals(original, actions);
        }

        // In case of TCP or UDP.
        expected.clear();
        Collections.addAll(
            expected,
            createVtnSetPortDstAction(0, 203),
            createVtnSetInetDstAction(1, nwDst.getMdAddress()),
            createVtnPushVlanAction(2),
            createVtnSetVlanIdAction(3, 1000),
            createVtnSetVlanPcpAction(4, 5),
            createVtnSetDlDstAction(5, dlDst.getMacAddress()),
            createVtnDropAction(6),
            createVtnPopVlanAction(7),
            createVtnSetDlSrcAction(8, dlSrc.getMacAddress()),
            createVtnSetInetDscpAction(9, 45),
            createVtnSetPortSrcAction(10, 9),
            createVtnSetInetSrcAction(11, nwSrc.getMdAddress()));
        reversed = new ArrayList<>(expected);
        Collections.reverse(reversed);

        protocols = new Short[]{
            InetProtocols.TCP.shortValue(),
            InetProtocols.UDP.shortValue(),
        };

        for (Short ipproto: protocols) {
            List<VtnFlowAction> results = FlowActionUtils.
                toVtnFlowActions(actions, comp, ipproto);
            assertEquals(expected, results);
            assertEquals(original, actions);

            results = FlowActionUtils.
                toVtnFlowActions(actions, rcomp, ipproto);
            assertEquals(expected.size(), results.size());
            for (int order = 0; order < results.size(); order++) {
                VtnAction ex = reversed.get(order).getVtnAction();
                VtnFlowAction vfa = results.get(order);
                assertEquals(order, vfa.getOrder().intValue());
                assertEquals(ex, vfa.getVtnAction());
            }
            assertEquals(original, actions);
        }

        // In case of ICMP.
        expected.clear();
        Collections.addAll(
            expected,
            createVtnSetIcmpCodeAction(0, 203),
            createVtnSetInetDstAction(1, nwDst.getMdAddress()),
            createVtnPushVlanAction(2),
            createVtnSetVlanIdAction(3, 1000),
            createVtnSetVlanPcpAction(4, 5),
            createVtnSetDlDstAction(5, dlDst.getMacAddress()),
            createVtnDropAction(6),
            createVtnPopVlanAction(7),
            createVtnSetDlSrcAction(8, dlSrc.getMacAddress()),
            createVtnSetInetDscpAction(9, 45),
            createVtnSetIcmpTypeAction(10, 9),
            createVtnSetInetSrcAction(11, nwSrc.getMdAddress()));
        reversed = new ArrayList<>(expected);
        Collections.reverse(reversed);

        Short ipproto = InetProtocols.ICMP.shortValue();
        List<VtnFlowAction> results = FlowActionUtils.
            toVtnFlowActions(actions, comp, ipproto);
        assertEquals(expected, results);
        assertEquals(original, actions);

        results = FlowActionUtils.toVtnFlowActions(actions, rcomp, ipproto);
        assertEquals(expected.size(), results.size());
        for (int order = 0; order < results.size(); order++) {
            VtnAction ex = reversed.get(order).getVtnAction();
            VtnFlowAction vfa = results.get(order);
            assertEquals(order, vfa.getOrder().intValue());
            assertEquals(ex, vfa.getVtnAction());
        }
        assertEquals(original, actions);
    }

    /**
     * Test case for
     * {@link FlowActionConverter#getDescription(org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.Action)}.
     */
    @Test
    public void testGetDescription() {
        String output = "openflow:123:456";
        EtherAddress dlSrc = new EtherAddress(0x3141592L);
        EtherAddress dlDst = new EtherAddress(0xaabbccddee11L);
        Ip4Network nwSrc = new Ip4Network("192.168.111.222");
        Ip4Network nwDst = new Ip4Network("10.33.55.77");
        int tpSrc = 12345;
        int tpDst = 45678;
        int dscp = 35;
        int vid = 4095;
        int pcp = 6;

        int order = 0;
        List<Action> actions = new ArrayList<>();
        Collections.addAll(
            actions,
            createOutputAction(order++, output),
            createDropAction(order++),
            createPopVlanAction(order++),
            createPushVlanAction(order++),
            createSetDlDstAction(order++, dlDst.getMacAddress()),
            createSetDlSrcAction(order++, dlSrc.getMacAddress()),
            createSetTpDstAction(order++, tpDst),
            createSetTpSrcAction(order++, tpSrc),
            createSetNwTosAction(order++, dscp),
            createSetNwDstAction(order++, nwDst.getMdAddress()),
            createSetNwSrcAction(order++, nwSrc.getMdAddress()),
            createSetVlanIdAction(order++, vid),
            createSetVlanPcpAction(order++, pcp));

        // Create unsupported MD-SAL action.
        Action unsupported = new ActionBuilder().
            setOrder(order++).
            setAction(new PopPbbActionCaseBuilder().setPopPbbAction(
                          new PopPbbActionBuilder().build()).build()).
            build();
        actions.add(unsupported);

        // Add an empty action.
        Action empty = new ActionBuilder().setOrder(order++).build();
        actions.add(empty);

        String[] descriptions = {
            "OUTPUT(port=" + output + ", len=65535)",
            "DROP",
            "POP_VLAN",
            "PUSH_VLAN(type=0x8100)",
            "SET_DL_DST(address=" + dlDst.getText() + ")",
            "SET_DL_SRC(address=" + dlSrc.getText() + ")",
            "SET_TP_DST(port=" + tpDst + ")",
            "SET_TP_SRC(port=" + tpSrc + ")",
            "SET_NW_TOS(tos=0x" + Integer.toHexString(dscp << 2) + ")",
            "SET_NW_DST(ipv4=" + nwDst.getCidrText() + ")",
            "SET_NW_SRC(ipv4=" + nwSrc.getCidrText() + ")",
            "SET_VLAN_ID(vid=" + vid + ")",
            "SET_VLAN_PCP(pcp=" + pcp + ")",
            unsupported.getAction().toString(),
            "Action(null)",
        };
        assertEquals(descriptions.length, actions.size());

        FlowActionConverter converter = FlowActionConverter.getInstance();
        int index = 0;
        for (Action action: actions) {
            assertEquals(descriptions[index],
                         converter.getDescription(action.getAction()));
            index++;
        }
    }
}
