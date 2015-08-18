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
 * JUnit test for {@link VInterfacePathAdapter}.
 */
public class VInterfacePathAdapterTest extends TestBase {
    /**
     * Test case for {@link VInterfacePathAdapter#marshal(VInterfacePath)} and
     * {@link VInterfacePathAdapter#unmarshal(VNodeLocation)}.
     */
    @Test
    public void testMarshal() {
        VInterfacePathAdapter adapter = new VInterfacePathAdapter();
        assertEquals(null, adapter.marshal(null));
        assertEquals(null, adapter.unmarshal(null));

        for (String tname: createStrings("tenant")) {
            for (String bname: createStrings("bridge", false)) {
                VBridgePath bpath = new VBridgePath(tname, bname);
                VTerminalPath vtpath = new VTerminalPath(tname, bname);
                for (String iname: createStrings("ifname", false)) {
                    VBridgeIfPath bipath = new VBridgeIfPath(bpath, iname);
                    VNodeLocation loc = new VNodeLocation(bipath);
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
        VInterfacePath path = new VBridgeIfPath("vtn", null, "if_1");
        VNodeLocation loc = path.toVNodeLocation();
        VInterfacePath invalid = adapter.unmarshal(loc);
        assertEquals(ErrorVNodePath.class, invalid.getClass());
        assertEquals("Unexpected interface location: " + loc,
                     ((ErrorVNodePath)invalid).getError());

        path = new VBridgeIfPath("vtn", "vbridge", null);
        loc = path.toVNodeLocation();
        invalid = adapter.unmarshal(loc);
        assertEquals(ErrorVNodePath.class, invalid.getClass());
        assertEquals("Interface name is not specified: " + loc,
                     ((ErrorVNodePath)invalid).getError());
    }
}
