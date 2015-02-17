/*
 * Copyright (c) 2014-2015 NEC Corporation
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

import org.opendaylight.controller.sal.action.SetNwTos;

/**
 * JUnit test for {@link SetDscpAction}.
 */
public class SetDscpActionTest extends TestBase {
    /**
     * Test case for getter methods.
     */
    @Test
    public void testGetter() {
        byte[] bytes = {Byte.MIN_VALUE, 0, 1, 30, 63, Byte.MAX_VALUE};
        for (byte b: bytes) {
            SetDscpAction act = new SetDscpAction(b);
            assertEquals(b, act.getDscp());
        }

        for (int b = 0; b <= 63; b++) {
            SetNwTos sact = new SetNwTos(b);
            SetDscpAction act = new SetDscpAction(sact);
            assertEquals(b, act.getDscp());
        }
    }

    /**
     * Test case for {@link SetDscpAction#equals(Object)} and
     * {@link SetDscpAction#hashCode()}.
     */
    @Test
    public void testEquals() {
        HashSet<Object> set = new HashSet<Object>();
        byte[] bytes = {Byte.MIN_VALUE, 0, 1, 30, 63, Byte.MAX_VALUE};
        for (byte b: bytes) {
            SetDscpAction act1 = new SetDscpAction(b);
            SetDscpAction act2 = new SetDscpAction(b);
            testEquals(set, act1, act2);
        }

        assertEquals(bytes.length, set.size());
    }

    /**
     * Test case for {@link SetDscpAction#toString()}.
     */
    @Test
    public void testToString() {
        byte[] bytes = {Byte.MIN_VALUE, 0, 1, 30, 63, Byte.MAX_VALUE};
        for (byte b: bytes) {
            SetDscpAction act = new SetDscpAction(b);
            assertEquals("SetDscpAction[dscp=" + b + "]", act.toString());
        }
    }

    /**
     * Ensure that {@link SetDscpAction} is serializable.
     */
    @Test
    public void testSerialize() {
        byte[] bytes = {Byte.MIN_VALUE, 0, 1, 30, 63, Byte.MAX_VALUE};
        for (byte b: bytes) {
            SetDscpAction act = new SetDscpAction(b);
            serializeTest(act);
        }
    }

    /**
     * Ensure that {@link SetDscpAction} is mapped to XML root element.
     */
    @Test
    public void testJAXB() {
        byte[] bytes = {Byte.MIN_VALUE, 0, 1, 30, 63, Byte.MAX_VALUE};
        for (byte b: bytes) {
            SetDscpAction act = new SetDscpAction(b);
            jaxbTest(act, "setdscp");
        }
    }

    /**
     * Ensure that {@link SetDscpAction} is mapped to JSON object.
     */
    @Test
    public void testJSON() {
        byte[] bytes = {Byte.MIN_VALUE, 0, 1, 30, 63, Byte.MAX_VALUE};
        for (byte b: bytes) {
            SetDscpAction act = new SetDscpAction(b);
            jsonTest(act, SetDscpAction.class);
        }
    }
}
