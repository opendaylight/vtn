/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.vnode;

import java.util.Iterator;
import java.util.List;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ConcurrentMap;
import java.util.concurrent.Future;
import java.util.concurrent.atomic.AtomicReference;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import org.opendaylight.vtn.manager.internal.TxContext;
import org.opendaylight.vtn.manager.internal.VTNManagerProvider;
import org.opendaylight.vtn.manager.internal.util.CompositeAutoCloseable;
import org.opendaylight.vtn.manager.internal.util.MiscUtils;
import org.opendaylight.vtn.manager.internal.util.VTNEntityType;
import org.opendaylight.vtn.manager.internal.util.concurrent.VTNFuture;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcFuture;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcUtils;
import org.opendaylight.vtn.manager.internal.util.vnode.VBridgeIdentifier;

import org.opendaylight.controller.md.sal.common.api.clustering.Entity;
import org.opendaylight.controller.md.sal.common.api.clustering.EntityOwnershipChange;
import org.opendaylight.controller.md.sal.common.api.clustering.EntityOwnershipListener;
import org.opendaylight.controller.md.sal.common.api.clustering.EntityOwnershipListenerRegistration;
import org.opendaylight.controller.sal.binding.api.RpcProviderRegistry;

import org.opendaylight.yangtools.yang.common.RpcResult;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.mac.rev150907.GetMacMappedHostInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.mac.rev150907.GetMacMappedHostOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.mac.rev150907.SetMacMapAclInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.mac.rev150907.SetMacMapAclOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.mac.rev150907.SetMacMapInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.mac.rev150907.SetMacMapOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.mac.rev150907.VtnMacMapService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.mac.rev150907.get.mac.mapped.host.output.MacMappedHost;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.vlan.rev150907.AddVlanMapInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.vlan.rev150907.AddVlanMapOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.vlan.rev150907.GetVlanMapInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.vlan.rev150907.GetVlanMapOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.vlan.rev150907.RemoveVlanMapInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.vlan.rev150907.RemoveVlanMapOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.vlan.rev150907.VtnVlanMapService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.vlan.rev150907.remove.vlan.map.output.RemoveVlanMapResult;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.vlan.rev150907.vtn.vlan.mappable.VlanMap;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.mac.rev150907.RemoveMacEntryInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.mac.rev150907.RemoveMacEntryOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.mac.rev150907.VtnMacTableService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.mac.rev150907.remove.mac.entry.output.RemoveMacEntryResult;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.RemoveVbridgeInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.UpdateVbridgeInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.UpdateVbridgeOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.VtnVbridgeService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.vbridge.info.VbridgeConfig;

import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.yang.types.rev100924.MacAddress;

/**
 * {@code VBridgeManager} provides services related to vBridge.
 */
public final class VBridgeManager
    implements EntityOwnershipListener, VtnVbridgeService, VtnMacTableService,
               VtnVlanMapService, VtnMacMapService {
    /**
     * Logger instance.
     */
    private static final Logger  LOG =
        LoggerFactory.getLogger(VBridgeManager.class);

    /**
     * VTN Manager provider service.
     */
    private final VTNManagerProvider  vtnProvider;

    /**
     * Entities of all existing vBridges.
     */
    private final ConcurrentMap<Entity, VBridgeEntity>  vBridges =
        new ConcurrentHashMap<>();

    /**
     * Registration of the entity ownership listener.
     */
    private final AtomicReference<EntityOwnershipListenerRegistration> listener =
        new AtomicReference<>();

    /**
     * Create a new entity descriptor for the given vBridge.
     *
     * @param ident  The identifier for the vBridge.
     * @return  An {@link Entity} instance.
     */
    static Entity createEntity(VBridgeIdentifier ident) {
        return new Entity(VTNEntityType.VBRIDGE.getType(), ident.toString());
    }

    /**
     * Construct a new instance.
     *
     * @param provider  A VTN Manager provider service.
     */
    public VBridgeManager(VTNManagerProvider provider) {
        vtnProvider = provider;

        // Register entity ownership listener to listen ownership status of
        // vBridges.
        listener.set(provider.registerListener(VTNEntityType.VBRIDGE, this));
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
        regs.add(rpcReg.addRpcImplementation(VtnVbridgeService.class, this));
        regs.add(rpcReg.addRpcImplementation(VtnMacTableService.class, this));
        regs.add(rpcReg.addRpcImplementation(VtnVlanMapService.class, this));
        regs.add(rpcReg.addRpcImplementation(VtnMacMapService.class, this));
    }

    /**
     * Update the entity of the specified vBridge.
     *
     * <p>
     *   The entity of the specified vBridge will be created if not present.
     *   If present, the specified configuration will be applied to the
     *   existing entity of the vBridge.
     * </p>
     *
     * @param ident   The identifier for the target vBridge.
     * @param config  The configuration of the vBridge.
     */
    public void updateEntity(VBridgeIdentifier ident, VbridgeConfig config) {
        if (listener.get() != null) {
            // Create the entity of the specified vBridge.
            Entity ent = createEntity(ident);
            VBridgeEntity vbent =
                new VBridgeEntity(vtnProvider, ident, config);
            VBridgeEntity current = vBridges.putIfAbsent(ent, vbent);
            if (current == null) {
                // Register a candidate for ownership of the specified vBridge.
                if (!vbent.register(ent)) {
                    // This should never happen.
                    vBridges.remove(ent, vbent);
                }
            } else {
                // Apply new vBridge configuration.
                current.apply(config);
            }
        }
    }

    /**
     * Remove the entity of the specified vBridge.
     *
     * @param ident   The identifier for the target vBridge.
     */
    public void removeEntity(VBridgeIdentifier ident) {
        Entity ent = createEntity(ident);
        VBridgeEntity vbent = vBridges.remove(ent);
        if (vbent != null) {
            // Destroy the entity of the given vBridge.
            vbent.destroy();
        }
    }

    /**
     * Shut down the vBridge entity management.
     */
    public void shutdown() {
        EntityOwnershipListenerRegistration reg = listener.getAndSet(null);
        if (reg != null) {
            // Unregister the entity ownership listener.
            reg.close();

            // Destroy all the vBridge entities.
            while (!vBridges.isEmpty()) {
                for (Iterator<VBridgeEntity> it = vBridges.values().iterator();
                     it.hasNext();) {
                    VBridgeEntity vbent = it.next();
                    it.remove();
                    vbent.destroy();
                }
            }
        }
    }

    /**
     * Start a background task that executes remove-mac-entry RPC.
     *
     * @param input  A {@link RemoveMacEntryInput} instance.
     * @return  A {@link Future} associated with the RPC task.
     * @throws RpcException  An error occurred.
     */
    private Future<RpcResult<RemoveMacEntryOutput>> startRpc(
        RemoveMacEntryInput input) throws RpcException {
        if (input == null) {
            throw RpcUtils.getNullInputException();
        }

        // Determine the target MAC address table.
        VBridgeIdentifier ident = VBridgeIdentifier.create(input, true);
        List<MacAddress> addrs = input.getMacAddresses();
        Future<RpcResult<RemoveMacEntryOutput>> ret;
        if (MiscUtils.isEmpty(addrs)) {
            // Clear the specified MAC address table.
            ClearMacEntryTask task = new ClearMacEntryTask(ident);
            VTNFuture<List<RemoveMacEntryResult>> f =
                vtnProvider.postSync(task);
            ret = new RpcFuture<List<RemoveMacEntryResult>, RemoveMacEntryOutput>(
                f, task);
        } else {
            // Remove only the specified MAC addresses.
            RemoveMacEntryTask task = RemoveMacEntryTask.create(ident, addrs);
            VTNFuture<List<VtnUpdateType>> f = vtnProvider.postSync(task);
            ret = new RpcFuture<List<VtnUpdateType>, RemoveMacEntryOutput>(
                f, task);
        }

        return ret;
    }

    /**
     * Start a background task that executes remove-vlan-map RPC.
     *
     * @param input  A {@link RemoveVlanMapInput} instance.
     * @return  A {@link Future} associated with the RPC task.
     * @throws RpcException  An error occurred.
     */
    private Future<RpcResult<RemoveVlanMapOutput>> startRpc(
        RemoveVlanMapInput input) throws RpcException {
        if (input == null) {
            throw RpcUtils.getNullInputException();
        }

        // Determine the target vBridge.
        VBridgeIdentifier ident = VBridgeIdentifier.create(input, true);
        List<String> mapIds = input.getMapIds();
        Future<RpcResult<RemoveVlanMapOutput>> ret;
        if (MiscUtils.isEmpty(mapIds)) {
            // Remove all the VLAN mappings.
            ClearVlanMapTask task = new ClearVlanMapTask(ident);
            VTNFuture<List<RemoveVlanMapResult>> f =
                vtnProvider.postSync(task);
            ret = new RpcFuture<List<RemoveVlanMapResult>, RemoveVlanMapOutput>(
                f, task);
        } else {
            // Remove only the specified VLAN mappings.
            RemoveVlanMapTask task = RemoveVlanMapTask.create(ident, mapIds);
            VTNFuture<List<VtnUpdateType>> f = vtnProvider.postSync(task);
            ret = new RpcFuture<List<VtnUpdateType>, RemoveVlanMapOutput>(
                f, task);
        }

        return ret;
    }

    // EntityOwnershipListener

    /**
     * Invoked when the ownership status of the vBridge has been changed.
     *
     * @param change  An {@link EntityOwnershipChange} instance.
     */
    @Override
    public void ownershipChanged(EntityOwnershipChange change) {
        LOG.debug("Received vBridge entity ownerchip change: {}", change);

        if (listener.get() != null) {
            Entity ent = change.getEntity();
            VBridgeEntity vbent = vBridges.get(ent);
            if (vbent != null) {
                if (change.isOwner()) {
                    vbent.startAging();

                    // Ensure that the listener is still valid.
                    if (listener.get() == null) {
                        vBridges.remove(ent);
                        vbent.stopAging();
                    }
                } else {
                    vbent.stopAging();
                }
            }
        }
    }

    // VtnVbridgeService

    /**
     * Create or modify the specified vBridge.
     *
     * @param input  Input of the RPC.
     * @return  A {@link Future} associated with the RPC task.
     */
    @Override
    public Future<RpcResult<UpdateVbridgeOutput>> updateVbridge(
        UpdateVbridgeInput input) {
        try {
            // Create a task that updates the vBridge.
            UpdateVbridgeTask task = UpdateVbridgeTask.create(input);
            VTNFuture<VtnUpdateType> taskFuture = vtnProvider.postSync(task);
            return new RpcFuture<VtnUpdateType, UpdateVbridgeOutput>(
                taskFuture, task);
        } catch (RpcException | RuntimeException e) {
            return RpcUtils.getErrorBuilder(UpdateVbridgeOutput.class, e).
                buildFuture();
        }
    }

    /**
     * Remove the specified vBridge.
     *
     * @param input  Input of the RPC.
     * @return  A {@link Future} associated with the RPC task.
     */
    @Override
    public Future<RpcResult<Void>> removeVbridge(RemoveVbridgeInput input) {
        try {
            // Create a task that removes the specified vBridge.
            RemoveVbridgeTask task = RemoveVbridgeTask.create(input);
            VTNFuture<VtnUpdateType> taskFuture = vtnProvider.postSync(task);
            return new RpcFuture<VtnUpdateType, Void>(taskFuture, task);
        } catch (RpcException | RuntimeException e) {
            return RpcUtils.getErrorBuilder(Void.class, e).buildFuture();
        }
    }

    // VtnMacTableService

    /**
     * Remove the specified MAC address information from the MAC address table
     * in the  specified vBridge.
     *
     * @param input  Input of the RPC.
     * @return  A {@link Future} associated with the RPC task.
     */
    @Override
    public Future<RpcResult<RemoveMacEntryOutput>> removeMacEntry(
        RemoveMacEntryInput input) {
        try {
            // Create a task that removes the specified MAC addresses.
            return startRpc(input);
        } catch (RpcException | RuntimeException e) {
            return RpcUtils.getErrorBuilder(RemoveMacEntryOutput.class, e).
                buildFuture();
        }
    }

    // VtnVlanMapService

    /**
     * Configure a VLAN mapping in the specified vBridge.
     *
     * @param input  Input of the RPC.
     * @return  A {@link Future} associated with the RPC task.
     */
    @Override
    public Future<RpcResult<AddVlanMapOutput>> addVlanMap(
        AddVlanMapInput input) {
        try {
            // Create a task that adds a VLAN mapping.
            AddVlanMapTask task = AddVlanMapTask.create(input);
            VTNFuture<VlanMap> taskFuture = vtnProvider.postSync(task);
            return new RpcFuture<VlanMap, AddVlanMapOutput>(
                taskFuture, task);
        } catch (RpcException | RuntimeException e) {
            return RpcUtils.getErrorBuilder(AddVlanMapOutput.class, e).
                buildFuture();
        }
    }

    /**
     * Remove the specified VLAN mappings from the vBridge.
     *
     * @param input  Input of the RPC.
     * @return  A {@link Future} associated with the RPC task.
     */
    @Override
    public Future<RpcResult<RemoveVlanMapOutput>> removeVlanMap(
        RemoveVlanMapInput input) {
        try {
            // Create a task that removes the specified VLAN mappings.
            return startRpc(input);
        } catch (RpcException | RuntimeException e) {
            return RpcUtils.getErrorBuilder(RemoveVlanMapOutput.class, e).
                buildFuture();
        }
    }

    /**
     * Search for a VLAN mapping with the specified VLAN mapping configuration
     * in the specified vBridge.
     *
     * @param input  Input of the RPC.
     * @return  A {@link Future} associated with the RPC task.
     */
    @Override
    public Future<RpcResult<GetVlanMapOutput>> getVlanMap(
        GetVlanMapInput input) {
        TxContext ctx = vtnProvider.newTxContext();
        try {
            GetVlanMapFuture f = new GetVlanMapFuture(ctx, input);
            return new RpcFuture<VlanMap, GetVlanMapOutput>(f, f);
        } catch (RpcException | RuntimeException e) {
            ctx.cancelTransaction();
            return RpcUtils.getErrorBuilder(GetVlanMapOutput.class, e).
                buildFuture();
        }
    }

    // VtnMacMapService

    /**
     * Configure MAC mapping in the specified vBridge.
     *
     * @param input  Input of the RPC.
     * @return  A {@link Future} associated with the RPC task.
     */
    @Override
    public Future<RpcResult<SetMacMapOutput>> setMacMap(SetMacMapInput input) {
        try {
            // Create a task that configures the MAC mapping.
            SetMacMapTask task = SetMacMapTask.create(input);
            VTNFuture<VtnUpdateType> taskFuture = vtnProvider.postSync(task);
            return new RpcFuture<VtnUpdateType, SetMacMapOutput>(
                taskFuture, task);
        } catch (RpcException | RuntimeException e) {
            return RpcUtils.getErrorBuilder(SetMacMapOutput.class, e).
                buildFuture();
        }
    }

    /**
     * Configure MAC mapping in the specified vBridge.
     *
     * @param input  Input of the RPC.
     * @return  A {@link Future} associated with the RPC task.
     */
    @Override
    public Future<RpcResult<SetMacMapAclOutput>> setMacMapAcl(
        SetMacMapAclInput input) {
        try {
            // Create a task that configures the access control list in the
            // MAC mapping.
            SetMacMapAclTask task = SetMacMapAclTask.create(input);
            VTNFuture<VtnUpdateType> taskFuture = vtnProvider.postSync(task);
            return new RpcFuture<VtnUpdateType, SetMacMapAclOutput>(
                taskFuture, task);
        } catch (RpcException | RuntimeException e) {
            return RpcUtils.getErrorBuilder(SetMacMapAclOutput.class, e).
                buildFuture();
        }
    }

    /**
     * Get a list of hosts where mapping is actually active based on
     * MAC mapping configured in the specified vBridge.
     *
     * @param input  Input of the RPC.
     * @return  A {@link Future} associated with the RPC task.
     */
    @Override
    public Future<RpcResult<GetMacMappedHostOutput>> getMacMappedHost(
        GetMacMappedHostInput input) {
        TxContext ctx = vtnProvider.newTxContext();
        try {
            GetMacMappedHostFuture f = new GetMacMappedHostFuture(ctx, input);
            return new RpcFuture<List<MacMappedHost>, GetMacMappedHostOutput>(
                f, f);
        } catch (RpcException | RuntimeException e) {
            ctx.cancelTransaction();
            return RpcUtils.getErrorBuilder(GetMacMappedHostOutput.class, e).
                buildFuture();
        }
    }
}
