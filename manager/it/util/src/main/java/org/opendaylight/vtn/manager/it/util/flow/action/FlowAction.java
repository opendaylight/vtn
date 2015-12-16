/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.util.flow.action;

import static org.junit.Assert.assertEquals;

import static org.opendaylight.vtn.manager.it.util.TestBase.createInteger;

import java.util.Random;
import java.util.Set;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.VtnAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.flow.action.list.VtnFlowAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.flow.action.list.VtnFlowActionBuilder;

/**
 * {@code FlowAction} describes the abstracted configuration of flow action.
 *
 * @param <A>  The type of vtn-action.
 */
public abstract class FlowAction<A extends VtnAction> {
    /**
     * The order value of the flow action.
     */
    private Integer  order;

    /**
     * Construct an empty instance.
     */
    protected FlowAction() {
        this(null);
    }

    /**
     * Construct a new instance.
     *
     * @param ord  The order of the flow action.
     */
    protected FlowAction(Integer ord) {
        order = ord;
    }

    /**
     * Return the order of the flow action.
     *
     * @return  The order of the flow action.
     */
    public final Integer getOrder() {
        return order;
    }

    /**
     * Convert this instance into a {@link VtnFlowAction} instance.
     *
     * @return  A {@link VtnFlowAction} instance.
     */
    public final VtnFlowAction toVtnFlowAction() {
        return new VtnFlowActionBuilder().
            setOrder(order).
            setVtnAction(newVtnAction()).
            build();
    }

    /**
     * Verify the given flow action.
     *
     * @param vfact  A {@link VtnFlowAction} instance to be verified.
     */
    public final void verify(VtnFlowAction vfact) {
        assertEquals(order, vfact.getOrder());

        VtnAction vact = vfact.getVtnAction();
        Class<A> type = getActionType();
        assertEquals(true, type.isInstance(vact));
        verify(type.cast(vact));
    }

    /**
     * Set parameters using the given random generator.
     *
     * @param rand    A pseudo random generator.
     * @param orders  A set of action order values to store generated values.
     */
    public final void set(Random rand, Set<Integer> orders) {
        order = createInteger(rand, orders);
        setImpl(rand);
    }

    /**
     * Set parameters specific to the action type using the given random
     * generator.
     *
     * @param rand  A pseudo random generator.
     */
    protected void setImpl(Random rand) {
    }

    /**
     * Return a class that specifies the type of vtn-action.
     *
     * @return  A class that specifies the type of vtn-action.
     */
    public abstract Class<A> getActionType();

    /**
     * Create a new vtn-action value.
     *
     * @return  A vtn-action value.
     */
    public abstract A newVtnAction();

    /**
     * Verify the given vtn-action value.
     *
     * @param vact  A vtn-action  to be verified.
     */
    public abstract void verify(A vact);
}
