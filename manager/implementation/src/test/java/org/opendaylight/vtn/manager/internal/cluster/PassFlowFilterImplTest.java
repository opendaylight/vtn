/*
 * Copyright (c) 2014, 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.cluster;

import org.junit.Test;
import org.opendaylight.vtn.manager.internal.TestBase;
import org.opendaylight.vtn.manager.flow.filter.FlowFilter;
/**
 * JUnit test for {@link PassFlowFilterImpl}.
 */
public class PassFlowFilterImplTest extends TestBase {
    /**
     * Testing the all the methods in {@link PassFlowFilterImpl}.
     */
    @Test
    public void testGetter() {
        PassFlowFilterImpl filterImpl = null;

        //Checking for all the scenarios for PassFlowFilterImpl raise method.
        for (int idx:INDEX_ARRAY) {
            for (FlowFilter flowFilters: createFlowFilter()) {
                FlowFilter[] filterList = {flowFilters};
                for (FlowFilter flowfilter: filterList) {
                    try {
                        filterImpl = new PassFlowFilterImpl(idx, flowfilter);
                        filterImpl.apply(null, null, null);

                        assertEquals(filterImpl.getIndex(), idx);
                        assertEquals(filterImpl.getLogger().getName(), PassFlowFilterImpl.class.getName());
                        assertNotNull(filterImpl.getFlowFilter());
                        assertNotNull(filterImpl.getFilterType());
                    } catch (Exception ex) {
                        //ex.printStackTrace();
                    }
                }
            }
        }
    }
}
