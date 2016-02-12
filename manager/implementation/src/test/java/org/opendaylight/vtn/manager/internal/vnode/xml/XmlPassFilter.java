/*
 * Copyright (c) 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.vnode.xml;

import static org.junit.Assert.assertEquals;

import org.opendaylight.vtn.manager.internal.util.flow.filter.VTNFlowFilter;
import org.opendaylight.vtn.manager.internal.util.flow.filter.VTNPassFilter;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.flow.filter.type.fields.vtn.flow.filter.type.VtnPassFilterCase;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.flow.filter.type.fields.vtn.flow.filter.type.VtnPassFilterCaseBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.flow.filter.type.fields.vtn.flow.filter.type.vtn.pass.filter._case.VtnPassFilterBuilder;

/**
 * {@code PassFilter} describes the configuration of PASS flow filter that
 * lets packets through the virtual node in the VTN.
 */
public final class XmlPassFilter extends XmlFlowFilter<VtnPassFilterCase> {
    /**
     * Construct an empty instance.
     */
    public XmlPassFilter() {
    }

    /**
     * Construct a new instance.
     *
     * @param idx   The index of the flow filter.
     * @param cond  The name of the flow condition.
     */
    public XmlPassFilter(Integer idx, String cond) {
        super(idx, cond);
    }

    // XmlFlowFilter

    /**
     * Return a class that specifies the type of vtn-flow-filter-type.
     *
     * @return  A class of {@link VtnPassFilterCase}.
     */
    @Override
    protected Class<VtnPassFilterCase> getFilterType() {
        return VtnPassFilterCase.class;
    }

    /**
     * Return the name of the XML root node.
     *
     * @return  "pass-filter"
     */
    @Override
    protected String getXmlRoot() {
        return "pass-filter";
    }

    /**
     * Create a new vtn-flow-filter-type.
     *
     * @return  A {@link VtnPassFilterCase} instance.
     */
    @Override
    protected VtnPassFilterCase newVtnFlowFilterType() {
        return new VtnPassFilterCaseBuilder().
            setVtnPassFilter(new VtnPassFilterBuilder().build()).
            build();
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void verifyImpl(VTNFlowFilter ff) {
        assertEquals(VTNPassFilter.class, ff.getClass());
    }
}
