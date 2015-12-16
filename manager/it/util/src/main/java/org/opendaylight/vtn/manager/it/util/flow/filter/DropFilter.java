/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.util.flow.filter;

import static org.junit.Assert.assertNotNull;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.flow.filter.type.fields.vtn.flow.filter.type.VtnDropFilterCase;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.flow.filter.type.fields.vtn.flow.filter.type.VtnDropFilterCaseBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.flow.filter.type.fields.vtn.flow.filter.type.vtn.drop.filter._case.VtnDropFilterBuilder;

/**
 * {@code DropFilter} describes the configuration of DROP flow filter that
 * discards packets.
 */
public final class DropFilter extends FlowFilter<VtnDropFilterCase> {
    /**
     * Construct an empty instance.
     */
    public DropFilter() {
    }

    /**
     * Construct a new instance.
     *
     * @param idx   The index of the flow filter.
     * @param cond  The name of the flow condition.
     */
    public DropFilter(Integer idx, String cond) {
        super(idx, cond);
    }

    // FlowFilter

    /**
     * Return a class that specifies the type of vtn-flow-filter-type.
     *
     * @return  A class of {@link VtnDropFilterCase}.
     */
    @Override
    public Class<VtnDropFilterCase> getFilterType() {
        return VtnDropFilterCase.class;
    }

    /**
     * Create a new vtn-flow-filter-type.
     *
     * @return  A {@link VtnDropFilterCase} instance.
     */
    @Override
    public VtnDropFilterCase newVtnFlowFilterType() {
        return new VtnDropFilterCaseBuilder().
            setVtnDropFilter(new VtnDropFilterBuilder().build()).
            build();
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void verify(VtnDropFilterCase vftype) {
        assertNotNull(vftype.getVtnDropFilter());
    }
}
