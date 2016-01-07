/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.util.flow.cond;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;

import static org.opendaylight.vtn.manager.it.util.ModelDrivenTestBase.getRpcOutput;
import static org.opendaylight.vtn.manager.it.util.ModelDrivenTestBase.getRpcResult;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.RemoveFlowConditionMatchInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.RemoveFlowConditionMatchInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.RemoveFlowConditionMatchOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.SetFlowConditionInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.SetFlowConditionInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.SetFlowConditionMatchInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.SetFlowConditionMatchInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.SetFlowConditionMatchOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.VtnFlowConditionService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.VtnFlowMatchResult;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.set.flow.condition.match.input.FlowMatchList;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.vtn.flow.cond.config.VtnFlowMatch;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.vtn.flow.conditions.VtnFlowCondition;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeName;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateOperationType;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;

/**
 * {@code FlowCondition} describes configuration for a flow condition.
 */
public final class FlowCondition {
    /**
     * The name of the flow condition.
     */
    private String  name;

    /**
     * A map that keeps flow matches.
     */
    private final Map<Integer, FlowMatch>  flowMatches = new HashMap<>();

    /**
     * Create a set-flow-condition-match input.
     *
     * @param name     The name of the target flow condition.
     * @param matches  An array of flow matches.
     * @return  A {@link SetFlowConditionMatchInput} instance.
     */
    public static SetFlowConditionMatchInput newSetMatchInput(
        String name, FlowMatch ... matches) {
        List<FlowMatch> list = (matches == null)
            ? null : Arrays.asList(matches);
        return newSetMatchInput(name, list);
    }

    /**
     * Create a set-flow-condition-match input.
     *
     * @param name     The name of the target flow condition.
     * @param matches  A collection of flow matches.
     * @return  A {@link SetFlowConditionMatchInput} instance.
     */
    public static SetFlowConditionMatchInput newSetMatchInput(
        String name, Collection<FlowMatch> matches) {
        List<FlowMatchList> fml;
        if (matches == null) {
            fml = null;
        } else {
            fml = new ArrayList<>(matches.size());
            for (FlowMatch fm: matches) {
                fml.add(fm.toFlowMatchList());
            }
        }

        return new SetFlowConditionMatchInputBuilder().
            setName(name).
            setFlowMatchList(fml).
            build();
    }

    /**
     * Create a map that indicates the given result of flow match RPC.
     *
     * @param list  A list of {@link VtnFlowMatchResult} instances.
     * @param <T>   The type of the RPC result.
     * @return  A map that indicates the given result of flow match RPC.
     *          Note that {@code null} is returned if the given list is
     *          {@code null}.
     */
    private static <T extends VtnFlowMatchResult> Map<Integer, VtnUpdateType> getResultMap(
        List<T> list) {
        Map<Integer, VtnUpdateType> result;
        if (list == null) {
            result = null;
        } else {
            result = new HashMap<>();
            for (VtnFlowMatchResult fmres: list) {
                Integer index = fmres.getIndex();
                VtnUpdateType status = fmres.getStatus();
                assertEquals(null, result.put(index, status));
            }
        }

        return result;
    }

    /**
     * Construct a new instance without specifying flow match.
     *
     * @param nm  The name of the flow condition.
     */
    public FlowCondition(String nm) {
        name = nm;
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
     * Add the given flow matches to this flow condition.
     *
     * @param fms  An array of {@link FlowMatch} instances.
     * @return  This instance.
     */
    public FlowCondition add(FlowMatch ... fms) {
        for (FlowMatch fm: fms) {
            Integer idx = fm.getIndex();
            flowMatches.put(idx, fm);
        }
        return this;
    }

    /**
     * Remove the given flow matches from this instance.
     *
     * @param fms  An array of {@link FlowMatch} instances.
     * @return  This instance.
     */
    public FlowCondition remove(FlowMatch ... fms) {
        for (FlowMatch fm: fms) {
            flowMatches.remove(fm.getIndex());
        }
        return this;
    }

    /**
     * Remove flow matches specifies by the given indices.
     *
     * @param indices  Indices that specifies flow matches to be removed.
     * @return  This instance.
     */
    public FlowCondition remove(Integer ... indices) {
        for (Integer idx: indices) {
            flowMatches.remove(idx);
        }
        return this;
    }

    /**
     * Remove all the flow matches from this instance.
     *
     * @return  This instance.
     */
    public FlowCondition clear() {
        flowMatches.clear();
        return this;
    }

    /**
     * Return the flow match specified by the given match index.
     *
     * @param index  The index value that specifies the flow match.
     * @return  A {@link FlowMatch} instance if found.
     *          {@code null} if not found.
     */
    public FlowMatch get(Integer index) {
        return flowMatches.get(index);
    }

    /**
     * Create a set-flow-condition-match input.
     *
     * @param matches  An array of flow matches.
     * @return  A {@link SetFlowConditionMatchInput} instance.
     */
    public SetFlowConditionMatchInput newSetMatchInput(FlowMatch ... matches) {
        return newSetMatchInput(name, matches);
    }

    /**
     * Create a set-flow-condition-match input.
     *
     * @param matches  A collection of flow matches.
     * @return  A {@link SetFlowConditionMatchInput} instance.
     */
    public SetFlowConditionMatchInput newSetMatchInput(
        Collection<FlowMatch> matches) {
        return newSetMatchInput(name, matches);
    }

    /**
     * Create a new input for set-flow-condition RPC.
     *
     * @param op       A {@link VtnUpdateOperationType} instance.
     * @param present  If {@code true}, the operation will fail unless the
     *                 specified flow condition is present.
     * @return  A {@link SetFlowConditionInput} instance.
     */
    public SetFlowConditionInput newInput(VtnUpdateOperationType op,
                                          Boolean present) {
        VnodeName vname = (name == null) ? null : new VnodeName(name);
        List<VtnFlowMatch> matches;
        if (flowMatches.isEmpty()) {
            matches = null;
        } else {
            matches = new ArrayList<>(flowMatches.size());
            for (FlowMatch fm: flowMatches.values()) {
                matches.add(fm.toVtnFlowMatch());
            }
        }

        return new SetFlowConditionInputBuilder().
            setName(vname).
            setVtnFlowMatch(matches).
            setOperation(op).
            setPresent(present).
            build();
    }

    /**
     * Update the flow condition.
     *
     * @param service  The vtn-flow-condition service.
     * @param op       A {@link VtnUpdateOperationType} instance.
     * @param present  If {@code true}, the operation will fail unless the
     *                 specified flow condition is present.
     * @return  A {@link VtnUpdateType} instance returned by the RPC.
     */
    public VtnUpdateType update(VtnFlowConditionService service,
                                VtnUpdateOperationType op, Boolean present) {
        SetFlowConditionInput input = newInput(op, present);
        return getRpcResult(service.setFlowCondition(input));
    }

    /**
     * Add the given flow match to the flow condition using
     * set-flow-condition-match RPC, and then add it to this instance.
     *
     * @param service  The vtn-flow-condition service.
     * @param fm       A {@link FlowMatch} instance to be added.
     * @return  A {@link VtnUpdateType} instance that indicates the result
     *          of RPC.
     */
    public VtnUpdateType addMatch(VtnFlowConditionService service,
                                  FlowMatch fm) {
        Map<Integer, VtnUpdateType> result =
            addMatches(service, Collections.singletonList(fm));
        Integer index = fm.getIndex();
        assertEquals(Collections.singleton(index), result.keySet());
        return result.get(index);
    }

    /**
     * Add the given flow matches to the flow condition using
     * set-flow-condition-match RPC, and then add them to this instance.
     *
     * @param service  The vtn-flow-condition service.
     * @param matches  An array of flow matches to be added.
     * @return  A map that contains the RPC result.
     */
    public Map<Integer, VtnUpdateType> addMatches(
        VtnFlowConditionService service, FlowMatch ... matches) {
        List<FlowMatch> list = (matches == null)
            ? null : Arrays.asList(matches);
        return addMatches(service, list);
    }

    /**
     * Add the given flow matches to the flow condition using
     * set-flow-condition-match RPC, and then add them to this instance.
     *
     * @param service  The vtn-flow-condition service.
     * @param matches  A collection of flow matches to be added.
     * @return  A map that contains the RPC result.
     */
    public Map<Integer, VtnUpdateType> addMatches(
        VtnFlowConditionService service, Collection<FlowMatch> matches) {
        SetFlowConditionMatchInput input = newSetMatchInput(name, matches);
        SetFlowConditionMatchOutput output =
            getRpcOutput(service.setFlowConditionMatch(input));
        Map<Integer, VtnUpdateType> result =
            getResultMap(output.getSetMatchResult());

        if (matches != null) {
            for (FlowMatch fm: matches) {
                Integer idx = fm.getIndex();
                flowMatches.put(idx, fm);
            }
        }

        return result;
    }

    /**
     * Remove the given flow match from the flow condition using
     * remove-flow-condition-match RPC, and then remove it from this instance.
     *
     * @param service  The vtn-flow-condition service.
     * @param fm       A {@link FlowMatch} instance to be removed.
     * @return  A {@link VtnUpdateType} instance that indicates the result
     *          of RPC.
     */
    public VtnUpdateType removeMatch(VtnFlowConditionService service,
                                     FlowMatch fm) {
        return removeMatch(service, fm.getIndex());
    }

    /**
     * Remove the given flow match from the flow condition using
     * remove-flow-condition-match RPC, and then remove it from this instance.
     *
     * @param service  The vtn-flow-condition service.
     * @param index    The flow match index to be removed.
     * @return  A {@link VtnUpdateType} instance that indicates the result
     *          of RPC.
     */
    public VtnUpdateType removeMatch(VtnFlowConditionService service,
                                     Integer index) {
        Map<Integer, VtnUpdateType> result =
            removeMatches(service, Collections.singletonList(index));
        assertEquals(Collections.singleton(index), result.keySet());
        return result.get(index);
    }

    /**
     * Remove the given flow matches from the flow condition using
     * remove-flow-condition-match RPC, and then remove them from this
     * instance.
     *
     * @param service  The vtn-flow-condition service.
     * @param matches  An array of flow matches to be removed.
     * @return  A map that contains the RPC result.
     */
    public Map<Integer, VtnUpdateType> removeMatches(
        VtnFlowConditionService service, FlowMatch ... matches) {
        List<Integer> list;
        if (matches == null) {
            list = null;
        } else {
            list = new ArrayList<>(matches.length);
            for (FlowMatch fm: matches) {
                list.add(fm.getIndex());
            }
        }

        return removeMatches(service, list);
    }

    /**
     * Remove the given flow matches from the flow condition using
     * remove-flow-condition-match RPC, and then remove them from this
     * instance.
     *
     * @param service  The vtn-flow-condition service.
     * @param matches  A collection of flow matches to be removed.
     * @return  A map that contains the RPC result.
     */
    public Map<Integer, VtnUpdateType> removeMatches(
        VtnFlowConditionService service, Collection<FlowMatch> matches) {
        List<Integer> list;
        if (matches == null) {
            list = null;
        } else {
            list = new ArrayList<>(matches.size());
            for (FlowMatch fm: matches) {
                list.add(fm.getIndex());
            }
        }

        return removeMatches(service, list);
    }

    /**
     * Remove the given flow matches from the flow condition using
     * remove-flow-condition-match RPC, and then remove them from this
     * instance.
     *
     * @param service  The vtn-flow-condition service.
     * @param indices  An array of flow match indices to be removed.
     * @return  A map that contains the RPC result.
     */
    public Map<Integer, VtnUpdateType> removeMatches(
        VtnFlowConditionService service, Integer ... indices) {
        List<Integer> list = (indices == null) ? null : Arrays.asList(indices);
        return removeMatches(service, list);
    }

    /**
     * Remove the given flow matches from the flow condition using
     * remove-flow-condition-match RPC, and then remove them from this
     * instance.
     *
     * @param service  The vtn-flow-condition service.
     * @param indices  A list of flow match indices to be removed.
     * @return  A map that contains the RPC result.
     */
    public Map<Integer, VtnUpdateType> removeMatches(
        VtnFlowConditionService service, List<Integer> indices) {
        RemoveFlowConditionMatchInput input =
            new RemoveFlowConditionMatchInputBuilder().
            setName(name).
            setMatchIndex(indices).
            build();
        RemoveFlowConditionMatchOutput output =
            getRpcOutput(service.removeFlowConditionMatch(input));
        Map<Integer, VtnUpdateType> result =
            getResultMap(output.getRemoveMatchResult());

        if (indices != null) {
            for (Integer index: indices) {
                flowMatches.remove(index);
            }
        }

        return result;
    }

    /**
     * Restore all the flow matches in this instance using
     * set-flow-condition-match RPC.
     *
     * <p>
     *   This method expects that all the following conditions are met.
     * </p>
     * <ul>
     *   <li>
     *     This instance contains at least one flow match.
     *   </li>
     *   <li>
     *     The flow condition associated with this instance is present.
     *   </li>
     *   <li>
     *     All the flow matches in this instance are not configured yet.
     *   </li>
     * </ul>
     *
     * @param service  The vtn-path-map service.
     */
    public void restore(VtnFlowConditionService service) {
        assertEquals(false, flowMatches.isEmpty());

        Map<Integer, VtnUpdateType> expected = new HashMap<>();
        for (Integer idx: flowMatches.keySet()) {
            assertEquals(null, expected.put(idx, VtnUpdateType.CREATED));
        }

        SetFlowConditionMatchInput input =
            newSetMatchInput(flowMatches.values());
        SetFlowConditionMatchOutput output =
            getRpcOutput(service.setFlowConditionMatch(input));
        assertEquals(expected, getResultMap(output.getSetMatchResult()));
    }

    /**
     *  Verify the given flow condition.
     *
     * @param vfc  A {@link VtnFlowCondition} instance.
     */
    public void verify(VtnFlowCondition vfc) {
        assertEquals(name, vfc.getName().getValue());

        List<VtnFlowMatch> vfms = vfc.getVtnFlowMatch();
        if (!flowMatches.isEmpty()) {
            assertNotNull("Flow match is empty: name=" + name, vfms);
            Set<Integer> checked = new HashSet<>();
            for (VtnFlowMatch vfm: vfms) {
                Integer idx = vfm.getIndex();
                FlowMatch fmatch = flowMatches.get(idx);
                assertNotNull("No flow match: name=" + name + ", index=" + idx,
                              fmatch);
                fmatch.verify(vfm);
                assertEquals(true, checked.add(idx));
            }
            assertEquals(checked, flowMatches.keySet());
        } else if (vfms != null) {
            assertEquals(Collections.<VtnFlowMatch>emptyList(), vfms);
        }
    }
}
