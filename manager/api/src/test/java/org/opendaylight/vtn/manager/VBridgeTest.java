/*
 * Copyright (c) 2013, 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager;

import java.util.Collections;
import java.util.HashSet;
import java.util.List;

import javax.xml.bind.Unmarshaller;

import org.junit.Test;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeState;

/**
 * JUnit test for {@link VBridge}.
 */
public class VBridgeTest extends TestBase {
    /**
     * Root XML element name associated with {@link VBridge} class.
     */
    private static final String  XML_ROOT = "vbridge";

    /**
     * Test case for getter methods.
     */
    @Test
    public void testGetter() {
        VnodeState[] stateValues = VnodeState.values();
        VnodeState[] states = new VnodeState[stateValues.length + 1];
        System.arraycopy(stateValues, 0, states, 1, stateValues.length);
        states[0] = null;

        for (String name: createStrings("nm")) {
            for (String desc: createStrings("desc")) {
                for (Integer ival: createIntegers(-2, 5)) {
                    VBridgeConfig bconf = createVBridgeConfig(desc, ival);
                    for (int flt = 0; flt < 3; flt++) {
                        for (VnodeState state: states) {
                            String emsg = "(name)" + name + ",(desc)" + desc
                                    + ",(ival)" + ((ival == null) ? "null" : ival.intValue())
                                    + ",(flt)" + flt;

                            VBridge vbridge = new VBridge(name, state, flt,
                                                          bconf);
                            assertEquals(emsg, name, vbridge.getName());
                            assertEquals(emsg, desc, vbridge.getDescription());
                            assertEquals(emsg, flt, vbridge.getFaults());
                            if (ival == null || ival.intValue() < 0) {
                                assertEquals(emsg, -1, vbridge.getAgeInterval());
                                assertNull(emsg, vbridge.getAgeIntervalValue());
                            } else {
                                assertEquals(emsg, ival.intValue(),
                                             vbridge.getAgeInterval());
                                assertEquals(emsg, ival,
                                             vbridge.getAgeIntervalValue());
                            }

                            // null state should be interpreted as UNKNOWN.
                            if (state == null) {
                                state = VnodeState.UNKNOWN;
                            }
                            assertSame(emsg, state, vbridge.getState());
                        }
                    }
                }
            }
        }
    }

    /**
     * Test case for {@link VBridge#equals(Object)} and
     * {@link VBridge#hashCode()}.
     */
    @Test
    public void testEquals() {
        HashSet<Object> set = new HashSet<Object>();
        List<String> names = createStrings("nm");
        List<String> descs = createStrings("desc");
        List<Integer> intervals = createIntegers(10, 3);
        List<Integer> faults = createIntegers(0, 2, false);
        VnodeState[] states = VnodeState.values();
        for (String name: names) {
            for (String desc: descs) {
                for (Integer ival: intervals) {
                    for (Integer flt: faults) {
                        for (VnodeState state: states) {
                            int f = flt.intValue();
                            VBridgeConfig bconf =
                                createVBridgeConfig(desc, ival);
                            VBridge b1 = new VBridge(name, state, f, bconf);
                            bconf = createVBridgeConfig(copy(desc), ival);
                            VBridge b2 = new VBridge(copy(name), state, f,
                                                     bconf);
                            testEquals(set, b1, b2);
                        }
                    }
                }
            }
        }

        int required = names.size() * descs.size() * intervals.size() *
            faults.size() * states.length;
        assertEquals(required, set.size());
    }

    /**
     * Test case for {@link VBridge#toString()}.
     */
    @Test
    public void testToString() {
        String prefix = "VBridge[";
        String suffix = "]";
        VnodeState[] states = VnodeState.values();
        for (String name: createStrings("nm")) {
            for (String desc: createStrings("desc")) {
                for (Integer ival: createIntegers(10, 2)) {
                    for (int flt = 0; flt < 3; flt++) {
                        for (VnodeState state: states) {
                            VBridgeConfig bconf =
                                createVBridgeConfig(desc, ival);
                            VBridge vbridge = new VBridge(name, state, flt,
                                                          bconf);
                            String n = (name == null) ? null : "name=" + name;
                            String d = (desc == null) ? null : "desc=" + desc;
                            String i = (ival == null)
                                ? null : "ageInterval=" + ival;
                            String f = "faults=" + flt;
                            String s = "state=" + state;

                            String required =
                                joinStrings(prefix, suffix, ",", n, d, i,
                                            f, s);
                            assertEquals(required, vbridge.toString());
                        }
                    }
                }
            }
        }
    }

    /**
     * Ensure that {@link VBridge} is serializable.
     */
    @Test
    public void testSerialize() {
        VnodeState[] states = VnodeState.values();
        for (String name: createStrings("nm")) {
            for (String desc: createStrings("desc")) {
                for (Integer ival: createIntegers(10, 2)) {
                    for (int flt = 0; flt < 3; flt++) {
                        for (VnodeState state: states) {
                            VBridgeConfig bconf =
                                createVBridgeConfig(desc, ival);
                            VBridge vbridge =
                                new VBridge(name, state, flt, bconf);
                            serializeTest(vbridge);
                        }
                    }
                }
            }
        }
    }

    /**
     * Ensure that {@link VBridge} is mapped to XML root element.
     */
    @Test
    public void testJAXB() {
        VnodeState[] states = VnodeState.values();
        for (String name: createStrings("nm")) {
            for (String desc: createStrings("desc")) {
                for (Integer ival: createIntegers(10, 2)) {
                    for (VnodeState state: states) {
                        VBridgeConfig bconf = createVBridgeConfig(desc, ival);
                        VBridge vbridge = new VBridge(name, state, 0, bconf);
                        jaxbTest(vbridge, VBridge.class, XML_ROOT);
                        vbridge = new VBridge(name, state, 1, bconf);
                        jaxbTest(vbridge, VBridge.class, XML_ROOT);
                    }

                    // Ensure that "state" attribute is decoded as VnodeState.
                    jaxbStateTest(name, desc, ival, null, -1,
                                  VnodeState.UNKNOWN);
                    jaxbStateTest(name, desc, ival, null, 0, VnodeState.DOWN);
                    jaxbStateTest(name, desc, ival, null, 1, VnodeState.UP);

                    // Ensure that UNKNOWN is set to the state if "state"
                    // attribute is omitted.
                    jaxbStateTest(name, desc, ival, new Integer(3), null,
                                  VnodeState.UNKNOWN);

                    // Ensure that UNKNOWN is set to the state if "state"
                    // attribute is invalid.
                    for (int st = -3; st <= 3; st++) {
                        if (st >= -1 && st <= 1) {
                            continue;
                        }
                        jaxbStateTest(name, desc, ival, new Integer(5),
                                      new Integer(st), VnodeState.UNKNOWN);
                    }
                }
            }
        }

        // Ensure that broken values in XML can be detected.
        List<XmlDataType> dlist = VBridgeConfigTest.getXmlDataTypes(XML_ROOT);
        Collections.addAll(dlist,
                           new XmlAttributeType(XML_ROOT, "faults", int.class),
                           new XmlAttributeType(XML_ROOT, "state", int.class));
        jaxbErrorTest(VBridge.class, dlist);
    }

    /**
     * JAXB test for the "state" attribute.
     *
     * @param name      The name of the bridge.
     * @param desc      Description about the bridge.
     * @param ival      Interval of MAC address table aging.
     * @param faults    The number of path faults.
     * @param state     The state value of the bridge.
     * @param required  Required node state.
     */
    private void jaxbStateTest(String name, String desc, Integer ival,
                               Integer faults, Integer state,
                               VnodeState required) {
        StringBuilder builder = new StringBuilder(XML_DECLARATION);
        builder.append('<').append(XML_ROOT);
        if (name != null) {
            builder.append(" name=\"").append(name).append('"');
        }
        if (desc != null) {
            builder.append(" description=\"").append(desc).append('"');
        }
        if (ival != null) {
            builder.append(" ageInterval=\"").append(ival.toString()).
                append('"');
        }
        if (faults != null) {
            builder.append(" faults=\"").append(faults.toString()).
                append('"');
        }
        if (state != null) {
            builder.append(" state=\"").append(state.toString()).append('"');
        }

        String xml = builder.append("/>").toString();
        Unmarshaller um = createUnmarshaller(VBridge.class);
        VBridge vbridge = null;
        try {
            vbridge = unmarshal(um, xml, VBridge.class);
        } catch (Exception e) {
            unexpected(e);
        }

        assertEquals(name, vbridge.getName());
        assertEquals(desc, vbridge.getDescription());
        int flt = (faults != null) ? faults.intValue() : 0;
        assertEquals(flt, vbridge.getFaults());
        if (ival == null || ival.intValue() < 0) {
            assertEquals(-1, vbridge.getAgeInterval());
            assertNull(vbridge.getAgeIntervalValue());
        } else {
            assertEquals(ival.intValue(), vbridge.getAgeInterval());
            assertEquals(ival, vbridge.getAgeIntervalValue());
        }
        assertSame(required, vbridge.getState());
    }
}
