/*
 * Copyright (c) 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.vnode.xml;

import static org.junit.Assert.assertEquals;

import static org.opendaylight.vtn.manager.internal.TestBase.MAX_RANDOM;
import static org.opendaylight.vtn.manager.internal.TestBase.createVtnIndex;

import java.util.Random;
import java.util.Set;

import org.opendaylight.vtn.manager.internal.util.flow.filter.VTNFlowFilter;

import org.opendaylight.vtn.manager.internal.XmlNode;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.flow.filter.list.VtnFlowFilter;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.flow.filter.list.VtnFlowFilterBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.flow.filter.type.fields.VtnFlowFilterType;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeName;

/**
 * {@code XmlFlowFilter} describes the abstracted configuration of flow filter.
 *
 * @param <F>  The type of vtn-flow-filter-type.
 */
public abstract class XmlFlowFilter<F extends VtnFlowFilterType> {
    /**
     * The index of the flow filter.
     */
    private Integer  index;

    /**
     * The name of the flow condition.
     */
    private String  condition;

    /**
     * A list of flow actions.
     */
    private XmlFlowActionList  flowActions = new XmlFlowActionList();

    /**
     * Cache for vtn-flow-filter instance.
     */
    private VtnFlowFilter  filterCache;

    /**
     * Construct an empty instance.
     */
    protected XmlFlowFilter() {
    }

    /**
     * Construct a new instance.
     *
     * @param idx   The index of the flow filter.
     * @param cond  The name of the flow condition.
     */
    protected XmlFlowFilter(Integer idx, String cond) {
        index = idx;
        condition = cond;
    }

    /**
     * Return the index of the flow filter.
     *
     * @return  The index of the flow filter.
     */
    public final Integer getIndex() {
        return index;
    }

    /**
     * Return the name of the flow condition.
     *
     * @return  The name of the flow condition.
     */
    public final String getCondition() {
        return condition;
    }

    /**
     * Return the list of flow actions.
     *
     * @return  A {@link XmlFlowActionList} instance.
     */
    public final XmlFlowActionList getFlowActions() {
        return flowActions;
    }

    /**
     * Set the list of flow actions.
     *
     * @param list  A {@link XmlFlowActionList} instance.
     * @return  This instance.
     */
    public final XmlFlowFilter setFlowActions(XmlFlowActionList list) {
        flowActions = list;
        filterCache = null;
        return this;
    }

    /**
     * Convert this instance into a {@link VtnFlowFilter} instance.
     *
     * @return  A {@link VtnFlowFilter} instance.
     */
    public final VtnFlowFilter toVtnFlowFilter() {
        VtnFlowFilter vff = filterCache;
        if (vff == null) {
            VnodeName vcond = (condition == null)
                ? null : new VnodeName(condition);
            vff = new VtnFlowFilterBuilder().
                setIndex(index).
                setCondition(vcond).
                setVtnFlowAction(flowActions.toVtnFlowActionList()).
                setVtnFlowFilterType(newVtnFlowFilterType()).
                build();
            filterCache = vff;
        }

        return vff;
    }

    /**
     * Convert this instancel into a {@link XmlNode} instance.
     *
     * @return  A {@link XmlNode} instance.
     */
    public final XmlNode toXmlNode() {
        XmlNode xnode = new XmlNode(getXmlRoot());
        if (index != null) {
            xnode.add(new XmlNode("index", index));
        }
        if (condition != null) {
            xnode.add(new XmlNode("condition", condition));
        }
        flowActions.setXml(xnode);
        setXml(xnode);
        return xnode;
    }

    /**
     * Set parameters using the given random generator.
     *
     * @param rand     A pseudo random generator.
     * @param indices  A set of index values to store generated values.
     */
    public final void set(Random rand, Set<Integer> indices) {
        filterCache = null;
        index = createVtnIndex(rand, indices);
        condition = "cond_" + rand.nextInt(MAX_RANDOM);
        flowActions.add(rand);
        setImpl(rand);
    }

    /**
     * Ensure that the given instance is identical to this instance.
     *
     * @param ff  A {@link VTNFlowFilter} instance to be checked.
     */
    public final void verify(VTNFlowFilter ff) {
        assertEquals(index, ff.getIdentifier());
        assertEquals(condition, ff.getCondition());
        flowActions.verify(ff.getActions());
        verifyImpl(ff);
    }

    /**
     * Ensure that the given instance is identical to this instance.
     *
     * @param vff  A {@link VtnFlowFilter} instance to be checked.
     */
    public final void verify(VtnFlowFilter vff) {
        assertEquals(index, vff.getIndex());
        assertEquals(condition, vff.getCondition().getValue());
        flowActions.verifyModel(vff.getVtnFlowAction());
        verifyFilterType(vff.getVtnFlowFilterType());
    }

    /**
     * Set parameters specific to the filter type using the given random
     * generator.
     *
     * @param rand  A pseudo random generator.
     */
    protected void setImpl(Random rand) {
    }

    /**
     * Set the filter-specific values into the given {@link XmlNode} instance.
     *
     * @param xnode  A {@link XmlNode} instance.
     */
    protected void setXml(XmlNode xnode) {
    }

    /**
     * Ensure that the given flow filter type represents this flow filter.
     *
     * @param ftype  A {@link VtnFlowFilterType} instance to be checked.
     */
    protected void verifyFilterType(VtnFlowFilterType ftype) {
        assertEquals(toVtnFlowFilter().getVtnFlowFilterType(), ftype);
    }

    /**
     * Return a class that specifies the type of vtn-flow-filter-type.
     *
     * @return  A class that specifies the type of vtn-flow-filter-type.
     */
    protected abstract Class<F> getFilterType();

    /**
     * Return the name of the XML root node.
     *
     * @return  The name of the XML root node.
     */
    protected abstract String getXmlRoot();

    /**
     * Create a new vtn-flow-filter-type.
     *
     * @return  An instance of vtn-flow-filter-type.
     */
    protected abstract F newVtnFlowFilterType();

    /**
     * Ensure that the given instance is identical to this instance.
     *
     * @param ff  A {@link VTNFlowFilter} instance to be checked.
     */
    protected abstract void verifyImpl(VTNFlowFilter ff);
}

