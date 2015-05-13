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
import org.opendaylight.vtn.manager.util.EtherAddress;

import org.opendaylight.vtn.manager.internal.util.MiscUtils;
import org.opendaylight.vtn.manager.internal.util.packet.EtherHeader;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.VtnAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.VtnSetDlSrcAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.VtnSetDlSrcActionBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.flow.action.list.VtnFlowActionBuilder;

import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.Action;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.SetDlSrcActionCase;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.SetDlSrcActionCaseBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.set.dl.src.action._case.SetDlSrcAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.set.dl.src.action._case.SetDlSrcActionBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.list.ActionBuilder;

import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.yang.types.rev100924.MacAddress;

/**
 * {@code VTNSetDlSrcAction} describes the flow action that sets the source
 * MAC address into Ethernet header.
 */
@XmlRootElement(name = "vtn-set-dl-src-action")
@XmlAccessorType(XmlAccessType.NONE)
public final class VTNSetDlSrcAction extends VTNDlAddrAction {
    /**
     * Construct an empty instance.
     */
    VTNSetDlSrcAction() {
    }

    /**
     * Construct a new instance.
     *
     * @param addr  An {@link EtherAddress} instance which represents the
     *              MAC address to be set.
     */
    public VTNSetDlSrcAction(EtherAddress addr) {
        super(addr);
    }

    /**
     * Construct a new instance.
     *
     * @param act
     *    A {@link org.opendaylight.vtn.manager.flow.action.SetDlSrcAction}
     *    instance.
     * @param ord  An integer which determines the order of flow actions
     *             in a flow entry.
     * @throws RpcException  An invalid argument is specified.
     */
    public VTNSetDlSrcAction(
        org.opendaylight.vtn.manager.flow.action.SetDlSrcAction act, int ord)
        throws RpcException {
        super(act, ord);
    }

    /**
     * Construct a new instance.
     *
     * @param act  A {@link VtnSetDlSrcAction} instance.
     * @param ord  An integer which determines the order of flow actions
     *             in a flow entry.
     * @throws RpcException  An invalid argument is specified.
     */
    public VTNSetDlSrcAction(VtnSetDlSrcAction act, Integer ord)
        throws RpcException {
        super(act, ord);
    }

    // VTNFlowAction

    /**
     * {@inheritDoc}
     */
    @Override
    public FlowAction toFlowAction(VtnAction vact) throws RpcException {
        VtnSetDlSrcAction vsrc = cast(VtnSetDlSrcAction.class, vact);
        EtherAddress addr = MiscUtils.toEtherAddress(vsrc.getAddress());
        return new org.opendaylight.vtn.manager.flow.action.
            SetDlSrcAction(addr);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public VtnSetDlSrcAction toVtnAction(Action act) throws RpcException {
        SetDlSrcActionCase ac = cast(SetDlSrcActionCase.class, act);
        SetDlSrcAction action = ac.getSetDlSrcAction();
        if (action != null) {
            MacAddress mac = action.getAddress();
            if (mac != null) {
                return new VtnSetDlSrcActionBuilder().setAddress(mac).build();
            }
        }

        throw noMacAddress(ac);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public String getDescription(Action act) throws RpcException {
        SetDlSrcActionCase ac = cast(SetDlSrcActionCase.class, act);
        SetDlSrcAction action = ac.getSetDlSrcAction();
        MacAddress mac = (action == null) ? null : action.getAddress();
        return getDescription("SET_DL_SRC", mac);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected VtnFlowActionBuilder set(VtnFlowActionBuilder builder) {
        VtnSetDlSrcAction vact = new VtnSetDlSrcActionBuilder().
            setAddress(getMacAddress()).build();
        return builder.setVtnAction(vact);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected ActionBuilder set(ActionBuilder builder) {
        SetDlSrcAction dl = new SetDlSrcActionBuilder().
            setAddress(getMacAddress()).build();
        return builder.setAction(new SetDlSrcActionCaseBuilder().
                                 setSetDlSrcAction(dl).build());
    }

    // FlowFilterAction

    /**
     * {@inheritDoc}
     */
    @Override
    public boolean apply(FlowActionContext ctx) {
        EtherHeader ether = ctx.getEtherHeader();
        ether.setSourceAddress(getAddress());
        ctx.addFilterAction(this);
        return true;
    }
}
