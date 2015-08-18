/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.flow.cond;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.TreeMap;

import org.opendaylight.vtn.manager.flow.cond.FlowCondition;
import org.opendaylight.vtn.manager.flow.cond.FlowMatch;

import org.opendaylight.vtn.manager.internal.TestBase;
import org.opendaylight.vtn.manager.internal.XmlNode;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.SetFlowConditionInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.VtnFlowConditions;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.vtn.flow.cond.config.VtnFlowMatch;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.vtn.flow.conditions.VtnFlowCondition;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.vtn.flow.conditions.VtnFlowConditionBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.vtn.flow.conditions.VtnFlowConditionKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeName;

/**
 * {@code FlowCondParams} describes parameters for a flow condition.
 */
public final class FlowCondParams extends TestBase implements Cloneable {
    /**
     * A map that contains {@link FlowCondParams} instances for test.
     */
    private static final Map<FlowCondParams, FlowCondParams> COND_PARAMS;

    /**
     * The name of the flow condition.
     */
    private String  name;

    /**
     * A list of {@link FlowMatchParams} instances.
     */
    private List<FlowMatchParams>  matchParams;

    /**
     * Initialize static fields.
     */
    static {
        Map<FlowCondParams, FlowCondParams> map = new HashMap<>();
        Map<FlowMatchParams, FlowMatchParams> matches =
            FlowMatchParams.createFlowMatches();
        LinkedList<FlowMatchParams> paramList = new LinkedList<>();
        List<FlowMatchParams> expectedList = new ArrayList<>(matches.size());
        int count = 0;
        for (Map.Entry<FlowMatchParams, FlowMatchParams> entry:
                 matches.entrySet()) {
            FlowMatchParams params = entry.getKey();
            FlowMatchParams expected = entry.getValue();
            String name = "fcond" + count;
            paramList.addFirst(params);
            expectedList.add(expected);
            map.put(new FlowCondParams(name, paramList),
                    new FlowCondParams(name, expectedList));
            count++;
        }

        String name = "empty";
        FlowCondParams empty = new FlowCondParams(name);
        map.put(empty, empty);

        COND_PARAMS = map;
    }

    /**
     * Create a map that contains {@link FlowCondParams} instances for test.
     *
     * <p>
     *   An instance of {@link FlowCondParams} to be used to construct
     *   {@link VTNFlowCondition} instance is set as a key, and an instance of
     *   {@link FlowCondParams} instances that contains conditions expected to
     *   be configured in {@link VTNFlowCondition} is set as a value.
     * </p>
     *
     * @return  A map that contains {@link FlowCondParams} instances.
     */
    public static Map<FlowCondParams, FlowCondParams> createFlowConditions() {
        Map<FlowCondParams, FlowCondParams> map =
            new HashMap<>(COND_PARAMS.size());
        for (Map.Entry<FlowCondParams, FlowCondParams> entry:
                 COND_PARAMS.entrySet()) {
            FlowCondParams params = entry.getKey();
            FlowCondParams expected = entry.getValue();
            map.put(params.clone(), expected.clone());
        }

        return map;
    }

    /**
     * Construct a new instance.
     *
     * @param nm       The name of the flow condition.
     */
    public FlowCondParams(String nm) {
        name = nm;
    }

    /**
     * Construct a new instance.
     *
     * @param nm       The name of the flow condition.
     * @param matches  A list of {@link FlowMatchParams} instances.
     */
    public FlowCondParams(String nm, List<FlowMatchParams> matches) {
        name = nm;
        if (matches != null) {
            matchParams = new ArrayList<FlowMatchParams>(matches);
        }
    }

    /**
     * Return the name of the flow condition.
     *
     * @return  The name of the flow condition.
     */
    public String getName() {
        return name;
    }

    /**
     * Set the name of the flow condition.
     *
     * @param nm  The name of the flow condition.
     * @return  This instance.
     */
    public FlowCondParams setName(String nm) {
        name = nm;
        return this;
    }

    /**
     * Add the given {@link FlowMatchParams}.
     *
     * @param fmp  A {@link FlowMatchParams} instance.
     * @return  This instance.
     */
    public FlowCondParams addMatch(FlowMatchParams fmp) {
        if (matchParams == null) {
            matchParams = new ArrayList<FlowMatchParams>();
        }
        matchParams.add(fmp);
        return this;
    }

    /**
     * Construct a new {@link VTNFlowCondition} instance.
     *
     * @return  A {@link VTNFlowCondition} instance.
     * @throws Exception  An error occurred.
     */
    public VTNFlowCondition toVTNFlowCondition() throws Exception {
        return new VTNFlowCondition(name, toFlowCondition());
    }

    /**
     * Return the path to the flow condition in the MD-SAL datastore.
     *
     * @return  An {@link InstanceIdentifier} instance.
     */
    public InstanceIdentifier<VtnFlowCondition> getPath() {
        VtnFlowConditionKey key = new VtnFlowConditionKey(new VnodeName(name));
        return InstanceIdentifier.builder(VtnFlowConditions.class).
            child(VtnFlowCondition.class, key).build();
    }

    /**
     * Return a list of {@link VtnFlowMatch} instances.
     *
     * @return  A list of {@link FlowMatch} instances.
     */
    public List<FlowMatch> getFlowMatchList() {
        if (matchParams == null) {
            return null;
        }

        Map<Integer, FlowMatch> map = new TreeMap<>();
        for (FlowMatchParams params: matchParams) {
            FlowMatch fm = params.toFlowMatch();
            Integer index = fm.getIndex();
            assertEquals(null, map.put(index, fm));
        }

        return new ArrayList<FlowMatch>(map.values());
    }

    /**
     * Return a list of {@link FlowMatch} instances.
     *
     * @return  A list of {@link VtnFlowMatch} instances.
     */
    public List<VtnFlowMatch> getVtnFlowMatchList() {
        if (matchParams == null) {
            return null;
        }

        List<VtnFlowMatch> list = new ArrayList<>(matchParams.size());
        for (FlowMatchParams params: matchParams) {
            list.add(params.toVtnFlowMatchBuilder().build());
        }

        return list;
    }

    /**
     * Construct a new {@link FlowCondition} instance.
     *
     * @return  A {@link FlowCondition} instance.
     */
    public FlowCondition toFlowCondition() {
        return new FlowCondition(name, getFlowMatchList());
    }

    /**
     * Construct a new {@link VtnFlowConditionBuilder} instance.
     *
     * @return  A {@link VtnFlowConditionBuilder} instance.
     */
    public VtnFlowConditionBuilder toVtnFlowConditionBuilder() {
        return new VtnFlowConditionBuilder().
            setName(new VnodeName(name)).
            setVtnFlowMatch(getVtnFlowMatchList());
    }

    /**
     * Return a {@link XmlNode} instances which represents this instance.
     *
     * @param nm  The name of the root node.
     * @return  A {@link XmlNode} instance.
     */
    public XmlNode toXmlNode(String nm) {
        XmlNode root = new XmlNode(nm);
        if (name != null) {
            root.add(new XmlNode("name", name));
        }
        if (matchParams != null) {
            XmlNode list = new XmlNode("vtn-flow-matches");
            for (FlowMatchParams params: matchParams) {
                list.add(params.toXmlNode("vtn-flow-match"));
            }
            root.add(list);
        }

        return root;
    }

    /**
     * Ensure that the given {@link VTNFlowCondition} instance contains the
     * same conditions as this instance.
     *
     * @param vfcond  A {@link VTNFlowCondition} instance.
     * @throws Exception  An error occurred.
     */
    public void verify(VTNFlowCondition vfcond) throws Exception {
        FlowCondition fc = vfcond.toFlowCondition();
        assertEquals(toFlowCondition(), fc);
        assertEquals(getPath(), vfcond.getPath());

        int prev = 0;
        for (FlowMatch fm: fc.getMatches()) {
            int idx = fm.getIndex().intValue();
            assertTrue(idx > prev);
            prev = idx;
        }

        VtnFlowCondition vfc = toVtnFlowConditionBuilder().build();
        assertEquals(vfcond, VTNFlowCondition.create(vfc));

        SetFlowConditionInput input =
            vfcond.toSetFlowConditionInputBuilder().build();
        assertEquals(vfcond, new VTNFlowCondition(input));
    }

    // Cloneable

    /**
     * Return a deep copy of this instance.
     *
     * @return  A deep copy of this instance.
     */
    @Override
    public FlowCondParams clone() {
        try {
            FlowCondParams params = (FlowCondParams)super.clone();
            if (matchParams != null) {
                List<FlowMatchParams> list =
                    new ArrayList<>(matchParams.size());
                for (FlowMatchParams fmp: matchParams) {
                    list.add(fmp.clone());
                }
                params.matchParams = list;
            }

            return params;
        } catch (CloneNotSupportedException e) {
            // This should never happen.
            throw new IllegalStateException("clone() failed", e);
        }
    }
}
