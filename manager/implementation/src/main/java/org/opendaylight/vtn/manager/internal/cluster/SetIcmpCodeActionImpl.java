/*
 * Copyright (c) 2014, 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.cluster;


import org.opendaylight.vtn.manager.VTNException;
import org.opendaylight.vtn.manager.flow.action.SetIcmpCodeAction;

import org.opendaylight.vtn.manager.internal.PacketContext;
import org.opendaylight.vtn.manager.internal.packet.cache.IcmpPacket;
import org.opendaylight.vtn.manager.internal.packet.cache.L4Packet;
import org.opendaylight.vtn.manager.internal.util.ProtocolUtils;
import org.opendaylight.vtn.manager.internal.util.flow.action.VTNSetIcmpCodeAction;

import org.opendaylight.controller.sal.utils.StatusCode;

/**
 * Implementation of flow action that modifies ICMP code field value in
 * ICMP header in IPv4 packet.
 *
 * <p>
 *   Although this class is public to other packages, this class does not
 *   provide any API. Applications other than VTN Manager must not use this
 *   class.
 * </p>
 */
public final class SetIcmpCodeActionImpl extends FlowActionImpl {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = -875563808348172512L;

    /**
     * ICMP code value to be set.
     */
    private final short  code;

    /**
     * Construct a new instance.
     *
     * @param act  A {@link SetIcmpCodeAction} instance.
     * @throws VTNException
     *    {@code act} contains invalid value.
     */
    public SetIcmpCodeActionImpl(SetIcmpCodeAction act) throws VTNException {
        super(act);

        code = act.getCode();
        if (!ProtocolUtils.isIcmpValueValid(code)) {
            String msg = getErrorMessage(act, "Invalid ICMP code: ", code);
            throw new VTNException(StatusCode.BADREQUEST, msg);
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

        SetIcmpCodeActionImpl act = (SetIcmpCodeActionImpl)o;
        return (code == act.code);
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        return super.hashCode() + ((int)code * 13);
    }

    /**
     * Return a string representation of this object.
     *
     * @return  A string representation of this object.
     */
    @Override
    public String toString() {
        StringBuilder builder = new StringBuilder("SetIcmpCodeActionImpl[");
        builder.append("code=").append((int)code).append(']');

        return builder.toString();
    }

    // FlowActionImpl

    /**
     * {@inheritDoc}
     */
    @Override
    public SetIcmpCodeAction getFlowAction() {
        return new SetIcmpCodeAction(code);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public boolean apply(PacketContext pctx) {
        L4Packet packet = pctx.getL4Packet();
        if (packet instanceof IcmpPacket) {
            IcmpPacket icmp = (IcmpPacket)packet;
            icmp.setIcmpCode(code);
            pctx.addFilterAction(new VTNSetIcmpCodeAction(code));
            return true;
        }

        return false;
    }
}
