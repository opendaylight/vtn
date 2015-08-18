/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.flow.match;

import java.util.EnumSet;
import java.util.Set;

import org.opendaylight.vtn.manager.internal.util.packet.EtherHeader;
import org.opendaylight.vtn.manager.internal.util.packet.InetHeader;
import org.opendaylight.vtn.manager.internal.util.packet.Layer4Header;

import org.opendaylight.vtn.manager.internal.TestBase;

/**
 * {@code TestMatchContext} describes a {@code FlowMatchContext} only for
 * test.
 */
public final class TestMatchContext extends TestBase
    implements FlowMatchContext, Cloneable {
    /**
     * A set of {@link FlowMatchType} instances which represents match fields
     * to be configured.
     */
    private EnumSet<FlowMatchType>  matchFields =
        EnumSet.noneOf(FlowMatchType.class);

    /**
     * Ethernet header.
     */
    private EtherHeader  etherHeader;

    /**
     * IP header.
     */
    private InetHeader  inetHeader;

    /**
     * Layer 4 protocol header.
     */
    private Layer4Header  layer4Header;

    /**
     * Set the Ethernet header.
     *
     * @param eth  An {@link EtherHeader} instance.
     * @return  This instance.
     */
    public TestMatchContext setEtherHeader(EtherHeader eth) {
        etherHeader = eth;
        return this;
    }

    /**
     * Set the IP header.
     *
     * @param ip  An {@link InetHeader} instance.
     * @return  This instance.
     */
    public TestMatchContext setInetHeader(InetHeader ip) {
        inetHeader = ip;
        return this;
    }

    /**
     * Set the layer 4 protocol header.
     *
     * @param l4  A {@link Layer4Header} instance.
     * @return  This instance.
     */
    public TestMatchContext setLayer4Header(Layer4Header l4) {
        layer4Header = l4;
        return this;
    }

    /**
     * Ensure that only the given match fields are configured.
     *
     * @param types  An array of {@link FlowMatchType} instances to be
     *               configured.
     * @return  This instance.
     */
    public TestMatchContext checkMatchFields(FlowMatchType ... types) {
        assertEquals(types.length, matchFields.size());
        for (FlowMatchType type: types) {
            assertEquals(true, matchFields.contains(type));
        }
        return this;
    }

    /**
     * Ensure that only the given match fields are configured.
     *
     * @param set  A set of {@link FlowMatchType} instances to be configured.
     * @return  This instance.
     */
    public TestMatchContext checkMatchFields(Set<FlowMatchType> set) {
        assertEquals(set, matchFields);
        return this;
    }

    /**
     * Clear the match field.
     *
     * @return  This instance.
     */
    public TestMatchContext reset() {
        matchFields.clear();
        return this;
    }

    // PacketHeader

    /**
     * {@inheritDoc}
     */
    @Override
    public EtherHeader getEtherHeader() {
        return etherHeader;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public InetHeader getInetHeader() {
        return inetHeader;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public Layer4Header getLayer4Header() {
        return layer4Header;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public String getHeaderDescription() {
        return "";
    }

    // FlowMatchContext

    /**
     * {@inheritDoc}
     */
    @Override
    public void addMatchField(FlowMatchType type) {
        matchFields.add(type);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public boolean hasMatchField(FlowMatchType type) {
        return matchFields.contains(type);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void addUnicastMatchFields() {
        FlowMatchType.addUnicastTypes(matchFields);
    }


    // Cloneable

    /**
     * Return a deep copy of this instance.
     *
     * @return  A deep copy of this instance.
     */
    @Override
    public TestMatchContext clone() {
        try {
            TestMatchContext ctx = (TestMatchContext)super.clone();
            ctx.matchFields = EnumSet.copyOf(matchFields);
            return ctx;
        } catch (CloneNotSupportedException e) {
            // This should never happen.
            throw new IllegalStateException("clone() failed.", e);
        }
    }
}
