/*
 * Copyright (c) 2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal;

import org.junit.Test;

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
            assertTrue(emsg, type.match(MapType.ALL));
            assertTrue(emsg, MapType.ALL.match(type));
        }

        assertFalse(MapType.PORT.match(MapType.VLAN));
        assertFalse(MapType.VLAN.match(MapType.PORT));
    }
}
