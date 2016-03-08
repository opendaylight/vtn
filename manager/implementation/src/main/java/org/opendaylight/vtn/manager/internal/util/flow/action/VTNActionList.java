/*
 * Copyright (c) 2015, 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.flow.action;

import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.List;

import org.opendaylight.vtn.manager.internal.util.MiscUtils;
import org.opendaylight.vtn.manager.internal.util.inventory.SalPort;
import org.opendaylight.vtn.manager.internal.util.packet.EtherHeader;

import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.list.Action;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.rev131026.flow.Instructions;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.rev131026.flow.InstructionsBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.rev131026.instruction.instruction.ApplyActionsCase;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.rev131026.instruction.instruction.ApplyActionsCaseBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.rev131026.instruction.instruction.apply.actions._case.ApplyActionsBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.rev131026.instruction.list.Instruction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.rev131026.instruction.list.InstructionBuilder;

/**
 * {@code VTNActionList} is a utility to construct a list of flow actions
 * in a flow entry.
 */
public final class VTNActionList {
    /**
     * A list of MD-SAL flow actions.
     */
    private final List<VTNFlowAction>  actionList = new ArrayList<>();

    /**
     * Create a new apply action that contains the specified MD-SAL action
     * list.
     *
     * @param actions  A list of MD-SAL actions.
     * @return  An {@link Instructions} instance.
     */
    public static Instructions newInstructions(List<Action> actions) {
        ApplyActionsCase apply = new ApplyActionsCaseBuilder().
            setApplyActions(new ApplyActionsBuilder().
                            setAction(actions).build()).
            build();
        Instruction inst = new InstructionBuilder().
            setOrder(MiscUtils.ORDER_MIN).setInstruction(apply).build();

        return new InstructionsBuilder().
            setInstruction(Collections.singletonList(inst)).build();
    }

    /**
     * Append all flow actions in the given collection to the tail of the
     * action list.
     *
     * @param c  A collection of VTN flow actions.
     * @return  This object.
     */
    public VTNActionList addAll(Collection<? extends VTNFlowAction> c) {
        if (c != null) {
            actionList.addAll(c);
        }
        return this;
    }

    /**
     * Add a flow action which transmits packet to the specified switch port
     * to the tail of the action list.
     *
     * @param sport  A {@link SalPort} instance corresponding to the output
     *               switch port.
     * @return  This object.
     */
    public VTNActionList addOutputAction(SalPort sport) {
        actionList.add(new VTNOutputAction(sport));
        return this;
    }

    /**
     * Add a flow action which modifies the VLAN ID.
     *
     * @param inVid   A VLAN ID configured in the origial packet.
     *                {@link EtherHeader#VLAN_NONE} indicates that the VLAN tag
     *                is not present in the original packet.
     * @param outVid  A VLAN ID to be set.
     *                If {@link EtherHeader#VLAN_NONE} is specified, the VLAN
     *                tag in the outgoing packet will be stripped.
     * @return  This object.
     */
    public VTNActionList addVlanAction(int inVid, int outVid) {
        // Don't append action if not needed.
        if (inVid != outVid) {
            if (outVid == EtherHeader.VLAN_NONE) {
                // Strip the VLAN tag.
                actionList.add(new VTNPopVlanAction());
            } else {
                if (inVid == EtherHeader.VLAN_NONE) {
                    // Add a new VLAN tag.
                    actionList.add(new VTNPushVlanAction());
                }
                actionList.add(new VTNSetVlanIdAction(outVid));
            }
        }

        return this;
    }

    /**
     * Return a list of MD-SAL actions.
     *
     * @return  A list of MD-SAL actions.
     */
    public List<Action> toActions() {
        int size = actionList.size();
        int order = MiscUtils.ORDER_MIN;
        if (size == 0) {
            // Set a flow action that drops every packet.
            VTNDropAction drop = new VTNDropAction();
            return Collections.singletonList(
                drop.toActionBuilder(order).build());
        }

        List<Action> actions = new ArrayList<>(size);
        for (VTNFlowAction va: actionList) {
            actions.add(va.toActionBuilder(order).build());
            order++;
        }

        return actions;
    }

    /**
     * Return a flow instructions to be set in a flow entry.
     *
     * @return  An {@link Instructions} instance.
     */
    public Instructions toInstructions() {
        return newInstructions(toActions());
    }
}
