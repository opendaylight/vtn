/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.util.flow.filter;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;

import static org.opendaylight.vtn.manager.it.util.TestBase.createVnodeName;

import java.util.Random;

import org.opendaylight.vtn.manager.it.util.vnode.VInterfaceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.flow.filter.type.fields.vtn.flow.filter.type.VtnRedirectFilterCase;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.flow.filter.type.fields.vtn.flow.filter.type.VtnRedirectFilterCaseBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.flow.filter.type.fields.vtn.flow.filter.type.vtn.redirect.filter._case.VtnRedirectFilter;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.flow.filter.type.fields.vtn.flow.filter.type.vtn.redirect.filter._case.VtnRedirectFilterBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.redirect.filter.config.RedirectDestination;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.redirect.filter.config.RedirectDestinationBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodePathFields;

/**
 * {@code RedirectFilter} describes the configuration of REDIRECT flow filter
 * that forwards packets to another virtual interface.
 */
public final class RedirectFilter extends FlowFilter<VtnRedirectFilterCase> {
    /**
     * The location of the destination virtual interface.
     */
    private RedirectDestination  destination;

    /**
     * A boolean value that determines the direction of the packet redirection.
     *
     * <p>
     *   {@code true} means that the packet is redirected as outgoing packet.
     * </p>
     */
    private Boolean  output;

    /**
     * Construct an empty instance.
     */
    public RedirectFilter() {
        destination = null;
        output = null;
    }

    /**
     * Construct a new instance.
     *
     * @param idx   The index of the flow filter.
     * @param cond  The name of the flow condition.
     * @param dst   The location of the destination virtual interface.
     * @param out   {@code true} means that the packet is redirected as
     *              outgoing packet.
     */
    public RedirectFilter(Integer idx, String cond, RedirectDestination dst,
                          Boolean out) {
        super(idx, cond);
        destination = dst;
        output = out;
    }

    /**
     * Construct a new instance.
     *
     * @param idx   The index of the flow filter.
     * @param cond  The name of the flow condition.
     * @param dst   The location of the destination virtual interface.
     * @param out   {@code true} means that the packet is redirected as
     *              outgoing packet.
     */
    public RedirectFilter(Integer idx, String cond, VnodePathFields dst,
                          Boolean out) {
        super(idx, cond);
        setDestination(dst);
        output = out;
    }

    /**
     * Construct a new instance.
     *
     * @param idx   The index of the flow filter.
     * @param cond  The name of the flow condition.
     * @param dst   The location of the destination virtual interface.
     * @param out   {@code true} means that the packet is redirected as
     *              outgoing packet.
     */
    public RedirectFilter(Integer idx, String cond,
                          VInterfaceIdentifier<?> dst, Boolean out) {
        super(idx, cond);
        setDestination(dst);
        output = out;
    }

    /**
     * Return the destination of the redirection.
     *
     * @return  A {@link RedirectDestination} instance that indicates the
     *          destination virtual interface.
     */
    public RedirectDestination getDestination() {
        return destination;
    }

    /**
     * Set the destination of the redirection.
     *
     * @param dst  A {@link RedirectDestination} instance that indicates the
     *             destination virtual interface.
     * @return  This instance.
     */
    public RedirectFilter setDestination(RedirectDestination dst) {
        destination = dst;
        return this;
    }

    /**
     * Set the destination of the redirection.
     *
     * @param dst  A {@link VnodePathFields} instance that indicates the
     *             destination virtual interface.
     * @return  This instance.
     */
    public RedirectFilter setDestination(VnodePathFields dst) {
        destination = (dst == null)
            ? null
            : new RedirectDestinationBuilder(dst).build();
        return this;
    }

    /**
     * Set the destination of the redirection.
     *
     * @param dst  A {@link VInterfaceIdentifier} instance that indicates the
     *             destination virtual interface.
     * @return  This instance.
     */
    public RedirectFilter setDestination(VInterfaceIdentifier<?> dst) {
        destination = (dst == null) ? null : dst.toRedirectDestination();
        return this;
    }

    // FlowFilter

    /**
     * Return a class that specifies the type of vtn-flow-filter-type.
     *
     * @return  A class of {@link VtnRedirectFilterCase}.
     */
    @Override
    public Class<VtnRedirectFilterCase> getFilterType() {
        return VtnRedirectFilterCase.class;
    }

    /**
     * Create a new vtn-flow-filter-type.
     *
     * @return  A {@link VtnRedirectFilterCase} instance.
     */
    @Override
    public VtnRedirectFilterCase newVtnFlowFilterType() {
        VtnRedirectFilter vrf = new VtnRedirectFilterBuilder().
            setRedirectDestination(destination).
            setOutput(output).
            build();
        return new VtnRedirectFilterCaseBuilder().
            setVtnRedirectFilter(vrf).
            build();
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void verify(VtnRedirectFilterCase vftype) {
        VtnRedirectFilter vrf = vftype.getVtnRedirectFilter();
        assertNotNull(vrf);

        Boolean out = output;
        if (out == null) {
            out = Boolean.FALSE;
        }
        assertEquals(out, vrf.isOutput());

        RedirectDestination rdst = vrf.getRedirectDestination();
        assertEquals(null, rdst.getTenantName());
        assertEquals(null, rdst.getRouterName());
        assertEquals(rdst.getInterfaceName(),
                     destination.getInterfaceName());

        String bname = destination.getBridgeName();
        if (bname != null) {
            assertEquals(bname, rdst.getBridgeName());
            assertEquals(null, rdst.getTerminalName());
        } else {
            assertEquals(destination.getTerminalName(),
                         rdst.getTerminalName());
            assertEquals(null, rdst.getBridgeName());
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void setImpl(Random rand) {
        String tname = (rand.nextBoolean())
            ? null : createVnodeName("vtn_", rand);
        String bname = createVnodeName("bridge_", rand);
        String iname = createVnodeName("if_", rand);

        RedirectDestinationBuilder builder = new RedirectDestinationBuilder().
            setTenantName(tname).
            setInterfaceName(iname);
        if (rand.nextBoolean()) {
            builder.setBridgeName(bname);
        } else {
            builder.setTerminalName(bname);
        }
        destination = builder.build();
        output = (rand.nextBoolean()) ? rand.nextBoolean() : null;
    }
}
