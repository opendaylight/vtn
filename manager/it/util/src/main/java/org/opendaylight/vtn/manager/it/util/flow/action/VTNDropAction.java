/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.util.flow.action;

import static org.junit.Assert.assertNotNull;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.VtnDropActionCase;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.VtnDropActionCaseBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.vtn.drop.action._case.VtnDropActionBuilder;

/**
 * {@code VTNDropAction} describes the configuration of DROP flow action.
 */
public final class VTNDropAction extends FlowAction<VtnDropActionCase> {
    /**
     * Construct an empty instance.
     */
    public VTNDropAction() {
    }

    /**
     * Construct a new instance.
     *
     * @param ord  The order of the flow action.
     */
    public VTNDropAction(Integer ord) {
        super(ord);
    }

    // FlowAction

    /**
     * Return a class that specifies the type of vtn-action.
     *
     * @return  A class of {@link VtnDropActionCase}.
     */
    @Override
    public Class<VtnDropActionCase> getActionType() {
        return VtnDropActionCase.class;
    }

    /**
     * Create a new vtn-action value.
     *
     * @return  A {@link VtnDropActionCase} instance.
     */
    @Override
    public VtnDropActionCase newVtnAction() {
        return new VtnDropActionCaseBuilder().
            setVtnDropAction(new VtnDropActionBuilder().build()).
            build();
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void verify(VtnDropActionCase vact) {
        assertNotNull(vact.getVtnDropAction());
    }
}
