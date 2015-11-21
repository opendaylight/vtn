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
import org.opendaylight.vtn.manager.internal.util.vnode.VBridgeIdentifier;
import org.opendaylight.vtn.manager.internal.util.vnode.VTNVlanMapConfig;
import org.opendaylight.vtn.manager.internal.util.vnode.VlanMapIdentifier;

import org.opendaylight.controller.md.sal.binding.api.ReadWriteTransaction;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.vlan.rev150907.AddVlanMapInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.vlan.rev150907.AddVlanMapOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.vlan.rev150907.AddVlanMapOutputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.vlan.rev150907.vtn.vlan.mappable.VlanMap;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeState;

/**
 * {@code AddVlanMapTask} describes the MD-SAL datastore transaction task
 * that adds a VLAN mapping to the specified vBridge.
 *
 * @see #create(AddVlanMapInput)
 */
public final class AddVlanMapTask extends AbstractRpcTask<VlanMap>
    implements RpcOutputGenerator<VlanMap, AddVlanMapOutput> {
    /**
     * The identifier for the target vBridge.
     */
    private final VBridgeIdentifier  bridgeId;

    /**
     * The configuration of the VLAN mapping.
     */
    private final VTNVlanMapConfig  config;

    /**
     * Construct a new task that adds a VLAN mapping to the specified vBridge.
     *
     * @param input  An {@link AddVlanMapInput} instance.
     * @return  A {@link AddVlanMapTask} instance associated with the task
     *          that adds a VLAN mapping to the specified vBridge.
     * @throws RpcException
     *     The given input contains invalid value.
     */
    public static AddVlanMapTask create(AddVlanMapInput input)
        throws RpcException {
        if (input == null) {
            throw RpcUtils.getNullInputException();
        }

        // Determine the target vBridge.
        VBridgeIdentifier vbrId = VBridgeIdentifier.create(input, true);

        // Verify the VLAN mapping configuration.
        VTNVlanMapConfig cfg = new VTNVlanMapConfig(input);

        return new AddVlanMapTask(vbrId, cfg);
    }

    /**
     * Construct a new instance.
     *
     * @param vbrId  The identifier for the target vBridge.
     * @param cfg    The configuration of the VLAN mapping.
     */
    private AddVlanMapTask(VBridgeIdentifier vbrId, VTNVlanMapConfig cfg) {
        bridgeId = vbrId;
        config = cfg;
    }

    // AbstractTxTask

    /**
     * Add a VLAN mapping to the target vBridge.
     *
     * @param ctx  MD-SAL datastore transaction context.
     * @return  A {@link VlanMap} instance.
     * @throws VTNException  An error occurred.
     */
    @Override
    protected VlanMap execute(TxContext ctx) throws VTNException {
        // Ensure that the target vBridge is present.
        ReadWriteTransaction tx = ctx.getReadWriteTransaction();
        bridgeId.fetch(tx);

        // Register the VLAN mapping.
        VTNVlanMap vmap = new VTNVlanMap(bridgeId, config);
        VnodeState st = vmap.register(ctx, true);
        VlanMap vlmap = vmap.getVlanMap();
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        VlanMapIdentifier vmapId = vmap.getIdentifier();
        tx.put(oper, vmapId.getIdentifier(), vlmap, false);

        // Update the status of the target vBridge.
        VBridge vbr = new VBridge(bridgeId, bridgeId.fetch(tx));
        vbr.putState(ctx, st);

        return vlmap;
    }

    // RpcOutputGenerator

    /**
     * {@inheritDoc}
     */
    @Override
    public Class<AddVlanMapOutput> getOutputType() {
        return AddVlanMapOutput.class;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public AddVlanMapOutput createOutput(VlanMap result) {
        AddVlanMapOutputBuilder builder = new AddVlanMapOutputBuilder();
        builder.fieldsFrom(result.getVlanMapStatus());

        return builder.setMapId(result.getMapId()).build();
    }
}

