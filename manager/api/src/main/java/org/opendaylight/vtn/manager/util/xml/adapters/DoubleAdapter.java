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
 * {@code DoubleAdapter} establishes XML data binding between {@link Double}
 * type and {@code String}.
 *
 * <p>
 *   This adapter is used to detect arithmetic overflow.
 * </p>
 *
 * @since  Lithium
 */
public final class DoubleAdapter extends XmlAdapter<String, Double> {
    /**
     * Convert the given {@link Double} instance into a string.
     *
     * @param v  A {@link Double} instance to be converted.
     * @return   A string if {@code v} is not {@code null}.
     *           {@code null} if {@code v} is {@code null}.
     */
    @Override
    public String marshal(Double v) {
        return (v == null) ? null : v.toString();
    }

    /**
     * Convert the given string into a {@link Double} instance.
     *
     * @param v  A string to be converted.
     * @return   A {@link Double} instance if {@code v} is not {@code null}.
     *           {@code null} if {@code v} is {@code null}.
     */
    @Override
    public Double unmarshal(String v) {
        return (v == null) ? null : Double.valueOf(v);
    }
}
