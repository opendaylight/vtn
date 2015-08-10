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

/**
 * An instance of {@code MacMapDuplicateLog} class represents a trace log
 * message which indicates that the MAC mapping could not established because
 * the same MAC address on another VLAN network is mapped to the same vBridge.
 */
public final class MacMapDuplicateLog extends MapLog {
    /**
     * A {@link MacVlan} instance which specifies a L2 host that failed to map.
     */
    private final MacVlan  host;

    /**
     * A {@link MacVlan} instance which specifies a duplicate L2 host.
     */
    private final MacVlan  duplicate;

    /**
     * Construct a new instance.
     *
     * @param ref    A reference to a MAC mapping.
     * @param mvlan  A {@link MacVlan} instance which specifies the host.
     * @param dup    A {@link MacVlan} instance which specifies the host
     *               which has the same MAC address as {@code mvlan}.
     */
    public MacMapDuplicateLog(MapReference ref, MacVlan mvlan, MacVlan dup) {
        super(ref);
        host = mvlan;
        duplicate = dup;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected String getMessage() {
        StringBuilder builder =
            new StringBuilder("MAC mapping is unavailable: " +
                              "Same MAC address is already mapped: host={");
        host.appendContents(builder);
        builder.append("}, duplicate={");
        duplicate.appendContents(builder);
        builder.append('}');

        return builder.toString();
    }
}
