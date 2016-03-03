/*
 * Copyright (c) 2015, 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.flow;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import javax.annotation.Nonnull;

import org.opendaylight.vtn.manager.internal.util.inventory.NodeRpcWatcher;
import org.opendaylight.vtn.manager.internal.util.inventory.SalNode;

import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.service.rev130819.RemoveFlowInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.service.rev130819.SalFlowService;

/**
 * {@code RemoveFlowRpcList} describes a list of RPC calls that remove flow
 * entries in a single transaction.
 */
public final class RemoveFlowRpcList {
    /**
     * The node-routed RPC watcher.
     */
    private final NodeRpcWatcher  rpcWatcher;

    /**
     * A list of remove-flow RPC input builder.
     */
    private final List<RemoveFlowInputBuilder>  inputBuilders =
        new ArrayList<>();

    /**
     * RPC input builders to be configured barrier request.
     */
    private final Map<SalNode, RemoveFlowInputBuilder>  lastRequests =
        new HashMap<>();

    /**
     * Construct a new instance.
     *
     * @param watcher  The node-routed RPC watcher.
     */
    public RemoveFlowRpcList(NodeRpcWatcher watcher) {
        rpcWatcher = watcher;
    }

    /**
     * Add the given remove-flow RPC input builder.
     *
     * @param snode    A {@link SalNode} that specifies the target switch.
     * @param builder  A {@link RemoveFlowInputBuilder} instance.
     */
    public void add(@Nonnull SalNode snode,
                    @Nonnull RemoveFlowInputBuilder builder) {
        // Set barrier flag only into the last request.
        inputBuilders.add(builder.setBarrier(true));
        RemoveFlowInputBuilder old = lastRequests.put(snode, builder);
        if (old != null) {
            old.setBarrier(false);
        }
    }

    /**
     * Invoke remove-flow RPC.
     *
     * @param sfs  MD-SAL flow service.
     * @return  A list of {@link RemoveFlowRpc} instances.
     */
    public List<RemoveFlowRpc> invoke(SalFlowService sfs) {
        List<RemoveFlowRpc> rpcs = new ArrayList<>(inputBuilders.size());
        for (RemoveFlowInputBuilder builder: inputBuilders) {
            rpcs.add(new RemoveFlowRpc(rpcWatcher, sfs, builder.build()));
        }

        return rpcs;
    }
}
