/*
 * Copyright (c) 2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */
package org.opendaylight.vtn.manager.internal.cluster;

import java.util.ArrayList;
import java.util.List;

import org.junit.Test;
import org.opendaylight.controller.sal.core.UpdateType;
import org.opendaylight.vtn.manager.VBridge;
import org.opendaylight.vtn.manager.VBridgeConfig;
import org.opendaylight.vtn.manager.VBridgePath;
import org.opendaylight.vtn.manager.VNodeState;
import org.opendaylight.vtn.manager.internal.VNodeEventTestBase;
import org.opendaylight.vtn.manager.internal.VTNManagerImpl;

/**
 * JUnit test for {@link VBridgeEvent}.
 *
 * <p>
 *   Test case for methods except for
 *   {@link VBridgeEvent#eventReceived(VTNManagerImpl, boolean)}.
 *   Test for {@link VBridgeEvent#eventReceived(VTNManagerImpl, boolean)} is
 *   implemented in {@code VTNManagerImplClusterTest}.
 * </p>
 */
public class VBridgeEventTest extends VNodeEventTestBase {

    /**
     * Test case for getter methods.
     */
    @Test
    public void testGetter() {
        VNodeState[] stateValues = VNodeState.values();
        VNodeState[] states = new VNodeState[stateValues.length + 1];
        System.arraycopy(stateValues, 0, states, 1, stateValues.length);
        states[0] = null;
        List<Integer> ages = new ArrayList<Integer>();
        ages.add(null);
        ages.add(10);
        ages.add(600);
        ages.add(1000000);

        String tname = "tenant";
        for (String bname : createStrings("vbr")) {
            VBridgePath bpath = new VBridgePath(tname, bname);
            for (String desc : createStrings("desc")) {
                for (Integer age : ages) {
                    VBridgeConfig bconf = createVBridgeConfig(desc, age);
                    for (int flt = 0; flt < 3; flt++) {
                        for (VNodeState state : states) {
                            for (UpdateType utype : UpdateType.values()) {
                                for (Boolean save : createBooleans(false)) {
                                    String emsg = "(name)" + bname + ",(desc)" + desc
                                            + ",(ival)"
                                            + ((age == null) ? "null" : age.intValue())
                                            + ",(flt)" + flt
                                            + ",(state)" + state
                                            + ",(updateType)" + utype
                                            + ",(save)" + save;

                                    VBridge vbridge = new VBridge(bname, state, flt,
                                                                  bconf);
                                    switch (utype) {
                                    case ADDED:
                                        VBridgeEvent.added(vtnMgr, bpath, vbridge);
                                        break;
                                    case CHANGED:
                                        VBridgeEvent.changed(vtnMgr, bpath, vbridge,
                                                             save.booleanValue());
                                        break;
                                    case REMOVED:
                                        VBridgeEvent.removed(vtnMgr, bpath, vbridge,
                                                             save.booleanValue());
                                        break;
                                    default:
                                        break;
                                    }

                                    forceFlushEventQueue();
                                    List<ClusterEvent> events = getClusterEvent();

                                    assertEquals(emsg, 1, events.size());
                                    VBridgeEvent event
                                        = (VBridgeEvent) events.get(0);

                                    checkVBridgeEvent(event, bpath, vbridge,
                                                      state, utype, save, emsg);

                                    clearClusterEvent();
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    /**
     * Ensure that {@link VBridgeEvent} is serializable.
     */
    @Test
    public void testSerialize() {
        VNodeState[] stateValues = VNodeState.values();
        VNodeState[] states = new VNodeState[stateValues.length + 1];
        System.arraycopy(stateValues, 0, states, 1, stateValues.length);
        states[0] = null;
        List<Integer> ages = new ArrayList<Integer>();
        ages.add(null);
        ages.add(10);
        ages.add(600);
        ages.add(1000000);

        String tname = "tenant";
        for (String bname : createStrings("vbr")) {
            VBridgePath bpath = new VBridgePath(tname, bname);
            for (String desc : createStrings("desc")) {
                for (Integer age : ages) {
                    VBridgeConfig bconf = createVBridgeConfig(desc, age);
                    for (int flt = 0; flt < 3; flt++) {
                        for (VNodeState state: states) {
                            for (UpdateType utype : UpdateType.values()) {
                                for (Boolean save : createBooleans(false)) {
                                    String emsg = "(name)" + bname + ",(desc)" + desc
                                            + ",(age)"
                                            + ((age == null) ? "null" : age.intValue())
                                            + ",(flt)" + flt
                                            + ",(state)" + state
                                            + ",(updateType)" + utype
                                            + ",(save)" + save;

                                    VBridge vbridge = new VBridge(bname, state, flt,
                                                                  bconf);
                                    switch (utype) {
                                    case ADDED:
                                        VBridgeEvent.added(vtnMgr, bpath, vbridge);
                                        break;
                                    case CHANGED:
                                        VBridgeEvent.changed(vtnMgr, bpath, vbridge,
                                                             save.booleanValue());
                                        break;
                                    case REMOVED:
                                        VBridgeEvent.removed(vtnMgr, bpath, vbridge,
                                                             save.booleanValue());
                                        break;
                                    default:
                                        break;
                                    }

                                    forceFlushEventQueue();
                                    List<ClusterEvent> events = getClusterEvent();

                                    assertEquals(emsg, 1, events.size());
                                    VBridgeEvent event
                                        = (VBridgeEvent) events.get(0);
                                    VBridgeEvent newEvent
                                        = (VBridgeEvent) eventSerializeTest(event);

                                    checkVBridgeEvent(newEvent, bpath, vbridge,
                                                      state, utype, save, emsg);

                                    clearClusterEvent();
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    /**
     * Check {@link VBridgeEvent}.
     *
     * @param bev   A {@link VBridgeEvent}.
     * @param bpath A {@link VBridgePath}.
     * @param brdg  A {@link VBridge}.
     * @param state A state of {@link VBridge}.
     * @param utype An {@link UpdateType}.
     * @param save  If {@code true} expect that {@link VBridgeEvent#isSaveConfig()} is
     *              {@code true}.
     *              If {@code false} expect that it is {@code false}.
     * @param emsg  Output error message.
     */
    private void checkVBridgeEvent(VBridgeEvent bev, VBridgePath bpath,
                                   VBridge brdg, VNodeState state,
                                   UpdateType utype, boolean save, String emsg) {
        assertEquals(emsg, bpath, bev.getPath());
        assertEquals(emsg, brdg, bev.getObject());
        assertEquals(emsg, brdg, bev.getVBridge());
        assertEquals(emsg, utype, bev.getUpdateType());
        assertEquals(emsg, "virtual bridge", bev.getTypeName());
        assertTrue(emsg, bev.isSingleThreaded(true));
        assertTrue(emsg, bev.isSingleThreaded(false));
        if (utype == UpdateType.ADDED || save) {
            assertTrue(emsg, bev.isSaveConfig());
        } else {
            assertFalse(emsg, bev.isSaveConfig());
        }
    }
}
