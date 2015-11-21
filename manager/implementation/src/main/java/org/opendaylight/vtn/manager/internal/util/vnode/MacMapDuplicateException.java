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
 * An exception which indicates the same MAC address as the specified host
 * is already mapped by the same MAC mapping.
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
public final class MacMapDuplicateException extends MacMapException {
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
     * @param ident  The identifier for the MAC mapping which maps the host
     *               specified by {@code mvlan}.
     * @param dup    A {@link MacVlan} instance which represents a duplicate
     *               host.
     */
    public MacMapDuplicateException(MacVlan mvlan, MacMapIdentifier ident,
                                    MacVlan dup) {
        super(RpcErrorTag.DATA_EXISTS, VtnErrorTag.CONFLICT,
              "Duplicate MAC address found", mvlan, ident);
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

    // MacMapException

    /**
     * {@inheritDoc}
     */
    @Override
    public String getErrorMessage() {
        return "MAC address conflicts with " + duplicate;
    }
}
