/*
 * Copyright (c) 2013, 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.adsal;

import java.util.HashSet;
import java.util.List;

import org.junit.Test;

import org.opendaylight.vtn.manager.VBridgeIfPath;
import org.opendaylight.vtn.manager.VBridgePath;
import org.opendaylight.vtn.manager.VTenantPath;
import org.opendaylight.vtn.manager.VTerminalPath;

import org.opendaylight.vtn.manager.internal.TestBase;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.BridgeMapInfo;

/**
 * JUnit test for {@link VlanMapPath}.
 */
public class VlanMapPathTest extends TestBase {
    /**
     * Test case for getter methods.
     */
    @Test
    public void testGetter() {
        for (String tname: createStrings("tenant")) {
            for (String bname: createStrings("bridge")) {
                VBridgePath bpath = new VBridgePath(tname, bname);
                for (String mapId: createStrings("mapId", false)) {
                    VlanMapPath path = new VlanMapPath(bpath, mapId);
                    String emsg = path.toString();
                    assertEquals(emsg, tname, path.getTenantName());
                    assertEquals(emsg, bname, path.getBridgeName());
                    assertEquals(emsg, mapId, path.getMapId());
                    assertEquals("VlanMap", path.getNodeType());
                }
            }
        }
    }

    /**
     * Test case for {@link VlanMapPath#getBridgeMapInfo()}.
     */
    @Test
    public void testGetBridgeMapInfo() {
        String tname = "vtn";
        String bname = "vbr";
        VBridgePath bpath = new VBridgePath(tname, bname);
        for (String mapId: createStrings("mapId", false)) {
            VlanMapPath path = new VlanMapPath(bpath, mapId);
            BridgeMapInfo minfo = path.getBridgeMapInfo();
            assertEquals(mapId, minfo.getVlanMapId());
            assertEquals(null, minfo.getMacMappedHost());
        }
    }

    /**
     * Ensure that {@link VTenantPath#contains(VTenantPath)} works.
     */
    @Test
    public void testContains() {
        for (String tname: createStrings("tenant")) {
            for (String bname: createStrings("bridge")) {
                VBridgePath bpath = new VBridgePath(tname, bname);
                for (String mapId: createStrings("mapId", false)) {
                    VlanMapPath path = new VlanMapPath(bpath, mapId);
                    assertFalse(path.contains(null));
                    assertTrue(path.contains(path));

                    String notMatched = "not_matched";
                    containsTest(path, tname, bname, mapId, true);
                    containsTest(path, notMatched, bname, mapId, false);
                    containsTest(path, tname, notMatched, mapId, false);
                    containsTest(path, tname, bname, notMatched, false);
                }
            }
        }
    }

    /**
     * Test case for {@link VlanMapPath#equals(Object)} and
     * {@link VlanMapPath#hashCode()}.
     */
    @Test
    public void testEquals() {
        HashSet<Object> set = new HashSet<Object>();
        List<String> tnames = createStrings("tenant");
        List<String> bnames = createStrings("bridge");
        List<String> mapIds = createStrings("mapId", false);
        for (String tname: tnames) {
            VTenantPath tpath = new VTenantPath(tname);
            for (String bname: bnames) {
                VBridgePath bp1 = new VBridgePath(tname, bname);
                VBridgePath bp2 = new VBridgePath(copy(tname), copy(bname));
                for (String mapId: mapIds) {
                    VlanMapPath p1 = new VlanMapPath(bp1, mapId);
                    VlanMapPath p2 = new VlanMapPath(bp2, copy(mapId));
                    testEquals(set, p1, p2);

                    VBridgeIfPath ifPath = new VBridgeIfPath(bp1, mapId);
                    assertFalse("(p1)" + p1.toString() + ",(ifPath)" + ifPath.toString(),
                            p1.equals(ifPath));
                    assertFalse("(p2)" + p2.toString() + ",(ifPath)" + ifPath.toString(),
                            p2.equals(ifPath));
                    assertFalse("(p1)" + p1.toString() + ",(ifPath)" + ifPath.toString(),
                            ifPath.equals(p1));
                    assertFalse("(p2)" + p2.toString() + ",(ifPath)" + ifPath.toString(),
                            ifPath.equals(p2));

                    // A instance of VTenantPath, VBridgePath must be treated
                    // as different object even if it has the same path
                    // component.
                    assertFalse(p1.equals(tpath));
                    assertFalse(tpath.equals(p1));

                    assertFalse(p1.equals(bp1));
                    assertFalse(bp1.equals(p1));
                }
            }
        }

        int required = tnames.size() * bnames.size() * mapIds.size();
        assertEquals(required, set.size());
    }

    /**
     * Test case which verifies that {@link VTenantPath#compareTo(VTenantPath)}
     * can handle {@link VlanMapPath} and {@link MacMapPath} instance.
     */
    @Test
    public void testCompareTo() {
        int size = 0;
        HashSet<VTenantPath> set = new HashSet<VTenantPath>();
        for (String tname: createStrings("tenant_name")) {
            VTenantPath path = new VTenantPath(tname);
            assertTrue(set.add(path));
            assertFalse(set.add(path));
            size++;

            // VTenantPath.compareTo() can accept VTenantPath variants.
            for (String bname: createStrings("bridge_name")) {
                VBridgePath bpath = new VBridgePath(path, bname);
                assertTrue(set.add(bpath));
                assertFalse(set.add(bpath));
                size++;

                VTerminalPath tmpath = new VTerminalPath(path, bname);
                assertTrue(set.add(tmpath));
                assertFalse(set.add(tmpath));
                size++;

                MacMapPath mpath = new MacMapPath(bpath);
                assertTrue(set.add(mpath));
                assertFalse(set.add(mpath));
                size++;

                for (String iname: createStrings("interface_name")) {
                    VBridgeIfPath ipath = new VBridgeIfPath(bpath, iname);
                    assertTrue(set.add(ipath));
                    assertFalse(set.add(ipath));
                    size++;

                    if (iname != null) {
                        VlanMapPath vpath = new VlanMapPath(bpath, iname);
                        assertTrue(set.add(vpath));
                        assertFalse(set.add(vpath));
                        size++;
                    }
                }
            }
        }

        assertEquals(size, set.size());
        comparableTest(set);
    }

    /**
     * Test case for {@link VlanMapPath#toString()}.
     */
    @Test
    public void testToString() {
        String prefix = "VlanMap:";
        for (String tname: createStrings("tenant")) {
            for (String bname: createStrings("bridge")) {
                VBridgePath bpath = new VBridgePath(tname, bname);
                for (String mapId: createStrings("mapId", false)) {
                    VlanMapPath path = new VlanMapPath(bpath, mapId);
                    String tn = (tname == null) ? "<null>" : tname;
                    String bn = (bname == null) ? "<null>" : bname;
                    String required =
                        joinStrings(prefix, null, ".", tn, bn, mapId);
                    assertEquals(required, path.toString());
                }
            }
        }
    }

    /**
     * Internal method for {@link #testContains()}.
     *
     * @param path      A {@link VBridgePath} instance to be tested.
     * @param tname     The name of the tenant for test.
     * @param bname     The name of the vBridge for test.
     * @param mapId     The identifier of the VLAN mapping for test.
     * @param expected  Expected result.
     */
    private void containsTest(VBridgePath path, String tname, String bname,
                              String mapId, boolean expected) {
        VTenantPath tpath = new VTenantPath(tname);
        boolean sameTenant = equals(tname, path.getTenantName());
        assertEquals(false, path.contains(tpath));
        assertEquals(sameTenant, tpath.contains(path));

        VBridgePath bpath = new VBridgePath(tpath, bname);
        boolean sameBridge =
            (sameTenant && equals(bname, path.getBridgeName()));
        assertEquals(false, path.contains(bpath));
        assertEquals(sameBridge, bpath.contains(path));

        VTerminalPath tmpath = new VTerminalPath(tpath, bname);
        assertEquals(false, path.contains(tmpath));
        assertEquals(false, tmpath.contains(path));

        VBridgeIfPath ipath = new VBridgeIfPath(bpath, mapId);
        assertEquals(false, path.contains(ipath));
        assertEquals(false, ipath.contains(path));

        VlanMapPath vpath = new VlanMapPath(bpath, mapId);
        assertEquals(expected, path.contains(vpath));
        assertEquals(expected, vpath.contains(path));
    }
}
