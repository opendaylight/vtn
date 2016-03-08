/*
 * Copyright (c) 2015, 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.flow.remove;

import static org.opendaylight.vtn.manager.internal.flow.remove.FlowRemoveContext.LOG;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.RemovedFlows;
import org.opendaylight.vtn.manager.internal.TxContext;
import org.opendaylight.vtn.manager.internal.util.flow.FlowCache;
import org.opendaylight.vtn.manager.internal.util.flow.FlowUtils;
import org.opendaylight.vtn.manager.internal.util.flow.RemoveFlowRpcList;
import org.opendaylight.vtn.manager.internal.util.inventory.InventoryReader;
import org.opendaylight.vtn.manager.internal.util.inventory.SalNode;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.vtn.data.flow.fields.VtnFlowEntry;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.VtnOpenflowVersion;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.nodes.VtnNode;

import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.service.rev130819.RemoveFlowInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.service.rev130819.SalFlowService;

/**
 * {@link RemovedAllFlows} describes flow entries removed by
 * {@link AllFlowRemover}.
 */
public final class RemovedAllFlows implements RemovedFlows {
    /**
     * A map that keeps flow entries per switches.
     */
    private final Map<SalNode, List<VtnFlowEntry>>  flowEntries =
        new HashMap<>();

    /**
     * A list of removed VTN data flows.
     */
    private final List<FlowCache>  removedFlows;

    /**
     * Consruct an empty instance.
     */
    public RemovedAllFlows() {
        removedFlows = (LOG.isDebugEnabled())
            ? new ArrayList<FlowCache>()
            : null;
    }

    /**
     * Add the specified data flow to this instance.
     *
     * @param fc  A {@link FlowCache} instance which contains the VTN data flow
     *            to be removed.
     */
    void add(FlowCache fc) {
        for (VtnFlowEntry vfent: fc.getFlowEntries()) {
            SalNode snode = SalNode.create(vfent.getNode());
            List<VtnFlowEntry> entries = flowEntries.get(snode);
            if (entries == null) {
                entries = new ArrayList<>();
                flowEntries.put(snode, entries);
            }
            entries.add(vfent);
        }

        if (removedFlows != null) {
            removedFlows.add(fc);
        }
    }

    // RemovedFlows

    /**
     * {@inheritDoc}
     */
    @Override
    public RemoveFlowRpcList removeFlowEntries(TxContext ctx,
                                               SalFlowService sfs)
        throws VTNException {
        if (removedFlows != null) {
            if (LOG.isDebugEnabled()) {
                String desc = AllFlowRemover.DESCRIPTION;
                for (FlowCache fc: removedFlows) {
                    FlowUtils.removedLog(LOG, desc, fc);
                }
            }
        }

        InventoryReader reader = ctx.getReadSpecific(InventoryReader.class);
        RemoveFlowRpcList rpcs = new RemoveFlowRpcList(ctx.getProvider(), sfs);
        for (Map.Entry<SalNode, List<VtnFlowEntry>> entry:
                 flowEntries.entrySet()) {
            SalNode snode = entry.getKey();
            VtnNode vnode = reader.get(snode);
            if (vnode == null) {
                continue;
            }

            if (vnode.getOpenflowVersion() == VtnOpenflowVersion.OF10) {
                LOG.trace("Remove all flow entries individually: {}", snode);
                List<VtnFlowEntry> list = entry.getValue();
                for (VtnFlowEntry vfent: list) {
                    RemoveFlowInputBuilder builder = FlowUtils.
                        createRemoveFlowInputBuilder(snode, vfent);
                    rpcs.invoke(builder);
                }
            } else {
                LOG.trace("Remove all flow entries by cookie mask: {}", snode);
                RemoveFlowInputBuilder builder =
                    FlowUtils.createRemoveFlowInputBuilder(snode);
                rpcs.invoke(builder);
            }
        }

        return rpcs.flush();
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public boolean isEmpty() {
        return flowEntries.isEmpty();
    }
}
