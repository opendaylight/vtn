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
import org.opendaylight.vtn.manager.internal.VTNManagerImpl;
import org.opendaylight.controller.sal.core.UpdateType;
import org.opendaylight.vtn.manager.VTenantPath;

/**
 * JUnit test for {@link FlowFilterEvent}.
 */
public class FlowFilterEventTest extends TestBase {

    /**
     * Junit for FlowFilterEvent method.
    */
    @Test
    public void testRaise() {
        //Checking for all the scenarios for raise method in FlowFilterEvent.
        for (VTenantPath tenantPath : creatVtenantPaths()) {
            for (UpdateType updateTye: updateTypeList()) {
                for (VTNManagerImpl mgr: createVtnManagerImplobj()) {
                    for (Boolean out: createBooleans()) {
                        for (int idx:INDEX_ARRAY) {
                            try {
                                FlowFilterEvent.raise(mgr, tenantPath, (boolean)out, idx, updateTye);
                            } catch (NullPointerException ex) {
                            }
                        }
                    }
                }
            }
        }
    }
}
