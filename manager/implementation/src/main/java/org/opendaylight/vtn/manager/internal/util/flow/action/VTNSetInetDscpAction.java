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

import org.opendaylight.vtn.manager.internal.util.ProtocolUtils;
import org.opendaylight.vtn.manager.internal.util.packet.InetHeader;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.VtnAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.VtnSetInetDscpActionCase;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.VtnSetInetDscpActionCaseBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.vtn.set.inet.dscp.action._case.VtnSetInetDscpAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.vtn.set.inet.dscp.action._case.VtnSetInetDscpActionBuilder;
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
     * Create a new {@link VtnSetInetDscpActionCase} instance.
     *
     * @param d  A {@link Short} instance which specifies the DSCP value.
     * @return   A {@link VtnSetInetDscpActionCase} instance.
     */
    public static VtnSetInetDscpActionCase newVtnAction(Short d) {
        VtnSetInetDscpAction vaction = new VtnSetInetDscpActionBuilder().
            setDscp(new Dscp(d)).build();
        return new VtnSetInetDscpActionCaseBuilder().
            setVtnSetInetDscpAction(vaction).build();
    }

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
     * Construct a new instance with specifying action order.
     *
     * @param d    A DSCP value to be set.
     * @param ord  An integer which determines the order of flow actions
     *             in a flow entry.
     */
    public VTNSetInetDscpAction(short d, Integer ord) {
        super(ord);
        dscp = d;
    }

    /**
     * Construct a new instance.
     *
     * @param ac   A {@link VtnSetInetDscpActionCase} instance.
     * @param ord  An integer which determines the order of flow actions
     *             in a flow entry.
     * @throws RpcException  An invalid argument is specified.
     */
    public VTNSetInetDscpAction(VtnSetInetDscpActionCase ac, Integer ord)
        throws RpcException {
        super(ord);
        dscp = getDscpValue(ac.getVtnSetInetDscpAction());
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
     * @param vaction  A {@link VtnSetInetDscpAction} instance.
     * @return  IP DSCP value.
     */
    private short getDscpValue(VtnSetInetDscpAction vaction) {
        if (vaction != null) {
            Dscp d = vaction.getDscp();
            if (d != null) {
                Short v = d.getValue();
                if (v != null) {
                    return v.shortValue();
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
    public VtnSetInetDscpActionCase toVtnAction(Action act)
        throws RpcException {
        SetNwTosActionCase ac = cast(SetNwTosActionCase.class, act);
        SetNwTosAction action = ac.getSetNwTosAction();
        if (action != null) {
            Integer tos = action.getTos();
            if (tos != null) {
                return newVtnAction(ProtocolUtils.tosToDscp(tos));
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
        return builder.setVtnAction(newVtnAction(dscp));
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
    public VTNSetInetDscpAction toFlowFilterAction(VtnAction vact, Integer ord)
        throws RpcException {
        VtnSetInetDscpActionCase ac =
            cast(VtnSetInetDscpActionCase.class, vact);
        return new VTNSetInetDscpAction(ac, ord);
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
        return super.hashCode() * HASH_PRIME + (int)dscp;
    }
}
