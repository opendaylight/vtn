/*
 * Copyright (c) 2014, 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.cluster;

import org.opendaylight.vtn.manager.VTNException;
import org.opendaylight.vtn.manager.flow.action.SetInet4DstAction;
import org.opendaylight.vtn.manager.util.IpNetwork;

import org.opendaylight.vtn.manager.internal.PacketContext;
import org.opendaylight.vtn.manager.internal.packet.cache.Inet4Packet;
import org.opendaylight.vtn.manager.internal.util.flow.action.VTNSetInetDstAction;

/**
 * Implementation of flow action that modifies destination IP address in IPv4
 * header.
 *
 * <p>
 *   Although this class is public to other packages, this class does not
 *   provide any API. Applications other than VTN Manager must not use this
 *   class.
 * </p>
 */
public final class SetInet4DstActionImpl extends InetAddressActionImpl {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = -846214438904204416L;

    /**
     * Construct a new instance.
     *
     * @param act  A {@link SetInet4DstAction} instance.
     * @throws VTNException
     *    {@code act} contains invalid value.
     */
    public SetInet4DstActionImpl(SetInet4DstAction act) throws VTNException {
        super(act);
    }

    // FlowActionImpl

    /**
     * {@inheritDoc}
     */
    @Override
    public SetInet4DstAction getFlowAction() {
        return new SetInet4DstAction(getAddress());
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public boolean apply(PacketContext pctx) {
        Inet4Packet ipv4 = pctx.getInet4Packet();
        if (ipv4 != null) {
            IpNetwork ipn = getAddress();
            ipv4.setDestinationAddress(ipn);
            pctx.addFilterAction(new VTNSetInetDstAction(ipn));
            return true;
        }

        return false;
    }
}
