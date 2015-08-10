/*
 * Copyright (c) 2014, 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.cluster;


import org.opendaylight.vtn.manager.VTNException;
import org.opendaylight.vtn.manager.flow.action.SetDscpAction;

import org.opendaylight.vtn.manager.internal.PacketContext;
import org.opendaylight.vtn.manager.internal.packet.cache.Inet4Packet;
import org.opendaylight.vtn.manager.internal.util.ProtocolUtils;
import org.opendaylight.vtn.manager.internal.util.flow.action.VTNSetInetDscpAction;

import org.opendaylight.controller.sal.utils.StatusCode;

/**
 * Implementation of flow action that modifies DSCP field value in IP header.
 *
 * <p>
 *   Although this class is public to other packages, this class does not
 *   provide any API. Applications other than VTN Manager must not use this
 *   class.
 * </p>
 */
public final class SetDscpActionImpl extends FlowActionImpl {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = -1924798722051088828L;

    /**
     * DSCP field value to be set.
     */
    private final byte  dscp;

    /**
     * Construct a new instance.
     *
     * @param act  A {@link SetDscpAction} instance.
     * @throws VTNException
     *    {@code act} contains invalid value.
     */
    public SetDscpActionImpl(SetDscpAction act) throws VTNException {
        super(act);

        dscp = act.getDscp();
        if (!ProtocolUtils.isDscpValid(dscp)) {
            String msg = getErrorMessage(act, "Invalid DSCP value: ", dscp);
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

        SetDscpActionImpl act = (SetDscpActionImpl)o;
        return (dscp == act.dscp);
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        return super.hashCode() + ((int)dscp * 17);
    }

    /**
     * Return a string representation of this object.
     *
     * @return  A string representation of this object.
     */
    @Override
    public String toString() {
        StringBuilder builder = new StringBuilder("SetDscpActionImpl[");
        builder.append("dscp=").append((int)dscp).append(']');

        return builder.toString();
    }

    // FlowActionImpl

    /**
     * Return a SAL action which represents this instance.
     *
     * @return  A SAL action.
     */
    @Override
    public SetDscpAction getFlowAction() {
        return new SetDscpAction(dscp);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public boolean apply(PacketContext pctx) {
        Inet4Packet ipv4 = pctx.getInet4Packet();
        if (ipv4 != null) {
            ipv4.setDscp(dscp);
            pctx.addFilterAction(new VTNSetInetDscpAction((short)dscp));
            return true;
        }

        return false;
    }
}
