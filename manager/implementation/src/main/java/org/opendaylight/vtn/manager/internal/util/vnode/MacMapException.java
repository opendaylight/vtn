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
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnErrorTag;

/**
 * A base class of exception which indicates invalid state of the MAC mapping.
 */
public abstract class MacMapException extends RpcException {
    /**
     * A {@link MacVlan} instance which specifies the host to be mapped by
     * MAC mapping.
     */
    private final MacVlan  host;

    /**
     * The identifier for the target MAC mapping.
     */
    private final MacMapIdentifier  identifier;

    /**
     * Construct a new instance.
     *
     * @param tag    A {@link RpcErrorTag} instance.
     * @param etag   A {@link VtnErrorTag} for this exception.
     * @param msg    An error message.
     * @param mvlan  A {@link MacVlan} instance corresponding to the host
     *               to be mapped by MAC mapping.
     * @param ident  The identifier for the MAC mapping which maps the host
     *               specified by {@code mvlan}.
     */
    protected MacMapException(RpcErrorTag tag, VtnErrorTag etag, String msg,
                              MacVlan mvlan, MacMapIdentifier ident) {
        super(tag, etag, msg);
        host = mvlan;
        identifier = ident;
    }

    /**
     * Return a {@link MacVlan} instance which specifies the host to be
     * mapped by the target MAC mapping.
     *
     * @return  A {@link MacVlan} instance.
     */
    public final MacVlan getHost() {
        return host;
    }

    /**
     * Return the identifier for the the target MAC mapping.
     *
     * @return  A {@link MacMapIdentifier} instance which specifies the target
     *          MAC mapping.
     */
    public final MacMapIdentifier getIdentifier() {
        return identifier;
    }

    /**
     * Create an error log message.
     *
     * @return  An error log message.
     */
    public abstract String getErrorMessage();
}
