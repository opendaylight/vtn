/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.flow.reader;

import java.util.ArrayList;
import java.util.List;

import javax.annotation.Nonnull;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import com.google.common.base.Optional;
import com.google.common.util.concurrent.ListenableFuture;
import com.google.common.util.concurrent.FutureCallback;
import com.google.common.util.concurrent.Futures;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.TxContext;
import org.opendaylight.vtn.manager.internal.TxQueue;
import org.opendaylight.vtn.manager.internal.flow.stats.FlowStatsReader;
import org.opendaylight.vtn.manager.internal.flow.stats.StatsReaderService;
import org.opendaylight.vtn.manager.internal.util.flow.FlowCache;
import org.opendaylight.vtn.manager.internal.util.flow.FlowUtils;
import org.opendaylight.vtn.manager.internal.util.inventory.MacVlan;
import org.opendaylight.vtn.manager.internal.util.inventory.SalNode;
import org.opendaylight.vtn.manager.internal.util.inventory.SalPort;

import org.opendaylight.controller.md.sal.binding.api.ReadTransaction;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.GetDataFlowInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.VtnFlowId;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.get.data.flow.output.DataFlowInfo;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.FlowIdSet;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.flow.id.set.FlowIdList;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.tenant.flow.info.NodeFlows;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.tenant.flow.info.PortFlows;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.tenant.flow.info.SourceHostFlows;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.tenant.flow.info.SourceHostFlowsKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.tenant.flow.info.VtnDataFlow;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.vtn.flows.VtnFlowTable;

/**
 * {@code ReadDataFlowFuture} describes a future associated with the task which
 * reads information about data flows present in the VTN.
 */
public final class ReadDataFlowFuture extends ReadFlowFuture
    implements FutureCallback<Optional<VtnFlowTable>> {
    /**
     * An implementation of {@link FutureCallback} that will be called when
     * the read operation for flow index has completed.
     *
     * @param <T>  The type of the flow index.
     */
    private final class IndexCallback<T extends FlowIdSet>
        implements FutureCallback<Optional<T>> {
        /**
         * Start read operation, and add a new callback for the operation.
         *
         * @param path  Path to the flow index.
         */
        private IndexCallback(InstanceIdentifier<T> path) {
            ReadTransaction rtx = getTxContext().getTransaction();
            LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
            Futures.addCallback(rtx.read(oper, path), this);
        }

        /**
         * Invoked when the read operation has completed successfully.
         *
         * @param result  An {@link Optional} instance that contains the
         *                flow index.
         */
        @Override
        public void onSuccess(@Nonnull Optional<T> result) {
            List<FlowIdList> list;
            if (result.isPresent()) {
                list = result.get().getFlowIdList();
            } else if (checkTenant()) {
                list = null;
            } else {
                // The target VTN is not present.
                return;
            }

            if (list == null || list.isEmpty()) {
                // No data flow is indexed.
                notFound();
                return;
            }

            // Read data flows.
            ReadTransaction rtx = getTxContext().getTransaction();
            LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
            List<ListenableFuture<Optional<VtnDataFlow>>> fl =
                new ArrayList<>();
            List<VtnFlowId> flowIds = new ArrayList<>();
            for (FlowIdList ids: list) {
                VtnFlowId flowId = ids.getFlowId();
                flowIds.add(flowId);
                InstanceIdentifier<VtnDataFlow> path = FlowUtils.
                    getIdentifier(getTenantName(), flowId);
                fl.add(rtx.read(oper, path));
            }

            Futures.addCallback(Futures.allAsList(fl),
                                new FlowReadCallback(flowIds));
        }

        /**
         * Invoked when the read operation has failed.
         *
         * @param cause  A {@link Throwable} that indicates the cause of
         *               failure.
         */
        @Override
        public void onFailure(Throwable cause) {
            setFailure(cause);
        }
    }

    /**
     * An implementation of {@link FutureCallback} that will be called when
     * all the specified read operations have completed.
     */
    private final class FlowReadCallback
        implements FutureCallback<List<Optional<VtnDataFlow>>> {
        /**
         * A list of data flows.
         */
        private List<VtnFlowId>  flowIds;

        /**
         * Construct a new instance.
         *
         * @param ids  A list of data flow IDs.
         */
        private FlowReadCallback(List<VtnFlowId> ids) {
            flowIds = ids;
        }

        /**
         * Invoked when the read operation has completed successfully.
         *
         * @param result  An {@link Optional} instance that contains the
         *                data flows.
         */
        @Override
        public void onSuccess(@Nonnull List<Optional<VtnDataFlow>> result) {
            int size = result.size();
            List<DataFlowInfo> flows = new ArrayList<>(size);
            int index = 0;
            FlowStatsReader stReader = getFlowStatsReader(result.size());
            for (Optional<VtnDataFlow> opt: result) {
                if (opt.isPresent()) {
                    addDataFlowInfo(flows, opt.get(), stReader);
                } else {
                    // This should never happen.
                    Logger logger = LoggerFactory.
                        getLogger(ReadDataFlowFuture.class);
                    logger.warn("Data flow index corrupted: {}",
                                flowIds.get(index));
                }

                index++;
            }

            setResult(flows);
        }

        /**
         * Invoked when the read operation has failed.
         *
         * @param cause  A {@link Throwable} that indicates the cause of
         *               failure.
         */
        @Override
        public void onFailure(Throwable cause) {
            setFailure(cause);
        }
    }

    /**
     * Construct a new instance.
     *
     * @param ctx    A {@link TxContext} instance.
     * @param txq    A {@link TxQueue} instance used to update the MD-SAL
     *               datastore.
     * @param srs    The flow statistics reader service.
     * @param input  Input of the RPC call.
     * @throws VTNException  An error occurred.
     */
    public ReadDataFlowFuture(TxContext ctx, TxQueue txq,
                              StatsReaderService srs, GetDataFlowInput input)
        throws VTNException {
        super(ctx, txq, srs, input);

        // Determine flow index to be used.
        if (!isDone() && readIndex(input) == null) {
            // Scan all the data flows present in the VTN.
            ReadTransaction rtx = getTxContext().getTransaction();
            LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
            InstanceIdentifier<VtnFlowTable> path = FlowUtils.
                getIdentifier(input.getTenantName());
            Futures.addCallback(rtx.read(oper, path), this);
        }
    }

    /**
     * Read data flows using flow index.
     *
     * @param input  Input of the RPC call.
     * @return  An {@link IndexCallback} instance associated with the task for
     *          index read operation. {@code null} if no index should be used.
     */
    private IndexCallback<?> readIndex(GetDataFlowInput input) {
        MacVlan src = getSourceHost();
        if (src != null) {
            // Use source host inedx.
            SourceHostFlowsKey hostId = src.getSourceHostFlowsKey();
            clearSourceHost();
            InstanceIdentifier<SourceHostFlows> path =
                FlowUtils.getIdentifier(input.getTenantName(), hostId);
            return new IndexCallback<SourceHostFlows>(path);
        }

        SalPort sport = getFlowPort();
        if (sport != null) {
            // Use switch port index.
            clearFlowPort();
            InstanceIdentifier<PortFlows> path = FlowUtils.getIdentifier(
                input.getTenantName(), sport.getNodeConnectorId());
            return new IndexCallback<PortFlows>(path);
        }

        SalNode snode = getFlowNode();
        if (snode != null) {
            // Use switch index.
            clearFlowNode();
            InstanceIdentifier<NodeFlows> path = FlowUtils.getIdentifier(
                input.getTenantName(), snode.getNodeId());
            return new IndexCallback<NodeFlows>(path);
        }

        return null;
    }

    /**
     * Return a flow statistics reader.
     *
     * @param count  The estimated number of flow entries present in switches.
     * @return  A {@link FlowStatsReader} service.
     */
    private FlowStatsReader getFlowStatsReader(int count) {
        StatsReaderService srs = getStatsReaderService();
        return FlowStatsReader.getInstance(srs, count);
    }

    /**
     * Convert the given VTN data flow into a {@link DataFlowInfo} instance,
     * and add it to the given list.
     *
     * @param list      A list to store converted instances.
     * @param vdf       A {@link VtnDataFlow} instance.
     * @param stReader  A {@link FlowStatsReader} instance used to update
     *                  flow statistics.
     */
    private void addDataFlowInfo(List<DataFlowInfo> list, VtnDataFlow vdf,
                                 FlowStatsReader stReader) {
        FlowCache fc = new FlowCache(vdf);
        if (select(fc)) {
            DataFlowInfo df = toDataFlowInfo(fc, stReader.get(fc));
            if (df != null) {
                list.add(df);
            }
        }
    }

    // FutureCallback

    /**
     * Invoked when the read operation has completed successfully.
     *
     * @param result  An {@link Optional} instance that contains the VTN flow
     *                table.
     */
    @Override
    public void onSuccess(@Nonnull Optional<VtnFlowTable> result) {
        if (result.isPresent()) {
            // Select data flows which meet the condition.
            List<VtnDataFlow> flows = result.get().getVtnDataFlow();
            if (flows != null) {
                int size = flows.size();
                FlowStatsReader stReader = getFlowStatsReader(size);
                List<DataFlowInfo> list = new ArrayList<>(size);
                for (VtnDataFlow vdf: flows) {
                    addDataFlowInfo(list, vdf, stReader);
                }
                set(list);
                return;
            }
        } else if (!checkTenant()) {
            // The target VTN is not present.
            return;
        }

        notFound();
    }

    /**
     * Invoked when the read operation has failed.
     *
     * @param cause  A {@link Throwable} that indicates the cause of failure.
     */
    @Override
    public void onFailure(Throwable cause) {
        setFailure(cause);
    }
}
