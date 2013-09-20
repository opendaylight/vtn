/*
 * Copyright (c) 2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */
package org.opendaylight.vtn.manager.internal.cluster;

import java.util.HashSet;
import java.util.List;

import org.junit.Test;

import org.opendaylight.vtn.manager.internal.TestBase;

/**
 * JUnit test for {@link PortProperty}.
 */
public class PortPropertyTest extends TestBase {
    /**
     * Test case for getter methods.
     */
    @Test
    public void testGetter() {
        for (String name: createStrings("port-name")) {
            for (Boolean en: createBooleans(false)) {
                boolean enabled = en.booleanValue();
                PortProperty pp = new PortProperty(name, enabled);
                assertEquals(name, pp.getName());
                assertEquals(enabled, pp.isEnabled());
            }
        }
    }

    /**
     * Test case for {@link PortProperty#equals(Object)} and
     * {@link PortProperty#hashCode()}.
     */
    @Test
    public void testEquals() {
        HashSet<Object> set = new HashSet<Object>();
        List<String> names = createStrings("port-name");
        List<Boolean> enables = createBooleans(false);
        for (String name: names) {
            for (Boolean en: enables) {
                boolean enabled = en.booleanValue();
                PortProperty pp1 = new PortProperty(name, enabled);
                PortProperty pp2 = new PortProperty(copy(name), enabled);
                testEquals(set, pp1, pp2);
            }
        }

        assertEquals(names.size() * enables.size(), set.size());
    }

    /**
     * Test case for {@link PortProperty#toString()}.
     */
    @Test
    public void testToString() {
        String prefix = "PortProperty[";
        String suffix = "]";
        for (String name: createStrings("port-name")) {
            for (Boolean en: createBooleans(false)) {
                boolean enabled = en.booleanValue();
                PortProperty pp = new PortProperty(name, enabled);
                if (name != null) {
                    name = "name=" + name;
                }
                String e = (enabled) ? "enabled" : "disabled";
                String required = joinStrings(prefix, suffix, ",", name, e);
                assertEquals(required, pp.toString());
            }
        }
    }

    /**
     * Ensure that {@link PortProperty} is serializable.
     */
    @Test
    public void testSerialize() {
        for (String name: createStrings("port-name")) {
            for (Boolean en: createBooleans(false)) {
                boolean enabled = en.booleanValue();
                PortProperty pp = new PortProperty(name, enabled);
                serializeTest(pp);
            }
        }
    }
}
