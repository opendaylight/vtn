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
 * An instance of {@code MacMapInactivatedLog} class represents a trace log
 * message which indicates the MAC mapping for the specified host has been
 * inactivated.
 */
public final class MacMapInactivatedLog extends MapLog {
    /**
     * A {@link MacVlan} instance which specifies a L2 host.
     */
    private final MacVlan  host;

    /**
     * A {@link NodeConnector} instance corresponding to a switch port.
     */
    private final NodeConnector  port;

    /**
     * Create a log message which indicates the MAC mapping has been
     * inactivated.
     *
     * @param ref    A reference to a MAC mapping.
     * @param mvlan  A {@link MacVlan} instance which specifies the host.
     * @param nc     A {@link NodeConnector} instance corresponding to
     *               a switch port.
     */
    public MacMapInactivatedLog(MapReference ref, MacVlan mvlan,
                                NodeConnector nc) {
        super(ref);
        host = mvlan;
        port = nc;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected String getMessage() {
        StringBuilder builder =
            new StringBuilder("MAC mapping has been inactivated: host={");
        host.appendContents(builder);
        builder.append("}, port=").append(port.toString());

        return builder.toString();
    }
}
