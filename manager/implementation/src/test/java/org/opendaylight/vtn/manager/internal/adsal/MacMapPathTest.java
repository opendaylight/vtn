/*
 * Copyright (c) 2014, 2015 NEC Corporation. All rights reserved.
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
 * JUnit test for {@link MacMapPath}.
 */
public class MacMapPathTest extends TestBase {
    /**
     * Test case for getter methods.
     */
    @Test
    public void testGetter() {
        Long host = Long.valueOf(-1L);
        for (String tname: createStrings("tenant")) {
            VTenantPath tpath = new VTenantPath(tname);
            for (String bname: createStrings("bridge")) {
                VBridgePath bpath = new VBridgePath(tname, bname);
                MacMapPath path = new MacMapPath(bpath);
                assertEquals(tname, path.getTenantName());
                assertEquals(bname, path.getBridgeName());
                assertEquals("MacMap", path.getNodeType());

                BridgeMapInfo minfo = path.getBridgeMapInfo();
                assertEquals(null, minfo.getVlanMapId());
                assertEquals(host, minfo.getMacMappedHost());
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
                VBridgePath bpath = new VBridgePath(tname, bname);
                MacMapPath path = new MacMapPath(bpath);
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
     * Test case for {@link MacMapPath#equals(Object)} and
     * {@link MacMapPath#hashCode()}.
     */
    @Test
    public void testEquals() {
        HashSet<Object> set = new HashSet<Object>();
        List<String> tnames = createStrings("tenant");
        List<String> bnames = createStrings("bridge");
        for (String tname: tnames) {
            VTenantPath tpath = new VTenantPath(tname);
            for (String bname: bnames) {
                VBridgePath b1 = new VBridgePath(tname, bname);
                VBridgePath b2 = new VBridgePath(copy(tname), copy(bname));
                MacMapPath p1 = new MacMapPath(b1);
                MacMapPath p2 = new MacMapPath(b2);
                testEquals(set, p1, p2);

                // VTenantPath object that has the same tenant name and
                // VBridgePath object that has the same tenant and bridge name
                // must be treated as different object.
                assertFalse(p1.equals(tpath));
                assertFalse(tpath.equals(p1));

                assertFalse(p1.equals(b1));
                assertFalse(b1.equals(p1));
                assertFalse(p2.equals(b2));
                assertFalse(b2.equals(p2));
            }
        }

        int required = tnames.size() * bnames.size();
        assertEquals(required, set.size());
    }

    /**
     * Test case for {@link MacMapPath#toString()}.
     */
    @Test
    public void testToString() {
        String prefix = "MacMap:";
        for (String tname: createStrings("tenant")) {
            for (String bname: createStrings("bridge")) {
                VBridgePath bpath = new VBridgePath(tname, bname);
                MacMapPath path = new MacMapPath(bpath);
                String tn = (tname == null) ? "<null>" : tname;
                String bn = (bname == null) ? "<null>" : bname;
                String required = joinStrings(prefix, null, ".", tn, bn);
                assertEquals(required, path.toString());
            }
        }
    }

    /**
     * Internal method for {@link #testContains()}.
     *
     * @param path      A {@link VBridgePath} instance to be tested.
     * @param tname     The name of the tenant for test.
     * @param bname     The name of the vBridge for tnest.
     * @param expected  Expected result.
     */
    private void containsTest(VBridgePath path, String tname, String bname,
                              boolean expected) {
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

        VBridgeIfPath ipath = new VBridgeIfPath(bpath, "interface");
        assertEquals(false, path.contains(ipath));
        assertEquals(false, ipath.contains(path));

        MacMapPath mpath = new MacMapPath(bpath);
        assertEquals(expected, path.contains(mpath));
        assertEquals(expected, mpath.contains(path));
    }
}
