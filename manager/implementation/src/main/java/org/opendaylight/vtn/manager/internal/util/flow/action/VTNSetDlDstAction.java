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

import org.opendaylight.vtn.manager.util.EtherAddress;

import org.opendaylight.vtn.manager.internal.util.packet.EtherHeader;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.VtnAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.VtnSetDlDstActionCase;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.VtnSetDlDstActionCaseBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.vtn.set.dl.dst.action._case.VtnSetDlDstAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.vtn.set.dl.dst.action._case.VtnSetDlDstActionBuilder;
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
     * Create a new {@link VtnSetDlDstActionCase} instance.
     *
     * @param mac  A {@link MacAddress} instance which specifies the MAC
     *             address.
     * @return  A {@link VtnSetDlDstActionCase} instance.
     */
    public static VtnSetDlDstActionCase newVtnAction(MacAddress mac) {
        VtnSetDlDstAction vaction = new VtnSetDlDstActionBuilder().
            setAddress(mac).build();
        return new VtnSetDlDstActionCaseBuilder().
            setVtnSetDlDstAction(vaction).build();
    }

    /**
     * Construct an empty instance.
     */
    VTNSetDlDstAction() {
    }

    /**
     * Construct a new instance without specifying action order.
     *
     * @param addr  An {@link EtherAddress} instance which represents the
     *              MAC address to be set.
     */
    public VTNSetDlDstAction(EtherAddress addr) {
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
    public VTNSetDlDstAction(EtherAddress addr, Integer ord) {
        super(addr, ord);
    }

    /**
     * Construct a new instance.
     *
     * @param ac   A {@link VtnSetDlDstActionCase} instance.
     * @param ord  An integer which determines the order of flow actions
     *             in a flow entry.
     * @throws RpcException  An invalid argument is specified.
     */
    public VTNSetDlDstAction(VtnSetDlDstActionCase ac, Integer ord)
        throws RpcException {
        super(ac.getVtnSetDlDstAction(), ord);
    }

    // VTNFlowAction

    /**
     * {@inheritDoc}
     */
    @Override
    public VtnSetDlDstActionCase toVtnAction(Action act) throws RpcException {
        SetDlDstActionCase ac = cast(SetDlDstActionCase.class, act);
        SetDlDstAction action = ac.getSetDlDstAction();
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
        return builder.setVtnAction(newVtnAction(getMacAddress()));
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

    /**
     * {@inheritDoc}
     */
    @Override
    public VTNSetDlDstAction toFlowFilterAction(VtnAction vact, Integer ord)
        throws RpcException {
        VtnSetDlDstActionCase ac = cast(VtnSetDlDstActionCase.class, vact);
        return new VTNSetDlDstAction(ac, ord);
    }
}
