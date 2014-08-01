/*
 * Copyright (c) 2013-2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager;

import java.util.HashSet;
import java.util.List;

import org.junit.Test;

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

                    path = new VBridgeIfPath(bpath, iname);
                    assertEquals(tname, path.getTenantName());
                    assertEquals(bname, path.getBridgeName());
                    assertEquals(iname, path.getInterfaceName());
                    assertEquals("vBridge-IF", path.getNodeType());
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
}
