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

import org.opendaylight.controller.sal.action.SetTpSrc;

/**
 * JUnit test for {@link SetIcmpTypeAction}.
 */
public class SetIcmpTypeActionTest extends TestBase {
    /**
     * Test case for getter methods.
     */
    @Test
    public void testGetter() {
        short[] types = {Short.MIN_VALUE, 0, 1, 100, 255, Short.MAX_VALUE};
        for (short type: types) {
            SetIcmpTypeAction act = new SetIcmpTypeAction(type);
            assertEquals(type, act.getType());
        }

        short[] valid = {1, 30, 90, 130, 180, 200, 254, 255};
        for (short type: valid) {
            SetTpSrc sact = new SetTpSrc((int)type);
            SetIcmpTypeAction act = new SetIcmpTypeAction(sact);
            assertEquals(type, act.getType());
        }
    }

    /**
     * Test case for {@link SetIcmpTypeAction#equals(Object)} and
     * {@link SetIcmpTypeAction#hashCode()}.
     */
    @Test
    public void testEquals() {
        HashSet<Object> set = new HashSet<Object>();
        short[] types = {Short.MIN_VALUE, 0, 1, 100, 255, Short.MAX_VALUE};
        for (short type: types) {
            SetIcmpTypeAction act1 = new SetIcmpTypeAction(type);
            SetIcmpTypeAction act2 = new SetIcmpTypeAction(type);
            testEquals(set, act1, act2);
        }

        assertEquals(types.length, set.size());
    }

    /**
     * Test case for {@link SetIcmpTypeAction#toString()}.
     */
    @Test
    public void testToString() {
        short[] types = {Short.MIN_VALUE, 0, 1, 100, 255, Short.MAX_VALUE};
        for (short type: types) {
            SetIcmpTypeAction act = new SetIcmpTypeAction(type);
            assertEquals("SetIcmpTypeAction[type=" + type + "]",
                         act.toString());
        }
    }

    /**
     * Ensure that {@link SetIcmpTypeAction} is serializable.
     */
    @Test
    public void testSerialize() {
        short[] types = {Short.MIN_VALUE, 0, 1, 100, 255, Short.MAX_VALUE};
        for (short type: types) {
            SetIcmpTypeAction act = new SetIcmpTypeAction(type);
            serializeTest(act);
        }
    }

    /**
     * Ensure that {@link SetIcmpTypeAction} is mapped to XML root element.
     */
    @Test
    public void testJAXB() {
        short[] types = {Short.MIN_VALUE, 0, 1, 100, 255, Short.MAX_VALUE};
        for (short type: types) {
            SetIcmpTypeAction act = new SetIcmpTypeAction(type);
            jaxbTest(act, "seticmptype");
        }
    }

    /**
     * Ensure that {@link SetIcmpTypeAction} is mapped to JSON object.
     */
    @Test
    public void testJSON() {
        short[] types = {Short.MIN_VALUE, 0, 1, 100, 255, Short.MAX_VALUE};
        for (short type: types) {
            SetIcmpTypeAction act = new SetIcmpTypeAction(type);
            jsonTest(act);
        }
    }
}
