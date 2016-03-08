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

import org.opendaylight.controller.md.sal.binding.api.ReadWriteTransaction;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.VtnFlows;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.VtnFlowsBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.tenant.flow.info.VtnDataFlow;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.vtn.flows.VtnFlowTable;

/**
 * An implementation of {@link FlowRemover} which removes all VTN data flows
 * configured in all the VTNs.
 *
 * <ul>
 *   <li>
 *     This flow remover affects flow entries in all the VTNs.
 *   </li>
 *   <li>
 *     This flow remover never removes table miss flow entries from OF1.3
 *     switches.
 *   </li>
 * </ul>
 */
public final class AllFlowRemover implements FlowRemover {
    /**
     * A brief description about this flow remover.
     */
    static final String  DESCRIPTION = "all";

    // FlowRemover

    /**
     * {@inheritDoc}
     */
    @Override
    public RemovedAllFlows removeDataFlow(TxContext ctx) throws VTNException {
        ReadWriteTransaction tx = ctx.getReadWriteTransaction();
        InstanceIdentifier<VtnFlows> path =
            InstanceIdentifier.create(VtnFlows.class);
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        RemovedAllFlows removed = new RemovedAllFlows();

        // Read all the VTN flow tables.
        Optional<VtnFlows> opt = DataStoreUtils.read(tx, oper, path);
        boolean doRemove = false;
        if (opt.isPresent()) {
            // Collect data flows to be removed from switches.
            List<VtnFlowTable> tables = opt.get().getVtnFlowTable();
            if (tables != null) {
                for (VtnFlowTable table: tables) {
                    doRemove = true;
                    List<VtnDataFlow> flows = table.getVtnDataFlow();
                    if (flows != null) {
                        for (VtnDataFlow vdf: flows) {
                            removed.add(new FlowCache(vdf));
                        }
                    }
                }
            }
        }

        if (doRemove) {
            // Set an empty VTN table list.
            VtnFlows root = new VtnFlowsBuilder().build();
            tx.put(oper, path, root, true);
        }

        return removed;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public String getDescription() {
        return DESCRIPTION;
    }
}
