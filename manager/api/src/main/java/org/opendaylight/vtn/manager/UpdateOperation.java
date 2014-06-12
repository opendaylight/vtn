/*
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager;

/**
 * {@code UpdateOperation} is an enumerated class which is used to specify
 * the method during modifications in VTN Manager settings.
 *
 * @since  Helium
 */
public enum UpdateOperation {
    /**
     * Indicates that present settings are to be replaced with the specified
     * contents.
     */
    SET,

    /**
     * Indicates that the specified contents are to be added to the present
     * settings.
     */
    ADD,

    /**
     * Indicates that the specified contents are to be deleted from the
     * present settings.
     */
    REMOVE;
}
