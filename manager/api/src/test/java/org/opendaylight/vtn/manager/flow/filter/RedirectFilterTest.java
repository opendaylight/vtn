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

import com.fasterxml.jackson.databind.ObjectMapper;

import org.codehaus.jettison.json.JSONObject;

import org.opendaylight.vtn.manager.ErrorVNodePath;
import org.opendaylight.vtn.manager.VBridgeIfPath;
import org.opendaylight.vtn.manager.VBridgePath;
import org.opendaylight.vtn.manager.VInterfacePath;
import org.opendaylight.vtn.manager.VNodeLocation;
import org.opendaylight.vtn.manager.VNodePath;
import org.opendaylight.vtn.manager.VTenantPath;
import org.opendaylight.vtn.manager.VTerminalIfPath;
import org.opendaylight.vtn.manager.VTerminalPath;

import org.opendaylight.vtn.manager.TestBase;

/**
 * JUnit test for {@link RedirectFilter}.
 */
public class RedirectFilterTest extends TestBase {
    /**
     * A list of {@link VInterfacePath} instances for test.
     */
    private List<VInterfacePath>  interfaces;

    /**
     * Test case for getter methods.
     */
    @Test
    public void testGetter() {
        boolean[] bools = {true, false};
        for (VInterfacePath path: createPaths()) {
            for (boolean output: bools) {
                RedirectFilter rf = (path instanceof VBridgeIfPath)
                    ? new RedirectFilter((VBridgeIfPath)path, output)
                    : new RedirectFilter((VTerminalIfPath)path, output);
                assertEquals(path, rf.getDestination());
                assertEquals(output, rf.isOutput());
            }
        }
    }

    /**
     * Test case for {@link RedirectFilter#equals(Object)} and
     * {@link RedirectFilter#hashCode()}.
     */
    @Test
    public void testEquals() {
        HashSet<Object> set = new HashSet<Object>();
        List<VInterfacePath> paths = createPaths();
        boolean[] bools = {true, false};
        for (VInterfacePath path: createPaths()) {
            for (boolean output: bools) {
                RedirectFilter rf1 = (path instanceof VBridgeIfPath)
                    ? new RedirectFilter((VBridgeIfPath)path, output)
                    : new RedirectFilter((VTerminalIfPath)path, output);
                VTenantPath dest = (path == null)
                    ? null
                    : ((VNodePath)path).clone();
                RedirectFilter rf2 = (dest instanceof VBridgeIfPath)
                    ? new RedirectFilter((VBridgeIfPath)dest, output)
                    : new RedirectFilter((VTerminalIfPath)dest, output);
                testEquals(set, rf1, rf2);
            }
        }

        assertEquals(paths.size() * bools.length, set.size());
    }

    /**
     * Test case for {@link RedirectFilter#toString()}.
     */
    @Test
    public void testToString() {
        String prefix = "RedirectFilter[";
        String suffix = "]";
        boolean[] bools = {true, false};
        for (VInterfacePath path: createPaths()) {
            for (boolean output: bools) {
                RedirectFilter rf = (path instanceof VBridgeIfPath)
                    ? new RedirectFilter((VBridgeIfPath)path, output)
                    : new RedirectFilter((VTerminalIfPath)path, output);
                String p = (path == null) ? null : "destination=" + path;
                String o = "output=" + output;
                String required = joinStrings(prefix, suffix, ",", p, o);
                assertEquals(required, rf.toString());
            }
        }
    }

    /**
     * Ensure that {@link RedirectFilter} is serializable.
     */
    @Test
    public void testSerialize() {
        boolean[] bools = {true, false};
        for (VInterfacePath path: createPaths()) {
            for (boolean output: bools) {
                RedirectFilter rf = (path instanceof VBridgeIfPath)
                    ? new RedirectFilter((VBridgeIfPath)path, output)
                    : new RedirectFilter((VTerminalIfPath)path, output);
                serializeTest(rf);
            }
        }
    }

    /**
     * Ensure that {@link RedirectFilter} is mapped to XML root element.
     */
    @Test
    public void testJAXB() {
        boolean[] bools = {true, false};
        for (VInterfacePath path: createPaths()) {
            for (boolean output: bools) {
                RedirectFilter rf = (path instanceof VBridgeIfPath)
                    ? new RedirectFilter((VBridgeIfPath)path, output)
                    : new RedirectFilter((VTerminalIfPath)path, output);
                jaxbTest(rf, RedirectFilter.class, "redirect");
            }
        }
    }

    /**
     * Ensure that {@link RedirectFilter} is mapped to JSON object.
     */
    @Test
    public void testJSON() {
        boolean[] bools = {true, false};
        for (VInterfacePath path: createPaths()) {
            for (boolean output: bools) {
                RedirectFilter rf = (path instanceof VBridgeIfPath)
                    ? new RedirectFilter((VBridgeIfPath)path, output)
                    : new RedirectFilter((VTerminalIfPath)path, output);
                jsonTest(rf, RedirectFilter.class);
            }
        }

        // Error detection test: Virtual node name is not specified.
        ObjectMapper mapper = getJsonObjectMapper();
        try {
            JSONObject json = new JSONObject();
            JSONObject dest = new JSONObject();
            dest.put("interface", "if_1");
            json.put("destination", dest);

            RedirectFilter rf =
                mapper.readValue(json.toString(), RedirectFilter.class);
            VInterfacePath path = rf.getDestination();
            assertEquals(ErrorVNodePath.class, path.getClass());
            ErrorVNodePath err = (ErrorVNodePath)path;
            VNodeLocation loc =
                new VNodeLocation(new VBridgeIfPath(null, null, "if_1"));
            assertEquals("Unexpected interface location: " + loc,
                         err.getError());
        } catch (Exception e) {
            unexpected(e);
        }

        // Error detection test: Interface name is not specified.
        VNodePath[] paths = {
            new VBridgePath((String)null, "node1"),
            new VTerminalPath((String)null, "node2"),
        };
        for (VNodePath vnpath: paths) {
            try {
                JSONObject json = new JSONObject();
                JSONObject dest = new JSONObject();
                String key = (vnpath instanceof VBridgePath)
                    ? "bridge" : "terminal";
                dest.put(key, vnpath.getTenantNodeName());
                json.put("destination", dest);

                RedirectFilter rf =
                    mapper.readValue(json.toString(), RedirectFilter.class);
                VInterfacePath path = rf.getDestination();
                assertEquals(ErrorVNodePath.class, path.getClass());
                ErrorVNodePath err = (ErrorVNodePath)path;
                VNodeLocation loc = (vnpath instanceof VBridgePath)
                    ? new VNodeLocation((VBridgePath)vnpath)
                    : new VNodeLocation((VTerminalPath)vnpath);
                assertEquals("Interface name is not specified: " + loc,
                             err.getError());
            } catch (Exception e) {
                unexpected(e);
            }
        }
    }

    /**
     * Create a list of {@link VInterfacePath} instances.
     *
     * @return  A list of {@link VInterfacePath} instances.
     */
    private List<VInterfacePath> createPaths() {
        List<VInterfacePath> list = interfaces;
        if (list == null) {
            list = new ArrayList<VInterfacePath>();
            list.add(null);
            for (String tname: createStrings("tenant")) {
                for (String bname: createStrings("vbr", false)) {
                    for (String iname: createStrings("iface", false)) {
                        list.add(new VBridgeIfPath(tname, bname, iname));
                        list.add(new VTerminalIfPath(tname, bname, iname));
                    }
                }
            }
            interfaces = list;
        }

        return list;
    }
}
