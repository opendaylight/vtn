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
import org.opendaylight.vtn.manager.internal.TxContext;
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
 * An implementation of {@link FlowRemover} which removes VTN data flows
 * in the specified VTN using flow index.
 *
 * @param <T>  The type of the flow index.
 */
public abstract class TenantIndexFlowRemover<T extends FlowIdSet>
    implements FlowRemover {
    /**
     * The name of the target VTN.
     */
    private final String  tenantName;

    /**
     * Construct a new instance.
     *
     * @param tname  The name of the target VTN.
     */
    protected TenantIndexFlowRemover(String tname) {
        tenantName = tname;
    }

    /**
     * Return the name of the target VTN.
     *
     * @return  The name of the target VTN.
     */
    public final String getTenantName() {
        return tenantName;
    }

    /**
     * Return the path to the flow index to be removed.
     *
     * @param key  A {@link VtnFlowTableKey} instance which specifies the
     *             target VTN flow table.
     * @return  Path to the flow index to be removed.
     */
    protected abstract InstanceIdentifier<T> getPath(VtnFlowTableKey key);

    /**
     * Determine whether the given data flow should be removed or not.
     *
     * @param fc  A {@link FlowCache} instance.
     * @return  {@code true} if the given data flow should be removed.
     *          Otherwise {@code false}.
     * @throws VTNException  An error occurred.
     */
    protected abstract boolean select(FlowCache fc) throws VTNException;

    /**
     * Remove the given VTN data flow from the MD-SAL datastore.
     *
     * @param removed  A {@link RemovedDataFlows} instance to store flow
     *                 entries to be removed.
     * @param tx       A {@link ReadWriteTransaction} instance.
     * @param tpath    Path to the VTN flow table in the DS.
     * @param idKey    A {@link FlowIdListKey} instance which contains the
     *                 data flow identifier.
     * @throws VTNException  An error occurred.
     */
    private void remove(RemovedDataFlows removed, ReadWriteTransaction tx,
                        InstanceIdentifier<VtnFlowTable> tpath,
                        FlowIdListKey idKey) throws VTNException {
        // Read the VTN data flow to be removed.
        VtnFlowId flowId = idKey.getFlowId();
        InstanceIdentifier<VtnDataFlow> fpath =
            tpath.child(VtnDataFlow.class, new VtnDataFlowKey(flowId));
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        Optional<VtnDataFlow> opt = DataStoreUtils.read(tx, oper, fpath);
        if (opt.isPresent()) {
            FlowCache fc = new FlowCache(opt.get());
            if (select(fc)) {
                // Remove this data flow.
                removed.add(fc);
                tx.delete(oper, fpath);

                // Clean up indices.
                FlowUtils.removeIndex(tx, tpath, fc, idKey);
            }
        }
    }

    // FlowRemover

    /**
     * {@inheritDoc}
     */
    @Override
    public final RemovedDataFlows removeDataFlow(TxContext ctx)
        throws VTNException {
        // Read flow IDs in the flow index.
        ReadWriteTransaction tx = ctx.getReadWriteTransaction();
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        InstanceIdentifier<T> path = getPath(new VtnFlowTableKey(tenantName));
        Optional<T> opt = DataStoreUtils.read(tx, oper, path);
        RemovedDataFlows removed =
            new RemovedDataFlows<TenantIndexFlowRemover>(this);
        if (opt.isPresent()) {
            List<FlowIdList> idList = opt.get().getFlowIdList();
            if (idList != null) {
                // Test data flows in the flow index.
                InstanceIdentifier<VtnFlowTable> tpath =
                    path.firstIdentifierOf(VtnFlowTable.class);
                for (FlowIdList idl: idList) {
                    remove(removed, tx, tpath, idl.getKey());
                }
            }
        }

        return removed;
    }
}
