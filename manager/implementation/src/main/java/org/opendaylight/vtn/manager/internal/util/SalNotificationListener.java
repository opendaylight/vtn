/*
 * Copyright (c) 2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util;

import java.util.concurrent.atomic.AtomicReference;

import org.slf4j.Logger;

import org.opendaylight.controller.sal.binding.api.NotificationService;

import org.opendaylight.yangtools.concepts.ListenerRegistration;
import org.opendaylight.yangtools.yang.binding.NotificationListener;

/**
 * Abstract base class for MD-SAL notification listener.
 */
public abstract class SalNotificationListener
    implements AutoCloseable, NotificationListener {
    /**
     * Registration of the notification listener.
     */
    private final AtomicReference<ListenerRegistration<NotificationListener>> registration =
        new AtomicReference<>();

    /**
     * Construct a new instance.
     */
    protected SalNotificationListener() {
    }

    /**
     * Register this instance as a SAL notification listener.
     *
     * @param nsv  A {@link NotificationService} service instance.
     */
    protected final void registerListener(NotificationService nsv) {
        try {
            registration.set(nsv.registerNotificationListener(this));
        } catch (Exception e) {
            String msg = "Failed to register notification listener: " +
                getClass().getName();
            getLogger().error(msg, e);
            throw new IllegalStateException(msg, e);
        }
    }

    /**
     * Return a logger instance.
     *
     * @return  A {@link Logger} instance.
     */
    protected abstract Logger getLogger();

    // AutoCloseable

    /**
     * Close this listener.
     */
    @Override
    public void close() {
        ListenerRegistration<NotificationListener> reg =
            registration.getAndSet(null);
        if (reg != null) {
            try {
                reg.close();
            } catch (Exception e) {
                String msg = "Failed to unregister notification listener: " +
                    getClass().getName();
                getLogger().error(msg, e);
            }
        }
    }
}
