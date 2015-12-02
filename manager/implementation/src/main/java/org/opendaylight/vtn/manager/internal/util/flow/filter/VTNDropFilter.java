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

import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;
import org.opendaylight.vtn.manager.internal.util.vnode.VNodeIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.VtnFlowFilterConfig;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.flow.filter.type.fields.vtn.flow.filter.type.VtnDropFilterCase;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.flow.filter.type.fields.vtn.flow.filter.type.VtnDropFilterCaseBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.flow.filter.type.fields.vtn.flow.filter.type.vtn.drop.filter._case.VtnDropFilterBuilder;

/**
 * {@code VTNDropFilter} describes configuration information about flow
 * filter which discards packets.
 */
@XmlRootElement(name = "vtn-drop-filter")
@XmlAccessorType(XmlAccessType.NONE)
public final class VTNDropFilter extends VTNFlowFilter {
    /**
     * Logger instance.
     */
    private static final Logger  LOG =
        LoggerFactory.getLogger(VTNDropFilter.class);

    /**
     * Private constructor only for JAXB.
     */
    private VTNDropFilter() {
    }

    /**
     * Construct a new instance.
     *
     * @param vffc  A {@link VtnFlowFilterConfig} instance which contains
     *              flow filter configuration.
     * @param drop  A {@link VtnDropFilterCase} instance configured in
     *              {@code vffc}.
     * @throws RpcException
     *    {@code vffc} contains invalid value.
     */
    VTNDropFilter(VtnFlowFilterConfig vffc, VtnDropFilterCase drop)
        throws RpcException {
        super(vffc);

        if (drop.getVtnDropFilter() == null) {
            throw RpcException.getNullArgumentException("vtn-drop-filter");
        }
    }

    // VTNFlowFilter

    /**
     * Determine whether this flow filter can handle multicast packets or not.
     *
     * <p>
     *   This method always returns {@code true} because DROP filter can
     *   discard multicast packets.
     * </p>
     *
     * @return  {@code true}.
     */
    @Override
    public boolean isMulticastSupported() {
        return true;
    }

    /**
     * Determine whether this flow filter needs to apply flow actions to the
     * packet.
     *
     * <p>
     *   This method always returns {@code false} because applying flow actions
     *   to the packet to be discarded is meaningless.
     * </p>
     *
     * @return  {@code false}.
     */
    public boolean needFlowAction() {
        return false;
    }

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
     * Return a {@link org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.flow.filter.type.fields.VtnFlowFilterType}
     * instance which indicates the type of this flow filter.
     *
     * @return  A {@link VtnDropFilterCase} instance.
     */
    @Override
    protected VtnDropFilterCase getVtnFlowFilterType() {
        return new VtnDropFilterCaseBuilder().
            setVtnDropFilter(new VtnDropFilterBuilder().build()).
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
     * Apply this DROP flow filter to the given packet.
     *
     * @param fctx   A flow filter context which contains the packet.
     * @param flid   A {@link FlowFilterListId} instance that specifies the
     *               location of this flow filter list.
     * @throws DropFlowException  Always thrown.
     */
    @Override
    protected void apply(FlowFilterContext fctx, FlowFilterListId flid)
        throws DropFlowException {
        if (!fctx.isFlooding()) {
            LOG.info("{}: Discard packet: cond={}, packet={}",
                     getPathString(flid), getCondition(),
                     fctx.getDescription());
        }

        // Install a flow entry that discards the given packet.
        fctx.installDropFlow();

        throw new DropFlowException();
    }
}
