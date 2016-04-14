/*
 * Copyright (c) 2015, 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.flow.action;

import javax.xml.bind.annotation.XmlAccessType;
import javax.xml.bind.annotation.XmlAccessorType;
import javax.xml.bind.annotation.XmlRootElement;

import org.opendaylight.vtn.manager.util.EtherAddress;

import org.opendaylight.vtn.manager.internal.util.packet.EtherHeader;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.VtnAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.VtnSetDlSrcActionCase;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.VtnSetDlSrcActionCaseBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.vtn.set.dl.src.action._case.VtnSetDlSrcAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.vtn.set.dl.src.action._case.VtnSetDlSrcActionBuilder;
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
     * Create a new {@link VtnSetDlSrcActionCase} instance.
     *
     * @param mac  A {@link MacAddress} instance which specifies the MAC
     *             address.
     * @return  A {@link VtnSetDlSrcActionCase} instance.
     */
    public static VtnSetDlSrcActionCase newVtnAction(MacAddress mac) {
        VtnSetDlSrcAction vaction = new VtnSetDlSrcActionBuilder().
            setAddress(mac).build();
        return new VtnSetDlSrcActionCaseBuilder().
            setVtnSetDlSrcAction(vaction).build();
    }

    /**
     * Construct an empty instance.
     */
    VTNSetDlSrcAction() {
    }

    /**
     * Construct a new instance without specifying action order.
     *
     * @param addr  An {@link EtherAddress} instance which represents the
     *              MAC address to be set.
     */
    public VTNSetDlSrcAction(EtherAddress addr) {
        super(addr);
    }

    /**
     * Construct a new instance with specifying action order.
     *
     * @param addr  An {@link EtherAddress} instance which represents the
     *              MAC address to be set.
     * @param ord   An integer which determines the order of flow actions
     *              in a flow entry.
     */
    public VTNSetDlSrcAction(EtherAddress addr, Integer ord) {
        super(addr, ord);
    }

    /**
     * Construct a new instance.
     *
     * @param ac   A {@link VtnSetDlSrcActionCase} instance.
     * @param ord  An integer which determines the order of flow actions
     *             in a flow entry.
     * @throws RpcException  An invalid argument is specified.
     */
    public VTNSetDlSrcAction(VtnSetDlSrcActionCase ac, Integer ord)
        throws RpcException {
        super(ac.getVtnSetDlSrcAction(), ord);
    }

    // VTNFlowAction

    /**
     * {@inheritDoc}
     */
    @Override
    public VtnSetDlSrcActionCase toVtnAction(Action act) throws RpcException {
        SetDlSrcActionCase ac = cast(SetDlSrcActionCase.class, act);
        SetDlSrcAction action = ac.getSetDlSrcAction();
        if (action != null) {
            MacAddress mac = action.getAddress();
            if (mac != null) {
                return newVtnAction(mac);
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
        return builder.setVtnAction(newVtnAction(getMacAddress()));
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

    /**
     * {@inheritDoc}
     */
    @Override
    public String getDescription(VtnAction vact) throws RpcException {
        VtnSetDlSrcActionCase ac = cast(VtnSetDlSrcActionCase.class, vact);
        return getDescription("set-dl-src", ac.getVtnSetDlSrcAction());
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public VTNSetDlSrcAction toFlowFilterAction(VtnAction vact, Integer ord)
        throws RpcException {
        VtnSetDlSrcActionCase ac = cast(VtnSetDlSrcActionCase.class, vact);
        return new VTNSetDlSrcAction(ac, ord);
    }
}
