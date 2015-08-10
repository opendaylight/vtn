/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.flow.action;

import org.opendaylight.vtn.manager.flow.action.FlowAction;

import org.opendaylight.vtn.manager.internal.util.MiscUtils;
import org.opendaylight.vtn.manager.internal.util.ProtocolUtils;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.VtnAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.VtnSetVlanIdActionCase;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.VtnSetVlanIdActionCaseBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.vtn.set.vlan.id.action._case.VtnSetVlanIdAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.vtn.set.vlan.id.action._case.VtnSetVlanIdActionBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.flow.action.list.VtnFlowActionBuilder;

import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.Action;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.SetVlanIdActionCase;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.SetVlanIdActionCaseBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.set.vlan.id.action._case.SetVlanIdAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.set.vlan.id.action._case.SetVlanIdActionBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.list.ActionBuilder;

import org.opendaylight.yang.gen.v1.urn.opendaylight.l2.types.rev130827.VlanId;

/**
 * {@code VTNSetVlanIdAction} describes the flow action that sets the VLAN ID
 * into VLAN tag.
 */
public final class VTNSetVlanIdAction extends VTNFlowAction {
    /**
     * The VLAN ID to be set.
     */
    private int  vlanId;

    /**
     * Create a new {@link VtnSetVlanIdActionCase} instance.
     *
     * @param vid  An {@link Integer} instance which specifies the VLAN ID.
     * @return  A {@link VtnSetVlanIdActionCase} instance.
     */
    public static VtnSetVlanIdActionCase newVtnAction(Integer vid) {
        VtnSetVlanIdAction vaction = new VtnSetVlanIdActionBuilder().
            setVlanId(vid).build();
        return new VtnSetVlanIdActionCaseBuilder().
            setVtnSetVlanIdAction(vaction).build();
    }

    /**
     * Construct an empty instance.
     */
    VTNSetVlanIdAction() {
    }

    /**
     * Construct a new instance that sets the VLAN ID into VLAN tag.
     *
     * @param vid  The VLAN ID to be set.
     */
    public VTNSetVlanIdAction(int vid) {
        vlanId = vid;
    }

    /**
     * Return the VLAN ID to be set.
     *
     * @return  The VLAN ID to be set.
     */
    public int getVlanId() {
        return vlanId;
    }

    // VTNFlowAction

    /**
     * {@inheritDoc}
     */
    @Override
    public FlowAction toFlowAction(VtnAction vact) throws RpcException {
        VtnSetVlanIdActionCase ac = cast(VtnSetVlanIdActionCase.class, vact);
        VtnSetVlanIdAction vaction = ac.getVtnSetVlanIdAction();
        if (vaction != null) {
            Integer arg = vaction.getVlanId();
            if (arg != null) {
                return new org.opendaylight.vtn.manager.flow.action.
                    SetVlanIdAction(arg.shortValue());
            }
        }

        String msg = getErrorMessage("No VLAN ID", vact);
        throw RpcException.getMissingArgumentException(msg);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public VtnSetVlanIdActionCase toVtnAction(Action act) throws RpcException {
        SetVlanIdActionCase ac = cast(SetVlanIdActionCase.class, act);
        SetVlanIdAction action = ac.getSetVlanIdAction();
        if (action != null) {
            Integer vid = ProtocolUtils.getVlanId(action.getVlanId());
            if (vid != null) {
                return newVtnAction(vid);
            }
        }

        String msg = getErrorMessage("No VLAN ID", ac);
        throw RpcException.getMissingArgumentException(msg);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public String getDescription(Action act) throws RpcException {
        SetVlanIdActionCase ac = cast(SetVlanIdActionCase.class, act);
        SetVlanIdAction action = ac.getSetVlanIdAction();
        Integer vlan = null;
        if (action != null) {
            VlanId vid = action.getVlanId();
            vlan = (vid == null) ? null : vid.getValue();
        }

        return new StringBuilder("SET_VLAN_ID(vid=").
            append(vlan).append(')').toString();
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected VtnFlowActionBuilder set(VtnFlowActionBuilder builder) {
        return builder.setVtnAction(newVtnAction(vlanId));
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected ActionBuilder set(ActionBuilder builder) {
        SetVlanIdAction vid = new SetVlanIdActionBuilder().
            setVlanId(new VlanId(vlanId)).build();
        return builder.setAction(new SetVlanIdActionCaseBuilder().
                                 setSetVlanIdAction(vid).build());
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void appendContents(StringBuilder builder) {
        builder.append("vlan=").append(vlanId);
    }

    // Object

    /**
     * Determine whether the given object is identical to this object.
     *
     * @param o  An object to be compared.
     * @return   {@code true} if identical. Otherwise {@code false}.
     */
    @Override
    public boolean equals(Object o) {
        if (o == this) {
            return true;
        }
        if (!super.equals(o)) {
            return false;
        }

        VTNSetVlanIdAction va = (VTNSetVlanIdAction)o;
        return (vlanId == va.vlanId);
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        return super.hashCode() * MiscUtils.HASH_PRIME + vlanId;
    }
}
