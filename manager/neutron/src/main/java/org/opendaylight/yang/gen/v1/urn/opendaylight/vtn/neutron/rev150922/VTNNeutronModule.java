/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */


package org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.neutron.rev150922;

import org.opendaylight.vtn.manager.neutron.NeutronProvider;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

public class VTNNeutronModule extends org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.neutron.rev150922.AbstractVTNNeutronModule {
    private static final Logger LOG = LoggerFactory.getLogger(VTNNeutronModule.class);
    public VTNNeutronModule(org.opendaylight.controller.config.api.ModuleIdentifier identifier, org.opendaylight.controller.config.api.DependencyResolver dependencyResolver) {
        super(identifier, dependencyResolver);
    }

    public VTNNeutronModule(org.opendaylight.controller.config.api.ModuleIdentifier identifier, org.opendaylight.controller.config.api.DependencyResolver dependencyResolver, org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.neutron.rev150922.VTNNeutronModule oldModule, java.lang.AutoCloseable oldInstance) {
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
