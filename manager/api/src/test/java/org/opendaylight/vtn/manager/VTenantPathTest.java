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
 * JUnit test for {@link VTenantPath}.
 */
public class VTenantPathTest extends TestBase {
    /**
     * Test case for getter methods.
     */
    @Test
    public void testGetter() {
        for (String tname: createStrings("tenant_name")) {
            VTenantPath path = new VTenantPath(tname);
            assertEquals(tname, path.getTenantName());
        }
    }

    /**
     * Test case for {@link VTenantPath#equals(Object)} and
     * {@link VTenantPath#hashCode()}.
     */
    @Test
    public void testEquals() {
        HashSet<Object> set = new HashSet<Object>();
        List<String> tnames = createStrings("tenant_name");
        for (String tname: tnames) {
            VTenantPath p1 = new VTenantPath(tname);
            VTenantPath p2 = new VTenantPath(copy(tname));
            testEquals(set, p1, p2);
        }

        assertEquals(tnames.size(), set.size());
    }

    /**
     * Test case for {@link VTenantPath#toString()}.
     */
    @Test
    public void testToString() {
        for (String tname: createStrings("tenant_name")) {
            VTenantPath path = new VTenantPath(tname);
            String required = (tname == null) ? "<null>" : tname;
            assertEquals(required, path.toString());
        }
    }

    /**
     * Ensure that {@link VTenantPath} is serializable.
     */
    @Test
    public void testSerialize() {
        for (String tname: createStrings("tenant_name")) {
            VTenantPath path = new VTenantPath(tname);
            serializeTest(path);
        }
    }
}
