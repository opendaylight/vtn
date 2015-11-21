/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.vnode;

import static org.opendaylight.vtn.manager.internal.util.vnode.VBridgeIdentifier.getMacEntryPath;
import static org.opendaylight.vtn.manager.internal.vnode.VBridgeEntity.LOG;

import java.util.List;

import org.opendaylight.vtn.manager.packet.Ethernet;
import org.opendaylight.vtn.manager.util.EtherAddress;
import org.opendaylight.vtn.manager.util.Ip4Network;

import org.opendaylight.vtn.manager.internal.TxContext;
import org.opendaylight.vtn.manager.internal.VTNConfig;
import org.opendaylight.vtn.manager.internal.packet.PacketOutQueue;
import org.opendaylight.vtn.manager.internal.util.MiscUtils;
import org.opendaylight.vtn.manager.internal.util.inventory.SalPort;
import org.opendaylight.vtn.manager.internal.util.packet.ArpPacketBuilder;
import org.opendaylight.vtn.manager.internal.util.vnode.BridgeIdentifier;

import org.opendaylight.controller.md.sal.binding.api.ReadWriteTransaction;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.mac.rev150907.vtn.mac.table.entry.MacTableEntry;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.mac.rev150907.vtn.mac.table.entry.MacTableEntryBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.vbridge.list.Vbridge;

import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.inet.types.rev100924.IpAddress;

/**
 * {@code MacEntryAger} describes a task that ages MAC address table entries
 * by scanning MAC address table.
 *
 * <p>
 *   Unsed entries will be removed from MAC address table.
 * </p>
 */
public final class MacEntryAger extends MacEntryWalker {
    /**
     * The sender hardware address.
     */
    private final EtherAddress  senderAddress;

    /**
     * A boolean value that determines whether to enable host tracking.
     */
    private final boolean  hostTracking;

    /**
     * Construct a new instance.
     *
     * @param ctx  A runtime context for transaction task.
     */
    public MacEntryAger(TxContext ctx) {
        VTNConfig vcfg = ctx.getProvider().getVTNConfig();
        senderAddress = vcfg.getControllerMacAddress();
        hostTracking = vcfg.isHostTracking();
    }

    /**
     * Age the given MAC address table entry.
     *
     * @param ctx    A runtime context for transaction task.
     * @param ident  The identifier for the target vBridge.
     * @param path   The path to the target MAC address table entry.
     * @param ment   A {@link MacTableEntry} instance.
     */
    private void age(TxContext ctx, BridgeIdentifier<Vbridge> ident,
                     InstanceIdentifier<MacTableEntry> path,
                     MacTableEntry ment) {
        // Clear the used flag.
        MacTableEntry newEnt = new MacTableEntryBuilder(ment).
            setUsed(null).build();
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        ReadWriteTransaction tx = ctx.getReadWriteTransaction();
        tx.put(oper, path, newEnt, false);

        if (LOG.isDebugEnabled()) {
            LOG.debug("{}: MAC address has been aged: {}", ident, newEnt);
        }

        if (hostTracking) {
            // Send a packet for probing the given host.
            probe(ident, ctx.getSpecific(PacketOutQueue.class), ment);
        }
    }

    /**
     * Send a packet for probing the host specified by the given MAC address
     * table entry.
     *
     * @param ident   The identifier for the target vBridge.
     * @param outq    An {@link PacketOutQueue} instance associated with the
     *                current MD-SAL datastore transaction.
     * @param ment    A {@link MacTableEntry} instance.
     */
    private void probe(BridgeIdentifier<Vbridge> ident, PacketOutQueue outq,
                       MacTableEntry ment) {
        List<IpAddress> addrs = ment.getIpAddresses();
        if (!MiscUtils.isEmpty(addrs)) {
            SalPort egress = SalPort.create(ment);
            int vid = ment.getVlanId().intValue();
            EtherAddress tha = new EtherAddress(ment.getMacAddress());
            for (IpAddress ip: addrs) {
                probe(ident, outq, egress, vid, tha, ip);
            }
        }
    }

    /**
     * Send a packet for probing the specified host.
     *
     * @param ident   The identifier for the target vBridge.
     * @param outq    An {@link PacketOutQueue} instance associated with the
     *                current MD-SAL datastore transaction.
     * @param egress  A {@link SalPort} instance that specifies the egress
     *                switch port.
     * @param vid     A VLAN ID to be set in an ARP request.
     * @param tha     The target hardware address.
     * @param tpa     The target protocol address.
     */
    private void probe(BridgeIdentifier<Vbridge> ident, PacketOutQueue outq,
                       SalPort egress, int vid, EtherAddress tha,
                       IpAddress tpa) {
        if (tpa != null) {
            Ip4Network ip4 = Ip4Network.create(tpa.getIpv4Address());
            if (ip4 != null) {
                // Send an ARP request.
                if (LOG.isTraceEnabled()) {
                    LOG.trace("{}: Sending an ARP request: mac={}, ip={}, " +
                              "vid={}, port={}", ident, tha.getText(),
                              ip4.getText(), vid, egress);
                }
                Ethernet ether = new ArpPacketBuilder(vid).
                    build(senderAddress, tha, ip4);
                outq.enqueue(egress, ether);
            }
        }
    }

    // MacEntryWalker
    /**
     * Invoke when a MAC address table entry is detected.
     *
     * @param ctx     A runtime context for transaction task.
     * @param ident   The identifier for the target vBridge.
     * @param ment    A {@link MacTableEntry} instance.
     * @throws VTNException  An error occurred.
     */
    @Override
    protected void found(TxContext ctx, BridgeIdentifier<Vbridge> ident,
                         MacTableEntry ment) {
        InstanceIdentifier<MacTableEntry> path =
            getMacEntryPath(ident, ment.getMacAddress());
        Boolean used = ment.isUsed();
        if (used != null && used.booleanValue()) {
            // Age the given entry.
            age(ctx, ident, path, ment);
        } else {
            // Aged out the given entry.
            LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
            ReadWriteTransaction tx = ctx.getReadWriteTransaction();
            tx.delete(oper, path);
            if (LOG.isDebugEnabled()) {
                LOG.debug("{}: MAC address has been aged out: {}",
                          ident, ment);
            }
        }
    }
}
