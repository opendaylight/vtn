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

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.vtn.flow.cond.config.VtnFlowMatchBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnIndex;

/**
 * JUnit test for {@link VtnIndexComparator}.
 */
public class VtnIndexComparatorTest extends TestBase {
    /**
     * Test case for {@link VtnIndexComparator#compare(VtnIndex, VtnIndex)}.
     */
    @Test
    public void testCompare() {
        Random rand = new Random();
        List<VtnIndex> list = new ArrayList<>();
        for (int i = 0; i < 100; i++) {
            Integer index = rand.nextInt(65535) + 1;
            list.add(new VtnFlowMatchBuilder().setIndex(index).build());
        }

        Collections.sort(list, new VtnIndexComparator());
        int prev = 0;
        for (VtnIndex data: list) {
            Integer index = data.getIndex();
            int value = index.intValue();
            assertTrue(prev <= value);
            prev = value;
        }
    }

    /**
     * Test case for {@link VtnIndexComparator#equals(Object)} and
     * {@link VtnIndexComparator#hashCode()}.
     */
    @Test
    public void testEquals() {
        HashSet<Object> set = new HashSet<>();
        VtnIndexComparator comp = new VtnIndexComparator();
        testEquals(set, comp, new VtnIndexComparator());
        assertEquals(true, set.contains(new VtnIndexComparator()));
        assertEquals(false, set.contains(new OrderedComparator()));
        assertEquals(1, set.size());
    }
}
