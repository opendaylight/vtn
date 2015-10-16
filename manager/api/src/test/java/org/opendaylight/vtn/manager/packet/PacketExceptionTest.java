/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.packet;

import org.junit.Test;

import org.opendaylight.vtn.manager.TestBase;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnErrorTag;

/**
 * JUnit test for {@link PacketException}.
 */
public class PacketExceptionTest extends TestBase {
    /**
     * Test case for {@link PacketException#PacketException(String)}.
     */
    @Test
    public void testDefault() {
        String[] messages = {
            null,
            "message 1",
            "message 2",
        };

        IllegalArgumentException cause = new IllegalArgumentException();
        for (String msg: messages) {
            PacketException e = new PacketException(msg);
            assertEquals(VtnErrorTag.INTERNALERROR, e.getVtnErrorTag());
            assertEquals(msg, e.getMessage());
            assertEquals(null, e.getCause());

            // Ensure that no cause is configured.
            e.initCause(cause);
            assertEquals(cause, e.getCause());
        }
    }

    /**
     * Test case for {@link PacketException#PacketException(String, Throwable)}.
     */
    @Test
    public void testCause() {
        String[] messages = {
            null,
            "message 1",
            "message 2",
        };
        Throwable[] causes = {
            new IllegalArgumentException(),
            new IllegalStateException(),
            new NullPointerException(),
        };

        for (String msg: messages) {
            for (Throwable cause: causes) {
                PacketException e = new PacketException(msg, cause);
                assertEquals(VtnErrorTag.INTERNALERROR, e.getVtnErrorTag());
                assertEquals(msg, e.getMessage());
                assertEquals(cause, e.getCause());
            }
        }
    }
}
