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

import static org.opendaylight.vtn.manager.it.util.TestBase.createUnsignedByte;

import java.util.Random;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.VtnSetIcmpTypeActionCase;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.VtnSetIcmpTypeActionCaseBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.vtn.set.icmp.type.action._case.VtnSetIcmpTypeAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.vtn.set.icmp.type.action._case.VtnSetIcmpTypeActionBuilder;

/**
 * {@code VTNSetIcmpTypeAction} describes the configuration of flow action that
 * sets the ICMP type into ICMP header.
 */
public final class VTNSetIcmpTypeAction
    extends FlowAction<VtnSetIcmpTypeActionCase> {
    /**
     * The default value of the ICMP type.
     */
    public static final Short  DEFAULT_TYPE = (short)0;

    /**
     * The ICMP type to set.
     */
    private Short  type;

    /**
     * Construct an empty instance.
     */
    public VTNSetIcmpTypeAction() {
        this(null, null);
    }

    /**
     * Construct a new instance.
     *
     * @param ord  The order of the flow action.
     * @param t    The ICMP type to set.
     */
    public VTNSetIcmpTypeAction(Integer ord, Short t) {
        super(ord);
        type = t;
    }

    /**
     * Return the ICMP type to set.
     *
     * @return  The ICMP type to set.
     */
    public Short getType() {
        return type;
    }

    // FlowAction

    /**
     * Return a class that specifies the type of vtn-action.
     *
     * @return  A class of {@link VtnSetIcmpTypeActionCase}.
     */
    @Override
    public Class<VtnSetIcmpTypeActionCase> getActionType() {
        return VtnSetIcmpTypeActionCase.class;
    }

    /**
     * Create a new vtn-action value.
     *
     * @return  A {@link VtnSetIcmpTypeActionCase} instance.
     */
    @Override
    public VtnSetIcmpTypeActionCase newVtnAction() {
        VtnSetIcmpTypeAction vaction = new VtnSetIcmpTypeActionBuilder().
            setType(type).build();
        return new VtnSetIcmpTypeActionCaseBuilder().
            setVtnSetIcmpTypeAction(vaction).build();
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void verify(VtnSetIcmpTypeActionCase vact) {
        VtnSetIcmpTypeAction vaction = vact.getVtnSetIcmpTypeAction();
        assertNotNull(vaction);

        Short t = (type == null) ? DEFAULT_TYPE : type;
        assertEquals(t, vaction.getType());
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void setImpl(Random rand) {
        type = createUnsignedByte(rand);
    }
}
