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
import org.opendaylight.vtn.manager.flow.action.SetIcmpCodeAction;
import org.opendaylight.vtn.manager.util.NumberUtils;

import org.opendaylight.vtn.manager.internal.util.MiscUtils;
import org.opendaylight.vtn.manager.internal.util.ProtocolUtils;
import org.opendaylight.vtn.manager.internal.util.packet.IcmpHeader;
import org.opendaylight.vtn.manager.internal.util.packet.Layer4Header;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.VtnAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.VtnSetIcmpCodeAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.VtnSetIcmpCodeActionBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.flow.action.list.VtnFlowActionBuilder;

import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.Action;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.SetTpDstActionCase;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.SetTpDstActionCaseBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.set.tp.dst.action._case.SetTpDstAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.set.tp.dst.action._case.SetTpDstActionBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.list.ActionBuilder;

import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.inet.types.rev100924.PortNumber;

/**
 * {@code VTNSetIcmpCodeAction} describes the flow action that sets the
 * ICMP code into ICMP header.
 */
@XmlRootElement(name = "vtn-set-icmp-code-action")
@XmlAccessorType(XmlAccessType.NONE)
public final class VTNSetIcmpCodeAction extends FlowFilterAction {
    /**
     * The ICMP code value to be set.
     */
    @XmlElement
    private short  code;

    /**
     * Construct an empty instance.
     */
    VTNSetIcmpCodeAction() {
    }

    /**
     * Construct a new instance without specifying action order.
     *
     * @param c  The ICMP code to be set.
     */
    public VTNSetIcmpCodeAction(short c) {
        code = c;
    }

    /**
     * Construct a new instance.
     *
     * @param act  A {@link SetIcmpCodeAction} instance.
     * @param ord  An integer which determines the order of flow actions
     *             in a flow entry.
     * @throws RpcException  An invalid argument is specified.
     */
    public VTNSetIcmpCodeAction(SetIcmpCodeAction act, int ord)
        throws RpcException {
        super(Integer.valueOf(ord));
        code = act.getCode();
        verify();
    }

    /**
     * Construct a new instance.
     *
     * @param act  A {@link VtnSetIcmpCodeAction} instance.
     * @param ord  An integer which determines the order of flow actions
     *             in a flow entry.
     * @throws RpcException  An invalid argument is specified.
     */
    public VTNSetIcmpCodeAction(VtnSetIcmpCodeAction act, Integer ord)
        throws RpcException {
        super(ord);
        Short icmpCode = act.getCode();
        if (icmpCode != null) {
            code = icmpCode.shortValue();
        }
        verify();
    }

    /**
     * Return the ICMP code to be set.
     *
     * @return  The ICMP code to be set.
     */
    public short getCode() {
        return code;
    }

    // VTNFlowAction

    /**
     * {@inheritDoc}
     */
    @Override
    public FlowAction toFlowAction(VtnAction vact) throws RpcException {
        VtnSetIcmpCodeAction vcode = cast(VtnSetIcmpCodeAction.class, vact);
        Short arg = vcode.getCode();
        if (arg == null) {
            String msg = getErrorMessage("No ICMP code", vact);
            throw RpcException.getMissingArgumentException(msg);
        }

        return new SetIcmpCodeAction(arg.shortValue());
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public VtnSetIcmpCodeAction toVtnAction(Action act) throws RpcException {
        SetTpDstActionCase ac = cast(SetTpDstActionCase.class, act);
        SetTpDstAction action = ac.getSetTpDstAction();
        if (action != null) {
            Integer c = ProtocolUtils.getPortNumber(action.getPort());
            if (c != null) {
                return new VtnSetIcmpCodeActionBuilder().
                    setCode(NumberUtils.toShort(c)).build();
            }
        }

        String msg = getErrorMessage("No ICMP code", ac);
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
        VtnSetIcmpCodeAction vact = new VtnSetIcmpCodeActionBuilder().
            setCode(Short.valueOf(code)).build();
        return builder.setVtnAction(vact);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected ActionBuilder set(ActionBuilder builder) {
        SetTpDstAction tp = new SetTpDstActionBuilder().
            setPort(new PortNumber(Integer.valueOf((int)code))).build();
        return builder.setAction(new SetTpDstActionCaseBuilder().
                                 setSetTpDstAction(tp).build());
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void appendContents(StringBuilder builder) {
        builder.append("code=").append(Short.toString(code));
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
            icmp.setIcmpCode(code);
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
        if (!ProtocolUtils.isIcmpValueValid(code)) {
            String msg = getErrorMessage("Invalid ICMP code", code);
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

        VTNSetIcmpCodeAction va = (VTNSetIcmpCodeAction)o;
        return (code == va.code);
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        return super.hashCode() * MiscUtils.HASH_PRIME + (int)code;
    }
}
