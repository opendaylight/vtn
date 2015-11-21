/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
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
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.VtnSetPortDstActionCase;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.VtnSetPortDstActionCaseBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.vtn.set.port.dst.action._case.VtnSetPortDstAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.vtn.set.port.dst.action._case.VtnSetPortDstActionBuilder;
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
     * Create a new {@link VtnSetPortDstActionCase} instance.
     *
     * @param port  A {@link PortNumber} instance which specifies the port
     *              number.
     * @return  A {@link VtnSetPortDstActionCase} instance.
     */
    public static VtnSetPortDstActionCase newVtnAction(PortNumber port) {
        VtnSetPortDstAction vaction = new VtnSetPortDstActionBuilder().
            setPort(port).build();
        return new VtnSetPortDstActionCaseBuilder().
            setVtnSetPortDstAction(vaction).build();
    }

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
     * Construct a new instance with specifying action order.
     *
     * @param p    The port numbet to be set.
     * @param ord  An integer which determines the order of flow actions
     *             in a flow entry.
     */
    public VTNSetPortDstAction(int p, Integer ord) {
        super(p, ord);
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
     * @param ac   A {@link VtnSetPortDstActionCase} instance.
     * @param ord  An integer which determines the order of flow actions
     *             in a flow entry.
     * @throws RpcException  An invalid argument is specified.
     */
    public VTNSetPortDstAction(VtnSetPortDstActionCase ac, Integer ord)
        throws RpcException {
        super(ac.getVtnSetPortDstAction(), ord);
    }

    // VTNFlowAction

    /**
     * {@inheritDoc}
     */
    @Override
    public FlowAction toFlowAction(VtnAction vact) throws RpcException {
        VtnSetPortDstActionCase ac = cast(VtnSetPortDstActionCase.class, vact);
        int arg = getPortNumber(ac.getVtnSetPortDstAction());
        return new org.opendaylight.vtn.manager.flow.action.
            SetTpDstAction(arg);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public VtnSetPortDstActionCase toVtnAction(Action act) throws RpcException {
        SetTpDstActionCase ac = cast(SetTpDstActionCase.class, act);
        SetTpDstAction action = ac.getSetTpDstAction();
        if (action != null) {
            PortNumber port = action.getPort();
            if (port != null) {
                return newVtnAction(port);
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
        return builder.setVtnAction(newVtnAction(getPortNumber()));
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

    /**
     * {@inheritDoc}
     */
    @Override
    public FlowAction toFlowAction() {
        return new org.opendaylight.vtn.manager.flow.action.
            SetTpDstAction(getPort());
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public VTNSetPortDstAction toFlowFilterAction(VtnAction vact, Integer ord)
        throws RpcException {
        VtnSetPortDstActionCase ac = cast(VtnSetPortDstActionCase.class, vact);
        return new VTNSetPortDstAction(ac, ord);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public VTNSetPortDstAction toFlowFilterAction(FlowAction fact, int ord)
        throws RpcException {
        org.opendaylight.vtn.manager.flow.action.SetTpDstAction act =
            cast(org.opendaylight.vtn.manager.flow.action.SetTpDstAction.class,
                 fact);
        return new VTNSetPortDstAction(act, ord);
    }
}
