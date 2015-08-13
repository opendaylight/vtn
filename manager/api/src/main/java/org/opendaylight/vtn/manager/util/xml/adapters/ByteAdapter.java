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
 * {@code ByteAdapter} establishes XML data binding between {@link Byte}
 * type and {@code String}.
 *
 * <p>
 *   This adapter is used to detect arithmetic overflow.
 * </p>
 *
 * @since  Lithium
 */
public final class ByteAdapter extends XmlAdapter<String, Byte> {
    /**
     * Convert the given {@link Byte} instance into a string.
     *
     * @param v  A {@link Byte} instance to be converted.
     * @return   A string if {@code v} is not {@code null}.
     *           {@code null} if {@code v} is {@code null}.
     */
    @Override
    public String marshal(Byte v) {
        return (v == null) ? null : v.toString();
    }

    /**
     * Convert the given string into a {@link Byte} instance.
     *
     * @param v  A string to be converted.
     * @return   A {@link Byte} instance if {@code v} is not {@code null}.
     *           {@code null} if {@code v} is {@code null}.
     */
    @Override
    public Byte unmarshal(String v) {
        return (v == null) ? null : Byte.decode(v);
    }
}
