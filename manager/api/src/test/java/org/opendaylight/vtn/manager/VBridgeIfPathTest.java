/*
 * Copyright (c) 2013, 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Objects;

import org.junit.Test;

import org.opendaylight.vtn.manager.flow.filter.RedirectFilter;

import org.opendaylight.controller.sal.core.Node;
import org.opendaylight.controller.sal.core.NodeConnector;
import org.opendaylight.controller.sal.core.UpdateType;
import org.opendaylight.controller.sal.utils.NodeConnectorCreator;
import org.opendaylight.controller.sal.utils.NodeCreator;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.virtual.route.info.VirtualNodePath;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeState;

/**
 * JUnit test for {@link VBridgeIfPath}.
 */
public class VBridgeIfPathTest extends TestBase {
    /**
     * Test case for getter methods.
     */
    @Test
    public void testGetter() {
        for (String tname: createStrings("tenant")) {
            for (String bname: createStrings("bridge")) {
                VBridgePath bpath = new VBridgePath(tname, bname);
                for (String iname: createStrings("ifname")) {
                    VBridgeIfPath path =
                        new VBridgeIfPath(tname, bname, iname);
                    assertEquals(tname, path.getTenantName());
                    assertEquals(bname, path.getBridgeName());
                    assertEquals(iname, path.getInterfaceName());
                    assertEquals("vBridge-IF", path.getNodeType());
                    checkRedirectFilter(path);
                    checkVirtualNodePath(path);

                    path = new VBridgeIfPath(bpath, iname);
                    assertEquals(tname, path.getTenantName());
                    assertEquals(bname, path.getBridgeName());
                    assertEquals(iname, path.getInterfaceName());
                    assertEquals("vBridge-IF", path.getNodeType());
                    checkRedirectFilter(path);
                    checkVirtualNodePath(path);
                }
            }
        }
    }

    /**
     * Test case for {@link VBridgeIfPath#clone()}.
     */
    @Test
    public void testClone() {
        for (String tname: createStrings("tenant")) {
            for (String bname: createStrings("bridge")) {
                VBridgePath bpath = new VBridgePath(tname, bname);
                for (String iname: createStrings("ifname")) {
                    // In case where the hash code is cached.
                    VBridgeIfPath path =
                        new VBridgeIfPath(tname, bname, iname);
                    int hash = path.hashCode();

                    VTenantPath clone = path.clone();
                    assertNotSame(path, clone);
                    assertEquals(path, clone);
                    assertEquals(hash, clone.hashCode());
                    checkVirtualNodePath(clone);
                    assertEquals(VBridgeIfPath.class, clone.getClass());
                    VBridgeIfPath p = (VBridgeIfPath)clone;
                    assertEquals(tname, p.getTenantName());
                    assertEquals(bname, p.getBridgeName());
                    assertEquals(iname, p.getInterfaceName());

                    // In case where the hash code is not cached.
                    path = new VBridgeIfPath(tname, bname, iname);
                    clone = path.clone();
                    p = (VBridgeIfPath)clone;
                    assertNotSame(path, clone);
                    assertEquals(tname, p.getTenantName());
                    assertEquals(bname, p.getBridgeName());
                    assertEquals(iname, p.getInterfaceName());
                    assertEquals(path, clone);
                    assertEquals(hash, clone.hashCode());
                    checkVirtualNodePath(clone);
                }
            }
        }
    }

    /**
     * Test case for {@link VBridgeIfPath#replaceTenantName(String)}.
     */
    @Test
    public void testReplaceTenantName() {
        for (String tname: createStrings("tenant")) {
            for (String bname: createStrings("bridge")) {
                VBridgePath bpath = new VBridgePath(tname, bname);
                for (String iname: createStrings("ifname")) {
                    // In case where the hash code is cached.
                    VBridgeIfPath path =
                        new VBridgeIfPath(tname, bname, iname);
                    int hash = path.hashCode();

                    Map<String, Integer> hashCodes = new HashMap<>();
                    String[] names = {null, tname + "_new"};
                    for (String name: names) {
                        VBridgeIfPath path1 = path.replaceTenantName(name);
                        assertEquals(name, path1.getTenantName());
                        assertEquals(bname, path1.getBridgeName());
                        assertEquals(iname, path1.getInterfaceName());
                        assertEquals("vBridge-IF", path1.getNodeType());
                        checkRedirectFilter(path1);
                        checkVirtualNodePath(path1);
                        int hash1 = path1.hashCode();
                        assertEquals(null, hashCodes.put(name, hash1));
                        if (Objects.equals(name, tname)) {
                            assertEquals(hash, hash1);
                        } else {
                            assertNotEquals(hash, hash1);
                        }
                    }

                    // In case where the hash code is not cached.
                    path = new VBridgeIfPath(tname, bname, iname);
                    for (String name: names) {
                        VBridgeIfPath path1 = path.replaceTenantName(name);
                        assertEquals(name, path1.getTenantName());
                        assertEquals(bname, path1.getBridgeName());
                        assertEquals(iname, path1.getInterfaceName());
                        assertEquals("vBridge-IF", path1.getNodeType());
                        checkRedirectFilter(path1);
                        checkVirtualNodePath(path1);
                        assertEquals(hashCodes.get(name).intValue(),
                                     path1.hashCode());
                    }
                }
            }
        }
    }

    /**
     * Ensure that {@link VTenantPath#contains(VTenantPath)} works.
     */
    @Test
    public void testContains() {
        for (String tname: createStrings("tenant")) {
            for (String bname: createStrings("bridge")) {
                for (String iname: createStrings("ifname")) {
                    VBridgeIfPath path =
                        new VBridgeIfPath(tname, bname, iname);
                    assertFalse(path.contains(null));
                    assertTrue(path.contains(path));

                    String notMatched = "not_matched";
                    containsTest(path, tname, bname, iname, true);
                    containsTest(path, notMatched, bname, iname, false);
                    containsTest(path, tname, notMatched, iname, false);
                    containsTest(path, tname, bname, notMatched, false);
                }
            }
        }
    }

    /**
     * Test case for {@link VBridgeIfPath#vInterfaceChanged(IVTNManagerAware,VInterface,UpdateType)}.
     */
    @Test
    public void testVInterfaceChanged() {
        TestVTNManagerAware listener = new TestVTNManagerAware();
        String iname = "if_1";
        VBridgeIfPath path = new VBridgeIfPath("tenant", "vbridge", iname);
        VInterfaceConfig iconf = new VInterfaceConfig("vBridge 1", null);
        VInterface vif = new VInterface(iname, VnodeState.DOWN,
                                        VnodeState.UNKNOWN, iconf);
        UpdateType type = UpdateType.ADDED;
        List<List<Object>> expected = new ArrayList<List<Object>>();
        expected.add(toList(path, vif, type));
        path.vInterfaceChanged(listener, vif, type);

        vif = new VInterface(iname, VnodeState.UP, VnodeState.UP, iconf);
        type = UpdateType.CHANGED;
        expected.add(toList(path, vif, type));
        path.vInterfaceChanged(listener, vif, type);

        type = UpdateType.REMOVED;
        expected.add(toList(path, vif, type));
        path.vInterfaceChanged(listener, vif, type);

        for (VTNListenerType ltype: VTNListenerType.values()) {
            List<List<Object>> called = listener.getArguments(ltype);

            switch (ltype) {
            case VBRIDGE_IF:
                assertEquals(expected, called);
                break;

            default:
                assertEquals(null, called);
                break;
            }
        }
    }

    /**
     * Test case for {@link VBridgeIfPath#portMapChanged(IVTNManagerAware,PortMap,UpdateType)}.
     */
    @Test
    public void testPortMapChanged() {
        TestVTNManagerAware listener = new TestVTNManagerAware();
        String iname = "if_1";
        VBridgeIfPath path = new VBridgeIfPath("tenant", "vbridge", iname);
        Node node = NodeCreator.createOFNode(Long.valueOf(123L));
        NodeConnector port = NodeConnectorCreator.
            createOFNodeConnector(Short.valueOf((short)1), node);
        SwitchPort swport = new SwitchPort("OF", "1");
        PortMapConfig pmconf = new PortMapConfig(node, swport, (short)0);
        PortMap pmap = new PortMap(pmconf, null);
        UpdateType type = UpdateType.ADDED;
        List<List<Object>> expected = new ArrayList<List<Object>>();
        expected.add(toList(path, pmap, type));
        path.portMapChanged(listener, pmap, type);

        pmap = new PortMap(pmconf, port);
        type = UpdateType.CHANGED;
        expected.add(toList(path, pmap, type));
        path.portMapChanged(listener, pmap, type);

        type = UpdateType.REMOVED;
        expected.add(toList(path, pmap, type));
        path.portMapChanged(listener, pmap, type);

        for (VTNListenerType ltype: VTNListenerType.values()) {
            List<List<Object>> called = listener.getArguments(ltype);

            switch (ltype) {
            case PORTMAP:
                assertEquals(expected, called);
                break;

            default:
                assertEquals(null, called);
                break;
            }
        }
    }

    /**
     * Test case for {@link VBridgeIfPath#equals(Object)} and
     * {@link VBridgeIfPath#hashCode()}.
     */
    @Test
    public void testEquals() {
        HashSet<Object> set = new HashSet<Object>();
        List<String> tnames = createStrings("tenant");
        List<String> bnames = createStrings("bridge");
        List<String> inames = createStrings("ifname");
        for (String tname: tnames) {
            VTenantPath tpath = new VTenantPath(tname);
            for (String bname: bnames) {
                VBridgePath bpath = new VBridgePath(tname, bname);
                VTerminalPath tmpath = new VTerminalPath(tname, bname);
                for (String iname: inames) {
                    VBridgeIfPath p1 = new VBridgeIfPath(tname, bname, iname);
                    VBridgeIfPath p2 =
                        new VBridgeIfPath(copy(tname), copy(bname),
                                          copy(iname));
                    testEquals(set, p1, p2);

                    // An instance of VTenantPath, VBridgePath, VTerminalPath,
                    // and VTerminalIfPath must be treated as different object
                    // even if it has the same path component.
                    VTerminalIfPath tifpath =
                        new VTerminalIfPath(tname, bname, iname);
                    assertFalse(tpath.equals(p1));
                    assertFalse(p1.equals(tpath));
                    assertFalse(bpath.equals(p1));
                    assertFalse(p1.equals(bpath));
                    assertFalse(tmpath.equals(p1));
                    assertFalse(p1.equals(tmpath));
                    assertFalse(tifpath.equals(p1));
                    assertFalse(p1.equals(tifpath));
                }
            }
        }

        int required = tnames.size() * bnames.size() * inames.size();
        assertEquals(required, set.size());
    }

    /**
     * Test case for {@link VBridgeIfPath#toString()}.
     */
    @Test
    public void testToString() {
        String prefix = "vBridge-IF:";
        for (String tname: createStrings("tenant")) {
            for (String bname: createStrings("bridge")) {
                for (String iname: createStrings("ifname")) {
                    VBridgeIfPath path =
                        new VBridgeIfPath(tname, bname, iname);
                    String tn = (tname == null) ? "<null>" : tname;
                    String bn = (bname == null) ? "<null>" : bname;
                    String in = (iname == null) ? "<null>" : iname;
                    String required =
                        joinStrings(prefix, null, ".", tn, bn, in);
                    assertEquals(required, path.toString());
                }
            }
        }
    }

    /**
     * Ensure that {@link VBridgeIfPath} is serializable.
     */
    @Test
    public void testSerialize() {
        for (String tname: createStrings("tenant")) {
            for (String bname: createStrings("bridge")) {
                for (String iname: createStrings("ifname")) {
                    VBridgeIfPath path =
                        new VBridgeIfPath(tname, bname, iname);
                    serializeTest(path);
                }
            }
        }
    }

    /**
     * Internal method for {@link #testContains()}.
     *
     * @param path      A {@link VBridgeIfPath} instance to be tested.
     * @param tname     The name of the tenant for test.
     * @param bname     The name of the vBridge for test.
     * @param iname     The name of the interface for test.
     * @param expected  Expected result.
     */
    private void containsTest(VBridgeIfPath path, String tname, String bname,
                              String iname, boolean expected) {
        VTenantPath tpath = new VTenantPath(tname);
        assertEquals(false, path.contains(tpath));

        VBridgePath bpath = new VBridgePath(tpath, bname);
        assertEquals(false, path.contains(bpath));

        VTerminalPath tmpath = new VTerminalPath(tpath, bname);
        assertEquals(false, path.contains(tmpath));

        VTerminalIfPath tipath = new VTerminalIfPath(tname, bname, iname);
        assertEquals(false, path.contains(tipath));

        VBridgeIfPath ipath = new VBridgeIfPath(bpath, iname);
        assertEquals(expected, path.contains(ipath));
    }

    /**
     * Ensure that {@link VBridgeIfPath#getRedirectFilter(boolean)} works
     * correctly.
     *
     * @param path  A {@link VBridgeIfPath} instance.
     */
    private void checkRedirectFilter(VBridgeIfPath path) {
        boolean[] bools = {true, false};
        for (boolean b: bools) {
            RedirectFilter rf = path.getRedirectFilter(b);
            assertEquals(path, rf.getDestination());
            assertEquals(b, rf.isOutput());
        }
    }

    /**
     * Test case for {@link VBridgeIfPath#toVirtualNodePath()}.
     *
     * @param path  A {@link VTenantPath} to be tested.
     */
    private void checkVirtualNodePath(VTenantPath path) {
        assertTrue(path instanceof VBridgeIfPath);
        VBridgeIfPath bipath = (VBridgeIfPath)path;

        VirtualNodePath vpath = path.toVirtualNodePath();
        assertEquals(bipath.getTenantName(), vpath.getTenantName());
        assertEquals(bipath.getBridgeName(), vpath.getBridgeName());
        assertEquals(bipath.getInterfaceName(), vpath.getInterfaceName());
        assertEquals(null, vpath.getRouterName());
        assertEquals(null, vpath.getTerminalName());
    }
}
