/*
 * Copyright (c) 2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.cluster;

import java.util.HashSet;
import java.util.List;

import org.junit.Test;

import org.opendaylight.vtn.manager.VBridgePath;
import org.opendaylight.vtn.manager.VBridgeIfPath;
import org.opendaylight.vtn.manager.internal.TestBase;

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
                }
            }
        }

        int required = tnames.size() * bnames.size() * mapIds.size();
        assertEquals(required, set.size());
    }

    /**
     * Test case for {@link VlanMapPath#toString()}.
     */
    @Test
    public void testToString() {
        for (String tname: createStrings("tenant")) {
            for (String bname: createStrings("bridge")) {
                VBridgePath bpath = new VBridgePath(tname, bname);
                for (String mapId: createStrings("mapId", false)) {
                    VlanMapPath path = new VlanMapPath(bpath, mapId);
                    String tn = (tname == null) ? "<null>" : tname;
                    String bn = (bname == null) ? "<null>" : bname;
                    String required =
                        joinStrings(null, null, ".", tn, bn, mapId);
                    assertEquals(required, path.toString());
                }
            }
        }
    }

    /**
     * Ensure that {@link VlanMapPath} is serializable.
     */
    @Test
    public void testSerialize() {
        for (String tname: createStrings("tenant")) {
            for (String bname: createStrings("bridge")) {
                VBridgePath bpath = new VBridgePath(tname, bname);
                for (String mapId: createStrings("mapId", false)) {
                    VlanMapPath path = new VlanMapPath(bpath, mapId);
                    serializeTest(path);
                }
            }
        }
    }
}
