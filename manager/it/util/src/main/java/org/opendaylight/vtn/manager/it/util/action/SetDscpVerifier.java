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

import org.opendaylight.vtn.manager.it.util.packet.Inet4Factory;
import org.opendaylight.vtn.manager.it.util.packet.EthernetFactory;

import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.SetNwTosActionCase;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.set.nw.tos.action._case.SetNwTosAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.list.Action;

/**
 * {@code SetDscpVerifier} is a utility class used to verify a SET_NW_TOS
 * (DSCP) action.
 */
public final class SetDscpVerifier extends ActionVerifier {
    /**
     * The DSCP value to be set.
     */
    private final byte  dscp;

    /**
     * Ensure that the specified action is an expected SET_NW_TOS action.
     *
     * @param it  Action list iterator.
     * @param d   Expected DSCP value.
     */
    public static void verify(ListIterator<Action> it, byte d) {
        SetNwTosActionCase act = verify(it, SetNwTosActionCase.class);
        SetNwTosAction snta = act.getSetNwTosAction();
        Integer value = snta.getTos();
        assertNotNull(value);
        assertEquals(d, value.byteValue());
    }

    /**
     * Construct a new instance.
     *
     * @param d  The DSCP value to be set.
     */
    public SetDscpVerifier(byte d) {
        dscp = d;
    }

    /**
     * Return the DSCP value to be set.
     *
     * @return  The DSCP value to be set.
     */
    public byte setDscp() {
        return dscp;
    }

    // ActionVerifier

    /**
     * {@inheritDoc}
     */
    @Override
    public void verify(EthernetFactory efc, ListIterator<Action> it) {
        Inet4Factory i4fc = getFactory(efc, Inet4Factory.class);
        if (i4fc != null) {
            verify(it, dscp);
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void apply(EthernetFactory efc) {
        Inet4Factory i4fc = getFactory(efc, Inet4Factory.class);
        if (i4fc != null) {
            i4fc.setDscp(dscp);
        }
    }
}
