/*
 * Copyright (c) 2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.flow.action;

import javax.xml.bind.annotation.XmlAccessType;
import javax.xml.bind.annotation.XmlAccessorType;
import javax.xml.bind.annotation.XmlElement;
import javax.xml.bind.annotation.XmlRootElement;

import org.opendaylight.vtn.manager.flow.action.FlowAction;
import org.opendaylight.vtn.manager.flow.action.SetDscpAction;

import org.opendaylight.vtn.manager.internal.util.MiscUtils;
import org.opendaylight.vtn.manager.internal.util.ProtocolUtils;
import org.opendaylight.vtn.manager.internal.util.packet.InetHeader;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.VtnAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.VtnSetInetDscpAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.VtnSetInetDscpActionBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.flow.action.list.VtnFlowActionBuilder;

import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.Action;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.SetNwTosActionCase;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.SetNwTosActionCaseBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.set.nw.tos.action._case.SetNwTosAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.set.nw.tos.action._case.SetNwTosActionBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.list.ActionBuilder;

import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.inet.types.rev100924.Dscp;

/**
 * {@code VTNSetInetDscpAction} describes the flow action that sets the DSCP
 * value into IP header.
 */
@XmlRootElement(name = "vtn-set-inet-dscp-action")
@XmlAccessorType(XmlAccessType.NONE)
public final class VTNSetInetDscpAction extends FlowFilterAction {
    /**
     * Default DSCP value.
     */
    private static final short  DEFAULT_VALUE = 0;

    /**
     * The DSCP value value to be set.
     */
    @XmlElement
    private short  dscp;

    /**
     * Construct an empty instance.
     */
    VTNSetInetDscpAction() {
    }

    /**
     * Construct a new instance without specifying action order.
     *
     * @param d  A DSCP value to be set.
     */
    public VTNSetInetDscpAction(short d) {
        dscp = d;
    }

    /**
     * Construct a new instance.
     *
     * @param act  A {@link SetDscpAction} instance.
     * @param ord  An integer which determines the order of flow actions
     *             in a flow entry.
     * @throws RpcException  An invalid argument is specified.
     */
    public VTNSetInetDscpAction(SetDscpAction act, int ord)
        throws RpcException {
        super(Integer.valueOf(ord));
        dscp = (short)act.getDscp();
        verify();
    }

    /**
     * Construct a new instance.
     *
     * @param act  A {@link VtnSetInetDscpAction} instance.
     * @param ord  An integer which determines the order of flow actions
     *             in a flow entry.
     * @throws RpcException  An invalid argument is specified.
     */
    public VTNSetInetDscpAction(VtnSetInetDscpAction act, Integer ord)
        throws RpcException {
        super(ord);
        dscp = getDscpValue(act.getDscp());
        verify();
    }

    /**
     * Return the DSCP value to be set.
     *
     * @return  The DSCP value to be set.
     */
    public short getDscp() {
        return dscp;
    }

    /**
     * Return the IP DSCP value in the given instance.
     *
     * @param d A {@link Dscp} instance.
     * @return  IP DSCP value.
     */
    private short getDscpValue(Dscp d) {
        if (d != null) {
            Short v = d.getValue();
            if (v != null) {
                return v.shortValue();
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
        VtnSetInetDscpAction vdscp = cast(VtnSetInetDscpAction.class, vact);
        short arg = getDscpValue(vdscp.getDscp());
        return new SetDscpAction((byte)arg);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public VtnSetInetDscpAction toVtnAction(Action act) throws RpcException {
        SetNwTosActionCase ac = cast(SetNwTosActionCase.class, act);
        SetNwTosAction action = ac.getSetNwTosAction();
        if (action != null) {
            Integer tos = action.getTos();
            if (tos != null) {
                Short d = Short.valueOf(ProtocolUtils.tosToDscp(tos));
                return new VtnSetInetDscpActionBuilder().
                    setDscp(new Dscp(d)).build();
            }
        }

        String msg = getErrorMessage("No DSCP value", ac);
        throw RpcException.getMissingArgumentException(msg);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public String getDescription(Action act) throws RpcException {
        SetNwTosActionCase ac = cast(SetNwTosActionCase.class, act);
        SetNwTosAction action = ac.getSetNwTosAction();
        String tos = null;
        if (action != null) {
            Integer value = action.getTos();
            if (value != null) {
                tos = String.format("0x%x", value);
            }
        }

        return new StringBuilder("SET_NW_TOS(tos=").
            append(tos).append(')').toString();
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected VtnFlowActionBuilder set(VtnFlowActionBuilder builder) {
        VtnSetInetDscpAction vact = new VtnSetInetDscpActionBuilder().
            setDscp(new Dscp(Short.valueOf(dscp))).build();
        return builder.setVtnAction(vact);
    }

    /**
     * {@inheritDoc}
     */
    protected ActionBuilder set(ActionBuilder builder) {
        Integer tos = Integer.valueOf(ProtocolUtils.dscpToTos(dscp));
        SetNwTosAction nw = new SetNwTosActionBuilder().setTos(tos).build();
        return builder.setAction(new SetNwTosActionCaseBuilder().
                                 setSetNwTosAction(nw).build());
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void appendContents(StringBuilder builder) {
        builder.append("dscp=").append(Short.toString(dscp));
        super.appendContents(builder);
    }

    // FlowFilterAction

    /**
     * {@inheritDoc}
     */
    @Override
    public boolean apply(FlowActionContext ctx) {
        InetHeader inet = ctx.getInetHeader();
        boolean result;
        if (inet != null) {
            inet.setDscp(dscp);
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
    protected void verifyImpl() throws RpcException {
        if (!ProtocolUtils.isDscpValid(dscp)) {
            String msg = getErrorMessage("Invalid IP DSCP value", dscp);
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

        VTNSetInetDscpAction va = (VTNSetInetDscpAction)o;
        return (dscp == va.dscp);
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        return super.hashCode() * MiscUtils.HASH_PRIME + (int)dscp;
    }
}
