/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.vnode;

import java.util.ArrayList;
import java.util.List;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.TxContext;
import org.opendaylight.vtn.manager.internal.util.MiscUtils;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcOutputGenerator;
import org.opendaylight.vtn.manager.internal.util.tx.AbstractRpcTask;
import org.opendaylight.vtn.manager.internal.util.vnode.VBridgeIdentifier;

import org.opendaylight.controller.md.sal.binding.api.ReadWriteTransaction;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.vlan.rev150907.RemoveVlanMapOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.vlan.rev150907.RemoveVlanMapOutputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.vlan.rev150907.remove.vlan.map.output.RemoveVlanMapResult;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.vlan.rev150907.remove.vlan.map.output.RemoveVlanMapResultBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.vlan.rev150907.vtn.vlan.mappable.VlanMap;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.vbridge.list.Vbridge;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.vbridge.list.VbridgeBuilder;

/**
 * {@code ClearVlanMapTask} describes the MD-SAL datastore transaction task
 * that removes all the VLAN mappings configured in the specified vBridge.
 */
public final class ClearVlanMapTask
    extends AbstractRpcTask<List<RemoveVlanMapResult>>
    implements RpcOutputGenerator<List<RemoveVlanMapResult>,
                                 RemoveVlanMapOutput> {
    /**
     * The identifier for the target vBridge.
     */
    private final VBridgeIdentifier  identifier;

    /**
     * Construct a new instance.
     *
     * @param ident  A {@link VBridgeIdentifier} instance that specifies the
     *               target vBridge.
     */
    ClearVlanMapTask(VBridgeIdentifier ident) {
        identifier = ident;
    }

    // AbstractTxTask

    /**
     * Clear all the VLAN mappings configured in the specified vBridge.
     *
     * @param ctx  A runtime context for transaction task.
     * @return  A list of {@link RemoveVlanMapResult} instances that indicates
     *          removed VLAN mappings.
     *          {@code null} if no VLAN mapping is configured in the specified
     *          vBridge.
     * @throws VTNException  An error occurred.
     */
    @Override
    protected List<RemoveVlanMapResult> execute(TxContext ctx)
        throws VTNException {
        // Ensure that the target vBridge is present.
        ReadWriteTransaction tx = ctx.getReadWriteTransaction();
        Vbridge vbridge = identifier.fetch(tx);

        List<VlanMap> vlmaps = vbridge.getVlanMap();
        List<RemoveVlanMapResult> result;
        if (MiscUtils.isEmpty(vlmaps)) {
            // No VLAN map is configured.
            result = null;
        } else {
            // Destroy all the VLAN mappings.
            result = new ArrayList<>(vlmaps.size());
            for (VlanMap vlmap: vlmaps) {
                VTNVlanMap vmap = new VTNVlanMap(identifier, vlmap);
                vmap.destroy(ctx, true);
                RemoveVlanMapResult res = new RemoveVlanMapResultBuilder().
                    setMapId(vlmap.getMapId()).
                    setStatus(VtnUpdateType.REMOVED).
                    build();
                result.add(res);
            }

            // Update the status of the target vBridge.
            Vbridge newVbr = new VbridgeBuilder(vbridge).
                setVlanMap(null).build();
            VBridge vbr = new VBridge(identifier, newVbr);
            vbr.updateState(ctx);
            LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
            tx.put(oper, identifier.getIdentifier(), vbr.getVbridge(), false);
        }

        return result;
    }

    // RpcOutputGenerator

    /**
     * {@inheritDoc}
     */
    @Override
    public Class<RemoveVlanMapOutput> getOutputType() {
        return RemoveVlanMapOutput.class;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public RemoveVlanMapOutput createOutput(List<RemoveVlanMapResult> result) {
        return new RemoveVlanMapOutputBuilder().
            setRemoveVlanMapResult(result).build();
    }
}
