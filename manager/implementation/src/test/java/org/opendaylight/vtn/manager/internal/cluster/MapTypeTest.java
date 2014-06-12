/*
 * Copyright (c) 2013-2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.cluster;

import org.junit.Test;

import org.opendaylight.vtn.manager.internal.TestBase;

/**
 * JUnit test for {@link MapType}.
 */
public class MapTypeTest extends TestBase {
    /**
     * Test case for {@link MapType#match(MapType)}.
     */
    @Test
    public void testMatch() {
        MapType[] types = MapType.values();
        for (MapType type: types) {
            String emsg = "(MapType)" + type.toString();
            assertTrue(emsg, type.match(type));

            for (MapType type2: types) {
                boolean matches = (type.equals(MapType.ALL) ||
                                   type2.equals(MapType.ALL) ||
                                   type.equals(type2));
                assertEquals(matches, type.match(type2));
                assertEquals(matches, type2.match(type));
            }
        }
    }

    /**
     * Ensure that {@link MapType} is serializable.
     */
    @Test
    public void testSerialize() {
        for (MapType type: MapType.values()) {
            serializeTest(type);
        }
    }
}
