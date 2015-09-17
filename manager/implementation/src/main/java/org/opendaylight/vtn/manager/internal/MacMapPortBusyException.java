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
 * An exception which indicates the specified VLAN network on a switch port is
 * reserved by another virtual mapping.
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
public class MacMapPortBusyException extends MacMapException {
    /**
     * A reference to the virtual mapping which reserves the target VLAN
     * network on a switch port.
     */
    private final MapReference  anotherMapping;

    /**
     * Create a new exception which indicates the specified VLAN network on a
     * switch port is reserved by another virtual mapping.
     *
     * @param mvlan  A {@link MacVlan} instance which represents a host that
     *               failed to map.
     * @param ref    A reference to the target MAC mapping.
     * @param map    A reference to the virtual mapping which reserves the
     *               target VLAN network on a switch port.
     */
    public MacMapPortBusyException(MacVlan mvlan, MapReference ref,
                                   MapReference map) {
        super(VtnErrorTag.CONFLICT, "VLAN on a switch port is reserved",
              mvlan, ref);
        anotherMapping = map;
    }

    /**
     * Return a reference to the virtual mapping which reserves the target
     * VLAN network on a switch port.
     *
     * @return  A reference to the virtual mapping.
     */
    public MapReference getAnotherMapping() {
        return anotherMapping;
    }
}
