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
import javax.xml.bind.annotation.XmlRootElement;

import org.opendaylight.vtn.manager.flow.action.FlowAction;

import org.opendaylight.vtn.manager.internal.util.packet.Layer4Header;
import org.opendaylight.vtn.manager.internal.util.packet.Layer4PortHeader;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.VtnAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.VtnSetPortDstAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.VtnSetPortDstActionBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.flow.action.list.VtnFlowActionBuilder;

import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.Action;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.SetTpDstActionCase;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.SetTpDstActionCaseBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.set.tp.dst.action._case.SetTpDstAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.set.tp.dst.action._case.SetTpDstActionBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.list.ActionBuilder;

import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.inet.types.rev100924.PortNumber;

/**
 * {@code VTNSetPortDstAction} describes the flow action that sets the
 * destination port number for IP transport layer protocol into packet.
 */
@XmlRootElement(name = "vtn-set-port-dst-action")
@XmlAccessorType(XmlAccessType.NONE)
public final class VTNSetPortDstAction extends VTNPortAction {
    /**
     * Construct an empty instance.
     */
    VTNSetPortDstAction() {
    }

    /**
     * Construct a new instance without specifying action order.
     *
     * @param p  The port numbet to be set.
     */
    public VTNSetPortDstAction(int p) {
        super(p);
    }

    /**
     * Construct a new instance.
     *
     * @param act
     *    A {@link org.opendaylight.vtn.manager.flow.action.SetTpDstAction}
     *    instance.
     * @param ord  An integer which determines the order of flow actions
     *             in a flow entry.
     * @throws RpcException  An invalid argument is specified.
     */
    public VTNSetPortDstAction(
        org.opendaylight.vtn.manager.flow.action.SetTpDstAction act, int ord)
        throws RpcException {
        super(act, ord);
    }

    /**
     * Construct a new instance.
     *
     * @param act  A {@link VtnSetPortDstAction} instance.
     * @param ord  An integer which determines the order of flow actions
     *             in a flow entry.
     * @throws RpcException  An invalid argument is specified.
     */
    public VTNSetPortDstAction(VtnSetPortDstAction act, Integer ord)
        throws RpcException {
        super(act, ord);
    }

    // VTNFlowAction

    /**
     * {@inheritDoc}
     */
    @Override
    public FlowAction toFlowAction(VtnAction vact) throws RpcException {
        VtnSetPortDstAction vdst = cast(VtnSetPortDstAction.class, vact);
        int arg = getPortNumber(vdst.getPort());
        return new org.opendaylight.vtn.manager.flow.action.
            SetTpDstAction(arg);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public VtnSetPortDstAction toVtnAction(Action act) throws RpcException {
        SetTpDstActionCase ac = cast(SetTpDstActionCase.class, act);
        SetTpDstAction action = ac.getSetTpDstAction();
        if (action != null) {
            PortNumber port = action.getPort();
            if (port != null) {
                return new VtnSetPortDstActionBuilder().setPort(port).build();
            }
        }

        throw noPortNumber(ac);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public String getDescription(Action act) throws RpcException {
        SetTpDstActionCase ac = cast(SetTpDstActionCase.class, act);
        SetTpDstAction action = ac.getSetTpDstAction();
        PortNumber port = (action == null) ? null : action.getPort();
        return getDescription("SET_TP_DST", port);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected VtnFlowActionBuilder set(VtnFlowActionBuilder builder) {
        VtnSetPortDstAction vact = new VtnSetPortDstActionBuilder().
            setPort(getPortNumber()).build();
        return builder.setVtnAction(vact);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected ActionBuilder set(ActionBuilder builder) {
        SetTpDstAction tp = new SetTpDstActionBuilder().
            setPort(getPortNumber()).build();
        return builder.setAction(new SetTpDstActionCaseBuilder().
                                 setSetTpDstAction(tp).build());
    }

    // FlowFilterAction

    /**
     * {@inheritDoc}
     */
    @Override
    public boolean apply(FlowActionContext ctx) {
        Layer4Header l4 = ctx.getLayer4Header();
        boolean result;
        if (l4 instanceof Layer4PortHeader) {
            Layer4PortHeader head = (Layer4PortHeader)l4;
            head.setDestinationPort(getPort());
            ctx.addFilterAction(this);
            result = true;
        } else {
            result = false;
        }

        return result;
    }
}
