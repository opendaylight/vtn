/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.util.xml.adapters;

import javax.xml.bind.annotation.adapters.XmlAdapter;

import org.opendaylight.yang.gen.v1.urn.opendaylight.l2.types.rev130827.VlanId;

/**
 * {@code VlanIdAdapter} establishes XML data binding between {@link VlanId}
 * type and {@link Integer}.
 *
 * @since  Beryllium
 */
public final class VlanIdAdapter extends XmlAdapter<String, VlanId> {
    /**
     * Convert the given {@link VlanId} instance into a string.
     *
     * @param v  A {@link VlanId} instance to be converted.
     * @return   A string if {@code v} is not {@code null}.
     *           {@code null} if {@code v} is {@code null}.
     */
    @Override
    public String marshal(VlanId v) {
        return (v == null) ? null : String.valueOf(v.getValue());
    }

    /**
     * Convert the given string into a {@link VlanId} instance.
     *
     * @param v  A string to be converted.
     * @return   A {@link VlanId} instance if {@code v} is not {@code null}.
     *           {@code null} if {@code v} is {@code null}.
     */
    @Override
    public VlanId unmarshal(String v) {
        return (v == null) ? null : new VlanId(Integer.decode(v));
    }
}
