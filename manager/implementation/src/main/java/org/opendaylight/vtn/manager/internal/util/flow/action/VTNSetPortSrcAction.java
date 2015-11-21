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
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.VtnSetPortSrcActionCase;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.VtnSetPortSrcActionCaseBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.vtn.set.port.src.action._case.VtnSetPortSrcAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.vtn.set.port.src.action._case.VtnSetPortSrcActionBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.flow.action.list.VtnFlowActionBuilder;

import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.Action;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.SetTpSrcActionCase;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.SetTpSrcActionCaseBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.set.tp.src.action._case.SetTpSrcAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.set.tp.src.action._case.SetTpSrcActionBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.list.ActionBuilder;

import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.inet.types.rev100924.PortNumber;

/**
 * {@code VTNSetPortSrcAction} describes the flow action that sets the source
 * port number for IP transport layer protocol into packet.
 */
@XmlRootElement(name = "vtn-set-port-src-action")
@XmlAccessorType(XmlAccessType.NONE)
public final class VTNSetPortSrcAction extends VTNPortAction {
    /**
     * Create a new {@link VtnSetPortSrcActionCase} instance.
     *
     * @param port  A {@link PortNumber} instance which specifies the port
     *              number.
     * @return  A {@link VtnSetPortSrcActionCase} instance.
     */
    public static VtnSetPortSrcActionCase newVtnAction(PortNumber port) {
        VtnSetPortSrcAction vaction = new VtnSetPortSrcActionBuilder().
            setPort(port).build();
        return new VtnSetPortSrcActionCaseBuilder().
            setVtnSetPortSrcAction(vaction).build();
    }

    /**
     * Construct an empty instance.
     */
    VTNSetPortSrcAction() {
    }

    /**
     * Construct a new instance without specifying action order.
     *
     * @param p  The port numbet to be set.
     */
    public VTNSetPortSrcAction(int p) {
        super(p);
    }

    /**
     * Construct a new instance with specifying action order.
     *
     * @param p  The port numbet to be set.
     * @param ord  An integer which determines the order of flow actions
     *             in a flow entry.
     */
    public VTNSetPortSrcAction(int p, Integer ord) {
        super(p, ord);
    }

    /**
     * Construct a new instance.
     *
     * @param act
     *    A {@link org.opendaylight.vtn.manager.flow.action.SetTpSrcAction}
     *    instance.
     * @param ord  An integer which determines the order of flow actions
     *             in a flow entry.
     * @throws RpcException  An invalid argument is specified.
     */
    public VTNSetPortSrcAction(
        org.opendaylight.vtn.manager.flow.action.SetTpSrcAction act, int ord)
        throws RpcException {
        super(act, ord);
    }

    /**
     * Construct a new instance.
     *
     * @param ac   A {@link VtnSetPortSrcActionCase} instance.
     * @param ord  An integer which determines the order of flow actions
     *             in a flow entry.
     * @throws RpcException  An invalid argument is specified.
     */
    public VTNSetPortSrcAction(VtnSetPortSrcActionCase ac, Integer ord)
        throws RpcException {
        super(ac.getVtnSetPortSrcAction(), ord);
    }

    // VTNFlowAction

    /**
     * {@inheritDoc}
     */
    @Override
    public FlowAction toFlowAction(VtnAction vact) throws RpcException {
        VtnSetPortSrcActionCase ac = cast(VtnSetPortSrcActionCase.class, vact);
        int arg = getPortNumber(ac.getVtnSetPortSrcAction());
        return new org.opendaylight.vtn.manager.flow.action.
            SetTpSrcAction(arg);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public VtnSetPortSrcActionCase toVtnAction(Action act) throws RpcException {
        SetTpSrcActionCase ac = cast(SetTpSrcActionCase.class, act);
        SetTpSrcAction action = ac.getSetTpSrcAction();
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
        SetTpSrcActionCase ac = cast(SetTpSrcActionCase.class, act);
        SetTpSrcAction action = ac.getSetTpSrcAction();
        PortNumber port = (action == null) ? null : action.getPort();
        return getDescription("SET_TP_SRC", port);
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
        SetTpSrcAction tp = new SetTpSrcActionBuilder().
            setPort(getPortNumber()).build();
        return builder.setAction(new SetTpSrcActionCaseBuilder().
                                 setSetTpSrcAction(tp).build());
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
            head.setSourcePort(getPort());
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
            SetTpSrcAction(getPort());
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public VTNSetPortSrcAction toFlowFilterAction(VtnAction vact, Integer ord)
        throws RpcException {
        VtnSetPortSrcActionCase ac = cast(VtnSetPortSrcActionCase.class, vact);
        return new VTNSetPortSrcAction(ac, ord);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public VTNSetPortSrcAction toFlowFilterAction(FlowAction fact, int ord)
        throws RpcException {
        org.opendaylight.vtn.manager.flow.action.SetTpSrcAction act =
            cast(org.opendaylight.vtn.manager.flow.action.SetTpSrcAction.class,
                 fact);
        return new VTNSetPortSrcAction(act, ord);
    }
}
