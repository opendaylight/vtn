/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.flow.action;

import javax.xml.bind.annotation.XmlAccessType;
import javax.xml.bind.annotation.XmlAccessorType;
import javax.xml.bind.annotation.XmlElement;
import javax.xml.bind.annotation.XmlRootElement;

import org.opendaylight.vtn.manager.flow.action.FlowAction;

import org.opendaylight.vtn.manager.internal.util.MiscUtils;
import org.opendaylight.vtn.manager.internal.util.ProtocolUtils;
import org.opendaylight.vtn.manager.internal.util.packet.EtherHeader;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.VtnAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.VtnSetVlanPcpActionCase;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.VtnSetVlanPcpActionCaseBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.vtn.set.vlan.pcp.action._case.VtnSetVlanPcpAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.vtn.set.vlan.pcp.action._case.VtnSetVlanPcpActionBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.flow.action.list.VtnFlowActionBuilder;

import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.Action;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.SetVlanPcpActionCase;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.SetVlanPcpActionCaseBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.set.vlan.pcp.action._case.SetVlanPcpAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.set.vlan.pcp.action._case.SetVlanPcpActionBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.list.ActionBuilder;

import org.opendaylight.yang.gen.v1.urn.opendaylight.l2.types.rev130827.VlanPcp;

/**
 * {@code VTNSetVlanPcpAction} describes the flow action that sets the VLAN
 * priority into the IEEE 802.1Q VLAN tag.
 */
@XmlRootElement(name = "vtn-set-vlan-pcp-action")
@XmlAccessorType(XmlAccessType.NONE)
public final class VTNSetVlanPcpAction extends FlowFilterAction {
    /**
     * Default VLAN priority.
     */
    private static final short  DEFAULT_VALUE = 0;

    /**
     * The VLAN priority value to be set.
     */
    @XmlElement
    private short  priority;

    /**
     * Create a new {@link VtnSetVlanPcpActionCase} instance.
     *
     * @param pcp  A {@link VlanPcp} instance which specifies the
     *             VLAN priority.
     * @return  A {@link VtnSetVlanPcpActionCase} instance.
     */
    public static VtnSetVlanPcpActionCase newVtnAction(VlanPcp pcp) {
        VtnSetVlanPcpAction vaction = new VtnSetVlanPcpActionBuilder().
            setVlanPcp(pcp).build();
        return new VtnSetVlanPcpActionCaseBuilder().
            setVtnSetVlanPcpAction(vaction).build();
    }

    /**
     * Construct an empty instance.
     */
    VTNSetVlanPcpAction() {
    }

    /**
     * Construct a new instane without specifying action order.
     *
     * @param pri  A VLAN priority value to be set.
     */
    public VTNSetVlanPcpAction(short pri) {
        priority = pri;
    }

    /**
     * Construct a new instance.
     *
     * @param act
     *    A {@link org.opendaylight.vtn.manager.flow.action.SetVlanPcpAction}
     *    instance.
     * @param ord  An integer which determines the order of flow actions
     *             in a flow entry.
     * @throws RpcException  An invalid argument is specified.
     */
    public VTNSetVlanPcpAction(
        org.opendaylight.vtn.manager.flow.action.SetVlanPcpAction act, int ord)
        throws RpcException {
        super(Integer.valueOf(ord));
        priority = (short)act.getPriority();
        verify();
    }

    /**
     * Construct a new instance.
     *
     * @param ac   A {@link VtnSetVlanPcpActionCase} instance.
     * @param ord  An integer which determines the order of flow actions
     *             in a flow entry.
     * @throws RpcException  An invalid argument is specified.
     */
    public VTNSetVlanPcpAction(VtnSetVlanPcpActionCase ac, Integer ord)
        throws RpcException {
        super(ord);
        priority = getVlanPriority(ac.getVtnSetVlanPcpAction());
        verify();
    }

    /**
     * Return the VLAN priority to be set.
     *
     * @return  The VLAN priority to be set.
     */
    public short getPriority() {
        return priority;
    }

    /**
     * Return the VLAN priority in the given instance.
     *
     * @param vaction  A {@link VtnSetVlanPcpAction} instance.
     * @return  The VLAN priority.
     */
    private short getVlanPriority(VtnSetVlanPcpAction vaction) {
        if (vaction != null) {
            VlanPcp pcp = vaction.getVlanPcp();
            if (pcp != null) {
                Short value = pcp.getValue();
                if (value != null) {
                    return value.shortValue();
                }
            }
        }

        return DEFAULT_VALUE;
    }

    // VTNFlowAction

    /**
     * {@inheritDoc}
     */
    @Override
    public FlowAction toFlowAction(VtnAction vact) throws RpcException {
        VtnSetVlanPcpActionCase ac = cast(VtnSetVlanPcpActionCase.class, vact);
        short pri = getVlanPriority(ac.getVtnSetVlanPcpAction());
        return new org.opendaylight.vtn.manager.flow.action.
            SetVlanPcpAction((byte)pri);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public VtnSetVlanPcpActionCase toVtnAction(Action act) throws RpcException {
        SetVlanPcpActionCase ac = cast(SetVlanPcpActionCase.class, act);
        SetVlanPcpAction action = ac.getSetVlanPcpAction();
        if (action != null) {
            VlanPcp pcp = action.getVlanPcp();
            if (pcp != null) {
                return newVtnAction(pcp);
            }
        }

        String msg = getErrorMessage("No VLAN PCP", ac);
        throw RpcException.getMissingArgumentException(msg);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public String getDescription(Action act) throws RpcException {
        SetVlanPcpActionCase ac = cast(SetVlanPcpActionCase.class, act);
        SetVlanPcpAction action = ac.getSetVlanPcpAction();
        Short pcp = null;
        if (action != null) {
            VlanPcp vpcp = action.getVlanPcp();
            pcp = (vpcp == null) ? null : vpcp.getValue();
        }

        return new StringBuilder("SET_VLAN_PCP(pcp=").
            append(pcp).append(')').toString();
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected VtnFlowActionBuilder set(VtnFlowActionBuilder builder) {
        VlanPcp pcp = new VlanPcp(Short.valueOf(priority));
        return builder.setVtnAction(newVtnAction(pcp));
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected ActionBuilder set(ActionBuilder builder) {
        SetVlanPcpAction pcp = new SetVlanPcpActionBuilder().
            setVlanPcp(new VlanPcp(Short.valueOf(priority))).build();
        return builder.setAction(new SetVlanPcpActionCaseBuilder().
                                 setSetVlanPcpAction(pcp).build());
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void appendContents(StringBuilder builder) {
        builder.append("pcp=").append(Short.toString(priority));
        super.appendContents(builder);
    }

    // FlowFilterAction

    /**
     * {@inheritDoc}
     */
    @Override
    public boolean apply(FlowActionContext ctx) {
        EtherHeader ether = ctx.getEtherHeader();
        ether.setVlanPriority(priority);
        ctx.addFilterAction(this);
        return true;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void verifyImpl() throws RpcException {
        if (!ProtocolUtils.isVlanPriorityValid(priority)) {
            String msg = getErrorMessage("Invalid VLAN priority", priority);
            throw RpcException.getBadArgumentException(msg);
        }
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

        VTNSetVlanPcpAction va = (VTNSetVlanPcpAction)o;
        return (priority == va.priority);
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        return super.hashCode() * MiscUtils.HASH_PRIME + (int)priority;
    }
}
