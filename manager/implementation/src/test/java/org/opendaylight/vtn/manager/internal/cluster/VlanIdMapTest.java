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
import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;

import org.junit.Test;

import org.opendaylight.vtn.manager.VBridgePath;
import org.opendaylight.vtn.manager.VTNException;
import org.opendaylight.vtn.manager.VlanMapConfig;
import org.opendaylight.vtn.manager.internal.TestBase;

import org.opendaylight.controller.sal.core.Node;
import org.opendaylight.controller.sal.utils.StatusCode;

/**
 * JUnit test for {@link VlanIdMap}.
 */
public class VlanIdMapTest extends TestBase {
    /**
     * Test case for the following methods which handles a global VLAN mapping.
     *
     * <ul>
     *   <li>{@link VlanIdMap#create(VlanMapImpl)}</li>
     *   <li>{@link VlanIdMap.GlobalMap#getSize()}</li>
     *   <li>{@link VlanIdMap.GlobalMap#isEmpty()}</li>
     *   <li>{@link VlanIdMap.GlobalMap#add(VlanMapImpl)}</li>
     *   <li>{@link VlanIdMap.GlobalMap#remove(Node)}</li>
     *   <li>{@link VlanIdMap.GlobalMap#get(Node)}</li>
     *   <li>{@link VlanIdMap.GlobalMap#match(Node)}</li>
     * </ul>
     */
    @Test
    public void testGlobalMap() {
        VBridgePath bpath = new VBridgePath("tenant", "bridge");
        String mapId = "vmap";
        ArrayList<VlanMapImpl> others = new ArrayList<VlanMapImpl>();
        List<Node> nodes = createNodes(5);
        for (Node node: nodes) {
            VlanMapConfig vlc = new VlanMapConfig(node, (short)0);
            others.add(new VlanMapImpl(bpath, mapId, vlc));
        }

        VlanMapConfig vlc = new VlanMapConfig(null, (short)0);
        VlanMapImpl vmap = new VlanMapImpl(bpath, mapId, vlc);
        VlanIdMap idmap = VlanIdMap.create(vmap);
        assertTrue(idmap instanceof VlanIdMap.GlobalMap);
        assertEquals(1, idmap.getSize());
        assertFalse(idmap.isEmpty());

        // No more VLAN mapping can be added to this object.
        for (VlanMapImpl other: others) {
            try {
                idmap.add(other);
                fail("An exception must be thrown.");
            } catch (VTNException e) {
                assertEquals(StatusCode.CONFLICT, e.getStatus().getCode());
            } catch (Exception e) {
                unexpected(e);
            }
        }

        assertEquals(1, idmap.getSize());
        assertFalse(idmap.isEmpty());

        for (Node node: nodes) {
            if (node == null) {
                assertSame(vmap, idmap.get(node));
            } else {
                assertNull(idmap.get(node));
            }

            // A global VLAN mapping maps all nodes.
            assertSame(vmap, idmap.match(node));
        }

        for (Node node: nodes) {
            if (node == null) {
                continue;
            }
            assertFalse(idmap.remove(node));
            assertEquals(1, idmap.getSize());
            assertFalse(idmap.isEmpty());
        }

        assertTrue(idmap.remove(null));
        assertEquals(0, idmap.getSize());
        assertTrue(idmap.isEmpty());
    }

    /**
     * Test case for the following methods which handles VLAN mappings
     * specified by pairs of VLAN IDs and nodes.
     *
     * <ul>
     *   <li>{@link VlanIdMap#create(VlanMapImpl)}</li>
     *   <li>{@link VlanIdMap.NodeMap#getSize()}</li>
     *   <li>{@link VlanIdMap.NodeMap#isEmpty()}</li>
     *   <li>{@link VlanIdMap.NodeMap#add(VlanMapImpl)}</li>
     *   <li>{@link VlanIdMap.NodeMap#remove(Node)}</li>
     *   <li>{@link VlanIdMap.NodeMap#get(Node)}</li>
     *   <li>{@link VlanIdMap.NodeMap#match(Node)}</li>
     * </ul>
     */
    @Test
    public void testNodeMap() {
        VBridgePath bpath = new VBridgePath("tenant", "bridge");
        String mapId = "vmap";
        List<Node> nodes = createNodes(15, false);
        ArrayList<Node> otherNodes = new ArrayList<Node>();
        for (int i = 0; i < 5; i++) {
            otherNodes.add(nodes.remove(0));
        }

        int size = 0;
        VlanIdMap idmap = null;
        HashMap<Node, VlanMapImpl> vlanMaps = new HashMap<Node, VlanMapImpl>();
        for (Node node: nodes) {
            VlanMapConfig vlc = new VlanMapConfig(node, (short)0);
            VlanMapImpl vmap = new VlanMapImpl(bpath, mapId, vlc);
            assertNull(vlanMaps.put(node, vmap));

            if (idmap == null) {
                idmap = VlanIdMap.create(vmap);
                assertTrue(idmap instanceof VlanIdMap.NodeMap);
            } else {
                assertNull(idmap.get(node));
                assertNull(idmap.match(node));

                try {
                    idmap.add(vmap);
                } catch (Exception e) {
                    unexpected(e);
                }
            }

            size++;
            assertEquals(size, idmap.getSize());
            assertFalse(idmap.isEmpty());

            // Same VLAN mapping can not be added.
            try {
                idmap.add(vmap);
                fail("An exception must be thrown.");
            } catch (VTNException e) {
                assertEquals(StatusCode.CONFLICT, e.getStatus().getCode());
            } catch (Exception e) {
                unexpected(e);
            }

            assertEquals(size, idmap.getSize());
            assertFalse(idmap.isEmpty());
        }

        for (Node node: nodes) {
            assertSame(vlanMaps.get(node), idmap.get(node));
            assertSame(vlanMaps.get(node), idmap.match(node));
        }

        assertNull(idmap.get(null));
        for (Node node: otherNodes) {
            assertNull(idmap.get(node));
            assertNull(idmap.match(node));
        }

        // A global VLAN mapping can not be added.
        VlanMapConfig gvlc = new VlanMapConfig(null, (short)0);
        VlanMapImpl gvmap = new VlanMapImpl(bpath, mapId, gvlc);
        try {
            idmap.add(gvmap);
            fail("An exception must be thrown.");
        } catch (VTNException e) {
            assertEquals(StatusCode.CONFLICT, e.getStatus().getCode());
        } catch (Exception e) {
            unexpected(e);
        }

        // A VLAN mapping that maps the same node can not be added.
        for (Node node: nodes) {
            Node cnode = copy(node);
            VlanMapConfig vlc = new VlanMapConfig(cnode, (short)0);
            VlanMapImpl vmap = new VlanMapImpl(bpath, mapId, vlc);
            try {
                idmap.add(vmap);
                fail("An exception must be thrown.");
            } catch (VTNException e) {
                assertEquals(StatusCode.CONFLICT, e.getStatus().getCode());
            } catch (Exception e) {
                unexpected(e);
            }
        }

        for (Node node: nodes) {
            size--;
            boolean expected= (size == 0);
            assertFalse(idmap.remove(null));
            assertEquals(expected, idmap.remove(node));
            assertEquals(expected, idmap.isEmpty());
            assertEquals(size, idmap.getSize());
        }

        assertTrue(idmap.isEmpty());
        assertEquals(0, idmap.getSize());
    }

    /**
     * Test case for {@link VlanIdMap#equals(Object)} and
     * {@link VlanIdMap#hashCode()}.
     */
    @Test
    public void testGlobalEquals() {
        HashSet<Object> set = new HashSet<Object>();

        List<VlanIdMap> maps = new ArrayList<VlanIdMap>();
        List<VlanIdMap> clone = new ArrayList<VlanIdMap>();
        createVlanIdMap(maps, clone);
        assertEquals(clone.size(), maps.size());

        for (int i = 0; i < maps.size(); i++) {
            VlanIdMap idmap1 = maps.get(i);
            VlanIdMap idmap2 = clone.get(i);
            testEquals(set, idmap1, idmap2);
        }

        assertEquals(maps.size(), set.size());
    }

    /**
     * Ensure that {@link VlanIdMap} is serializable.
     */
    @Test
    public void testSerialize() {
        List<VlanIdMap> maps = new ArrayList<VlanIdMap>();
        createVlanIdMap(maps, null);
        for (VlanIdMap idmap: maps) {
            serializeTest(idmap);
        }
    }

    /**
     * Create {@link VlanIdMap} objects for testing.
     *
     * @param list   A list of {@link VlanIdMap} to store objects.
     * @param clone  Another list of {@link VlanIdMap} to store objects.
     *               If a non-{@code null} value is specified, all objects in
     *               {@code list} are copied to this list.
     */
    private void createVlanIdMap(List<VlanIdMap> list, List<VlanIdMap>clone) {
        VBridgePath bpath = new VBridgePath("tenant", "bridge");
        String mapId = "vmap";

        short[] vlans = {0, 1, 2, 100, 200, 1000, 2000, 4095};
        List<Node> nodes = createNodes(10, false);
        for (short vlan: vlans) {
            VlanMapConfig vlc = new VlanMapConfig(null, vlan);
            VlanMapImpl vmap = new VlanMapImpl(bpath, mapId, vlc);
            VlanIdMap idmap = VlanIdMap.create(vmap);
            assertTrue(idmap instanceof VlanIdMap.GlobalMap);
            list.add(idmap);

            if (clone != null) {
                vlc = new VlanMapConfig(null, vlan);
                vmap = new VlanMapImpl(bpath, mapId, vlc);
                idmap = VlanIdMap.create(vmap);
                assertTrue(idmap instanceof VlanIdMap.GlobalMap);
                clone.add(idmap);
            }

            for (int i = 0; i < nodes.size(); i++) {
                Node node = nodes.get(i);
                vlc = new VlanMapConfig(node, vlan);
                vmap = new VlanMapImpl(bpath, mapId, vlc);
                idmap = VlanIdMap.create(vmap);
                assertTrue(idmap instanceof VlanIdMap.NodeMap);
                list.add(idmap);

                VlanIdMap idmap1 = null;
                if (clone != null) {
                    vlc = new VlanMapConfig(copy(node), vlan);
                    vmap = new VlanMapImpl(bpath, mapId, vlc);
                    idmap1 = VlanIdMap.create(vmap);
                    assertTrue(idmap1 instanceof VlanIdMap.NodeMap);
                    clone.add(idmap1);
                }

                try {
                    for (int j = 0; j < i; j++) {
                        Node nd = nodes.get(j);
                        vlc = new VlanMapConfig(nd, vlan);
                        vmap = new VlanMapImpl(bpath, mapId, vlc);
                        idmap.add(vmap);

                        if (idmap1 != null) {
                            vlc = new VlanMapConfig(copy(nd), vlan);
                            vmap = new VlanMapImpl(bpath, mapId, vlc);
                            idmap1.add(vmap);
                        }
                    }
                } catch (Exception e) {
                    unexpected(e);
                }
            }
        }
    }
}
