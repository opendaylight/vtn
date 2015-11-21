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
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcOutputGenerator;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcUtils;
import org.opendaylight.vtn.manager.internal.util.tx.AbstractRpcTask;
import org.opendaylight.vtn.manager.internal.util.vnode.BridgeIdentifier;
import org.opendaylight.vtn.manager.internal.util.vnode.VBridgeIfIdentifier;
import org.opendaylight.vtn.manager.internal.util.vnode.VInterfaceIdentifier;
import org.opendaylight.vtn.manager.internal.util.vnode.VTerminalIfIdentifier;

import org.opendaylight.controller.md.sal.binding.api.ReadWriteTransaction;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.port.rev150907.RemovePortMapInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.port.rev150907.RemovePortMapOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.port.rev150907.RemovePortMapOutputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.VtnPortMappableBridge;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.vbridge.list.Vbridge;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.vtn.mappable.vinterface.list.Vinterface;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vterminal.rev150907.vtn.vterminal.list.Vterminal;

/**
 * {@code RemovePortMapTask} describes the MD-SAL datastore transaction task
 * that removes the port mapping from the specified virtual interface.
 *
 * @param <B>  The type of the virtual bridge that contains the target
 *             virtual interface.
 * @see #create(RemovePortMapInput)
 */
public final class RemovePortMapTask<B extends VtnPortMappableBridge>
    extends AbstractRpcTask<VtnUpdateType>
    implements RpcOutputGenerator<VtnUpdateType, RemovePortMapOutput> {
    /**
     * The identifier for the target virtual interface.
     */
    private final VInterfaceIdentifier<B>  identifier;

    /**
     * Construct a new task that removes the port mapping from the specified
     * virtual interface.
     *
     * @param input  A {@link RemovePortMapInput} instance.
     * @return  A {@link RemovePortMapTask} instance associated with the task
     *          that removes the port mapping.
     * @throws RpcException
     *     The given input contains invalid value.
     */
    public static RemovePortMapTask<?> create(RemovePortMapInput input)
        throws RpcException {
        if (input == null) {
            throw RpcUtils.getNullInputException();
        }

        // Determine the target virtual interface.
        String tname = input.getTenantName();
        String bname = input.getBridgeName();
        String iname = input.getInterfaceName();
        RemovePortMapTask<?> task;
        if (bname == null) {
            String tmname = input.getTerminalName();
            VTerminalIfIdentifier ifId = VTerminalIfIdentifier.
                create(tname, tmname, iname, true);
            task = new RemovePortMapTask<Vterminal>(ifId);
        } else {
            VBridgeIfIdentifier ifId = VBridgeIfIdentifier.
                create(tname, bname, iname, true);
            task = new RemovePortMapTask<Vbridge>(ifId);
        }

        return task;
    }

    /**
     * Construct a new instance.
     *
     * @param ifId  The identifier for the target virtual interface.
     */
    private RemovePortMapTask(VInterfaceIdentifier<B> ifId) {
        identifier = ifId;
    }

    // AbstractTxTask

    /**
     * Remove the port mapping from the specified virtual interface.
     *
     * @param ctx  MD-SAL datastore transaction context.
     * @throws VTNException  An error occurred.
     */
    @Override
    protected VtnUpdateType execute(TxContext ctx) throws VTNException {
        // Ensure that the target virtual interface is present.
        ReadWriteTransaction tx = ctx.getReadWriteTransaction();
        Vinterface vintf = identifier.fetch(tx);

        // Remove the port mapping.
        VInterface<B> vif = VInterface.create(identifier, vintf);
        VtnUpdateType result = vif.removePortMap(ctx);

        if (result != null) {
            // Update the vinterface container.
            LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
            vintf = vif.getVinterface();
            tx.put(oper, identifier.getIdentifier(), vintf, false);

            // Update the status of the parent bridge.
            BridgeIdentifier<B> brId = identifier.getBridgeIdentifier();
            VirtualBridge<B> br = VirtualBridge.create(tx, brId);
            br.putState(ctx);
        }

        return result;
    }

    // RpcOutputGenerator

    /**
     * {@inheritDoc}
     */
    @Override
    public Class<RemovePortMapOutput> getOutputType() {
        return RemovePortMapOutput.class;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public RemovePortMapOutput createOutput(VtnUpdateType result) {
        return new RemovePortMapOutputBuilder().
            setStatus(result).build();
    }
}
