/*
 * Copyright (c) 2015, 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.flow.remove;

import java.util.List;

import com.google.common.base.Optional;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.FlowRemover;
import org.opendaylight.vtn.manager.internal.TxContext;
import org.opendaylight.vtn.manager.internal.util.DataStoreUtils;
import org.opendaylight.vtn.manager.internal.util.flow.FlowCache;
import org.opendaylight.vtn.manager.internal.util.flow.FlowUtils;
import org.opendaylight.vtn.manager.internal.util.vnode.VNodeIdentifier;

import org.opendaylight.controller.md.sal.binding.api.ReadWriteTransaction;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.tenant.flow.info.VtnDataFlow;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.vtn.flows.VtnFlowTable;

/**
 * An implementation of {@link FlowRemover} which removes all VTN data flows
 * in the specified VTN.
 */
public final class TenantFlowRemover implements FlowRemover {
    /**
     * The name of the target VTN.
     */
    private final String  tenantName;

    /**
     * Construct a new instance.
     *
     * @param tname  The name of the target VTN.
     */
    public TenantFlowRemover(String tname) {
        tenantName = tname;
    }

    /**
     * Construct a new instance.
     *
     * @param ident  The identifier for the virtual node in the target VTN.
     */
    public TenantFlowRemover(VNodeIdentifier<?> ident) {
        this(ident.getTenantNameString());
    }

    // FlowRemover

    /**
     * {@inheritDoc}
     */
    @Override
    public RemovedDataFlows<TenantFlowRemover> removeDataFlow(TxContext ctx)
        throws VTNException {
        ReadWriteTransaction tx = ctx.getReadWriteTransaction();
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        InstanceIdentifier<VtnFlowTable> path =
            FlowUtils.getIdentifier(tenantName);
        Optional<VtnFlowTable> opt = DataStoreUtils.read(tx, oper, path);
        RemovedDataFlows<TenantFlowRemover> removed =
            new RemovedDataFlows<>(ctx, this);
        if (opt.isPresent()) {
            // Delete the VTN flow table.
            VtnFlowTable table = opt.get();
            tx.delete(oper, path);

            // Collect data flows to be removed from switches.
            List<VtnDataFlow> flows = table.getVtnDataFlow();
            if (flows != null) {
                for (VtnDataFlow vdf: flows) {
                    removed.add(new FlowCache(vdf));
                }
            }
        }

        return removed;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public String getDescription() {
        return new StringBuilder("tenant[").append(tenantName).append(']').
            toString();
    }
}
