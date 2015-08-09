/*
 * Copyright (c) 2014, 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.flow.filter;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;

import org.junit.Test;

import org.opendaylight.vtn.manager.VBridgeIfPath;
import org.opendaylight.vtn.manager.VBridgePath;
import org.opendaylight.vtn.manager.VTenantPath;
import org.opendaylight.vtn.manager.VTerminalIfPath;

import org.opendaylight.vtn.manager.TestBase;

/**
 * JUnit test for {@link FlowFilterId}.
 */
public class FlowFilterIdTest extends TestBase {
    /**
     * Test case for getter methods.
     */
    @Test
    public void testGetter() {
        boolean[] bools = {true, false};
        for (String tname: createStrings("tenant")) {
            VTenantPath tpath = new VTenantPath(tname);
            FlowFilterId fid = new FlowFilterId(tpath);
            assertEquals(tpath, fid.getPath());
            assertEquals(false, fid.isOutput());

            for (String bname: createStrings("vbr")) {
                VBridgePath bpath = new VBridgePath(tpath, bname);
                for (boolean out: bools) {
                    fid = new FlowFilterId(bpath, out);
                    assertEquals(bpath, fid.getPath());
                    assertEquals(out, fid.isOutput());
                }

                for (String iname: createStrings("ifname")) {
                    VBridgeIfPath bipath = new VBridgeIfPath(bpath, iname);
                    for (boolean out: bools) {
                        fid = new FlowFilterId(bipath, out);
                        assertEquals(bipath, fid.getPath());
                        assertEquals(out, fid.isOutput());
                    }

                    VTerminalIfPath vipath =
                        new VTerminalIfPath(tname, bname, iname);
                    for (boolean out: bools) {
                        fid = new FlowFilterId(vipath, out);
                        assertEquals(vipath, fid.getPath());
                        assertEquals(out, fid.isOutput());
                    }
                }
            }
        }

        // Specify null path.
        FlowFilterId fid = new FlowFilterId((VTenantPath)null);
        assertEquals(null, fid.getPath());
        assertEquals(false, fid.isOutput());

        fid = new FlowFilterId((VBridgePath)null, true);
        assertEquals(null, fid.getPath());
        assertEquals(true, fid.isOutput());

        fid = new FlowFilterId((VBridgeIfPath)null, true);
        assertEquals(null, fid.getPath());
        assertEquals(true, fid.isOutput());

        fid = new FlowFilterId((VTerminalIfPath)null, true);
        assertEquals(null, fid.getPath());
        assertEquals(true, fid.isOutput());
    }

    /**
     * Test case for {@link FlowFilterId#equals(Object)} and
     * {@link FlowFilterId#hashCode()}.
     */
    @Test
    public void testEquals() {
        HashSet<Object> set = new HashSet<Object>();
        List<String> tnames = createStrings("tenant");
        List<String> bnames = createStrings("vbr");
        List<String> inames = createStrings("ifname");
        boolean[] bools = {true, false};
        for (String tname: tnames) {
            VTenantPath tpath1 = new VTenantPath(tname);
            VTenantPath tpath2 = new VTenantPath(copy(tname));
            FlowFilterId fid1 = new FlowFilterId(tpath1);
            FlowFilterId fid2 = new FlowFilterId(tpath2);
            testEquals(set, fid1, fid2);

            for (String bname: bnames) {
                VBridgePath bpath1 = new VBridgePath(tpath1, bname);
                VBridgePath bpath2 = new VBridgePath(tpath2, copy(bname));
                for (boolean out: bools) {
                    fid1 = new FlowFilterId(bpath1, out);
                    fid2 = new FlowFilterId(bpath2, out);
                    testEquals(set, fid1, fid2);
                }

                for (String iname: inames) {
                    VBridgeIfPath bipath1 = new VBridgeIfPath(bpath1, iname);
                    VBridgeIfPath bipath2 =
                        new VBridgeIfPath(bpath2, copy(iname));
                    for (boolean out: bools) {
                        fid1 = new FlowFilterId(bipath1, out);
                        fid2 = new FlowFilterId(bipath2, out);
                        testEquals(set, fid1, fid2);
                    }

                    VTerminalIfPath vipath1 =
                        new VTerminalIfPath(tname, bname, iname);
                    VTerminalIfPath vipath2 =
                        new VTerminalIfPath(copy(tname), copy(bname),
                                            copy(iname));
                    for (boolean out: bools) {
                        fid1 = new FlowFilterId(vipath1, out);
                        fid2 = new FlowFilterId(vipath2, out);
                        testEquals(set, fid1, fid2);
                    }
                }
            }
        }

        int required = tnames.size() +
            (tnames.size() * bnames.size() * 2) +
            (tnames.size() * bnames.size() * (inames.size() * 2) * 2);
        assertEquals(required, set.size());
    }

    /**
     * Test case for {@link FlowFilterId#toString()}.
     */
    @Test
    public void testToString() {
        String prefix = "FlowFilterId[";
        String suffix = "]";
        for (FlowFilterId fid: createFlowFilterIds()) {
            VTenantPath path = fid.getPath();
            boolean out = fid.isOutput();
            String p = "path=" + path;
            String o = "output=" + out;
            String required = joinStrings(prefix, suffix, ",", p, o);
            assertEquals(required, fid.toString());
        }
    }

    /**
     * Ensure that {@link VTerminalIfPath} is serializable.
     */
    @Test
    public void testSerialize() {
        for (FlowFilterId fid: createFlowFilterIds()) {
            serializeTest(fid);
        }
    }

    /**
     * Create unique {@link FlowFilterId} instances.
     *
     * @return  A list of {@link FlowFilterId} instances.
     */
    private List<FlowFilterId> createFlowFilterIds() {
        List<FlowFilterId> list = new ArrayList<FlowFilterId>();
        boolean[] bools = {true, false};
        for (String tname: createStrings("tenant")) {
            VTenantPath tpath = new VTenantPath(tname);
            list.add(new FlowFilterId(tpath));

            for (String bname: createStrings("vbr")) {
                VBridgePath bpath = new VBridgePath(tpath, bname);
                for (boolean out: bools) {
                    list.add(new FlowFilterId(bpath, out));
                }

                for (String iname: createStrings("ifname")) {
                    VBridgeIfPath bipath = new VBridgeIfPath(bpath, iname);
                    for (boolean out: bools) {
                        list.add(new FlowFilterId(bipath, out));
                    }

                    VTerminalIfPath vipath =
                        new VTerminalIfPath(tname, bname, iname);
                    for (boolean out: bools) {
                        list.add(new FlowFilterId(vipath, out));
                    }
                }
            }
        }

        return list;
    }
}
