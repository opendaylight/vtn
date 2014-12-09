/*
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal;

import org.junit.Test;
import org.opendaylight.vtn.manager.flow.cond.FlowCondition;

/**
 * JUnit test for {@link FlowConditionComparator}.
 */

public class FlowConditionComparatorTest extends TestBase {

    /**
     * Test method for
     * {@link FlowConditionComparator#hashCode(),
     *  @link FlowConditionComparator#equals(Object),
     *  @link FlowConditionComparator#compare(FlowCondition, FlowCondition)}.
     */
    @Test
    public void testComparator() {
        try {
            FlowConditionComparator flowConditionomparator = new FlowConditionComparator();

            assertTrue(flowConditionomparator.equals(flowConditionomparator));
            assertFalse(flowConditionomparator.equals(null));
            assertFalse(flowConditionomparator.equals(new Integer(1)));
            assertNotNull(flowConditionomparator.hashCode());

            for (FlowCondition flowCondition1 : createFlowConditions()) {
                for (FlowCondition flowCondition2 : createFlowConditions()) {
                    try {
                        assertEquals(0, flowConditionomparator.compare(flowCondition2, flowCondition2));
                        assertNotNull(flowConditionomparator.compare(flowCondition2, flowCondition1));
                    } catch (Exception ex) {
                        //ex.printStackTrace();
                    }
                }
            }
        } catch (Exception ex) {
            //ex.printStackTrace();
        }
    }
}
