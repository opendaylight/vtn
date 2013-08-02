/*
 * Copyright (c) 2013 NEC Corporation
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
     * Test case for {@link VBridgePath#equals(Object)} and
     * {@link VBridgePath#hashCode()}.
     */
    @Test
    public void testEquals() {
        HashSet<Object> set = new HashSet<Object>();
        List<String> tnames = createStrings("tenant");
        List<String> bnames = createStrings("bridge");
        for (String tname: tnames) {
            for (String bname: bnames) {
                VBridgePath p1 = new VBridgePath(tname, bname);
                VBridgePath p2 = new VBridgePath(copy(tname), copy(bname));
                testEquals(set, p1, p2);
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
}
