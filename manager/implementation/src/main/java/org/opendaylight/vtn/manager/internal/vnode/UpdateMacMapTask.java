/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.vnode;

import static org.opendaylight.vtn.manager.internal.vnode.MappingRegistry.registerMacMap;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.TxContext;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;
import org.opendaylight.vtn.manager.internal.util.tx.AbstractRpcTask;
import org.opendaylight.vtn.manager.internal.util.vnode.MacMapChange;
import org.opendaylight.vtn.manager.internal.util.vnode.MacMapConflictException;
import org.opendaylight.vtn.manager.internal.util.vnode.MacMapIdentifier;
import org.opendaylight.vtn.manager.internal.util.vnode.VBridgeIdentifier;
import org.opendaylight.vtn.manager.internal.util.vnode.VTNMacMapConfig;
import org.opendaylight.vtn.manager.internal.util.vnode.VTNMacMapStatus;

import org.opendaylight.controller.md.sal.binding.api.ReadWriteTransaction;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.mac.rev150907.vtn.mac.mappable.MacMap;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.mac.rev150907.vtn.mac.mappable.MacMapBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateOperationType;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.vbridge.list.Vbridge;

/**
 * Base class for classes that implement RPC task to update the MAC mapping
 * configuration.
 */
public abstract class UpdateMacMapTask extends AbstractRpcTask<VtnUpdateType> {
    /**
     * The identifier for the target vBridge.
     */
    private final VBridgeIdentifier  bridgeId;

    /**
     * Describes how to update the MAC mapping configuration.
     */
    private final VtnUpdateOperationType  operation;

    /**
     * Construct a new instance.
     *
     * @param vbrId  The identifier for the target vBridge.
     * @param op     A {@link VtnUpdateOperationType} instance.
     */
    protected UpdateMacMapTask(VBridgeIdentifier vbrId,
                               VtnUpdateOperationType op) {
        bridgeId = vbrId;
        operation = op;
    }

    // AbstractTxTask

    /**
     * Update the MAC mapping configuration.
     *
     * @param ctx  MD-SAL datastore transaction context.
     * @return  A {@link VtnUpdateType} instance which indicates the result.
     *          {@code null} is returned if the MAC mapping was not changed.
     * @throws VTNException  An error occurred.
     */
    @Override
    protected final VtnUpdateType execute(TxContext ctx) throws VTNException {
        // Ensure that the target vBridge is present.
        ReadWriteTransaction tx = ctx.getReadWriteTransaction();
        Vbridge vbridge = bridgeId.fetch(tx);

        // Determine the current MAC mapping configuration.
        MacMap mmp = vbridge.getMacMap();
        VTNMacMapConfig cfg = (mmp == null)
            ? new VTNMacMapConfig()
            : new VTNMacMapConfig(mmp.getMacMapConfig());

        // Update the configuration.
        MacMapChange change = updateConfig(cfg, operation);

        // Apply the new configuration if changed.
        return (change == null)
            ? null
            : update(ctx, mmp, cfg, change);
    }

    /**
     * Update the MAC mapping.
     *
     * @param ctx     MD-SAL datastore transaction context.
     * @param mmp     The current MAC mapping container.
     * @param cfg     The MAC mapping configuration to be applied.
     * @param change  A {@link MacMapChange} instance that contains changes to
     *                the MAC mapping configuration.
     * @return  A {@link VtnUpdateType} instance.
     * @throws VTNException  An error occurred.
     */
    private VtnUpdateType update(TxContext ctx, MacMap mmp,
                                 VTNMacMapConfig cfg, MacMapChange change)
        throws VTNException {
        VTNMacMapStatus vmst;
        VtnUpdateType result;
        if (mmp == null) {
            // MAC mapping is going to be created.
            vmst = new VTNMacMapStatus();
            result = VtnUpdateType.CREATED;
        } else {
            vmst = new VTNMacMapStatus(mmp.getMacMapStatus());
            if (change.isRemoving()) {
                // MAC mapping is going to be removed.
                result = VtnUpdateType.REMOVED;
            } else {
                // MAC mapping is going to be changed.
                result = VtnUpdateType.CHANGED;
            }
        }

        // Prepare the MAC mapping status.
        MacMapIdentifier mapId = new MacMapIdentifier(bridgeId);
        MacMapStatusReader reader = ctx.getSpecific(MacMapStatusReader.class);
        vmst = reader.put(mapId, vmst);

        // Update the mapping.
        try {
            registerMacMap(ctx, mapId, change);
        } catch (MacMapConflictException e) {
            String msg = e.getErrorMessage();
            throw RpcException.getDataExistsException(msg, e);
        }

        ReadWriteTransaction tx = ctx.getReadWriteTransaction();
        InstanceIdentifier<MacMap> path = mapId.getIdentifier();
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        if (result == VtnUpdateType.REMOVED) {
            // Remove the mac-map container.
            tx.delete(oper, path);
            reader.remove(mapId);
        } else {
            // Clear dirty flag in the MAC mapping status so that the
            // TX pre-submit hook ignores this status.
            vmst.isDirty();

            // Update the whole mac-map container.
            MacMap newMap = new MacMapBuilder().
                setMacMapConfig(cfg.toMacMapConfig()).
                setMacMapStatus(vmst.toMacMapStatus()).
                build();
            tx.put(oper, path, newMap, false);
        }

        // Update the status of the target vBridge.
        VBridge vbr = new VBridge(bridgeId, bridgeId.fetch(tx));
        vbr.putState(ctx);

        return result;
    }

    /**
     * Update the MAC mapping configuration.
     *
     * @param cfg  The current MAC mapping configuration.
     * @param op   A {@link VtnUpdateOperationType} instance that specifies
     *             how to update the configuration.
     * @return  A {@link MacMapChange} instance if the configuration was
     *          chaged. {@code null} if not changed.
     * @throws RpcException
     *    Failed to update the configuration.
     */
    protected abstract MacMapChange updateConfig(
        VTNMacMapConfig cfg, VtnUpdateOperationType op) throws RpcException;
}
