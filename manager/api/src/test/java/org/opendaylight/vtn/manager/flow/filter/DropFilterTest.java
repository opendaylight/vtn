/*
 * Copyright (c) 2014, 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.flow.filter;

import java.util.HashSet;

import org.junit.Test;

import org.opendaylight.vtn.manager.VBridgeIfPath;

import org.opendaylight.vtn.manager.TestBase;

/**
 * JUnit test for {@link DropFilter}.
 */
public class DropFilterTest extends TestBase {
    /**
     * Test case for {@link DropFilter#equals(Object)} and
     * {@link DropFilter#hashCode()}.
     */
    @Test
    public void testEquals() {
        HashSet<Object> set = new HashSet<Object>();
        testEquals(set, new DropFilter(), new DropFilter());
        for (int i = 0; i < 10; i++) {
            assertFalse(set.add(new DropFilter()));
        }

        assertEquals(1, set.size());
        assertTrue(set.add(new PassFilter()));

        VBridgeIfPath path = new VBridgeIfPath("vtn", "vbr", "if");
        assertTrue(set.add(new RedirectFilter(path, true)));
        assertEquals(3, set.size());
    }

    /**
     * Test case for {@link DropFilter#toString()}.
     */
    @Test
    public void testToString() {
        for (int i = 0; i < 10; i++) {
            DropFilter pf = new DropFilter();
            assertEquals("DropFilter", pf.toString());
        }
    }

    /**
     * Ensure that {@link DropFilter} is serializable.
     */
    @Test
    public void testSerialize() {
        for (int i = 0; i < 10; i++) {
            serializeTest(new DropFilter());
        }
    }

    /**
     * Ensure that {@link DropFilter} is mapped to XML root element.
     */
    @Test
    public void testJAXB() {
        for (int i = 0; i < 10; i++) {
            jaxbTest(new DropFilter(), DropFilter.class, "drop");
        }
    }

    /**
     * Ensure that {@link DropFilter} is mapped to JSON object.
     */
    @Test
    public void testJSON() {
        for (int i = 0; i < 10; i++) {
            jsonTest(new DropFilter(), DropFilter.class);
        }
    }
}
