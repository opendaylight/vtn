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

import static org.opendaylight.vtn.manager.it.util.TestBase.unexpected;

import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Random;
import java.util.Set;

import com.google.common.collect.ImmutableSet;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.flow.action.list.VtnFlowAction;

/**
 * {@code FlowActionList} describes an ordered list of flow action
 * configurations.
 */
public final class FlowActionList {
    /**
     * A set of flow actions supported by flow filter.
     */
    private static final Set<Class<? extends FlowAction>>  FILTER_ACTION_TYPES;

    /**
     * Flow actions indexed by order value.
     */
    private final Map<Integer, FlowAction<?>>  flowActions = new HashMap<>();

    /**
     * Initialize static field.
     */
    static {
        FILTER_ACTION_TYPES =
            ImmutableSet.<Class<? extends FlowAction>>builder().
            add(VTNSetDlSrcAction.class).
            add(VTNSetDlDstAction.class).
            add(VTNSetVlanPcpAction.class).
            add(VTNSetInetSrcAction.class).
            add(VTNSetInetDstAction.class).
            add(VTNSetInetDscpAction.class).
            add(VTNSetPortSrcAction.class).
            add(VTNSetPortDstAction.class).
            add(VTNSetIcmpTypeAction.class).
            add(VTNSetIcmpCodeAction.class).
            build();
    }

    /**
     * Add random flow actions using the given random generator.
     *
     * @param rand  A pseudo random generator.
     * @return  This instance.
     */
    public FlowActionList add(Random rand) {
        Set<Integer> orders = new HashSet<>(flowActions.keySet());
        for (Class<? extends FlowAction> type: FILTER_ACTION_TYPES) {
            add(rand, orders, type);
        }

        return this;
    }

    /**
     * Add the given flow action to this list.
     *
     * @param fact  A {@link FlowAction} instance.
     * @return  This instance.
     */
    public FlowActionList add(FlowAction<?> fact) {
        flowActions.put(fact.getOrder(), fact);
        return this;
    }

    /**
     * Remove the flow action specified by the action order.
     *
     * @param order  The order of flow action.
     * @return  This instance.
     */
    public FlowActionList remove(Integer order) {
        flowActions.remove(order);
        return this;
    }

    /**
     * Remove all the flow actions in this list.
     *
     * @return  This instance.
     */
    public FlowActionList clear() {
        flowActions.clear();
        return this;
    }

    /**
     * Return an unmodifiable collection of flow action configurations.
     *
     * @return  An unmodifiable collection of {@link FlowAction} instances.
     */
    public Collection<FlowAction<?>> getFlowActions() {
        return Collections.unmodifiableCollection(flowActions.values());
    }

    /**
     * Convert this instance into a list of {@link VtnFlowAction} instances.
     *
     * @return  A list of {@link VtnFlowAction} instances or {@code null}.
     */
    public List<VtnFlowAction> toVtnFlowActionList() {
        return toVtnFlowActionList(false);
    }

    /**
     * Convert this instance into a list of {@link VtnFlowAction} instances.
     *
     * @param empty  If {@code true}, this method returns an empty list
     *               if the action list is empty.
     *               If {@code false}, this method returns {@code null}
     *               if the action list is empty.
     * @return  A list of {@link VtnFlowAction} instances or {@code null}.
     */
    public List<VtnFlowAction> toVtnFlowActionList(boolean empty) {
        List<VtnFlowAction> vfactions;
        if (!flowActions.isEmpty()) {
            vfactions = new ArrayList<>(flowActions.size());
            for (FlowAction<?> fact: flowActions.values()) {
                vfactions.add(fact.toVtnFlowAction());
            }
        } else if (empty) {
            vfactions = Collections.<VtnFlowAction>emptyList();
        } else {
            vfactions = null;
        }

        return vfactions;
    }

    /**
     * Verify the given list of flow actions.
     *
     * @param vfactions  A list of flow actions to be verified.
     */
    public void verify(List<VtnFlowAction> vfactions) {
        if (!flowActions.isEmpty()) {
            assertNotNull(vfactions);
            Set<Integer> checked = new HashSet<>();
            for (VtnFlowAction vfact: vfactions) {
                Integer order = vfact.getOrder();
                FlowAction<?> fact = flowActions.get(order);
                assertNotNull(fact);
                fact.verify(vfact);
                assertEquals(true, checked.add(order));
            }

            assertEquals(checked, flowActions.keySet());
        } else if (vfactions != null) {
            assertEquals(true, vfactions.isEmpty());
        }
    }

    /**
     * Add the specified flow action using the given random generator.
     *
     * @param rand    A pseudo random generator.
     * @param orders  A set of action order values to store generated values.
     * @param type    A class that specifies the type of flow action.
     * @param <A>     The type of flow action.
     */
    private <A extends FlowAction<?>> void add(
        Random rand, Set<Integer> orders, Class<A> type) {
        if (rand.nextBoolean()) {
            try {
                A fact = type.newInstance();
                fact.set(rand, orders);
                add(fact);
            } catch (Exception e) {
                unexpected(e);
            }
        }
    }
}
