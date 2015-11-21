/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.vnode;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.TxContext;
import org.opendaylight.vtn.manager.internal.flow.remove.TenantFlowRemover;
import org.opendaylight.vtn.manager.internal.flow.remove.VNodeFlowRemover;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcUtils;
import org.opendaylight.vtn.manager.internal.util.vnode.BridgeIdentifier;
import org.opendaylight.vtn.manager.internal.util.vnode.VBridgeIfIdentifier;
import org.opendaylight.vtn.manager.internal.util.vnode.VInterfaceIdentifier;
import org.opendaylight.vtn.manager.internal.util.vnode.VTerminalIfIdentifier;

import org.opendaylight.controller.md.sal.binding.api.ReadWriteTransaction;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.VtnPortMappableBridge;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.vbridge.list.Vbridge;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.RemoveVinterfaceInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.vtn.mappable.vinterface.list.Vinterface;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vterminal.rev150907.vtn.vterminal.list.Vterminal;

/**
 * {@code RemoveVinterfaceTask} describes the MD-SAL datastore transaction task
 * that removes the specified virtual interface.
 *
 * @param <B>  The type of the virtual bridge that contains the target
 *             virtual interface.
 * @see  #create(RemoveVinterfaceInput)
 */
public final class RemoveVinterfaceTask<B extends VtnPortMappableBridge>
    extends RemoveVirtualNodeTask<Vinterface, VInterfaceIdentifier<B>> {
    /**
     * Construct a new task that removes the specified virtual interface.
     *
     * @param input  A {@link RemoveVinterfaceInput} instance.
     * @return  A {@link RemoveVbridgeTask} instance.
     * @throws RpcException
     *     The given input contains invalid value.
     */
    public static RemoveVinterfaceTask<?> create(RemoveVinterfaceInput input)
        throws RpcException {
        if (input == null) {
            throw RpcUtils.getNullInputException();
        }

        // Determine the target virtual interface.
        String tname = input.getTenantName();
        String bname = input.getBridgeName();
        String iname = input.getInterfaceName();
        RemoveVinterfaceTask<?> task;
        if (bname == null) {
            String tmname = input.getTerminalName();
            VTerminalIfIdentifier ifId = VTerminalIfIdentifier.
                create(tname, tmname, iname, true);
            task = new RemoveVinterfaceTask<Vterminal>(ifId);
        } else {
            VBridgeIfIdentifier ifId = VBridgeIfIdentifier.
                create(tname, bname, iname, true);
            task = new RemoveVinterfaceTask<Vbridge>(ifId);
        }

        return task;
    }

    /**
     * Construct a new instance.
     *
     * @param ident  The identifier for the target virtual interface.
     */
    private RemoveVinterfaceTask(VInterfaceIdentifier<B> ident) {
        super(ident);
    }

    // RemoveVirtualNodeTask

    /**
     * {@inheritDoc}
     */
    @Override
    protected void destroy(TxContext ctx, Vinterface current)
        throws VTNException {
        // Destroy the specified virtual interface.
        VInterfaceIdentifier<B> ident = getIdentifier();
        VInterface<B> vif = VInterface.create(ident, current);
        boolean noFilter = vif.isFilterEmpty();
        vif.destroy(ctx, true);

        if (noFilter) {
            // Purge all the VTN flows established by the removed interface.
            setFlowRemover(new VNodeFlowRemover(ident));
        } else {
            // REVISIT: Select flow entries affected by obsolete flow filters.
            setFlowRemover(new TenantFlowRemover(ident));
        }
    }

    // DeleteDataTask

    /**
     * Invoked when the target data object has been deleted.
     *
     * @param ctx  A runtime context for transaction task.
     * @throws VTNException  An error occurred.
     */
    @Override
    protected void onDeleted(TxContext ctx) throws VTNException {
        // Update the status of the parent virtual bridge.
        VInterfaceIdentifier<B> ident = getIdentifier();
        BridgeIdentifier<B> parentId = ident.getBridgeIdentifier();
        ReadWriteTransaction tx = ctx.getReadWriteTransaction();
        VirtualBridge<B> bridge = VirtualBridge.create(tx, parentId);
        bridge.putState(ctx);
    }
}
