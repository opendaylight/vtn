/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.util.flow.action;

import static org.junit.Assert.assertNotNull;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.VtnSetPortSrcActionCase;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.VtnSetPortSrcActionCaseBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.vtn.set.port.src.action._case.VtnSetPortSrcAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.vtn.set.port.src.action._case.VtnSetPortSrcActionBuilder;

/**
 * {@code VTNSetPortSrcAction} describes the configuration of flow action that
 * sets the source port number for IP transport layer protocol into packet.
 */
public final class VTNSetPortSrcAction
    extends VTNPortAction<VtnSetPortSrcActionCase> {
    /**
     * Construct an empty instance.
     */
    public VTNSetPortSrcAction() {
    }

    /**
     * Construct a new instance.
     *
     * @param ord   The order of the flow action.
     * @param pnum  The port number to set.
     */
    public VTNSetPortSrcAction(Integer ord, Integer pnum) {
        super(ord, pnum);
    }

    // FlowAction

    /**
     * Return a class that specifies the type of vtn-action.
     *
     * @return  A class of {@link VtnSetPortSrcActionCase}.
     */
    @Override
    public Class<VtnSetPortSrcActionCase> getActionType() {
        return VtnSetPortSrcActionCase.class;
    }

    /**
     * Create a new vtn-action value.
     *
     * @return  A {@link VtnSetPortSrcActionCase} instance.
     */
    @Override
    public VtnSetPortSrcActionCase newVtnAction() {
        VtnSetPortSrcAction vaction = new VtnSetPortSrcActionBuilder().
            setPort(getPortNumber()).build();
        return new VtnSetPortSrcActionCaseBuilder().
            setVtnSetPortSrcAction(vaction).build();
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void verify(VtnSetPortSrcActionCase vact) {
        VtnSetPortSrcAction vaction = vact.getVtnSetPortSrcAction();
        assertNotNull(vaction);
        verifyPort(vaction.getPort());
    }
}
