/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.util.xml.adapters;

import javax.xml.bind.annotation.adapters.XmlAdapter;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnPortDesc;

/**
 * {@code VtnPortDescAdapter} establishes XML data binding between
 * {@link VtnPortDesc} type and {@code String}.
 *
 * @since  Beryllium
 */
public final class VtnPortDescAdapter extends XmlAdapter<String, VtnPortDesc> {
    /**
     * Convert the given {@link VtnPortDesc} instance into a string.
     *
     * @param v  A {@link VtnPortDesc} instance to be converted.
     * @return   A string if {@code v} is not {@code null}.
     *           {@code null} if {@code v} is {@code null}.
     */
    @Override
    public String marshal(VtnPortDesc v) {
        return (v == null) ? null : v.getValue();
    }

    /**
     * Convert the given string into a {@link VtnPortDesc} instance.
     *
     * @param v  A string to be converted.
     * @return   A {@link VtnPortDesc} instance if {@code v} is not
     *           {@code null}. {@code null} if {@code v} is {@code null}.
     */
    @Override
    public VtnPortDesc unmarshal(String v) {
        return (v == null) ? null : new VtnPortDesc(v);
    }
}
