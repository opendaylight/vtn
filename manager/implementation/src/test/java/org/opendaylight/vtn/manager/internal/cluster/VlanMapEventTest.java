/*
 * Copyright (c) 2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */
package org.opendaylight.vtn.manager.internal.cluster;

import java.util.List;

import org.junit.Test;
import org.opendaylight.controller.sal.core.Node;
import org.opendaylight.controller.sal.core.UpdateType;
import org.opendaylight.vtn.manager.VBridgePath;
import org.opendaylight.vtn.manager.VlanMap;
import org.opendaylight.vtn.manager.internal.VNodeEventTestBase;
import org.opendaylight.vtn.manager.internal.VTNManagerImpl;

/**
 * JUnit test for {@link VlanMapEvent}.
 *
 * <p>
 *   Test case for methods except for
 *   {@link VlanMapEvent#eventReceived(VTNManagerImpl, boolean)}.
 *   Test for {@link VlanMapEvent#eventReceived(VTNManagerImpl, boolean)} is
 *   implemented in {@code VTNManagerImplClusterTest}.
 * </p>
 */
public class VlanMapEventTest extends VNodeEventTestBase {

    /**
     * Test case for getter methods.
     */
    @Test
    public void testGetter() {
        short[] vlans = new short[] {0, 1, 4095};

        String tname = "tenant";
        String bname = "vbridge";
        VBridgePath bpath = new VBridgePath(tname, bname);
        for (String id : createStrings("id")) {
            for (Node node : createNodes(4)) {
                for (short vlan : vlans) {
                    for (UpdateType utype : UpdateType.values()) {
                        for (Boolean save : createBooleans(false)) {
                            String emsg = "(name)" + bname
                                    + ",(id)" + id + ",(node)" + node
                                    + ",(vlan)" + vlan
                                    + ",(updateType)" + utype
                                    + ",(save)" + save;


                            VlanMap vmap = new VlanMap(id, node ,vlan);

                            switch (utype) {
                            case ADDED:
                                VlanMapEvent.added(vtnMgr, bpath, vmap);
                                break;
                            case CHANGED:
                                continue;
                            case REMOVED:
                                VlanMapEvent.removed(vtnMgr, bpath, vmap,
                                        save.booleanValue());
                                break;
                            default:
                                break;
                            }

                            forceFlushEventQueue();
                            List<ClusterEvent> events = getClusterEvent();

                            assertEquals(emsg, 1, events.size());
                            VlanMapEvent event
                                = (VlanMapEvent) events.get(0);

                            checkVlanMapEvent(event, bpath, vmap, utype, save,
                                              emsg);

                            clearClusterEvent();
                        }
                    }
                }
            }
        }
    }

    /**
     * Ensure that {@link VlanMapEvent} is serializable.
     */
    @Test
    public void testSerialize() {
        short[] vlans = new short[] {0, 1, 4095};

        String tname = "tenant";
        String bname = "vbridge";
        VBridgePath bpath = new VBridgePath(tname, bname);
        for (String id : createStrings("id")) {
            for (Node node : createNodes(4)) {
                for (short vlan : vlans) {
                    for (UpdateType utype : UpdateType.values()) {
                        for (Boolean save : createBooleans(false)) {
                            String emsg = "(name)" + bname
                                    + ",(id)" + id + ",(node)" + node
                                    + ",(vlan)" + vlan
                                    + ",(updateType)" + utype
                                    + ",(save)" + save;

                            VlanMap vmap = new VlanMap(id, node ,vlan);

                            switch (utype) {
                            case ADDED:
                                VlanMapEvent.added(vtnMgr, bpath, vmap);
                                break;
                            case CHANGED:
                                continue;
                            case REMOVED:
                                VlanMapEvent.removed(vtnMgr, bpath, vmap,
                                        save.booleanValue());
                                break;
                            default:
                                break;
                            }

                            forceFlushEventQueue();
                            List<ClusterEvent> events = getClusterEvent();

                            assertEquals(emsg, 1, events.size());
                            VlanMapEvent event
                                = (VlanMapEvent) events.get(0);

                            VlanMapEvent newEvent
                                = (VlanMapEvent) eventSerializeTest(event);

                            checkVlanMapEvent(newEvent, bpath, vmap, utype, save,
                                              emsg);

                            clearClusterEvent();
                        }
                    }
                }
            }
        }
    }

    /**
     * Check {@link VlanMapEvent}.
     *
     * @param ev    A {@link VlanMapEvent}.
     * @param bpath A {@link VBridgePath}.
     * @param map   A {@link VlanMap}.
     * @param utype An {@link UpdateType}.
     * @param save  If {@code true} expect that
     *              {@link VlanMapEvent#isSaveConfig()} is {@code true}.
     *              If {@code false} expect that it is {@code false}.
     * @param emsg  Output error message.
     */
    private void checkVlanMapEvent(VlanMapEvent ev,
                                   VBridgePath bpath, VlanMap map,
                                   UpdateType utype, boolean save, String emsg) {
        assertEquals(bpath, ev.getPath());
        assertEquals(map, ev.getObject());
        assertEquals(map, ev.getVlanMap());
        assertEquals(utype, ev.getUpdateType());
        assertEquals("VLAN mapping", ev.getTypeName());
        assertTrue(emsg, ev.isSingleThreaded(true));
        assertTrue(emsg, ev.isSingleThreaded(false));
        if (utype == UpdateType.ADDED || save) {
            assertTrue(emsg, ev.isSaveConfig());
        } else {
            assertFalse(emsg, ev.isSaveConfig());
        }
    }
}
