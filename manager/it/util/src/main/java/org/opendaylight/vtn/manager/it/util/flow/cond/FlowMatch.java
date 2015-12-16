/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.util.flow.cond;

import static org.junit.Assert.assertEquals;

import org.opendaylight.vtn.manager.it.util.flow.match.VTNMatch;
import org.opendaylight.vtn.manager.it.util.flow.match.VTNEtherMatch;
import org.opendaylight.vtn.manager.it.util.flow.match.VTNInetMatch;
import org.opendaylight.vtn.manager.it.util.flow.match.VTNLayer4Match;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.set.flow.condition.match.input.FlowMatchList;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.set.flow.condition.match.input.FlowMatchListBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.vtn.flow.cond.config.VtnFlowMatch;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.vtn.flow.cond.config.VtnFlowMatchBuilder;

/**
 * {@code VTNFlowMatch} describes the condition to match against packtes
 * in a flow condition.
 */
public final class FlowMatch extends VTNMatch {
    /**
     * An index value which identifies flow match in a flow condition.
     */
    private Integer  index;

    /**
     * Construct a new instance without specifying match condition.
     *
     * @param idx  An index value that identifies flow match in a flow
     *             condition.
     */
    public FlowMatch(Integer idx) {
        index = idx;
    }

    /**
     * Construct a new instance.
     *
     * @param idx  An index value that identifies flow match in a flow
     *             condition.
     * @param ematch   A {@link VTNEtherMatch} instance which specifies the
     *                 condition for Ethernet header.
     * @param imatch   A {@link VTNInetMatch} instance which specifies the
     *                 condition for IP header.
     * @param l4match  A {@link VTNLayer4Match} instance which specifies the
     *                 condition for layer 4 protocol header.
     */
    public FlowMatch(Integer idx, VTNEtherMatch ematch, VTNInetMatch imatch,
                     VTNLayer4Match l4match) {
        super(ematch, imatch, l4match);
        index = idx;
    }

    /**
     * Return the index value that identifies the flow match in a flow
     * condition.
     *
     * @return  The index value.
     */
    public Integer getIndex() {
        return index;
    }

    /**
     * Convert this instance into a {@link VtnFlowMatch} instance.
     *
     * @return  A {@link VtnFlowMatch} instance.
     */
    public VtnFlowMatch toVtnFlowMatch() {
        VtnFlowMatchBuilder builder = new VtnFlowMatchBuilder().
            setIndex(index);

        VTNEtherMatch eth = getEtherMatch();
        if (eth != null) {
            builder.setVtnEtherMatch(eth.toVtnEtherMatch());
        }

        VTNInetMatch inet = getInetMatch();
        if (inet != null) {
            builder.setVtnInetMatch(inet.toVtnInetMatch());
        }

        VTNLayer4Match l4 = getLayer4Match();
        if (l4 != null) {
            builder.setVtnLayer4Match(l4.toVtnLayer4Match());
        }

        return builder.build();
    }

    /**
     * Convert this instance into a {@link FlowMatchList} instance.
     *
     * @return  A {@link FlowMatchList} instance.
     */
    public FlowMatchList toFlowMatchList() {
        FlowMatchListBuilder builder = new FlowMatchListBuilder().
            setIndex(index);

        VTNEtherMatch eth = getEtherMatch();
        if (eth != null) {
            builder.setVtnEtherMatch(eth.toVtnEtherMatch());
        }

        VTNInetMatch inet = getInetMatch();
        if (inet != null) {
            builder.setVtnInetMatch(inet.toVtnInetMatch());
        }

        VTNLayer4Match l4 = getLayer4Match();
        if (l4 != null) {
            builder.setVtnLayer4Match(l4.toVtnLayer4Match());
        }

        return builder.build();
    }

    /**
     * Verify the given vtn-flow-match instance.
     *
     * @param vfm  A {@link VtnFlowMatch} instance.
     */
    public void verify(VtnFlowMatch vfm) {
        assertEquals(index, vfm.getIndex());
        verifyMatch(vfm);
    }

    // VTNMatch

    /**
     * {@inheritDoc}
     */
    @Override
    public FlowMatch setEtherMatch(VTNEtherMatch ematch) {
        super.setEtherMatch(ematch);
        return this;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public FlowMatch setInetMatch(VTNInetMatch imatch) {
        super.setInetMatch(imatch);
        return this;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public FlowMatch setLayer4Match(VTNLayer4Match l4match) {
        super.setLayer4Match(l4match);
        return this;
    }

    // Object

    /**
     * Create a copy of this instance.
     *
     * @return  A copy of this instance.
     */
    @Override
    public FlowMatch clone() {
        return (FlowMatch)super.clone();
    }
}
