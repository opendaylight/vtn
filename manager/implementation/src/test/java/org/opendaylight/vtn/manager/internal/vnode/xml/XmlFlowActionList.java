/*
 * Copyright (c) 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.vnode.xml;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.fail;

import static org.opendaylight.vtn.manager.internal.TestBase.MAX_RANDOM;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Random;
import java.util.Set;

import com.google.common.collect.ImmutableList;

import org.opendaylight.vtn.manager.internal.util.flow.action.FlowFilterAction;

import org.opendaylight.vtn.manager.internal.XmlNode;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.flow.action.list.VtnFlowAction;

/**
 * {@code XmlFlowActionList} describes a list of flow actions.
 */
public final class XmlFlowActionList {
    /**
     * A set of flow actions supported by flow filter.
     */
    private static final List<Class<? extends XmlFlowAction<?>>>  ACTION_TYPES;

    /**
     * Initialize static field.
     */
    static {
        ACTION_TYPES =
            ImmutableList.<Class<? extends XmlFlowAction<?>>>builder().
            add(XmlSetDlSrcAction.class).
            add(XmlSetDlDstAction.class).
            add(XmlSetVlanPcpAction.class).
            add(XmlSetInetSrcAction.class).
            add(XmlSetInetDstAction.class).
            add(XmlSetInetDscpAction.class).
            add(XmlSetPortSrcAction.class).
            add(XmlSetPortDstAction.class).
            add(XmlSetIcmpTypeAction.class).
            add(XmlSetIcmpCodeAction.class).
            build();
    }

    /**
     * A list of flow actions indexed by action order.
     */
    private final Map<Integer, XmlFlowAction<?>>  flowActions = new HashMap<>();

    /**
     * Cache for XML node.
     */
    private XmlNode  xmlCache;

    /**
     * Construct an empty instance.
     */
    public XmlFlowActionList() {
    }

    /**
     * Construct a new instance using the given pseudo random number generator.
     *
     * @param rand  A pseudo random number generator.
     */
    public XmlFlowActionList(Random rand) {
        add(rand);
    }

    /**
     * Convert this instance into a list of vtn-flow-action.
     *
     * @return  A list of vtn-flow-action or {@code null}.
     */
    public List<VtnFlowAction> toVtnFlowActionList() {
        List<VtnFlowAction> actions;
        if (flowActions.isEmpty()) {
            actions = null;
        } else {
            actions = new ArrayList<>(flowActions.size());
            for (XmlFlowAction<?> xact: flowActions.values()) {
                actions.add(xact.toVtnFlowAction());
            }
        }

        return actions;
    }

    /**
     * Set flow actions in this instance into the specified XML node.
     *
     * @param xnode  A {@link XmlNode} instance.
     */
    public void setXml(XmlNode xnode) {
        if (!flowActions.isEmpty()) {
            XmlNode xactions = new XmlNode("actions");
            for (XmlFlowAction<?> xact: flowActions.values()) {
                xactions.add(xact.toXmlNode());
            }
            xnode.add(xactions);
        }
    }

    /**
     * Add the specified flow action.
     *
     * @param fact  A flow action to add.
     * @return  This instance.
     */
    public XmlFlowActionList add(XmlFlowAction<?> fact) {
        Integer order = fact.getOrder();
        assertEquals(null, flowActions.put(order, fact));
        return this;
    }

    /**
     * Add random flow actions using the given pseudo random number generator.
     *
     * @param rand   A pseudo random number generator.
     * @return  This instance.
     */
    public XmlFlowActionList add(Random rand) {
        int i = rand.nextInt(4);
        if (i == 1) {
            // All the supported flow actions.
            addAll(rand);
        } else if (i != 0) {
            addRandom(rand);
        }
        return this;
    }

    /**
     * Add all of the flow actions supported by flow filter.
     *
     * @param rand  A pseudo random number generator.
     * @return  This instance.
     */
    public XmlFlowActionList addAll(Random rand) {
        Set<Integer> orders = new HashSet<>(flowActions.keySet());
        for (Class<? extends XmlFlowAction<?>> type: ACTION_TYPES) {
            add(create(rand, orders, type));
        }
        return this;
    }

    /**
     * Add random flow actions using the given pseudo random number generator.
     *
     * @param rand   A pseudo random number generator.
     * @return  This instance.
     */
    public XmlFlowActionList addRandom(Random rand) {
        int count = rand.nextInt(MAX_RANDOM);
        Set<Integer> orders = new HashSet<>(flowActions.keySet());
        for (int i = 0; i < count; i++) {
            // Determine action type.
            int act = rand.nextInt(ACTION_TYPES.size());
            Class<? extends XmlFlowAction<?>> type = ACTION_TYPES.get(act);
            add(create(rand, orders, type));
        }
        return this;
    }

    /**
     * Ensure that the given vtn-flow-action list is identical to this
     * instance.
     *
     * <p>
     *   This method expects that the flow actions in the given list are sorted
     *   by action order.
     * </p>
     *
     * @param actions  A list of vtn-flow-action instances.
     */
    public void verifyModel(List<VtnFlowAction> actions) {
        if (actions == null || actions.isEmpty()) {
            assertEquals(Collections.<Integer, XmlFlowAction<?>>emptyMap(),
                         flowActions);
        } else {
            Set<Integer> checked = new HashSet<>();
            Integer prev = null;
            for (VtnFlowAction vfact: actions) {
                Integer order = vfact.getOrder();
                assertEquals(true, checked.add(order));
                assertEquals(flowActions.get(order).toVtnFlowAction(), vfact);

                if (prev == null) {
                    prev = order;
                } else if (prev.intValue() >= order.intValue()) {
                    fail("Invalid action order: prev=" + prev + ", order=" +
                         order);
                }
            }
            assertEquals(flowActions.keySet(), checked);
        }
    }

    /**
     * Ensure that the given {@link FlowFilterAction} list is identical to
     * this instance.
     *
     * @param actions  A list of {@link FlowFilterAction} instances.
     */
    public void verify(List<FlowFilterAction> actions) {
        if (flowActions.isEmpty()) {
            assertEquals(Collections.<FlowFilterAction>emptyList(), actions);
        } else {
            Set<Integer> checked = new HashSet<>();
            for (FlowFilterAction ffact: actions) {
                Integer order = ffact.getOrder();
                assertEquals(true, checked.add(order));
                flowActions.get(order).verify(ffact);
            }
            assertEquals(flowActions.keySet(), checked);
        }
    }

    /**
     * Create a new flow action using the given random generator.
     *
     * @param rand    A pseudo random generator.
     * @param orders  A set of action order values to store generated values.
     * @param type    A class that specifies the type of flow action.
     * @param <A>     The type of flow action.
     * @return  A new flow action.
     */
    private <A extends XmlFlowAction<?>> A create(
        Random rand, Set<Integer> orders, Class<A> type) {
        try {
            A fact = type.newInstance();
            fact.set(rand, orders);
            return fact;
        } catch (Exception e) {
            throw new IllegalStateException(
                "Failed to create flow action: " + type.getSimpleName(), e);
        }
    }
}
