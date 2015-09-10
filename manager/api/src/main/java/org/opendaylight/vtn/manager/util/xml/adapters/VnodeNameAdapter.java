/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.util.xml.adapters;

import javax.xml.bind.annotation.adapters.XmlAdapter;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeName;

/**
 * {@code VnodeNameAdapter} establishes XML data binding between
 * {@link VnodeName} type and {@code String}.
 *
 * @since  Beryllium
 */
public final class VnodeNameAdapter extends XmlAdapter<String, VnodeName> {
    /**
     * Convert the given {@link VnodeName} instance into a string.
     *
     * @param v  A {@link VnodeName} instance to be converted.
     * @return   A string if {@code v} is not {@code null}.
     *           {@code null} if {@code v} is {@code null}.
     */
    @Override
    public String marshal(VnodeName v) {
        return (v == null) ? null : v.getValue();
    }

    /**
     * Convert the given string into a {@link VnodeName} instance.
     *
     * @param v  A string to be converted.
     * @return   A {@link VnodeName} instance if {@code v} is not {@code null}.
     *           {@code null} if {@code v} is {@code null}.
     */
    @Override
    public VnodeName unmarshal(String v) {
        return (v == null) ? null : new VnodeName(v);
    }
}
