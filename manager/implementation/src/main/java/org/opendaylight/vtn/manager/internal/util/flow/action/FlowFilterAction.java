/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.flow.action;

import java.util.Objects;

import javax.xml.bind.annotation.XmlAccessType;
import javax.xml.bind.annotation.XmlAccessorType;
import javax.xml.bind.annotation.XmlElement;
import javax.xml.bind.annotation.XmlRootElement;
import javax.xml.bind.annotation.XmlSeeAlso;

import org.opendaylight.vtn.manager.util.VTNIdentifiable;

import org.opendaylight.vtn.manager.internal.util.MiscUtils;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;

/**
 * {@code FlowFilterAction} descrbes the flow action which can be configured
 * in a flow filter.
 */
@XmlRootElement(name = "flow-filter-action")
@XmlAccessorType(XmlAccessType.NONE)
@XmlSeeAlso({
    VTNSetDlDstAction.class,
    VTNSetDlSrcAction.class,
    VTNSetIcmpCodeAction.class,
    VTNSetIcmpTypeAction.class,
    VTNSetInetDscpAction.class,
    VTNSetInetDstAction.class,
    VTNSetInetSrcAction.class,
    VTNSetPortDstAction.class,
    VTNSetPortSrcAction.class,
    VTNSetVlanPcpAction.class})
public abstract class FlowFilterAction extends VTNFlowAction
    implements VTNIdentifiable<Integer> {
    /**
     * An integer which determines the order of flow actions in a flow filter.
     */
    @XmlElement(required = true)
    private Integer  order;

    /**
     * Construct an empty instance.
     */
    FlowFilterAction() {
    }

    /**
     * Construct a new instance.
     *
     * @param ord  An integer which determines the order of flow actions
     *             in a flow filter.
     */
    protected FlowFilterAction(Integer ord) {
        order = ord;
    }

    /**
     * Verify the contents of this instance.
     *
     * @throws RpcException  Verifycation failed.
     */
    public final void verify() throws RpcException {
        if (getIdentifier() == null) {
            String msg = getErrorMessage("Action order");
            throw MiscUtils.getNullArgumentException(msg);
        }

        verifyImpl();
    }

    /**
     * Apply this flow action to the given packet.
     *
     * @param ctx  A {@link FlowActionContext} instance.
     * @return  {@code true} if this flow action has been actually applied.
     *          {@code false} if this flow action has been ignored.
     */
    public abstract boolean apply(FlowActionContext ctx);

    /**
     * Verify the contents of this instance.
     *
     * @throws RpcException  Verifycation failed.
     */
    protected abstract void verifyImpl() throws RpcException;

    // VTNFlowAction

    /**
     * {@inheritDoc}
     */
    @Override
    protected void appendContents(StringBuilder builder) {
        if (order != null) {
            builder.append(",order=").append(order);
        }
    }

    // VTNIdentifiable

    /**
     * Return the index value assigned to this instance.
     *
     * @return  An {@link Integer} instance which represents the index value
     *          assigned to this instance. {@code null} if not assigned.
     */
    @Override
    public Integer getIdentifier() {
        return order;
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
        if (!super.equals(o)) {
            return false;
        }

        FlowFilterAction ffa = (FlowFilterAction)o;
        return Objects.equals(order, ffa.order);
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        int h = super.hashCode();
        if (order != null) {
            h = h * MiscUtils.HASH_PRIME + order.intValue();
        }

        return h;
    }
}
