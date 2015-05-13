/*
 * Copyright (c) 2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.inventory;

import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Set;

import com.google.common.base.Optional;

import org.opendaylight.vtn.manager.SwitchPort;
import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.PortFilter;
import org.opendaylight.vtn.manager.internal.util.DataStoreUtils;

import org.opendaylight.controller.md.sal.binding.api.ReadTransaction;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.controller.sal.core.Node;
import org.opendaylight.controller.sal.core.NodeConnector;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.VtnNodes;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.node.info.VtnPort;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.nodes.VtnNode;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnSwitchPort;

/**
 * Helper class to read VTN node and port information using the given
 * MD-SAL datastore transaction.
 *
 * <p>
 *   Note that this class is not synchronized.
 * </p>
 */
public final class InventoryReader {
    /**
     * Cached VTN node information.
     */
    private final Map<SalNode, NodeCacheEntry>  nodeCache = new HashMap<>();

    /**
     * Cached VTN port information.
     */
    private final Map<SalPort, VtnPort>  portCache = new HashMap<>();

    /**
     * Read transaction.
     */
    private final ReadTransaction  readTx;

    /**
     * Set {@code true} if all the nodes are cached.
     */
    private boolean  allCached;

    /**
     * A node cache entry.
     */
    private static final class NodeCacheEntry {
        /**
         * A {@link VtnNode} instance which represents node information.
         */
        private final VtnNode  vtnNode;

        /**
         * Pairs of {@link SalPort} and {@link VtnPort} instances in the
         * VTN node.
         */
        private final Map<SalPort, VtnPort>  cache =
            new HashMap<SalPort, VtnPort>();

        /**
         * Pairs of port names and {@link SalPort} instances in the VTN node.
         */
        private final Map<String, SalPort>  nameCache =
            new HashMap<String, SalPort>();

        /**
         * Construct a new instance.
         *
         * @param vnode  A {@link VtnNode} instance.
         */
        private NodeCacheEntry(VtnNode vnode) {
            vtnNode = vnode;
            List<VtnPort> vports = vnode.getVtnPort();
            if (vports != null) {
                for (VtnPort vport: vports) {
                    SalPort sport = SalPort.create(vport.getId());
                    putSalPort(sport, vport);
                }
            }
        }

        /**
         * Return a {@link VtnNode} instance.
         *
         * @return  A {@link VtnNode} instance.
         */
        private VtnNode getVtnNode() {
            return vtnNode;
        }

        /**
         * Return a {@link SalPort} instance associated with the given name.
         *
         * @param name  The port name.
         * @return  A {@link SalPort} instance if found.
         *          {@code null} if not found.
         */
        private SalPort getSalPort(String name) {
            return nameCache.get(name);
        }

        /**
         * Add a cache entry for the given port.
         *
         * @param sport  A {@link SalPort} instance.
         * @param vport  A {@link VtnPort} instance.
         */
        private void putSalPort(SalPort sport, VtnPort vport) {
            cache.put(sport, vport);

            String name = vport.getName();
            if (name != null) {
                nameCache.put(name, sport);
            }
        }

        /**
         * Determine whether the switch port specified by the given
         * {@link SalPort} is present in the node or not.
         *
         * @param sport  A {@link SalPort} instance.
         * @return  {@code true} if this node contains the specified port.
         *          Otherwise {@code false}.
         */
        private boolean contains(SalPort sport) {
            return cache.containsKey(sport);
        }

        /**
         * Collect node connectors associated with edge switch ports in
         * up state.
         *
         * @param portSet  A set of {@link NodeConnector} instances to store
         *                 results.
         * @param filter   A {@link PortFilter} instance which filters switch
         *                 port. All switch ports are stored to {@code portSet}
         *                 if {@code null} is specified.
         */
        private void collectUpEdgePorts(Set<NodeConnector> portSet,
                                        PortFilter filter) {
            for (Map.Entry<SalPort, VtnPort> ent: cache.entrySet()) {
                VtnPort vport = ent.getValue();
                if (InventoryUtils.isEnabledEdge(vport)) {
                    SalPort sport = ent.getKey();
                    NodeConnector nc = sport.getAdNodeConnector();
                    if (filter == null || filter.accept(nc, vport)) {
                        portSet.add(nc);
                    }
                }
            }
        }
    }

    /**
     * Construct a new instance.
     *
     * @param rtx  A {@link ReadTransaction} instance.
     */
    public InventoryReader(ReadTransaction rtx) {
        readTx = rtx;
    }

    /**
     * Return read transaction.
     *
     * @return  A {@link ReadTransaction} instance.
     */
    public ReadTransaction getReadTransaction() {
        return readTx;
    }

    /**
     * Return the VTN node information specified by the given {@link SalNode}
     * instance.
     *
     * @param snode  A {@link SalNode} instance which specifies the VTN node
     *               information.
     * @return  A {@link VtnNode} instance if present.
     *          {@code null} if not present.
     * @throws VTNException
     *    Failed to read VTN port information.
     */
    public VtnNode get(SalNode snode) throws VTNException {
        if (snode == null) {
            return null;
        }

        NodeCacheEntry ent = nodeCache.get(snode);
        VtnNode vnode;
        if (ent != null) {
            vnode = ent.getVtnNode();
        } else if (!nodeCache.containsKey(snode)) {
            vnode = read(snode);
        } else {
            vnode = null;
        }

        return vnode;
    }

    /**
     * Return the VTN port information specified by the given {@link SalPort}
     * instance.
     *
     * @param sport  A {@link SalPort} instance which specifies the VTN port
     *               information.
     * @return  A {@link VtnPort} instance if present.
     *          {@code null} if not present.
     * @throws VTNException
     *    Failed to read VTN port information.
     */
    public VtnPort get(SalPort sport) throws VTNException {
        if (sport == null) {
            return null;
        }

        VtnPort vport = portCache.get(sport);
        if (vport == null && !portCache.containsKey(sport)) {
            vport = read(sport);
        }

        return vport;
    }

    /**
     * Put the given VTN node information into the cache.
     *
     * @param snode  A {@link SalNode} instance.
     * @param vnode  A {@link VtnNode} instance.
     */
    public void prefetch(SalNode snode, VtnNode vnode) {
        // Purge old port information corresponding to the given node.
        long num = snode.getNodeNumber();
        for (Iterator<SalPort> it = portCache.keySet().iterator();
             it.hasNext();) {
            SalPort sport = it.next();
            if (sport.getNodeNumber() == num) {
                it.remove();
            }
        }

        prefetchNode(snode, vnode);
    }

    /**
     * Put all VTN port information in the given collection into the VTN port
     * cache.
     *
     * @param vports  A collection of {@link VtnPort} instances.
     */
    public void prefetch(Collection<VtnPort> vports) {
        if (vports != null) {
            for (VtnPort vport: vports) {
                prefetch(vport);
            }
        }
    }

    /**
     * Put the given VTN port information into the VTN port cache.
     *
     * @param vport  A {@link VtnPort} instance.
     */
    public void prefetch(VtnPort vport) {
        if (vport != null) {
            SalPort sport = SalPort.create(vport.getId());
            prefetch(sport, vport);
        }
    }

    /**
     * Put the given VTN port information into the VTN port cache.
     *
     * @param sport  A {@link SalPort} instance
     * @param vport  A {@link VtnPort} instance associated with {@code sport}.
     */
    public void prefetch(SalPort sport, VtnPort vport) {
        portCache.put(sport, vport);
        if (vport != null) {
            SalNode snode = sport.getSalNode();
            NodeCacheEntry ent = nodeCache.get(snode);
            if (ent != null) {
                ent.putSalPort(sport, vport);
            }
        }
    }

    /**
     * Return all node information as a list.
     *
     * @return  A list of {@link VtnNode} instances.
     * @throws VTNException  An error occurred.
     */
    public List<VtnNode> getVtnNodes() throws VTNException {
        if (allCached) {
            List<VtnNode> list = new ArrayList<VtnNode>(nodeCache.size());
            for (NodeCacheEntry ent: nodeCache.values()) {
                list.add(ent.getVtnNode());
            }

            return list;
        }

        InstanceIdentifier<VtnNodes> path =
            InstanceIdentifier.create(VtnNodes.class);
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        Optional<VtnNodes> opt = DataStoreUtils.read(readTx, oper, path);

        nodeCache.clear();
        portCache.clear();
        allCached = true;

        if (opt.isPresent()) {
            VtnNodes nodes = opt.get();
            List<VtnNode> list = nodes.getVtnNode();
            if (list != null) {
                for (VtnNode vnode: list) {
                    SalNode snode = SalNode.create(vnode.getId());
                    prefetchNode(snode, vnode);
                }

                return list;
            }
        }

        return Collections.<VtnNode>emptyList();
    }

    /**
     * Determine whether the given node is present or not.
     *
     * @param snode  A {@link SalNode} instance.
     * @return  {@code true} if the given node is present.
     *          Otherwise {@code false}.
     * @throws VTNException  An error occurred.
     */
    public boolean exists(SalNode snode) throws VTNException {
        return (get(snode) != null);
    }

    /**
     * Determine whether the given port is enabled or not.
     *
     * @param sport  A {@link SalPort} instance.
     * @return  {@code true} if the given port is enabled.
     *          Otherwise {@code false}.
     * @throws VTNException  An error occurred.
     */
    public boolean isEnabled(SalPort sport) throws VTNException {
        VtnPort vport = get(sport);
        return (vport == null) ? false : InventoryUtils.isEnabled(vport);
    }

    /**
     * Find a {@link SalPort} instance that meets the specified condition.
     *
     * @param node  A {@link Node} instance corresponding to a physical
     *              switch.
     * @param port  A {@link SwitchPort} instance which specifies the
     *              condition to select switch port.
     * @return  A {@link SalPort} instance if found.
     *          {@code null} is returned if not found.
     * @throws VTNException  An error occurred.
     */
    public SalPort findPort(Node node, SwitchPort port) throws VTNException {
        SalNode snode = SalNode.create(node);
        if (get(snode) == null) {
            return null;
        }

        SalPort target = null;
        String type = port.getType();
        String id = port.getId();
        if (type != null && id != null) {
            // Try to construct a NodeConnector instance.
            // This returns null if invalid parameter is specified.
            NodeConnector nc = NodeConnector.fromStringNoNode(type, id, node);
            target = SalPort.create(nc);
            if (target == null) {
                return null;
            }
        }

        return findPort(snode, target, port.getName());
    }

    /**
     * Find a {@link SalPort} instance that meets the specified condition.
     *
     * @param snode A {@link SalNode} instance corresponding to a physical
     *              switch.
     * @param vswp  A {@link VtnSwitchPort} instance which specifies the
     *              condition to select switch port.
     * @return  A {@link SalPort} instance if found.
     *          {@code null} is returned if not found.
     * @throws VTNException  An error occurred.
     */
    public SalPort findPort(SalNode snode, VtnSwitchPort vswp)
        throws VTNException {
        if (get(snode) == null) {
            return null;
        }

        SalPort target = null;
        String id = vswp.getPortId();
        if (id != null) {
            // Try to construct a SalPort instance.
            // This returns null if invalid parameter is specified.
            target = SalPort.create(snode.getNodeNumber(), id);
            if (target == null) {
                return null;
            }
        }

        return findPort(snode, target, vswp.getPortName());
    }

    /**
     * Collect node connectors associated with edge switch ports in up state.
     *
     * @param portSet  A set of {@link NodeConnector} instances to store
     *                 results.
     * @throws VTNException  An error occurred.
     */
    public void collectUpEdgePorts(Set<NodeConnector> portSet)
        throws VTNException {
        collectUpEdgePorts(portSet, null);
    }

    /**
     * Collect node connectors associated with edge switch ports in up state.
     *
     * <p>
     *   Switch ports to be collected can be selected by specifying
     *   {@link PortFilter} instance to {@code filter}.
     *   If a {@link PortFilter} instance is specified, this method collects
     *   switch ports accepted by
     *   {@link PortFilter#accept(NodeConnector, VtnPort)}.
     * </p>
     *
     * @param portSet  A set of {@link NodeConnector} instances to store
     *                 results.
     * @param filter   A {@link PortFilter} instance which filters switch port.
     *                 All switch ports are stored to {@code portSet} if
     *                 {@code null} is specified.
     * @throws VTNException  An error occurred.
     */
    public void collectUpEdgePorts(Set<NodeConnector> portSet,
                                   PortFilter filter)
        throws VTNException {
        // Fill all the nodes into cache.
        getVtnNodes();

        for (NodeCacheEntry ent: nodeCache.values()) {
            if (ent != null) {
                ent.collectUpEdgePorts(portSet, filter);
            }
        }
    }

    /**
     * Return cached node information only for unit test.
     *
     * @return  A map that contains cached node information.
     */
    Map<SalNode, VtnNode> getCachedNodes() {
        Map<SalNode, VtnNode> map = new HashMap<>();
        for (Map.Entry<SalNode, NodeCacheEntry> entry: nodeCache.entrySet()) {
            SalNode snode = entry.getKey();
            NodeCacheEntry ent = entry.getValue();
            VtnNode vnode = (ent == null) ? null : ent.getVtnNode();
            map.put(snode, vnode);
        }

        return map;
    }

    /**
     * Return cached port information only for unit test.
     *
     * @return  A map that contains cached port information.
     */
    Map<SalPort, VtnPort> getCachedPorts() {
        return new HashMap<SalPort, VtnPort>(portCache);
    }

    /**
     * Put the given VTN node information into the cache.
     *
     * @param snode  A {@link SalNode} instance.
     * @param vnode  A {@link VtnNode} instance.
     */
    private void prefetchNode(SalNode snode, VtnNode vnode) {
        if (vnode == null) {
            nodeCache.put(snode, null);
        } else {
            NodeCacheEntry ent = new NodeCacheEntry(vnode);
            nodeCache.put(snode, ent);
            prefetch(vnode.getVtnPort());
        }
    }

    /**
     * Read the specified VTN node information from the VTN inventory
     * datastore.
     *
     * @param snode  A {@link SalNode} instance.
     * @return  A {@link VtnNode} instance if present.
     *          {@code null} if not present.
     * @throws VTNException
     *    Failed to read VTN node information.
     */
    private VtnNode read(SalNode snode) throws VTNException {
        InstanceIdentifier<VtnNode> path = snode.getVtnNodeIdentifier();
        VtnNode vnode = DataStoreUtils.read(
            readTx, LogicalDatastoreType.OPERATIONAL, path).orNull();
        prefetch(snode, vnode);

        return vnode;
    }

    /**
     * Read the specified VTN port information from the VTN inventory
     * datastore.
     *
     * @param sport  A {@link SalPort} instance.
     * @return  A {@link VtnPort} instance if present.
     *          {@code null} if not present.
     * @throws VTNException
     *    Failed to read VTN port information.
     */
    private VtnPort read(SalPort sport) throws VTNException {
        InstanceIdentifier<VtnPort> path = sport.getVtnPortIdentifier();
        VtnPort vport = DataStoreUtils.read(
            readTx, LogicalDatastoreType.OPERATIONAL, path).orNull();
        prefetch(sport, vport);

        return vport;
    }

    /**
     * Find a {@link SalPort} instance that has the given port name.
     *
     * @param snode   A {@link SalNode} instance corresponding to a physical
     *                switch.
     * @param target  A {@link SalPort} instance to be tested.
     *                If {@code null} is specified, this method searches for
     *                a switch port in {@code snode} by the port name specified
     *                by {@code name}.
     * @param name    The name of the switch port.
     * @return  A {@link SalPort} instance if found.
     *          {@code null} is returned if not found.
     */
    private SalPort findPort(SalNode snode, SalPort target, String name) {
        NodeCacheEntry ent = nodeCache.get(snode);
        if (name != null) {
            // Search for a switch port by its name.
            SalPort sport = ent.getSalPort(name);
            if (sport == null) {
                return null;
            }
            if (target != null && !target.equals(sport)) {
                sport = null;
            }
            return sport;
        }

        // Ensure that the detected switch port exists.
        if (target != null && !ent.contains(target)) {
            return null;
        }

        return target;
    }
}
