/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.vnode;

import static org.opendaylight.vtn.manager.internal.util.vnode.VBridgeIdentifier.getMacTablePath;

import java.util.List;

import com.google.common.base.Optional;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.TxContext;
import org.opendaylight.vtn.manager.internal.util.DataStoreUtils;
import org.opendaylight.vtn.manager.internal.util.vnode.BridgeIdentifier;
import org.opendaylight.vtn.manager.internal.util.vnode.VBridgeIdentifier;

import org.opendaylight.controller.md.sal.binding.api.ReadWriteTransaction;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeName;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.mac.rev150907.MacTables;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.mac.rev150907.mac.tables.TenantMacTable;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.mac.rev150907.mac.tables.TenantMacTableKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.mac.rev150907.vtn.mac.table.entry.MacTableEntry;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.mac.rev150907.vtn.mac.table.list.MacAddressTable;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.vbridge.list.Vbridge;

/**
 * {@code MacTable} describes a task that walks all the entries in the
 * specified MAC address tables.
 */
public abstract class MacEntryWalker {
    /**
     * Construct a new instance.
     */
    protected MacEntryWalker() {
    }

    /**
     * Scan MAC address table entries in all the MAC address tables.
     *
     * @param ctx  A runtime context for transaction task.
     * @throws VTNException  An error occurred.
     */
    public final void scan(TxContext ctx) throws VTNException {
        // Read the root container of the MAC address tables.
        InstanceIdentifier<MacTables> path =
            InstanceIdentifier.create(MacTables.class);
        ReadWriteTransaction tx = ctx.getReadWriteTransaction();
        Optional<MacTables> opt = DataStoreUtils.read(tx, path);
        if (opt.isPresent()) {
            List<TenantMacTable> tenants = opt.get().getTenantMacTable();
            if (tenants != null) {
                for (TenantMacTable tenant: tenants) {
                    scan(ctx, tenant);
                }
            }
        }
    }

    /**
     * Scan MAC address table entries in all the MAC address tables in the
     * specified VTN.
     *
     * @param ctx    A runtime context for transaction task.
     * @param tname  The name of the target VTN.
     * @throws VTNException  An error occurred.
     */
    public final void scan(TxContext ctx, String tname) throws VTNException {
        // Read the tenant MAC table container.
        InstanceIdentifier<TenantMacTable> path =
            InstanceIdentifier.builder(MacTables.class).
            child(TenantMacTable.class, new TenantMacTableKey(tname)).
            build();
        ReadWriteTransaction tx = ctx.getReadWriteTransaction();
        Optional<TenantMacTable> opt = DataStoreUtils.read(tx, path);
        if (opt.isPresent()) {
            scan(ctx, opt.get());
        }
    }

    /**
     * Scan MAC address table entries in the MAC address table for the
     * specified vBridge.
     *
     * @param ctx    A runtime context for transaction task.
     * @param ident  The identifier for the target vBridge.
     * @throws VTNException  An error occurred.
     */
    public final void scan(TxContext ctx, BridgeIdentifier<Vbridge> ident)
        throws VTNException {
        // Read the MAC address table.
        InstanceIdentifier<MacAddressTable> path = getMacTablePath(ident);
        ReadWriteTransaction tx = ctx.getReadWriteTransaction();
        Optional<MacAddressTable> opt = DataStoreUtils.read(tx, path);
        if (opt.isPresent()) {
            scan(ctx, ident, opt.get());
        }
    }

    /**
     * Invoke when a MAC address table entry is detected.
     *
     * @param ctx     A runtime context for transaction task.
     * @param ident   The identifier for the target vBridge.
     * @param ment    A {@link MacTableEntry} instance.
     * @throws VTNException  An error occurred.
     */
    protected abstract void found(
        TxContext ctx, BridgeIdentifier<Vbridge> ident, MacTableEntry ment)
        throws VTNException;

    /**
     * Scan MAC address table entries in all the MAC address tables in the
     * given tenant MAC tables.
     *
     * @param ctx     A runtime context for transaction task.
     * @param tenant  A {@link TenantMacTable} instance.
     * @throws VTNException  An error occurred.
     */
    private void scan(TxContext ctx, TenantMacTable tenant)
        throws VTNException {
        VnodeName vtnName = new VnodeName(tenant.getName());
        List<MacAddressTable> mtables = tenant.getMacAddressTable();
        if (mtables != null) {
            for (MacAddressTable mtable: mtables) {
                VnodeName vbrName = new VnodeName(mtable.getName());
                VBridgeIdentifier ident =
                    new VBridgeIdentifier(vtnName, vbrName);
                scan(ctx, ident, mtable);
            }
        }
    }

    /**
     * Remove MAC addresses specified by the given filter from the given
     * MAC address table.
     *
     * @param ctx     A runtime context for transaction task.
     * @param ident   The identifier for the target vBridge.
     * @param mtable  A {@link MacAddressTable} instance.
     * @throws VTNException  An error occurred.
     */
    private void scan(TxContext ctx, BridgeIdentifier<Vbridge> ident,
                      MacAddressTable mtable) throws VTNException {
        List<MacTableEntry> entries = mtable.getMacTableEntry();
        if (entries != null) {
            for (MacTableEntry ment: entries) {
                found(ctx, ident, ment);
            }
        }
    }
}
