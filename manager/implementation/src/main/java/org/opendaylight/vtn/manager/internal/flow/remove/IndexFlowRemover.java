/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
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
import org.opendaylight.vtn.manager.internal.RemovedFlows;
import org.opendaylight.vtn.manager.internal.util.DataStoreUtils;
import org.opendaylight.vtn.manager.internal.util.flow.FlowCache;
import org.opendaylight.vtn.manager.internal.util.flow.FlowUtils;

import org.opendaylight.controller.md.sal.binding.api.ReadWriteTransaction;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.VtnFlowId;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.FlowIdSet;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.flow.id.set.FlowIdList;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.flow.id.set.FlowIdListKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.tenant.flow.info.VtnDataFlow;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.tenant.flow.info.VtnDataFlowKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.vtn.flows.VtnFlowTable;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.vtn.flows.VtnFlowTableKey;

/**
 * An implementation of {@link FlowRemover} which removes all VTN data flows
 * in the specified flow index.
 *
 * <p>
 *   This flow remover affects flow entries in all the VTNs.
 * </p>
 *
 * @param <T>  The type of the flow index.
 * @param <R>  The type of instance which specifies flow entries to be removed.
 */
public abstract class IndexFlowRemover<T extends FlowIdSet, R extends RemovedFlows>
    implements FlowRemover {
    /**
     * Return the path to the flow index to be removed.
     *
     * @param key  A {@link VtnFlowTableKey} instance which specifies the
     *             target VTN flow table.
     * @return  Path to the flow index to be removed.
     */
    protected abstract InstanceIdentifier<T> getPath(VtnFlowTableKey key);

    /**
     * Invoked when the given data flow has been removed.
     *
     * @param removed  A {@link RemovedFlows} instance to store flow entries
     *                 to be removed.
     * @param fc       A {@link FlowCache} instance which contains VTN data
     *                 flow to be removed.
     */
    protected abstract void add(R removed, FlowCache fc);

    /**
     * Remove VTN data flows related to the target node in the given VTN
     * flow table.
     *
     * @param removed  A {@link RemovedFlows} instance to store flow entries
     *                 to be removed.
     * @param tx       A {@link ReadWriteTransaction} instance.
     * @param table    The target VTN flow table.
     * @throws VTNException  An error occurred.
     */
    protected final void remove(R removed, ReadWriteTransaction tx,
                                VtnFlowTable table) throws VTNException {
        InstanceIdentifier<T> path = getPath(table.getKey());
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        Optional<T> opt = DataStoreUtils.read(tx, oper, path);
        if (!opt.isPresent()) {
            return;
        }

        // Remove the target index.
        tx.delete(oper, path);

        List<FlowIdList> idList = opt.get().getFlowIdList();
        if (idList != null) {
            // Remove data flows.
            InstanceIdentifier<VtnFlowTable> tpath =
                path.firstIdentifierOf(VtnFlowTable.class);
            for (FlowIdList idl: idList) {
                VtnFlowId flowId = idl.getFlowId();
                remove(removed, tx, tpath, flowId);
            }
        }
    }

    /**
     * Remove the given VTN data flow from the MD-SAL datastore.
     *
     * @param removed  A {@link RemovedFlows} instance to store flow entries
     *                 to be removed.
     * @param tx       A {@link ReadWriteTransaction} instance.
     * @param tpath    Path to the VTN flow table in the DS.
     * @param flowId   Identifier for a VTN data flow to be removed.
     * @throws VTNException  An error occurred.
     */
    private void remove(R removed, ReadWriteTransaction tx,
                        InstanceIdentifier<VtnFlowTable> tpath,
                        VtnFlowId flowId) throws VTNException {
        // Read the VTN data flow to be removed.
        InstanceIdentifier<VtnDataFlow> fpath =
            tpath.child(VtnDataFlow.class, new VtnDataFlowKey(flowId));
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        Optional<VtnDataFlow> opt = DataStoreUtils.read(tx, oper, fpath);
        if (opt.isPresent()) {
            // Remove the VTN data flow from the DS.
            FlowCache fc = new FlowCache(opt.get());
            add(removed, fc);
            tx.delete(oper, fpath);

            // Clean up indices.
            FlowUtils.removeIndex(tx, tpath, fc, new FlowIdListKey(flowId));
        }
    }
}
