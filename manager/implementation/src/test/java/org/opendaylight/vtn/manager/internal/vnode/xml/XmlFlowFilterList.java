/*
 * Copyright (c) 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.vnode.xml;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.fail;

import static org.opendaylight.vtn.manager.internal.TestBase.MAX_RANDOM;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashSet;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;
import java.util.Random;
import java.util.Set;

import com.google.common.collect.ImmutableList;

import org.opendaylight.vtn.manager.internal.util.flow.filter.FlowFilterList;
import org.opendaylight.vtn.manager.internal.util.flow.filter.VTNFlowFilter;

import org.opendaylight.vtn.manager.internal.XmlNode;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.VtnFlowFilterList;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.flow.filter.list.VtnFlowFilter;

/**
 * {@code XmlFlowFilterList} describes a list of flow filters.
 */
public final class XmlFlowFilterList implements Cloneable {
    /**
     * A set of supported flow filters.
     */
    private static final List<Class<? extends XmlFlowFilter<?>>>  FILTER_TYPES;

    /**
     * Initialize static field.
     */
    static {
        FILTER_TYPES =
            ImmutableList.<Class<? extends XmlFlowFilter<?>>>builder().
            add(XmlPassFilter.class).
            add(XmlDropFilter.class).
            add(XmlRedirectFilter.class).
            build();
    }

    /**
     * A list of flow filters indexed by filter index.
     */
    private Map<Integer, XmlFlowFilter<?>>  flowFilters =
        new LinkedHashMap<>();

    /**
     * Verify the given flow filter list.
     *
     * @param expected  A {@link XmlFlowFilterList} instance that contains the
     *                  expected flow filter configuration.
     * @param ffl       A {@link FlowFilterList} instance to be verified.
     * @param jaxb  {@code true} indicates {@code ffl} was deserialized from
     *              XML.
     */
    public static void verify(XmlFlowFilterList expected, FlowFilterList ffl,
                              boolean jaxb) {
        if (expected == null) {
            assertEquals(null, ffl);
        } else {
            expected.verify(ffl, jaxb);
        }
    }

    /**
     * Construct an empty instance.
     */
    public XmlFlowFilterList() {
    }

    /**
     * Construct a new instance using the given pseudo random number generator.
     *
     * @param rand  A pseudo random number generator.
     */
    public XmlFlowFilterList(Random rand) {
        add(rand);
    }

    /**
     * Convert this instance into a list of vtn-flow-filter instances.
     *
     * @return  A list of vtn-flow-filter or {@code null}.
     */
    public List<VtnFlowFilter> toVtnFlowFilterList() {
        List<VtnFlowFilter> filters;
        if (flowFilters.isEmpty()) {
            filters = null;
        } else {
            filters = new ArrayList<>(flowFilters.size());
            for (XmlFlowFilter<?> xff: flowFilters.values()) {
                filters.add(xff.toVtnFlowFilter());
            }
        }

        return filters;
    }

    /**
     * Set flow filters in this instance into the specified XML node.
     *
     * @param xnode   A {@link XmlNode} instance.
     * @param output  {@code true} indicates the output filter.
     *                {@code false} indicates the input filter.
     */
    public void setXml(XmlNode xnode, boolean output) {
        String root = (output) ? "output-filters" : "input-filters";
        XmlNode xfilters = new XmlNode(root);
        for (XmlFlowFilter<?> xff: flowFilters.values()) {
            xfilters.add(xff.toXmlNode());
        }
        xnode.add(xfilters);
    }

    /**
     * Add the specified flow filter.
     *
     * @param xff  A flow filter to add.
     * @return  This instance.
     */
    public XmlFlowFilterList add(XmlFlowFilter<?> xff) {
        Integer index = xff.getIndex();
        assertEquals(null, flowFilters.put(index, xff));
        return this;
    }

    /**
     * Add random flow filters using the given pseudo random number generator.
     *
     * @param rand   A pseudo random number generator.
     * @return  This instance.
     */
    public XmlFlowFilterList add(Random rand) {
        int i = rand.nextInt(4);
        if (i == 1) {
            // All the supported flow filters.
            addAll(rand);
        } else if (i != 0) {
            addRandom(rand);
        }
        return this;
    }

    /**
     * Add all of the flow filters supported by flow filter.
     *
     * @param rand  A pseudo random number generator.
     * @return  This instance.
     */
    public XmlFlowFilterList addAll(Random rand) {
        Set<Integer> indices = new HashSet<>(flowFilters.keySet());
        for (Class<? extends XmlFlowFilter<?>> type: FILTER_TYPES) {
            add(create(rand, indices, type));
        }
        return this;
    }

    /**
     * Add random flow filters using the given pseudo random number generator.
     *
     * @param rand   A pseudo random number generator.
     * @return  This instance.
     */
    public XmlFlowFilterList addRandom(Random rand) {
        return addRandom(rand, rand.nextInt(MAX_RANDOM));
    }

    /**
     * Add the given number of flow filters using the given random number
     * generator.
     *
     * @param rand   A pseudo random number generator.
     * @param count  The number of flow filters to add.
     * @return  This instance.
     */
    public XmlFlowFilterList addRandom(Random rand, int count) {
        if (count > 0) {
            Set<Integer> indices = new HashSet<>(flowFilters.keySet());
            for (int i = 0; i < count; i++) {
                // Determine filter type.
                int act = rand.nextInt(FILTER_TYPES.size());
                Class<? extends XmlFlowFilter<?>> type = FILTER_TYPES.get(act);
                add(create(rand, indices, type));
            }
        }
        return this;
    }

    /**
     * Return all the flow filter configurations in this instance.
     *
     * @return  A list of {@link XmlFlowFilter} instances.
     */
    public List<XmlFlowFilter<?>> getAll() {
        return new ArrayList<>(flowFilters.values());
    }

    /**
     * Determine whether the flow filter list is empty or not.
     *
     * @return  {@code true} if the flow filter list is empty.
     *          {@code false} otherwise.
     */
    public boolean isEmpty() {
        return flowFilters.isEmpty();
    }

    /**
     * Ensure that the given vtn-flow-filter list is identical to this
     * instance.
     *
     * <p>
     *   This method expects that the flow filters in the given list are sorted
     *   by filter index.
     * </p>
     *
     * @param vffl  A {@link VtnFlowFilterList} instance.
     */
    public void verify(VtnFlowFilterList vffl) {
        if (flowFilters.isEmpty()) {
            assertEquals(null, vffl);
        } else {
            assertNotNull(vffl);
            List<VtnFlowFilter> filters = vffl.getVtnFlowFilter();
            Set<Integer> checked = new HashSet<>();
            Integer prev = null;
            for (VtnFlowFilter vff: filters) {
                Integer index = vff.getIndex();
                assertEquals(true, checked.add(index));
                flowFilters.get(index).verify(vff);

                if (prev == null) {
                    prev = index;
                } else if (prev.intValue() >= index.intValue()) {
                    fail("Invalid filter index: prev=" + prev + ", index=" +
                         index);
                }
            }
            assertEquals(flowFilters.keySet(), checked);
        }
    }

    /**
     * Ensure that the given {@link FlowFilterList} is identical to this
     * instance.
     *
     * @param ffl   A {@link FlowFilterList} instance.
     * @param jaxb  {@code true} indicates {@code ffl} was deserialized from
     *              XML.
     */
    public void verify(FlowFilterList ffl, boolean jaxb) {
        if (flowFilters.isEmpty()) {
            if (jaxb) {
                assertNotNull(ffl);
                assertEquals(Collections.<VTNFlowFilter>emptyList(),
                             ffl.getFlowFilters());
            } else {
                assertEquals(null, ffl);
            }
        } else {
            assertNotNull(ffl);
            List<VTNFlowFilter> filters = ffl.getFlowFilters();
            assertNotNull(filters);
            Set<Integer> checked = new HashSet<>();
            for (VTNFlowFilter ff: filters) {
                Integer index = ff.getIdentifier();
                assertEquals(true, checked.add(index));
                flowFilters.get(index).verify(ff);
            }
            assertEquals(flowFilters.keySet(), checked);
        }
    }

    /**
     * Create a new flow filter using the given random generator.
     *
     * @param rand     A pseudo random generator.
     * @param indices  A set of filter index values to store generated values.
     * @param type     A class that specifies the type of flow filter.
     * @param <F>      The type of flow filter.
     * @return  A new flow filter.
     */
    private <F extends XmlFlowFilter<?>> F create(
        Random rand, Set<Integer> indices, Class<F> type) {
        try {
            F ff = type.newInstance();
            ff.set(rand, indices);
            return ff;
        } catch (Exception e) {
            throw new IllegalStateException(
                "Failed to create flow filter: " + type.getSimpleName(), e);
        }
    }

    // Object

    /**
     * Return a shallow copy of this instance.
     *
     * @return  A shallow copy of this instance.
     */
    @Override
    public XmlFlowFilterList clone() {
        try {
            XmlFlowFilterList xffl = (XmlFlowFilterList)super.clone();
            xffl.flowFilters = new LinkedHashMap<>(flowFilters);
            return xffl;
        } catch (CloneNotSupportedException e) {
            // This should never happen.
            throw new IllegalStateException("clone() failed.", e);
        }
    }
}
