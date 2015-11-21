/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.vnode;

import java.util.concurrent.Future;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import org.opendaylight.vtn.manager.internal.VTNManagerProvider;
import org.opendaylight.vtn.manager.internal.util.CompositeAutoCloseable;
import org.opendaylight.vtn.manager.internal.util.concurrent.VTNFuture;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcFuture;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcUtils;

import org.opendaylight.controller.sal.binding.api.RpcProviderRegistry;

import org.opendaylight.yangtools.yang.common.RpcResult;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.port.rev150907.RemovePortMapInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.port.rev150907.RemovePortMapOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.port.rev150907.SetPortMapInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.port.rev150907.SetPortMapOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.port.rev150907.VtnPortMapService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.RemoveVinterfaceInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.UpdateVinterfaceInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.UpdateVinterfaceOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.VtnVinterfaceService;

/**
 * {@code VInterfaceService} provides RPC services related to virtual
 * interface.
 */
public final class VInterfaceService
    implements VtnVinterfaceService, VtnPortMapService {
    /**
     * Logger instance.
     */
    private static final Logger  LOG =
        LoggerFactory.getLogger(VInterfaceService.class);

    /**
     * VTN Manager provider service.
     */
    private final VTNManagerProvider  vtnProvider;

    /**
     * Construct a new instance.
     *
     * @param provider  A VTN Manager provider service.
     */
    public VInterfaceService(VTNManagerProvider provider) {
        vtnProvider = provider;
    }

    /**
     * Register RPC services.
     *
     * @param rpcReg  A {@link RpcProviderRegistry} service instance.
     * @param regs    A {@link CompositeAutoCloseable} instance to store
     *                RPC registration.
     */
    public void initRpcServices(RpcProviderRegistry rpcReg,
                                CompositeAutoCloseable regs) {
        regs.add(rpcReg.addRpcImplementation(VtnVinterfaceService.class, this));
        regs.add(rpcReg.addRpcImplementation(VtnPortMapService.class, this));
    }

    // VtnVinterfaceService

    /**
     * Create or modify the specified virtual interface.
     *
     * @param input  Input of the RPC.
     * @return  A {@link Future} associated with the RPC task.
     */
    @Override
    public Future<RpcResult<UpdateVinterfaceOutput>> updateVinterface(
        UpdateVinterfaceInput input) {
        try {
            // Create a task that updates the virtual interface.
            UpdateVinterfaceTask<?> task = UpdateVinterfaceTask.create(input);
            VTNFuture<VtnUpdateType> taskFuture = vtnProvider.postSync(task);
            return new RpcFuture<VtnUpdateType, UpdateVinterfaceOutput>(
                taskFuture, task);
        } catch (RpcException | RuntimeException e) {
            return RpcUtils.getErrorBuilder(UpdateVinterfaceOutput.class, e).
                buildFuture();
        }
    }

    /**
     * Remove the specified virtual interface.
     *
     * @param input  Input of the RPC.
     * @return  A {@link Future} associated with the RPC task.
     */
    @Override
    public Future<RpcResult<Void>> removeVinterface(
        RemoveVinterfaceInput input) {
        try {
            // Create a task that removes the specified virtual interface.
            RemoveVinterfaceTask<?> task = RemoveVinterfaceTask.create(input);
            VTNFuture<VtnUpdateType> taskFuture = vtnProvider.postSync(task);
            return new RpcFuture<VtnUpdateType, Void>(taskFuture, task);
        } catch (RpcException | RuntimeException e) {
            return RpcUtils.getErrorBuilder(Void.class, e).buildFuture();
        }
    }

    // VtnPortMapService

    /**
     * Configure the port mapping in the specified virtual interface.
     *
     * @param input  Input of the RPC.
     * @return  A {@link Future} associated with the RPC task.
     */
    @Override
    public Future<RpcResult<SetPortMapOutput>> setPortMap(
        SetPortMapInput input) {
        try {
            // Create a task that configures the port mapping.
            SetPortMapTask<?> task = SetPortMapTask.create(input);
            VTNFuture<VtnUpdateType> taskFuture = vtnProvider.postSync(task);
            return new RpcFuture<VtnUpdateType, SetPortMapOutput>(
                taskFuture, task);
        } catch (RpcException | RuntimeException e) {
            return RpcUtils.getErrorBuilder(SetPortMapOutput.class, e).
                buildFuture();
        }
    }

    /**
     * Remove the port mapping from the specified virtual interface.
     *
     * @param input  Input of the RPC.
     * @return  A {@link Future} associated with the RPC task.
     */
    @Override
    public Future<RpcResult<RemovePortMapOutput>> removePortMap(
        RemovePortMapInput input) {
        try {
            // Create a task that removes the port mapping.
            RemovePortMapTask<?> task = RemovePortMapTask.create(input);
            VTNFuture<VtnUpdateType> taskFuture = vtnProvider.postSync(task);
            return new RpcFuture<VtnUpdateType, RemovePortMapOutput>(
                taskFuture, task);
        } catch (RpcException | RuntimeException e) {
            return RpcUtils.getErrorBuilder(RemovePortMapOutput.class, e).
                buildFuture();
        }
    }
}
