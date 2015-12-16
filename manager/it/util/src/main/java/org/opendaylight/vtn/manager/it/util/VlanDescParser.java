/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.util;

/**
 * Utility class to parse a string that consists of an identifier string and
 * a VLAN ID.
 */
public final class VlanDescParser {
    /**
     * Separator between identifier and VLAN ID.
     */
    public static final char  SEPARATOR = '@';

    /**
     * Parsed identifier string.
     */
    private final String  identifier;

    /**
     * Parsed VLAN ID.
     */
    private final int  vlanId;

    /**
     * Construct a new instance.
     *
     * @param value  A string to be parsed.
     *               The given string must consist of an arbitrary string and
     *               VLAN ID (decimal) joined with "@".
     * @param desc   A brief description about the given string.
     */
    public VlanDescParser(String value, String desc) {
        if (value == null) {
            throw new IllegalArgumentException(desc + " cannot be null.");
        }

        int idx = value.lastIndexOf(SEPARATOR);
        if (idx == -1) {
            throw new IllegalArgumentException(
                "No separator found in " + desc + ": " + value);
        }

        identifier  = (idx == 0) ? null : value.substring(0, idx);
        vlanId = Integer.parseInt(value.substring(idx + 1));
    }

    /**
     * Return a parsed identifier string.
     *
     * @return  A parsed identifier string.
     *          {@code null} if no identifier is specified.
     */
    public String getIdentifier() {
        return identifier;
    }

    /**
     * Return a parsed VLAN ID.
     *
     * @return  A parsed VLAN ID.
     *          Zero indicates untagged network.
     */
    public int getVlanId() {
        return vlanId;
    }
}
