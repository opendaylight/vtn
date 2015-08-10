/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.cluster;

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
 * JUnit test for {@link MacMappedHostPath}.
 */
public class MacMappedHostPathTest extends TestBase {
    /**
     * Test case for {@link MacMappedHostPath#getTenantName()}.
     */
    @Test
    public void testGetTenantName() {
        String bname = "vbr";
        MacVlan mv = new MacVlan(0x123456789abcL, (short)123);
        for (String tname: createStrings("tenant")) {
            VBridgePath bpath = new VBridgePath(tname, bname);
            MacMappedHostPath path = new MacMappedHostPath(bpath, mv);
            assertEquals(tname, path.getTenantName());
        }
    }

    /**
     * Test case for {@link MacMappedHostPath#getBridgeName()}.
     */
    @Test
    public void testGetBridgeName() {
        String tname = "vtn";
        MacVlan mv = new MacVlan(0x123456789abcL, (short)123);
        for (String bname: createStrings("bridge")) {
            VBridgePath bpath = new VBridgePath(tname, bname);
            MacMappedHostPath path = new MacMappedHostPath(bpath, mv);
            assertEquals(bname, path.getBridgeName());
        }
    }

    /**
     * Test case for {@link MacMappedHostPath#getMappedHost()}.
     */
    @Test
    public void testGetMappedHost() {
        String tname = "vtn";
        String bname = "vbr";
        VBridgePath bpath = new VBridgePath(tname, bname);
        MacVlan[] hosts = {
            null,
            new MacVlan(0L, (short)0),
            new MacVlan(0xffffffffffffL, (short)4095),
            new MacVlan(0x123456789abcL, (short)123),
        };
        for (MacVlan mv: hosts) {
            MacMappedHostPath path = new MacMappedHostPath(bpath, mv);
            assertEquals(mv, path.getMappedHost());
        }
    }

    /**
     * Test case for {@link MacMappedHostPath#getBridgeMapInfo()}.
     */
    @Test
    public void testGetBridgeMapInfo() {
        String tname = "vtn";
        String bname = "vbr";
        VBridgePath bpath = new VBridgePath(tname, bname);
        MacVlan[] hosts = {
            null,
            new MacVlan(0L, (short)0),
            new MacVlan(0xffffffffffffL, (short)4095),
            new MacVlan(0x123456789abcL, (short)123),
        };
        for (MacVlan mv: hosts) {
            MacMappedHostPath path = new MacMappedHostPath(bpath, mv);
            BridgeMapInfo minfo = path.getBridgeMapInfo();
            assertEquals(null, minfo.getVlanMapId());
            Long host = (mv == null)
                ? Long.valueOf(-1L)
                : Long.valueOf(mv.getEncodedValue());
            assertEquals(host, minfo.getMacMappedHost());
        }
    }

    /**
     * Ensure that {@link VTenantPath#contains(VTenantPath)} works.
     */
    @Test
    public void testContains() {
        String notMatched = "not_matched";
        MacVlan[] hosts = {
            null,
            new MacVlan(0L, (short)0),
            new MacVlan(0xffffffffffffL, (short)4095),
            new MacVlan(0x123456789abcL, (short)123),
        };
        MacVlan unknown = new MacVlan(0x777777L, (short)1);
        for (String tname: createStrings("tenant")) {
            for (String bname: createStrings("bridge")) {
                VBridgePath bpath = new VBridgePath(tname, bname);
                for (MacVlan mv: hosts) {
                    MacMappedHostPath path = new MacMappedHostPath(bpath, mv);
                    assertFalse(path.contains(null));
                    assertTrue(path.contains(path));

                    containsTest(path, tname, bname, mv, true);
                    containsTest(path, notMatched, bname, mv, false);
                    containsTest(path, tname, notMatched, mv, false);
                    containsTest(path, tname, bname, unknown, false);
                }
            }
        }
    }

    /**
     * Test case for {@link MacMappedHostPath#equals(Object)} and
     * {@link MacMappedHostPath#hashCode()}.
     */
    @Test
    public void testEquals() {
        HashSet<Object> set = new HashSet<Object>();
        List<String> tnames = createStrings("tenant");
        List<String> bnames = createStrings("bridge");
        MacVlan[] hosts = {
            null,
            new MacVlan(0L, (short)0),
            new MacVlan(0xffffffffffffL, (short)4095),
            new MacVlan(0x123456789abcL, (short)123),
        };
        for (String tname: tnames) {
            VTenantPath tpath = new VTenantPath(tname);
            for (String bname: bnames) {
                VBridgePath b1 = new VBridgePath(tname, bname);
                VBridgePath b2 = new VBridgePath(copy(tname), copy(bname));
                MacMapPath m1  = new MacMapPath(b1);
                MacMapPath m2  = new MacMapPath(b2);
                for (MacVlan mv1: hosts) {
                    MacVlan mv2 = (mv1 == null)
                        ? null : new MacVlan(mv1.getEncodedValue());
                    MacMappedHostPath p1 = new MacMappedHostPath(b1, mv1);
                    MacMappedHostPath p2 = new MacMappedHostPath(b2, mv2);
                    testEquals(set, p1, p2);

                    // VTenantPath object that has the same tenant name must
                    // be treated as different object.
                    assertFalse(p1.equals(tpath));
                    assertFalse(tpath.equals(p1));

                    // VBridgePath object that has the same tenant and bridge
                    // name must be treated as different object.
                    assertFalse(p1.equals(b1));
                    assertFalse(b1.equals(p1));
                    assertFalse(p2.equals(b2));
                    assertFalse(b2.equals(p2));

                    // MacMapPath object that has the same tenant and bridge
                    // name must be treated as different object.
                    assertFalse(p1.equals(m1));
                    assertFalse(m1.equals(p1));
                    assertFalse(p2.equals(m2));
                    assertFalse(m2.equals(p2));
                }
            }
        }

        int required = tnames.size() * bnames.size() * hosts.length;
        assertEquals(required, set.size());
    }

    /**
     * Test case for {@link MacMappedHostPath#toString()}.
     */
    @Test
    public void testToString() {
        String prefix = "MacMappedHost:";
        MacVlan[] hosts = {
            null,
            new MacVlan(0L, (short)0),
            new MacVlan(0xffffffffffffL, (short)4095),
            new MacVlan(0x123456789abcL, (short)123),
        };
        for (String tname: createStrings("tenant")) {
            for (String bname: createStrings("bridge")) {
                VBridgePath bpath = new VBridgePath(tname, bname);
                for (MacVlan mv: hosts) {
                    MacMappedHostPath path = new MacMappedHostPath(bpath, mv);
                    String tn = (tname == null) ? "<null>" : tname;
                    String bn = (bname == null) ? "<null>" : bname;
                    String hn = (mv == null)
                        ? "<null>" : Long.toString(mv.getEncodedValue());
                    String required = joinStrings(prefix, null, ".",
                                                  tn, bn, hn);
                    assertEquals(required, path.toString());
                }
            }
        }
    }

    /**
     * Ensure that {@link MacMappedHostPath} is serializable.
     */
    @Test
    public void testSerialize() {
        MacVlan[] hosts = {
            null,
            new MacVlan(0L, (short)0),
            new MacVlan(0xffffffffffffL, (short)4095),
            new MacVlan(0x123456789abcL, (short)123),
        };
        for (String tname: createStrings("tenant")) {
            for (String bname: createStrings("bridge")) {
                VBridgePath bpath = new VBridgePath(tname, bname);
                for (MacVlan mv: hosts) {
                    MacMappedHostPath path = new MacMappedHostPath(bpath, mv);
                    serializeTest(path);
                }
            }
        }
    }

    /**
     * Internal method for {@link #testContains()}.
     *
     * @param path      A {@link MacMappedHostPath} instance to be tested.
     * @param tname     The name of the tenant for test.
     * @param bname     The name of the vBridge for test.
     * @param host      A {@link MacVlan} instance which indicates the host
     *                  mapped by the MAC mapping.
     * @param expected  Expected result.
     */
    private void containsTest(MacMappedHostPath path, String tname,
                              String bname, MacVlan host, boolean expected) {
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
        assertEquals(false, path.contains(mpath));
        assertEquals(sameBridge, mpath.contains(path));

        MacMappedHostPath mhpath = new MacMappedHostPath(bpath, host);
        assertEquals(expected, path.contains(mhpath));
        assertEquals(expected, mhpath.contains(path));
    }
}
