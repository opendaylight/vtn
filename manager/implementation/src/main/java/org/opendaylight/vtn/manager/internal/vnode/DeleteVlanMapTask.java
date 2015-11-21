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
import org.opendaylight.vtn.manager.internal.util.tx.DeleteDataTask;
import org.opendaylight.vtn.manager.internal.util.vnode.VlanMapIdentifier;

import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.vlan.rev150907.vtn.vlan.mappable.VlanMap;

/**
 * {@code DeleteVlanMapTask} describes the MD-SAL datastore transaction task
 * that deletes the specified VLAN mapping from the vBridge.
 *
 * <p>
 *   A task corresponding to this task is used as a sub task for
 *   {@link RemoveVlanMapTask}.
 * </p>
 *
 * @see RemoveVlanMapTask
 */
public final class DeleteVlanMapTask extends DeleteDataTask<VlanMap> {
    /**
     * The identifier for the target VLAN mapping.
     */
    private final VlanMapIdentifier  vlanMapId;

    /**
     * Construct a new instance.
     *
     * @param ident  The identifier for the target VLAN mapping.
     */
    DeleteVlanMapTask(VlanMapIdentifier ident) {
        super(LogicalDatastoreType.OPERATIONAL, ident.getIdentifier());
        vlanMapId = ident;
    }

    /**
     * Return an identifier string for the target VLAN mapping.
     *
     * @return  An identifier for the VLAN mapping.
     */
    public String getMapId() {
        return vlanMapId.getMapId();
    }

    // DeleteDataTask

    /**
     * Invoked when the MD-SAL datastore transaction has started.
     *
     * @param ctx      A runtime context for transaction task.
     * @param current  The current value of the target VLAN mapping.
     *                 Note that {@code null} is passed if the target VLAN
     *                 mapping is not present.
     * @throws VTNException  An error occurred.
     */
    @Override
    protected void onStarted(TxContext ctx, VlanMap current)
        throws VTNException {
        if (current != null) {
            // Destroy the VLAN mapping.
            VTNVlanMap vmap = new VTNVlanMap(vlanMapId, current);
            vmap.destroy(ctx, true);
        }
    }
}
