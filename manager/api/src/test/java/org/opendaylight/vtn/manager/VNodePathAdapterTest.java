/*
 * Copyright (c) 2014, 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager;

import org.junit.Test;

/**
 * JUnit test for {@link VNodePathAdapter}.
 */
public class VNodePathAdapterTest extends TestBase {
    /**
     * Test case for {@link VNodePathAdapter#marshal(VNodePath)} and
     * {@link VNodePathAdapter#unmarshal(VNodeLocation)}.
     */
    @Test
    public void testMarshal() {
        VNodePathAdapter adapter = new VNodePathAdapter();
        assertEquals(null, adapter.marshal(null));
        assertEquals(null, adapter.unmarshal(null));

        for (String tname: createStrings("tenant")) {
            for (String bname: createStrings("bridge", false)) {
                VBridgePath bpath = new VBridgePath(tname, bname);
                VNodeLocation loc = new VNodeLocation(bpath);
                assertEquals(bpath, adapter.unmarshal(loc));
                assertEquals(loc, adapter.marshal(bpath));

                VTerminalPath vtpath = new VTerminalPath(tname, bname);
                loc = new VNodeLocation(vtpath);
                assertEquals(vtpath, adapter.unmarshal(loc));
                assertEquals(loc, adapter.marshal(vtpath));

                for (String iname: createStrings("ifname", false)) {
                    VBridgeIfPath bipath = new VBridgeIfPath(bpath, iname);
                    loc = new VNodeLocation(bipath);
                    assertEquals(bipath, adapter.unmarshal(loc));
                    assertEquals(loc, adapter.marshal(bipath));

                    VTerminalIfPath vipath =
                        new VTerminalIfPath(vtpath, iname);
                    loc = new VNodeLocation(vipath);
                    assertEquals(vipath, adapter.unmarshal(loc));
                    assertEquals(loc, adapter.marshal(vipath));
                }
            }
        }

        // Specifying invalid path.
        VNodePath path = new VBridgePath("vtn", null);
        VNodeLocation loc = path.toVNodeLocation();
        VNodePath invalid = adapter.unmarshal(loc);
        assertEquals(ErrorVNodePath.class, invalid.getClass());
        assertEquals("Unexpected node location: " + loc,
                     ((ErrorVNodePath)invalid).getError());

        path = new VBridgeIfPath("vtn", null, "if_1");
        loc = path.toVNodeLocation();
        invalid = adapter.unmarshal(loc);
        assertEquals(ErrorVNodePath.class, invalid.getClass());
        assertEquals("Unexpected node location: " + loc,
                     ((ErrorVNodePath)invalid).getError());
    }
}
