/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util;

import org.junit.Test;

import org.mockito.Mockito;

import org.slf4j.Logger;

import org.opendaylight.vtn.manager.internal.TestBase;

import org.opendaylight.controller.sal.binding.api.NotificationService;

import org.opendaylight.yangtools.concepts.ListenerRegistration;
import org.opendaylight.yangtools.yang.binding.NotificationListener;

/**
 * JUnit test for {@link SalNotificationListener}.
 */
public class SalNotificationListenerTest extends TestBase {
    /**
     * Stub notification listener for test.
     */
    private static class TestListener extends SalNotificationListener {
        /**
         * Logger instance.
         */
        private Logger  logger;

        /**
         * {@inheritDoc}
         */
        @Override
        protected Logger getLogger() {
            Logger log = logger;
            if (log == null) {
                log = Mockito.mock(Logger.class);
                logger = log;
            }
            return log;
        }
    }

    /**
     * Test case for registration and unregistration.
     *
     * <ul>
     *   <li>{@link SalNotificationListener#registerListener(NotificationService)}</li>
     *   <li>{@link SalNotificationListener#close()}</li>
     * </ul>
     */
    @Test
    public void testRegistration() {
        // Register a listener.
        TestListener listener = new TestListener();
        Logger logger = listener.getLogger();
        NotificationService nsv = Mockito.mock(NotificationService.class);
        @SuppressWarnings("unchecked")
        ListenerRegistration<NotificationListener> reg =
            Mockito.mock(ListenerRegistration.class);
        Mockito.when(nsv.registerNotificationListener(listener)).
            thenReturn(reg);
        listener.registerListener(nsv);
        Mockito.verify(nsv).registerNotificationListener(listener);
        Mockito.verify(reg, Mockito.never()).close();
        Mockito.verify(logger, Mockito.never()).
            error(Mockito.anyString(), Mockito.any(Throwable.class));
        Mockito.verify(logger, Mockito.never()).error(Mockito.anyString());

        // Unregister a listener.
        // Registration should be closed only one time.
        for (int i = 0; i < 10; i++) {
            listener.close();
            Mockito.verify(nsv).registerNotificationListener(listener);
            Mockito.verify(reg).close();
            Mockito.verify(logger, Mockito.never()).
                error(Mockito.anyString(), Mockito.any(Throwable.class));
            Mockito.verify(logger, Mockito.never()).error(Mockito.anyString());
        }
    }

    /**
     * Test case for registration failure.
     *
     * <ul>
     *   <li>{@link SalNotificationListener#registerListener(NotificationService)}</li>
     * </ul>
     */
    @Test
    public void testRegistrationError() {
        TestListener listener = new TestListener();
        Logger logger = listener.getLogger();
        NotificationService nsv = Mockito.mock(NotificationService.class);
        IllegalArgumentException iae =
            new IllegalArgumentException("Bad argument");
        Mockito.when(nsv.registerNotificationListener(listener)).
            thenThrow(iae);

        String msg = null;
        try {
            listener.registerListener(nsv);
            unexpected();
        } catch (IllegalStateException e) {
            msg = "Failed to register notification listener: " +
                listener.getClass().getName();
            assertEquals(iae, e.getCause());
            assertEquals(msg, e.getMessage());
        }

        Mockito.verify(nsv).registerNotificationListener(listener);
        Mockito.verify(logger).error(msg, iae);
        Mockito.verify(logger, Mockito.never()).error(Mockito.anyString());

        // close() should do nothing.
        for (int i = 0; i < 10; i++) {
            listener.close();
            Mockito.verify(nsv).registerNotificationListener(listener);
            Mockito.verify(logger).error(msg, iae);
            Mockito.verify(logger, Mockito.never()).error(Mockito.anyString());
        }
    }

    /**
     * Test case for close error.
     *
     * <ul>
     *   <li>{@link SalNotificationListener#close()}</li>
     * </ul>
     */
    @Test
    public void testCloseError() {
        // Register a listener.
        TestListener listener = new TestListener();
        Logger logger = listener.getLogger();
        NotificationService nsv = Mockito.mock(NotificationService.class);
        @SuppressWarnings("unchecked")
        ListenerRegistration<NotificationListener> reg =
            Mockito.mock(ListenerRegistration.class);
        Mockito.when(nsv.registerNotificationListener(listener)).
            thenReturn(reg);
        listener.registerListener(nsv);
        Mockito.verify(nsv).registerNotificationListener(listener);
        Mockito.verify(reg, Mockito.never()).close();
        Mockito.verify(logger, Mockito.never()).
            error(Mockito.anyString(), Mockito.any(Throwable.class));
        Mockito.verify(logger, Mockito.never()).error(Mockito.anyString());

        // Unregister a listener.
        String msg = "Failed to close instance: " + reg;
        IllegalArgumentException iae =
            new IllegalArgumentException("Bad argument");
        Mockito.doThrow(iae).when(reg).close();

        for (int i = 0; i < 10; i++) {
            listener.close();
            Mockito.verify(nsv).registerNotificationListener(listener);
            Mockito.verify(reg).close();
            Mockito.verify(logger).error(msg, iae);
            Mockito.verify(logger, Mockito.never()).error(Mockito.anyString());
        }
    }
}
