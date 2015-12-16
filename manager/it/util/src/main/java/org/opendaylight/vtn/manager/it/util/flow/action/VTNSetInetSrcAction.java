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

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.VtnSetInetSrcActionCase;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.VtnSetInetSrcActionCaseBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.vtn.set.inet.src.action._case.VtnSetInetSrcAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.vtn.set.inet.src.action._case.VtnSetInetSrcActionBuilder;

/**
 * {@code VTNSetInetSrcAtion} describes the configuation of flow entry that
 * sets the source IP address into IP header.
 */
public final class VTNSetInetSrcAction
    extends VTNInetAddrAction<VtnSetInetSrcActionCase> {
    /**
     * Construct an empty instance.
     */
    public VTNSetInetSrcAction() {
    }

    /**
     * Construct a new instance.
     *
     * @param ord   The order of the flow action.
     * @param addr  The IP address to set.
     */
    public VTNSetInetSrcAction(Integer ord, IpNetwork addr) {
        super(ord, addr);
    }

    // FlowAction

    /**
     * Return a class that specifies the type of vtn-action.
     *
     * @return  A class of {@link VtnSetInetSrcActionCase}.
     */
    @Override
    public Class<VtnSetInetSrcActionCase> getActionType() {
        return VtnSetInetSrcActionCase.class;
    }

    /**
     * Create a new vtn-action value.
     *
     * @return  A {@link VtnSetInetSrcActionCase} instance.
     */
    @Override
    public VtnSetInetSrcActionCase newVtnAction() {
        VtnSetInetSrcAction vaction = new VtnSetInetSrcActionBuilder().
            setAddress(getMdAddress()).build();
        return new VtnSetInetSrcActionCaseBuilder().
            setVtnSetInetSrcAction(vaction).build();
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void verify(VtnSetInetSrcActionCase vact) {
        VtnSetInetSrcAction vaction = vact.getVtnSetInetSrcAction();
        assertNotNull(vaction);
        verifyAddress(vaction.getAddress());
    }
}
