/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */
package org.opendaylight.vtn.manager.it.util.action;

import static org.junit.Assert.fail;

import java.util.ListIterator;

import org.opendaylight.vtn.manager.it.util.packet.EthernetFactory;
import org.opendaylight.vtn.manager.it.util.packet.PacketFactory;

import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.list.Action;

/**
 * {@code ActionVerifier} is a utility classs used to verify a flow action.
 */
public abstract class ActionVerifier {
    /**
     * Verify the given flow action.
     *
     * @param it   Action list iterator.
     * @param cls  Expected flow action class.
     * @param <T>  The type of flow action.
     * @return  Flow action instance.
     */
    public static <T> T verify(ListIterator<Action> it, Class<T> cls) {
        if (!it.hasNext()) {
            String msg = String.format("action[%d]: Expected %s action.",
                                       it.nextIndex(), cls.getSimpleName());
            fail(msg);
        }

        Action act = it.next();
        Object a = act.getAction();
        if (!cls.isInstance(a)) {
            String msg = String.format("action[%d]: %s is expected, but %s.",
                                       it.previousIndex(), cls.getSimpleName(),
                                       a);
            fail(msg);
        }

        return cls.cast(a);
    }


    /**
     * Return the specified packet factory in the given factory chain.
     *
     * @param factory  A {@link PacketFactory} instance.
     * @param cls      Expected packet factory class.
     * @param <T>      The type of packet factory.
     * @return  Packet factory instance or {@code null}.
     */
    public static <T extends PacketFactory> T getFactory(
        PacketFactory factory, Class<T> cls) {
        PacketFactory fc = factory;
        while (fc != null) {
            if (cls.isInstance(fc)) {
                return cls.cast(fc);
            }
            fc = fc.getNextFactory();
        }

        return null;
    }

    /**
     * Construct a new instance.
     */
    ActionVerifier() {}

    /**
     * Verify the given flow action.
     *
     * @param efc  A {@link PacketFactory} instance which represents the packet.
     * @param it   Action list iterator.
     */
    public abstract void verify(EthernetFactory efc, ListIterator<Action> it);

    /**
     * Apply the flow action against the given packet factory.
     *
     * @param efc  A {@link EthernetFactory} instance.
     */
    public abstract void apply(EthernetFactory efc);
}
