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
import org.opendaylight.vtn.manager.internal.util.vnode.VTNPortMapConfig;
import org.opendaylight.vtn.manager.internal.util.vnode.VTerminalIfIdentifier;

import org.opendaylight.controller.md.sal.binding.api.ReadWriteTransaction;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.port.rev150907.SetPortMapInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.port.rev150907.SetPortMapOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.port.rev150907.SetPortMapOutputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.port.rev150907.VtnPortMapConfig;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.VtnPortMappableBridge;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.vbridge.list.Vbridge;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.vtn.mappable.vinterface.list.Vinterface;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vterminal.rev150907.vtn.vterminal.list.Vterminal;

/**
 * {@code SetPortMapTask} describes the MD-SAL datastore transaction task
 * that configures the port mapping in the specified virtual interface.
 *
 * @param <B>  The type of the virtual bridge that contains the target
 *             virtual interface.
 * @see #create(SetPortMapInput)
 */
public final class SetPortMapTask<B extends VtnPortMappableBridge>
    extends AbstractRpcTask<VtnUpdateType>
    implements RpcOutputGenerator<VtnUpdateType, SetPortMapOutput> {
    /**
     * The identifier for the target virtual interface.
     */
    private final VInterfaceIdentifier<B>  identifier;

    /**
     * The configuration of the port mapping.
     */
    private final VTNPortMapConfig  config;

    /**
     * Construct a new task that configures the port mapping in the specified
     * virtual interface.
     *
     * @param input  A {@link SetPortMapInput} instance.
     * @return  A {@link SetPortMapTask} instance associated with the task
     *          that configures the port mapping.
     * @throws RpcException
     *     The given input contains invalid value.
     */
    public static SetPortMapTask<?> create(SetPortMapInput input)
        throws RpcException {
        if (input == null) {
            throw RpcUtils.getNullInputException();
        }

        // Determine the target virtual interface.
        String tname = input.getTenantName();
        String bname = input.getBridgeName();
        String iname = input.getInterfaceName();
        SetPortMapTask<?> task;
        if (bname == null) {
            String tmname = input.getTerminalName();
            VTerminalIfIdentifier ifId = VTerminalIfIdentifier.
                create(tname, tmname, iname, true);
            task = new SetPortMapTask<Vterminal>(ifId, input);
        } else {
            VBridgeIfIdentifier ifId = VBridgeIfIdentifier.
                create(tname, bname, iname, true);
            task = new SetPortMapTask<Vbridge>(ifId, input);
        }

        return task;
    }

    /**
     * Construct a new instance.
     *
     * @param ifId  The identifier for the target virtual interface.
     * @param cfg   The configuration of the port mapping.
     * @throws RpcException
     *    The specified configuration is invalid.
     */
    private SetPortMapTask(VInterfaceIdentifier<B> ifId, VtnPortMapConfig cfg)
        throws RpcException {
        identifier = ifId;
        config = new VTNPortMapConfig(cfg);
    }

    // AbstractTxTask

    /**
     * Configure the port mapping in the specified virtual interface.
     *
     * @param ctx  MD-SAL datastore transaction context.
     * @throws VTNException  An error occurred.
     */
    @Override
    protected VtnUpdateType execute(TxContext ctx) throws VTNException {
        // Ensure that the target virtual interface is present.
        ReadWriteTransaction tx = ctx.getReadWriteTransaction();
        Vinterface vintf = identifier.fetch(tx);

        // Configure the port mapping.
        VInterface<B> vif = VInterface.create(identifier, vintf);
        VtnUpdateType result = vif.setPortMap(ctx, config);

        if (result != null) {
            // Update the vinterface container.
            LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
            vintf = vif.getVinterface();
            tx.put(oper, identifier.getIdentifier(), vintf, false);

            // Update the status of the parent bridge.
            BridgeIdentifier<B> brId = identifier.getBridgeIdentifier();
            VirtualBridge<B> br = VirtualBridge.create(tx, brId);
            br.putState(ctx, vif);
        }

        return result;
    }

    // RpcOutputGenerator

    /**
     * {@inheritDoc}
     */
    @Override
    public Class<SetPortMapOutput> getOutputType() {
        return SetPortMapOutput.class;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public SetPortMapOutput createOutput(VtnUpdateType result) {
        return new SetPortMapOutputBuilder().
            setStatus(result).build();
    }
}
