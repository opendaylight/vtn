/*
 * Copyright (c) 2013-2014 NEC Corporation
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

import org.opendaylight.vtn.manager.VBridgeIfPath;
import org.opendaylight.vtn.manager.VInterface;
import org.opendaylight.vtn.manager.VInterfaceConfig;
import org.opendaylight.vtn.manager.VInterfacePath;
import org.opendaylight.vtn.manager.VNodeState;
import org.opendaylight.vtn.manager.VTerminalIfPath;
import org.opendaylight.vtn.manager.internal.VNodeEventTestBase;

import org.opendaylight.controller.sal.core.UpdateType;

/**
 * JUnit test for {@link VInterfaceEvent}.
 *
 * <p>
 *   Test case for methods except for
 *   {@link VInterfaceEvent#eventReceived(org.opendaylight.vtn.manager.internal.VTNManagerImpl, boolean)}.
 *   Test for {@link VInterfaceEvent#eventReceived(org.opendaylight.vtn.manager.internal.VTNManagerImpl, boolean)} is
 *   implemented in {@code VTNManagerImplClusterTest}.
 * </p>
 */
public class VInterfaceEventTest extends VNodeEventTestBase {

    /**
     * Test case for getter methods.
     */
    @Test
    public void testGetter() {
        String tname = "tenant";
        String bname = "bridge";
        for (VInterface vif: createInterfaces()) {
            String ifname = vif.getName();
            VInterfacePath[] ifpaths = {
                new VBridgeIfPath(tname, bname, ifname),
                new VTerminalIfPath(tname, bname, ifname),
            };
            for (VInterfacePath ifpath: ifpaths) {
                for (UpdateType utype : UpdateType.values()) {
                    for (Boolean save : createBooleans(false)) {
                        StringBuilder builder = new StringBuilder("vif=");
                        String emsg = builder.append(vif).
                            append("path=").append(ifpath).
                            append(",utype=").append(utype).
                            append(",save=").append(save).toString();

                        switch (utype) {
                        case ADDED:
                            VInterfaceEvent.added(vtnMgr, ifpath, vif);
                            break;

                        case CHANGED:
                            VInterfaceEvent.changed(vtnMgr, ifpath, vif,
                                                    save.booleanValue());
                            break;

                        case REMOVED:
                            VInterfaceEvent.removed(vtnMgr, ifpath, vif,
                                                    save.booleanValue());
                            break;

                        default:
                            fail("Unexpected update type: " + utype);
                            break;
                        }

                        forceFlushEventQueue();
                        List<ClusterEvent> events = getClusterEvent();

                        assertEquals(emsg, 1, events.size());
                        VInterfaceEvent event = (VInterfaceEvent)events.get(0);

                        checkVInterfaceEvent(event, ifpath, vif, utype, save,
                                             emsg);
                        clearClusterEvent();
                    }
                }
            }
        }
    }

    /**
     * Ensure that {@link VInterfaceEvent} is serializable.
     */
    @Test
    public void testSerialize() {
        String tname = "tenant";
        String bname = "bridge";
        for (VInterface vif: createInterfaces()) {
            String ifname = vif.getName();
            VInterfacePath[] ifpaths = {
                new VBridgeIfPath(tname, bname, ifname),
                new VTerminalIfPath(tname, bname, ifname),
            };
            for (VInterfacePath ifpath: ifpaths) {
                for (UpdateType utype : UpdateType.values()) {
                    for (Boolean save : createBooleans(false)) {
                        StringBuilder builder = new StringBuilder("vif=");
                        String emsg = builder.append(vif).
                            append("path=").append(ifpath).
                            append(",utype=").append(utype).
                            append(",save=").append(save).toString();

                        switch (utype) {
                        case ADDED:
                            VInterfaceEvent.added(vtnMgr, ifpath, vif);
                            break;

                        case CHANGED:
                            VInterfaceEvent.changed(vtnMgr, ifpath, vif,
                                                    save.booleanValue());
                            break;

                        case REMOVED:
                            VInterfaceEvent.removed(vtnMgr, ifpath, vif,
                                                    save.booleanValue());
                            break;

                        default:
                            fail("Unexpected update type: " + utype);
                            break;
                        }

                        forceFlushEventQueue();
                        List<ClusterEvent> events = getClusterEvent();

                        assertEquals(emsg, 1, events.size());
                        VInterfaceEvent event = (VInterfaceEvent)events.get(0);
                        VInterfaceEvent newEvent =
                            (VInterfaceEvent)eventSerializeTest(event);

                        checkVInterfaceEvent(newEvent, ifpath, vif, utype,
                                             save, emsg);
                        clearClusterEvent();
                    }
                }
            }
        }
    }

    /**
     * Check {@link VInterfaceEvent}.
     *
     * @param ifev      A {@link VInterfaceEvent}.
     * @param ifpath    Path to the target interface.
     * @param vif       A {@link VInterface}.
     * @param utype     An {@link UpdateType}.
     * @param save      If {@code true} expect that
     *                  {@link VInterfaceEvent#isSaveConfig()} is {@code true}.
     *                  If {@code false} expect that it is {@code false}.
     * @param emsg      Output error message.
     */
    private void checkVInterfaceEvent(VInterfaceEvent ifev,
                                      VInterfacePath ifpath,
                                      VInterface vif, UpdateType utype,
                                      boolean save, String emsg) {
        assertEquals(ifpath, ifev.getPath());
        assertEquals(vif, ifev.getObject());
        assertEquals(vif, ifev.getVInterface());
        assertEquals(utype, ifev.getUpdateType());
        assertEquals("virtual interface", ifev.getTypeName());

        assertTrue(emsg, ifev.isSingleThreaded(true));
        assertTrue(emsg, ifev.isSingleThreaded(false));
        if (utype == UpdateType.ADDED || save) {
            assertTrue(emsg, ifev.isSaveConfig());
        } else {
            assertFalse(emsg, ifev.isSaveConfig());
        }
    }

    /**
     * Create unique {@link VInterfaceConfig} instances.
     *
     * @return  A list of {@link VInterfaceConfig} instances.
     */
    private List<VInterfaceConfig> createInterfaceConfigs() {
        ArrayList<VInterfaceConfig> list = new ArrayList<VInterfaceConfig>();
        for (String desc: createStrings("desc")) {
            for (Boolean enabled : createBooleans(false)) {
                list.add(new VInterfaceConfig(desc, enabled));
            }
        }

        return list;
    }

    /**
     * Create unique {@link VInterface} instances.
     *
     * @return  A list of {@link VInterface} instances.
     */
    private List<VInterface> createInterfaces() {
        ArrayList<VInterface> list = new ArrayList<VInterface>();
        VNodeState[] states = VNodeState.values();
        for (VInterfaceConfig iconf: createInterfaceConfigs()) {
            for (String name: createStrings("vif")) {
                for (VNodeState state: states) {
                    for (VNodeState estate: states) {
                        list.add(new VInterface(name, state, estate, iconf));
                    }
                }
            }
        }

        return list;
    }
}
