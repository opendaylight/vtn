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
 * An instance of {@code MacMapAclType} class specifies the access control
 * list which determines hosts to be mapped by the MAC mapping.
 *
 * @see    <a href="package-summary.html#MAC-map">MAC mapping</a>
 * @since  Helium
 */
public enum MacMapAclType {
    /**
     * Indicates the {@linkplain <a href="package-summary.html#MAC-map.allow">Map Allow list</a>}.
     */
    ALLOW,

    /**
     * Indicates the {@linkplain <a href="package-summary.html#MAC-map.deny">Map Deny list</a>}.
     */
    DENY;
}
