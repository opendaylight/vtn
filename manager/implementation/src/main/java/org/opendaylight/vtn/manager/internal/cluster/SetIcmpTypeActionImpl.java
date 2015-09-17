/*
 * Copyright (c) 2014, 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.cluster;


import org.opendaylight.vtn.manager.VTNException;
import org.opendaylight.vtn.manager.flow.action.SetIcmpTypeAction;

import org.opendaylight.vtn.manager.internal.PacketContext;
import org.opendaylight.vtn.manager.internal.packet.cache.IcmpPacket;
import org.opendaylight.vtn.manager.internal.packet.cache.L4Packet;
import org.opendaylight.vtn.manager.internal.util.ProtocolUtils;
import org.opendaylight.vtn.manager.internal.util.flow.action.VTNSetIcmpTypeAction;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;

/**
 * Implementation of flow action that modifies ICMP type field value in
 * ICMP header in IPv4 packet.
 *
 * <p>
 *   Although this class is public to other packages, this class does not
 *   provide any API. Applications other than VTN Manager must not use this
 *   class.
 * </p>
 */
public final class SetIcmpTypeActionImpl extends FlowActionImpl {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = 7985609778860682790L;

    /**
     * ICMP type value to be set.
     */
    private final short  type;

    /**
     * Construct a new instance.
     *
     * @param act  A {@link SetIcmpTypeAction} instance.
     * @throws VTNException
     *    {@code act} contains invalid value.
     */
    public SetIcmpTypeActionImpl(SetIcmpTypeAction act) throws VTNException {
        super(act);

        type = act.getType();
        if (!ProtocolUtils.isIcmpValueValid(type)) {
            String msg = getErrorMessage(act, "Invalid ICMP type: ", type);
            throw RpcException.getBadArgumentException(msg);
        }
    }

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

        SetIcmpTypeActionImpl act = (SetIcmpTypeActionImpl)o;
        return (type == act.type);
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        return super.hashCode() + ((int)type * 29);
    }

    /**
     * Return a string representation of this object.
     *
     * @return  A string representation of this object.
     */
    @Override
    public String toString() {
        StringBuilder builder = new StringBuilder("SetIcmpTypeActionImpl[");
        builder.append("type=").append((int)type).append(']');

        return builder.toString();
    }

    // FlowActionImpl

    /**
     * {@inheritDoc}
     */
    @Override
    public SetIcmpTypeAction getFlowAction() {
        return new SetIcmpTypeAction(type);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public boolean apply(PacketContext pctx) {
        L4Packet packet = pctx.getL4Packet();
        if (packet instanceof IcmpPacket) {
            IcmpPacket icmp = (IcmpPacket)packet;
            icmp.setIcmpType(type);
            pctx.addFilterAction(new VTNSetIcmpTypeAction(type));
            return true;
        }

        return false;
    }
}
