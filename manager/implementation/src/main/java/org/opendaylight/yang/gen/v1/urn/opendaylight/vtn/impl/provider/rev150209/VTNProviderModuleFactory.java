/*
 * Copyright (c) 2015, 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.provider.rev150209;

import org.osgi.framework.BundleContext;

import org.opendaylight.controller.config.api.DependencyResolver;

/**
 * Factory class for VTN Manager provider module.
 */
public class VTNProviderModuleFactory extends AbstractVTNProviderModuleFactory {
    /**
     * Instantiate the VTN Manager provider module.
     *
     * @param instanceName        The name of the instance.
     * @param dependencyResolver  Dependency resolver.
     * @param oldModule           Old {@link VTNProviderModule} instance.
     * @param oldInstance         Old provider instance.
     * @param bundleContext       OSGi bundle context.
     * @return  VTN Manager provider module instance.
     */
    @Override
    public VTNProviderModule instantiateModule(
        String instanceName, DependencyResolver dependencyResolver,
        VTNProviderModule oldModule, AutoCloseable oldInstance,
        BundleContext bundleContext) {
        VTNProviderModule module = super.instantiateModule(
            instanceName, dependencyResolver, oldModule, oldInstance,
            bundleContext);
        module.setBundleContext(bundleContext);
        return module;
    }

    /**
     * Instantiate the VTN Manager provider module.
     *
     * @param instanceName        The name of the instance.
     * @param dependencyResolver  Dependency resolver.
     * @param bundleContext       OSGi bundle context.
     * @return  VTN Manager provider module instance.
     */
    @Override
    public VTNProviderModule instantiateModule(
        String instanceName, DependencyResolver dependencyResolver,
        BundleContext bundleContext) {
        VTNProviderModule module = super.instantiateModule(
            instanceName, dependencyResolver, bundleContext);
        module.setBundleContext(bundleContext);
        return module;
    }
}
