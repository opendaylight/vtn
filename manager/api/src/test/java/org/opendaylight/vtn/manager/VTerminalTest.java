/*
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager;

import java.util.HashSet;
import java.util.List;
import java.io.ByteArrayInputStream;

import javax.xml.bind.JAXB;

import org.junit.Test;

/**
 * JUnit test for {@link VTerminal}.
 */
public class VTerminalTest extends TestBase {
    /**
     * Root XML element name associated with {@link VTerminal} class.
     */
    private static final String  XML_ROOT = "vterminal";

    /**
     * Test case for getter methods.
     */
    @Test
    public void testGetter() {
        VNodeState[] stateValues = VNodeState.values();
        VNodeState[] states = new VNodeState[stateValues.length + 1];
        System.arraycopy(stateValues, 0, states, 1, stateValues.length);
        states[0] = null;

        for (String name: createStrings("nm")) {
            for (String desc: createStrings("desc")) {
                VTerminalConfig vtconf = new VTerminalConfig(desc);
                for (int flt = 0; flt < 3; flt++) {
                    for (VNodeState state: states) {
                        VTerminal vterm =
                            new VTerminal(name, state, flt, vtconf);
                        assertEquals(name, vterm.getName());
                        assertEquals(desc, vterm.getDescription());
                        assertEquals(flt, vterm.getFaults());

                        // null state should be interpreted as UNKNOWN.
                        if (state == null) {
                            state = VNodeState.UNKNOWN;
                        }
                        assertSame(state, vterm.getState());
                    }
                }
            }
        }
    }

    /**
     * Test case for {@link VTerminal#equals(Object)} and
     * {@link VTerminal#hashCode()}.
     */
    @Test
    public void testEquals() {
        HashSet<Object> set = new HashSet<Object>();
        List<String> names = createStrings("nm");
        List<String> descs = createStrings("desc");
        int nfaults = 2;
        VNodeState[] states = VNodeState.values();
        for (String name: names) {
            for (String desc: descs) {
                VTerminalConfig vtconf1 = new VTerminalConfig(desc);
                VTerminalConfig vtconf2 = new VTerminalConfig(copy(desc));
                for (int flt = 0; flt < nfaults; flt++) {
                    for (VNodeState state: states) {
                        VTerminal vt1 =
                            new VTerminal(name, state, flt, vtconf1);
                        VTerminal vt2 =
                            new VTerminal(copy(name), state, flt, vtconf2);
                        testEquals(set, vt1, vt2);
                    }
                }
            }
        }

        int required = names.size() * descs.size() * states.length * nfaults;
        assertEquals(required, set.size());
    }

    /**
     * Test case for {@link VTerminal#toString()}.
     */
    @Test
    public void testToString() {
        String prefix = "VTerminal[";
        String suffix = "]";
        VNodeState[] states = VNodeState.values();
        for (String name: createStrings("nm")) {
            for (String desc: createStrings("desc")) {
                VTerminalConfig vtconf = new VTerminalConfig(desc);
                for (int flt = 0; flt < 3; flt++) {
                    for (VNodeState state: states) {
                        VTerminal vterm =
                            new VTerminal(name, state, flt, vtconf);
                        String n = (name == null) ? null : "name=" + name;
                        String d = (desc == null) ? null : "desc=" + desc;
                        String f = "faults=" + flt;
                        String s = "state=" + state;

                        String required =
                            joinStrings(prefix, suffix, ",", n, d, f, s);
                        assertEquals(required, vterm.toString());
                    }
                }
            }
        }
    }

    /**
     * Ensure that {@link VTerminal} is serializable.
     */
    @Test
    public void testSerialize() {
        VNodeState[] states = VNodeState.values();
        for (String name: createStrings("nm")) {
            for (String desc: createStrings("desc")) {
                VTerminalConfig vtconf = new VTerminalConfig(desc);
                for (int flt = 0; flt < 3; flt++) {
                    for (VNodeState state: states) {
                        VTerminal vterm =
                            new VTerminal(name, state, flt, vtconf);
                        serializeTest(vterm);
                    }
                }
            }
        }
    }

    /**
     * Ensure that {@link VTerminal} is mapped to XML root element.
     */
    @Test
    public void testJAXB() {
        VNodeState[] states = VNodeState.values();
        for (String name: createStrings("nm")) {
            for (String desc: createStrings("desc")) {
                VTerminalConfig vtconf = new VTerminalConfig(desc);
                for (VNodeState state: states) {
                    VTerminal vterm = new VTerminal(name, state, 0, vtconf);
                    jaxbTest(vterm, XML_ROOT);

                    // Ensure that "state" attribute is decoded as
                    // VNodeState.
                    jaxbStateTest(name, desc, null, -1, VNodeState.UNKNOWN);
                    jaxbStateTest(name, desc, null, 0, VNodeState.DOWN);
                    jaxbStateTest(name, desc, null, 1, VNodeState.UP);

                    // Ensure that UNKNOWN is set to the state if "state"
                    // attribute is omitted.
                    jaxbStateTest(name, desc, Integer.valueOf(3), null,
                                  VNodeState.UNKNOWN);

                    // Ensure that UNKNOWN is set to the state if "state"
                    // attribute is invalid.
                    int faults = 0;
                    for (int st = -3; st <= 3; st++) {
                        if (st >= -1 && st <= 1) {
                            continue;
                        }
                        jaxbStateTest(name, desc, Integer.valueOf(faults),
                                      Integer.valueOf(st), VNodeState.UNKNOWN);
                        faults++;
                    }
                }
            }
        }
    }

    /**
     * JAXB test for the "state" attribute.
     *
     * @param name      The name of the vTerminal.
     * @param desc      Description about the vTerminal.
     * @param faults    The number of path faults.
     * @param state     The state value of the vTerminal.
     * @param required  Required node state.
     */
    private void jaxbStateTest(String name, String desc, Integer faults,
                               Integer state, VNodeState required) {
        StringBuilder builder = new StringBuilder(XML_DECLARATION);
        builder.append('<').append(XML_ROOT);
        if (name != null) {
            builder.append(" name=\"").append(name).append('"');
        }
        if (desc != null) {
            builder.append(" description=\"").append(desc).append('"');
        }
        if (faults != null) {
            builder.append(" faults=\"").append(faults).append('"');
        }
        if (state != null) {
            builder.append(" state=\"").append(state.toString()).append('"');
        }

        String xml = builder.append("/>").toString();
        VTerminal vterm = null;
        try {
            ByteArrayInputStream in = new ByteArrayInputStream(xml.getBytes());
            vterm = JAXB.unmarshal(in, VTerminal.class);
        } catch (Exception e) {
            unexpected(e);
        }

        assertEquals(name, vterm.getName());
        assertEquals(desc, vterm.getDescription());
        int flt = (faults == null) ? 0 : faults.intValue();
        assertEquals(flt, vterm.getFaults());
        assertSame(required, vterm.getState());
    }
}
