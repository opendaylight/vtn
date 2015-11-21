/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.vnode;

import javax.annotation.Nonnull;

import com.google.common.base.Optional;
import com.google.common.util.concurrent.FutureCallback;
import com.google.common.util.concurrent.Futures;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.TxContext;
import org.opendaylight.vtn.manager.internal.util.concurrent.SettableVTNFuture;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcOutputGenerator;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcUtils;
import org.opendaylight.vtn.manager.internal.util.vnode.VBridgeIdentifier;
import org.opendaylight.vtn.manager.internal.util.vnode.VTNVlanMapConfig;
import org.opendaylight.vtn.manager.internal.util.vnode.VlanMapIdentifier;

import org.opendaylight.controller.md.sal.binding.api.ReadTransaction;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.vlan.rev150907.GetVlanMapInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.vlan.rev150907.GetVlanMapOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.vlan.rev150907.GetVlanMapOutputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.vlan.rev150907.vtn.vlan.mappable.VlanMap;

/**
 * {@code GetVlanMapFuture} describes a future associated with the task that
 * implements get-vlan-map RPC.
 */
public final class GetVlanMapFuture extends SettableVTNFuture<VlanMap>
    implements FutureCallback<Optional<VlanMap>>,
               RpcOutputGenerator<VlanMap, GetVlanMapOutput> {
    /**
     * MD-SAL transaction context.
     */
    private final TxContext  context;

    /**
     * The identifier for the target vBridge.
     */
    private final VBridgeIdentifier  bridgeId;

    /**
     * Construct a new instance.
     *
     * @param ctx    MD-SAL datastore transaction context.
     * @param input  A {@link GetVlanMapInput} instance.
     * @throws RpcException
     *     The given input contains invalid value.
     */
    public GetVlanMapFuture(TxContext ctx, GetVlanMapInput input)
        throws RpcException {
        context = ctx;

        if (input == null) {
            throw RpcUtils.getNullInputException();
        }

        // Determine the target vBridge.
        VBridgeIdentifier vbrId = VBridgeIdentifier.create(input, true);
        bridgeId = vbrId;

        // Determine the identifier for the target VLAN mapping.
        String mapId = VTNVlanMapConfig.createMapId(input);
        if (mapId == null) {
            // The specified VLAN mapping should not be present.
            setResult(null);
        } else {
            // Read the specified VLAN mapping.
            VlanMapIdentifier vmapId = new VlanMapIdentifier(vbrId, mapId);
            LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
            ReadTransaction rtx = ctx.getTransaction();
            Futures.addCallback(rtx.read(oper, vmapId.getIdentifier()), this);
        }
    }

    /**
     * Set the result of this future.
     *
     * @param vlmap  A {@link VlanMap} instance.
     */
    private void setResult(VlanMap vlmap) {
        set(vlmap);
        context.cancelTransaction();
    }

    /**
     * Set the cause of failure.
     *
     * @param cause  A {@link Throwable} that indicates the cause of failure.
     */
    private void setFailure(Throwable cause) {
        setException(cause);
        context.cancelTransaction();
    }

    // FutureCallback

    /**
     * Invoked when the read operation has completed successfully.
     *
     * @param result  An {@link Optional} instance that contains the
     *                VLAN mapping.
     */
    @Override
    public void onSuccess(@Nonnull Optional<VlanMap> result) {
        VlanMap vlmap;
        if (result.isPresent()) {
            vlmap = result.get();
        } else {
            // Check to see if the target vBridge is present.
            try {
                bridgeId.fetch(context.getTransaction());
            } catch (VTNException | RuntimeException e) {
                setFailure(e);
                return;
            }

            vlmap = null;
        }

        setResult(vlmap);
    }

    /**
     * Invoked when the read operation has failed.
     *
     * @param cause  A {@link Throwable} that indicates the cause of failure.
     */
    @Override
    public void onFailure(Throwable cause) {
        setFailure(cause);
    }

    // RpcOutputGenerator

    /**
     * {@inheritDoc}
     */
    @Override
    public Class<GetVlanMapOutput> getOutputType() {
        return GetVlanMapOutput.class;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public GetVlanMapOutput createOutput(VlanMap result) {
        GetVlanMapOutputBuilder builder = new GetVlanMapOutputBuilder();
        if (result != null) {
            builder.setMapId(result.getMapId()).
                fieldsFrom(result.getVlanMapStatus());
        }

        return builder.build();
    }
}
