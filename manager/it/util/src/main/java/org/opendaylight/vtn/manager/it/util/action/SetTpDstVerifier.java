/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
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
import org.opendaylight.vtn.manager.it.util.packet.PacketFactory;

import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.SetTpDstActionCase;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.set.tp.dst.action._case.SetTpDstAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.list.Action;

import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.inet.types.rev100924.PortNumber;

/**
 * {@code SetTpDstVerifier} is a utility class used to verify a SET_TP_DST
 * action.
 *
 * @param <T>  The type of packet factory corresponding to the target protocol.
 */
public abstract class SetTpDstVerifier<T extends PacketFactory>
    extends InetPortVerifier<T> {
    /**
     * Ensure that the specified action is an expected SET_TP_DST action.
     *
     * @param it  Action list iterator.
     * @param v   Expected value to be set.
     */
    public static final void verify(ListIterator<Action> it, int v) {
        SetTpDstActionCase act = verify(it, SetTpDstActionCase.class);
        SetTpDstAction stda = act.getSetTpDstAction();
        PortNumber port = stda.getPort();
        assertNotNull(port);
        Integer num = port.getValue();
        assertNotNull(num);
        assertEquals(v, num.intValue());
    }

    /**
     * Construct a new instance.
     *
     * @param cls  Packet factory class corresponding to the target packet.
     * @param v    The value to be set.
     */
    SetTpDstVerifier(Class<T> cls, int v) {
        super(cls, v);
    }

    // ActionVerifier

    /**
     * {@inheritDoc}
     */
    @Override
    public void verify(EthernetFactory efc, ListIterator<Action> it) {
        if (getFactory(efc, getFactoryType()) != null) {
            verify(it, getValue());
        }
    }
}
