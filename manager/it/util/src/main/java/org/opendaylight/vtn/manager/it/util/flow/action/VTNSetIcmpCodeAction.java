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

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.VtnSetIcmpCodeActionCase;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.VtnSetIcmpCodeActionCaseBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.vtn.set.icmp.code.action._case.VtnSetIcmpCodeAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.vtn.set.icmp.code.action._case.VtnSetIcmpCodeActionBuilder;

/**
 * {@code VTNSetIcmpCodeAction} describes the configuration of flow action that
 * sets the ICMP code into ICMP header.
 */
public final class VTNSetIcmpCodeAction
    extends FlowAction<VtnSetIcmpCodeActionCase> {
    /**
     * The default value of the ICMP code.
     */
    public static final Short  DEFAULT_CODE = (short)0;

    /**
     * The ICMP code to set.
     */
    private Short  code;

    /**
     * Construct an empty instance.
     */
    public VTNSetIcmpCodeAction() {
        this(null, null);
    }

    /**
     * Construct a new instance.
     *
     * @param ord  The order of the flow action.
     * @param c    The ICMP code to set.
     */
    public VTNSetIcmpCodeAction(Integer ord, Short c) {
        super(ord);
        code = c;
    }

    /**
     * Return the ICMP code to set.
     *
     * @return  The ICMP code to set.
     */
    public Short getCode() {
        return code;
    }

    // FlowAction

    /**
     * Return a class that specifies the code of vtn-action.
     *
     * @return  A class of {@link VtnSetIcmpCodeActionCase}.
     */
    @Override
    public Class<VtnSetIcmpCodeActionCase> getActionType() {
        return VtnSetIcmpCodeActionCase.class;
    }

    /**
     * Create a new vtn-action value.
     *
     * @return  A {@link VtnSetIcmpCodeActionCase} instance.
     */
    @Override
    public VtnSetIcmpCodeActionCase newVtnAction() {
        VtnSetIcmpCodeAction vaction = new VtnSetIcmpCodeActionBuilder().
            setCode(code).build();
        return new VtnSetIcmpCodeActionCaseBuilder().
            setVtnSetIcmpCodeAction(vaction).build();
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void verify(VtnSetIcmpCodeActionCase vact) {
        VtnSetIcmpCodeAction vaction = vact.getVtnSetIcmpCodeAction();
        assertNotNull(vaction);

        Short c = (code == null) ? DEFAULT_CODE : code;
        assertEquals(c, vaction.getCode());
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void setImpl(Random rand) {
        code = createUnsignedByte(rand);
    }
}
