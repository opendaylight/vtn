/*
 * Copyright (c) 2015, 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.vnode;

import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.verifyNoMoreInteractions;
import static org.mockito.Mockito.when;

import java.util.ArrayList;
import java.util.Collection;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

import org.junit.Test;

import org.opendaylight.vtn.manager.util.EtherAddress;

import org.opendaylight.vtn.manager.internal.FlowRemover;
import org.opendaylight.vtn.manager.internal.TxContext;
import org.opendaylight.vtn.manager.internal.flow.remove.FlowRemoverQueue;
import org.opendaylight.vtn.manager.internal.util.inventory.PortVlan;
import org.opendaylight.vtn.manager.internal.util.inventory.SalPort;
import org.opendaylight.vtn.manager.internal.util.vnode.VBridgeIdentifier;
import org.opendaylight.vtn.manager.internal.util.vnode.VlanMapIdentifier;

import org.opendaylight.vtn.manager.internal.LoopIterator;
import org.opendaylight.vtn.manager.internal.TestBase;

import org.opendaylight.controller.md.sal.binding.api.ReadWriteTransaction;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeName;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.mac.rev150907.MacTables;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.mac.rev150907.mac.tables.TenantMacTable;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.mac.rev150907.mac.tables.TenantMacTableBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.mac.rev150907.mac.tables.TenantMacTableKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.mac.rev150907.vtn.mac.table.entry.MacTableEntry;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.mac.rev150907.vtn.mac.table.entry.MacTableEntryBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.mac.rev150907.vtn.mac.table.entry.MacTableEntryKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.mac.rev150907.vtn.mac.table.list.MacAddressTable;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.mac.rev150907.vtn.mac.table.list.MacAddressTableKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.mac.rev150907.vtn.mac.table.list.MacAddressTableBuilder;

/**
 * JUnit test for {@link PortMapCleaner}.
 */
public class PortMapCleanerTest extends TestBase{
    /**
     * Test case for {@link PortMapCleaner#add(PortVlan)} and
     * {@link PortMapCleaner#purge(TxContext, String)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testPurge() throws Exception {
        SalPort targetPort = new SalPort(2L, 3L);
        int targetVid = 987;
        PortVlan target = new PortVlan(targetPort, targetVid);

        Set<SalPort> portSet = new HashSet<>();
        for (long dpid = 1L; dpid <= 4; dpid++) {
            for (long pnum = 1L; pnum <= 5L; pnum++) {
                assertTrue(portSet.add(new SalPort(dpid, pnum)));
            }
        }
        assertTrue(portSet.contains(targetPort));

        int[] vidArray = {0, 1, 4, 345, targetVid, 2385, 4095};
        Set<Integer> vidSet = new HashSet<>();
        for (int vid: vidArray) {
            assertTrue(vidSet.add(vid));
        }

        // Create MAC address tables for test.
        LoopIterator<SalPort> ports = new LoopIterator<>(portSet);
        LoopIterator<Integer> vids = new LoopIterator<>(vidSet);
        List<MacAddressTable> tables = new ArrayList<>();
        List<InstanceIdentifier<MacTableEntry>> filtered = new ArrayList<>();
        long baseMac = 12345L;
        String tname = "vtn_1";
        VnodeName vtnName = new VnodeName(tname);
        for (int i = 1; i <= 4; i++) {
            String bname = "vbr_" + i;
            VBridgeIdentifier vbrId =
                new VBridgeIdentifier(vtnName, new VnodeName(bname));
            tables.add(createMacTable(vbrId, ports, vids, baseMac, target,
                                      filtered));
            baseMac += 0x100000000L;
        }
        assertFalse(filtered.isEmpty());

        TenantMacTable table = new TenantMacTableBuilder().
            setName(tname).
            setMacAddressTable(tables).
            build();

        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        TxContext ctx = mock(TxContext.class);

        // MAC address table for the given VTN must be scanned.
        InstanceIdentifier<TenantMacTable> tpath = InstanceIdentifier.
            builder(MacTables.class).
            child(TenantMacTable.class, new TenantMacTableKey(tname)).
            build();
        ReadWriteTransaction tx = mock(ReadWriteTransaction.class);
        when(tx.read(oper, tpath)).thenReturn(getReadResult(table));
        when(ctx.getReadWriteTransaction()).thenReturn(tx);

        FlowRemoverQueue frq = new FlowRemoverQueue(ctx);
        verify(ctx).addPostSubmitHook(frq);
        when(ctx.getSpecific(FlowRemoverQueue.class)).thenReturn(frq);

        PortMapCleaner cleaner = new PortMapCleaner();
        for (int i = 0; i < 4; i++) {
            // The target should be added only one time.
            cleaner.add(target);
        }

        cleaner.purge(ctx, tname);

        // MAC addresses detected on the target VLAN should be removed.
        verify(ctx, times(filtered.size() + 1)).getReadWriteTransaction();
        verify(tx).read(oper, tpath);

        for (InstanceIdentifier<MacTableEntry> path: filtered) {
            verify(tx).delete(oper, path);
        }

        // Flow entries on the target VLAN should be removed.
        verify(ctx).getSpecific(FlowRemoverQueue.class);
        Collection<FlowRemover> removers = frq.getRequests();
        assertEquals(1, removers.size());
        FlowRemover fr = removers.iterator().next();
        String desc = String.format("edge-port[port=%s, vlan=%s]",
                                    targetPort, targetVid);
        assertEquals(desc, fr.getDescription());

        verifyNoMoreInteractions(ctx, tx);
    }

    /**
     * Create a MAC address table for the given vBridge.
     *
     * @param vbrId     The identifier for the vBridge.
     * @param ports     A collection of physical switch ports.
     * @param vids      A collection of VLAN IDs.
     * @param baseMac   The base MAC address.
     * @param target    A {@link PortVlan} instance that specifies the target
     *                  VLAN.
     * @param filtered  A list to store instance identifiers for MAC address
     *                  table entries to be filtered out.
     * @return  A {@link MacAddressTable} instance.
     */
    private MacAddressTable createMacTable(
        VBridgeIdentifier vbrId, LoopIterator<SalPort> ports,
        LoopIterator<Integer> vids, long baseMac, PortVlan target,
        List<InstanceIdentifier<MacTableEntry>> filtered) {
        List<MacTableEntry> entries = new ArrayList<>();
        long mac = baseMac;

        for (int i = 0; i < 40; i++) {
            SalPort sport = ports.next();
            Integer vid = vids.next();
            entries.add(createMacTableEntry(vbrId, target, mac, sport, vid,
                                            filtered));
            mac++;
        }

        // Create an entry to be filtered out.
        entries.add(createMacTableEntry(vbrId, target, mac, target.getPort(),
                                        target.getVlanId(), filtered));

        return new MacAddressTableBuilder().
            setName(vbrId.getBridgeNameString()).
            setMacTableEntry(entries).
            build();
    }

    /**
     * Create a new MAC address table entry.
     *
     * @param vbrId     The identifier for the vBridge.
     * @param target    A {@link PortVlan} instance that specifies the target
     *                  VLAN.
     * @param mac       A long value that indicates the MAC address.
     * @param sport     A {@link SalPort} instance that specifies the physical
     *                  switch port.
     * @param vid       The VLAN ID.
     * @param filtered  A list to store instance identifiers for MAC address
     *                  table entries to be filtered out.
     * @return  A {@link MacTableEntry} instance.
     */
    private MacTableEntry createMacTableEntry(
        VBridgeIdentifier vbrId, PortVlan target, long mac, SalPort sport,
        Integer vid, List<InstanceIdentifier<MacTableEntry>> filtered) {
        EtherAddress eaddr = new EtherAddress(mac);
        VlanMapIdentifier mapId = new VlanMapIdentifier(vbrId, "ANY." + vid);
        MacTableEntry mtent = new MacTableEntryBuilder().
            setEntryData(mapId.toString()).
            setIpProbeCount(0).
            setMacAddress(eaddr.getMacAddress()).
            setNode(sport.getNodeId()).
            setPortId(String.valueOf(sport.getPortNumber())).
            setVlanId(vid).
            build();

        if (sport.equals(target.getPort()) &&
            target.getVlanId() == vid.intValue()) {
            TenantMacTableKey tkey = new TenantMacTableKey(
                vbrId.getTenantNameString());
            MacAddressTableKey bkey = new MacAddressTableKey(
                vbrId.getBridgeNameString());
            MacTableEntryKey key = new MacTableEntryKey(mtent.getMacAddress());
            InstanceIdentifier<MacTableEntry> path = InstanceIdentifier.
                builder(MacTables.class).
                child(TenantMacTable.class, tkey).
                child(MacAddressTable.class, bkey).
                child(MacTableEntry.class, key).
                build();
            filtered.add(path);
        }

        return mtent;
    }
}
