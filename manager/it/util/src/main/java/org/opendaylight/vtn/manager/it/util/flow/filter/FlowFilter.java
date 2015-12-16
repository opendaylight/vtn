/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.util.flow.filter;

import static org.junit.Assert.assertEquals;

import static org.opendaylight.vtn.manager.it.util.TestBase.createVnodeName;
import static org.opendaylight.vtn.manager.it.util.TestBase.createVtnIndex;

import java.util.Collections;
import java.util.Random;
import java.util.Set;

import org.opendaylight.vtn.manager.it.util.flow.action.FlowActionList;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.SetFlowFilterInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.flow.filter.list.VtnFlowFilter;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.flow.filter.list.VtnFlowFilterBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.flow.filter.type.fields.VtnFlowFilterType;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeName;

/**
 * {@code FlowFilter} describes the abstracted configuration of flow filter.
 *
 * @param <F>  The type of vtn-flow-filter-type.
 */
public abstract class FlowFilter<F extends VtnFlowFilterType> {
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
    private final FlowActionList  flowActions = new FlowActionList();

    /**
     * Construct an empty instance.
     */
    protected FlowFilter() {
        this(null, null);
    }

    /**
     * Construct a new instance.
     *
     * @param idx   The index of the flow filter.
     * @param cond  The name of the flow condition.
     */
    protected FlowFilter(Integer idx, String cond) {
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
     * @return  A {@link FlowActionList} instance.
     */
    public final FlowActionList getFlowActions() {
        return flowActions;
    }

    /**
     * Convert this instance into a {@link VtnFlowFilter} instance.
     *
     * @return  A {@link VtnFlowFilter} instance.
     */
    public final VtnFlowFilter toVtnFlowFilter() {
        VnodeName vcond = (condition == null)
            ? null : new VnodeName(condition);
        return new VtnFlowFilterBuilder().
            setIndex(index).
            setCondition(vcond).
            setVtnFlowAction(flowActions.toVtnFlowActionList()).
            setVtnFlowFilterType(newVtnFlowFilterType()).
            build();
    }

    /**
     * Create a new input builder for set-flow-filter RPC.
     *
     * @return  A {@link SetFlowFilterInputBuilder} instance.
     */
    public SetFlowFilterInputBuilder newInputBuilder() {
        VtnFlowFilter vff = toVtnFlowFilter();
        return new SetFlowFilterInputBuilder().
            setVtnFlowFilter(Collections.singletonList(vff));
    }

    /**
     * Verify the given flow filter.
     *
     * @param vff  A {@link VtnFlowFilter} instance to be verified.
     */
    public final void verify(VtnFlowFilter vff) {
        assertEquals(index, vff.getIndex());
        assertEquals(condition, vff.getCondition().getValue());

        VtnFlowFilterType vftype = vff.getVtnFlowFilterType();
        Class<F> type = getFilterType();
        assertEquals(true, type.isInstance(vftype));
        verify(type.cast(vftype));
    }

    /**
     * Set parameters using the given random generator.
     *
     * @param rand     A pseudo random generator.
     * @param indices  A set of index values to store generated values.
     */
    public final void set(Random rand, Set<Integer> indices) {
        index = createVtnIndex(rand, indices);
        condition = createVnodeName("cond_", rand);
        flowActions.add(rand);
        setImpl(rand);
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
     * Return a class that specifies the type of vtn-flow-filter-type.
     *
     * @return  A class that specifies the type of vtn-flow-filter-type.
     */
    public abstract Class<F> getFilterType();

    /**
     * Create a new vtn-flow-filter-type.
     *
     * @return  An instance of vtn-flow-filter-type.
     */
    public abstract F newVtnFlowFilterType();

    /**
     * Verify the given vtn-flow-filter-type.
     *
     * @param vftype  An instance of vtn-flow-filter-type.
     */
    public abstract void verify(F vftype);
}
