/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util;

import java.util.Deque;
import java.util.LinkedHashSet;
import java.util.LinkedList;
import java.util.Set;
import java.util.concurrent.atomic.AtomicReference;

import org.slf4j.Logger;

/**
 * {@code CompositeAutoCloseable} describes a set of {@link AutoCloseable}.
 */
public final class CompositeAutoCloseable implements AutoCloseable {
    /**
     * A logger instance.
     */
    private final Logger  logger;

    /**
     * A set of {@link AutoCloseable} instances.
     */
    private final AtomicReference<Set<AutoCloseable>>  closeables =
        new AtomicReference<>();

    /**
     * Construct a new instance.
     *
     * @param log  A logger instance used to record error logs.
     */
    public CompositeAutoCloseable(Logger log) {
        logger = log;
        closeables.set(new LinkedHashSet<AutoCloseable>());
    }

    /**
     * Add the given {@link AutoCloseable} instance.
     *
     * <p>
     *   Note that the given {@link AutoCloseable} will be closed if this
     *   instance is already closed.
     * </p>
     *
     * @param ac  An {@link AutoCloseable} instance.
     */
    public void add(AutoCloseable ac) {
        Set<AutoCloseable> set = closeables.get();
        if (set == null) {
            logger.warn("Already closed: {}", ac);
            close(ac);
        } else {
            set.add(ac);
        }
    }

    /**
     * Determine whether this instance is already closed or not.
     *
     * @return  {@code true} only if this instance is already closed.
     */
    public boolean isClosed() {
        return (closeables.get() == null);
    }

    /**
     * Close the given closeable.
     *
     * @param ac  An {@link AutoCloseable} to be closed.
     */
    private void close(AutoCloseable ac) {
        try {
            ac.close();
        } catch (Exception e) {
            logger.error("Failed to close instance: " + ac, e);
        }
    }

    // AutoCloseable

    /**
     * Close all closeables contained by this instance.
     */
    @Override
    public void close() {
        Set<AutoCloseable> set = closeables.getAndSet(null);
        if (set != null) {
            // Close all closeables in reverse order.
            Deque<AutoCloseable> list = new LinkedList<>(set);
            while (!list.isEmpty()) {
                close(list.removeLast());
            }

            // For gc.
            set.clear();
        }
    }
}
