/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.flow.action;

import static org.opendaylight.vtn.manager.util.NumberUtils.HASH_PRIME;

import javax.xml.bind.annotation.XmlAccessType;
import javax.xml.bind.annotation.XmlAccessorType;
import javax.xml.bind.annotation.XmlElement;
import javax.xml.bind.annotation.XmlRootElement;

import org.opendaylight.vtn.manager.flow.action.FlowAction;
import org.opendaylight.vtn.manager.flow.action.SetIcmpTypeAction;
import org.opendaylight.vtn.manager.util.NumberUtils;

import org.opendaylight.vtn.manager.internal.util.MiscUtils;
import org.opendaylight.vtn.manager.internal.util.ProtocolUtils;
import org.opendaylight.vtn.manager.internal.util.packet.IcmpHeader;
import org.opendaylight.vtn.manager.internal.util.packet.Layer4Header;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.VtnAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.VtnSetIcmpTypeActionCase;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.VtnSetIcmpTypeActionCaseBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.vtn.set.icmp.type.action._case.VtnSetIcmpTypeAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.vtn.set.icmp.type.action._case.VtnSetIcmpTypeActionBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.flow.action.list.VtnFlowActionBuilder;

import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.Action;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.SetTpSrcActionCase;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.SetTpSrcActionCaseBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.set.tp.src.action._case.SetTpSrcAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.set.tp.src.action._case.SetTpSrcActionBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.list.ActionBuilder;

import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.inet.types.rev100924.PortNumber;

/**
 * {@code VTNSetIcmpTypeAction} describes the flow action that sets the
 * ICMP type into ICMP header.
 */
@XmlRootElement(name = "vtn-set-icmp-type-action")
@XmlAccessorType(XmlAccessType.NONE)
public final class VTNSetIcmpTypeAction extends FlowFilterAction {
    /**
     * Default ICMP type.
     */
    private static final short  DEFAULT_VALUE = 0;

    /**
     * The ICMP type value to be set.
     */
    @XmlElement
    private short  type;

    /**
     * Create a new {@link VtnSetIcmpTypeActionCase} instance.
     *
     * @param icmpType  A {@link Short} instance which specifies the ICMP type.
     * @return  A {@link VtnSetIcmpTypeActionCase} instance.
     */
    public static VtnSetIcmpTypeActionCase newVtnAction(Short icmpType) {
        VtnSetIcmpTypeAction vaction = new VtnSetIcmpTypeActionBuilder().
            setType(icmpType).build();
        return new VtnSetIcmpTypeActionCaseBuilder().
            setVtnSetIcmpTypeAction(vaction).build();
    }

    /**
     * Construct an empty instance.
     */
    VTNSetIcmpTypeAction() {
    }

    /**
     * Construct a new instance without specifying action order.
     *
     * @param t  The ICMP type to be set.
     */
    public VTNSetIcmpTypeAction(short t) {
        type = t;
    }

    /**
     * Construct a new instance without specifying action order.
     *
     * @param t    The ICMP type to be set.
     * @param ord  An integer which determines the order of flow actions
     *             in a flow entry.
     */
    public VTNSetIcmpTypeAction(short t, Integer ord) {
        super(ord);
        type = t;
    }

    /**
     * Construct a new instance.
     *
     * @param act  A {@link SetIcmpTypeAction} instance.
     * @param ord  An integer which determines the order of flow actions
     *             in a flow entry.
     * @throws RpcException  An invalid argument is specified.
     */
    public VTNSetIcmpTypeAction(SetIcmpTypeAction act, int ord)
        throws RpcException {
        super(Integer.valueOf(ord));
        type = act.getType();
        verify();
    }

    /**
     * Construct a new instance.
     *
     * @param ac   A {@link VtnSetIcmpTypeActionCase} instance.
     * @param ord  An integer which determines the order of flow actions
     *             in a flow entry.
     * @throws RpcException  An invalid argument is specified.
     */
    public VTNSetIcmpTypeAction(VtnSetIcmpTypeActionCase ac, Integer ord)
        throws RpcException {
        super(ord);
        type = getIcmpType(ac.getVtnSetIcmpTypeAction());
        verify();
    }

    /**
     * Return the ICMP type to be set.
     *
     * @return  The ICMP type to be set.
     */
    public short getType() {
        return type;
    }

    /**
     * Return the ICMP type configured in the given instance.
     *
     * @param vaction  A {@link VtnSetIcmpTypeAction} instance.
     * @return  ICMP type.
     */
    private short getIcmpType(VtnSetIcmpTypeAction vaction) {
        if (vaction != null) {
            Short t = vaction.getType();
            if (t != null) {
                return t.shortValue();
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
        VtnSetIcmpTypeActionCase ac =
            cast(VtnSetIcmpTypeActionCase.class, vact);
        short icmpType = getIcmpType(ac.getVtnSetIcmpTypeAction());
        return new SetIcmpTypeAction(icmpType);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public VtnSetIcmpTypeActionCase toVtnAction(Action act)
        throws RpcException {
        SetTpSrcActionCase ac = cast(SetTpSrcActionCase.class, act);
        SetTpSrcAction action = ac.getSetTpSrcAction();
        if (action != null) {
            Integer t = ProtocolUtils.getPortNumber(action.getPort());
            if (t != null) {
                return newVtnAction(NumberUtils.toShort(t));
            }
        }

        String msg = getErrorMessage("No ICMP type", ac);
        throw RpcException.getMissingArgumentException(msg);
    }

    /**
     * This method is not supported.
     *
     * @param act  An {@link Action} instance.
     * @return  Never returns.
     * @throws IllegalStateException  Always thrown.
     */
    @Override
    public String getDescription(Action act) {
        throw MiscUtils.unexpected();
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected VtnFlowActionBuilder set(VtnFlowActionBuilder builder) {
        return builder.setVtnAction(newVtnAction(type));
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected ActionBuilder set(ActionBuilder builder) {
        SetTpSrcAction tp = new SetTpSrcActionBuilder().
            setPort(new PortNumber(Integer.valueOf((int)type))).build();
        return builder.setAction(new SetTpSrcActionCaseBuilder().
                                 setSetTpSrcAction(tp).build());
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void appendContents(StringBuilder builder) {
        builder.append("type=").append(Short.toString(type));
        super.appendContents(builder);
    }

    // FlowFilterAction

    /**
     * {@inheritDoc}
     */
    @Override
    public boolean apply(FlowActionContext ctx) {
        Layer4Header l4 = ctx.getLayer4Header();
        boolean result;
        if (l4 instanceof IcmpHeader) {
            IcmpHeader icmp = (IcmpHeader)l4;
            icmp.setIcmpType(type);
            ctx.addFilterAction(this);
            result = true;
        } else {
            result = false;
        }

        return result;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public FlowAction toFlowAction() {
        return new SetIcmpTypeAction(type);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public VTNSetIcmpTypeAction toFlowFilterAction(VtnAction vact, Integer ord)
        throws RpcException {
        VtnSetIcmpTypeActionCase ac =
            cast(VtnSetIcmpTypeActionCase.class, vact);
        return new VTNSetIcmpTypeAction(ac, ord);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public VTNSetIcmpTypeAction toFlowFilterAction(FlowAction fact, int ord)
        throws RpcException {
        SetIcmpTypeAction act = cast(SetIcmpTypeAction.class, fact);
        return new VTNSetIcmpTypeAction(act, ord);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void verifyImpl() throws RpcException {
        if (!ProtocolUtils.isIcmpValueValid(type)) {
            String msg = getErrorMessage("Invalid ICMP type", type);
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

        VTNSetIcmpTypeAction va = (VTNSetIcmpTypeAction)o;
        return (type == va.type);
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        return super.hashCode() * HASH_PRIME + (int)type;
    }
}
