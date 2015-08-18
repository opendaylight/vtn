/*
 * Copyright (c) 2013, 2015 NEC Corporation. All rights reserved.
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
 * JUnit test for {@link VBridgePath}.
 */
public class VBridgePathTest extends TestBase {
    /**
     * Test case for getter methods.
     */
    @Test
    public void testGetter() {
        for (String tname: createStrings("tenant")) {
            VTenantPath tpath = new VTenantPath(tname);
            for (String bname: createStrings("bridge")) {
                VBridgePath path = new VBridgePath(tname, bname);
                assertEquals(tname, path.getTenantName());
                assertEquals(bname, path.getBridgeName());
                assertEquals("vBridge", path.getNodeType());
                checkVirtualNodePath(path);

                path = new VBridgePath(tpath, bname);
                assertEquals(tname, path.getTenantName());
                assertEquals(bname, path.getBridgeName());
                assertEquals("vBridge", path.getNodeType());
                checkVirtualNodePath(path);
            }
        }
    }

    /**
     * Test case for {@link VBridgePath#clone()}.
     */
    @Test
    public void testClone() {
        for (String tname: createStrings("tenant")) {
            VTenantPath tpath = new VTenantPath(tname);
            for (String bname: createStrings("bridge")) {
                // In case where the hash code is cached.
                VBridgePath path = new VBridgePath(tname, bname);
                int hash = path.hashCode();

                VTenantPath clone = path.clone();
                assertNotSame(path, clone);
                assertEquals(path, clone);
                assertEquals(hash, clone.hashCode());
                checkVirtualNodePath(clone);
                assertEquals(VBridgePath.class, clone.getClass());
                VBridgePath p = (VBridgePath)clone;
                assertEquals(tname, p.getTenantName());
                assertEquals(bname, p.getBridgeName());

                // In case where the hash code is not cached.
                path = new VBridgePath(tname, bname);
                clone = path.clone();
                assertNotSame(path, clone);
                assertEquals(path, clone);
                assertEquals(hash, clone.hashCode());
                checkVirtualNodePath(clone);
                assertEquals(VBridgePath.class, clone.getClass());
                p = (VBridgePath)clone;
                assertEquals(tname, p.getTenantName());
                assertEquals(bname, p.getBridgeName());
            }
        }
    }

    /**
     * Test case for {@link VBridgePath#replaceTenantName(String)}.
     */
    @Test
    public void testReplaceTenantName() {
        for (String tname: createStrings("tenant")) {
            VTenantPath tpath = new VTenantPath(tname);
            for (String bname: createStrings("bridge")) {
                // In case where the hash code is cached.
                VBridgePath path = new VBridgePath(tname, bname);
                int hash = path.hashCode();

                String name = tname + "_new";
                VBridgePath path1 = (VBridgePath)path.replaceTenantName(name);
                assertEquals(name, path1.getTenantName());
                assertEquals(bname, path1.getBridgeName());
                assertEquals("vBridge", path1.getNodeType());
                assertEquals(VBridgePath.class, path1.getClass());
                checkVirtualNodePath(path1);
                int hash1 = path1.hashCode();
                assertNotEquals(hash, hash1);

                // In case where the hash code is not cached.
                path = new VBridgePath(tname, bname);
                path1 = (VBridgePath)path.replaceTenantName(name);
                assertEquals(name, path1.getTenantName());
                assertEquals(bname, path1.getBridgeName());
                assertEquals("vBridge", path1.getNodeType());
                assertEquals(VBridgePath.class, path1.getClass());
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
            for (String bname: createStrings("bridge")) {
                VBridgePath path = new VBridgePath(tname, bname);
                assertFalse(path.contains(null));
                assertTrue(path.contains(path));

                String notMatched = "not_matched";
                containsTest(path, tname, bname, true);
                containsTest(path, notMatched, bname, false);
                containsTest(path, tname, notMatched, false);
            }
        }
    }

    /**
     * Test case for {@link VBridgePath#equals(Object)} and
     * {@link VBridgePath#hashCode()}.
     */
    @Test
    public void testEquals() {
        HashSet<Object> set = new HashSet<Object>();
        List<String> tnames = createStrings("tenant");
        List<String> bnames = createStrings("bridge");
        for (String tname: tnames) {
            VTenantPath tpath = new VTenantPath(tname);
            for (String bname: bnames) {
                VBridgePath p1 = new VBridgePath(tname, bname);
                VBridgePath p2 = new VBridgePath(copy(tname), copy(bname));
                testEquals(set, p1, p2);

                // VTenantPath object that has the same tenant name must be
                // treated as different object.
                assertFalse(p1.equals(tpath));
                assertFalse(tpath.equals(p1));
            }
        }

        int required = tnames.size() * bnames.size();
        assertEquals(required, set.size());
    }

    /**
     * Test case for {@link VBridgePath#toString()}.
     */
    @Test
    public void testToString() {
        String prefix = "vBridge:";
        for (String tname: createStrings("tenant")) {
            for (String bname: createStrings("bridge")) {
                VBridgePath path = new VBridgePath(tname, bname);
                String tn = (tname == null) ? "<null>" : tname;
                String bn = (bname == null) ? "<null>" : bname;
                String required = joinStrings(prefix, null, ".", tn, bn);
                assertEquals(required, path.toString());
            }
        }
    }

    /**
     * Ensure that {@link VBridgePath} is serializable.
     */
    @Test
    public void testSerialize() {
        for (String tname: createStrings("tenant")) {
            for (String bname: createStrings("bridge")) {
                VBridgePath path = new VBridgePath(tname, bname);
                serializeTest(path);
            }
        }
    }

    /**
     * Internal method for {@link #testContains()}.
     *
     * @param path      A {@link VBridgePath} instance to be tested.
     * @param tname     The name of the tenant for test.
     * @param bname     The name of the vBridge for test.
     * @param expected  Expected result.
     */
    private void containsTest(VBridgePath path, String tname, String bname,
                              boolean expected) {
        VTenantPath tpath = new VTenantPath(tname);
        assertEquals(false, path.contains(tpath));

        VBridgePath bpath = new VBridgePath(tpath, bname);
        assertEquals(expected, path.contains(bpath));

        VTerminalPath tmpath = new VTerminalPath(tpath, bname);
        assertEquals(false, path.contains(tmpath));

        VTerminalIfPath tipath = new VTerminalIfPath(tmpath, "interface");
        assertEquals(false, path.contains(tipath));

        VBridgeIfPath ipath = new VBridgeIfPath(bpath, "interface");
        assertEquals(expected, path.contains(ipath));
    }

    /**
     * Test case for {@link VBridgePath#toVirtualNodePath()}.
     *
     * @param path  A {@link VTenantPath} to be tested.
     */
    private void checkVirtualNodePath(VTenantPath path) {
        assertTrue(path instanceof VBridgePath);
        VBridgePath bpath = (VBridgePath)path;

        VirtualNodePath vpath = path.toVirtualNodePath();
        assertEquals(bpath.getTenantName(), vpath.getTenantName());
        assertEquals(bpath.getBridgeName(), vpath.getBridgeName());
        assertEquals(null, vpath.getInterfaceName());
        assertEquals(null, vpath.getRouterName());
        assertEquals(null, vpath.getTerminalName());
    }
}
