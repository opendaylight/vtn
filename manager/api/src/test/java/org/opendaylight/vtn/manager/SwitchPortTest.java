/*
 * Copyright (c) 2013, 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager;

import java.util.HashSet;
import java.util.List;

import org.junit.Test;

/**
 * JUnit test for {@link SwitchPort}.
 */
public class SwitchPortTest extends TestBase {
    /**
     * Test case for getter methods.
     */
    @Test
    public void testGetter() {
        for (String name: createStrings("name")) {
            SwitchPort port = new SwitchPort(name);
            assertEquals(name, port.getName());
            assertNull(port.getType());
            assertNull(port.getId());

            for (String type: createStrings("node type")) {
                for (String id: createStrings("node ID")) {
                    port = new SwitchPort(type, id);
                    assertEquals(type, port.getType());
                    assertEquals(id, port.getId());
                    assertNull(port.getName());

                    port = new SwitchPort(name, type, id);
                    assertEquals(name, port.getName());
                    assertEquals(type, port.getType());
                    assertEquals(id, port.getId());
                }
            }
        }
    }

    /**
     * Test case for {@link SwitchPort#equals(Object)} and
     * {@link SwitchPort#hashCode()}.
     */
    @Test
    public void testEquals() {
        HashSet<Object> set = new HashSet<Object>();
        List<SwitchPort> ports = createSwitchPorts(100, false);
        for (SwitchPort port: ports) {
            SwitchPort p1 = copy(port);
            testEquals(set, port, p1);
        }

        assertEquals(ports.size(), set.size());
    }

    /**
     * Test case for {@link SwitchPort#toString()}.
     */
    @Test
    public void testToString() {
        String prefix = "SwitchPort[";
        String suffix = "]";
        for (SwitchPort port: createSwitchPorts(100, false)) {
            String name = port.getName();
            String type = port.getType();
            String id = port.getId();
            if (name != null) {
                name = "name=" + name;
            }
            if (type != null) {
                type = "type=" + type;
            }
            if (id != null) {
                id = "id=" + id;
            }
            String required = joinStrings(prefix, suffix, ",", name, type, id);
            assertEquals(required, port.toString());
        }
    }

    /**
     * Ensure that {@link SwitchPort} is serializable.
     */
    @Test
    public void testSerialize() {
        for (SwitchPort port: createSwitchPorts(100, false)) {
            serializeTest(port);
        }
    }

    /**
     * Ensure that {@link SwitchPort} is mapped to XML root element.
     */
    @Test
    public void testJAXB() {
        for (SwitchPort port: createSwitchPorts(100, false)) {
            jaxbTest(port, SwitchPort.class, "switchport");
        }
    }
}
