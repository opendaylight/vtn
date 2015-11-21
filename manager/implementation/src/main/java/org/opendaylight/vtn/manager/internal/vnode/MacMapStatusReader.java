/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.vnode;

import java.util.HashMap;
import java.util.Map.Entry;
import java.util.Map;

import com.google.common.base.Optional;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.TxContext;
import org.opendaylight.vtn.manager.internal.TxTask;
import org.opendaylight.vtn.manager.internal.util.DataStoreUtils;
import org.opendaylight.vtn.manager.internal.util.tx.AbstractPreSubmitHook;
import org.opendaylight.vtn.manager.internal.util.vnode.MacMapIdentifier;
import org.opendaylight.vtn.manager.internal.util.vnode.VTNMacMapStatus;

import org.opendaylight.controller.md.sal.binding.api.ReadWriteTransaction;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.mac.rev150907.vtn.mac.map.info.MacMapStatus;

/**
 * Helper class to read runtime information of MAC mapping in the VTN tree
 * using the specified MD-SAL datastore transaction.
 *
 * <p>
 *   An instance of this class must be obtained by
 *   {@link TxContext#getSpecific(Class)}.
 * </p>
 */
public final class MacMapStatusReader extends AbstractPreSubmitHook {
    /**
     * The order of the pre-submit hook.
     */
    private static final int  HOOK_ORDER = 0;

    /**
     * Cache for status of MAC mappings.
     */
    private final Map<MacMapIdentifier, VTNMacMapStatus> statusMap =
        new HashMap<>();

    /**
     * Construct a new instance.
     *
     * @param ctx  A runtime context for transaction task.
     */
    public MacMapStatusReader(TxContext ctx) {
        super(ctx);
    }

    /**
     * Return the runtime status of the given MAC mapping.
     *
     * @param ident  A {@link MacMapIdentifier} instance that specifies the
     *               target MAC mapping.
     * @return  A {@link VTNMacMapStatus} instance.
     * @throws VTNException  An error occurred.
     */
    public VTNMacMapStatus get(MacMapIdentifier ident) throws VTNException {
        VTNMacMapStatus vmst = statusMap.get(ident);
        if (vmst == null) {
            vmst = read(ident);
            statusMap.put(ident, vmst);
        }

        return vmst;
    }

    /**
     * Return the runtime status of the given MAC mapping only if it is cached
     * in this instance.
     *
     * @param ident  A {@link MacMapIdentifier} instance that specifies the
     *               target MAC mapping.
     * @return  A {@link VTNMacMapStatus} instance if the status of the
     *          specified MAC mapping is cached in this instance.
     *          {@code null} if not cached.
     */
    public VTNMacMapStatus getCached(MacMapIdentifier ident) {
        return statusMap.get(ident);
    }

    /**
     * Put the given MAC mapping status only if not cached.
     *
     * @param ident  A {@link MacMapIdentifier} instance that specifies the
     *               target MAC mapping.
     * @param vmst   A {@link VTNMacMapStatus} instance that represents the
     *               status of the MAC mapping specified by {@code ident}.
     * @return  A {@link VTNMacMapStatus} instance actually cached in this
     *          instance.
     */
    public VTNMacMapStatus put(MacMapIdentifier ident, VTNMacMapStatus vmst) {
        VTNMacMapStatus value = statusMap.get(ident);
        if (value == null) {
            statusMap.put(ident, vmst);
            value = vmst;
        }

        return value;
    }

    /**
     * Discard the status information about the specified MAC mapping.
     *
     * @param ident  A {@link MacMapIdentifier} instance that specifies the
     *               target MAC mapping.
     * @return  If actually removed, the removed {@link VTNMacMapStatus}
     *          instance is returned. {@code null} is returned if the status
     *          information for the specified MAC mapping is not present in
     *          this instance.
     */
    public VTNMacMapStatus remove(MacMapIdentifier ident) {
        return statusMap.remove(ident);
    }

    /**
     * Discard all the cached instances.
     */
    public void clear() {
        statusMap.clear();
    }

    /**
     * Read the status information about the specified MAC mapping from the
     * MD-SAL datastore.
     *
     * @param ident  A {@link MacMapIdentifier} instance that specifies the
     *               target MAC mapping.
     * @return  A {@link VTNMacMapStatus} instance.
     * @throws VTNException  An error occurred.
     */
    private VTNMacMapStatus read(MacMapIdentifier ident) throws VTNException {
        ReadWriteTransaction tx = getContext().getReadWriteTransaction();
        Optional<MacMapStatus> opt =
            DataStoreUtils.read(tx, ident.getStatusPath());
        if (opt.isPresent()) {
            return new VTNMacMapStatus(opt.get());
        }

        // This should never happen.
        throw ident.getNotFoundException();
    }

    // TxHook

    /**
     * Invoked when the MD-SAL datastore transaction is going to be submitted.
     *
     * <p>
     *   This method puts all modified MAC mapping status to the transaction.
     * </p>
     *
     * @param ctx   A runtime context for transaction task.
     * @param task  The current transaction task.
     */
    @Override
    public void run(TxContext ctx, TxTask<?> task) {
        ReadWriteTransaction tx = ctx.getReadWriteTransaction();
        for (Entry<MacMapIdentifier, VTNMacMapStatus> entry:
                 statusMap.entrySet()) {
            VTNMacMapStatus vmst = entry.getValue();
            vmst.submit(tx, entry.getKey());
        }
    }

    /**
     * {@inheritDoc}
     */
    public int getOrder() {
        return HOOK_ORDER;
    }
}
