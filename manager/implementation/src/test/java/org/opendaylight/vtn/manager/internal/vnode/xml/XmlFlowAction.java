/*
 * Copyright (c) 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.vnode.xml;

import static org.junit.Assert.assertEquals;

import static org.opendaylight.vtn.manager.internal.TestBase.createInteger;

import java.util.Random;
import java.util.Set;

import org.opendaylight.vtn.manager.internal.util.flow.action.FlowFilterAction;

import org.opendaylight.vtn.manager.internal.XmlNode;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.VtnAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.flow.action.list.VtnFlowAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.flow.action.list.VtnFlowActionBuilder;

/**
 * {@code XmlFlowAction} describes the abstracted configuration of flow action.
 *
 * @param <A>  The type of vtn-action.
 */
public abstract class XmlFlowAction<A extends VtnAction> {
    /**
     * The order value of the flow action.
     */
    private Integer  order;

    /**
     * Cache for vtn-flow-action instance.
     */
    private VtnFlowAction  actionCache;

    /**
     * Construct an empty instance.
     */
    protected XmlFlowAction() {
    }

    /**
     * Construct a new instance.
     *
     * @param ord  The order of the flow action.
     */
    protected XmlFlowAction(Integer ord) {
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
        VtnFlowAction vfact = actionCache;
        if (vfact == null) {
            vfact = new VtnFlowActionBuilder().
                setOrder(order).
                setVtnAction(newVtnAction()).
                build();
            actionCache = vfact;
        }

        return vfact;
    }

    /**
     * Convert this instancel into a {@link XmlNode} instance.
     *
     * @return  A {@link XmlNode} instance.
     */
    public final XmlNode toXmlNode() {
        XmlNode xnode = new XmlNode(getXmlRoot());
        if (order != null) {
            xnode.add(new XmlNode("order", order));
        }
        setXml(xnode);
        return xnode;
    }

    /**
     * Set parameters using the given random generator.
     *
     * @param rand    A pseudo random generator.
     * @param orders  A set of action order values to store generated values.
     */
    public final void set(Random rand, Set<Integer> orders) {
        actionCache = null;
        order = createInteger(rand, orders);
        setImpl(rand);
    }

    /**
     * Ensure that the given instance is identical to this instance.
     *
     * @param ffact  A {@link FlowFilterAction} instance to be checked.
     */
    public final void verify(FlowFilterAction ffact) {
        assertEquals(order, ffact.getOrder());
        verifyImpl(ffact);
    }

    /**
     * Return a class that specifies the type of vtn-action.
     *
     * @return  A class that specifies the type of vtn-action.
     */
    protected abstract Class<A> getActionType();

    /**
     * Return the name of the XML root node.
     *
     * @return  The name of the XML root node.
     */
    protected abstract String getXmlRoot();

    /**
     * Create a new vtn-action value.
     *
     * @return  A vtn-action value.
     */
    protected abstract A newVtnAction();

    /**
     * Set parameters specific to the action type using the given random
     * generator.
     *
     * @param rand  A pseudo random generator.
     */
    protected abstract void setImpl(Random rand);

    /**
     * Set the action-specific values into the given {@link XmlNode} instance.
     *
     * @param xnode  A {@link XmlNode} instance.
     */
    protected abstract void setXml(XmlNode xnode);

    /**
     * Ensure that the given instance is identical to this instance.
     *
     * @param ffact  A {@link FlowFilterAction} instance to be checked.
     */
    protected abstract void verifyImpl(FlowFilterAction ffact);
}
