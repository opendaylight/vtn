/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.util.xml.adapters;

import javax.xml.bind.annotation.adapters.XmlAdapter;

/**
 * {@code LongAdapter} establishes XML data binding between {@link Long}
 * type and {@code String}.
 *
 * <p>
 *   This adapter is used to detect arithmetic overflow.
 * </p>
 *
 * @since  Lithium
 */
public final class LongAdapter extends XmlAdapter<String, Long> {
    /**
     * Convert the given {@link Long} instance into a string.
     *
     * @param v  A {@link Long} instance to be converted.
     * @return   A string if {@code v} is not {@code null}.
     *           {@code null} if {@code v} is {@code null}.
     */
    @Override
    public String marshal(Long v) {
        return (v == null) ? null : v.toString();
    }

    /**
     * Convert the given string into a {@link Long} instance.
     *
     * @param v  A string to be converted.
     * @return   A {@link Long} instance if {@code v} is not {@code null}.
     *           {@code null} if {@code v} is {@code null}.
     */
    @Override
    public Long unmarshal(String v) {
        return (v == null) ? null : Long.decode(v);
    }
}
