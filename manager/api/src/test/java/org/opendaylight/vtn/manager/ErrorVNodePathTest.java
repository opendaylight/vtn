/*
 * Copyright (c) 2014, 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager;

import java.util.HashSet;
import java.util.List;

import org.junit.Test;

/**
 * JUnit test for {@link ErrorVNodePath}.
 */
public class ErrorVNodePathTest extends TestBase {
    /**
     * Test case for getter methods.
     */
    @Test
    public void testGetter() {
        for (String msg: createStrings("Error message")) {
            ErrorVNodePath path = new ErrorVNodePath(msg);
            assertEquals(msg, path.getError());
            assertEquals("*** ERROR ***", path.getNodeType());
            assertEquals(null, path.getTenantNodeName());

            try {
                path.toVNodeLocation();
                unexpected();
            } catch (IllegalStateException e) {
            }

            try {
                path.toVirtualNodePath();
                unexpected();
            } catch (IllegalStateException e) {
            }

            try {
                path.getInterfaceName();
                unexpected();
            } catch (IllegalStateException e) {
            }

            try {
                path.replaceTenantName(null);
                unexpected();
            } catch (IllegalStateException e) {
            }

            try {
                path.getRedirectFilter(true);
                unexpected();
            } catch (IllegalStateException e) {
            }

            try {
                path.vInterfaceChanged(null, null, null);
                unexpected();
            } catch (IllegalStateException e) {
            }

            try {
                path.portMapChanged(null, null, null);
                unexpected();
            } catch (IllegalStateException e) {
            }
        }
    }

    /**
     * Ensure that {@link VTenantPath#contains(VTenantPath)} works.
     */
    @Test
    public void testContains() {
        String bname = "bridge";
        String iname = "if";

        for (String msg: createStrings("Error message")) {
            ErrorVNodePath path = new ErrorVNodePath(msg);
            assertFalse(path.contains(null));
            assertTrue(path.contains(path));

            VTenantPath tpath = new VTenantPath(msg);
            assertEquals(false, path.contains(tpath));

            VBridgePath bpath = new VBridgePath(tpath, bname);
            assertEquals(false, path.contains(bpath));

            VTerminalPath tmpath = new VTerminalPath(tpath, bname);
            assertEquals(false, path.contains(tmpath));

            VTerminalIfPath tipath = new VTerminalIfPath(tmpath, iname);
            assertEquals(false, path.contains(tipath));

            VBridgeIfPath ipath = new VBridgeIfPath(bpath, iname);
            assertEquals(false, path.contains(ipath));
        }
    }

    /**
     * Test case for {@link ErrorVNodePath#equals(Object)} and
     * {@link ErrorVNodePath#hashCode()}.
     */
    @Test
    public void testEquals() {
        HashSet<Object> set = new HashSet<Object>();
        List<String> messages = createStrings("Error message");
        for (String msg: messages) {
            ErrorVNodePath p1 = new ErrorVNodePath(msg);
            ErrorVNodePath p2 = new ErrorVNodePath(copy(msg));
            testEquals(set, p1, p2);

            // VTenantPath object that is configured the same string as the
            // tenant name should be treated as different object.
            VTenantPath tpath = new VTenantPath(msg);
            assertFalse(p1.equals(tpath));
            assertFalse(tpath.equals(p1));
        }

        assertEquals(messages.size(), set.size());
    }

    /**
     * Test case for {@link ErrorVNodePath#toString()}.
     */
    @Test
    public void testToString() {
        String prefix = "*** ERROR ***:";
        for (String msg: createStrings("Error message")) {
            ErrorVNodePath path = new ErrorVNodePath(msg);
            String m = (msg == null) ? "<null>" : msg;
            String required = joinStrings(prefix, null, ".", m, "<null>");
            assertEquals(required, path.toString());
        }
    }

    /**
     * Ensure that {@link ErrorVNodePath} is serializable.
     */
    @Test
    public void testSerialize() {
        for (String msg: createStrings("Error message")) {
            ErrorVNodePath path = new ErrorVNodePath(msg);
            serializeTest(path);
        }
    }
}
