/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.vnode;

import java.util.HashMap;
import java.util.Map;

import com.google.common.base.Optional;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.TxContext;
import org.opendaylight.vtn.manager.internal.TxTask;
import org.opendaylight.vtn.manager.internal.util.tx.AbstractPreSubmitHook;
import org.opendaylight.vtn.manager.internal.util.vnode.TenantNodeIdentifier;
import org.opendaylight.vtn.manager.internal.util.vnode.VBridgeIdentifier;
import org.opendaylight.vtn.manager.internal.util.vnode.VNodeType;
import org.opendaylight.vtn.manager.internal.util.vnode.VTerminalIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.vbridge.list.Vbridge;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vterminal.rev150907.vtn.vterminal.list.Vterminal;

/**
 * Helper class to read virtual node in the VTN tree using the specified
 * MD-SAL datastore transaction.
 *
 * <p>
 *   An instance of this class must be obtained by
 *   {@link TxContext#getSpecific(Class)}.
 * </p>
 */
public final class VNodeReader extends AbstractPreSubmitHook {
    /**
     * The order of the pre-submit hook.
     */
    private static final int  HOOK_ORDER = 10;

    /**
     * Cache for vBridges.
     */
    private final Map<VBridgeIdentifier, VBridge>  vBridges = new HashMap<>();

    /**
     * Cache for vTerminals.
     */
    private final Map<VTerminalIdentifier, VTerminal>  vTerminals =
        new HashMap<>();

    /**
     * Construct a new instance.
     *
     * @param ctx  A runtime context for transaction task.
     */
    public VNodeReader(TxContext ctx) {
        super(ctx);
    }

    /**
     * Return the virtual bridge that contains the specified virtual node.
     *
     * @param ident  A {@link TenantNodeIdentifier} instance that specifies the
     *               virtual node in the VTN.
     * @return  A {@link VirtualBridge} instance on success.
     *          {@code null} if the specified virtual bridge is not present.
     * @throws VTNException  An error occurred.
     */
    public VirtualBridge<?> getBridge(TenantNodeIdentifier<?, ?> ident)
        throws VTNException {
        VNodeType btype = ident.getType().getBridgeType();
        VirtualBridge<?> bridge;
        if (btype == VNodeType.VBRIDGE) {
            @SuppressWarnings("unchecked")
            VBridgeIdentifier vbrId =
                (VBridgeIdentifier)ident.getBridgeIdentifier();
            bridge = getVBridge(vbrId);
        } else if (btype == VNodeType.VTERMINAL) {
            @SuppressWarnings("unchecked")
            VTerminalIdentifier vtermId =
                (VTerminalIdentifier)ident.getBridgeIdentifier();
            bridge = getVTerminal(vtermId);
        } else {
            bridge = null;
        }

        return bridge;
    }

    /**
     * Return the specified vBridge.
     *
     * @param vbrId  A {@link VBridgeIdentifier} instance which specifies the
     *               vBridge.
     * @return  A {@link VBridge} instance on success.
     *          {@code null} if the specified vBridge is not present.
     * @throws VTNException  An error occurred.
     */
    public VBridge getVBridge(VBridgeIdentifier vbrId) throws VTNException {
        VBridge vbr = vBridges.get(vbrId);
        if (vbr == null && !vBridges.containsKey(vbrId)) {
            TxContext ctx = getContext();
            Optional<Vbridge> opt = vbrId.read(ctx.getTransaction());
            vbr = (opt.isPresent()) ? new VBridge(vbrId, opt.get()) : null;
            vBridges.put(vbrId, vbr);
        }

        return vbr;
    }

    /**
     * Return the specified vTerminal.
     *
     * @param vtmId  A {@link VTerminalIdentifier} instance which specifies
     *               the vTerminal.
     * @return  A {@link VTerminal} instance on success.
     *          {@code null} if the specified vTerminal is not present.
     * @throws VTNException  An error occurred.
     */
    public VTerminal getVTerminal(VTerminalIdentifier vtmId)
        throws VTNException {
        VTerminal vtm = vTerminals.get(vtmId);
        if (vtm == null && !vTerminals.containsKey(vtmId)) {
            TxContext ctx = getContext();
            Optional<Vterminal> opt = vtmId.read(ctx.getTransaction());
            vtm = (opt.isPresent()) ? new VTerminal(vtmId, opt.get()) : null;
            vTerminals.put(vtmId, vtm);
        }

        return vtm;
    }

    // TxHook

    /**
     * Invoked when the MD-SAL datastore transaction is going to be submitted.
     *
     * <p>
     *   This method puts all modified virtual nodes to the transaction.
     * </p>
     *
     * @param ctx   A runtime context for transaction task.
     * @param task  The current transaction task.
     */
    @Override
    public void run(TxContext ctx, TxTask<?> task) {
        // Submit changes to the virtual bridge status.
        for (VBridge vbr: vBridges.values()) {
            vbr.submit(ctx);
        }

        for (VTerminal vterm: vTerminals.values()) {
            vterm.submit(ctx);
        }
    }

    /**
     * {@inheritDoc}
     */
    public int getOrder() {
        return HOOK_ORDER;
    }
}
