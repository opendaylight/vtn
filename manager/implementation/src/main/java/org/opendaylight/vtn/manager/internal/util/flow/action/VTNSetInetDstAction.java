/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
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
import org.opendaylight.vtn.manager.flow.action.SetInet4DstAction;
import org.opendaylight.vtn.manager.util.IpNetwork;

import org.opendaylight.vtn.manager.internal.util.packet.InetHeader;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.VtnAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.VtnSetInetDstActionCase;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.VtnSetInetDstActionCaseBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.vtn.set.inet.dst.action._case.VtnSetInetDstAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.vtn.set.inet.dst.action._case.VtnSetInetDstActionBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.flow.action.list.VtnFlowActionBuilder;

import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.Action;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.SetNwDstActionCase;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.SetNwDstActionCaseBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.set.nw.dst.action._case.SetNwDstAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.set.nw.dst.action._case.SetNwDstActionBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.list.ActionBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.address.Address;

/**
 * {@code VTNSetInetDstAction} describes the flow action that sets the
 * destination IP address into IP header.
 */
@XmlRootElement(name = "vtn-set-inet-dst-action")
@XmlAccessorType(XmlAccessType.NONE)
public final class VTNSetInetDstAction extends VTNInetAddrAction {
    /**
     * Create a new {@link VtnSetInetDstActionCase} instance.
     *
     * @param addr  An {@link Address} instance which specifies the
     *              IP address.
     * @return  A {@link VtnSetInetDstActionCase} instance.
     */
    public static VtnSetInetDstActionCase newVtnAction(Address addr) {
        VtnSetInetDstAction vaction = new VtnSetInetDstActionBuilder().
            setAddress(addr).build();
        return new VtnSetInetDstActionCaseBuilder().
            setVtnSetInetDstAction(vaction).build();
    }

    /**
     * Construct an empty instance.
     */
    VTNSetInetDstAction() {
    }

    /**
     * Construct a new instance without specifying action order.
     *
     * @param addr  An {@link IpNetwork} instance which represents the
     *              IP address to be set.
     */
    public VTNSetInetDstAction(IpNetwork addr) {
        super(addr);
    }

    /**
     * Construct a new instance.
     *
     * @param act  A {@link SetInet4DstAction} instance.
     * @param ord  An integer which determines the order of flow actions
     *             in a flow entry.
     * @throws RpcException  An invalid argument is specified.
     */
    public VTNSetInetDstAction(SetInet4DstAction act, int ord)
        throws RpcException {
        super(act, ord);
    }

    /**
     * Construct a new instance.
     *
     * @param ac   A {@link VtnSetInetDstActionCase} instance.
     * @param ord  An integer which determines the order of flow actions
     *             in a flow entry.
     * @throws RpcException  An invalid argument is specified.
     */
    public VTNSetInetDstAction(VtnSetInetDstActionCase ac, Integer ord)
        throws RpcException {
        super(ac.getVtnSetInetDstAction(), ord);
    }

    // VTNFlowAction

    /**
     * {@inheritDoc}
     */
    @Override
    public FlowAction toFlowAction(VtnAction vact) throws RpcException {
        VtnSetInetDstActionCase ac = cast(VtnSetInetDstActionCase.class, vact);
        VtnSetInetDstAction vaction = ac.getVtnSetInetDstAction();
        if (vaction != null) {
            IpNetwork addr = IpNetwork.create(vaction.getAddress());
            if (addr != null) {
                try {
                    return new SetInet4DstAction(addr);
                } catch (RuntimeException e) {
                    throw invalidIpAddress(vact, e);
                }
            }
        }

        throw noIpAddress(ac);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public VtnSetInetDstActionCase toVtnAction(Action act) throws RpcException {
        SetNwDstActionCase ac = cast(SetNwDstActionCase.class, act);
        SetNwDstAction action = ac.getSetNwDstAction();
        if (action != null) {
            Address addr = action.getAddress();
            if (addr != null) {
                return newVtnAction(addr);
            }
        }

        throw noIpAddress(ac);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public String getDescription(Action act) throws RpcException {
        SetNwDstActionCase ac = cast(SetNwDstActionCase.class, act);
        SetNwDstAction action = ac.getSetNwDstAction();
        Address addr = (action == null) ? null : action.getAddress();
        return getDescription("SET_NW_DST", addr);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected VtnFlowActionBuilder set(VtnFlowActionBuilder builder) {
        return builder.setVtnAction(newVtnAction(getMdAddress()));
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected ActionBuilder set(ActionBuilder builder) {
        SetNwDstAction nw = new SetNwDstActionBuilder().
            setAddress(getMdAddress()).build();
        return builder.setAction(new SetNwDstActionCaseBuilder().
                                 setSetNwDstAction(nw).build());
    }

    // FlowFilterAction

    /**
     * {@inheritDoc}
     */
    @Override
    public boolean apply(FlowActionContext ctx) {
        InetHeader inet = ctx.getInetHeader();
        boolean result;
        if (inet != null && inet.setDestinationAddress(getAddress())) {
            ctx.addFilterAction(this);
            result = true;
        } else {
            result = false;
        }

        return result;
    }
}
