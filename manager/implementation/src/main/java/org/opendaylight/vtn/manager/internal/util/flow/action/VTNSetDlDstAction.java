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
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.VtnSetDlDstAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.VtnSetDlDstActionBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.flow.action.list.VtnFlowActionBuilder;

import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.Action;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.SetDlDstActionCase;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.SetDlDstActionCaseBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.set.dl.dst.action._case.SetDlDstAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.set.dl.dst.action._case.SetDlDstActionBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.list.ActionBuilder;

import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.yang.types.rev100924.MacAddress;

/**
 * {@code VTNSetDlDstAction} describes the flow action that sets the
 * destination MAC address into Ethernet header.
 */
@XmlRootElement(name = "vtn-set-dl-dst-action")
@XmlAccessorType(XmlAccessType.NONE)
public final class VTNSetDlDstAction extends VTNDlAddrAction {
    /**
     * Construct an empty instance.
     */
    VTNSetDlDstAction() {
    }

    /**
     * Construct a new instance.
     *
     * @param addr  An {@link EtherAddress} instance which represents the
     *              MAC address to be set.
     */
    public VTNSetDlDstAction(EtherAddress addr) {
        super(addr);
    }

    /**
     * Construct a new instance.
     *
     * @param act
     *    A {@link org.opendaylight.vtn.manager.flow.action.SetDlDstAction}
     *    instance.
     * @param ord  An integer which determines the order of flow actions
     *             in a flow entry.
     * @throws RpcException  An invalid argument is specified.
     */
    public VTNSetDlDstAction(
        org.opendaylight.vtn.manager.flow.action.SetDlDstAction act, int ord)
        throws RpcException {
        super(act, ord);
    }

    /**
     * Construct a new instance.
     *
     * @param act  A {@link VtnSetDlDstAction} instance.
     * @param ord  An integer which determines the order of flow actions
     *             in a flow entry.
     * @throws RpcException  An invalid argument is specified.
     */
    public VTNSetDlDstAction(VtnSetDlDstAction act, Integer ord)
        throws RpcException {
        super(act, ord);
    }

    // VTNFlowAction

    /**
     * {@inheritDoc}
     */
    @Override
    public FlowAction toFlowAction(VtnAction vact) throws RpcException {
        VtnSetDlDstAction vdst = cast(VtnSetDlDstAction.class, vact);
        EtherAddress addr = MiscUtils.toEtherAddress(vdst.getAddress());
        return new org.opendaylight.vtn.manager.flow.action.
            SetDlDstAction(addr);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public VtnSetDlDstAction toVtnAction(Action act) throws RpcException {
        SetDlDstActionCase ac = cast(SetDlDstActionCase.class, act);
        SetDlDstAction action = ac.getSetDlDstAction();
        if (action != null) {
            MacAddress mac = action.getAddress();
            if (mac != null) {
                return new VtnSetDlDstActionBuilder().setAddress(mac).build();
            }
        }

        throw noMacAddress(ac);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public String getDescription(Action act) throws RpcException {
        SetDlDstActionCase ac = cast(SetDlDstActionCase.class, act);
        SetDlDstAction action = ac.getSetDlDstAction();
        MacAddress mac = (action == null) ? null : action.getAddress();
        return getDescription("SET_DL_DST", mac);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected VtnFlowActionBuilder set(VtnFlowActionBuilder builder) {
        VtnSetDlDstAction vact = new VtnSetDlDstActionBuilder().
            setAddress(getMacAddress()).build();
        return builder.setVtnAction(vact);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected ActionBuilder set(ActionBuilder builder) {
        SetDlDstAction dl = new SetDlDstActionBuilder().
            setAddress(getMacAddress()).build();
        return builder.setAction(new SetDlDstActionCaseBuilder().
                                 setSetDlDstAction(dl).build());
    }

    // FlowFilterAction

    /**
     * {@inheritDoc}
     */
    @Override
    public boolean apply(FlowActionContext ctx) {
        EtherHeader ether = ctx.getEtherHeader();
        ether.setDestinationAddress(getAddress());
        ctx.addFilterAction(this);
        return true;
    }
}
