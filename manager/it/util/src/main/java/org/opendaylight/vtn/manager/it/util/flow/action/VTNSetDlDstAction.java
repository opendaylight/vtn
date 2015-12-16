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

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.VtnSetDlDstActionCase;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.VtnSetDlDstActionCaseBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.vtn.set.dl.dst.action._case.VtnSetDlDstAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.vtn.set.dl.dst.action._case.VtnSetDlDstActionBuilder;

import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.yang.types.rev100924.MacAddress;

/**
 * {@code VTNSetDlDstAction} describes the configuration of flow action that
 * sets the destination MAC address into Ethernet header.
 */
public final class VTNSetDlDstAction
    extends VTNDlAddrAction<VtnSetDlDstActionCase> {
    /**
     * Construct an empty instance.
     */
    public VTNSetDlDstAction() {
    }

    /**
     * Construct a new instance.
     *
     * @param ord   The order of the flow action.
     * @param addr  The MAC address to set.
     */
    public VTNSetDlDstAction(Integer ord, EtherAddress addr) {
        super(ord, addr);
    }

    // FlowAction

    /**
     * Return a class that specifies the type of vtn-action.
     *
     * @return  A class of {@link VtnSetDlDstActionCase}.
     */
    @Override
    public Class<VtnSetDlDstActionCase> getActionType() {
        return VtnSetDlDstActionCase.class;
    }

    /**
     * Create a new vtn-action value.
     *
     * @return  A {@link VtnSetDlDstActionCase} instance.
     */
    @Override
    public VtnSetDlDstActionCase newVtnAction() {
        EtherAddress addr = getAddress();
        MacAddress mac = (addr == null) ? null : addr.getMacAddress();
        VtnSetDlDstAction vaction = new VtnSetDlDstActionBuilder().
            setAddress(mac).build();
        return new VtnSetDlDstActionCaseBuilder().
            setVtnSetDlDstAction(vaction).build();
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void verify(VtnSetDlDstActionCase vact) {
        VtnSetDlDstAction vaction = vact.getVtnSetDlDstAction();
        assertNotNull(vaction);

        EtherAddress addr = getAddress();
        assertEquals(addr.getMacAddress(), vaction.getAddress());
    }
}
