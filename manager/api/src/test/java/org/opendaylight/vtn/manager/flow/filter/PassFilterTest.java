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

import org.opendaylight.vtn.manager.VTerminalIfPath;

import org.opendaylight.vtn.manager.TestBase;

/**
 * JUnit test for {@link PassFilter}.
 */
public class PassFilterTest extends TestBase {
    /**
     * Test case for {@link PassFilter#equals(Object)} and
     * {@link PassFilter#hashCode()}.
     */
    @Test
    public void testEquals() {
        HashSet<Object> set = new HashSet<Object>();
        testEquals(set, new PassFilter(), new PassFilter());
        for (int i = 0; i < 10; i++) {
            assertFalse(set.add(new PassFilter()));
        }

        assertEquals(1, set.size());
        assertTrue(set.add(new DropFilter()));

        VTerminalIfPath path = new VTerminalIfPath("vtn", "vtm", "if");
        assertTrue(set.add(new RedirectFilter(path, false)));
        assertEquals(3, set.size());
    }

    /**
     * Test case for {@link PassFilter#toString()}.
     */
    @Test
    public void testToString() {
        for (int i = 0; i < 10; i++) {
            PassFilter pf = new PassFilter();
            assertEquals("PassFilter", pf.toString());
        }
    }

    /**
     * Ensure that {@link PassFilter} is serializable.
     */
    @Test
    public void testSerialize() {
        for (int i = 0; i < 10; i++) {
            serializeTest(new PassFilter());
        }
    }

    /**
     * Ensure that {@link PassFilter} is mapped to XML root element.
     */
    @Test
    public void testJAXB() {
        for (int i = 0; i < 10; i++) {
            jaxbTest(new PassFilter(), PassFilter.class, "pass");
        }
    }

    /**
     * Ensure that {@link PassFilter} is mapped to JSON object.
     */
    @Test
    public void testJSON() {
        for (int i = 0; i < 10; i++) {
            jsonTest(new PassFilter(), PassFilter.class);
        }
    }
}
