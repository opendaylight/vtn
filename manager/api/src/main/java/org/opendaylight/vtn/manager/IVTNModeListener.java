/*
 * Copyright (c) 2013, 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager;

/**
 * {@code IVTNModeListener} defines the listener interface that monitors
 * the presence of virtual network environment provided by the VTN Manager
 * inside the container.
 *
 * <p>
 *   Once an OSGi service which implements {@code IVTNModeListener} is
 *   registered in the OSGi service registry, {@link #vtnModeChanged(boolean)}
 *   is called when the virtual networking provided by the VTN Manager is
 *   started or stopped.
 * </p>
 */
public interface IVTNModeListener {
    /**
     * Invoked when the operation mode of the VTN Manager inside the container
     * is changed.
     *
     * <p>
     *   This method is called when the value returned by
     *   {@link IVTNManager#isActive()} is changed. The value to be returned
     *   by the call of {@link IVTNManager#isActive()} is passed to
     *   {@code active}. That is, {@code true} is passed if the virtual
     *   networking provided by the VTN Manager is operating, and {@code false}
     *   is specified if it is not operating.
     * </p>
     * <p>
     *   In order to notify the VTN Manager operation mode, this method will
     *   be called just after {@code IVTNModeListener} listener is registered
     *   in the VTN Manager.
     * </p>
     *
     * @param active  {@code true} is passed if the virtual networking
     *                provided by the VTN Manager becomes active in the
     *                container. Otherwise {@code false} is passed.
     */
    void vtnModeChanged(boolean active);
}
