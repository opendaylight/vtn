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

import org.opendaylight.controller.sal.action.PushVlan;
import org.opendaylight.controller.sal.utils.EtherTypes;

/**
 * JUnit test for {@link PushVlanAction}.
 */
public class PushVlanActionTest extends TestBase {
    /**
     * Test case for getter methods.
     */
    @Test
    public void testGetter() {
        EtherTypes[] types = {EtherTypes.VLANTAGGED, EtherTypes.QINQ};
        for (EtherTypes type: types) {
            int t = type.intValue();
            PushVlanAction act = new PushVlanAction(t);
            assertEquals(t, act.getType());

            act = new PushVlanAction(type);
            assertEquals(t, act.getType());

            PushVlan sact = new PushVlan(type);
            act = new PushVlanAction(sact);
            assertEquals(t, act.getType());
        }
    }

    /**
     * Test case for {@link PushVlanAction#equals(Object)} and
     * {@link PushVlanAction#hashCode()}.
     */
    @Test
    public void testEquals() {
        HashSet<Object> set = new HashSet<Object>();
        EtherTypes[] types = {EtherTypes.VLANTAGGED, EtherTypes.QINQ};
        for (EtherTypes type: types) {
            PushVlanAction act1 = new PushVlanAction(type);
            PushVlanAction act2 = new PushVlanAction(type);
            testEquals(set, act1, act2);
        }

        assertEquals(types.length, set.size());
    }

    /**
     * Test case for {@link PushVlanAction#toString()}.
     */
    @Test
    public void testToString() {
        EtherTypes[] types = {EtherTypes.VLANTAGGED, EtherTypes.QINQ};
        for (EtherTypes type: types) {
            PushVlanAction act = new PushVlanAction(type);
            String t = Integer.toHexString(type.intValue());
            assertEquals("PushVlanAction[type=" + t + "]", act.toString());
        }
    }

    /**
     * Ensure that {@link PushVlanAction} is serializable.
     */
    @Test
    public void testSerialize() {
        EtherTypes[] types = {EtherTypes.VLANTAGGED, EtherTypes.QINQ};
        for (EtherTypes type: types) {
            PushVlanAction act = new PushVlanAction(type);
            serializeTest(act);
        }
    }

    /**
     * Ensure that {@link PushVlanAction} is mapped to XML root element.
     */
    @Test
    public void testJAXB() {
        EtherTypes[] types = {EtherTypes.VLANTAGGED, EtherTypes.QINQ};
        for (EtherTypes type: types) {
            PushVlanAction act = new PushVlanAction(type);
            jaxbTest(act, "pushvlan");
        }
    }

    /**
     * Ensure that {@link PushVlanAction} is mapped to JSON object.
     */
    @Test
    public void testJSON() {
        EtherTypes[] types = {EtherTypes.VLANTAGGED, EtherTypes.QINQ};
        for (EtherTypes type: types) {
            PushVlanAction act = new PushVlanAction(type);
            jsonTest(act, PushVlanAction.class);
        }
    }
}
