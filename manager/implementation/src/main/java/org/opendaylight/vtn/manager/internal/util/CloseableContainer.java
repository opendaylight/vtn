/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util;

import org.slf4j.Logger;

/**
 * {@code CloseableContainer} describes a container that contains one or
 * more {@link AutoCloseable} instances.
 */
public abstract class CloseableContainer implements AutoCloseable {
    /**
     * A closeable objects to be closed on a {@link #close()} call.
     */
    private final CompositeAutoCloseable  closeables;

    /**
     * Construct a new instance.
     */
    protected CloseableContainer() {
        closeables = new CompositeAutoCloseable(getLogger());
    }

    /**
     * Add the given closeable object to the closeable object set.
     *
     * <p>
     *   The given object will be closed on the first {@link #close()} call.
     * </p>
     *
     * @param ac  An {@link AutoCloseable} instance.
     */
    protected final void addCloseable(AutoCloseable ac) {
        closeables.add(ac);
    }

    /**
     * Return a logger instance.
     *
     * <p>
     *   Note that this method will be called before the constructor returns.
     * </p>
     *
     * @return  A {@link Logger} instance.
     */
    protected abstract Logger getLogger();

    // AutoCloseable

    /**
     * Close all the closeables in this instance.
     */
    @Override
    public void close() {
        closeables.close();
    }
}
