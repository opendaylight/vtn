/*
 * Copyright (c) 2014, 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.cluster;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;

import javax.xml.bind.Unmarshaller;

import org.junit.Test;

import org.mockito.Mockito;

import org.opendaylight.vtn.manager.VTNException;
import org.opendaylight.vtn.manager.util.ByteUtils;
import org.opendaylight.vtn.manager.util.EtherAddress;

import org.opendaylight.vtn.manager.flow.action.SetDlDstAction;

import org.opendaylight.vtn.manager.internal.PacketContext;
import org.opendaylight.vtn.manager.internal.packet.cache.EtherPacket;
import org.opendaylight.vtn.manager.internal.util.flow.action.VTNSetDlDstAction;

import org.opendaylight.vtn.manager.internal.TestBase;

import org.opendaylight.controller.sal.packet.Ethernet;
import org.opendaylight.controller.sal.packet.address.EthernetAddress;
import org.opendaylight.controller.sal.utils.StatusCode;

/**
 * JUnit test for {@link SetDlDstActionImpl}.
 */
public class SetDlDstActionImplTest extends TestBase {
    /**
     * Test case for getter methods.
     *
     * @throws Exception  An unexpected exception occurred.
     */
    @Test
    public void testGetter() throws Exception {
        for (EthernetAddress eaddr: createEthernetAddresses(false)) {
            byte[] bytes = eaddr.getValue();
            SetDlDstAction act = new SetDlDstAction(bytes);
            try {
                SetDlDstActionImpl impl = new SetDlDstActionImpl(act);
                assertEquals(act, impl.getFlowAction());
                assertArrayEquals(bytes, impl.getAddress().getBytes());
            } catch (Exception e) {
                unexpected(e);
            }
        }

        // null action.
        try {
            new SetDlDstActionImpl(null);
            unexpected();
        } catch (VTNException e) {
            assertEquals(StatusCode.BADREQUEST, e.getStatus().getCode());
        }

        // Create invalid MAC addresses.
        ArrayList<byte[]> addrs = new ArrayList<byte[]>();
        addrs.add(null);

        // Invalid length.
        for (int len = 0; len <= 10; len++) {
            if (len != EtherAddress.SIZE) {
                byte[] addr = new byte[len];
                if (len > 0) {
                    addr[0] = (byte)len;
                }
                addrs.add(addr);
            }
        }

        // Broadcast address.
        addrs.add(new byte[]{-1, -1, -1, -1, -1, -1});

        // Multicast addresses.
        for (EthernetAddress eaddr: createEthernetAddresses(false)) {
            byte[] bytes = eaddr.getValue();
            bytes[0] |= (byte)0x1;
            addrs.add(bytes);
        }

        // Zero address.
        addrs.add(new byte[EtherAddress.SIZE]);

        for (byte[] addr: addrs) {
            SetDlDstAction act = new SetDlDstAction(addr);
            try {
                new SetDlDstActionImpl(act);
                unexpected();
            } catch (VTNException e) {
                assertEquals(StatusCode.BADREQUEST, e.getStatus().getCode());
            }
        }

        // Specify invalid MAC address via JAXB.
        String xml = new StringBuilder(XML_DECLARATION).
            append("<setdldst address=\"bad MAC address\" />").toString();
        Unmarshaller um = createUnmarshaller(SetDlDstAction.class);
        SetDlDstAction act = unmarshal(um, xml, SetDlDstAction.class);
        try {
            new SetDlDstActionImpl(act);
            unexpected();
        } catch (VTNException e) {
            assertEquals(StatusCode.BADREQUEST, e.getStatus().getCode());
        }
    }

    /**
     * Test case for {@link SetDlDstActionImpl#apply(PacketContext)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testApply() throws Exception {
        PacketContext pctx = Mockito.mock(PacketContext.class);
        EtherAddress src = new EtherAddress(0x0123456789abL);
        EtherAddress dst = new EtherAddress(0xf0fafbfcfdfeL);
        int type = 0x1234;
        short vid = 10;
        byte pcp = 1;
        byte[] payload = new byte[]{0x01, 0x02, 0x03, 0x04};
        Ethernet pkt = createEthernet(src.getBytes(), dst.getBytes(), type,
                                      vid, pcp, payload);
        EtherPacket ether = new EtherPacket(pkt);
        Mockito.when(pctx.getEtherPacket()).thenReturn(ether);

        EtherAddress dst1 = new EtherAddress(0xa0a1a2a3a4a5L);
        VTNSetDlDstAction vact = new VTNSetDlDstAction(dst1);
        SetDlDstAction a = new SetDlDstAction(dst1);
        SetDlDstActionImpl act = new SetDlDstActionImpl(a);
        assertEquals(true, act.apply(pctx));
        Mockito.verify(pctx).getEtherPacket();
        Mockito.verify(pctx).addFilterAction(vact);
        assertEquals(src, ether.getSourceAddress());
        assertEquals(dst1, ether.getDestinationAddress());
        assertEquals(type, ether.getEtherType());
        assertEquals((int)vid, ether.getVlanId());
        assertEquals((short)pcp, ether.getVlanPriority());
    }

    /**
     * Test case for {@link SetDlDstActionImpl#equals(Object)} and
     * {@link SetDlDstActionImpl#hashCode()}.
     */
    @Test
    public void testEquals() {
        HashSet<Object> set = new HashSet<Object>();
        List<EthernetAddress> eaddrs = createEthernetAddresses(false);
        for (EthernetAddress eaddr: eaddrs) {
            byte[] bytes1 = eaddr.getValue();
            byte[] bytes2 = bytes1.clone();
            SetDlDstAction act1 = new SetDlDstAction(bytes1);
            SetDlDstAction act2 = new SetDlDstAction(bytes2);
            try {
                SetDlDstActionImpl impl1 = new SetDlDstActionImpl(act1);
                SetDlDstActionImpl impl2 = new SetDlDstActionImpl(act2);
                testEquals(set, impl1, impl2);
            } catch (Exception e) {
                unexpected(e);
            }
        }

        assertEquals(eaddrs.size(), set.size());
    }

    /**
     * Test case for {@link SetDlDstActionImpl#toString()}.
     */
    @Test
    public void testToString() {
        String prefix = "SetDlDstActionImpl[";
        String suffix = "]";
        for (EthernetAddress eaddr: createEthernetAddresses(false)) {
            byte[] bytes = eaddr.getValue();
            SetDlDstAction act = new SetDlDstAction(bytes);
            try {
                SetDlDstActionImpl impl = new SetDlDstActionImpl(act);
                String a = "addr=" + ByteUtils.toHexString(bytes);
                String required = joinStrings(prefix, suffix, ",", a);
                assertEquals(required, impl.toString());
            } catch (Exception e) {
                unexpected(e);
            }
        }
    }

    /**
     * Ensure that {@link SetDlDstActionImpl} is serializable.
     */
    @Test
    public void testSerialize() {
        for (EthernetAddress eaddr: createEthernetAddresses(false)) {
            byte[] bytes = eaddr.getValue();
            SetDlDstAction act = new SetDlDstAction(bytes);
            try {
                SetDlDstActionImpl impl = new SetDlDstActionImpl(act);
                serializeTest(impl);
            } catch (Exception e) {
                unexpected(e);
            }
        }
    }
}
