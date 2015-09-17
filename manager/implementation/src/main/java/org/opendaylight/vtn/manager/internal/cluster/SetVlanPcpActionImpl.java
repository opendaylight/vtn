/*
 * Copyright (c) 2014, 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.cluster;


import org.opendaylight.vtn.manager.VTNException;
import org.opendaylight.vtn.manager.flow.action.SetVlanPcpAction;

import org.opendaylight.vtn.manager.internal.PacketContext;
import org.opendaylight.vtn.manager.internal.packet.cache.EtherPacket;
import org.opendaylight.vtn.manager.internal.util.ProtocolUtils;
import org.opendaylight.vtn.manager.internal.util.flow.action.VTNSetVlanPcpAction;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;

/**
 * Implementation of flow action that modifies VLAN priority value in VLAN tag.
 *
 * <p>
 *   Although this class is public to other packages, this class does not
 *   provide any API. Applications other than VTN Manager must not use this
 *   class.
 * </p>
 */
public final class SetVlanPcpActionImpl extends FlowActionImpl {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = -1355670340796521474L;

    /**
     * VLAN priority to be set.
     */
    private final byte  priority;

    /**
     * Construct a new instance.
     *
     * @param act  A {@link SetVlanPcpAction} instance.
     * @throws VTNException
     *    {@code act} contains invalid value.
     */
    public SetVlanPcpActionImpl(SetVlanPcpAction act) throws VTNException {
        super(act);

        priority = act.getPriority();
        if (!ProtocolUtils.isVlanPriorityValid(priority)) {
            String msg = getErrorMessage(act, "Invalid VLAN priority: ",
                                         priority);
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

        SetVlanPcpActionImpl act = (SetVlanPcpActionImpl)o;
        return (priority == act.priority);
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        return super.hashCode() + ((int)priority * 37);
    }

    /**
     * Return a string representation of this object.
     *
     * @return  A string representation of this object.
     */
    @Override
    public String toString() {
        StringBuilder builder = new StringBuilder("SetVlanPcpActionImpl[");
        builder.append("priority=").append((int)priority).append(']');

        return builder.toString();
    }

    // FlowActionImpl

    /**
     * Return a SAL action which represents this instance.
     *
     * @return  A SAL action.
     */
    @Override
    public SetVlanPcpAction getFlowAction() {
        return new SetVlanPcpAction(priority);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public boolean apply(PacketContext pctx) {
        EtherPacket ether = pctx.getEtherPacket();
        ether.setVlanPriority(priority);
        pctx.addFilterAction(new VTNSetVlanPcpAction((short)priority));
        return true;
    }
}
