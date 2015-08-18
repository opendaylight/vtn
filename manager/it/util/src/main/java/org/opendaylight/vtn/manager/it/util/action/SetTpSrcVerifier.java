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
import org.opendaylight.vtn.manager.it.util.packet.PacketFactory;

import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.SetTpSrcActionCase;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.set.tp.src.action._case.SetTpSrcAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.list.Action;

import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.inet.types.rev100924.PortNumber;

/**
 * {@code SetTpSrcVerifier} is a utility class used to verify a SET_TP_SRC
 * action.
 *
 * @param <T>  The type of packet factory corresponding to the target protocol.
 */
public abstract class SetTpSrcVerifier<T extends PacketFactory>
    extends InetPortVerifier<T> {
    /**
     * Ensure that the specified action is an expected SET_TP_SRC action.
     *
     * @param it  Action list iterator.
     * @param v   Expected value to be set.
     */
    public static final void verify(ListIterator<Action> it, short v) {
        SetTpSrcActionCase act = verify(it, SetTpSrcActionCase.class);
        SetTpSrcAction stsa = act.getSetTpSrcAction();
        PortNumber port = stsa.getPort();
        assertNotNull(port);
        Integer num = port.getValue();
        assertNotNull(num);
        assertEquals(v, num.shortValue());
    }

    /**
     * Construct a new instance.
     *
     * @param cls  Packet factory class corresponding to the target packet.
     * @param v    The value to be set.
     */
    SetTpSrcVerifier(Class<T> cls, short v) {
        super(cls, v);
    }

    // ActionVerifier

    /**
     * {@inheritDoc}
     */
    @Override
    public final void verify(EthernetFactory efc, ListIterator<Action> it) {
        if (getFactory(efc, getFactoryType()) != null) {
            verify(it, getValue());
        }
    }
}
