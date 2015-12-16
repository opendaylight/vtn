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

import org.opendaylight.vtn.manager.util.EtherAddress;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.VtnSetDlSrcActionCase;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.VtnSetDlSrcActionCaseBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.vtn.set.dl.src.action._case.VtnSetDlSrcAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.vtn.set.dl.src.action._case.VtnSetDlSrcActionBuilder;

import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.yang.types.rev100924.MacAddress;

/**
 * {@code VTNSetDlSrcAction} describes the configuration of flow action that
 * sets the source MAC address into Ethernet header.
 */
public final class VTNSetDlSrcAction
    extends VTNDlAddrAction<VtnSetDlSrcActionCase> {
    /**
     * Construct an empty instance.
     */
    public VTNSetDlSrcAction() {
    }

    /**
     * Construct a new instance.
     *
     * @param ord   The order of the flow action.
     * @param addr  The MAC address to set.
     */
    public VTNSetDlSrcAction(Integer ord, EtherAddress addr) {
        super(ord, addr);
    }

    // FlowAction

    /**
     * Return a class that specifies the type of vtn-action.
     *
     * @return  A class of {@link VtnSetDlSrcActionCase}.
     */
    @Override
    public Class<VtnSetDlSrcActionCase> getActionType() {
        return VtnSetDlSrcActionCase.class;
    }

    /**
     * Create a new vtn-action value.
     *
     * @return  A {@link VtnSetDlSrcActionCase} instance.
     */
    @Override
    public VtnSetDlSrcActionCase newVtnAction() {
        EtherAddress addr = getAddress();
        MacAddress mac = (addr == null) ? null : addr.getMacAddress();
        VtnSetDlSrcAction vaction = new VtnSetDlSrcActionBuilder().
            setAddress(mac).build();
        return new VtnSetDlSrcActionCaseBuilder().
            setVtnSetDlSrcAction(vaction).build();
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void verify(VtnSetDlSrcActionCase vact) {
        VtnSetDlSrcAction vaction = vact.getVtnSetDlSrcAction();
        assertNotNull(vaction);

        EtherAddress addr = getAddress();
        assertEquals(addr.getMacAddress(), vaction.getAddress());
    }
}
