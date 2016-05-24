/*
 * Copyright (c) 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.neutron.impl;

import static org.opendaylight.vtn.manager.neutron.impl.VTNNeutronUtils.getNonNullValue;

import javax.annotation.Nonnull;

/**
 * {@code NeutronConfig} describes the configuration for the manager.neutron
 * bundle specified by the config subsystem.
 */
public final class NeutronConfig {
    /**
     * A string that indicates secure fail mode.
     */
    public static final String  FAILMODE_SECURE = "secure";

    /**
     * A string that indicates standalone fail mode.
     */
    public static final String  FAILMODE_STANDALONE = "standalone";

    /**
     * A string that indicates OpenFlow 1.3.
     */
    public static final String  PROTO_OF13 = "OpenFlow13";

    /**
     * The default name of the OVS bridge.
     */
    private static final String  DEFAULT_BRIDGE_NAME = "br-int";

    /**
     * The default value of the port name in the OVS bridge.
     */
    private static final String  DEFAULT_PORT_NAME = "eth0";

    /**
     * The default value of the protocol used by the OVS.
     */
    private static final String  DEFAULT_PROTOCOL = PROTO_OF13;

    /**
     * The default value of the OVSDB failmode.
     */
    private static final String  DEFAULT_FAILMODE = FAILMODE_SECURE;

    /**
     * The name of the bridge to create in the OVS.
     */
    private final String  ovsdbBridgeName;

    /**
     * The name of the interface to be added as port to the OVS bridge.
     */
    private final String  ovsdbPortName;

    /**
     * The OpenFlow protocol used by the OVS.
     */
    private final String  ovsdbProtocol;

    /**
     * The string that specifies the OVSDB failmode.
     */
    private final String  ovsdbFailMode;

    /**
     * Construct a new instance that contains default values.
     */
    public NeutronConfig() {
        this(null, null, null, null);
    }

    /**
     * Construct a new instance.
     *
     * @param bname  The name of the bridge to create in the OVS.
     * @param pname  The name of the port in the OVS bridge.
     * @param proto  The OpenFlow protocol used by the OVS.
     * @param fail   The OVSDB failmode.
     */
    public NeutronConfig(String bname, String pname, String proto,
                         String fail) {
        ovsdbBridgeName = getNonNullValue(bname, DEFAULT_BRIDGE_NAME);
        ovsdbPortName = getNonNullValue(pname, DEFAULT_PORT_NAME);
        ovsdbProtocol = getNonNullValue(proto, DEFAULT_PROTOCOL);
        ovsdbFailMode = getNonNullValue(fail, DEFAULT_FAILMODE);
    }

    /**
     * Return the name of the OVS bridge.
     *
     * @return  The name of the OVS bridge.
     */
    @Nonnull
    public String getOvsdbBridgeName() {
        return ovsdbBridgeName;
    }

    /**
     * Return the name of the port in the OVS bridge.
     *
     * @return  The name of the OVS bridge port.
     */
    @Nonnull
    public String getOvsdbPortName() {
        return ovsdbPortName;
    }

    /**
     * Return the OpenFlow protocol used by the OVS.
     *
     * @return  A string that indicates the OpenFlow protocol.
     */
    @Nonnull
    public String getOvsdbProtocol() {
        return ovsdbProtocol;
    }

    /**
     * Return the OVSDB failmode.
     *
     * @return  A string that indicates the OVSDB failmode.
     */
    @Nonnull
    public String getOvsdbFailMode() {
        return ovsdbFailMode;
    }

    // Object

    /**
     * Return a string representation of this instance.
     *
     * @return  A string representation of this instance.
     */
    @Override
    public String toString() {
        return "NeutronConfig[bridge-name=" + ovsdbBridgeName +
            ", port-name=" + ovsdbPortName +
            ", protocol=" + ovsdbProtocol +
            ", failmode=" + ovsdbFailMode + "]";
    }
}
