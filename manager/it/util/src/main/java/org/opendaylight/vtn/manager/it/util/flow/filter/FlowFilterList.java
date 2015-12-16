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

import static org.opendaylight.vtn.manager.it.util.ModelDrivenTestBase.getRpcOutput;
import static org.opendaylight.vtn.manager.it.util.TestBase.unexpected;

import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Random;
import java.util.Set;

import com.google.common.collect.ImmutableSet;

import org.opendaylight.vtn.manager.it.util.VTNServices;
import org.opendaylight.vtn.manager.it.util.vnode.VNodeIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.RemoveFlowFilterInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.RemoveFlowFilterInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.RemoveFlowFilterOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.SetFlowFilterInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.SetFlowFilterInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.VtnFlowFilterList;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.VtnFlowFilterService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.flow.filter.list.VtnFlowFilter;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.flow.filter.result.FlowFilterResult;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;

/**
 * {@code FlowFilterList} describes an ordered list of flow filter
 * configurations.
 */
public final class FlowFilterList {
    /**
     * A set of supported filter types.
     */
    private static final Set<Class<? extends FlowFilter>>  FILTER_TYPES;

    /**
     * Flow filters indexed by index value.
     */
    private final Map<Integer, FlowFilter<?>>  flowFilters = new HashMap<>();

    /**
     * Initialize static field.
     */
    static {
        FILTER_TYPES = ImmutableSet.<Class<? extends FlowFilter>>builder().
            add(PassFilter.class).
            add(DropFilter.class).
            add(RedirectFilter.class).
            build();
    }

    /**
     * Remove all the flow filters in the specified flow filter list.
     *
     * @param service  The vtn-flow-filter RPC service.
     * @param ident    The identifier for the virtual node.
     * @param out      A boolean value that determines the packet direction.
     * @return  A map that specifies the removed flow filters.
     */
    public static Map<Integer, VtnUpdateType> removeFlowFilter(
        VtnFlowFilterService service, VNodeIdentifier<?> ident, boolean out) {
        return removeFlowFilter(service, ident, out, null);
    }

    /**
     * Remove the specified flow filters in the specified flow filter list.
     *
     * @param service  The vtn-flow-filter RPC service.
     * @param ident    The identifier for the virtual node.
     * @param out      A boolean value that determines the packet direction.
     * @param indices  A list of flow filter indices to be removed.
     * @return  A map that specifies the removed flow filters.
     */
    public static Map<Integer, VtnUpdateType> removeFlowFilter(
        VtnFlowFilterService service, VNodeIdentifier<?> ident, boolean out,
        List<Integer> indices) {
        RemoveFlowFilterInputBuilder builder =
            new RemoveFlowFilterInputBuilder().
            setOutput(out).
            setIndices(indices);
        builder.fieldsFrom(ident.getVirtualNodePath());

        RemoveFlowFilterInput input = builder.build();
        RemoveFlowFilterOutput output =
            getRpcOutput(service.removeFlowFilter(input));
        List<FlowFilterResult> removed = output.getFlowFilterResult();
        Map<Integer, VtnUpdateType> result = new HashMap<>();
        if (removed != null) {
            for (FlowFilterResult fres: removed) {
                Integer index = fres.getIndex();
                VtnUpdateType status = fres.getStatus();
                assertEquals(null, result.put(index, status));
            }
        }

        return result;
    }

    /**
     * Add random flow filters using the given random generator.
     *
     * @param rand  A pseudo random generator.
     * @return  This instance.
     */
    public FlowFilterList add(Random rand) {
        Set<Integer> indices = new HashSet<>(flowFilters.keySet());
        for (Class<? extends FlowFilter> type: FILTER_TYPES) {
            add(rand, indices, type);
        }

        return this;
    }

    /**
     * Add the given flow filter to this list.
     *
     * @param fact  A {@link FlowFilter} instance.
     * @return  This instance.
     */
    public FlowFilterList add(FlowFilter<?> fact) {
        flowFilters.put(fact.getIndex(), fact);
        return this;
    }

    /**
     * Remove the flow filter specified by the filter index.
     *
     * @param index  The index of flow filter.
     * @return  This instance.
     */
    public FlowFilterList remove(Integer index) {
        flowFilters.remove(index);
        return this;
    }

    /**
     * Remove all the flow filters in this list.
     *
     * @return  This instance.
     */
    public FlowFilterList clear() {
        flowFilters.clear();
        return this;
    }

    /**
     * Return an unmodifiable collection of flow filter configurations.
     *
     * @return  An unmodifiable collection of {@link FlowFilter} instances.
     */
    public Collection<FlowFilter<?>> getFlowFilters() {
        return Collections.unmodifiableCollection(flowFilters.values());
    }

    /**
     * Convert this instance into a list of {@link VtnFlowFilter} instances.
     *
     * @return  A list of {@link VtnFlowFilter} instances or {@code null}.
     */
    public List<VtnFlowFilter> toVtnFlowFilterList() {
        return toVtnFlowFilterList(false);
    }

    /**
     * Convert this instance into a list of {@link VtnFlowFilter} instances.
     *
     * @param empty  If {@code true}, this method returns an empty list
     *               if the filter list is empty.
     *               If {@code false}, this method returns {@code null}
     *               if the filter list is empty.
     * @return  A list of {@link VtnFlowFilter} instances or {@code null}.
     */
    public List<VtnFlowFilter> toVtnFlowFilterList(boolean empty) {
        List<VtnFlowFilter> vfilters;
        if (!flowFilters.isEmpty()) {
            vfilters = new ArrayList<>(flowFilters.size());
            for (FlowFilter<?> fact: flowFilters.values()) {
                vfilters.add(fact.toVtnFlowFilter());
            }
        } else if (empty) {
            vfilters = Collections.<VtnFlowFilter>emptyList();
        } else {
            vfilters = null;
        }

        return vfilters;
    }

    /**
     * Create a new input builder for set-flow-filter RPC.
     *
     * @return  A {@link SetFlowFilterInputBuilder} instance.
     */
    public SetFlowFilterInputBuilder newInputBuilder() {
        return new SetFlowFilterInputBuilder().
            setVtnFlowFilter(toVtnFlowFilterList());
    }

    /**
     * Verify the given list of flow filters.
     *
     * @param vflist  The container that contains a list of flow filters
     *                to be verified.
     */
    public void verify(VtnFlowFilterList vflist) {
        if (!flowFilters.isEmpty()) {
            assertNotNull(vflist);
            List<VtnFlowFilter> vfilters = vflist.getVtnFlowFilter();
            assertNotNull(vfilters);
            Set<Integer> checked = new HashSet<>();
            for (VtnFlowFilter vff: vfilters) {
                Integer index = vff.getIndex();
                FlowFilter<?> ff = flowFilters.get(index);
                assertNotNull(ff);
                ff.verify(vff);
                assertEquals(true, checked.add(index));
            }

            assertEquals(checked, flowFilters.keySet());
        } else if (vflist != null) {
            List<VtnFlowFilter> vfilters = vflist.getVtnFlowFilter();
            if (vfilters != null) {
                assertEquals(Collections.<VtnFlowFilter>emptyList(), vfilters);
            }
        }
    }

    /**
     * Apply all the configurations in this instance.
     *
     * @param service  A {@link VTNServices} instance.
     * @param ident    The identifier for the virtual node.
     * @param out      A boolean value that determines the packet direction.
     * @param vflist   The container that contains a list of flow filters
     *                 present in the specified virtual node.
     */
    public void apply(VTNServices service, VNodeIdentifier<?> ident,
                      boolean out, VtnFlowFilterList vflist) {
        VtnFlowFilterService vffSrv = service.getFlowFilterService();
        removeUnwanted(vffSrv, ident, out, vflist);

        if (!flowFilters.isEmpty()) {
            SetFlowFilterInputBuilder builder = newInputBuilder().
                setOutput(out);
            builder.fieldsFrom(ident.getVirtualNodePath());

            SetFlowFilterInput input = builder.build();
            getRpcOutput(vffSrv.setFlowFilter(input));
        }
    }

    /**
     * Remove all the flow filters that are not associated with the
     * configuration in this instance.
     *
     * @param service  The vtn-flow-filter RPC service.
     * @param ident    The identifier for the virtual node.
     * @param out      A boolean value that determines the packet direction.
     * @param vflist   The container that contains a list of flow filters
     *                 present in the specified virtual node.
     */
    private void removeUnwanted(
        VtnFlowFilterService service, VNodeIdentifier<?> ident, boolean out,
        VtnFlowFilterList vflist) {
        List<Integer> unwanted = new ArrayList<>();
        if (vflist != null) {
            List<VtnFlowFilter> vfilters = vflist.getVtnFlowFilter();
            if (vfilters != null) {
                for (VtnFlowFilter vff: vfilters) {
                    Integer index = vff.getIndex();
                    if (!flowFilters.containsKey(index)) {
                        unwanted.add(index);
                    }
                }
            }
        }

        if (!unwanted.isEmpty()) {
            Map<Integer, VtnUpdateType> result = (flowFilters.isEmpty())
                ? removeFlowFilter(service, ident, out)
                : removeFlowFilter(service, ident, out, unwanted);
            assertEquals(unwanted.size(), result.size());
            for (Integer index: unwanted) {
                assertEquals(VtnUpdateType.REMOVED, result.get(index));
            }
        }
    }

    /**
     * Add the specified flow filter using the given random generator.
     *
     * @param rand     A pseudo random generator.
     * @param indices  A set of filter indices to store generated values.
     * @param type     A class that specifies the type of flow filter.
     * @param <F>      The type of flow filter.
     */
    private <F extends FlowFilter<?>> void add(
        Random rand, Set<Integer> indices, Class<F> type) {
        if (rand.nextBoolean()) {
            try {
                F ff = type.newInstance();
                ff.set(rand, indices);
                add(ff);
            } catch (Exception e) {
                unexpected(e);
            }
        }
    }
}
