/*
 * Copyright (c) 2014, 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.cluster;

import org.opendaylight.vtn.manager.VTNException;
import org.opendaylight.vtn.manager.flow.action.SetDlDstAction;
import org.opendaylight.vtn.manager.util.EtherAddress;

import org.opendaylight.vtn.manager.internal.PacketContext;
import org.opendaylight.vtn.manager.internal.packet.cache.EtherPacket;
import org.opendaylight.vtn.manager.internal.util.flow.action.VTNSetDlDstAction;

/**
 * Implementation of flow action that modifies destination MAC address in
 * Ethernet frame.
 *
 * <p>
 *   Although this class is public to other packages, this class does not
 *   provide any API. Applications other than VTN Manager must not use this
 *   class.
 * </p>
 */
public final class SetDlDstActionImpl extends DlAddrActionImpl {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = -4389217399774516871L;

    /**
     * Construct a new instance.
     *
     * @param act  A {@link SetDlDstAction} instance.
     * @throws VTNException
     *    {@code act} contains invalid value.
     */
    public SetDlDstActionImpl(SetDlDstAction act) throws VTNException {
        super(act);
    }

    // FlowActionImpl

    /**
     * {@inheritDoc}
     */
    @Override
    public SetDlDstAction getFlowAction() {
        return new SetDlDstAction(getAddress());
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public boolean apply(PacketContext pctx) {
        EtherPacket ether = pctx.getEtherPacket();
        EtherAddress addr = getAddress();
        ether.setDestinationAddress(addr);
        pctx.addFilterAction(new VTNSetDlDstAction(addr));
        return true;
    }
}
