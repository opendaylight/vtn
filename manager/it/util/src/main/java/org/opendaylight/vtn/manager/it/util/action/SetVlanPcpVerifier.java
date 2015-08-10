/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.util.action;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;

import java.util.ListIterator;

import org.opendaylight.vtn.manager.it.util.packet.EthernetFactory;

import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.SetVlanPcpActionCase;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.set.vlan.pcp.action._case.SetVlanPcpAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.list.Action;

import org.opendaylight.yang.gen.v1.urn.opendaylight.l2.types.rev130827.VlanPcp;

/**
 * {@code SetDlSrcVerifier} is a utility class used to verify a SET_VLAN_PCP
 * action.
 */
public final class SetVlanPcpVerifier extends ActionVerifier {
    /**
     * The VLAN priority to be set.
     */
    private final byte  vlanPcp;

    /**
     * Ensure that the specified action is an expected SET_VLAN_PCP action.
     *
     * @param it   Action list iterator.
     * @param pcp  Expected VLAN priority.
     */
    public static void verify(ListIterator<Action> it, byte pcp) {
        SetVlanPcpActionCase act = verify(it, SetVlanPcpActionCase.class);
        SetVlanPcpAction svpa = act.getSetVlanPcpAction();
        VlanPcp vpcp = svpa.getVlanPcp();
        assertNotNull(vpcp);

        Short value = vpcp.getValue();
        assertNotNull(value);
        assertEquals(pcp, value.byteValue());
    }

    /**
     * Construct a new instance.
     *
     * @param pcp  VLAN priority to be set.
     */
    public SetVlanPcpVerifier(byte pcp) {
        vlanPcp = pcp;
    }

    /**
     * Return the VLAN priority to be set.
     *
     * @return  The VLAN priority to be set.
     */
    public byte getVlanPcp() {
        return vlanPcp;
    }

    // ActionVerifier

    /**
     * {@inheritDoc}
     */
    @Override
    public void verify(EthernetFactory efc, ListIterator<Action> it) {
        if (efc.getVlanId() != 0) {
            verify(it, vlanPcp);
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void apply(EthernetFactory efc) {
        if (efc.getVlanId() != 0) {
            efc.setVlanPcp(vlanPcp);
        }
    }
}
