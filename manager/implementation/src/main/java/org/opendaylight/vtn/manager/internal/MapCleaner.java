/*
 * Copyright (c) 2014, 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal;

/**
 * {@code MapCleaner} provides interfaces to be implemented by classes
 * which purge cached network resources created by virtual network mapping.
 */
public interface MapCleaner {
    /**
     * Purge caches for the virtual network.
     *
     * <p>
     *   Note that this method will be called with holding the lock of the
     *   VTN Manager specified by {@code mgr}.
     * </p>
     *
     * @param mgr         VTN Manager service.
     * @param tenantName  The name of virtual tenant.
     */
    void purge(VTNManagerImpl mgr, String tenantName);
}
