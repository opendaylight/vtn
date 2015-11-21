/*
 * Copyright (c) 2014, 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.vnode;

import org.opendaylight.vtn.manager.internal.util.inventory.MacVlan;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcErrorTag;

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
 *     {@link #getIdentifier()} returns the identifier for the target
 *     MAC mapping.
 *   </li>
 * </ul>
 */
public final class MacMapPortBusyException extends MacMapException {
    /**
     * A reference to the virtual mapping which reserves the target VLAN
     * network on a switch port.
     */
    private final VNodeIdentifier<?>  anotherMapping;

    /**
     * Create a new exception which indicates the specified VLAN network on a
     * switch port is reserved by another virtual mapping.
     *
     * @param mvlan  A {@link MacVlan} instance which represents a host that
     *               failed to map.
     * @param ident  The identifier for the MAC mapping which maps the host
     *               specified by {@code mvlan}.
     * @param map    A reference to the virtual mapping which reserves the
     *               target VLAN network on a switch port.
     */
    public MacMapPortBusyException(MacVlan mvlan, MacMapIdentifier ident,
                                   VNodeIdentifier<?> map) {
        super(RpcErrorTag.IN_USE, VtnErrorTag.CONFLICT,
              "VLAN on a switch port is reserved", mvlan, ident);
        anotherMapping = map;
    }

    /**
     * Return a reference to the virtual mapping which reserves the target
     * VLAN network on a switch port.
     *
     * @return  A reference to the virtual mapping.
     */
    public VNodeIdentifier<?> getAnotherMapping() {
        return anotherMapping;
    }

    // MacMapException

    /**
     * {@inheritDoc}
     */
    @Override
    public String getErrorMessage() {
        return "Switch port is reserved by " + anotherMapping;
    }
}
