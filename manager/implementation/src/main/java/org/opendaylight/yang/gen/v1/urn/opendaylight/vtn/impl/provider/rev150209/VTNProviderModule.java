/*
 * Copyright (c) 2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.provider.rev150209;

import java.util.Dictionary;
import java.util.Hashtable;

import org.osgi.framework.Bundle;
import org.osgi.framework.BundleContext;
import org.osgi.framework.FrameworkUtil;
import org.osgi.framework.ServiceReference;
import org.osgi.framework.ServiceRegistration;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import org.opendaylight.vtn.manager.internal.VTNManagerProvider;
import org.opendaylight.vtn.manager.internal.provider.VTNManagerProviderImpl;

import org.opendaylight.controller.config.api.DependencyResolver;
import org.opendaylight.controller.config.api.ModuleIdentifier;

import org.opendaylight.controller.sal.utils.GlobalConstants;

/**
 * MD-SAL provider module for the VTN Manager service.
 */
public final class VTNProviderModule extends AbstractVTNProviderModule {
    /**
     * Logger instance.
     */
    private static final Logger  LOG =
        LoggerFactory.getLogger(VTNProviderModule.class);

    /**
     * OSGi service registration.
     */
    private ServiceRegistration<VTNManagerProvider>  osgiRegistration;

    /**
     * Construct a VTN Manager provider module.
     *
     * @param identifier
     *    Module identifier for the VTN Manager provider.
     * @param dependencyResolver
     *    Dependency resolver.
     */
    public VTNProviderModule(ModuleIdentifier identifier,
                             DependencyResolver dependencyResolver) {
        super(identifier, dependencyResolver);
    }

    /**
     * Construct a VTN Manager provider module.
     *
     * @param identifier
     *    Module identifier for the VTN Manager provider.
     * @param dependencyResolver
     *    Dependency resolver.
     * @param oldModule
     *    Old {@link VTNProviderModule} instance.
     * @param oldInstance
     *    Old provider instance.
     */
    public VTNProviderModule(ModuleIdentifier identifier,
                             DependencyResolver dependencyResolver,
                             VTNProviderModule oldModule,
                             AutoCloseable oldInstance) {
        super(identifier, dependencyResolver, oldModule, oldInstance);
        osgiRegistration = oldModule.osgiRegistration;
    }

    /**
     * Construct a new VTN Manager provider instance.
     *
     * @return  A {@link VTNManagerProviderImpl} instance.
     */
    @Override
    public AutoCloseable createInstance() {
        VTNManagerProviderImpl provider = createProvider();
        registerService(getBundle(), provider);
        return provider;
    }

    /**
     * Reuse the given VTN Manager provider.
     *
     * @param old  VTN Manager provider to be reused.
     * @return  An {@link AutoCloseable} instance associated with a reused
     *          provider module.
     */
    @Override
    public AutoCloseable reuseInstance(AutoCloseable old) {
        if (!(old instanceof VTNManagerProviderImpl)) {
            throw new IllegalStateException("Unexpected provider: " + old);
        }

        // Check the bundle state.
        Bundle b = getBundle();
        int state = b.getState();
        if (state != Bundle.ACTIVE) {
            // The VTN Manager bundle is not active.
            // In this case we should reuse the given instance without
            // registering any service.
            LOG.trace("Reuse inactive provider: state={}, this={}, old={}",
                      state, this, old);
            return old;
        }

        VTNManagerProviderImpl oldProvider = (VTNManagerProviderImpl)old;
        VTNManagerProviderImpl provider;
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
     * Create a new VTN Manager provider.
     *
     * @return  A {@link VTNManagerProviderImpl} instance.
     */
    private VTNManagerProviderImpl createProvider() {
        return new VTNManagerProviderImpl(
            getDataBrokerDependency(), getRpcRegistryDependency(),
            getNotificationServiceDependency());
    }

    /**
     * Register VTN Manager provider to the OSGi service registry.
     *
     * @param bundle    A {@link Bundle} instance.
     * @param provider  A {@link VTNManagerProviderImpl} instance.
     */
    private void registerService(Bundle bundle,
                                 VTNManagerProviderImpl provider) {
        BundleContext bc = bundle.getBundleContext();
        if (isOsgiServiceRequired(bc, provider)) {
            Dictionary<String, Object> prop = new Hashtable<>();
            prop.put("containerName", GlobalConstants.DEFAULT.toString());
            try {
                osgiRegistration = bc.
                    registerService(VTNManagerProvider.class, provider, prop);
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
     * Determine whether the given VTN Manager provider needs to be registered
     * to the OSGi service registry or not.
     *
     * @param bc        A {@link BundleContext} instance.
     * @param provider  A {@link VTNManagerProviderImpl} instance.
     * @return  {@code true} only if the given VTN Manager provider needs to
     *          be registered as an OSGi service.
     */
    private boolean isOsgiServiceRequired(BundleContext bc,
                                          VTNManagerProviderImpl provider) {
        if (osgiRegistration == null) {
            LOG.trace("No OSGi service: this={}", this);
            return true;
        }

        try {
            ServiceReference<VTNManagerProvider> ref =
                osgiRegistration.getReference();
            if (provider.equals(bc.getService(ref))) {
                LOG.trace("Reuse OSGI service: provider={}, reg={}",
                          provider, osgiRegistration);
                return false;
            }
        } catch (Exception e) {
            LOG.trace("OSGi service registration is obsolete", e);
            osgiRegistration = null;
            return true;
        }

        // Unregister old service.
        try {
            osgiRegistration.unregister();
        } catch (Exception e) {
            LOG.trace("Failed to unregister OSGi service", e);
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
        Bundle b = FrameworkUtil.getBundle(VTNProviderModule.class);
        if (b == null) {
            // This should never happen.
            throw new IllegalStateException(
                "Failed to get OSGi bundle context");
        }

        return b;
    }
}
