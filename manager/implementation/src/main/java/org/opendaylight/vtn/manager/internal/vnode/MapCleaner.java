/*
 * Copyright (c) 2014, 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.vnode;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.TxContext;

/**
 * {@code MapCleaner} provides interfaces to be implemented by classes
 * which purge cached network resources created by virtual network mapping.
 */
public interface MapCleaner {
    /**
     * Purge caches for the virtual network.
     *
     * @param ctx    A runtime context for transaction task.
     * @param tname  The name of the target VTN.
     * @throws VTNException  An error occurred.
     */
    void purge(TxContext ctx, String tname) throws VTNException;
}
