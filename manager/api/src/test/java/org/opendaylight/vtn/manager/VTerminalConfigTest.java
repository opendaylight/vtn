/*
 * Copyright (c) 2014, 2015 NEC Corporation. All rights reserved.
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
 * JUnit test for {@link VTerminalConfig}.
 */
public class VTerminalConfigTest extends TestBase {
    /**
     * Test case for getter methods.
     */
    @Test
    public void testGetter() {
        for (String desc: createStrings("description")) {
            VTerminalConfig vtconf = new VTerminalConfig(desc);
            assertEquals(desc, vtconf.getDescription());
        }
    }

    /**
     * Test case for {@link VTerminalConfig#equals(Object)} and
     * {@link VTerminalConfig#hashCode()}.
     */
    @Test
    public void testEquals() {
        HashSet<Object> set = new HashSet<Object>();
        List<String> descs = createStrings("description");
        for (String desc: descs) {
            VTerminalConfig bc1 = new VTerminalConfig(desc);
            VTerminalConfig bc2 = new VTerminalConfig(copy(desc));
            testEquals(set, bc1, bc2);
        }

        assertEquals(descs.size(), set.size());
    }

    /**
     * Test case for {@link VTerminalConfig#toString()}.
     */
    @Test
    public void testToString() {
        String prefix = "VTerminalConfig[";
        String suffix = "]";
        for (String desc: createStrings("description")) {
            VTerminalConfig vtconf = new VTerminalConfig(desc);
            String s = (desc == null) ? null : "desc=" + desc;
            String required = joinStrings(prefix, suffix, ",", s);
            assertEquals(required, vtconf.toString());
        }
    }

    /**
     * Ensure that {@link VTerminalConfig} is serializable.
     */
    @Test
    public void testSerialize() {
        for (String desc: createStrings("description")) {
            VTerminalConfig vtconf = new VTerminalConfig(desc);
            serializeTest(vtconf);
        }
    }

    /**
     * Ensure that {@link VTerminalConfig} is mapped to XML root element.
     */
    @Test
    public void testJAXB() {
        for (String desc: createStrings("description")) {
            VTerminalConfig vtconf = new VTerminalConfig(desc);
            jaxbTest(vtconf, VTerminalConfig.class, "vterminalconf");
        }
    }
}
