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
 * JUnit test for {@link VInterfaceConfig}.
 */
public class VInterfaceConfigTest extends TestBase {
    /**
     * Test case for getter methods.
     */
    @Test
    public void testGetter() {
        for (String desc: createStrings("description")) {
            for (Boolean enabled: createBooleans()) {
                VInterfaceConfig iconf = new VInterfaceConfig(desc, enabled);
                assertEquals(desc, iconf.getDescription());
                assertEquals(enabled, iconf.getEnabled());
            }
        }
    }

    /**
     * Test case for {@link VInterfaceConfig#equals(Object)} and
     * {@link VInterfaceConfig#hashCode()}.
     */
    @Test
    public void testEquals() {
        HashSet<Object> set = new HashSet<Object>();
        List<String> descs = createStrings("description");
        List<Boolean> bools = createBooleans();
        for (String desc: descs) {
            for (Boolean enabled: bools) {
                VInterfaceConfig ic1 = new VInterfaceConfig(desc, enabled);
                VInterfaceConfig ic2 =
                    new VInterfaceConfig(copy(desc), copy(enabled));
                testEquals(set, ic1, ic2);
            }
        }

        assertEquals(descs.size() * bools.size(), set.size());
    }

    /**
     * Test case for {@link VInterfaceConfig#toString()}.
     */
    @Test
    public void testToString() {
        String prefix = "VInterfaceConfig[";
        String suffix = "]";
        for (String desc: createStrings("desc")) {
            for (Boolean enabled: createBooleans()) {
                VInterfaceConfig iconf = new VInterfaceConfig(desc, enabled);
                if (desc != null) {
                    desc = "desc=" + desc;
                }

                String required;
                if (enabled != null) {
                    String en = (enabled.booleanValue())
                        ? "enabled" : "disabled";
                    required = joinStrings(prefix, suffix, ",", desc, en);
                } else {
                    required = joinStrings(prefix, suffix, ",", desc);
                }

                assertEquals(required, iconf.toString());
            }
        }
    }

    /**
     * Ensure that {@link VInterfaceConfig} is serializable.
     */
    @Test
    public void testSerialize() {
        for (String desc: createStrings("description")) {
            for (Boolean enabled: createBooleans()) {
                VInterfaceConfig iconf = new VInterfaceConfig(desc, enabled);
                serializeTest(iconf);
            }
        }
    }

    /**
     * Ensure that {@link VInterfaceConfig} is mapped to XML root element.
     */
    @Test
    public void testJAXB() {
        for (String desc: createStrings("description")) {
            for (Boolean enabled: createBooleans()) {
                VInterfaceConfig iconf = new VInterfaceConfig(desc, enabled);
                jaxbTest(iconf, VInterfaceConfig.class, "interfaceconf");
            }
        }
    }
}
