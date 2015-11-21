/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.inventory;

import javax.xml.bind.annotation.adapters.XmlAdapter;

/**
 * {@link SalNodeAdapter} establishes XML data binding between {@link SalNode}
 * type and {@link String}.
 */
public final class SalNodeAdapter extends XmlAdapter<String, SalNode> {
    /**
     * Convert the given {@link SalNode} instance into a string.
     *
     * @param v  A {@link SalNode} instance to be converted.
     * @return  A string if {@code v} is not {@code null}.
     *          {@code null} if {@code v} is {@code null}.
     */
    @Override
    public String marshal(SalNode v) {
        return (v == null) ? null : v.toString();
    }

    /**
     * Convert the given string into a {@link SalNode} instance.
     *
     * @param v  A string to be converted.
     * @return  A {@link SalNode} instance if {@code v} is not {@code null}.
     *          {@code null} if {@code v} is {@code null}.
     * @throws IllegalArgumentException
     *    The given string could not be converted.
     */
    @Override
    public SalNode unmarshal(String v) {
        SalNode snode;
        if (v == null) {
            snode = null;
        } else {
            snode = SalNode.create(v);
            if (snode == null) {
                throw new IllegalArgumentException(
                    "Unsupported SAL node identifier: " + v);
            }
        }

        return snode;
    }
}
