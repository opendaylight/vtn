/*
 * Copyright (c) 2014, 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager;

import java.util.HashSet;
import java.util.List;

import org.junit.Test;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.virtual.route.info.VirtualNodePath;

/**
 * JUnit test for {@link VTerminalPath}.
 */
public class VTerminalPathTest extends TestBase {
    /**
     * Test case for getter methods.
     */
    @Test
    public void testGetter() {
        for (String tname: createStrings("tenant")) {
            VTenantPath tpath = new VTenantPath(tname);
            for (String mname: createStrings("term")) {
                VTerminalPath path = new VTerminalPath(tname, mname);
                assertEquals(tname, path.getTenantName());
                assertEquals(mname, path.getTerminalName());
                assertEquals("vTerminal", path.getNodeType());
                checkVirtualNodePath(path);

                path = new VTerminalPath(tpath, mname);
                assertEquals(tname, path.getTenantName());
                assertEquals(mname, path.getTerminalName());
                assertEquals("vTerminal", path.getNodeType());
                checkVirtualNodePath(path);

                VTenantPath clone = path.clone();
                assertNotSame(path, clone);
                assertEquals(path, clone);
                checkVirtualNodePath(clone);

                String name = tname + "_new";
                VTerminalPath path1 =
                    (VTerminalPath)path.replaceTenantName(name);
                assertEquals(name, path1.getTenantName());
                assertEquals(mname, path1.getTerminalName());
                assertEquals("vTerminal", path1.getNodeType());
                assertEquals(VTerminalPath.class, path1.getClass());
                checkVirtualNodePath(path1);
            }
        }
    }

    /**
     * Test case for {@link VTerminalPath#clone()}.
     */
    @Test
    public void testClone() {
        for (String tname: createStrings("tenant")) {
            VTenantPath tpath = new VTenantPath(tname);
            for (String mname: createStrings("term")) {
                // In case where the hash code is cached.
                VTerminalPath path = new VTerminalPath(tname, mname);
                int hash = path.hashCode();

                VTenantPath clone = path.clone();
                assertNotSame(path, clone);
                assertEquals(path, clone);
                assertEquals(hash, clone.hashCode());
                checkVirtualNodePath(clone);
                assertEquals(VTerminalPath.class, clone.getClass());
                VTerminalPath p = (VTerminalPath)clone;
                assertEquals(tname, p.getTenantName());
                assertEquals(mname, p.getTerminalName());

                // In case where the hash code is not cached.
                path = new VTerminalPath(tname, mname);
                clone = path.clone();
                assertNotSame(path, clone);
                assertEquals(path, clone);
                assertEquals(hash, clone.hashCode());
                checkVirtualNodePath(clone);
                assertEquals(VTerminalPath.class, clone.getClass());
                p = (VTerminalPath)clone;
                assertEquals(tname, p.getTenantName());
                assertEquals(mname, p.getTerminalName());
            }
        }
    }

    /**
     * Test case for {@link VTerminalPath#replaceTenantName(String)}.
     */
    @Test
    public void testReplaceTenantName() {
        for (String tname: createStrings("tenant")) {
            VTenantPath tpath = new VTenantPath(tname);
            for (String mname: createStrings("term")) {
                // In case where the hash code is cached.
                VTerminalPath path = new VTerminalPath(tname, mname);
                int hash = path.hashCode();

                String name = tname + "_new";
                VTerminalPath path1 =
                    (VTerminalPath)path.replaceTenantName(name);
                assertEquals(name, path1.getTenantName());
                assertEquals(mname, path1.getTerminalName());
                assertEquals("vTerminal", path1.getNodeType());
                assertEquals(VTerminalPath.class, path1.getClass());
                checkVirtualNodePath(path1);
                int hash1 = path1.hashCode();
                assertNotEquals(hash, hash1);

                // In case where the hash code is not cached.
                path = new VTerminalPath(tname, mname);
                path1 = (VTerminalPath)path.replaceTenantName(name);
                assertEquals(name, path1.getTenantName());
                assertEquals(mname, path1.getTerminalName());
                assertEquals("vTerminal", path1.getNodeType());
                assertEquals(VTerminalPath.class, path1.getClass());
                checkVirtualNodePath(path1);
                assertEquals(hash1, path1.hashCode());
            }
        }
    }

    /**
     * Ensure that {@link VTenantPath#contains(VTenantPath)} works.
     */
    @Test
    public void testContains() {
        for (String tname: createStrings("tenant")) {
            for (String mname: createStrings("term")) {
                VTerminalPath path = new VTerminalPath(tname, mname);
                assertFalse(path.contains(null));
                assertTrue(path.contains(path));

                String notMatched = "not_matched";
                containsTest(path, tname, mname, true);
                containsTest(path, notMatched, mname, false);
                containsTest(path, tname, notMatched, false);
            }
        }
    }

    /**
     * Test case for {@link VTerminalPath#equals(Object)} and
     * {@link VTerminalPath#hashCode()}.
     */
    @Test
    public void testEquals() {
        HashSet<Object> set = new HashSet<Object>();
        List<String> tnames = createStrings("tenant");
        List<String> mnames = createStrings("term");
        for (String tname: tnames) {
            VTenantPath tpath = new VTenantPath(tname);
            for (String mname: mnames) {
                VTerminalPath p1 = new VTerminalPath(tname, mname);
                VTerminalPath p2 = new VTerminalPath(copy(tname), copy(mname));
                testEquals(set, p1, p2);

                // VTenantPath object that has the same tenant name must be
                // treated as different object.
                assertFalse(p1.equals(tpath));
                assertFalse(tpath.equals(p1));

                // VBridgePath object that has the same path components must
                // be treated as different object.
                VBridgePath bpath = new VBridgePath(tname, mname);
                assertFalse(p1.equals(bpath));
                assertFalse(bpath.equals(p1));
            }
        }

        int required = tnames.size() * mnames.size();
        assertEquals(required, set.size());
    }

    /**
     * Test case for {@link VTerminalPath#toString()}.
     */
    @Test
    public void testToString() {
        String prefix = "vTerminal:";
        for (String tname: createStrings("tenant")) {
            for (String mname: createStrings("term")) {
                VTerminalPath path = new VTerminalPath(tname, mname);
                String tn = (tname == null) ? "<null>" : tname;
                String bn = (mname == null) ? "<null>" : mname;
                String required = joinStrings(prefix, null, ".", tn, bn);
                assertEquals(required, path.toString());
            }
        }
    }

    /**
     * Ensure that {@link VTerminalPath} is serializable.
     */
    @Test
    public void testSerialize() {
        for (String tname: createStrings("tenant")) {
            for (String mname: createStrings("term")) {
                VTerminalPath path = new VTerminalPath(tname, mname);
                serializeTest(path);
            }
        }
    }

    /**
     * Internal method for {@link #testContains()}.
     *
     * @param path      A {@link VTerminalPath} instance to be tested.
     * @param tname     The name of the tenant for test.
     * @param mname     The name of the vTerminal for test.
     * @param expected  Expected result.
     */
    private void containsTest(VTerminalPath path, String tname, String mname,
                              boolean expected) {
        VTenantPath tpath = new VTenantPath(tname);
        assertEquals(false, path.contains(tpath));

        VBridgePath bpath = new VBridgePath(tpath, mname);
        assertEquals(false, path.contains(bpath));

        VTerminalPath tmpath = new VTerminalPath(tpath, mname);
        assertEquals(expected, path.contains(tmpath));

        VBridgeIfPath bipath = new VBridgeIfPath(bpath, "interface");
        assertEquals(false, path.contains(bipath));

        VTerminalIfPath ipath = new VTerminalIfPath(tmpath, "interface");
        assertEquals(expected, path.contains(ipath));
    }

    /**
     * Test case for {@link VTerminalPath#toVirtualNodePath()}.
     *
     * @param path  A {@link VTenantPath} to be tested.
     */
    private void checkVirtualNodePath(VTenantPath path) {
        assertTrue(path instanceof VTerminalPath);
        VTerminalPath tpath = (VTerminalPath)path;

        VirtualNodePath vpath = path.toVirtualNodePath();
        assertEquals(tpath.getTenantName(), vpath.getTenantName());
        assertEquals(tpath.getTerminalName(), vpath.getTerminalName());
        assertEquals(null, vpath.getInterfaceName());
        assertEquals(null, vpath.getRouterName());
        assertEquals(null, vpath.getBridgeName());
    }
}
