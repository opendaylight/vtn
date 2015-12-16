/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.util.flow.action;

import static org.junit.Assert.assertNotNull;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.VtnPushVlanActionCase;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.VtnPushVlanActionCaseBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.vtn.push.vlan.action._case.VtnPushVlanAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.vtn.push.vlan.action._case.VtnPushVlanActionBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VlanType;

/**
 * {@code VTNPushVlanAction} describes the configuration of flow action that
 * adds a VLAN tag into Ethernet frame.
 */
public final class VTNPushVlanAction
    extends FlowAction<VtnPushVlanActionCase> {
    /**
     * The Ethernet type for a new VLAN tag.
     */
    private final VlanType  vlanType;

    /**
     * Construct a new instance that adds an IEEE 802.1q VLAN tag.
     */
    public VTNPushVlanAction() {
        this(null, VlanType.VLAN);
    }

    /**
     * Construct a new instance that adds an IEEE 802.1q VLAN tag.
     *
     * @param ord  The order of the flow action.
     */
    public VTNPushVlanAction(Integer ord) {
        this(ord, VlanType.VLAN);
    }

    /**
     * Construct a new instance.
     *
     * @param ord   The order of the flow action.
     * @param type  The Ethernet type for a new VLAN tag.
     */
    public VTNPushVlanAction(Integer ord, VlanType type) {
        super(ord);
        vlanType = type;
    }

    /**
     * Return the Ethernet type for a new VLAN tag.
     *
     * @return  A {@link VlanType} instance.
     */
    public VlanType getVlanType() {
        return vlanType;
    }

    // FlowAction

    /**
     * Return a class that specifies the type of vtn-action.
     *
     * @return  A class of {@link VtnPushVlanActionCase}.
     */
    @Override
    public Class<VtnPushVlanActionCase> getActionType() {
        return VtnPushVlanActionCase.class;
    }

    /**
     * Create a new vtn-action value.
     *
     * @return  A {@link VtnPushVlanActionCase} instance.
     */
    @Override
    public VtnPushVlanActionCase newVtnAction() {
        VtnPushVlanAction vaction = new VtnPushVlanActionBuilder().
            setVlanType(vlanType).build();
        return new VtnPushVlanActionCaseBuilder().
            setVtnPushVlanAction(vaction).build();
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void verify(VtnPushVlanActionCase vact) {
        assertNotNull(vact.getVtnPushVlanAction());
    }
}
