/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util;

import org.opendaylight.controller.md.sal.binding.api.NotificationService;

import org.opendaylight.yangtools.yang.binding.NotificationListener;

/**
 * Abstract base class for MD-SAL notification listener.
 */
public abstract class SalNotificationListener extends CloseableContainer
    implements NotificationListener {
    /**
     * Register this instance as a SAL notification listener.
     *
     * @param nsv  A {@link NotificationService} service instance.
     */
    protected final void registerListener(NotificationService nsv) {
        try {
            addCloseable(nsv.registerNotificationListener(this));
        } catch (Exception e) {
            String msg = "Failed to register notification listener: " +
                getClass().getName();
            getLogger().error(msg, e);
            throw new IllegalStateException(msg, e);
        }
    }
}
