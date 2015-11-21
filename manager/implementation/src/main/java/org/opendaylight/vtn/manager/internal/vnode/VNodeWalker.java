/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.vnode;

import java.util.List;

import com.google.common.base.Optional;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.TxContext;
import org.opendaylight.vtn.manager.internal.util.DataStoreUtils;
import org.opendaylight.vtn.manager.internal.util.vnode.VBridgeIdentifier;
import org.opendaylight.vtn.manager.internal.util.vnode.VTerminalIdentifier;

import org.opendaylight.controller.md.sal.binding.api.ReadTransaction;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.Vtns;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.vtns.Vtn;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeName;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.vbridge.list.Vbridge;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vterminal.rev150907.vtn.vterminal.list.Vterminal;

/**
 * {@code VNodeWalker} describes a VTN task that walks all the virtual nodes
 * in the VTN model tree.
 *
 * @param <T>  The type of arbitrary data to be passed to virtual nodes.
 */
public abstract class VNodeWalker<T> {
    /**
     * Construct a new instance.
     */
    protected VNodeWalker() {
    }

    /**
     * Scan all the virtual nodes in the VTN model tree.
     *
     * @param ctx   A runtime context for transaction task.
     * @param data  An arbitrary data to be passed to virtual nodes.
     * @throws VTNException  An error occurred.
     */
    public final void scan(TxContext ctx, T data) throws VTNException {
        // Read the root container of the VTNs.
        InstanceIdentifier<Vtns> path = InstanceIdentifier.create(Vtns.class);
        ReadTransaction rtx = ctx.getTransaction();
        Optional<Vtns> opt = DataStoreUtils.read(rtx, path);
        if (opt.isPresent()) {
            List<Vtn> vtns = opt.get().getVtn();
            if (vtns != null) {
                for (Vtn vtn: vtns) {
                    scan(ctx, vtn, data);
                }
            }
        }
    }

    /**
     * Scan all the virtual nodes in the specified vBridge.
     *
     * @param ctx   A runtime context for transaction task.
     * @param vbr   A {@link VBridge} instance.
     * @param data  An arbitrary data to be passed to virtual nodes.
     * @throws VTNException  An error occurred.
     */
    protected abstract void scan(TxContext ctx, VBridge vbr, T data)
        throws VTNException;

    /**
     * Scan all the virtual nodes in the specified vTerminal.
     *
     * @param ctx   A runtime context for transaction task.
     * @param vtm   A {@link VTerminal} instance.
     * @param data  An arbitrary data to be passed to virtual nodes.
     * @throws VTNException  An error occurred.
     */
    protected abstract void scan(TxContext ctx, VTerminal vtm, T data)
        throws VTNException;

    /**
     * Scan all the virtual nodes in the specified VTN.
     *
     * @param ctx   A runtime context for transaction task.
     * @param vtn   A {@link Vtn} instance.
     * @param data  An arbitrary data to be passed to virtual nodes.
     * @throws VTNException  An error occurred.
     */
    private void scan(TxContext ctx, Vtn vtn, T data) throws VTNException {
        VnodeName tname = vtn.getName();
        List<Vbridge> vbridges = vtn.getVbridge();
        if (vbridges != null) {
            for (Vbridge vbridge: vbridges) {
                VnodeName bname = vbridge.getName();
                VBridgeIdentifier vbrId = new VBridgeIdentifier(tname, bname);
                scan(ctx, new VBridge(vbrId, vbridge), data);
            }
        }

        List<Vterminal> vterminals = vtn.getVterminal();
        if (vterminals != null) {
            for (Vterminal vterm: vterminals) {
                VnodeName bname = vterm.getName();
                VTerminalIdentifier vtmId =
                    new VTerminalIdentifier(tname, bname);
                scan(ctx, new VTerminal(vtmId, vterm), data);
            }
        }
    }
}
