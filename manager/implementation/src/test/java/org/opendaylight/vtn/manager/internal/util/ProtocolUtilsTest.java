/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
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

import org.opendaylight.controller.sal.utils.Status;
import org.opendaylight.controller.sal.utils.StatusCode;

/**
 * JUnit test for {@link ProtocolUtils}.
 */
public class ProtocolUtilsTest extends TestBase {
    /**
     * Verify static constants.
     */
    @Test
    public void testConstants() {
        assertEquals(0xffff, ProtocolUtils.MASK_ETHER_TYPE);
        assertEquals(12, ProtocolUtils.NBITS_VLAN_ID);
        assertEquals(0xfffL, ProtocolUtils.MASK_VLAN_ID);
    }

    /**
     * Test case for {@link ProtocolUtils#checkEtherType(int)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testCheckEtherType() throws Exception {
        int[] types = {
            0, 1, 2, 0x100, 0x200, 0x300, 0x1000, 0x2000, 0x3333, 0x7fff,
            0x8000, 0x8001, 0x9999, 0xaaaa, 0xbbbb, 0xcccc, 0xeeee, 0xffff,
        };
        for (int type: types) {
            ProtocolUtils.checkEtherType(type);
        }

        int[] badTypes = {
            Integer.MIN_VALUE, -10000000, -2000000, -999999, -23456, -2, -1,
            0x10000, 0x10001, 0x10002, 0x999999, 0xaaaaaaa, 0xbbbbbbb,
            Integer.MAX_VALUE - 1, Integer.MAX_VALUE,
        };
        for (int type: badTypes) {
            try {
                ProtocolUtils.checkEtherType(type);
                unexpected();
            } catch (RpcException e) {
                Status st = e.getStatus();
                assertEquals(StatusCode.BADREQUEST, st.getCode());
                assertEquals("Invalid Ethernet type: " + type,
                             st.getDescription());
                assertEquals(RpcErrorTag.BAD_ELEMENT, e.getErrorTag());
            }
        }
    }

    /**
     * Test case for {@link ProtocolUtils#checkVlan(short)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testCheckVlan() throws Exception {
        for (short vid = 0; vid <= 4095; vid++) {
            ProtocolUtils.checkVlan(vid);
        }

        short[] badVids = {
            Short.MIN_VALUE, -30000, -20000, -10000, -5000, -100, -23, -2, -1,
            4096, 4097, 6000, 8000, 10000, 15000, 20000, 30000, 32000,
            Short.MAX_VALUE,
        };
        for (short vid: badVids) {
            try {
                ProtocolUtils.checkVlan(vid);
                unexpected();
            } catch (RpcException e) {
                Status st = e.getStatus();
                assertEquals(StatusCode.BADREQUEST, st.getCode());
                assertEquals("Invalid VLAN ID: " + vid, st.getDescription());
                assertEquals(RpcErrorTag.BAD_ELEMENT, e.getErrorTag());
            }
        }
    }

    /**
     * Test case for {@link ProtocolUtils#isVlanPriorityValid(short)} and
     * {@link ProtocolUtils#checkVlanPriority(short)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testCheckVlanPriority() throws Exception {
        for (short pcp = 0; pcp <= 7; pcp++) {
            assertEquals(true, ProtocolUtils.isVlanPriorityValid(pcp));
            ProtocolUtils.checkVlanPriority(pcp);
        }

        short[] invalid = {
            Short.MIN_VALUE, -2048, -256, -3, -2, -1,
            8, 9, 3072, 28672, 32512, Short.MAX_VALUE,
        };
        for (short pcp: invalid) {
            assertEquals(false, ProtocolUtils.isVlanPriorityValid(pcp));
            try {
                ProtocolUtils.checkVlanPriority(pcp);
                unexpected();
            } catch (RpcException e) {
                Status st = e.getStatus();
                assertEquals(StatusCode.BADREQUEST, st.getCode());
                assertEquals("Invalid VLAN priority: " + pcp,
                             st.getDescription());
                assertEquals(RpcErrorTag.BAD_ELEMENT, e.getErrorTag());
            }
        }
    }

    /**
     * Test case for {@link ProtocolUtils#isDscpValid(short)}.
     */
    @Test
    public void testIsDscpValid() {
        for (int i = 0; i <= 0xff; i++) {
            short dscp = (short)i;
            assertEquals(dscp >= 0 && dscp <= 63,
                         ProtocolUtils.isDscpValid(dscp));
        }
    }

    /**
     * Test case for {@link ProtocolUtils#dscpToTos(short)} and
     * {@link ProtocolUtils#tosToDscp(int)}.
     */
    @Test
    public void testDscpToTos() {
        for (short dscp = 0; dscp <= 63; dscp++) {
            int tos = ProtocolUtils.dscpToTos(dscp);
            assertEquals((int)dscp << 2, tos);
            assertEquals(dscp, ProtocolUtils.tosToDscp(tos));
        }

        assertEquals(0xfc, ProtocolUtils.dscpToTos((short)0xffff));
        assertEquals(0x3f, ProtocolUtils.tosToDscp(0xffffffff));
    }

    /**
     * Test case for {@link ProtocolUtils#isIcmpValueValid(short)}.
     */
    @Test
    public void testIsIcmpValueValid() {
        for (short v = 0; v <= 0xff; v++) {
            assertTrue(ProtocolUtils.isIcmpValueValid(v));
        }

        short[] badValues = {
            Short.MIN_VALUE, -30000, -20000, -10000, -5000, -100, -14, -2, -1,
            256, 257, 300, 2000, 8000, 12000, 28000, 30000, Short.MAX_VALUE,
        };
        for (short v: badValues) {
            assertFalse(ProtocolUtils.isIcmpValueValid(v));
        }
    }

    /**
     * Test case for {@link ProtocolUtils#isPortNumberValid(int)}.
     */
    @Test
    public void testIsPortNumberValid() {
        int[] validPorts = {
            0, 1, 44, 555, 1000, 4000, 10000, 13000, 20000, 30000, 40000,
            50000, 65000, 65534, 65535,
        };
        for (int port: validPorts) {
            assertTrue(ProtocolUtils.isPortNumberValid(port));
        }

        int[] badValues = {
            Integer.MIN_VALUE, (int)0xfff00000, (int)0xf0000000, -10000,
            -100, -3, -2, -1,
            0x10000, 0x10001, 0x11000, 0x100000, 0x333333,
            0x444444, 0x500000, 0x1000000, 0xa000000, 0x10000000,
            0x50000000, 0x7f000000, Integer.MAX_VALUE,
        };
        for (int port: badValues) {
            assertFalse(ProtocolUtils.isPortNumberValid(port));
        }
    }
}
