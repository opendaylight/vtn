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

import static org.opendaylight.vtn.manager.it.util.TestBase.createVlanPcp;

import java.util.Random;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.VtnSetVlanPcpActionCase;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.VtnSetVlanPcpActionCaseBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.vtn.set.vlan.pcp.action._case.VtnSetVlanPcpAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.vtn.set.vlan.pcp.action._case.VtnSetVlanPcpActionBuilder;

import org.opendaylight.yang.gen.v1.urn.opendaylight.l2.types.rev130827.VlanPcp;

/**
 * {@code VTNSetVlanPcpAction} describes the configuration of flow action that
 * sets the VLAN priority into a VLAN tag.
 */
public final class VTNSetVlanPcpAction
    extends FlowAction<VtnSetVlanPcpActionCase> {
    /**
     * The default value of the VLAN priority.
     */
    public static final Short  DEFAULT_PRIORITY = (short)0;

    /**
     * The VLAN priority to set.
     */
    private Short  priority;

    /**
     * Construct an empty instance.
     */
    public VTNSetVlanPcpAction() {
        this(null, null);
    }

    /**
     * Construct a new instance.
     *
     * @param ord  The order of the flow action.
     * @param pcp  The VLAN priority to set.
     */
    public VTNSetVlanPcpAction(Integer ord, Short pcp) {
        super(ord);
        priority = pcp;
    }

    /**
     * Return the VLAN priority to set.
     *
     * @return  The VLAN priority to set.
     */
    public Short getPriority() {
        return priority;
    }

    // FlowAction

    /**
     * Return a class that specifies the type of vtn-action.
     *
     * @return  A class of {@link VtnSetVlanPcpActionCase}.
     */
    @Override
    public Class<VtnSetVlanPcpActionCase> getActionType() {
        return VtnSetVlanPcpActionCase.class;
    }

    /**
     * Create a new vtn-action value.
     *
     * @return  A {@link VtnSetVlanPcpActionCase} instance.
     */
    @Override
    public VtnSetVlanPcpActionCase newVtnAction() {
        VtnSetVlanPcpAction vaction = new VtnSetVlanPcpActionBuilder().
            setVlanPcp(new VlanPcp(priority)).build();
        return new VtnSetVlanPcpActionCaseBuilder().
            setVtnSetVlanPcpAction(vaction).build();
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void verify(VtnSetVlanPcpActionCase vact) {
        VtnSetVlanPcpAction vaction = vact.getVtnSetVlanPcpAction();
        assertNotNull(vaction);

        Short pcp = (priority == null) ? DEFAULT_PRIORITY : priority;
        assertEquals(pcp, vaction.getVlanPcp().getValue());
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void setImpl(Random rand) {
        priority = createVlanPcp(rand);
    }
}
