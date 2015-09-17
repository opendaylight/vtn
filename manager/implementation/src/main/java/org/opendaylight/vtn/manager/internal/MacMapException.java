/*
 * Copyright (c) 2014, 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal;

import org.opendaylight.vtn.manager.VTNException;
import org.opendaylight.vtn.manager.internal.cluster.MacVlan;
import org.opendaylight.vtn.manager.internal.cluster.MapReference;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnErrorTag;

/**
 * A base class of exception which indicates invalid state of the MAC mapping.
 */
public abstract class MacMapException extends VTNException {
    /**
     * A {@link MacVlan} instance which specifies the host to be mapped by
     * MAC mapping.
     */
    private final MacVlan  host;

    /**
     * A {@link MapReference} instance which points to the target MAC mapping.
     */
    private final MapReference  reference;

    /**
     * Construct a new instance.
     *
     * @param etag   A {@link VtnErrorTag} for this exception.
     * @param msg    An error message.
     * @param mvlan  A {@link MacVlan} instance corresponding to the host
     *               to be mapped by MAC mapping.
     * @param ref    A reference to the MAC mapping which maps the host
     *               specified by {@code mvlan}.
     */
    protected MacMapException(VtnErrorTag etag, String msg, MacVlan mvlan,
                              MapReference ref) {
        super(etag, msg);
        host = mvlan;
        reference = ref;
    }

    /**
     * Return a {@link MacVlan} instance which specifies the host to be
     * mapped by the target MAC mapping.
     *
     * @return  A {@link MacVlan} instance.
     */
    public MacVlan getHost() {
        return host;
    }

    /**
     * Return a reference to the target MAC mapping.
     *
     * @return  A {@link MapReference} which points the MAC mapping.
     */
    public MapReference getMapReference() {
        return reference;
    }
}
