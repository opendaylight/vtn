/*
 * Copyright (c) 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.vnode.xml;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;

import static org.opendaylight.vtn.manager.internal.TestBase.MAX_RANDOM;

import java.util.Random;

import org.opendaylight.vtn.manager.internal.util.flow.filter.VTNFlowFilter;
import org.opendaylight.vtn.manager.internal.util.flow.filter.VTNRedirectFilter;
import org.opendaylight.vtn.manager.internal.util.vnode.VBridgeIfIdentifier;
import org.opendaylight.vtn.manager.internal.util.vnode.VInterfaceIdentifier;
import org.opendaylight.vtn.manager.internal.util.vnode.VNodeType;
import org.opendaylight.vtn.manager.internal.util.vnode.VTerminalIfIdentifier;

import org.opendaylight.vtn.manager.internal.XmlNode;
import org.opendaylight.vtn.manager.internal.util.flow.filter.VTNRedirectFilterTest;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.flow.filter.type.fields.VtnFlowFilterType;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.flow.filter.type.fields.vtn.flow.filter.type.VtnRedirectFilterCase;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.flow.filter.type.fields.vtn.flow.filter.type.VtnRedirectFilterCaseBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.flow.filter.type.fields.vtn.flow.filter.type.vtn.redirect.filter._case.VtnRedirectFilter;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.flow.filter.type.fields.vtn.flow.filter.type.vtn.redirect.filter._case.VtnRedirectFilterBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.redirect.filter.config.RedirectDestination;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeName;

/**
 * {@code XmlRedirectFilter} describes the configuration of REDIRECT flow
 * filter that forwards packets to another virtual interface.
 */
public final class XmlRedirectFilter
    extends XmlFlowFilter<VtnRedirectFilterCase> {
    /**
     * The base name of the virtual node for the destination.
     */
    public static final String  DESTINATION_BASE = "redirect_";

    /**
     * The location of the destination virtual interface.
     */
    private VInterfaceIdentifier<?>  destination;

    /**
     * Determine the direction of the packet redirection.
     *
     * {@code true} means that the packet is redirected as outgoing packet.
     */
    private Boolean  output;

    /**
     * Construct an empty instance.
     */
    public XmlRedirectFilter() {
    }

    /**
     * Construct a new instance.
     *
     * @param idx   The index of the flow filter.
     * @param cond  The name of the flow condition.
     * @param dest  The location of the destination interface.
     * @param out   A boolean value that specifies the direction.
     */
    public XmlRedirectFilter(Integer idx, String cond,
                             VInterfaceIdentifier<?> dest, Boolean out) {
        super(idx, cond);
        destination = dest;
        output = out;
    }

    /**
     * Return the location of the destination interface.
     *
     * @return  A {@link VInterfaceIdentifier} instance.
     */
    public VInterfaceIdentifier<?> getDestination() {
        return destination;
    }

    /**
     * Return the direction of the redirection.
     *
     * @return  {@code true} if the packet is redirected as outgoing packet.
     *          {@code false} otherwise.
     */
    public Boolean isOutput() {
        return output;
    }

    /**
     * Create a virtual node name for the destination interface.
     *
     * @param rand  A pseudo random number generator.
     * @param name  A string to be embedded in the name.
     * @return  The name of the virtual node.
     */
    private VnodeName createVnodeName(Random rand, String name) {
        return new VnodeName(
            DESTINATION_BASE + name + "_" + rand.nextInt(MAX_RANDOM));
    }

    /**
     * Ensure that the given vtn-redirect-filter-type represents this REDIRECT
     * filter.
     *
     * @param rftype  A {@link VtnRedirectFilterCase} instance to be checked.
     */
    private void verifyRedirectFilterType(VtnRedirectFilterCase rftype) {
        VtnRedirectFilter vrf = rftype.getVtnRedirectFilter();
        RedirectDestination rdest = vrf.getRedirectDestination();
        if (destination == null) {
            assertEquals(null, rdest);
        } else {
            assertEquals(null, rdest.getTenantName());
            assertEquals(null, rdest.getRouterName());
            assertEquals(destination.getInterfaceNameString(),
                         rdest.getInterfaceName());

            String bname = destination.getBridgeNameString();
            VNodeType btype = destination.getType().getBridgeType();
            if (btype == VNodeType.VBRIDGE) {
                assertEquals(bname, rdest.getBridgeName());
                assertEquals(null, rdest.getTerminalName());
            } else {
                assertEquals(null, rdest.getBridgeName());
                assertEquals(bname, rdest.getTerminalName());
            }
        }

        Boolean out = output;
        if (out == null) {
            // Defaults to false.
            out = Boolean.FALSE;
        }
        assertEquals(out, vrf.isOutput());
    }

    // XmlFlowFilter

    /**
     * {@inheritDoc}
     */
    @Override
    protected void setImpl(Random rand) {
        VnodeName tname = null;
        VnodeName bname = createVnodeName(rand, "bridge");
        VnodeName iname = createVnodeName(rand, "if");
        destination = (rand.nextBoolean())
            ? new VBridgeIfIdentifier(tname, bname, iname)
            : new VTerminalIfIdentifier(tname, bname, iname);
        if (rand.nextBoolean()) {
            output = Boolean.TRUE;
        } else {
            output = (rand.nextBoolean()) ? Boolean.FALSE : null;
        }
    }

    /**
     * Set the filter-specific values into the given {@link XmlNode} instance.
     *
     * @param xnode  A {@link XmlNode} instance.
     */
    @Override
    protected void setXml(XmlNode xnode) {
        if (destination != null) {
            VNodeType btype = destination.getType().getBridgeType();
            String name = (btype == VNodeType.VBRIDGE)
                ? "vbridge-if"
                : "vterminal-if";
            String bname = destination.getBridgeNameString();
            String iname = destination.getInterfaceNameString();
            XmlNode dest = new XmlNode(name).
                add(new XmlNode("bridge-name", bname)).
                add(new XmlNode("interface-name", iname));
            xnode.add(dest);
        }
        if (output != null) {
            xnode.add(new XmlNode("output", output));
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void verifyFilterType(VtnFlowFilterType ftype) {
        assertTrue(ftype instanceof VtnRedirectFilterCase);
        verifyRedirectFilterType((VtnRedirectFilterCase)ftype);
    }

    /**
     * Return a class that specifies the type of vtn-flow-filter-type.
     *
     * @return  A class of {@link VtnRedirectFilterCase}.
     */
    @Override
    protected Class<VtnRedirectFilterCase> getFilterType() {
        return VtnRedirectFilterCase.class;
    }

    /**
     * Return the name of the XML root node.
     *
     * @return  "redirect-filter"
     */
    @Override
    protected String getXmlRoot() {
        return "redirect-filter";
    }

    /**
     * Create a new vtn-flow-filter-type.
     *
     * @return  A {@link VtnRedirectFilterCase} instance.
     */
    @Override
    protected VtnRedirectFilterCase newVtnFlowFilterType() {
        RedirectDestination rdest = (destination == null)
            ? null
            : destination.toRedirectDestination();
        VtnRedirectFilter vrf = new VtnRedirectFilterBuilder().
            setRedirectDestination(rdest).
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
    protected void verifyImpl(VTNFlowFilter ff) {
        assertEquals(VTNRedirectFilter.class, ff.getClass());
        VTNRedirectFilter rff = (VTNRedirectFilter)ff;
        verifyRedirectFilterType(VTNRedirectFilterTest.getFilterType(rff));
    }
}
