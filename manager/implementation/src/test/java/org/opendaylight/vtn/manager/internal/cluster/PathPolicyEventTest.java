/*
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */
package org.opendaylight.vtn.manager.internal.cluster;

import org.junit.Test;
import org.opendaylight.vtn.manager.internal.TestBase;
import org.opendaylight.vtn.manager.internal.VTNManagerImpl;
import org.opendaylight.controller.sal.core.UpdateType;

/**
 * JUnit test for {@link PathPolicyEvent}.
 */
public class PathPolicyEventTest extends TestBase {

    /**
     * Junit for PathPolicyEvent method.
     */
    @Test
    public void testRaise() {
        //Checking for all the scenarios for raise method in FlowConditionEvent.
        for (int idx : INDEX_ARRAY) {
            for (UpdateType updateTye: updateTypeList()) {
                for (VTNManagerImpl mgr: createVtnManagerImplobj()) {
                    try {
                        PathPolicyEvent.raise(mgr, idx, updateTye);
                    } catch (NullPointerException ex) {
                        //ex.printStackTrace();
                    }
                }
            }
        }
    }
}
