/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.vnode;

import com.google.common.base.Optional;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.TxContext;
import org.opendaylight.vtn.manager.internal.util.DataStoreUtils;
import org.opendaylight.vtn.manager.internal.util.inventory.NodeVlan;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;
import org.opendaylight.vtn.manager.internal.util.vnode.VBridgeIdentifier;
import org.opendaylight.vtn.manager.internal.util.vnode.VNodeIdentifier;
import org.opendaylight.vtn.manager.internal.util.vnode.VlanMapIdentifier;

import org.opendaylight.controller.md.sal.binding.api.ReadWriteTransaction;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.mapping.rev151001.vtn.mappings.VlanMapping;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.mapping.rev151001.vtn.mappings.VlanMappingBuilder;

/**
 * {@code VlanMapRegistry} describes a context for registering VLAN mapping.
 */
public final class VlanMapRegistry extends VirtualMapRegistry<VlanMapCleaner> {
    /**
     * The identifier for the target VLAN mapping.
     */
    private final VlanMapIdentifier  targetId;

    /**
     * Construct a new instance.
     *
     * @param ctx    A runtime context for transaction task.
     * @param ident  The identifier for the target VLAN mapping.
     */
    public VlanMapRegistry(TxContext ctx, VlanMapIdentifier ident) {
        super(ctx);
        targetId = ident;
    }

    /**
     * Register a new VLAN mapping.
     *
     * @param nvlan  A {@link NodeVlan} instances which represents VLAN to be
     *               mapped by a new VLAN mapping.
     * @param purge  If {@code true} is specified, network caches
     *               corresponding to VLAN superseded by a new VLAN mapping
     *               will be purged.
     * @throws VTNException  An error occurred.
     */
    public void register(NodeVlan nvlan, boolean purge)
        throws VTNException {
        // Determine whether the specified VLAN is mapped by VLAN mapping
        // or not.
        TxContext ctx = getContext();
        InstanceIdentifier<VlanMapping> path = MappingRegistry.getPath(nvlan);
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        ReadWriteTransaction tx = ctx.getReadWriteTransaction();
        Optional<VlanMapping> opt = DataStoreUtils.read(tx, oper, path);
        if (opt.isPresent()) {
            // Conflicted with another VLAN mapping.
            throw getException(opt.get(), nvlan);
        }

        // Register the new VLAN mapping.
        VlanMapping vmap = new VlanMappingBuilder().
            setIdentifier(nvlan.toString()).
            setMapping(targetId.toString()).
            build();
        tx.put(oper, path, vmap, true);

        if (purge && nvlan.getNode() != null) {
            // Check to see if a new VLAN mappnig supersedes existing
            // VLAN mapping.
            NodeVlan wild = new NodeVlan(null, nvlan.getVlanId());
            opt = MappingRegistry.read(tx, wild);
            if (opt.isPresent()) {
                setMapCleaner(nvlan, opt.get());
            }
        }

        cleanUp();
    }

    /**
     * Create a {@link VlanMapCleaner} instance which represents VLAN
     * superseded by a new VLAN mapping.
     *
     * @param nvlan  A {@link NodeVlan} instance which represents VLAN mapped
     *               mapped by a new VLAN mapping.
     * @param vmap   A {@link VlanMapping} instance associated with the
     *               VLAN mapping superseded by a new VLAN mapping.
     */
    private void setMapCleaner(NodeVlan nvlan, VlanMapping vmap) {
        VlanMapIdentifier ident = VNodeIdentifier.
            create(vmap.getMapping(), VlanMapIdentifier.class);
        String tname = ident.getTenantNameString();
        VlanMapCleaner cleaner = getMapCleaner(tname);
        if (cleaner == null) {
            cleaner = new VlanMapCleaner();
            addMapCleaner(tname, cleaner);
        }

        cleaner.add(nvlan);
    }

    /**
     * Create a new exception that indicates the specified VLAN is already
     * mapped by another VLAN mapping.
     *
     * @param vmap  The VLAN mapping that maps the specified VLAN.
     * @param nvlan  A {@link NodeVlan} instance which specifies the VLAN.
     * @return  An {@link RpcException} instance.
     */
    private RpcException getException(VlanMapping vmap, NodeVlan nvlan) {
        VlanMapIdentifier conflicted =
            VNodeIdentifier.create(vmap.getMapping(), VlanMapIdentifier.class);
        VBridgeIdentifier vbrId = conflicted.getBridgeIdentifier();
        String msg;
        if (vbrId.contains(targetId)) {
            msg = "Already mapped to this vBridge";
        } else {
            msg = "VLAN(" + nvlan + ") is mapped to " + vbrId;
        }

        return RpcException.getDataExistsException(msg);
    }

    // VirtualMapRegistry

    /**
     * This method does nothing.
     *
     * @param ctx  Unused.
     */
    @Override
    protected void cleanUpImpl(TxContext ctx) {
    }
}
