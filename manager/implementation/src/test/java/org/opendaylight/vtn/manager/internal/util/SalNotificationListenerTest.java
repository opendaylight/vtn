/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util;

import static org.mockito.Mockito.any;
import static org.mockito.Mockito.anyString;
import static org.mockito.Mockito.doThrow;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.never;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

import org.junit.Test;

import org.slf4j.Logger;

import org.opendaylight.vtn.manager.internal.TestBase;

import org.opendaylight.controller.md.sal.binding.api.NotificationService;

import org.opendaylight.yangtools.concepts.ListenerRegistration;

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
                log = mock(Logger.class);
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
        NotificationService nsv = mock(NotificationService.class);
        @SuppressWarnings("unchecked")
        ListenerRegistration<TestListener> reg =
            mock(ListenerRegistration.class);
        when(nsv.registerNotificationListener(listener)).thenReturn(reg);
        listener.registerListener(nsv);
        verify(nsv).registerNotificationListener(listener);
        verify(reg, never()).close();
        verify(logger, never()).error(anyString(), any(Throwable.class));
        verify(logger, never()).error(anyString());

        // Unregister a listener.
        // Registration should be closed only one time.
        for (int i = 0; i < 10; i++) {
            listener.close();
            verify(nsv).registerNotificationListener(listener);
            verify(reg).close();
            verify(logger, never()).error(anyString(), any(Throwable.class));
            verify(logger, never()).error(anyString());
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
        NotificationService nsv = mock(NotificationService.class);
        IllegalArgumentException iae =
            new IllegalArgumentException("Bad argument");
        when(nsv.registerNotificationListener(listener)).thenThrow(iae);

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

        verify(nsv).registerNotificationListener(listener);
        verify(logger).error(msg, iae);
        verify(logger, never()).error(anyString());

        // close() should do nothing.
        for (int i = 0; i < 10; i++) {
            listener.close();
            verify(nsv).registerNotificationListener(listener);
            verify(logger).error(msg, iae);
            verify(logger, never()).error(anyString());
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
        NotificationService nsv = mock(NotificationService.class);
        @SuppressWarnings("unchecked")
        ListenerRegistration<TestListener> reg =
            mock(ListenerRegistration.class);
        when(nsv.registerNotificationListener(listener)).
            thenReturn(reg);
        listener.registerListener(nsv);
        verify(nsv).registerNotificationListener(listener);
        verify(reg, never()).close();
        verify(logger, never()).error(anyString(), any(Throwable.class));
        verify(logger, never()).error(anyString());

        // Unregister a listener.
        String msg = "Failed to close instance: " + reg;
        IllegalArgumentException iae =
            new IllegalArgumentException("Bad argument");
        doThrow(iae).when(reg).close();

        for (int i = 0; i < 10; i++) {
            listener.close();
            verify(nsv).registerNotificationListener(listener);
            verify(reg).close();
            verify(logger).error(msg, iae);
            verify(logger, never()).error(anyString());
        }
    }
}
