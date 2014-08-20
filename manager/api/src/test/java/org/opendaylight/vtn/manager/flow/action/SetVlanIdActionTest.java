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

import org.opendaylight.controller.sal.action.SetVlanId;

/**
 * JUnit test for {@link SetVlanIdAction}.
 */
public class SetVlanIdActionTest extends TestBase {
    /**
     * Test case for getter methods.
     */
    @Test
    public void testGetter() {
        short[] vlans = {Short.MIN_VALUE, 0, 1, 100, 4095, Short.MAX_VALUE};
        for (short vlan: vlans) {
            SetVlanIdAction act = new SetVlanIdAction(vlan);
            assertEquals(vlan, act.getVlan());
        }

        short[] valid = {1, 2, 300, 1000, 2000, 3000, 4095};
        for (short vlan: valid) {
            SetVlanId sact = new SetVlanId((int)vlan);
            SetVlanIdAction act = new SetVlanIdAction(sact);
            assertEquals(vlan, act.getVlan());
        }
    }

    /**
     * Test case for {@link SetVlanIdAction#equals(Object)} and
     * {@link SetVlanIdAction#hashCode()}.
     */
    @Test
    public void testEquals() {
        HashSet<Object> set = new HashSet<Object>();
        short[] vlans = {Short.MIN_VALUE, 0, 1, 100, 4095, Short.MAX_VALUE};
        for (short vlan: vlans) {
            SetVlanIdAction act1 = new SetVlanIdAction(vlan);
            SetVlanIdAction act2 = new SetVlanIdAction(vlan);
            testEquals(set, act1, act2);
        }

        assertEquals(vlans.length, set.size());
    }

    /**
     * Test case for {@link SetVlanIdAction#toString()}.
     */
    @Test
    public void testToString() {
        short[] vlans = {Short.MIN_VALUE, 0, 1, 100, 4095, Short.MAX_VALUE};
        for (short vlan: vlans) {
            SetVlanIdAction act = new SetVlanIdAction(vlan);
            assertEquals("SetVlanIdAction[vlan=" + vlan + "]",
                         act.toString());
        }
    }

    /**
     * Ensure that {@link SetVlanIdAction} is serializable.
     */
    @Test
    public void testSerialize() {
        short[] vlans = {Short.MIN_VALUE, 0, 1, 100, 4095, Short.MAX_VALUE};
        for (short vlan: vlans) {
            SetVlanIdAction act = new SetVlanIdAction(vlan);
            serializeTest(act);
        }
    }

    /**
     * Ensure that {@link SetVlanIdAction} is mapped to XML root element.
     */
    @Test
    public void testJAXB() {
        short[] vlans = {Short.MIN_VALUE, 0, 1, 100, 4095, Short.MAX_VALUE};
        for (short vlan: vlans) {
            SetVlanIdAction act = new SetVlanIdAction(vlan);
            jaxbTest(act, "setvlanid");
        }
    }

    /**
     * Ensure that {@link SetVlanIdAction} is mapped to JSON object.
     */
    @Test
    public void testJSON() {
        short[] vlans = {Short.MIN_VALUE, 0, 1, 100, 4095, Short.MAX_VALUE};
        for (short vlan: vlans) {
            SetVlanIdAction act = new SetVlanIdAction(vlan);
            jsonTest(act);
        }
    }
}
