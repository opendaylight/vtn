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

                path = new VBridgePath(tpath, bname);
                assertEquals(tname, path.getTenantName());
                assertEquals(bname, path.getBridgeName());
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
        for (String tname: createStrings("tenant")) {
            for (String bname: createStrings("bridge")) {
                VBridgePath path = new VBridgePath(tname, bname);
                String tn = (tname == null) ? "<null>" : tname;
                String bn = (bname == null) ? "<null>" : bname;
                String required = joinStrings(null, null, ".", tn, bn);
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

        VBridgeIfPath ipath = new VBridgeIfPath(bpath, "interface");
        assertEquals(expected, path.contains(ipath));
    }
}
