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

import org.opendaylight.controller.sal.core.Node;
import org.opendaylight.controller.sal.core.NodeConnector;
import org.opendaylight.controller.sal.utils.NodeConnectorCreator;
import org.opendaylight.controller.sal.utils.NodeCreator;

/**
 * JUnit test for {@link MacMapDuplicateLog}.
 */

public class MacMapPortBusyLogTest extends TestBase {
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
            Node node10 = NodeCreator.createOFNode(Long.valueOf("10"));
            NodeConnector nc = NodeConnectorCreator.
                createOFNodeConnector(Short.valueOf("99"), node10);
            MapReference pref  = new MapReference(MapType.MAC, "default",
                                                  mpath);
            MacMapPortBusyLog log =
                new MacMapPortBusyLog(ref, mvlan, nc, pref);
            StringBuilder builder =
                new StringBuilder(MAC_UNAVAILABLE + SWITCH_RESERVED);
            mvlan.appendContents(builder);
            builder.append("}, port=").append(nc.toString()).
                append(",map=").append(pref.toString());
            assertEquals(builder.toString(), log.getMessage());
        }
    }
}
