/*
 * Copyright (c) 2014-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.cluster;

import java.util.EnumSet;

import org.opendaylight.vtn.manager.util.EtherAddress;

import org.opendaylight.vtn.manager.internal.PacketContext;
import org.opendaylight.vtn.manager.internal.VTNManagerImpl;

import org.opendaylight.vtn.manager.internal.TestBase;

import org.opendaylight.controller.sal.core.Node;
import org.opendaylight.controller.sal.core.NodeConnector;
import org.opendaylight.controller.sal.match.MatchType;
import org.opendaylight.controller.sal.packet.ARP;
import org.opendaylight.controller.sal.packet.Ethernet;
import org.opendaylight.controller.sal.utils.EtherTypes;
import org.opendaylight.controller.sal.utils.NodeConnectorCreator;
import org.opendaylight.controller.sal.utils.NodeCreator;

/**
 * Utility to test {@link PacketMatch#match(PacketContext)}.
 */
public final class PacketMatchTest extends TestBase {
    /**
     * A node connector for test.
     */
    static final NodeConnector  NODE_CONNECTOR;

    /**
     * An ARP packet for test.
     */
    static final ARP  ARP_PACKET;

    /**
     * Initialize test class.
     */
    static {
        // Create a node connector.
        Node node = NodeCreator.createOFNode(Long.valueOf(1L));
        NODE_CONNECTOR = NodeConnectorCreator.
            createNodeConnector(Short.valueOf((short)1), node);

        // Create an ARP packet.
        byte[] sha = {
            (byte)0x01, (byte)0x23, (byte)0x45,
            (byte)0x67, (byte)0x89, (byte)0xab,
        };
        byte[] tha = {
            (byte)0xff, (byte)0xff, (byte)0xff,
            (byte)0xff, (byte)0xff, (byte)0xff,
        };
        byte[] spa = {(byte)10, (byte)0, (byte)1, (byte)2};
        byte[] tpa = {(byte)192, (byte)168, (byte)100, (byte)200};

        ARP_PACKET = new ARP();
        ARP_PACKET.setHardwareType(ARP.HW_TYPE_ETHERNET).
            setHardwareAddressLength((byte)EtherAddress.SIZE).
            setProtocolType(EtherTypes.IPv4.shortValue()).
            setProtocolAddressLength((byte)4).
            setOpCode(ARP.REQUEST).
            setSenderHardwareAddress(sha).
            setTargetHardwareAddress(tha).
            setSenderProtocolAddress(spa).
            setTargetProtocolAddress(tpa);
    }

    /**
     * A set of {@link MatchType} instances to be configured in a
     * {@link PacketContext} instance.
     */
    private final EnumSet<MatchType>  matchTypes =
        EnumSet.noneOf(MatchType.class);

    /**
     * Configure {@link MatchType} instances expected to be configured
     * into {@link PacketContext}.
     *
     * @param types  An array of {@link MatchType} instances.
     * @return  This instance.
     */
    public PacketMatchTest setMatchType(MatchType ... types) {
        if (types != null) {
            for (MatchType mtype: types) {
                matchTypes.add(mtype);
            }
        }

        return this;
    }

    /**
     * Remove {@link MatchType} instances from a set of match fields
     * expected to be configured into {@link PacketContext}.
     *
     * @param types  An array of {@link MatchType} instances.
     * @return  This instance.
     */
    public PacketMatchTest clearMatchType(MatchType ... types) {
        if (types != null) {
            for (MatchType mtype: types) {
                matchTypes.remove(mtype);
            }
        }

        return this;
    }

    /**
     * Reset to the initial state.
     *
     * @return  This instance.
     */
    public PacketMatchTest reset() {
        matchTypes.clear();
        return this;
    }

    /**
     * Run test for {@link PacketMatch#match(PacketContext)}.
     *
     * @param pm     A {@link PacketMatch} instance to be tested.
     * @param ether  An {@link Ethernet} instance to be passed to {@code pm}.
     * @return  A boolean value returned by the call of
     *          {@link PacketMatch#match(PacketContext)}.
     */
    public boolean run(PacketMatch pm, Ethernet ether) {
        PacketContext pctx = createPacketContext(ether, NODE_CONNECTOR);
        boolean ret = pm.match(pctx);

        for (MatchType mtype: MatchType.values()) {
            assertEquals(mtype.toString(), matchTypes.contains(mtype),
                         pctx.hasMatchField(mtype));
        }

        return ret;
    }

    /**
     * Run test for {@link FlowCondImpl#match(VTNManagerImpl, PacketContext)}.
     *
     * @param mgr    VTN Manager service.
     * @param fci    A {@link FlowCondImpl} instance to be tested.
     * @param ether  An {@link Ethernet} instance to be passed to {@code fci}.
     * @return  A boolean value returned by the call of
     *          {@link FlowCondImpl#match(VTNManagerImpl, PacketContext)}.
     */
    public boolean run(VTNManagerImpl mgr, FlowCondImpl fci, Ethernet ether) {
        PacketContext pctx = createPacketContext(ether, NODE_CONNECTOR);
        boolean ret = fci.match(mgr, pctx);

        for (MatchType mtype: MatchType.values()) {
            assertEquals(mtype.toString(), matchTypes.contains(mtype),
                         pctx.hasMatchField(mtype));
        }

        return ret;
    }
}
