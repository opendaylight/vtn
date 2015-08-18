/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashSet;
import java.util.List;
import java.util.Random;

import org.junit.Test;

import org.opendaylight.vtn.manager.internal.TestBase;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.flow.action.list.VtnFlowActionBuilder;

import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.Ordered;

/**
 * JUnit test for {@link OrderedComparator}.
 */
public class OrderedComparatorTest extends TestBase {
    /**
     * Test case for {@link OrderedComparator#compare(Ordered, Ordered)}.
     */
    @Test
    public void testCompare() {
        Random rand = new Random();
        List<Ordered> list = new ArrayList<>();
        for (int i = 0; i < 100; i++) {
            Integer order = rand.nextInt();
            list.add(new VtnFlowActionBuilder().setOrder(order).build());
        }

        Collections.sort(list, new OrderedComparator());
        Integer prev = null;
        for (Ordered data: list) {
            Integer order = data.getOrder();
            int value = order.intValue();
            if (prev != null) {
                assertTrue(prev.intValue() <= value);
            }
            prev = order;
        }
    }

    /**
     * Test case for {@link OrderedComparator#equals(Object)} and
     * {@link OrderedComparator#hashCode()}.
     */
    @Test
    public void testEquals() {
        HashSet<Object> set = new HashSet<>();
        OrderedComparator comp = new OrderedComparator();
        testEquals(set, comp, new OrderedComparator());
        assertEquals(true, set.contains(new OrderedComparator()));
        assertEquals(false, set.contains(new VtnIndexComparator()));
        assertEquals(1, set.size());
    }
}
