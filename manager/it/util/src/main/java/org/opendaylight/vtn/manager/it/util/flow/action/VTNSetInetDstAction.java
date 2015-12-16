/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.util.flow.action;

import static org.junit.Assert.assertNotNull;

import org.opendaylight.vtn.manager.util.IpNetwork;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.VtnSetInetDstActionCase;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.VtnSetInetDstActionCaseBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.vtn.set.inet.dst.action._case.VtnSetInetDstAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.vtn.set.inet.dst.action._case.VtnSetInetDstActionBuilder;

/**
 * {@code VTNSetInetDstAtion} describes the configuation of flow entry that
 * sets the destination IP address into IP header.
 */
public final class VTNSetInetDstAction
    extends VTNInetAddrAction<VtnSetInetDstActionCase> {
    /**
     * Construct an empty instance.
     */
    public VTNSetInetDstAction() {
    }

    /**
     * Construct a new instance.
     *
     * @param ord   The order of the flow action.
     * @param addr  The IP address to set.
     */
    public VTNSetInetDstAction(Integer ord, IpNetwork addr) {
        super(ord, addr);
    }

    // FlowAction

    /**
     * Return a class that specifies the type of vtn-action.
     *
     * @return  A class of {@link VtnSetInetDstActionCase}.
     */
    @Override
    public Class<VtnSetInetDstActionCase> getActionType() {
        return VtnSetInetDstActionCase.class;
    }

    /**
     * Create a new vtn-action value.
     *
     * @return  A {@link VtnSetInetDstActionCase} instance.
     */
    @Override
    public VtnSetInetDstActionCase newVtnAction() {
        VtnSetInetDstAction vaction = new VtnSetInetDstActionBuilder().
            setAddress(getMdAddress()).build();
        return new VtnSetInetDstActionCaseBuilder().
            setVtnSetInetDstAction(vaction).build();
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void verify(VtnSetInetDstActionCase vact) {
        VtnSetInetDstAction vaction = vact.getVtnSetInetDstAction();
        assertNotNull(vaction);
        verifyAddress(vaction.getAddress());
    }
}
