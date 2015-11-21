/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.vnode;

import static org.opendaylight.vtn.manager.internal.util.vnode.VBridgeIdentifier.getTenantMacTablePath;

import java.util.List;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.TxContext;
import org.opendaylight.vtn.manager.internal.flow.remove.TenantFlowRemover;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcUtils;
import org.opendaylight.vtn.manager.internal.util.vnode.VBridgeIdentifier;
import org.opendaylight.vtn.manager.internal.util.vnode.VTenantIdentifier;
import org.opendaylight.vtn.manager.internal.util.vnode.VTerminalIdentifier;

import org.opendaylight.controller.md.sal.binding.api.ReadWriteTransaction;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.RemoveVtnInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.vtns.Vtn;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.mac.rev150907.mac.tables.TenantMacTable;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.vbridge.list.Vbridge;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vterminal.rev150907.vtn.vterminal.list.Vterminal;

/**
 * {@code RemoveVtnTask} describes the MD-SAL datastore transaction task
 * that removes the specified VTN.
 *
 * @see  #create(RemoveVtnInput)
 */
public final class RemoveVtnTask
    extends RemoveVirtualNodeTask<Vtn, VTenantIdentifier> {
    /**
     * Create a new task that removes the specified VTN.
     *
     * @param input  A {@link RemoveVtnInput} instance.
     * @return  A {@link RemoveVtnTask} instance.
     * @throws RpcException
     *     The given input contains invalid value.
     */
    public static RemoveVtnTask create(RemoveVtnInput input)
        throws RpcException {
        if (input == null) {
            throw RpcUtils.getNullInputException();
        }

        VTenantIdentifier vtnId = VTenantIdentifier.create(
            input.getTenantName(), true);

        return new RemoveVtnTask(vtnId);
    }

    /**
     * Construct a new instance.
     *
     * @param ident  An identifier for the target VTN.
     */
    private RemoveVtnTask(VTenantIdentifier ident) {
        super(ident);
    }

    // RemoveVirtualNodeTask

    /**
     * {@inheritDoc}
     */
    @Override
    protected void destroy(TxContext ctx, Vtn current) throws VTNException {
        // Destroy all the vBridges.
        VTenantIdentifier vtnId = getIdentifier();
        List<Vbridge> vbrs = current.getVbridge();
        if (vbrs != null) {
            for (Vbridge vb: vbrs) {
                VBridgeIdentifier vbrId = new VBridgeIdentifier(
                    vtnId, vb.getName());
                VBridge vbr = new VBridge(vbrId, vb);
                vbr.destroy(ctx, false);
            }
        }

        // Destroy all the vTerminals.
        List<Vterminal> vtms = current.getVterminal();
        if (vtms != null) {
            for (Vterminal vt: vtms) {
                VTerminalIdentifier vtermId = new VTerminalIdentifier(
                    vtnId, vt.getName());
                VTerminal vterm = new VTerminal(vtermId, vt);
                vterm.destroy(ctx, false);
            }
        }

        // Delete a list of MAC address tables for the target VTN.
        ReadWriteTransaction tx = ctx.getReadWriteTransaction();
        InstanceIdentifier<TenantMacTable> mpath =
            getTenantMacTablePath(vtnId);
        tx.delete(getDatastoreType(), mpath);

        // Remove all the flow entries in the removed VTN.
        String tname = vtnId.getTenantNameString();
        setFlowRemover(new TenantFlowRemover(tname));
    }
}
