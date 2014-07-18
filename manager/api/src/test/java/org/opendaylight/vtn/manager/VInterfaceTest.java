/*
 * Copyright (c) 2013-2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager;

import java.util.HashSet;
import java.util.ArrayList;
import java.util.List;
import java.io.ByteArrayInputStream;

import javax.xml.bind.JAXB;

import org.junit.Test;

/**
 * JUnit test for {@link VInterface}.
 */
public class VInterfaceTest extends TestBase {
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
            for (VInterfaceConfig iconf: createConfigs()) {
                for (VNodeState state: states) {
                    for (VNodeState estate: states) {
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
                            state = VNodeState.UNKNOWN;
                        }
                        if (estate == null) {
                            estate = VNodeState.UNKNOWN;
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
        VNodeState[] states = VNodeState.values();
        for (String name: names) {
            for (VInterfaceConfig iconf: configs) {
                for (VNodeState state: states) {
                    for (VNodeState estate: states) {
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
        VNodeState[] states = VNodeState.values();
        for (String name: createStrings("nm")) {
            for (VInterfaceConfig iconf: createConfigs()) {
                for (VNodeState state: states) {
                    for (VNodeState estate: states) {
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
        VNodeState[] states = VNodeState.values();
        for (String name: createStrings("nm")) {
            for (VInterfaceConfig iconf: createConfigs()) {
                for (VNodeState state: states) {
                    for (VNodeState estate: states) {
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
        VNodeState[] states = VNodeState.values();
        for (String name: createStrings("nm")) {
            for (VInterfaceConfig iconf: createConfigs()) {
                for (VNodeState state: states) {
                    for (VNodeState estate: states) {
                        VInterface viface =
                            new VInterface(name, state, estate, iconf);
                        jaxbTest(viface, "interface");
                    }
                }
            }
        }


        // Ensure that "state" attribute is decoded as VNodeState.
        String name = "name";
        String desc = "description";
        Boolean enabled = Boolean.TRUE;
        jaxbStateTest(name, desc, enabled, -1, 0,
                      VNodeState.UNKNOWN, VNodeState.DOWN);
        jaxbStateTest(name, desc, enabled, 0, 1,
                      VNodeState.DOWN, VNodeState.UP);
        jaxbStateTest(name, desc, enabled, 1, -1,
                      VNodeState.UP, VNodeState.UNKNOWN);

        // Ensure that UNKNOWN is set to the state if "state"
        // attribute is omitted.
        jaxbStateTest(name, desc, enabled, null, 0,
                      VNodeState.UNKNOWN, VNodeState.DOWN);
        jaxbStateTest(name, desc, enabled, 1, null,
                      VNodeState.UP, VNodeState.UNKNOWN);

        // Ensure that UNKNOWN is set to the state if "state"
        // attribute is invalid.
        for (int st = -10; st <= 10; st++) {
            if (st >= -1 && st <= 1) {
                continue;
            }
            jaxbStateTest(name, desc, enabled, new Integer(st), 0,
                          VNodeState.UNKNOWN, VNodeState.DOWN);
            jaxbStateTest(name, desc, enabled, 1, new Integer(st),
                          VNodeState.UP, VNodeState.UNKNOWN);
        }
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
                               VNodeState required, VNodeState reqestate) {
        StringBuilder builder = new StringBuilder(
            "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>" +
            "<interface");
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
        VInterface viface = null;
        try {
            ByteArrayInputStream in = new ByteArrayInputStream(xml.getBytes());
            viface = JAXB.unmarshal(in, VInterface.class);
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
