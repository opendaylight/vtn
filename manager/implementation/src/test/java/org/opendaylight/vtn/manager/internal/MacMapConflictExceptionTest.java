/*
 * Copyright (c) 2014, 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal;

import org.junit.Test;

import org.opendaylight.vtn.manager.VBridgePath;
import org.opendaylight.vtn.manager.internal.cluster.MacMapPath;
import org.opendaylight.vtn.manager.internal.cluster.MacVlan;
import org.opendaylight.vtn.manager.internal.cluster.MapReference;
import org.opendaylight.vtn.manager.internal.cluster.MapType;

import org.opendaylight.controller.sal.utils.Status;
import org.opendaylight.controller.sal.utils.StatusCode;

/**
 * JUnit test for {@link MacMapConflictException}.
 */
public class MacMapConflictExceptionTest extends TestBase {
    /**
     * Test case for getter methods.
     */
    @Test
    public void testGetter() {
        for (int i = 0; i < 10; i++) {
            VBridgePath bpath = new VBridgePath("tenant", "bridge" + i);
            MacMapPath mpath = new MacMapPath(bpath);
            MapReference ref = new MapReference(MapType.MAC, "default", mpath);
            MacVlan mvlan = new MacVlan((long)(i + 10), (short)i);
            MacMapConflictException e =
                new MacMapConflictException(mvlan, ref);

            assertEquals(mvlan, e.getHost());
            assertEquals(ref, e.getMapReference());

            Status st = e.getStatus();
            StatusCode code = StatusCode.CONFLICT;
            assertEquals(code, st.getCode());

            String msg = "Already mapped by MAC mapping";
            assertEquals(msg, st.getDescription());

            StringBuilder builder = new StringBuilder(code.toString());
            builder.append(": ").append(msg).append(" (0)");
            assertEquals(builder.toString(), e.getMessage());
        }
    }
}
