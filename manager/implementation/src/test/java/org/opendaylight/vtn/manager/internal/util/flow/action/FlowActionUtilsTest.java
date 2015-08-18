/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
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

import org.opendaylight.vtn.manager.flow.action.DropAction;
import org.opendaylight.vtn.manager.flow.action.FlowAction;
import org.opendaylight.vtn.manager.flow.action.PopVlanAction;
import org.opendaylight.vtn.manager.flow.action.PushVlanAction;
import org.opendaylight.vtn.manager.flow.action.SetDlDstAction;
import org.opendaylight.vtn.manager.flow.action.SetDlSrcAction;
import org.opendaylight.vtn.manager.flow.action.SetDscpAction;
import org.opendaylight.vtn.manager.flow.action.SetIcmpCodeAction;
import org.opendaylight.vtn.manager.flow.action.SetIcmpTypeAction;
import org.opendaylight.vtn.manager.flow.action.SetInet4DstAction;
import org.opendaylight.vtn.manager.flow.action.SetInet4SrcAction;
import org.opendaylight.vtn.manager.flow.action.SetTpDstAction;
import org.opendaylight.vtn.manager.flow.action.SetTpSrcAction;
import org.opendaylight.vtn.manager.flow.action.SetVlanIdAction;
import org.opendaylight.vtn.manager.flow.action.SetVlanPcpAction;
import org.opendaylight.vtn.manager.util.EtherAddress;
import org.opendaylight.vtn.manager.util.Ip4Network;

import org.opendaylight.vtn.manager.internal.L2Host;
import org.opendaylight.vtn.manager.internal.util.OrderedComparator;
import org.opendaylight.vtn.manager.internal.util.inventory.SalPort;

import org.opendaylight.vtn.manager.internal.TestBase;

import org.opendaylight.controller.sal.utils.IPProtocols;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.VtnAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.vtn.data.flow.info.DataFlowAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.vtn.data.flow.info.DataFlowActionBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VlanType;

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
    public static Action createDropAction(int order) {
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
    public static Action createPopVlanAction(int order) {
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
    public static Action createPushVlanAction(int order) {
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
    public static Action createOutputAction(int order, String port) {
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
    public static Action createSetDlDstAction(int order, MacAddress mac) {
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
    public static Action createSetDlSrcAction(int order, MacAddress mac) {
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
    public static Action createSetTpDstAction(int order, int port) {
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
    public static Action createSetTpSrcAction(int order, int port) {
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
    public static Action createSetNwTosAction(int order, int dscp) {
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
    public static Action createSetNwDstAction(int order, Address addr) {
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
    public static Action createSetNwSrcAction(int order, Address addr) {
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
    public static Action createSetVlanIdAction(int order, int vid) {
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
    public static Action createSetVlanPcpAction(int order, int pri) {
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
     * Create a VTN DROP action.
     *
     * @param order  Action order in the action list.
     * @return  A VTN DROP action.
     */
    public static DataFlowAction createVtnDropAction(int order) {
        return new DataFlowActionBuilder().
            setOrder(order).
            setVtnAction(VTNDropAction.newVtnAction()).
            build();
    }

    /**
     * Create a VTN POP_VLAN action.
     *
     * @param order  Action order in the action list.
     * @return  A VTN POP_VLAN action.
     */
    public static DataFlowAction createVtnPopVlanAction(int order) {
        return new DataFlowActionBuilder().
            setOrder(order).
            setVtnAction(VTNPopVlanAction.newVtnAction()).
            build();
    }

    /**
     * Create a VTN PUSH_VLAN action.
     *
     * @param order  Action order in the action list.
     * @return  A VTN PUSH_VLAN action.
     */
    public static DataFlowAction createVtnPushVlanAction(int order) {
        return new DataFlowActionBuilder().
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
    public static DataFlowAction createVtnSetDlDstAction(int order,
                                                         MacAddress mac) {
        return new DataFlowActionBuilder().
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
    public static DataFlowAction createVtnSetDlSrcAction(int order,
                                                         MacAddress mac) {
        return new DataFlowActionBuilder().
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
    public static DataFlowAction createVtnSetIcmpCodeAction(int order,
                                                            int code) {
        return new DataFlowActionBuilder().
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
    public static DataFlowAction createVtnSetIcmpTypeAction(int order,
                                                            int type) {
        return new DataFlowActionBuilder().
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
    public static DataFlowAction createVtnSetInetDscpAction(int order,
                                                            int dscp) {
        return new DataFlowActionBuilder().
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
    public static DataFlowAction createVtnSetInetDstAction(int order,
                                                           Address addr) {
        return new DataFlowActionBuilder().
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
    public static DataFlowAction createVtnSetInetSrcAction(int order,
                                                           Address addr) {
        return new DataFlowActionBuilder().
            setOrder(order).
            setVtnAction(VTNSetInetSrcAction.newVtnAction(addr)).
            build();
    }

    /**
     * Create a VTN SET_TP_DST action.
     *
     * @param order  Action order in the action list.
     * @param port   A port number.
     * @return  A VTN SET_TP_DST action.
     */
    public static DataFlowAction createVtnSetPortDstAction(int order,
                                                           int port) {
        PortNumber pnum = new PortNumber(port);
        return new DataFlowActionBuilder().
            setOrder(order).
            setVtnAction(VTNSetPortDstAction.newVtnAction(pnum)).
            build();
    }

    /**
     * Create a VTN SET_TP_SRC action.
     *
     * @param order  Action order in the action list.
     * @param port   A port number.
     * @return  A VTN SET_TP_SRC action.
     */
    public static DataFlowAction createVtnSetPortSrcAction(int order,
                                                           int port) {
        PortNumber pnum = new PortNumber(port);
        return new DataFlowActionBuilder().
            setOrder(order).
            setVtnAction(VTNSetPortSrcAction.newVtnAction(pnum)).
            build();
    }

    /**
     * Create a VTN SET_VLAN_ID action.
     *
     * @param order  Action order in the action list.
     * @param vid    A VLAN ID.
     * @return  A VTN SET_VLAN_ID action.
     */
    public static DataFlowAction createVtnSetVlanIdAction(int order, int vid) {
        return new DataFlowActionBuilder().
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
    public static DataFlowAction createVtnSetVlanPcpAction(int order, int pri) {
        VlanPcp pcp = new VlanPcp((short)pri);
        return new DataFlowActionBuilder().
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
     * Test case for the following methods.
     *
     * <ul>
     *   <li>{@link FlowActionUtils#toFlowActions(List,Comparator)}</li>
     *   <li>{@link FlowActionConverter#toFlowAction(VtnAction)}</li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testToFlowAcitons() throws Exception {
        Comparator<Ordered> comp = new OrderedComparator();
        assertEquals(null, FlowActionUtils.toFlowActions(null, comp));

        List<DataFlowAction> actions = Collections.<DataFlowAction>emptyList();
        List<FlowAction> expected = new ArrayList<>();
        assertEquals(expected, FlowActionUtils.toFlowActions(actions, comp));

        // Try to convert a list that contains all the supported VTN actions.
        EtherAddress dlSrc = new EtherAddress(0x123456789L);
        EtherAddress dlDst = new EtherAddress(0xaabbccddee11L);
        Ip4Network nwSrc = new Ip4Network("192.168.100.200");
        Ip4Network nwDst = new Ip4Network("127.0.0.1");

        actions = new ArrayList<>();
        Collections.addAll(
            actions,
            createVtnDropAction(12),
            createVtnPopVlanAction(15),
            createVtnPushVlanAction(13),
            createVtnSetDlDstAction(6, dlDst.getMacAddress()),
            createVtnSetDlSrcAction(5, dlSrc.getMacAddress()),
            createVtnSetPortDstAction(1, 100),
            createVtnSetPortSrcAction(8, 33333),
            createVtnSetIcmpTypeAction(3, 129),
            createVtnSetIcmpCodeAction(7, 38),
            createVtnSetInetDscpAction(2, 45),
            createVtnSetInetDstAction(11, nwDst.getMdAddress()),
            createVtnSetInetSrcAction(14, nwSrc.getMdAddress()),
            createVtnSetVlanIdAction(9, 1000),
            createVtnSetVlanPcpAction(4, 5),

            // Insert broken VTN action.
            new DataFlowActionBuilder().setOrder(10).build());
        List<DataFlowAction> original = new ArrayList<>(actions);

        Collections.addAll(
            expected,
            new SetTpDstAction(100),
            new SetDscpAction((byte)45),
            new SetIcmpTypeAction((short)129),
            new SetVlanPcpAction((byte)5),
            new SetDlSrcAction(dlSrc),
            new SetDlDstAction(dlDst),
            new SetIcmpCodeAction((short)38),
            new SetTpSrcAction(33333),
            new SetVlanIdAction((short)1000),
            new SetInet4DstAction(nwDst),
            new DropAction(),
            new PushVlanAction(VlanType.VLAN.getIntValue()),
            new SetInet4SrcAction(nwSrc),
            new PopVlanAction());

        assertEquals(expected, FlowActionUtils.toFlowActions(actions, comp));

        // The specified list should not be changed.
        assertEquals(original, actions);

        // Sort actions in descending order.
        comp = Collections.reverseOrder(comp);
        Collections.reverse(expected);
        assertEquals(expected, FlowActionUtils.toFlowActions(actions, comp));
        assertEquals(original, actions);
    }

    /**
     * Test case for the following methods.
     *
     * <ul>
     *   <li>{@link FlowActionUtils#toDataFlowActions(List,Comparator,Short)}</li>
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
        List<DataFlowAction> drop = Collections.
            singletonList(createVtnDropAction(0));
        assertEquals(drop,
                     FlowActionUtils.toDataFlowActions(actions, comp, null));
        actions = new ArrayList<>();
        assertEquals(drop,
                     FlowActionUtils.toDataFlowActions(actions, comp, null));

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
        List<DataFlowAction> expected = new ArrayList<>();
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
        List<DataFlowAction> reversed = new ArrayList<>(expected);
        Collections.reverse(reversed);

        for (Short ipproto: protocols) {
            List<DataFlowAction> results = FlowActionUtils.
                toDataFlowActions(actions, comp, ipproto);
            assertEquals(expected, results);

            // The specified list should not be changed.
            assertEquals(original, actions);

            // Sort actions in descending order.
            results = FlowActionUtils.
                toDataFlowActions(actions, rcomp, ipproto);
            assertEquals(expected.size(), results.size());
            for (int order = 0; order < results.size(); order++) {
                VtnAction ex = reversed.get(order).getVtnAction();
                DataFlowAction dfa = results.get(order);
                assertEquals(order, dfa.getOrder().intValue());
                assertEquals(ex, dfa.getVtnAction());
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
            IPProtocols.TCP.shortValue(),
            IPProtocols.UDP.shortValue(),
        };

        for (Short ipproto: protocols) {
            List<DataFlowAction> results = FlowActionUtils.
                toDataFlowActions(actions, comp, ipproto);
            assertEquals(expected, results);
            assertEquals(original, actions);

            results = FlowActionUtils.
                toDataFlowActions(actions, rcomp, ipproto);
            assertEquals(expected.size(), results.size());
            for (int order = 0; order < results.size(); order++) {
                VtnAction ex = reversed.get(order).getVtnAction();
                DataFlowAction dfa = results.get(order);
                assertEquals(order, dfa.getOrder().intValue());
                assertEquals(ex, dfa.getVtnAction());
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

        Short ipproto = IPProtocols.ICMP.shortValue();
        List<DataFlowAction> results = FlowActionUtils.
            toDataFlowActions(actions, comp, ipproto);
        assertEquals(expected, results);
        assertEquals(original, actions);

        results = FlowActionUtils.toDataFlowActions(actions, rcomp, ipproto);
        assertEquals(expected.size(), results.size());
        for (int order = 0; order < results.size(); order++) {
            VtnAction ex = reversed.get(order).getVtnAction();
            DataFlowAction dfa = results.get(order);
            assertEquals(order, dfa.getOrder().intValue());
            assertEquals(ex, dfa.getVtnAction());
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
