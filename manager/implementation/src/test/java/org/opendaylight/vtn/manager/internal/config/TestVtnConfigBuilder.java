/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.config;

import java.util.Map;
import java.util.EnumMap;

import org.opendaylight.vtn.manager.internal.TestBase;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.config.rev150209.VtnConfig;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.config.rev150209.VtnConfigBuilder;

/**
 * A helper class to verify the contents of {@link VtnConfig} and
 * {@link VtnConfigBuilder} instance.
 */
public final class TestVtnConfigBuilder extends TestBase {
    /**
     * Expected values.
     */
    private final Map<ConfigType, Object>  expectedValues =
        new EnumMap<>(ConfigType.class);

    /**
     * Fill the configuration with default values.
     *
     * @return  This instance.
     */
    public TestVtnConfigBuilder fillDefault() {
        for (ConfigType type: ConfigType.values()) {
            if (!expectedValues.containsKey(type)) {
                expectedValues.put(type, type.getDefaultValue());
            }
        }
        return this;
    }

    /**
     * Set expected value for the given parameter.
     *
     * @param type   The parameter type.
     * @param value  An expected value.
     * @return  This instance.
     */
    public TestVtnConfigBuilder set(ConfigType type, Object value) {
        expectedValues.put(type, value);
        return this;
    }

    /**
     * Verify the contents of {@link VtnConfigBuilder} instance.
     *
     * @param builder  A {@link VtnConfigBuilder} instance.
     * @return  This instance.
     */
    public TestVtnConfigBuilder verify(VtnConfigBuilder builder) {
        for (ConfigType type: ConfigType.values()) {
            Object expected = expectedValues.get(type);
            assertEquals(expected, type.get(builder));
        }
        return this;
    }

    /**
     * Verify the contents of {@link VtnConfig} instance.
     *
     * @param vcfg  A {@link VtnConfig} instance.
     * @return  This instance.
     */
    public TestVtnConfigBuilder verify(VtnConfig vcfg) {
        for (ConfigType type: ConfigType.values()) {
            Object expected = expectedValues.get(type);
            assertEquals(expected, type.get(vcfg, VtnConfig.class));
        }
        return this;
    }
}
