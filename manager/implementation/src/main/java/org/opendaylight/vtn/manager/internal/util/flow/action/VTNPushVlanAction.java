/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.flow.action;

import java.util.Objects;

import org.opendaylight.vtn.manager.flow.action.FlowAction;

import org.opendaylight.vtn.manager.internal.util.MiscUtils;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.VtnAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.VtnPushVlanActionCase;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.VtnPushVlanActionCaseBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.vtn.push.vlan.action._case.VtnPushVlanAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.vtn.push.vlan.action._case.VtnPushVlanActionBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.flow.action.list.VtnFlowActionBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VlanType;

import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.Action;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.PushVlanActionCase;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.PushVlanActionCaseBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.push.vlan.action._case.PushVlanAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.push.vlan.action._case.PushVlanActionBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.list.ActionBuilder;

/**
 * {@code VTNPushVlanAction} describes the flow action that adds a VLAN tag
 * into packet.
 */
public final class VTNPushVlanAction extends VTNFlowAction {
    /**
     * The Ethernet type for a new VLAN tag.
     */
    private VlanType  vlanType;

    /**
     * Create a new {@link VtnPushVlanActionCase} instance.
     *
     * @param vtype  A {@link VlanType} instance which specifies the type of
     *               the VLAN tag.
     * @return  A {@link VtnPushVlanActionCase} instance.
     */
    public static VtnPushVlanActionCase newVtnAction(VlanType vtype) {
        VtnPushVlanAction vaction = new VtnPushVlanActionBuilder().
            setVlanType(vtype).build();
        return new VtnPushVlanActionCaseBuilder().
            setVtnPushVlanAction(vaction).build();
    }

    /**
     * Construct a new instance that adds an IEEE 802.1q VLAN tag.
     */
    public VTNPushVlanAction() {
        this(VlanType.VLAN);
    }

    /**
     * Construct a new instance.
     *
     * @param type  A {@link VlanType} instance which specifies the VLAN type.
     */
    public VTNPushVlanAction(VlanType type) {
        vlanType = type;
    }

    /**
     * Return the VLAN type for a new VLAN tag.
     *
     * @return  A {@link VlanType} instance.
     */
    public VlanType getVlanType() {
        return vlanType;
    }

    /**
     * Return an exception which indicates VLAN type is missing.
     *
     * @param obj  An object to be added to the error message.
     * @return  A {@link RpcException} instance.
     */
    private RpcException noVlanType(Object obj) {
        String msg = getErrorMessage("No VLAN type", obj);
        return RpcException.getMissingArgumentException(msg);
    }

    // VTNFlowAction

    /**
     * {@inheritDoc}
     */
    @Override
    public FlowAction toFlowAction(VtnAction vact) throws RpcException {
        VtnPushVlanActionCase ac = cast(VtnPushVlanActionCase.class, vact);
        VtnPushVlanAction vaction = ac.getVtnPushVlanAction();
        if (vaction != null) {
            VlanType vtype = vaction.getVlanType();
            if (vtype != null) {
                return new org.opendaylight.vtn.manager.flow.action.
                    PushVlanAction(vtype.getIntValue());
            }
        }

        throw noVlanType(ac);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public VtnPushVlanActionCase toVtnAction(Action act) throws RpcException {
        PushVlanActionCase ac = cast(PushVlanActionCase.class, act);
        PushVlanAction action = ac.getPushVlanAction();
        if (action != null) {
            Integer etype = action.getEthernetType();
            if (etype != null) {
                VlanType vtype = VlanType.forValue(etype.intValue());
                if (vtype == null) {
                    String msg = getErrorMessage("Unsupported VLAN type: " +
                                                 etype);
                    throw RpcException.getBadArgumentException(msg);
                }

                return newVtnAction(vtype);
            }
        }

        throw noVlanType(ac);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public String getDescription(Action act) throws RpcException {
        PushVlanActionCase ac = cast(PushVlanActionCase.class, act);
        PushVlanAction action = ac.getPushVlanAction();
        String type = null;
        if (action != null) {
            Integer etype = action.getEthernetType();
            if (etype != null) {
                type = String.format("0x%x", etype);
            }
        }

        return new StringBuilder("PUSH_VLAN(type=").
            append(type).append(')').toString();
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected VtnFlowActionBuilder set(VtnFlowActionBuilder builder) {
        return builder.setVtnAction(newVtnAction(vlanType));
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected ActionBuilder set(ActionBuilder builder) {
        PushVlanActionBuilder pbuilder = new PushVlanActionBuilder();
        if (vlanType != null) {
            pbuilder.setEthernetType(vlanType.getIntValue());
        }

        return builder.setAction(new PushVlanActionCaseBuilder().
                                 setPushVlanAction(pbuilder.build()).build());
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void appendContents(StringBuilder builder) {
        builder.append("type=").append(vlanType);
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

        VTNPushVlanAction va = (VTNPushVlanAction)o;
        return Objects.equals(vlanType, va.vlanType);
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        int h = super.hashCode();
        if (vlanType != null) {
            h = h * MiscUtils.HASH_PRIME + vlanType.hashCode();
        }

        return h;
    }
}
