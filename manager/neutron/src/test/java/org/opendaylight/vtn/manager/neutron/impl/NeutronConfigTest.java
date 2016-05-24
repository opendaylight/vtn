/*
 * Copyright (c) 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.neutron.impl;

import org.junit.Test;

/**
 * JUnit test for {@link NeutronConfig}.
 */
public final class NeutronConfigTest extends TestBase {
    /**
     * The default name of the OVS bridge.
     */
    static final String  DEFAULT_BRIDGE_NAME = "br-int";

    /**
     * The default value of the port name in the OVS bridge.
     */
    static final String  DEFAULT_PORT_NAME = "eth0";

    /**
     * The default value of the OVSDB failmode.
     */
    static final String  DEFAULT_FAILMODE = "secure";

    /**
     * The default value of the protocol used by the OVS.
     */
    static final String  DEFAULT_PROTOCOL = "OpenFlow13";

    /**
     * Test case for {@link NeutronConfig#NeutronConfig()}.
     */
    @Test
    public void testConstructor1() {
        NeutronConfig cfg = new NeutronConfig();
        assertEquals(DEFAULT_BRIDGE_NAME, cfg.getOvsdbBridgeName());
        assertEquals(DEFAULT_PORT_NAME, cfg.getOvsdbPortName());
        assertEquals(DEFAULT_PROTOCOL, cfg.getOvsdbProtocol());
        assertEquals(DEFAULT_FAILMODE, cfg.getOvsdbFailMode());
    }

    /**
     * Test case for {@link NeutronConfig#getOvsdbBridgeName()}.
     */
    @Test
    public void testGetOvsdbBridgeName() {
        String[] args = {null, "bridge_1", "bridge_2"};
        for (String arg: args) {
            String expected = (arg == null) ? DEFAULT_BRIDGE_NAME : arg;
            NeutronConfig cfg = new NeutronConfig(arg, null, null, null);
            assertEquals(expected, cfg.getOvsdbBridgeName());
            assertEquals(DEFAULT_PORT_NAME, cfg.getOvsdbPortName());
            assertEquals(DEFAULT_PROTOCOL, cfg.getOvsdbProtocol());
            assertEquals(DEFAULT_FAILMODE, cfg.getOvsdbFailMode());
        }
    }

    /**
     * Test case for {@link NeutronConfig#getOvsdbPortName()}.
     */
    @Test
    public void testGetOvsdbPortName() {
        String[] args = {null, "if_1", "eth1"};
        for (String arg: args) {
            String expected = (arg == null) ? DEFAULT_PORT_NAME : arg;
            NeutronConfig cfg = new NeutronConfig(null, arg, null, null);
            assertEquals(DEFAULT_BRIDGE_NAME, cfg.getOvsdbBridgeName());
            assertEquals(expected, cfg.getOvsdbPortName());
            assertEquals(DEFAULT_PROTOCOL, cfg.getOvsdbProtocol());
            assertEquals(DEFAULT_FAILMODE, cfg.getOvsdbFailMode());
        }
    }

    /**
     * Test case for {@link NeutronConfig#getOvsdbProtocol()}.
     */
    @Test
    public void testGetOvsdbProtocol() {
        String[] args = {null, "OpenFlow10", "OpenFlow14"};
        for (String arg: args) {
            String expected = (arg == null) ? DEFAULT_PROTOCOL : arg;
            NeutronConfig cfg = new NeutronConfig(null, null, arg, null);
            assertEquals(DEFAULT_BRIDGE_NAME, cfg.getOvsdbBridgeName());
            assertEquals(DEFAULT_PORT_NAME, cfg.getOvsdbPortName());
            assertEquals(expected, cfg.getOvsdbProtocol());
            assertEquals(DEFAULT_FAILMODE, cfg.getOvsdbFailMode());
        }
    }

    /**
     * Test case for {@link NeutronConfig#getOvsdbFailMode()}.
     */
    @Test
    public void testGetOvsdbFailMode() {
        String[] args = {null, "standalone", "unknown"};
        for (String arg: args) {
            String expected = (arg == null) ? DEFAULT_FAILMODE : arg;
            NeutronConfig cfg = new NeutronConfig(null, null, null, arg);
            assertEquals(DEFAULT_BRIDGE_NAME, cfg.getOvsdbBridgeName());
            assertEquals(DEFAULT_PORT_NAME, cfg.getOvsdbPortName());
            assertEquals(DEFAULT_PROTOCOL, cfg.getOvsdbProtocol());
            assertEquals(expected, cfg.getOvsdbFailMode());
        }
    }

    /**
     * Test case for {@link NeutronConfig#toString()}.
     */
    @Test
    public void testToString() {
        String[] bridges = {null, "bridge_1", "bridge_2"};
        String[] ports = {null, "eth1", "eth2"};
        String[] protocols = {null, "OpenFlow10", "OpenFlow14"};
        String[] failmodes = {null, "standalone", "unknown"};
        for (String bridge: bridges) {
            String bname = (bridge == null) ? DEFAULT_BRIDGE_NAME : bridge;
            for (String port: ports) {
                String pname = (port == null) ? DEFAULT_PORT_NAME : port;
                for (String protocol: protocols) {
                    String proto = (protocol == null)
                        ? DEFAULT_PROTOCOL : protocol;
                    for (String failmode: failmodes) {
                        String mode = (failmode == null)
                            ? DEFAULT_FAILMODE : failmode;
                        String expected = "NeutronConfig[bridge-name=" + bname +
                            ", port-name=" + pname +
                            ", protocol=" + proto +
                            ", failmode=" + mode + "]";
                        NeutronConfig cfg = new NeutronConfig(
                            bridge, port, protocol, failmode);
                        assertEquals(expected, cfg.toString());
                    }
                }
            }
        }
    }
}
