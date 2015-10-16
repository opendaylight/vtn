/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */


package org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.neutron.rev150922;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import org.opendaylight.vtn.manager.neutron.NeutronProvider;

import org.opendaylight.controller.config.api.DependencyResolver;
import org.opendaylight.controller.config.api.ModuleIdentifier;

public class VTNNeutronModule extends AbstractVTNNeutronModule {
    private static final Logger LOG =
        LoggerFactory.getLogger(VTNNeutronModule.class);

    public VTNNeutronModule(ModuleIdentifier identifier,
                            DependencyResolver dependencyResolver) {
        super(identifier, dependencyResolver);
    }

    public VTNNeutronModule(ModuleIdentifier identifier,
                            DependencyResolver dependencyResolver,
                            VTNNeutronModule oldModule,
                            AutoCloseable oldInstance) {
        super(identifier, dependencyResolver, oldModule, oldInstance);
    }

    @Override
    public void customValidation() {
        // add custom validation form module attributes here.
    }

    @Override
    public java.lang.AutoCloseable createInstance() {
        NeutronProvider provider = new NeutronProvider();
        getBrokerDependency().registerProvider(provider);
        return provider;
    }
}
