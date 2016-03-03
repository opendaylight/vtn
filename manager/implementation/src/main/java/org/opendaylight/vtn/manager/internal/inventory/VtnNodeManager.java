/*
 * Copyright (c) 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.inventory;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Set;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import org.opendaylight.vtn.manager.internal.util.inventory.NodeRpcInvocation;
import org.opendaylight.vtn.manager.internal.util.inventory.NodeRpcWatcher;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.nodes.VtnNode;

/**
 * {@code NodeManager} keeps all the VTN nodes in the vtn-nodes container.
 */
public final class VtnNodeManager implements NodeRpcWatcher {
    /**
     * Logger instance.
     */
    private static final Logger  LOG =
        LoggerFactory.getLogger(VtnNodeManager.class);

    /**
     * A map that keeps existing VTN nodes and ongoing RPC invocations.
     */
    private final Map<String, Set<NodeRpcInvocation<?, ?>>>  nodeMap =
        new HashMap<>();

    /**
     * Add the specified VTN node information.
     *
     * @param vnode  A {@link VtnNode} instance.
     * @return  The node-id value if the specified VTN node was actually added.
     *          {@code null} otherwise.
     */
    public String add(VtnNode vnode) {
        String id = vnode.getId().getValue();

        synchronized (nodeMap) {
            Set<NodeRpcInvocation<?, ?>> rpcs = nodeMap.get(id);
            if (rpcs == null) {
                rpcs = new HashSet<>();
                nodeMap.put(id, rpcs);
            } else {
                id = null;
            }
        }

        return id;
    }

    /**
     * Remove the specified VTN node information.
     *
     * <p>
     *   This method cancels all the RPC invocations routed to the specified
     *   node.
     * </p>
     *
     * @param vnode  A {@link VtnNode} instance.
     * @return  The node-id value if the specified VTN node was actually
     *          removed. {@code null} otherwise.
     */
    public String remove(VtnNode vnode) {
        String id = vnode.getId().getValue();
        Set<NodeRpcInvocation<?, ?>> rpcs;

        synchronized (nodeMap) {
            rpcs = nodeMap.remove(id);
        }
        if (rpcs == null) {
            id = null;
        } else {
            for (NodeRpcInvocation<?, ?> rpc: rpcs) {
                cancel(id, rpc);
            }
        }

        return id;
    }

    /**
     * Close the VTN node management.
     */
    public void close() {
        List<NodeRpcInvocation<?, ?>> rpcs = new ArrayList<>();
        synchronized (nodeMap) {
            for (Iterator<Set<NodeRpcInvocation<?, ?>>> it =
                     nodeMap.values().iterator(); it.hasNext();) {
                rpcs.addAll(it.next());
                it.remove();
            }
        }

        for (NodeRpcInvocation<?, ?> rpc: rpcs) {
            cancel(rpc.getNode(), rpc);
        }
    }

    /**
     * Return an unmodifiable map that contains ongoing RPC invocations
     * only for testing.
     *
     * @return  An unmodifiable map that contains ongoing RPC invocations.
     */
    synchronized Map<String, Set<NodeRpcInvocation<?, ?>>> getNodeMap() {
        return Collections.unmodifiableMap(nodeMap);
    }

    /**
     * Cancel the specified RPC invocation.
     *
     * @param id   The node identifier that specifies the target switch.
     * @param rpc  The RPC invocation to be canceled.
     */
    private void cancel(String id, NodeRpcInvocation<?, ?> rpc) {
        if (rpc.onNodeRemoved()) {
            LOG.debug("{}: RPC has been canceled: node={}, input={}",
                      rpc.getName(), id, rpc.getInputForLog());
        }
    }

    // NodeRpcWatcher

    /**
     * {@inheritDoc}
     */
    @Override
    public void registerRpc(NodeRpcInvocation<?, ?> rpc) {
        Set<NodeRpcInvocation<?, ?>> rpcs;
        String id = rpc.getNode();

        synchronized (nodeMap) {
            rpcs = nodeMap.get(id);
            if (rpcs != null) {
                rpcs.add(rpc);
            }
        }

        if (rpcs == null) {
            cancel(id, rpc);
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void unregisterRpc(NodeRpcInvocation<?, ?> rpc) {
        String id = rpc.getNode();

        synchronized (nodeMap) {
            Set<NodeRpcInvocation<?, ?>> rpcs = nodeMap.get(id);
            if (rpcs != null) {
                rpcs.remove(rpc);
            }
        }
    }
}
