/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.util.flow.action;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.VtnSetVlanIdActionCase;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.VtnSetVlanIdActionCaseBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.vtn.set.vlan.id.action._case.VtnSetVlanIdAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.vtn.set.vlan.id.action._case.VtnSetVlanIdActionBuilder;

/**
 * {@code VTNSetVlanIdAction} describes the configuration of flow action that
 * sets the VLAN ID into VLAN tag.
 */
public final class VTNSetVlanIdAction
    extends FlowAction<VtnSetVlanIdActionCase> {
    /**
     * The VLAN ID to set.
     */
    private final Integer  vlanId;

    /**
     * Construct an empty instance.
     */
    public VTNSetVlanIdAction() {
        this(null, null);
    }

    /**
     * Construct a new instance.
     *
     * @param ord  The order of the flow action.
     * @param vid  The VLAN ID to set.
     */
    public VTNSetVlanIdAction(Integer ord, Integer vid) {
        super(ord);
        vlanId = vid;
    }

    /**
     * Return the VLAN ID to set.
     *
     * @return  The VLAN ID to set.
     */
    public Integer getVlanId() {
        return vlanId;
    }

    // FlowAction

    /**
     * Return a class that specifies the type of vtn-action.
     *
     * @return  A class of {@link VtnSetVlanIdActionCase}.
     */
    @Override
    public Class<VtnSetVlanIdActionCase> getActionType() {
        return VtnSetVlanIdActionCase.class;
    }

    /**
     * Create a new vtn-action value.
     *
     * @return  A {@link VtnSetVlanIdActionCase} instance.
     */
    @Override
    public VtnSetVlanIdActionCase newVtnAction() {
        VtnSetVlanIdAction vaction = new VtnSetVlanIdActionBuilder().
            setVlanId(vlanId).build();
        return new VtnSetVlanIdActionCaseBuilder().
            setVtnSetVlanIdAction(vaction).build();
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void verify(VtnSetVlanIdActionCase vact) {
        VtnSetVlanIdAction vaction = vact.getVtnSetVlanIdAction();
        assertNotNull(vaction);
        assertEquals(vlanId, vaction.getVlanId());
    }
}
