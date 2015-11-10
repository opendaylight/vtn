/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.it.ownermock.service.rev151110;

import org.opendaylight.vtn.manager.it.ownermock.impl.OwnerMockService;

import org.opendaylight.controller.config.api.ModuleIdentifier;
import org.opendaylight.controller.config.api.DependencyResolver;

/**
 * MD-SAL service module for the mock-up of the entity ownership service.
 */
public class OwnerMockServiceModule extends AbstractOwnerMockServiceModule {
    /**
     * Construct a new mock-up of the entity ownership service.
     *
     * @param identifier
     *    Module identifier for the entity ownership mock-up service.
     * @param dependencyResolver
     *    Dependency resolver.
     */
    public OwnerMockServiceModule(ModuleIdentifier identifier,
                                  DependencyResolver dependencyResolver) {
        super(identifier, dependencyResolver);
    }

    /**
     * Construct a new mock-up of the entity ownership service.
     *
     * @param identifier
     *    Module identifier for the entity ownership mock-up service.
     * @param dependencyResolver
     *    Dependency resolver.
     * @param oldModule
     *    Old {@link OwnerMockServiceModule} instance.
     * @param oldInstance
     *    Old service instance.
     */
    public OwnerMockServiceModule(ModuleIdentifier identifier,
                                  DependencyResolver dependencyResolver,
                                  OwnerMockServiceModule oldModule,
                                  AutoCloseable oldInstance) {
        super(identifier, dependencyResolver, oldModule, oldInstance);
    }

    /**
     * Construct a new mock-up of the entity ownership service.
     *
     * @return  An {@link OwnerMockService} instance.
     */
    @Override
    public AutoCloseable createInstance() {
        return new OwnerMockService();
    }
}
