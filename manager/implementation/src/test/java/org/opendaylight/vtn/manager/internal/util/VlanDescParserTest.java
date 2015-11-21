/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util;

import org.junit.Test;

import org.opendaylight.vtn.manager.internal.util.rpc.RpcErrorTag;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;

import org.opendaylight.vtn.manager.internal.TestBase;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnErrorTag;

/**
 * JUnit test for {@link VlanDescParser}.
 */
public class VlanDescParserTest extends TestBase {
    /**
     * Ensure that {@link VlanDescParser} can parse the descriptor.
     *
     * <ul>
     *   <li>{@link VlanDescParser#VlanDescParser(String, String)}</li>
     *   <li>{@link VlanDescParser#getIdentifier()}</li>
     *   <li>{@link VlanDescParser#getVlanId()}</li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testSuccess() throws Exception {
        String[] ids = {null, "id", "test", "openflow:1", "openflow:3:45"};
        int[] vids = {0, 1, 3, 45, 123, 789, 1024, 3193, 4094, 4095};

        for (String id: ids) {
            for (int vid: vids) {
                StringBuilder builder = new StringBuilder();
                if (id != null) {
                    builder.append(id);
                }
                String value = builder.append('@').append(vid).toString();
                VlanDescParser parser = new VlanDescParser(value, "test");
                assertEquals(id, parser.getIdentifier());
                assertEquals(vid, parser.getVlanId());
            }
        }
    }

    /**
     * Ensure that {@link VlanDescParser#VlanDescParser(String, String)} can
     * detect invalid value.
     */
    @Test
    public void testFailure() {
        // Null value.
        String desc = "test value";

        RpcErrorTag etag = RpcErrorTag.MISSING_ELEMENT;
        VtnErrorTag vtag = VtnErrorTag.BADREQUEST;
        String msg = desc + " cannot be null";
        try {
            new VlanDescParser(null, desc);
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            assertEquals(vtag, e.getVtnErrorTag());
            assertEquals(msg, e.getMessage());
        }

        // No separator.
        etag = RpcErrorTag.BAD_ELEMENT;
        String[] noSeparators = {
            "", "id", "openflow:1", "Bad descriptor",
        };

        for (String value: noSeparators) {
            msg = "No separator found in " + desc + ": " + value;
            try {
                new VlanDescParser(value, desc);
                unexpected();
            } catch (RpcException e) {
                assertEquals(etag, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                assertEquals(msg, e.getMessage());
            }
        }

        // Invalid integer at VLAN ID field.
        String[] badNumbers = {
            "", "0x123", "bad VID"
        };
        for (String vid: badNumbers) {
            String value = "id@" + vid;
            msg = "Invalid VLAN ID in " + desc + ": " + value;
            try {
                new VlanDescParser(value, desc);
                unexpected();
            } catch (RpcException e) {
                assertEquals(etag, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                assertEquals(msg, e.getMessage());

                Throwable cause = e.getCause();
                assertTrue(cause instanceof NumberFormatException);
            }
        }

        // Invalid VLAN ID.
        int[] badVids = {-123, -1, 4096, 4097, 12345678};
        for (int vid: badVids) {
            String value = "id@" + vid;
            msg = "Invalid VLAN ID in " + desc + ": " + value;
            try {
                new VlanDescParser(value, desc);
                unexpected();
            } catch (RpcException e) {
                assertEquals(etag, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                assertEquals(msg, e.getMessage());

                Throwable cause = e.getCause();
                assertTrue(cause instanceof RpcException);
            }
        }
    }
}
