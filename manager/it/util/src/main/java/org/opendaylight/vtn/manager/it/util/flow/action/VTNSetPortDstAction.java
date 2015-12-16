/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.util.flow.action;

import static org.junit.Assert.assertNotNull;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.VtnSetPortDstActionCase;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.VtnSetPortDstActionCaseBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.vtn.set.port.dst.action._case.VtnSetPortDstAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.vtn.set.port.dst.action._case.VtnSetPortDstActionBuilder;

/**
 * {@code VTNSetPortDstAction} describes the configuration of flow action that
 * sets the destination port number for IP transport layer protocol into
 * packet.
 */
public final class VTNSetPortDstAction
    extends VTNPortAction<VtnSetPortDstActionCase> {
    /**
     * Construct an empty instance.
     */
    public VTNSetPortDstAction() {
    }

    /**
     * Construct a new instance.
     *
     * @param ord   The order of the flow action.
     * @param pnum  The port number to set.
     */
    public VTNSetPortDstAction(Integer ord, Integer pnum) {
        super(ord, pnum);
    }

    // FlowAction

    /**
     * Return a class that specifies the type of vtn-action.
     *
     * @return  A class of {@link VtnSetPortDstActionCase}.
     */
    @Override
    public Class<VtnSetPortDstActionCase> getActionType() {
        return VtnSetPortDstActionCase.class;
    }

    /**
     * Create a new vtn-action value.
     *
     * @return  A {@link VtnSetPortDstActionCase} instance.
     */
    @Override
    public VtnSetPortDstActionCase newVtnAction() {
        VtnSetPortDstAction vaction = new VtnSetPortDstActionBuilder().
            setPort(getPortNumber()).build();
        return new VtnSetPortDstActionCaseBuilder().
            setVtnSetPortDstAction(vaction).build();
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void verify(VtnSetPortDstActionCase vact) {
        VtnSetPortDstAction vaction = vact.getVtnSetPortDstAction();
        assertNotNull(vaction);
        verifyPort(vaction.getPort());
    }
}
