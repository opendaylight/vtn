/*
 * Copyright (c) 2014, 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal;

import org.opendaylight.vtn.manager.internal.cluster.MacVlan;
import org.opendaylight.vtn.manager.internal.cluster.MapReference;

import org.opendaylight.controller.sal.core.NodeConnector;

/**
 * An instance of {@code MacMapPortBusyLog} class represents a trace log
 * message which indicates that the MAC mapping could not established because
 * the VLAN network on a switch port is reserved by another mapping.
 */
public final class MacMapPortBusyLog extends MapLog {
    /**
     * A {@link MacVlan} instance which specifies a L2 host.
     */
    private final MacVlan  host;

    /**
     * A {@link NodeConnector} instance which specifies the switch port.
     */
    private final NodeConnector  port;

    /**
     * A reference to the virtual mapping which reserves a switch port.
     */
    private final MapReference  otherMap;

    /**
     * Construct a new instance.
     *
     * @param ref    A reference to a MAC mapping.
     * @param mvlan  A {@link MacVlan} instance which specifies the host.
     * @param nc     A {@link NodeConnector} instance corresponding to
     *               a switch port.
     * @param other  A reference to a virtual mapping whieh reserves the
     *               specified switch port.
     */
    public MacMapPortBusyLog(MapReference ref, MacVlan mvlan,
                             NodeConnector nc, MapReference other) {
        super(ref);
        host = mvlan;
        port = nc;
        otherMap = other;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected String getMessage() {
        StringBuilder builder =
            new StringBuilder("MAC mapping is unavailable: " +
                              "switch port is reserved: host={");
        host.appendContents(builder);
        builder.append("}, port=").append(port.toString()).
            append(",map=").append(otherMap.toString());

        return builder.toString();
    }
}
