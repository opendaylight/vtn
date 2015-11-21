/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.vnode;

import static org.opendaylight.vtn.manager.internal.util.vnode.VBridgeIdentifier.getMacEntryPath;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.TxContext;
import org.opendaylight.vtn.manager.internal.util.DataStoreUtils;
import org.opendaylight.vtn.manager.internal.util.vnode.BridgeIdentifier;
import org.opendaylight.vtn.manager.internal.util.vnode.mac.MacEntryFilter;

import org.opendaylight.controller.md.sal.binding.api.ReadWriteTransaction;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.mac.rev150907.vtn.mac.table.entry.MacTableEntry;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.vbridge.list.Vbridge;

import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.yang.types.rev100924.MacAddress;

/**
 * {@code MacEntryRemover} describes a task that removes MAC address table
 * entries by scanning MAC address tables.
 */
public final class MacEntryRemover extends MacEntryWalker {
    /**
     * Logger instance.
     */
    private static final Logger  LOG =
        LoggerFactory.getLogger(MacEntryRemover.class);

    /**
     * A filter that selects MAC address table entries to be removed.
     */
    private final MacEntryFilter  macFilter;

    /**
     * Remove the specified MAC address from the MAC address table associated
     * with the specified vBridge.
     *
     * @param ctx     A runtime context for transaction task.
     * @param ident   The identifier for the target vBridge.
     * @param mac     A MAC address.
     * @throws VTNException  An error occurred.
     */
    public static void remove(TxContext ctx, BridgeIdentifier<Vbridge> ident,
                              MacAddress mac) throws VTNException {
        InstanceIdentifier<MacTableEntry> path = getMacEntryPath(ident, mac);
        ReadWriteTransaction tx = ctx.getReadWriteTransaction();
        MacTableEntry ment = DataStoreUtils.delete(tx, path);
        if (ment != null && LOG.isTraceEnabled()) {
            LOG.trace("{}: MAC address removed: entry={}", ident, ment);
        }
    }

    /**
     * Construct a new instance.
     *
     * @param filter  A {@link MacEntryFilter} that selects MAC address table
     *                entries to be removed.
     */
    public MacEntryRemover(MacEntryFilter filter) {
        macFilter = filter;
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
                         MacTableEntry ment) throws VTNException {
        if (macFilter.accept(ment)) {
            // Remove the specified entry.
            InstanceIdentifier<MacTableEntry> path =
                getMacEntryPath(ident, ment.getMacAddress());
            LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
            ReadWriteTransaction tx = ctx.getReadWriteTransaction();
            tx.delete(oper, path);
            if (LOG.isTraceEnabled()) {
                LOG.trace("{}: MAC address removed: filter={}, entry={}",
                          ident, macFilter.getClass().getSimpleName(), ment);
            }
        }
    }
}
