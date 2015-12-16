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

import static org.opendaylight.vtn.manager.it.util.TestBase.createDscp;

import java.util.Random;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.VtnSetInetDscpActionCase;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.VtnSetInetDscpActionCaseBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.vtn.set.inet.dscp.action._case.VtnSetInetDscpAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.vtn.set.inet.dscp.action._case.VtnSetInetDscpActionBuilder;

import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.inet.types.rev100924.Dscp;

/**
 * {@code VTNSetInetDscpAction} describes the configuration of flow action that
 * sets the DSCP value into IP header.
 */
public final class VTNSetInetDscpAction
    extends FlowAction<VtnSetInetDscpActionCase> {
    /**
     * The default value of the DSCP value.
     */
    public static final Short  DEFAULT_DSCP = (short)0;

    /**
     * The DSCP value to set.
     */
    private Short  dscp;

    /**
     * Construct an empty instance.
     */
    public VTNSetInetDscpAction() {
        this(null, null);
    }

    /**
     * Construct a new instance.
     *
     * @param ord  The order of the flow action.
     * @param d    The DSCP value to set.
     */
    public VTNSetInetDscpAction(Integer ord, Short d) {
        super(ord);
        dscp = d;
    }

    /**
     * Return the DSCP value to set.
     *
     * @return  The DSCP value to set.
     */
    public Short getDscp() {
        return dscp;
    }

    // FlowAction

    /**
     * Return a class that specifies the type of vtn-action.
     *
     * @return  A class of {@link VtnSetInetDscpActionCase}.
     */
    @Override
    public Class<VtnSetInetDscpActionCase> getActionType() {
        return VtnSetInetDscpActionCase.class;
    }

    /**
     * Create a new vtn-action value.
     *
     * @return  A {@link VtnSetInetDscpActionCase} instance.
     */
    @Override
    public VtnSetInetDscpActionCase newVtnAction() {
        VtnSetInetDscpAction vaction = new VtnSetInetDscpActionBuilder().
            setDscp(new Dscp(dscp)).build();
        return new VtnSetInetDscpActionCaseBuilder().
            setVtnSetInetDscpAction(vaction).build();
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void verify(VtnSetInetDscpActionCase vact) {
        VtnSetInetDscpAction vaction = vact.getVtnSetInetDscpAction();
        assertNotNull(vaction);

        Short d = (dscp == null) ? DEFAULT_DSCP : dscp;
        assertEquals(d, vaction.getDscp().getValue());
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void setImpl(Random rand) {
        dscp = createDscp(rand);
    }
}
