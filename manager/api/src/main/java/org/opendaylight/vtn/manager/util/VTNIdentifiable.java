/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.util;

/**
 * {@code VTNIdentifiable} describes an object that can be identified by
 * a {@link Comparable}.
 *
 * @param <T>  The type of identifier.
 * @since  Lithium
 */
public interface VTNIdentifiable<T extends Comparable> {
    /**
     * Return the identifier of this instance.
     *
     * @return  A {@link Comparable} instance that identifies this instance.
     */
    T getIdentifier();
}
