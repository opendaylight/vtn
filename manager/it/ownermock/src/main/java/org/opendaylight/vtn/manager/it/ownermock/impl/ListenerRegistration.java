/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.ownermock.impl;

import org.opendaylight.controller.md.sal.common.api.clustering.EntityOwnershipListener;
import org.opendaylight.controller.md.sal.common.impl.clustering.AbstractEntityOwnershipListenerRegistration;

/**
 * {@code ListenerRegistration} describes a registration of an entity ownership
 * listener registered to the mock-up of the entity ownership service.
 */
final class ListenerRegistration
    extends AbstractEntityOwnershipListenerRegistration {
    /**
     * The mock-up of the entity ownership service.
     */
    private final OwnerMockService  ownerService;

    /**
     * Construct a new instance.
     *
     * @param service   The mock-up of the entity ownership service.
     * @param type      The type of the entity.
     * @param listener  The registered entity ownership listener.
     */
    ListenerRegistration(OwnerMockService service, String type,
                         EntityOwnershipListener listener) {
        super(listener, type);
        ownerService = service;
    }

    // AbstractRegistration

    /**
     * Unregister the listener associated with this registration.
     */
    @Override
    protected void removeRegistration() {
        ownerService.removeListener(this);
    }
}
