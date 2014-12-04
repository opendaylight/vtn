/*
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution,and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.cluster;

import org.junit.Test;
import java.util.List;
import org.opendaylight.vtn.manager.VTNException;
import org.opendaylight.vtn.manager.flow.filter.FlowFilter;
import org.opendaylight.vtn.manager.internal.TestBase;

/**
 * JUnit test for {@link PassFlowFilterImpl}.
 */
public class PassFlowFilterImplTest extends TestBase {
    /**
     * Testing the Get methods in {@link PassFlowFilterImpl}.
    */
    @Test
    public void testGetter() {
        PassFlowFilterImpl filterImpl = null;
        //Checking for all the scenarios for PassFlowFilterImpl raise method.
        for (int idx:INDEX_ARRAY) {
            List<FlowFilter> filterList = createFlowFilter();
            for (FlowFilter flowfilter: filterList) {
                try {
                    filterImpl = new PassFlowFilterImpl(idx, flowfilter);
                    assertEquals(filterImpl.getIndex(), idx);
                    assertNotNull(filterImpl.getFlowFilter());
                } catch (VTNException | NullPointerException ex) {
                    ex.printStackTrace();
                }
            }
        }
    }
}
