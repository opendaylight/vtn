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

import org.opendaylight.controller.sal.utils.StatusCode;

/**
 * An exception which indicates the same MAC address as the specified host
 * is already mapped by the same MAC mapping.
 *
 * <ul>
 *   <li>
 *     {@link #getHost()} returns a host information that failed to map.
 *   </li>
 *   <li>
 *     {@link #getMapReference()} returns a reference to the target
 *     MAC mapping.
 *   </li>
 * </ul>
 */
public class MacMapDuplicateException extends MacMapException {
    /**
     * A {@link MacVlan} instance which specifies the host already mapped by
     * the MAC mapping.
     */
    private final MacVlan  duplicate;

    /**
     * Create a new exception which indicates duplicate MAC address was
     * detecred.
     *
     * @param mvlan  A {@link MacVlan} instance which represents a host that
     *               failed to map.
     * @param ref    A reference to the target MAC mapping.
     * @param dup    A {@link MacVlan} instance which represents a duplicate
     *               host.
     */
    public MacMapDuplicateException(MacVlan mvlan, MapReference ref,
                                    MacVlan dup) {
        super(StatusCode.CONFLICT, "Duplicate MAC address found",
              mvlan, ref);
        duplicate = dup;
    }

    /**
     * Return a {@link MacVlan} instance which specifies the host already
     * mapped by the MAC mapping.
     *
     * @return  A {@link MacVlan} instance.
     */
    public MacVlan getDuplicate() {
        return duplicate;
    }
}
