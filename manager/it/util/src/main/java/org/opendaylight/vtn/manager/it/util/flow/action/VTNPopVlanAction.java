/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.util.flow.action;

import static org.junit.Assert.assertNotNull;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.VtnPopVlanActionCase;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.VtnPopVlanActionCaseBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.vtn.pop.vlan.action._case.VtnPopVlanActionBuilder;

/**
 * {@code VTNPopVlanAction} describes the configuration of flow action that
 * strips the outermost VLAN tag in Ethernet frame.
 */
public final class VTNPopVlanAction extends FlowAction<VtnPopVlanActionCase> {
    /**
     * Construct an empty instance.
     */
    public VTNPopVlanAction() {
    }

    /**
     * Construct a new instance.
     *
     * @param ord  The order of the flow action.
     */
    public VTNPopVlanAction(Integer ord) {
        super(ord);
    }

    // FlowAction

    /**
     * Return a class that specifies the type of vtn-action.
     *
     * @return  A class of {@link VtnPopVlanActionCase}.
     */
    @Override
    public Class<VtnPopVlanActionCase> getActionType() {
        return VtnPopVlanActionCase.class;
    }

    /**
     * Create a new vtn-action value.
     *
     * @return  A {@link VtnPopVlanActionCase} instance.
     */
    @Override
    public VtnPopVlanActionCase newVtnAction() {
        return new VtnPopVlanActionCaseBuilder().
            setVtnPopVlanAction(new VtnPopVlanActionBuilder().build()).
            build();
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void verify(VtnPopVlanActionCase vact) {
        assertNotNull(vact.getVtnPopVlanAction());
    }
}
