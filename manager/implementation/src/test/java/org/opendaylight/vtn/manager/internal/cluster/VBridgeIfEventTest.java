/*
 * Copyright (c) 2013-2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */
package org.opendaylight.vtn.manager.internal.cluster;

import java.util.List;

import org.junit.Test;
import org.opendaylight.controller.sal.core.UpdateType;
import org.opendaylight.vtn.manager.VBridgeIfPath;
import org.opendaylight.vtn.manager.VInterface;
import org.opendaylight.vtn.manager.VInterfaceConfig;
import org.opendaylight.vtn.manager.VNodeState;
import org.opendaylight.vtn.manager.internal.VNodeEventTestBase;

/**
 * JUnit test for {@link VBridgeIfEvent}.
 *
 * <p>
 *   Test case for methods except for
 *   {@link VBridgeIfEvent#eventReceived(org.opendaylight.vtn.manager.internal.VTNManagerImpl, boolean)}.
 *   Test for {@link VBridgeIfEvent#eventReceived(org.opendaylight.vtn.manager.internal.VTNManagerImpl, boolean)} is
 *   implemented in {@code VTNManagerImplClusterTest}.
 * </p>
 */
public class VBridgeIfEventTest extends VNodeEventTestBase {

    /**
     * Test case for getter methods.
     */
    @Test
    public void testGetter() {
        VNodeState[] stateValues = VNodeState.values();
        VNodeState[] states = new VNodeState[stateValues.length + 1];
        System.arraycopy(stateValues, 0, states, 1, stateValues.length);
        states[0] = null;

        String tname = "tenant";
        String bname = "bridge";
        for (String ifname: createStrings("vif")) {
            VBridgeIfPath ifpath = new VBridgeIfPath(tname, bname, ifname);
            for (String desc: createStrings("desc")) {
                for (Boolean enabled : createBooleans(false)) {
                    VInterfaceConfig ifconf = new VInterfaceConfig(desc, enabled);
                    for (VNodeState state: states) {
                        for (VNodeState estate: states) {
                            for (UpdateType utype : UpdateType.values()) {
                                for (Boolean save : createBooleans(false)) {
                                    String emsg = "(name)" + ifname
                                            + ",(desc)" + desc
                                            + ",(enabled)" + enabled
                                            + ",(state)" + state
                                            + ",(estate)" + estate
                                            + ",(updateType)" + utype
                                            + ",(save)" + save;

                                    VInterface vif = new VInterface(ifname, state,
                                                                    estate, ifconf);
                                    switch (utype) {
                                    case ADDED:
                                        VBridgeIfEvent.added(vtnMgr, ifpath, vif);
                                        break;
                                    case CHANGED:
                                        VBridgeIfEvent.changed(vtnMgr, ifpath, vif,
                                                             save.booleanValue());
                                        break;
                                    case REMOVED:
                                        VBridgeIfEvent.removed(vtnMgr, ifpath, vif,
                                                             save.booleanValue());
                                        break;
                                    default:
                                        break;
                                    }

                                    forceFlushEventQueue();
                                    List<ClusterEvent> events = getClusterEvent();

                                    assertEquals(emsg, 1, events.size());
                                    VBridgeIfEvent event = (VBridgeIfEvent)
                                        events.get(0);

                                    checkVBridgeIfEvent(event, ifpath, vif,
                                            utype, save, emsg);

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
     * Ensure that {@link VBridgeIfEvent} is serializable.
     */
    @Test
    public void testSerialize() {
        VNodeState[] stateValues = VNodeState.values();
        VNodeState[] states = new VNodeState[stateValues.length + 1];
        System.arraycopy(stateValues, 0, states, 1, stateValues.length);
        states[0] = null;

        String tname = "tenant";
        String bname = "bridge";
        for (String ifname: createStrings("vif")) {
            VBridgeIfPath ifpath = new VBridgeIfPath(tname, bname, ifname);
            for (String desc: createStrings("desc")) {
                for (Boolean enabled : createBooleans(false)) {
                    VInterfaceConfig ifconf = new VInterfaceConfig(desc, enabled);
                    for (VNodeState state: states) {
                        for (VNodeState estate: states) {
                            for (UpdateType utype : UpdateType.values()) {
                                for (Boolean save : createBooleans(false)) {
                                    String emsg = "(name)" + ifname
                                            + ",(desc)" + desc
                                            + ",(enabled)" + enabled
                                            + ",(state)" + state
                                            + ",(estate)" + estate
                                            + ",(updateType)" + utype
                                            + ",(save)" + save;

                                    VInterface vif = new VInterface(ifname, state,
                                                                    estate, ifconf);
                                    switch (utype) {
                                    case ADDED:
                                        VBridgeIfEvent.added(vtnMgr, ifpath, vif);
                                        break;
                                    case CHANGED:
                                        VBridgeIfEvent.changed(vtnMgr, ifpath, vif,
                                                             save.booleanValue());
                                        break;
                                    case REMOVED:
                                        VBridgeIfEvent.removed(vtnMgr, ifpath, vif,
                                                             save.booleanValue());
                                        break;
                                    default:
                                        break;
                                    }

                                    forceFlushEventQueue();
                                    List<ClusterEvent> events = getClusterEvent();

                                    assertEquals(emsg, 1, events.size());
                                    VBridgeIfEvent event = (VBridgeIfEvent)
                                        events.get(0);
                                    VBridgeIfEvent newEvent = (VBridgeIfEvent)
                                        eventSerializeTest(event);

                                    checkVBridgeIfEvent(newEvent, ifpath, vif,
                                                        utype, save, emsg);

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
     * Check {@link VBridgeIfEvent}.
     *
     * @param ifev      A {@link VBridgeIfEvent}.
     * @param ifpath    A {@link VBridgeIfPath}.
     * @param vif       A {@link VInterface}.
     * @param utype     An {@link UpdateType}.
     * @param save      If {@code true} expect that
     *                  {@link VBridgeIfEvent#isSaveConfig()} is {@code true}.
     *                  If {@code false} expect that it is {@code false}.
     * @param emsg      Output error message.
     */
    private void checkVBridgeIfEvent(VBridgeIfEvent ifev, VBridgeIfPath ifpath,
                                     VInterface vif, UpdateType utype, boolean save,
                                     String emsg) {
        assertEquals(ifpath, ifev.getPath());
        assertEquals(vif, ifev.getObject());
        assertEquals(vif, ifev.getVInterface());
        assertEquals(utype, ifev.getUpdateType());
        assertEquals("virtual bridge interface", ifev.getTypeName());

        assertTrue(emsg, ifev.isSingleThreaded(true));
        assertTrue(emsg, ifev.isSingleThreaded(false));
        if (utype == UpdateType.ADDED
                || save) {
            assertTrue(emsg, ifev.isSaveConfig());
        } else {
            assertFalse(emsg, ifev.isSaveConfig());
        }
    }

}
