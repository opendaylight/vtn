/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.inventory;

import org.opendaylight.vtn.manager.VTNException;

/**
 * Interface for classes that listen change of VTN node information.
 */
public interface VTNInventoryListener {
    /**
     * Invoked when a node information has been added or removed.
     *
     * @param ev  A {@link VtnNodeEvent} instance.
     * @throws VTNException  An error occurred.
     */
    void notifyVtnNode(VtnNodeEvent ev) throws VTNException;

    /**
     * Invoked when a port information has been added, removed, or changed.
     *
     * @param ev  A {@link VtnPortEvent} instance.
     * @throws VTNException  An error occurred.
     */
    void notifyVtnPort(VtnPortEvent ev) throws VTNException;
}
