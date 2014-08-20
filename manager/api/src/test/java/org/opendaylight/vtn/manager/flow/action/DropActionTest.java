/*
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.flow.action;

import java.util.HashSet;

import org.junit.Test;

import org.opendaylight.vtn.manager.TestBase;

import org.opendaylight.controller.sal.action.Drop;

/**
 * JUnit test for {@link DropAction}.
 */
public class DropActionTest extends TestBase {
    /**
     * Test case for {@link DropAction#equals(Object)} and
     * {@link DropAction#hashCode()}.
     */
    @Test
    public void testEquals() {
        HashSet<Object> set = new HashSet<Object>();
        assertTrue(set.add(new DropAction()));

        Drop drop = new Drop();
        for (int i = 0; i < 10; i++) {
            assertFalse(set.add(new DropAction()));
            assertFalse(set.add(new DropAction(drop)));
        }
        assertEquals(1, set.size());
    }

    /**
     * Test case for {@link DropAction#toString()}.
     */
    @Test
    public void testToString() {
        for (int i = 0; i < 10; i++) {
            DropAction drop = new DropAction();
            assertEquals("DropAction", drop.toString());
        }
    }

    /**
     * Ensure that {@link DropAction} is serializable.
     */
    @Test
    public void testSerialize() {
        for (int i = 0; i < 10; i++) {
            serializeTest(new DropAction());
        }
    }

    /**
     * Ensure that {@link DropAction} is mapped to XML root element.
     */
    @Test
    public void testJAXB() {
        for (int i = 0; i < 10; i++) {
            jaxbTest(new DropAction(), "drop");
        }
    }

    /**
     * Ensure that {@link DropAction} is mapped to JSON object.
     */
    @Test
    public void testJSON() {
        for (int i = 0; i < 10; i++) {
            jsonTest(new DropAction());
        }
    }
}
