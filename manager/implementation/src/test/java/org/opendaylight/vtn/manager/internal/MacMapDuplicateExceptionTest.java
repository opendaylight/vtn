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
 * JUnit test for {@link MacMapDuplicateException}.
 */
public class MacMapDuplicateExceptionTest extends TestBase {
    /**
     * Test case for getter methods.
     */
    @Test
    public void testGetter() {
        for (int i = 0; i < 10; i++) {
            VBridgePath bpath = new VBridgePath("tenant", "bridge" + i);
            MacMapPath mpath = new MacMapPath(bpath);
            MapReference ref = new MapReference(MapType.MAC, "default", mpath);
            long mac = (long)(i + 0x100000);
            MacVlan mvlan = new MacVlan(mac, (short)i);
            MacVlan dup = new MacVlan(mac, (short)(i + 5));
            MacMapDuplicateException e =
                new MacMapDuplicateException(mvlan, ref, dup);

            assertEquals(mvlan, e.getHost());
            assertEquals(ref, e.getMapReference());
            assertEquals(dup, e.getDuplicate());

            Status st = e.getStatus();
            StatusCode code = StatusCode.CONFLICT;
            assertEquals(code, st.getCode());

            String msg = "Duplicate MAC address found";
            assertEquals(msg, st.getDescription());
            assertEquals(msg, e.getMessage());
        }
    }
}
