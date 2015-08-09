/*
 * Copyright (c) 2013, 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager;

import java.util.HashSet;
import java.util.ArrayList;
import java.util.List;

import javax.xml.bind.Unmarshaller;

import org.junit.Test;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeState;

/**
 * JUnit test for {@link VInterface}.
 */
public class VInterfaceTest extends TestBase {
    /**
     * Root XML element name associated with {@link VInterface} class.
     */
    private static final String  XML_ROOT = "interface";

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
            for (VInterfaceConfig iconf: createConfigs()) {
                for (VnodeState state: states) {
                    for (VnodeState estate: states) {
                        String emsg = "(name)" + name
                                + ",(state)"
                                + ((state == null) ? "null" : state.toString())
                                + ",(estate)"
                                + ((estate == null) ? "null" : estate.toString());

                        VInterface viface =
                            new VInterface(name, state, estate, iconf);
                        assertEquals(emsg, name, viface.getName());
                        assertEquals(emsg, iconf.getDescription(),
                                     viface.getDescription());
                        assertEquals(emsg, iconf.getEnabled(),
                                     viface.getEnabled());

                        // null state should be interpreted as UNKNOWN.
                        if (state == null) {
                            state = VnodeState.UNKNOWN;
                        }
                        if (estate == null) {
                            estate = VnodeState.UNKNOWN;
                        }
                        assertSame(emsg, state, viface.getState());
                        assertSame(emsg, estate, viface.getEntityState());
                    }
                }
            }
        }
    }

    /**
     * Test case for {@link VInterface#equals(Object)} and
     * {@link VInterface#hashCode()}.
     */
    @Test
    public void testEquals() {
        HashSet<Object> set = new HashSet<Object>();
        List<String> names = createStrings("nm");
        List<VInterfaceConfig> configs = createConfigs();
        VnodeState[] states = VnodeState.values();
        for (String name: names) {
            for (VInterfaceConfig iconf: configs) {
                for (VnodeState state: states) {
                    for (VnodeState estate: states) {
                        VInterface i1 =
                            new VInterface(name, state, estate, iconf);
                        VInterface i2 =
                            new VInterface(copy(name), state, estate,
                                           copy(iconf));
                        testEquals(set, i1, i2);
                    }
                }
            }
        }

        int required = names.size() * configs.size() * states.length *
            states.length;
        assertEquals(required, set.size());
    }

    /**
     * Test case for {@link VInterface#toString()}.
     */
    @Test
    public void testToString() {
        String prefix = "VInterface[";
        String suffix = "]";
        VnodeState[] states = VnodeState.values();
        for (String name: createStrings("nm")) {
            for (VInterfaceConfig iconf: createConfigs()) {
                for (VnodeState state: states) {
                    for (VnodeState estate: states) {
                        VInterface viface =
                            new VInterface(name, state, estate, iconf);

                        ArrayList<String> list = new ArrayList<String>();
                        if (name != null) {
                            list.add("name=" + name);
                        }

                        String desc = iconf.getDescription();
                        if (desc != null) {
                            list.add("desc=" + desc);
                        }

                        Boolean enabled = iconf.getEnabled();
                        if (enabled != null) {
                            if (enabled.booleanValue()) {
                                list.add("enabled");
                            } else {
                                list.add("disabled");
                            }
                        }

                        list.add("state=" + state);
                        list.add("entityState=" + estate);
                        String required =
                            joinStrings(prefix, suffix, ",", list.toArray());
                        assertEquals(required, viface.toString());
                    }
                }
            }
        }
    }

    /**
     * Ensure that {@link VInterface} is serializable.
     */
    @Test
    public void testSerialize() {
        VnodeState[] states = VnodeState.values();
        for (String name: createStrings("nm")) {
            for (VInterfaceConfig iconf: createConfigs()) {
                for (VnodeState state: states) {
                    for (VnodeState estate: states) {
                        VInterface viface =
                            new VInterface(name, state, estate, iconf);
                        serializeTest(viface);
                    }
                }
            }
        }
    }

    /**
     * Ensure that {@link VInterface} is mapped to XML root element.
     */
    @Test
    public void testJAXB() {
        VnodeState[] states = VnodeState.values();
        for (String name: createStrings("nm")) {
            for (VInterfaceConfig iconf: createConfigs()) {
                for (VnodeState state: states) {
                    for (VnodeState estate: states) {
                        VInterface viface =
                            new VInterface(name, state, estate, iconf);
                        jaxbTest(viface, VInterface.class, XML_ROOT);
                    }
                }
            }
        }

        // Ensure that "state" attribute is decoded as VnodeState.
        String name = "name";
        String desc = "description";
        Boolean enabled = Boolean.TRUE;
        jaxbStateTest(name, desc, enabled, -1, 0,
                      VnodeState.UNKNOWN, VnodeState.DOWN);
        jaxbStateTest(name, desc, enabled, 0, 1,
                      VnodeState.DOWN, VnodeState.UP);
        jaxbStateTest(name, desc, enabled, 1, -1,
                      VnodeState.UP, VnodeState.UNKNOWN);

        // Ensure that UNKNOWN is set to the state if "state"
        // attribute is omitted.
        jaxbStateTest(name, desc, enabled, null, 0,
                      VnodeState.UNKNOWN, VnodeState.DOWN);
        jaxbStateTest(name, desc, enabled, 1, null,
                      VnodeState.UP, VnodeState.UNKNOWN);

        // Ensure that UNKNOWN is set to the state if "state"
        // attribute is invalid.
        for (int st = -10; st <= 10; st++) {
            if (st >= -1 && st <= 1) {
                continue;
            }
            jaxbStateTest(name, desc, enabled, new Integer(st), 0,
                          VnodeState.UNKNOWN, VnodeState.DOWN);
            jaxbStateTest(name, desc, enabled, 1, new Integer(st),
                          VnodeState.UP, VnodeState.UNKNOWN);
        }

        // Ensure that broken values in XML can be detected.
        jaxbErrorTest(VInterface.class,
                      new XmlAttributeType(XML_ROOT, "state", int.class),
                      new XmlAttributeType(XML_ROOT, "entityState", int.class));
    }

    /**
     * JAXB test for the "state" attribute.
     *
     * @param name      The name of the interface.
     * @param desc      Description about the interface.
     * @param enabled   Whether to enable the interface.
     * @param state     The state value of the interface.
     * @param estate    The state value of the network element.
     * @param required  Required node state.
     * @param reqestate Required node state of the network element.
     */
    private void jaxbStateTest(String name, String desc, Boolean enabled,
                               Integer state, Integer estate,
                               VnodeState required, VnodeState reqestate) {
        StringBuilder builder = new StringBuilder(XML_DECLARATION);
        builder.append('<').append(XML_ROOT);
        if (name != null) {
            builder.append(" name=\"").append(name).append('"');
        }
        if (desc != null) {
            builder.append(" description=\"").append(desc).append('"');
        }
        if (enabled != null) {
            builder.append(" enabled=\"").append(enabled.toString()).
                append('"');
        }
        if (state != null) {
            builder.append(" state=\"").append(state.toString()).append('"');
        }
        if (estate != null) {
            builder.append(" entityState=\"").append(estate.toString()).
                append('"');
        }

        String xml = builder.append("/>").toString();
        Unmarshaller um = createUnmarshaller(VInterface.class);
        VInterface viface = null;
        try {
            viface = unmarshal(um, xml, VInterface.class);
        } catch (Exception e) {
            unexpected(e);
        }

        assertEquals(name, viface.getName());
        assertEquals(desc, viface.getDescription());
        assertEquals(enabled, viface.getEnabled());
        assertSame(required, viface.getState());
        assertSame(reqestate, viface.getEntityState());
    }

    /**
     * Create a list of test interface configurations.
     *
     * @return  A list of {@link VInterfaceConfig} objects.
     */
    private List<VInterfaceConfig> createConfigs() {
        ArrayList<VInterfaceConfig> list = new ArrayList<VInterfaceConfig>();

        for (String desc: createStrings("ds")) {
            for (Boolean enabled: createBooleans()) {
                VInterfaceConfig iconf = new VInterfaceConfig(desc, enabled);
                list.add(iconf);
            }
        }

        return list;
    }
}
