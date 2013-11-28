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
import org.opendaylight.controller.sal.core.NodeConnector;
import org.opendaylight.controller.sal.core.UpdateType;
import org.opendaylight.vtn.manager.PortMap;
import org.opendaylight.vtn.manager.PortMapConfig;
import org.opendaylight.vtn.manager.SwitchPort;
import org.opendaylight.vtn.manager.VBridgeIfPath;
import org.opendaylight.vtn.manager.internal.VNodeEventTestBase;
import org.opendaylight.vtn.manager.internal.VTNManagerImpl;

/**
 * JUnit test for {@link PortMapEvent}.
 *
 * <p>
 *   Test case for methods except for
 *   {@link PortMapEvent#eventReceived(VTNManagerImpl, boolean)}.
 *   Test for {@link PortMapEvent#eventReceived(VTNManagerImpl, boolean)} is
 *   implemented in {@code VTNManagerImplClusterTest}.
 * </p>
 */
public class PortMapEventTest extends VNodeEventTestBase {

    /**
     * Test case for getter methods.
     */
    @Test
    public void testGetter() {
        short[] vlans = new short[] {0, 1, 4095};
        String tname = "tenant";
        String bname = "bridge";
        String ifname = "if";
        VBridgeIfPath ifpath = new VBridgeIfPath(tname, bname, ifname);

        for (NodeConnector nc : createNodeConnectors(4)) {
            for (short vlan : vlans) {
                for (UpdateType utype : UpdateType.values()) {
                    for (Boolean save : createBooleans(false)) {
                        String emsg = "(name)" + ifname
                                + ",(nc)" + nc + ",(vlan)" + vlan
                                + ",(updateType)" + utype
                                + ",(save)" + save;

                        PortMapConfig pmconf = null;
                        if (nc == null) {
                            pmconf = new PortMapConfig(null, null, vlan);
                        } else if (nc.getType()
                                == NodeConnector.NodeConnectorIDType.OPENFLOW) {
                            Short id = (Short) nc.getID();
                            SwitchPort sp = new SwitchPort(nc.getType(),
                                                           id.toString());
                            pmconf = new PortMapConfig(nc.getNode(), sp, vlan);
                        } else {
                            SwitchPort sp = new SwitchPort(nc.getType(),
                                                           (String) nc.getID());
                            pmconf = new PortMapConfig(nc.getNode(), sp, vlan);
                        }
                        PortMap pmap = new PortMap(pmconf, nc);

                        switch (utype) {
                        case ADDED:
                            PortMapEvent.added(vtnMgr, ifpath, pmap);
                            break;
                        case CHANGED:
                            PortMapEvent.changed(vtnMgr, ifpath, pmap,
                                                 save.booleanValue());
                            break;
                        case REMOVED:
                            PortMapEvent.removed(vtnMgr, ifpath, pmap,
                                    save.booleanValue());
                            break;
                        default:
                            break;
                        }

                        forceFlushEventQueue();
                        List<ClusterEvent> events = getClusterEvent();

                        assertEquals(emsg, 1, events.size());
                        PortMapEvent event = (PortMapEvent) events.get(0);

                        checkPortMapEvent(event, ifpath, pmap, utype, save, emsg);

                        clearClusterEvent();
                    }
                }
            }

        }
    }

    /**
     * Ensure that {@link PortMapEvent} is serializable.
     */
    @Test
    public void testSerialize() {
        short[] vlans = new short[] {0, 1, 4095};
        String tname = "tenant";
        String bname = "bridge";
        String ifname = "if";
        VBridgeIfPath ifpath = new VBridgeIfPath(tname, bname, ifname);

        for (NodeConnector nc : createNodeConnectors(4)) {
            for (short vlan : vlans) {
                for (UpdateType utype : UpdateType.values()) {
                    for (Boolean save : createBooleans(false)) {
                        String emsg = "(name)" + ifname
                                + ",(nc)" + nc + ",(vlan)" + vlan
                                + ",(updateType)" + utype
                                + ",(save)" + save;

                        PortMapConfig pmconf = null;
                        if (nc == null) {
                            pmconf = new PortMapConfig(null, null, vlan);
                        } else if (nc.getType()
                                == NodeConnector.NodeConnectorIDType.OPENFLOW) {
                            Short id = (Short) nc.getID();
                            SwitchPort sp = new SwitchPort(nc.getType(),
                                                           id.toString());
                            pmconf = new PortMapConfig(nc.getNode(), sp, vlan);
                        } else {
                            SwitchPort sp = new SwitchPort(nc.getType(),
                                                           (String) nc.getID());
                            pmconf = new PortMapConfig(nc.getNode(), sp, vlan);
                        }
                        PortMap pmap = new PortMap(pmconf, nc);

                        switch (utype) {
                        case ADDED:
                            PortMapEvent.added(vtnMgr, ifpath, pmap);
                            break;
                        case CHANGED:
                            PortMapEvent.changed(vtnMgr, ifpath, pmap,
                                                 save.booleanValue());
                            break;
                        case REMOVED:
                            PortMapEvent.removed(vtnMgr, ifpath, pmap,
                                                 save.booleanValue());
                            break;
                        default:
                            break;
                        }

                        forceFlushEventQueue();
                        List<ClusterEvent> events = getClusterEvent();

                        assertEquals(emsg, 1, events.size());
                        PortMapEvent event
                            = (PortMapEvent) events.get(0);
                        PortMapEvent newEvent
                            = (PortMapEvent) eventSerializeTest(event);

                        checkPortMapEvent(newEvent, ifpath, pmap, utype, save, emsg);

                        clearClusterEvent();
                    }
                }
            }
        }
    }

    /**
     * Check {@link PortMapEvent}.
     *
     * @param ev        A {@link PortMapEvent}.
     * @param ifpath    A {@link VBridgeIfPath}.
     * @param pmap      A {@link PortMap}.
     * @param utype     An {@link UpdateType}.
     * @param save      If {@code true} expect that
     *                  {@link PortMapEvent#isSaveConfig()} is {@code true}.
     *                  If {@code false} expect that it is {@code false}.
     * @param emsg      Output error message.
     */
    private void checkPortMapEvent(PortMapEvent ev,
                                   VBridgeIfPath ifpath, PortMap pmap,
                                   UpdateType utype, boolean save, String emsg) {
        assertEquals(ifpath, ev.getPath());
        assertEquals(emsg, pmap, ev.getObject());
        assertEquals(pmap, ev.getPortMap());
        assertEquals(utype, ev.getUpdateType());
        assertEquals("port mapping", ev.getTypeName());
        assertTrue(emsg, ev.isSingleThreaded(true));
        assertTrue(emsg, ev.isSingleThreaded(false));
        if (utype == UpdateType.ADDED || save) {
            assertTrue(emsg, ev.isSaveConfig());
        } else {
            assertFalse(emsg, ev.isSaveConfig());
        }
    }

}