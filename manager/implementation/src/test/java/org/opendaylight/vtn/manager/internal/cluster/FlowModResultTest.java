/*
 * Copyright (c) 2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */
package org.opendaylight.vtn.manager.internal.cluster;

import static org.junit.Assert.*;

import org.junit.Test;
import org.opendaylight.controller.sal.utils.Status;
import org.opendaylight.controller.sal.utils.StatusCode;

/**
 * JUnit Test for {@link FlowModResult}.
 */
public class FlowModResultTest {

    /**
     * Test method for {@link FlowModResult#toStatus()}.
     */
    @Test
    public void testToStatus() {
        for (FlowModResult result : FlowModResult.values()) {
            Status st = null;
            if (result == FlowModResult.SUCCEEDED) {
                st = new Status(StatusCode.SUCCESS, null);
            } else if (result == FlowModResult.TIMEDOUT) {
                st = new Status(StatusCode.TIMEOUT, "Operation timed out");
            } else {
                st = new Status(StatusCode.INTERNALERROR, "Operation failed: "
                        + result);
            }

            assertEquals(st.getCode(), result.toStatus().getCode());
        }
    }
}
