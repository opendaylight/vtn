/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.inventory;

import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.port.rev130925.PortConfig;

/**
 * Builder class for {@link PortConfig}.
 */
public final class PortConfigBuilder {
    /**
     * Current value of PORT-DOWN bit.
     */
    private Boolean  portDown;

    /**
     * Current value of NO-RECV bit.
     */
    private Boolean  noRecv;

    /**
     * Current value of NO-FWD bit.
     */
    private Boolean  noFwd;

    /**
     * Current value of NO-PACKET-IN.
     */
    private Boolean  noPacketIn;

    /**
     * Return the current value if PORT-DOWN bit.
     *
     * @return  A {@link Boolean} instance which represents the value of
     *          PORT-DOWN bit.
     */
    public Boolean isPortDown() {
        return portDown;
    }

    /**
     * Return the current value if NO-RECV bit.
     *
     * @return  A {@link Boolean} instance which represents the value of
     *          NO-RECV bit.
     */
    public Boolean isNoRecv() {
        return noRecv;
    }

    /**
     * Return the current value if NO-FWD bit.
     *
     * @return  A {@link Boolean} instance which represents the value of
     *          NO-FWD bit.
     */
    public Boolean isNoFwd() {
        return noFwd;
    }

    /**
     * Return the current value if NO-PACKET-IN bit.
     *
     * @return  A {@link Boolean} instance which represents the value of
     *          NO-PACKET-IN bit.
     */
    public Boolean isNoPacketIn() {
        return noPacketIn;
    }

    /**
     * Set the value of PORT-DOWN bit.
     *
     * @param b  A value to be set to PORT-DOWN bit.
     * @return  This instance.
     */
    public PortConfigBuilder setPortDown(Boolean b) {
        portDown = b;
        return this;
    }

    /**
     * Set the value of NO-RECV bit.
     *
     * @param b  A value to be set to NO-RECV bit.
     * @return  This instance.
     */
    public PortConfigBuilder setNoRecv(Boolean b) {
        noRecv = b;
        return this;
    }

    /**
     * Set the value of NO-FWD bit.
     *
     * @param b  A value to be set to NO-FWD bit.
     * @return  This instance.
     */
    public PortConfigBuilder setNoFwd(Boolean b) {
        noFwd = b;
        return this;
    }

    /**
     * Set the value of NO-PACKET-IN bit.
     *
     * @param b  A value to be set to NO-PACKET-IN bit.
     * @return  This instance.
     */
    public PortConfigBuilder setNoPacketIn(Boolean b) {
        noPacketIn = b;
        return this;
    }

    /**
     * Construct a new {@link PortConfig} instance with specifying
     * a set of values configured in this instance.
     *
     * @return  A {@link PortConfig} instance.
     */
    public PortConfig build() {
        return new PortConfig(noFwd, noPacketIn, noRecv, portDown);
    }
}
