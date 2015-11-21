/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.flow.filter;

import javax.xml.bind.annotation.XmlAccessType;
import javax.xml.bind.annotation.XmlAccessorType;
import javax.xml.bind.annotation.XmlRootElement;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import org.opendaylight.vtn.manager.flow.filter.FlowFilter;
import org.opendaylight.vtn.manager.flow.filter.PassFilter;

import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;
import org.opendaylight.vtn.manager.internal.util.vnode.VNodeIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.VtnFlowFilterConfig;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.flow.filter.type.fields.vtn.flow.filter.type.VtnPassFilterCase;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.flow.filter.type.fields.vtn.flow.filter.type.VtnPassFilterCaseBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.flow.filter.type.fields.vtn.flow.filter.type.vtn.pass.filter._case.VtnPassFilterBuilder;

/**
 * {@code VTNPassFilter} describes configuration information about flow
 * filter which lets packets through the virtual node in the VTN.
 */
@XmlRootElement(name = "vtn-pass-filter")
@XmlAccessorType(XmlAccessType.NONE)
public final class VTNPassFilter extends VTNFlowFilter {
    /**
     * Logger instance.
     */
    private static final Logger  LOG =
        LoggerFactory.getLogger(VTNPassFilter.class);

    /**
     * Private constructor only for JAXB.
     */
    private VTNPassFilter() {
    }

    /**
     * Construct a new instance.
     *
     * @param vffc  A {@link VtnFlowFilterConfig} instance which contains
     *              flow filter configuration.
     * @param pass  A {@link VtnPassFilterCase} instance configured in
     *              {@code vffc}.
     * @throws RpcException
     *    {@code vffc} contains invalid value.
     */
    VTNPassFilter(VtnFlowFilterConfig vffc, VtnPassFilterCase pass)
        throws RpcException {
        super(vffc);

        if (pass.getVtnPassFilter() == null) {
            throw RpcException.getNullArgumentException("vtn-pass-filter");
        }
    }

    /**
     * Construct a new instance.
     *
     * @param idx     An index number to be assigned to the flow filter.
     * @param filter  A {@link FlowFilter} instance which contains flow filter
     *                configuration.
     * @throws RpcException
     *    {@code vffc} contains invalid value.
     */
    VTNPassFilter(int idx, FlowFilter filter) throws RpcException {
        super(idx, filter);
    }

    // VTNFlowFilter

    /**
     * This method does nothing.
     */
    @Override
    protected void verifyImpl() {
    }

    /**
     * This method does nothing.
     *
     * @param ident  Unused.
     */
    @Override
    public void canSet(VNodeIdentifier<?> ident) {
    }

    /**
     * Return a {@link org.opendaylight.vtn.manager.flow.filter.FilterType}
     * instance which indicates the type of this flow filter.
     *
     * @return  A {@link PassFilter} instance.
     */
    @Override
    protected PassFilter getFilterType() {
        return new PassFilter();
    }

    /**
     * Return a {@link org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.flow.filter.type.fields.VtnFlowFilterType}
     * instance which indicates the type of this flow filter.
     *
     * @return  A {@link VtnPassFilterCase} instance.
     */
    @Override
    protected VtnPassFilterCase getVtnFlowFilterType() {
        return new VtnPassFilterCaseBuilder().
            setVtnPassFilter(new VtnPassFilterBuilder().build()).
            build();
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected Logger getLogger() {
        return LOG;
    }

    /**
     * Apply this PASS flow filter to the given packet.
     *
     * @param fctx   A flow filter context which contains the packet.
     * @param flid   A {@link FlowFilterListId} instance that specifies the
     *               location of this flow filter list.
     */
    @Override
    protected void apply(FlowFilterContext fctx, FlowFilterListId flid) {
        // Nothing to do.
    }
}
