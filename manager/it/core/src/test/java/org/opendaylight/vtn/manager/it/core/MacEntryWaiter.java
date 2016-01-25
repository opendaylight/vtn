/*
 * Copyright (c) 2015, 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.core;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.fail;

import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Set;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import org.opendaylight.vtn.manager.util.EtherAddress;

import org.opendaylight.vtn.manager.it.ofmock.DataChangeWaiter;
import org.opendaylight.vtn.manager.it.ofmock.OfMockService;
import org.opendaylight.vtn.manager.it.util.TestHost;
import org.opendaylight.vtn.manager.it.util.vnode.VBridgeIdentifier;
import org.opendaylight.vtn.manager.it.util.vnode.mac.MacEntry;

import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.mac.rev150907.vtn.mac.table.entry.MacTableEntry;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.mac.rev150907.vtn.mac.table.list.MacAddressTable;

/**
 * {@code MacEntryWaiter} is a utility class used to wait for MAC address table
 * to be changed.
 */
public final class MacEntryWaiter {
    /**
     * Logger instance.
     */
    private static final Logger LOG = LoggerFactory.
        getLogger(MacEntryWaiter.class);

    /**
     * The number of milliseconds to wait the MAC address table to be changed.
     */
    private static final long  WAIT_TIMEOUT = 10000L;

    /**
     * ofmock service.
     */
    private final OfMockService  ofMock;

    /**
     * The target vBridge identifier.
     */
    private final VBridgeIdentifier  vBridgeId;

    /**
     * A set of expected MAC address table entries.
     */
    private Map<EtherAddress, MacEntry>  macEntries;

    /**
     * Construct a new instance.
     *
     * @param mock   ofmock service.
     * @param ident  The identifier for the target vBridge.
     */
    public MacEntryWaiter(OfMockService mock, VBridgeIdentifier ident) {
        ofMock = mock;
        vBridgeId = ident;
    }

    /**
     * Return an unmodifiable collection that contains all the MAC address
     * table entries configured in this instance.
     *
     * @return  An unmodifiable collection of {@link MacEntry} instances
     *          or {@code null}.
     */
    public Collection<MacEntry> getMacEntries() {
        return (macEntries == null)
            ? null
            : Collections.unmodifiableCollection(macEntries.values());
    }

    /**
     * Set the contents of the expected MAC address table entries.
     *
     * @param ments  A set of MAC address table entries expected to be present
     *               in the MAC address table. {@code null} indicates the
     *               MAC address table should not be present.
     * @return  This instance.
     */
    public MacEntryWaiter set(Set<MacEntry> ments) {
        macEntries = null;
        if (ments != null) {
            Map<EtherAddress, MacEntry> map = new HashMap<>();
            macEntries = map;
            for (MacEntry ment: ments) {
                EtherAddress eaddr = ment.getMacAddress();
                map.put(eaddr, ment);
            }
        }

        return this;
    }

    /**
     * Add the given MAC address table entries.
     *
     * @param ments  An array of MAC address table entries that should be
     *               present in the MAC address table.
     * @return  This instance.
     */
    public MacEntryWaiter add(MacEntry ... ments) {
        Map<EtherAddress, MacEntry> map = macEntries;
        if (map == null) {
            map = new HashMap<>();
            macEntries = map;
        }

        for (MacEntry ment: ments) {
            EtherAddress eaddr = ment.getMacAddress();
            map.put(eaddr, ment);
        }

        return this;
    }

    /**
     * Remove the given MAC address table entries.
     *
     * @param eaddrs  An array of {@link EtherAddress} instances that should
     *                not be present in the MAC address table.
     * @return  This instance.
     */
    public MacEntryWaiter remove(EtherAddress ... eaddrs) {
        Map<EtherAddress, MacEntry> map = macEntries;
        if (map != null) {
            for (EtherAddress eaddr: eaddrs) {
                map.remove(eaddr);
            }
        }

        return this;
    }

    /**
     * Remove the MAC addresses associated with the given hosts.
     *
     * @param hosts  A collection of {@link TestHost} instances.
     * @return  This instance.
     */
    public MacEntryWaiter remove(Collection<TestHost> hosts) {
        Map<EtherAddress, MacEntry> map = macEntries;
        if (map != null) {
            for (TestHost host: hosts) {
                map.remove(host.getEtherAddress());
            }
        }

        return this;
    }

    /**
     * Clear the MAC address table entries.
     *
     * @return  This instance.
     */
    public MacEntryWaiter clear() {
        clear(true);
        return this;
    }

    /**
     * Clear the MAC address table entries.
     *
     * @param present  {@code true} indicates the target vBridge is present.
     * @return  This instance.
     */
    public MacEntryWaiter clear(boolean present) {
        if (present) {
            Map<EtherAddress, MacEntry> map = macEntries;
            if (map == null) {
                macEntries = new HashMap<>();
            } else {
                map.clear();
            }
        } else {
            macEntries = null;
        }

        return this;
    }

    /**
     * Eliminate MAC address table entries associated with the given node.
     *
     * @param nid  The MD-SAL node identifier.
     * @return  This instance.
     */
    public MacEntryWaiter filterOutByNode(String nid) {
        Map<EtherAddress, MacEntry> map = macEntries;
        if (map != null) {
            for (Iterator<MacEntry> it = map.values().iterator();
                 it.hasNext();) {
                MacEntry ment = it.next();
                if (nid.equals(ment.getNodeIdentifier())) {
                    it.remove();
                }
            }
        }

        return this;
    }

    /**
     * Eliminate MAC address table entries associated with the given port.
     *
     * @param pid  The MD-SAL port identifier.
     * @return  This instance.
     */
    public MacEntryWaiter filterOut(String pid) {
        Map<EtherAddress, MacEntry> map = macEntries;
        if (map != null) {
            for (Iterator<MacEntry> it = map.values().iterator();
                 it.hasNext();) {
                MacEntry ment = it.next();
                if (pid.equals(ment.getPortIdentifier())) {
                    it.remove();
                }
            }
        }

        return this;
    }

    /**
     * Eliminate MAC address table entries associated with the given ports.
     *
     * @param pids  A set of MD-SAL port identifiers.
     * @return  This instance.
     */
    public MacEntryWaiter filterOut(Set<String> pids) {
        Map<EtherAddress, MacEntry> map = macEntries;
        if (map != null) {
            for (Iterator<MacEntry> it = map.values().iterator();
                 it.hasNext();) {
                MacEntry ment = it.next();
                if (pids.contains(ment.getPortIdentifier())) {
                    it.remove();
                }
            }
        }

        return this;
    }

    /**
     * Eliminate MAC address table entries associated with the given VLAN ID.
     *
     * @param vid  The VLAN ID.
     * @return  This instance.
     */
    public MacEntryWaiter filterOut(int vid) {
        Map<EtherAddress, MacEntry> map = macEntries;
        if (map != null) {
            for (Iterator<MacEntry> it = map.values().iterator();
                 it.hasNext();) {
                MacEntry ment = it.next();
                if (ment.getVlanId() == vid) {
                    it.remove();
                }
            }
        }

        return this;
    }

    /**
     * Wait for the target MAC address table to be changed as expected.
     *
     * @return  This instance.
     * @throws InterruptedException
     *    The calling thread was interrupted.
     */
    public MacEntryWaiter await() throws InterruptedException {
        try (DataChangeWaiter<MacAddressTable> waiter = newWaiter()) {
            verify(waiter);
        }

        return this;
    }

    /**
     * Create a new data change waiter for the target MAC address table.
     *
     * @return  A {@link DataChangeWaiter} instance.
     */
    private DataChangeWaiter<MacAddressTable> newWaiter() {
        InstanceIdentifier<MacAddressTable> path =
            VBridgeIdentifier.getMacTablePath(vBridgeId);
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        return ofMock.newDataChangeWaiter(oper, path);
    }

    /**
     * Throw an error if the condition configured in this instance is not
     * satisfied.
     *
     * @param waiter  A {@link DataChangeWaiter} instance.
     * @throws InterruptedException
     *    The calling thread was interrupted.
     */
    private void verify(DataChangeWaiter<MacAddressTable> waiter)
        throws InterruptedException {
        long limit = System.currentTimeMillis() + WAIT_TIMEOUT;

        MacAddressTable mtable = waiter.getValue();
        if (!check(mtable)) {
            LOG.trace("Waiting: path={}, now={}, limit={}, value={}",
                      vBridgeId, System.currentTimeMillis(), limit, mtable);

            while (waiter.await(limit)) {
                mtable = waiter.getValue();
                if (LOG.isTraceEnabled()) {
                    LOG.trace("Woke up: path={}, value={}", vBridgeId, mtable);
                }
                if (check(mtable)) {
                    return;
                }
            }

            verify(waiter.getValue());
        }
    }

    /**
     * Determine whether the given MAC address table satisfies the condition
     * or not.
     *
     * @param mtable  The current value of the MAC address table.
     * @return  {@code true} only if the given MAC address table satisfies
     *          the condition.
     */
    private boolean check(MacAddressTable mtable) {
        boolean ret;
        if (macEntries == null) {
            ret = (mtable == null);
        } else {
            List<MacTableEntry> entries = mtable.getMacTableEntry();
            ret = (entries == null || entries.isEmpty())
                ? macEntries.isEmpty()
                : check(entries);
        }

        return ret;
    }

    /**
     * Determine whether the given list of MAC address table entries satisfies
     * the condition or not.
     *
     * @param entries  A list of {@link MacTableEntry} instances.
     * @return  {@code true} only if the given list of MAC address table
     *          entries satisfies the condition.
     */
    private boolean check(List<MacTableEntry> entries) {
        boolean ret = (entries.size() == macEntries.size());
        if (ret) {
            Set<EtherAddress> checked = new HashSet<>();
            for (MacTableEntry ment: entries) {
                EtherAddress eaddr = new EtherAddress(ment.getMacAddress());
                assertEquals(true, checked.add(eaddr));
                MacEntry expected = macEntries.get(eaddr);
                ret = (expected != null &&
                       expected.equals(new MacEntry(ment)));
                if (!ret) {
                    break;
                }
            }
        }

        return ret;
    }

    /**
     * Verify the contents of the MAC address table.
     *
     * @param mtable  The current value of the MAC address table.
     */
    private void verify(MacAddressTable mtable) {
        if (macEntries == null) {
            if (mtable != null) {
                LOG.error("{}: Unwanted MAC address table is present.",
                          vBridgeId);
                assertEquals(null, mtable);
            }
        } else {
            List<MacTableEntry> entries = mtable.getMacTableEntry();
            if (entries == null || entries.isEmpty()) {
                String msg = "MAC address table is empty";
                LOG.error("{}: {}: expected={}", vBridgeId, msg, macEntries);
                fail(msg);
            } else {
                verify(entries);
            }
        }
    }

    /**
     * Verify the given MAC address table entries.
     *
     * @param entries  A list of {@link MacTableEntry} instances.
     */
    private void verify(List<MacTableEntry> entries) {
        List<MacTableEntry> unwanted = new ArrayList<>();
        Map<EtherAddress, MacEntry> map = new HashMap<>(macEntries);
        for (MacTableEntry ment: entries) {
            EtherAddress eaddr = new EtherAddress(ment.getMacAddress());
            MacEntry expected = map.remove(eaddr);
            if (expected == null) {
                LOG.error("{}: Unexpected MAC address: {}", vBridgeId, ment);
                assertNotNull(expected);
            } else {
                assertEquals(expected, new MacEntry(ment));
            }
        }

        if (!map.isEmpty()) {
            LOG.error("{}: Missing MAC addresses: {}", vBridgeId, map);
            assertEquals(true, map.isEmpty());
        }
    }
}
