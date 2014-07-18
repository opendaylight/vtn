/*
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.cluster;

import java.util.List;
import java.util.Set;

import org.junit.Test;

import org.opendaylight.vtn.manager.DataLinkHost;
import org.opendaylight.vtn.manager.MacMapConfig;
import org.opendaylight.vtn.manager.VBridgePath;
import org.opendaylight.vtn.manager.internal.VNodeEventTestBase;

import org.opendaylight.controller.sal.core.UpdateType;

/**
 * JUnit test for {@link MacMapEvent}.
 *
 * <p>
 *   This class does not check if
 *   {@link MacMapEvent#eventReceived(org.opendaylight.vtn.manager.internal.VTNManagerImpl, boolean)} works.
 *   It will be done by
 *   {@link org.opendaylight.vtn.manager.internal.VTNManagerImplClusterTest}.
 * </p>
 */
public class MacMapEventTest extends VNodeEventTestBase {
    /**
     * Test case for all methods.
     */
    @Test
    public void testEvent() {
        String tname = "tenant";
        for (String bname: createStrings("bridge")) {
            VBridgePath bpath = new VBridgePath(tname, bname);
            for (Set<DataLinkHost> allow: createDataLinkHostSet(3)) {
                for (Set<DataLinkHost> deny: createDataLinkHostSet(3)) {
                    MacMapConfig mcconf = new MacMapConfig(allow, deny);
                    for (Boolean save : createBooleans(false)) {
                        for (UpdateType utype: UpdateType.values()) {
                            switch (utype) {
                            case ADDED:
                                MacMapEvent.added(vtnMgr, bpath, mcconf);
                                break;
                            case CHANGED:
                                MacMapEvent.changed(vtnMgr, bpath, mcconf);
                                break;
                            case REMOVED:
                                MacMapEvent.removed(vtnMgr, bpath, mcconf,
                                                    save.booleanValue());
                                break;
                            default:
                                fail("Unexpected update type: " + utype);
                            }

                            forceFlushEventQueue();
                            List<ClusterEvent> events = getClusterEvent();
                            assertEquals(1, events.size());
                            MacMapEvent ev = (MacMapEvent)events.get(0);

                            checkEvent(ev, bpath, mcconf, utype, save);
                            clearClusterEvent();

                            // Ensure that an MacMapEvent instance is
                            // serializable.
                            MacMapEvent newev = (MacMapEvent)
                                eventSerializeTest(ev);
                            checkEvent(newev, bpath, mcconf, utype, save);
                        }
                    }
                }
            }
        }
    }
}
