/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.flow.cond;

import java.util.Collections;
import java.util.HashMap;
import java.util.Map;

import org.opendaylight.vtn.manager.flow.cond.FlowMatch;

import org.opendaylight.vtn.manager.internal.XmlNode;
import org.opendaylight.vtn.manager.internal.util.flow.match.MatchParams;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.set.flow.condition.match.input.FlowMatchList;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.vtn.flow.cond.config.VtnFlowMatch;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.vtn.flow.cond.config.VtnFlowMatchBuilder;

/**
 * {@code FlowMatchParams} describes parameters for a flow match in a
 * flow condition.
 */
public final class FlowMatchParams extends MatchParams {
    /**
     * A map that contains {@link FlowMatchParams} instances for test.
     */
    private static final Map<FlowMatchParams, FlowMatchParams> MATCH_PARAMS;

    /**
     * The index to be assigned for a flow match.
     */
    private Integer  index;

    /**
     * Initialize static fields.
     */
    static {
        Map<FlowMatchParams, FlowMatchParams> map = new HashMap<>();

        int idx = 1;
        for (Map.Entry<MatchParams, MatchParams> entry:
                 MatchParams.createMatches().entrySet()) {
            MatchParams params = entry.getKey();
            MatchParams expected = entry.getValue();
            map.put(new FlowMatchParams(idx, params),
                    new FlowMatchParams(idx, expected));
            idx++;
        }

        MATCH_PARAMS = Collections.unmodifiableMap(map);
    }

    /**
     * Create a map that contains {@link FlowMatchParams} instances for test.
     *
     * <p>
     *   An instance of {@link FlowMatchParams} to be used to construct
     *   {@link VTNFlowMatch} instance is set as a key, and an instance of
     *   {@link FlowMatchParams} instances that contains conditions expected to
     *   be configured in {@link VTNFlowMatch} is set as a value.
     * </p>
     *
     * @return  A map that contains {@link FlowMatchParams} instances.
     */
    public static Map<FlowMatchParams, FlowMatchParams> createFlowMatches() {
        Map<FlowMatchParams, FlowMatchParams> map =
            new HashMap<>(MATCH_PARAMS.size());
        for (Map.Entry<FlowMatchParams, FlowMatchParams> entry:
                 MATCH_PARAMS.entrySet()) {
            FlowMatchParams params = entry.getKey();
            FlowMatchParams expected = entry.getValue();
            map.put(params.clone(), expected.clone());
        }

        return map;
    }

    /**
     * Construct an empty instance.
     */
    public FlowMatchParams() {
    }

    /**
     * Construct a new instance.
     *
     * @param idx     The index to be assigned.
     */
    public FlowMatchParams(int idx) {
        index = Integer.valueOf(idx);
    }

    /**
     * Construct a new instance.
     *
     * @param idx     The index to be assigned.
     * @param params  A {@link MatchParams} instance.
     */
    public FlowMatchParams(int idx, MatchParams params) {
        super(params);
        index = Integer.valueOf(idx);
    }

    /**
     * Return the index for this flow match.
     *
     * @return  The index for this flow match.
     */
    public Integer getIndex() {
        return index;
    }

    /**
     * Set the index for this flow match.
     *
     * @param idx  The index for this flow match.
     * @return  This instance.
     */
    public FlowMatchParams setIndex(Integer idx) {
        index = idx;
        return this;
    }

    /**
     * Construct a new {@link VTNFlowMatch} instance.
     *
     * @return  A {@link VTNFlowMatch} instance.
     * @throws Exception  An error occurred.
     */
    public VTNFlowMatch toVTNFlowMatch() throws Exception {
        return new VTNFlowMatch(toFlowMatch());
    }

    /**
     * Ensure that the given {@link VTNFlowMatch} instance contains the same
     * conditions as this instance.
     *
     * @param vfmatch  A {@link VTNFlowMatch} instance.
     * @throws Exception  An error occurred.
     */
    public void verify(VTNFlowMatch vfmatch) throws Exception {
        super.verify(vfmatch);
        assertEquals(index, vfmatch.getIdentifier());

        VtnFlowMatch vfm = toVtnFlowMatchBuilder().build();
        assertEquals(vfmatch, new VTNFlowMatch(vfm));

        FlowMatchList fml = vfmatch.toFlowMatchListBuilder().build();
        assertEquals(vfmatch, new VTNFlowMatch(fml));
    }

    // MatchParams

    /**
     * {@inheritDoc}
     */
    @Override
    public FlowMatchParams reset() {
        super.reset();
        index = null;
        return this;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public FlowMatch toFlowMatch() {
        return (index == null)
            ? super.toFlowMatch()
            : new FlowMatch(index.intValue(), getEthernetMatch(),
                            getInetMatch(), getL4Match());
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public VtnFlowMatchBuilder toVtnFlowMatchBuilder() {
        return super.toVtnFlowMatchBuilder().setIndex(index);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public XmlNode toXmlNode(String name) {
        XmlNode root = super.toXmlNode(name);
        if (index != null) {
            root.add(new XmlNode("index", index));
        }

        return root;
    }

    // Cloneable

    /**
     * Return a deep copy of this instance.
     *
     * @return  A deep copy of this instance.
     */
    @Override
    public FlowMatchParams clone() {
        return (FlowMatchParams)super.clone();
    }
}

