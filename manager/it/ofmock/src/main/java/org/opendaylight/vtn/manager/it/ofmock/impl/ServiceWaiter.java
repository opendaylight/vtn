/*
 * Copyright (c) 2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.ofmock.impl;

import java.util.Collection;
import java.util.Iterator;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import org.osgi.framework.BundleContext;
import org.osgi.framework.Constants;
import org.osgi.framework.ServiceEvent;
import org.osgi.framework.ServiceListener;
import org.osgi.framework.ServiceReference;

/**
 * {@code ServiceWaiter} is a utility to wait for the specified OSGi service
 * to be registered.
 *
 * @param <T>  The type of OSGi service.
 */
public final class ServiceWaiter<T> implements ServiceListener {
    /**
     * Logger instance.
     */
    private static final Logger  LOG =
        LoggerFactory.getLogger(ServiceWaiter.class);

    /**
     * The name of the OSGi service property which specifies the name of the
     * container.
     */
    static final String  PROP_CONTAINER = "containerName";

    /**
     * An OSGi bundle context for this OSGi bundle.
     */
    private final BundleContext  bundleContext;

    /**
     * A class that specifies the type of OSGi service.
     */
    private final Class<T>  serviceType;

    /**
     * OSGi service filter for retrieving.
     */
    private final String  serviceFilter;

    /**
     * OSGI service implementation to be returned.
     */
    private T  serviceInstance;

    /**
     * Construct a new instance that waits for the global OSGi service to be
     * registered.
     *
     * @param bc    OSGi bundle context.
     * @param type  A class that specifies the type of OSGi service.
     */
    public ServiceWaiter(BundleContext bc, Class<T> type) {
        this(bc, type, null);
    }

    /**
     * Construct a new instance that waits for the OSGi service in the given
     * container to be registered.
     *
     * @param bc     OSGi bundle context.
     * @param type   A class that specifies the type of OSGi service.
     * @param cname  The name of the container. {@code null} means the global
     *               OSGi service.
     */
    public ServiceWaiter(BundleContext bc, Class<T> type, String cname) {
        bundleContext = bc;
        serviceType = type;

        // Construct a OSGi service filter.
        String objFilter = new StringBuilder("(").
            append(Constants.OBJECTCLASS).append('=').append(type.getName()).
            append(')').toString();
        if (cname == null) {
            serviceFilter = objFilter;
        } else {
            StringBuilder builder = new StringBuilder("(&").
                append(objFilter).append('(').append(PROP_CONTAINER).
                append('=').append(cname).append("))");
            serviceFilter = builder.toString();
        }

        // Try to get service instance.
        getService();
    }

    /**
     * Wait for the target OSGi service to be registered.
     *
     * @return  An implementation of the target OSGi service.
     * @throws InterruptedException
     *    The calling thread was interrupted.
     */
    public T await() throws InterruptedException {
        synchronized (this) {
            if (serviceInstance != null) {
                return serviceInstance;
            }
        }

        try {
            bundleContext.addServiceListener(this, serviceFilter);
        } catch (Exception e) {
            throw new IllegalStateException(
                "Failed to add OSGi service listener.", e);
        }

        try {
            // We need to check the service again in order to avoid race
            // condition with service event.
            T impl = getService();
            if (impl == null) {
                impl = awaitImpl();
            }

            return impl;
        } finally {
            bundleContext.removeServiceListener(this);
        }
    }

    /**
     * Wait for the target OSGi service to be registered.
     *
     * @return  An implementation of the target OSGi service.
     * @throws InterruptedException
     *    The calling thread was interrupted.
     */
    private synchronized T awaitImpl() throws InterruptedException {
        LOG.trace("Waiting for {} service to be registered.",
                  serviceType.getSimpleName());

        long timeout = OfMockProvider.TASK_TIMEOUT;
        long deadline = System.currentTimeMillis() + timeout;
        do {
            wait(timeout);
            if (serviceInstance != null) {
                return serviceInstance;
            }
            timeout = deadline - System.currentTimeMillis();
        } while (timeout > 0);

        String msg = serviceType.getSimpleName() +
            " service was not registered.";
        throw new IllegalStateException(msg);
    }

    /**
     * Search for the target OSGi service implementation.
     *
     * @return  An implementation of the target OSGi service on success.
     *          {@code null} on failure.
     */
    private T getService() {
        synchronized (this) {
            if (serviceInstance != null) {
                return serviceInstance;
            }
        }

        Collection<ServiceReference<T>> c;
        try {
            c = bundleContext.getServiceReferences(serviceType, serviceFilter);
        } catch (Exception e) {
            String msg = "Failed to get OSGi service reference: " +
                serviceType.getSimpleName();
            throw new IllegalStateException(msg, e);
        }

        Iterator<ServiceReference<T>> it = c.iterator();
        if (it.hasNext()) {
            ServiceReference<T> ref = it.next();
            try {
                T impl = bundleContext.getService(ref);
                if (impl != null) {
                    setInstance(impl);
                }
                return impl;
            } catch (RuntimeException e) {
                invalidReference(ref, e);
            }
        }

        return null;
    }

    /**
     * Record a warning level log that indicates the given service reference
     * is invalid.
     *
     * @param ref  A {@link ServiceReference} instance.
     * @param e    The cause of error.
     */
    private void invalidReference(ServiceReference<?> ref, Exception e) {
        String msg = "Failed to get OSGi service: " + ref;
        LOG.warn(msg, e);
    }

    /**
     * Set an implementation of the target OSGi service.
     *
     * @param impl  An implementation of the target OSGi service.
     */
    private synchronized void setInstance(T impl) {
        if (serviceInstance == null) {
            LOG.trace("{} service has been registered: {}",
                      serviceType.getSimpleName(), impl);
            serviceInstance = impl;
            notifyAll();
        }
    }

    // ServiceListener

    /**
     * Invoked when the OSGi service has been changed.
     *
     * @param event  A {@link ServiceEvent} instance.
     */
    @Override
    public void serviceChanged(ServiceEvent event) {
        LOG.trace("OSGi service event: {}", event);
        if (event.getType() != ServiceEvent.REGISTERED) {
            return;
        }

        // Try to get OSGi service implementation.
        ServiceReference<?> ref = event.getServiceReference();
        Object impl;
        try {
            impl = bundleContext.getService(ref);
        } catch (RuntimeException e) {
            invalidReference(ref, e);
            return;
        }

        if (serviceType.isInstance(impl)) {
            setInstance(serviceType.cast(impl));
        }
    }
}
