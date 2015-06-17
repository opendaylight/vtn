/*
 * Copyright (c) 2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.flow.action;

import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.List;
import java.util.ListIterator;

import org.opendaylight.vtn.manager.flow.action.FlowAction;

import org.opendaylight.vtn.manager.internal.L2Host;
import org.opendaylight.vtn.manager.internal.util.MiscUtils;
import org.opendaylight.vtn.manager.internal.util.inventory.SalPort;
import org.opendaylight.vtn.manager.internal.util.packet.EtherHeader;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.VtnOrderedFlowAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.VtnAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.vtn.data.flow.info.DataFlowAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.vtn.data.flow.info.DataFlowActionBuilder;

import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.Ordered;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.OutputActionCase;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.PopVlanActionCase;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.SetDlDstActionCase;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.SetVlanIdActionCase;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.output.action._case.OutputAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.set.dl.dst.action._case.SetDlDstAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.set.vlan.id.action._case.SetVlanIdAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.list.Action;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.rev131026.InstructionList;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.rev131026.instruction.instruction.ApplyActionsCase;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.rev131026.instruction.instruction.apply.actions._case.ApplyActions;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.rev131026.instruction.list.Instruction;

import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.inet.types.rev100924.Uri;
import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.yang.types.rev100924.MacAddress;
import org.opendaylight.yang.gen.v1.urn.opendaylight.l2.types.rev130827.VlanId;

/**
 * {@code FlowActionUtils} class is a collection of utility class methods for
 * flow action handling.
 *
 * <p>
 *   Note that this class assumes that:
 * </p>
 * <ul>
 *   <li>
 *     Every flow instruction list contains only one APPLY_ACTION list.
 *   </li>
 *   <li>
 *     OUTPUT action is always the last element of the flow action list.
 *   </li>
 * </ul>
 */
public final class FlowActionUtils {
    /**
     * Private constructor that protects this class from instantiating.
     */
    private FlowActionUtils() {}

    /**
     * Return the output port configured in the given action list.
     *
     * @param actions  A list of MD-SAL actions.
     * @return  A MD-SAL node connector identifier which specifies the output
     *          switch port. {@code null} if not found.
     */
    public static String getOutputPort(List<Action> actions) {
        for (ListIterator<Action> it = actions.listIterator(actions.size());
             it.hasPrevious();) {
            Action act = it.previous();
            org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.
                rev131112.action.Action a = act.getAction();
            if (a instanceof OutputActionCase) {
                return getOutputPort((OutputActionCase)a);
            }
        }

        return null;
    }

    /**
     * Return the destination host configured in the given action list.
     *
     * @param actions  A list of MD-SAL actions.
     * @param mac      The destination MAC address specified by the ingress
     *                 flow entry.
     * @param vlan     The VLAN ID specified by the ingress flow entry.
     * @return  A {@link L2Host} instance if found.
     *          {@code null} if not found.
     */
    public static L2Host getDestinationHost(List<Action> actions,
                                            MacAddress mac, int vlan) {
        MacAddress dst = mac;
        int vid = vlan;
        String port = null;
        for (Action act: actions) {
            org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.
                rev131112.action.Action a = act.getAction();
            if (a instanceof OutputActionCase) {
                port = getOutputPort((OutputActionCase)a);
            } else if (a instanceof SetDlDstActionCase) {
                dst = getMacAddress((SetDlDstActionCase)a);
            } else {
                vid = getOutputVlan(a, vid);
            }
        }

        SalPort sport = SalPort.create(port);
        return (sport == null)
            ? null
            : new L2Host(dst, vid, sport);
    }

    /**
     * Return the action list configured in the given flow instructions.
     *
     * <p>
     *   Note that this method returns unmodifiable list.
     * </p>
     *
     * @param insts  An {@link InstructionList} instance.
     * @param comp   Comparator for MD-SAL actions.
     *               If {@code null} is specified, this method returns
     *               unsorted list.
     * @return  A list of {@link Action} instances.
     */
    public static List<Action> getActions(InstructionList insts,
                                          Comparator<Ordered> comp) {
        if (insts != null) {
            List<Instruction> ilist = insts.getInstruction();
            if (ilist != null && !ilist.isEmpty()) {
                Instruction inst = ilist.iterator().next();
                ApplyActionsCase apCase = MiscUtils.cast(
                    ApplyActionsCase.class, inst.getInstruction());
                if (apCase != null) {
                    ApplyActions apply = apCase.getApplyActions();
                    if (apply != null) {
                        List<Action> actions = apply.getAction();
                        if (comp != null) {
                            actions = MiscUtils.sortedCopy(actions, comp);
                        }

                        return actions;
                    }
                }
            }
        }

        return Collections.<Action>emptyList();
    }

    /**
     * Convert the given VTN flow action list into a list of {@link FlowAction}
     * instances.
     *
     * @param actions  A list of {@link VtnOrderedFlowAction} instances.
     * @param comp     A comparator for the given list.
     * @param <T>      The type of the action in {@code actions}.
     * @return  A list of {@link FlowAction} instances.
     * @throws RpcException
     *    Failed to convert the given instance.
     */
    public static <T extends VtnOrderedFlowAction> List<FlowAction> toFlowActions(
        List<T> actions, Comparator<? super T> comp) throws RpcException {
        if (actions == null) {
            return null;
        }
        if (actions.isEmpty()) {
            return Collections.<FlowAction>emptyList();
        }

        // Sort the given list.
        List<T> list = MiscUtils.sortedCopy(actions, comp);
        List<FlowAction> facts = new ArrayList<>(list.size());
        FlowActionConverter conv = FlowActionConverter.getInstance();
        for (T vaction: list) {
            VtnAction vact = vaction.getVtnAction();
            FlowAction fact = conv.toFlowAction(vact);
            if (fact != null) {
                facts.add(fact);
            }
        }

        return facts;
    }

    /**
     * Convert the given MD-SAL action list into a list of
     * {@link DataFlowAction} instances.
     *
     * @param actions  A list of {@link VtnOrderedFlowAction} instances.
     * @param comp     A comparator for the given list.
     * @param ipproto  An IP protocol number configured in the flow match.
     * @return  A list of {@link DataFlowAction} instances.
     * @throws RpcException
     *    Failed to convert the given instance.
     */
    public static List<DataFlowAction> toDataFlowActions(
        List<Action> actions, Comparator<? super Action> comp, Short ipproto)
        throws RpcException {
        int order = MiscUtils.ORDER_MIN;
        if (actions == null || actions.isEmpty()) {
            DataFlowAction dfa = new DataFlowActionBuilder().
                setVtnAction(VTNDropAction.newVtnAction()).
                setOrder(order).build();
            return Collections.singletonList(dfa);
        }

        List<Action> list = MiscUtils.sortedCopy(actions, comp);
        List<DataFlowAction> dfacts = new ArrayList<>(list.size());
        FlowActionConverter conv = FlowActionConverter.getInstance();
        for (Action a: list) {
            VtnAction vact = conv.toVtnAction(a.getAction(), ipproto);
            if (vact != null) {
                DataFlowAction dfa = new DataFlowActionBuilder().
                    setVtnAction(vact).setOrder(order).build();
                order++;
                dfacts.add(dfa);
            }
        }

        return dfacts;
    }

    /**
     * Return the egress switch port specified by the given action.
     *
     * @param action  An {@link OutputActionCase} instance.
     * @return  A MD-SAL node connector identifier which specifies the output
     *          switch port. {@code null} if not found.
     */
    private static String getOutputPort(OutputActionCase action) {
        OutputAction output = action.getOutputAction();
        if (output != null) {
            Uri uri = output.getOutputNodeConnector();
            if (uri != null) {
                return uri.getValue();
            }
        }

        return null;
    }

    /**
     * Return the destination MAC address specified by the given action.
     *
     * @param action  A {@link SetDlDstActionCase} instance.
     * @return  A {@link MacAddress} instance if found.
     *          {@code null} if not found.
     */
    private static MacAddress getMacAddress(SetDlDstActionCase action) {
        SetDlDstAction act = action.getSetDlDstAction();
        return (act == null) ? null : act.getAddress();
    }

    /**
     * Return the VLAN ID specified by the given action.
     *
     * @param action  An action in a flow entry.
     * @param vid     VLAN ID of the packet currently processing.
     * @return  VLAN ID to be used for succeeding packet processing.
     *          {@link EtherHeader#VLAN_NONE} is returned if the given action
     *          removes the VLAN tag.
     */
    private static int getOutputVlan(
        org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.Action action,
        int vid) {
        if (action instanceof PopVlanActionCase) {
            return EtherHeader.VLAN_NONE;
        }
        if (action instanceof SetVlanIdActionCase) {
            SetVlanIdActionCase act = (SetVlanIdActionCase)action;
            SetVlanIdAction setVid = act.getSetVlanIdAction();
            if (setVid != null) {
                VlanId vlanId = setVid.getVlanId();
                if (vlanId != null) {
                    return vlanId.getValue().intValue();
                }
            }
        }

        return vid;
    }
}
