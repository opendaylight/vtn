/*
 * Copyright (c) 2013, 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager;

import java.util.HashSet;
import java.util.TreeSet;
import java.util.Iterator;
import java.util.List;
import java.util.ArrayList;

import org.junit.Test;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.virtual.route.info.VirtualNodePath;

/**
 * JUnit test for {@link VTenantPath}.
 */
public class VTenantPathTest extends TestBase {
    /**
     * Test class which keeps the same path components as
     * {@link VBridgeIfPath}.
     */
    private final class TestPath extends VBridgeIfPath {
        /**
         * Construct a new instance.
         *
         * @param path   A {@link VBridgePath} instance.
         * @param name   The name of this instance.
         */
        private TestPath(VBridgePath path, String name) {
            super(path, name);
        }
    }

    /**
     * Test case for getter methods.
     */
    @Test
    public void testGetter() {
        for (String tname: createStrings("tenant_name")) {
            VTenantPath path = new VTenantPath(tname);
            assertEquals(tname, path.getTenantName());
            assertEquals("VTN", path.getNodeType());
            checkVirtualNodePath(path);
        }
    }

    /**
     * Test case for {@link VTenantPath#clone()}.
     */
    @Test
    public void testClone() {
        for (String tname: createStrings("tenant_name")) {
            // In case where the hash code is cached.
            VTenantPath path = new VTenantPath(tname);
            int hash = path.hashCode();
            VTenantPath clone = path.clone();
            assertNotSame(path, clone);
            assertEquals(tname, clone.getTenantName());
            assertEquals(path, clone);
            assertEquals(hash, clone.hashCode());
            checkVirtualNodePath(clone);

            // In case where the hash code is not cached.
            path = new VTenantPath(tname);
            clone = path.clone();
            assertNotSame(path, clone);
            assertEquals(tname, clone.getTenantName());
            assertEquals(path, clone);
            assertEquals(hash, clone.hashCode());
            checkVirtualNodePath(clone);
        }
    }

    /**
     * Test case for {@link VTenantPath#replaceTenantName(String)}.
     */
    @Test
    public void testReplaceTenantName() {
        for (String tname: createStrings("tenant_name")) {
            // In case where the hash code is cached.
            VTenantPath path = new VTenantPath(tname);
            int hash = path.hashCode();

            String name = tname + "_new";
            VTenantPath path1 = path.replaceTenantName(name);
            assertEquals(name, path1.getTenantName());
            assertEquals("VTN", path1.getNodeType());
            assertEquals(VTenantPath.class, path1.getClass());
            int hash1 = path1.hashCode();
            assertNotEquals(hash, hash1);
            checkVirtualNodePath(path1);

            // In case where the hash code is not cached.
            path = new VTenantPath(tname);
            path1 = path.replaceTenantName(name);
            assertEquals(name, path1.getTenantName());
            assertEquals("VTN", path1.getNodeType());
            assertEquals(VTenantPath.class, path1.getClass());
            assertEquals(hash1, path1.hashCode());
            checkVirtualNodePath(path1);
        }
    }

    /**
     * Test case for {@link VTenantPath#contains(VTenantPath)}.
     */
    @Test
    public void testContains() {
        for (String tname: createStrings("tenant_name")) {
            VTenantPath path = new VTenantPath(tname);
            assertFalse(path.contains(null));
            assertTrue(path.contains(path));
            containsTest(path, tname, true);
            containsTest(path, "not_matched", false);
        }
    }

    /**
     * Test case for {@link VTenantPath#equals(Object)} and
     * {@link VTenantPath#hashCode()}.
     */
    @Test
    public void testEquals() {
        HashSet<Object> set = new HashSet<Object>();
        List<String> tnames = createStrings("tenant_name");
        for (String tname: tnames) {
            VTenantPath p1 = new VTenantPath(tname);
            VTenantPath p2 = new VTenantPath(copy(tname));
            assertEquals(0, p1.compareTo(p2));
            testEquals(set, p1, p2);
        }

        assertEquals(tnames.size(), set.size());
    }

    /**
     * Test case for {@link VTenantPath#toString()}.
     */
    @Test
    public void testToString() {
        String prefix = "VTN:";
        for (String tname: createStrings("tenant_name")) {
            VTenantPath path = new VTenantPath(tname);
            String required = (tname == null) ? "<null>" : tname;
            assertEquals(prefix + required, path.toString());
        }
    }

    /**
     * Test case for {@link VTenantPath#compareTo(VTenantPath)}.
     */
    @Test
    public void testCompareTo() {
        int size = 0;
        HashSet<VTenantPath> hset = new HashSet<VTenantPath>();
        TreeSet<VTenantPath> set = new TreeSet<VTenantPath>();
        for (String tname: createStrings("tenant_name")) {
            VTenantPath path = new VTenantPath(tname);
            assertTrue(set.add(path));
            assertFalse(set.add(path));
            assertTrue(hset.add(path));
            assertFalse(hset.add(path));
            size++;

            // VTenantPath.compareTo() can accept VTenantPath variants.
            for (String bname: createStrings("bridge_name")) {
                VBridgePath bpath = new VBridgePath(path, bname);
                assertTrue(set.add(bpath));
                assertFalse(set.add(bpath));
                assertTrue(hset.add(bpath));
                assertFalse(hset.add(bpath));
                size++;

                VTerminalPath tmpath = new VTerminalPath(path, bname);
                assertTrue(set.add(tmpath));
                assertFalse(set.add(tmpath));
                assertTrue(hset.add(tmpath));
                assertFalse(hset.add(tmpath));
                size++;

                for (String iname: createStrings("interface_name")) {
                    VBridgeIfPath ipath = new VBridgeIfPath(bpath, iname);
                    assertTrue(set.add(ipath));
                    assertFalse(set.add(ipath));
                    assertTrue(hset.add(ipath));
                    assertFalse(hset.add(ipath));
                    size++;

                    VTerminalIfPath tipath =
                        new VTerminalIfPath(tmpath, iname);
                    assertTrue(set.add(tipath));
                    assertFalse(set.add(tipath));
                    assertTrue(hset.add(tipath));
                    assertFalse(hset.add(tipath));
                    size++;

                    TestPath tpath = new TestPath(bpath, iname);
                    assertTrue(set.add(tpath));
                    assertFalse(set.add(tpath));
                    assertTrue(hset.add(tpath));
                    assertFalse(hset.add(tpath));
                    size++;
                }
            }
        }

        assertEquals(size, set.size());
        assertEquals(hset, set);

        // The first element in the set must be a VTenantPath instance with
        // null name.
        Iterator<VTenantPath> it = set.iterator();
        assertTrue(it.hasNext());
        VTenantPath prev = it.next();
        assertEquals(VTenantPath.class, prev.getClass());
        assertNull(prev.getTenantName());

        Class<?> prevClass = VTenantPath.class;
        HashSet<Class<?>> classSet = new HashSet<Class<?>>();
        ArrayList<String> prevComponens = new ArrayList<String>();
        prevComponens.add(null);

        while (it.hasNext()) {
            VTenantPath path = it.next();
            assertTrue(prev.compareTo(path) < 0);
            assertFalse(prev.equals(path));

            ArrayList<String> components = new ArrayList<String>();
            components.add(path.getTenantName());
            if (path instanceof VNodePath) {
                components.add(((VNodePath)path).getTenantNodeName());
            }
            if (path instanceof VBridgeIfPath) {
                components.add(((VBridgeIfPath)path).getInterfaceName());
            } else if (path instanceof VTerminalIfPath) {
                components.add(((VTerminalIfPath)path).getInterfaceName());
            }

            int prevSize = prevComponens.size();
            int compSize = components.size();
            Class<?> cls = path.getClass();
            boolean classChanged = false;
            if (prevSize == compSize) {
                if (cls.equals(prevClass)) {
                    checkPathOrder(prevComponens, components);
                } else {
                    String name = cls.getName();
                    String prevName = prevClass.getName();
                    assertTrue("name=" + name + ", prevName=" + prevName,
                               prevName.compareTo(name) < 0);
                    classChanged = true;
                }
            } else {
                assertTrue(prevSize < compSize);
                classChanged = true;
            }

            if (classChanged) {
                assertTrue(classSet.add(cls));
                prevClass = cls;
            }

            prevComponens = components;
            prev = path;
        }
    }

    /**
     * Ensure that {@link VTenantPath} is serializable.
     */
    @Test
    public void testSerialize() {
        for (String tname: createStrings("tenant_name")) {
            VTenantPath path = new VTenantPath(tname);
            serializeTest(path);
        }
    }

    /**
     * Verify the order of the path components.
     *
     * @param lesser    A path components that should be less than
     *                  {@code greater}.
     * @param greater   A path components to be compared.
     */
    private void checkPathOrder(List<String> lesser, List<String> greater) {
        for (int i = 0; i < lesser.size(); i++) {
            String l = lesser.get(i);
            String g = greater.get(i);
            if (l == null) {
                return;
            }
            assertNotNull(g);
            int ret = l.compareTo(g);
            if (ret != 0) {
                assertTrue(ret < 0);
                return;
            }
        }

        fail("Identical: lesser=" + lesser + ", greater=" + greater);
    }

    /**
     * Internal method for {@link #testContains()}.
     *
     * @param path      A {@link VTenantPath} instance to be tested.
     * @param tname     The name of the tenant for test.
     * @param expected  Expected result.
     */
    private void containsTest(VTenantPath path, String tname,
                              boolean expected) {
        VTenantPath tpath = new VTenantPath(tname);
        assertEquals(expected, path.contains(tpath));

        VBridgePath bpath = new VBridgePath(tpath, "bridge");
        assertEquals(expected, path.contains(bpath));

        VBridgeIfPath ipath = new VBridgeIfPath(bpath, "interface");
        assertEquals(expected, path.contains(ipath));

        VTerminalPath vtpath = new VTerminalPath(tpath, "terminal");
        assertEquals(expected, path.contains(vtpath));

        VTerminalIfPath vtipath = new VTerminalIfPath(vtpath, "interface");
        assertEquals(expected, path.contains(vtipath));
    }

    /**
     * Test case for {@link VTenantPath#toVirtualNodePath()}.
     *
     * @param path  A {@link VTenantPath} to be tested.
     */
    private void checkVirtualNodePath(VTenantPath path) {
        VirtualNodePath vpath = path.toVirtualNodePath();
        assertEquals(path.getTenantName(), vpath.getTenantName());
        assertEquals(null, vpath.getBridgeName());
        assertEquals(null, vpath.getInterfaceName());
        assertEquals(null, vpath.getRouterName());
        assertEquals(null, vpath.getTerminalName());
    }
}
