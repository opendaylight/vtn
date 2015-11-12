/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.config;

import java.util.Map;
import java.util.EnumMap;

import org.opendaylight.vtn.manager.util.EtherAddress;

import org.opendaylight.vtn.manager.internal.VTNConfig;

import org.opendaylight.vtn.manager.internal.TestBase;

import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.yang.types.rev100924.MacAddress;

/**
 * A helper class to verify the contents of {@link VTNConfig} instance.
 */
public final class TestVTNConfig extends TestBase {
    /**
     * Expected values.
     */
    private final Map<ConfigType, Object>  expectedValues =
        new EnumMap<>(ConfigType.class);

    /**
     * Set expected value for the given parameter.
     *
     * @param type   The parameter type.
     * @param value  An expected value.
     * @return  This instance.
     */
    public TestVTNConfig set(ConfigType type, Object value) {
        expectedValues.put(type, value);
        return this;
    }

    /**
     * Reset all the parameters except controller-mac-address.
     *
     * @return  This instance.
     */
    public TestVTNConfig reset() {
        for (ConfigType type: ConfigType.values()) {
            if (type != ConfigType.CONTROLLER_MAC_ADDRESS) {
                expectedValues.remove(type);
            }
        }
        return this;
    }

    /**
     * Verify the contents of {@link VTNConfig} instance.
     *
     * @param vconf  A {@link VTNConfig} instance.
     * @return  This instance.
     */
    public TestVTNConfig verify(VTNConfig vconf) {
        for (ConfigType type: ConfigType.values()) {
            Object expected = expectedValues.get(type);
            if (expected == null) {
                expected = type.getDefaultValue();
            }

            Object value = type.get(vconf);
            if (expected instanceof MacAddress) {
                EtherAddress ea = new EtherAddress((MacAddress)expected);
                assertEquals(ea, value);
            } else {
                assertEquals(expected, value);
            }
        }
        return this;
    }
}
