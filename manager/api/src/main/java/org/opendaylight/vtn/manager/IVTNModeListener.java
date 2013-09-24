/*
 * Copyright (c) 2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager;

/**
 * This interface provides methods to notify listeners of VTN mode change.
 */
public interface IVTNModeListener {
    /**
     * Invoked when the VTN mode has changed.
     *
     * @param active  {@code true} is passed if the VTN is active in the
     *                container. Otherwise {@code false} is passed.
     */
    void vtnModeChanged(boolean active);
}
