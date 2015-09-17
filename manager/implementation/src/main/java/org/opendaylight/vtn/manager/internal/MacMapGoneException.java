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

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnErrorTag;

/**
 * An exception which indicates the specified host is no longer mapped by the
 * target MAC mapping.
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
public class MacMapGoneException extends MacMapException {
    /**
     * Create a new exception which indicates the specified host is no longer
     * mapped by MAC mapping.
     *
     * @param mvlan  A {@link MacVlan} instance which represents a host that
     *               failed to map.
     * @param ref    A reference to the target MAC mapping.
     */
    public MacMapGoneException(MacVlan mvlan, MapReference ref) {
        super(VtnErrorTag.GONE, "No longer mapped by MAC mapping",
              mvlan, ref);
    }
}
