/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.util.flow.filter;

import static org.junit.Assert.assertNotNull;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.flow.filter.type.fields.vtn.flow.filter.type.VtnPassFilterCase;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.flow.filter.type.fields.vtn.flow.filter.type.VtnPassFilterCaseBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.flow.filter.type.fields.vtn.flow.filter.type.vtn.pass.filter._case.VtnPassFilterBuilder;

/**
 * {@code PassFilter} describes the configuration of PASS flow filter that
 * lets packets through the virtual node in the VTN.
 */
public final class PassFilter extends FlowFilter<VtnPassFilterCase> {
    /**
     * Construct an empty instance.
     */
    public PassFilter() {
    }

    /**
     * Construct a new instance.
     *
     * @param idx   The index of the flow filter.
     * @param cond  The name of the flow condition.
     */
    public PassFilter(Integer idx, String cond) {
        super(idx, cond);
    }

    // FlowFilter

    /**
     * Return a class that specifies the type of vtn-flow-filter-type.
     *
     * @return  A class of {@link VtnPassFilterCase}.
     */
    @Override
    public Class<VtnPassFilterCase> getFilterType() {
        return VtnPassFilterCase.class;
    }

    /**
     * Create a new vtn-flow-filter-type.
     *
     * @return  A {@link VtnPassFilterCase} instance.
     */
    @Override
    public VtnPassFilterCase newVtnFlowFilterType() {
        return new VtnPassFilterCaseBuilder().
            setVtnPassFilter(new VtnPassFilterBuilder().build()).
            build();
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void verify(VtnPassFilterCase vftype) {
        assertNotNull(vftype.getVtnPassFilter());
    }
}
