/*
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager;

import org.junit.Test;

/**
 * JUnit test for {@link MacMapAclType}.
 */
public class MacMapAclTypeTest extends TestBase {
    /**
     * Test case for enum values.
     */
    @Test
    public void testEnum() {
        MacMapAclType[] values = MacMapAclType.values();
        assertEquals(2, values.length);
        assertEquals(MacMapAclType.ALLOW, values[0]);
        assertEquals(MacMapAclType.DENY, values[1]);
        assertEquals(0, MacMapAclType.ALLOW.ordinal());
        assertEquals(1, MacMapAclType.DENY.ordinal());
    }

    /**
     * Ensure that {@link MacMapAclType} is serializable.
     */
    @Test
    public void testSerialize() {
        for (MacMapAclType type: MacMapAclType.values()) {
            serializeTest(type);
        }
    }
}
