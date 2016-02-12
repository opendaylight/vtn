/*
 * Copyright (c) 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.vnode.xml;

import static org.junit.Assert.assertEquals;

import org.opendaylight.vtn.manager.internal.util.flow.filter.VTNDropFilter;
import org.opendaylight.vtn.manager.internal.util.flow.filter.VTNFlowFilter;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.flow.filter.type.fields.vtn.flow.filter.type.VtnDropFilterCase;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.flow.filter.type.fields.vtn.flow.filter.type.VtnDropFilterCaseBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.flow.filter.type.fields.vtn.flow.filter.type.vtn.drop.filter._case.VtnDropFilterBuilder;

/**
 * {@code XmlDropFilter} describes the configuration of DROP flow filter that
 * discards packets.
 */
public final class XmlDropFilter extends XmlFlowFilter<VtnDropFilterCase> {
    /**
     * Construct an empty instance.
     */
    public XmlDropFilter() {
    }

    /**
     * Construct a new instance.
     *
     * @param idx   The index of the flow filter.
     * @param cond  The name of the flow condition.
     */
    public XmlDropFilter(Integer idx, String cond) {
        super(idx, cond);
    }

    // XmlFlowFilter

    /**
     * Return a class that specifies the type of vtn-flow-filter-type.
     *
     * @return  A class of {@link VtnDropFilterCase}.
     */
    @Override
    protected Class<VtnDropFilterCase> getFilterType() {
        return VtnDropFilterCase.class;
    }

    /**
     * Return the name of the XML root node.
     *
     * @return  "drop-filter"
     */
    @Override
    protected String getXmlRoot() {
        return "drop-filter";
    }

    /**
     * Create a new vtn-flow-filter-type.
     *
     * @return  A {@link VtnDropFilterCase} instance.
     */
    @Override
    protected VtnDropFilterCase newVtnFlowFilterType() {
        return new VtnDropFilterCaseBuilder().
            setVtnDropFilter(new VtnDropFilterBuilder().build()).
            build();
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void verifyImpl(VTNFlowFilter ff) {
        assertEquals(VTNDropFilter.class, ff.getClass());
    }
}
