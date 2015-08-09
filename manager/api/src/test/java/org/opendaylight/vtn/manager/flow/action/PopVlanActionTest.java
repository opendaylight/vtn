/*
 * Copyright (c) 2014, 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.flow.action;

import java.util.HashSet;

import org.junit.Test;

import org.opendaylight.vtn.manager.TestBase;

/**
 * JUnit test for {@link PopVlanAction}.
 */
public class PopVlanActionTest extends TestBase {
    /**
     * Test case for {@link PopVlanAction#equals(Object)} and
     * {@link PopVlanAction#hashCode()}.
     */
    @Test
    public void testEquals() {
        HashSet<Object> set = new HashSet<Object>();
        assertTrue(set.add(new PopVlanAction()));

        for (int i = 0; i < 10; i++) {
            assertFalse(set.add(new PopVlanAction()));
        }
        assertEquals(1, set.size());
    }

    /**
     * Test case for {@link PopVlanAction#toString()}.
     */
    @Test
    public void testToString() {
        for (int i = 0; i < 10; i++) {
            PopVlanAction drop = new PopVlanAction();
            assertEquals("PopVlanAction", drop.toString());
        }
    }

    /**
     * Ensure that {@link PopVlanAction} is serializable.
     */
    @Test
    public void testSerialize() {
        for (int i = 0; i < 10; i++) {
            serializeTest(new PopVlanAction());
        }
    }

    /**
     * Ensure that {@link PopVlanAction} is mapped to XML root element.
     */
    @Test
    public void testJAXB() {
        for (int i = 0; i < 10; i++) {
            jaxbTest(new PopVlanAction(), PopVlanAction.class, "popvlan");
        }
    }

    /**
     * Ensure that {@link PopVlanAction} is mapped to JSON object.
     */
    @Test
    public void testJSON() {
        for (int i = 0; i < 10; i++) {
            jsonTest(new PopVlanAction(), PopVlanAction.class);
        }
    }
}
