/*
 * Copyright (c) 2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.provider.rev150209;

import org.osgi.framework.Bundle;
import org.osgi.framework.BundleContext;
import org.osgi.framework.FrameworkUtil;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import org.opendaylight.vtn.manager.internal.provider.VTNManagerProviderImpl;

import org.opendaylight.controller.config.api.DependencyResolver;
import org.opendaylight.controller.config.api.ModuleIdentifier;

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
    }

    /**
     * Construct a new VTN Manager provider instance.
     *
     * @return  A {@link VTNManagerProviderImpl} instance.
     */
    @Override
    public AutoCloseable createInstance() {
        BundleContext bc = getBundle().getBundleContext();
        return createProvider(bc);
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
        AutoCloseable instance = old;
        if (state != Bundle.ACTIVE) {
            // The VTN Manager bundle is not active.
            // In this case we should reuse the given instance.
            LOG.trace("Reuse inactive provider: state={}, this={}, old={}",
                      state, this, old);
        } else {
            BundleContext bc = b.getBundleContext();
            VTNManagerProviderImpl oldProvider = (VTNManagerProviderImpl)old;
            if (oldProvider.canReuse()) {
                LOG.trace("Reuse provider: this={}, old={}", this, old);
            } else {
                oldProvider.close();
                instance = createProvider(bc);
                LOG.trace("Create new provider: this={}, old={}, new={}",
                          this, oldProvider, instance);
            }
        }

        return instance;
    }

    /**
     * Create a new VTN Manager provider.
     *
     * @param bc  A {@link BundleContext} instance associated with the OSGi
     *            bundle that contains this class.
     * @return  A {@link VTNManagerProviderImpl} instance.
     */
    private VTNManagerProviderImpl createProvider(BundleContext bc) {
        return new VTNManagerProviderImpl(
            bc, getDataBrokerDependency(), getRpcRegistryDependency(),
            getNotificationServiceDependency(),
            getEntityOwnershipServiceDependency());
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
