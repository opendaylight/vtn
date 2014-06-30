/*
 * Copyright (c) 2013-2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.cluster;

import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;

import org.junit.Test;

import org.opendaylight.vtn.manager.internal.TestBase;

import org.opendaylight.controller.sal.core.Bandwidth;
import org.opendaylight.controller.sal.core.Config;
import org.opendaylight.controller.sal.core.Name;
import org.opendaylight.controller.sal.core.Property;
import org.opendaylight.controller.sal.core.State;

/**
 * JUnit test for {@link PortProperty}.
 */
public class PortPropertyTest extends TestBase {
    /**
     * Base link speed used to calculate the link cost.
     */
    private static final long  BASE_BANDWIDTH = 10L * Bandwidth.BW1Tbps;

    /**
     * The minimum value of the link cost.
     */
    private static final long  LINK_COST_MIN = 1L;

    /**
     * Test case for getter methods.
     */
    @Test
    public void testGetter() {
        long[] bandwidth = {
            1, 1000, 1000000, 2000000, 333333333,
            Bandwidth.BW1Gbps,
            Bandwidth.BW1Tbps,
            Bandwidth.BW1Pbps,
        };

        for (long speed: bandwidth) {
            long cost = BASE_BANDWIDTH / speed;
            if (cost < LINK_COST_MIN) {
                cost = LINK_COST_MIN;
            }
            for (String name: createStrings("port-name")) {
                for (Boolean en: createBooleans(false)) {
                    boolean enabled = en.booleanValue();
                    Map<String, Property> prop =
                        createProperty(name, speed, en.booleanValue());
                    PortProperty pp = new PortProperty(prop);
                    assertEquals(name, pp.getName());
                    assertEquals(enabled, pp.isEnabled());
                    assertEquals(cost, pp.getCost());
                }
            }
        }

        long defcost = BASE_BANDWIDTH / Bandwidth.BW1Gbps;
        String name = "portName";
        long speed = Bandwidth.BW100Mbps;
        long cost = BASE_BANDWIDTH / speed;
        boolean enabled = true;
        Map<String, Property> map = createProperty(name, speed, enabled);

        // In case of null property.
        PortProperty pp = new PortProperty(null);
        assertNull(pp.getName());
        assertFalse(pp.isEnabled());
        assertEquals(defcost, pp.getCost());

        // In case name property is unavailable.
        Map<String, Property> prop = new HashMap(map);
        prop.remove(Name.NamePropName);
        pp = new PortProperty(prop);
        assertEquals(null, pp.getName());
        assertTrue(pp.isEnabled());
        assertEquals(cost, pp.getCost());

        // In case config property is unavailable.
        prop = new HashMap(map);
        prop.remove(Config.ConfigPropName);
        pp = new PortProperty(prop);
        assertEquals(name, pp.getName());
        assertFalse(pp.isEnabled());
        assertEquals(cost, pp.getCost());

        // In case state property is unavailable.
        prop = new HashMap(map);
        prop.remove(State.StatePropName);
        pp = new PortProperty(prop);
        assertEquals(name, pp.getName());
        assertFalse(pp.isEnabled());
        assertEquals(cost, pp.getCost());

        // In case bandwidth property is unavailable.
        prop = new HashMap(map);
        prop.remove(Bandwidth.BandwidthPropName);
        pp = new PortProperty(prop);
        assertEquals(name, pp.getName());
        assertTrue(pp.isEnabled());
        assertEquals(defcost, pp.getCost());
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
        long[] bandwidth = {
            1, 1000, 1000000, 2000000, 333333333,
            Bandwidth.BW1Gbps,
            Bandwidth.BW1Tbps,
            Bandwidth.BW1Pbps,
        };
        for (long speed: bandwidth) {
            for (String name: names) {
                for (Boolean en: enables) {
                    Map<String, Property> prop1 =
                        createProperty(name, speed, en.booleanValue());
                    PortProperty pp1 = new PortProperty(prop1);
                    Map<String, Property> prop2 =
                        createProperty(copy(name), speed, en.booleanValue());
                    PortProperty pp2 = new PortProperty(prop2);
                    testEquals(set, pp1, pp2);
                }
            }
        }

        assertEquals(names.size() * enables.size() * bandwidth.length,
                     set.size());
    }

    /**
     * Test case for {@link PortProperty#toString()}.
     */
    @Test
    public void testToString() {
        long[] bandwidth = {
            1, 1000, 1000000, 2000000, 333333333,
            Bandwidth.BW1Gbps,
            Bandwidth.BW1Tbps,
            Bandwidth.BW1Pbps,
        };

        String prefix = "PortProperty[";
        String suffix = "]";
        for (long speed: bandwidth) {
            long cost = BASE_BANDWIDTH / speed;
            if (cost < LINK_COST_MIN) {
                cost = LINK_COST_MIN;
            }
            for (String name: createStrings("port-name")) {
                for (Boolean en: createBooleans(false)) {
                    boolean enabled = en.booleanValue();
                    Map<String, Property> prop =
                        createProperty(name, speed, en.booleanValue());
                    PortProperty pp = new PortProperty(prop);
                    if (name != null) {
                        name = "name=" + name;
                    }
                    String c = "cost=" + cost;
                    String e = (enabled) ? "enabled" : "disabled";
                    String required = joinStrings(prefix, suffix, ",",
                                                  name, c, e);
                    assertEquals(required, pp.toString());
                }
            }
        }
    }

    /**
     * Ensure that {@link PortProperty} is serializable.
     */
    @Test
    public void testSerialize() {
        long[] bandwidth = {
            1, 1000, 1000000, 2000000, 333333333,
            Bandwidth.BW1Gbps,
            Bandwidth.BW1Tbps,
            Bandwidth.BW1Pbps,
        };

        for (long speed: bandwidth) {
            for (String name: createStrings("port-name")) {
                for (Boolean en: createBooleans(false)) {
                    Map<String, Property> prop =
                        createProperty(name, speed, en.booleanValue());
                    PortProperty pp = new PortProperty(prop);
                    serializeTest(pp);
                }
            }
        }
    }

    /**
     * Create a port property map.
     *
     * @param name     The name of the switch port.
     * @param speed    The link speed configured to the port.
     * @param enabled  {@code true} if the port is enabled.
     */
    private Map<String, Property> createProperty(String name, long speed,
                                                 boolean enabled) {
        HashMap<String, Property> map = new HashMap<>();
        map.put(Name.NamePropName, new Name(name));
        map.put(Bandwidth.BandwidthPropName, new Bandwidth(speed));

        short admin, state;
        if (enabled) {
            admin = Config.ADMIN_UP;
            state = State.EDGE_UP;
        } else {
            admin = Config.ADMIN_DOWN;
            state = State.EDGE_DOWN;
        }
        map.put(Config.ConfigPropName, new Config(admin));
        map.put(State.StatePropName, new State(state));

        return map;
    }
}
