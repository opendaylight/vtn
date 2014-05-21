/*
 * Copyright (c) 2013-2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.northbound.integrationtest;

import java.io.File;
import java.net.InetAddress;
import java.util.HashSet;
import java.util.Set;

import org.junit.Assert;
import org.opendaylight.controller.sal.core.NodeConnector;
import org.opendaylight.controller.sal.packet.ARP;
import org.opendaylight.controller.sal.packet.Ethernet;
import org.opendaylight.controller.sal.packet.IEEE8021Q;
import org.opendaylight.controller.sal.packet.RawPacket;
import org.opendaylight.controller.sal.packet.address.EthernetAddress;
import org.opendaylight.controller.sal.utils.EtherTypes;
import org.opendaylight.vtn.manager.internal.PacketContext;

/**
 * Abstract base class for JUnit tests.
 */
public abstract class TestBase extends Assert {
    /**
     * Throw an error which indicates an unexpected throwable is caught.
     *
     * @param t  A throwable.
     */
    protected static void unexpected(Throwable t) {
        throw new AssertionError("Unexpected throwable: " + t, t);
    }

    /**
     * Create a copy of the specified {@link NodeConnector}.
     *
     * @param nc  A {@link NodeConnector} object to be copied.
     * @return    A copied {@link NodeConnector} object.
     */
    protected static NodeConnector copy(NodeConnector nc) {
        if (nc != null) {
            try {
                nc = new NodeConnector(nc);
            } catch (Exception e) {
                unexpected(e);
            }
        }
        return nc;
    }

    /**
     * Create a deep copy of the specified {@link InetAddress} set.
     *
     * @param ia    A {@link InetAddress} set to be copied.
     * @return      A copied {@link InetAddress} set.
     */
    protected static Set<InetAddress> copy(Set<InetAddress> ia) {
        if (ia != null) {
            Set<InetAddress> newset = new HashSet<InetAddress>();
            try {
                for (InetAddress iaddr: ia) {
                    newset.add(InetAddress.getByAddress(iaddr.getAddress()));
                }
            } catch (Exception e) {
                unexpected(e);
            }
            ia = newset;
        }
        return ia;
    }

    /**
     * create a {@link RawPacket} object.
     *
     * @param eth   A {@link Ethernet} object.
     * @param nc    A node connector.
     * @return A {@link RawPacket} object.
     */
    protected RawPacket createRawPacket (Ethernet eth, NodeConnector nc) {
        RawPacket raw = null;
        try {
            raw = new RawPacket(eth.serialize());
        } catch (Exception e) {
            unexpected(e);
        }
        raw.setIncomingNodeConnector(copy(nc));

        return raw;
    }

    /**
     * create a {@link Ethernet} object of ARP Packet.
     *
     * @param src       A source MAC address
     * @param dst       A destination MAC address
     * @param sender    A sender address
     * @param target    A target address
     * @param vlan      specify val ID. if vlan < 0, vlan tag is not added.
     * @param arptype   ARP.REQUEST or ARP.REPLY. (ARP Reply is not implemented yet )
     * @return  A {@link Ethernet} object.
     */
    protected Ethernet createARPPacket (byte[] src, byte[] dst, byte[] sender, byte [] target, short vlan, short arptype) {
        ARP arp = new ARP();
        arp.setHardwareType(ARP.HW_TYPE_ETHERNET).
            setProtocolType(EtherTypes.IPv4.shortValue()).
            setHardwareAddressLength((byte)EthernetAddress.SIZE).
            setProtocolAddressLength((byte)target.length).
            setOpCode(arptype).
            setSenderHardwareAddress(src).setSenderProtocolAddress(sender).
            setTargetHardwareAddress(dst).setTargetProtocolAddress(target);

        Ethernet eth = new Ethernet();
        eth.setSourceMACAddress(src).setDestinationMACAddress(dst);

        if (vlan >= 0) {
            eth.setEtherType(EtherTypes.VLANTAGGED.shortValue());

            IEEE8021Q vlantag = new IEEE8021Q();
            vlantag.setCfi((byte) 0x0).setPcp((byte) 0x0).setVid((short) vlan).
                setEtherType(EtherTypes.ARP.shortValue()).setParent(eth);
            eth.setPayload(vlantag);

            vlantag.setPayload(arp);

        } else {
            eth.setEtherType(EtherTypes.ARP.shortValue()).setPayload(arp);
        }
        return eth;
    }

    /**
     * create a {@link RawPacket} object of ARP Request.
     *
     * @param src       A source MAC address
     * @param dst       A destination MAC address
     * @param sender    A sender address
     * @param target    A target address
     * @param vlan      specify val ID. if vlan < 0, vlan tag is not added.
     * @param nc        A node connector
     * @param arptype   ARP.REQUEST or ARP.REPLY. (ARP Reply is not implemented yet )
     * @return  A {@link PacketContext}.
     */
    protected RawPacket createARPRawPacket (byte[] src, byte[] dst, byte[] sender, byte [] target, short vlan, NodeConnector nc, short arptype) {
        return createRawPacket(createARPPacket(src, dst, sender, target, vlan, arptype), nc);
    }

    /**
     * Detele the specified file.
     *
     * @param file  A {@link File} to be deleted.
     */
    protected static void delete(File file) {
        File[] list = file.listFiles();
        if (list != null) {
            for (File f: list) {
                delete(f);
            }
        }
        file.delete();
    }
}
