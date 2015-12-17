/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.it.ofmock.provider.rev150209;

import org.osgi.framework.Bundle;
import org.osgi.framework.BundleContext;
import org.osgi.framework.FrameworkUtil;
import org.osgi.framework.ServiceReference;
import org.osgi.framework.ServiceRegistration;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import org.opendaylight.vtn.manager.it.ofmock.OfMockService;
import org.opendaylight.vtn.manager.it.ofmock.impl.OfMockProvider;

import org.opendaylight.controller.config.api.ModuleIdentifier;
import org.opendaylight.controller.config.api.DependencyResolver;

/**
 * MD-SAL provider module for the openflowplugin mock-up.
 */
public class OfMockProviderModule extends AbstractOfMockProviderModule {
    /**
     * Logger instance.
     */
    private static final Logger  LOG =
        LoggerFactory.getLogger(OfMockProviderModule.class);

    /**
     * OSGi service registration.
     */
    private ServiceRegistration<OfMockService>  osgiRegistration;

    /**
     * Construct an openflowplugin mock-up provider module.
     *
     * @param identifier
     *    Module identifier for the openflowplugin mock-up provider.
     * @param dependencyResolver
     *    Dependency resolver.
     */
    public OfMockProviderModule(ModuleIdentifier identifier,
                                DependencyResolver dependencyResolver) {
        super(identifier, dependencyResolver);
    }

    /**
     * Construct an openflowplugin mock-up provider module.
     *
     * @param identifier
     *    Module identifier for the openflowplugin mock-up provider.
     * @param dependencyResolver
     *    Dependency resolver.
     * @param oldModule
     *    Old {@link OfMockProviderModule} instance.
     * @param oldInstance
     *    Old provider instance.
     */
    public OfMockProviderModule(ModuleIdentifier identifier,
                                DependencyResolver dependencyResolver,
                                OfMockProviderModule oldModule,
                                AutoCloseable oldInstance) {
        super(identifier, dependencyResolver, oldModule, oldInstance);
        osgiRegistration = oldModule.osgiRegistration;
    }

    /**
     * Construct a new openflowplugin mock-up provider instance.
     *
     * @return  An {@link OfMockProvider} instance.
     */
    @Override
    public AutoCloseable createInstance() {
        OfMockProvider provider = createProvider();
        registerService(getBundle(), provider);
        return provider;
    }

    /**
     * Reuse the given provider.
     *
     * @param old  The openflowplugin mock-up provider to be reused.
     * @return  An {@link AutoCloseable} instance associated with a reused
     *          provider module.
     */
    @Override
    public AutoCloseable reuseInstance(AutoCloseable old) {
        if (!(old instanceof OfMockProvider)) {
            throw new IllegalStateException("Unexpected provider: " + old);
        }

        // Check the bundle state.
        Bundle b = getBundle();
        int state = b.getState();
        if (state != Bundle.ACTIVE) {
            LOG.trace("Reuse inactive provider: state={}, this={}, old={}",
                      state, this, old);
            return old;
        }

        OfMockProvider oldProvider = (OfMockProvider)old;
        OfMockProvider provider;
        if (oldProvider.canReuse()) {
            LOG.trace("Reuse provider: this={}, old={}", this, old);
            provider = oldProvider;
        } else {
            oldProvider.close();
            provider = createProvider();
            LOG.trace("Create new provider: this={}, old={}, new={}",
                      this, oldProvider, provider);
        }

        registerService(b, provider);
        return provider;
    }

    /**
     * Create a new openflowplugin mock-up provider.
     *
     * @return  An {@link OfMockProvider} instance.
     */
    private OfMockProvider createProvider() {
        return new OfMockProvider(
            getDataBrokerDependency(), getRpcRegistryDependency(),
            getNotificationServiceDependency(),
            getNotificationPublishServiceDependency());
    }

    /**
     * Retister the openflowplugin mock-up provider to the OSGi service
     * registry.
     *
     * @param bundle    A {@link Bundle} instance.
     * @param provider  An {@link OfMockProvider} instance.
     */
    private void registerService(Bundle bundle, OfMockProvider provider) {
        BundleContext bc = bundle.getBundleContext();
        if (isOsgiServiceRequired(bc, provider)) {
            try {
                osgiRegistration =
                    bc.registerService(OfMockService.class, provider, null);
            } catch (RuntimeException e) {
                String msg = "Failed to register OSGi service.";
                LOG.error(msg, e);
                throw new IllegalStateException(msg, e);
            }

            LOG.trace("Registered OSGi service: provider={}, reg={}",
                      provider, osgiRegistration);
        }
    }

    /**
     * Determine whether the given openflowplugin mock-up provider needs to be
     * registered to the OSGi service registry or not.
     *
     * @param bc        A {@link BundleContext} instance.
     * @param provider  An {@link OfMockProvider} instance.
     * @return  {@code true} only if the given provider needs to be registered
     *          as an OSGi service.
     */
    private boolean isOsgiServiceRequired(BundleContext bc,
                                          OfMockProvider provider) {
        if (osgiRegistration == null) {
            LOG.trace("No OSGi service: this={}", this);
            return true;
        }

        try {
            ServiceReference<OfMockService> ref =
                osgiRegistration.getReference();
            if (provider.equals(bc.getService(ref))) {
                LOG.trace("Reuse OSGI service: provider={}, reg={}",
                          provider, osgiRegistration);
                return false;
            }
        } catch (Exception e) {
            LOG.trace("OSGi service registration is obsolete.", e);
            osgiRegistration = null;
            return true;
        }

        // Unregister old service.
        try {
            osgiRegistration.unregister();
        } catch (Exception e) {
            LOG.trace("Failed to unregister OSGi service.", e);
        }

        osgiRegistration = null;
        return false;
    }

    /**
     * Return the OSGi bundle.
     *
     * @return  The OSGi bundle.
     */
    private Bundle getBundle() {
        Bundle b = FrameworkUtil.getBundle(OfMockProviderModule.class);
        if (b == null) {
            // This should never happen.
            throw new IllegalStateException(
                "Failed to get OSGi bundle context");
        }

        return b;
    }
}
