/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.vnode;

import java.util.HashMap;
import java.util.Map;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.TxContext;

/**
 * {@code VirtualMapRegistry} describes a base context for registering virtual
 * network mapping.
 *
 * @param <C>  The type of {@link MapCleaner} that purges network caches.
 */
public abstract class VirtualMapRegistry<C extends MapCleaner> {
    /**
     * Context for MD-SAL datastore transaction.
     */
    private final TxContext  context;

    /**
     * A map which keeps {@link MapCleaner} instances used to purge network
     * caches.
     */
    private final Map<String, C> mapCleaners = new HashMap<>();

    /**
     * Construct a new instance.
     *
     * @param ctx  A runtime context for transaction task.
     */
    protected VirtualMapRegistry(TxContext ctx) {
        context = ctx;
    }

    /**
     * Return the runtime context for the MD-SAL datastore transaction.
     *
     * @return  A {@link TxContext} instance.
     */
    public final TxContext getContext() {
        return context;
    }

    /**
     * Clean up the transaction for changing virtual network mapping.
     *
     * @throws VTNException  An error occurred.
     */
    protected final void cleanUp() throws VTNException {
        cleanUpImpl(context);

        // Purge network caches.
        for (Map.Entry<String, C> entry: mapCleaners.entrySet()) {
            String tname = entry.getKey();
            C cleaner = entry.getValue();
            cleaner.purge(context, tname);
        }
    }

    /**
     * Return a {@link MapCleaner} instance associated with the specified VTN.
     *
     * @param name  The name of the target VTN.
     * @return  A {@link MapCleaner} instance if found.
     *            {@code null} is returned if not found.
     */
    protected final C getMapCleaner(String name) {
        return mapCleaners.get(name);
    }

    /**
     * Associate the specified {@link MapCleaner} instance with the specified
     * VTN name.
     *
     * @param tname  The name of the VTN.
     * @param cl     A {@link MapCleaner} instance.
     */
    protected final void addMapCleaner(String tname, C cl) {
        mapCleaners.put(tname, cl);
    }

    /**
     * Clean up the transaction for changing virtual network mapping.
     *
     * @param ctx  A runtime context for transaction task.
     * @throws VTNException  An error occurred.
     */
    protected abstract void cleanUpImpl(TxContext ctx) throws VTNException;
}
