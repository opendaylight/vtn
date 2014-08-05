/*
 * Copyright (c) 2014 NEC Corporation
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
 * JUnit test for {@link VTerminalIfPath}.
 */
public class VTerminalIfPathTest extends TestBase {
    /**
     * Test case for getter methods.
     */
    @Test
    public void testGetter() {
        for (String tname: createStrings("tenant")) {
            for (String vtname: createStrings("term")) {
                VTerminalPath vtpath = new VTerminalPath(tname, vtname);
                for (String iname: createStrings("ifname")) {
                    VTerminalIfPath path =
                        new VTerminalIfPath(tname, vtname, iname);
                    assertEquals(tname, path.getTenantName());
                    assertEquals(vtname, path.getTerminalName());
                    assertEquals(iname, path.getInterfaceName());
                    assertEquals("vTerminal-IF", path.getNodeType());

                    path = new VTerminalIfPath(vtpath, iname);
                    assertEquals(tname, path.getTenantName());
                    assertEquals(vtname, path.getTerminalName());
                    assertEquals(iname, path.getInterfaceName());
                    assertEquals("vTerminal-IF", path.getNodeType());

                    VTenantPath clone = path.clone();
                    assertNotSame(clone, path);
                    assertEquals(clone, path);

                    String name = tname + "_new";
                    VTerminalIfPath path1 =
                        (VTerminalIfPath)path.replaceTenantName(name);
                    assertEquals(name, path1.getTenantName());
                    assertEquals(vtname, path1.getTerminalName());
                    assertEquals(iname, path1.getInterfaceName());
                    assertEquals("vTerminal-IF", path1.getNodeType());
                    assertEquals(VTerminalIfPath.class, path1.getClass());
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
            for (String vtname: createStrings("term")) {
                for (String iname: createStrings("ifname")) {
                    VTerminalIfPath path =
                        new VTerminalIfPath(tname, vtname, iname);
                    assertFalse(path.contains(null));
                    assertTrue(path.contains(path));

                    String notMatched = "not_matched";
                    containsTest(path, tname, vtname, iname, true);
                    containsTest(path, notMatched, vtname, iname, false);
                    containsTest(path, tname, notMatched, iname, false);
                    containsTest(path, tname, vtname, notMatched, false);
                }
            }
        }
    }

    /**
     * Test case for {@link VTerminalIfPath#equals(Object)} and
     * {@link VTerminalIfPath#hashCode()}.
     */
    @Test
    public void testEquals() {
        HashSet<Object> set = new HashSet<Object>();
        List<String> tnames = createStrings("tenant");
        List<String> vtnames = createStrings("term");
        List<String> inames = createStrings("ifname");
        for (String tname: tnames) {
            VTenantPath tpath = new VTenantPath(tname);
            for (String vtname: vtnames) {
                VTerminalPath vtpath = new VTerminalPath(tname, vtname);
                VBridgePath bpath = new VBridgePath(tname, vtname);
                for (String iname: inames) {
                    VTerminalIfPath p1 =
                        new VTerminalIfPath(tname, vtname, iname);
                    VTerminalIfPath p2 =
                        new VTerminalIfPath(copy(tname), copy(vtname),
                                            copy(iname));
                    testEquals(set, p1, p2);

                    // An instance of VTenantPath, VBridgePath, VTerminalPath,
                    // and VBridgeIfPath must be treated as different object
                    // even if it has the same path component.
                    VBridgeIfPath bifpath =
                        new VBridgeIfPath(tname, vtname, iname);
                    assertFalse(tpath.equals(p1));
                    assertFalse(p1.equals(tpath));
                    assertFalse(vtpath.equals(p1));
                    assertFalse(p1.equals(vtpath));
                    assertFalse(bpath.equals(p1));
                    assertFalse(p1.equals(bpath));
                    assertFalse(bifpath.equals(p1));
                    assertFalse(p1.equals(bifpath));
                }
            }
        }

        int required = tnames.size() * vtnames.size() * inames.size();
        assertEquals(required, set.size());
    }

    /**
     * Test case for {@link VTerminalIfPath#toString()}.
     */
    @Test
    public void testToString() {
        String prefix = "vTerminal-IF:";
        for (String tname: createStrings("tenant")) {
            for (String vtname: createStrings("term")) {
                for (String iname: createStrings("ifname")) {
                    VTerminalIfPath path =
                        new VTerminalIfPath(tname, vtname, iname);
                    String tn = (tname == null) ? "<null>" : tname;
                    String vn = (vtname == null) ? "<null>" : vtname;
                    String in = (iname == null) ? "<null>" : iname;
                    String required =
                        joinStrings(prefix, null, ".", tn, vn, in);
                    assertEquals(required, path.toString());
                }
            }
        }
    }

    /**
     * Ensure that {@link VTerminalIfPath} is serializable.
     */
    @Test
    public void testSerialize() {
        for (String tname: createStrings("tenant")) {
            for (String vtname: createStrings("term")) {
                for (String iname: createStrings("ifname")) {
                    VTerminalIfPath path =
                        new VTerminalIfPath(tname, vtname, iname);
                    serializeTest(path);
                }
            }
        }
    }

    /**
     * Internal method for {@link #testContains()}.
     *
     * @param path      A {@link VTerminalIfPath} instance to be tested.
     * @param tname     The name of the tenant for test.
     * @param vtname    The name of the vBridge for test.
     * @param iname     The name of the interface for test.
     * @param expected  Expected result.
     */
    private void containsTest(VTerminalIfPath path, String tname,
                              String vtname, String iname, boolean expected) {
        VTenantPath tpath = new VTenantPath(tname);
        assertEquals(false, path.contains(tpath));

        VBridgePath vtpath = new VBridgePath(tpath, vtname);
        assertEquals(false, path.contains(vtpath));

        VTerminalPath tmpath = new VTerminalPath(tpath, vtname);
        assertEquals(false, path.contains(tmpath));

        VBridgeIfPath bipath = new VBridgeIfPath(tname, vtname, iname);
        assertEquals(false, path.contains(bipath));

        VTerminalIfPath ipath = new VTerminalIfPath(tmpath, iname);
        assertEquals(expected, path.contains(ipath));
    }
}
