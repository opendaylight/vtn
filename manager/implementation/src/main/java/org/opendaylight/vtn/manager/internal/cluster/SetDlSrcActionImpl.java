/*
 * Copyright (c) 2014, 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.cluster;

import org.opendaylight.vtn.manager.VTNException;
import org.opendaylight.vtn.manager.flow.action.SetDlSrcAction;
import org.opendaylight.vtn.manager.util.EtherAddress;

import org.opendaylight.vtn.manager.internal.PacketContext;
import org.opendaylight.vtn.manager.internal.packet.cache.EtherPacket;
import org.opendaylight.vtn.manager.internal.util.flow.action.VTNSetDlSrcAction;

/**
 * Implementation of flow action that modifies source MAC address in Ethernet
 * frame.
 *
 * <p>
 *   Although this class is public to other packages, this class does not
 *   provide any API. Applications other than VTN Manager must not use this
 *   class.
 * </p>
 */
public final class SetDlSrcActionImpl extends DlAddrActionImpl {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = -1613024024719683362L;

    /**
     * Construct a new instance.
     *
     * @param act  A {@link SetDlSrcAction} instance.
     * @throws VTNException
     *    {@code act} contains invalid value.
     */
    public SetDlSrcActionImpl(SetDlSrcAction act) throws VTNException {
        super(act);
    }

    // FlowActionImpl

    /**
     * {@inheritDoc}
     */
    @Override
    public SetDlSrcAction getFlowAction() {
        return new SetDlSrcAction(getAddress());
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public boolean apply(PacketContext pctx) {
        EtherPacket ether = pctx.getEtherPacket();
        EtherAddress addr = getAddress();
        ether.setSourceAddress(addr);
        pctx.addFilterAction(new VTNSetDlSrcAction(addr));
        return true;
    }
}
