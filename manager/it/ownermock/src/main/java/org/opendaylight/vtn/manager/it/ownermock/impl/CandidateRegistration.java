/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.ownermock.impl;

import org.opendaylight.controller.md.sal.common.api.clustering.Entity;
import org.opendaylight.controller.md.sal.common.impl.clustering.AbstractEntityOwnershipCandidateRegistration;

/**
 * {@code CandidateRegistration} describes a registration of an entity
 * registered to the mock-up of the entity ownership service.
 */
final class CandidateRegistration
    extends AbstractEntityOwnershipCandidateRegistration {
    /**
     * The mock-up of the entity ownership service.
     */
    private final OwnerMockService  ownerService;

    /**
     * Construct a new instance.
     *
     * @param service  The mock-up of the entity ownership service.
     * @param entity   The registered entity.
     */
    CandidateRegistration(OwnerMockService service, Entity entity) {
        super(entity);
        ownerService = service;
    }

    // AbstractRegistration

    /**
     * Unregister the entity associated with this registration.
     */
    @Override
    protected void removeRegistration() {
        ownerService.removeCandidate(getInstance());
    }
}
