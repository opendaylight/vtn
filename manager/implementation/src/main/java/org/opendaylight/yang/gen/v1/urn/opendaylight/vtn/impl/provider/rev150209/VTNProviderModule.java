/*
 * Copyright (c) 2015, 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.provider.rev150209;

import static org.opendaylight.controller.config.api.osgi.WaitingServiceTracker.FIVE_MINUTES;

import org.osgi.framework.BundleContext;

import org.opendaylight.vtn.manager.VTNManager;

import org.opendaylight.controller.config.api.DependencyResolver;
import org.opendaylight.controller.config.api.ModuleIdentifier;
import org.opendaylight.controller.config.api.osgi.WaitingServiceTracker;
import org.opendaylight.controller.sal.common.util.NoopAutoCloseable;

/**
 * Config subsystem module for the VTN Manager service.
 */
public final class VTNProviderModule extends AbstractVTNProviderModule {
    /**
     * OSGi bundle context.
     */
    private BundleContext  bundleContext;

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
     * Set the bundle context for the OSGi bundle that contains this class.
     *
     * @param bc  The OSGi bundle context.
     */
    void setBundleContext(BundleContext bc) {
        bundleContext = bc;
    }

    /**
     * Instantiate VTN Manager service.
     *
     * <p>
     *   VTN Manager instance is created via OSGi blueprint.
     *   This method only waits for VTN Manager instance to be created.
     * </p>
     *
     * @return  An {@link AutoCloseable} instance.
     */
    @Override
    public AutoCloseable createInstance() {
        try (WaitingServiceTracker<VTNManager> tracker =
             WaitingServiceTracker.create(VTNManager.class, bundleContext)) {
            tracker.waitForService(FIVE_MINUTES);
        }

        return NoopAutoCloseable.INSTANCE;
    }
}
