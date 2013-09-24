/*
 * Copyright (c) 2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal;

import java.util.Iterator;
import java.util.List;
import java.util.ArrayList;
import java.util.Map;
import java.util.HashMap;
import java.util.Set;
import java.util.HashSet;
import java.util.concurrent.ConcurrentMap;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import org.opendaylight.vtn.manager.VTenantPath;
import org.opendaylight.vtn.manager.internal.cluster.FlowGroupId;
import org.opendaylight.vtn.manager.internal.cluster.MacVlan;
import org.opendaylight.vtn.manager.internal.cluster.VTNFlow;

import org.opendaylight.controller.connectionmanager.ConnectionLocality;
import org.opendaylight.controller.connectionmanager.IConnectionManager;
import org.opendaylight.controller.forwardingrulesmanager.FlowEntry;
import org.opendaylight.controller.sal.core.Node;
import org.opendaylight.controller.sal.core.NodeConnector;

/**
 * Flow database which keeps all VTN flows in the container.
 */
public class VTNFlowDatabase {
    /**
     * Logger instance.
     */
    private static final Logger  LOG =
        LoggerFactory.getLogger(VTNFlowDatabase.class);

    /**
     * The name of the virtual tenant.
     */
    private final String  tenantName;

    /**
     * Flow entries in the VTN indexed by the ingress SAL flow.
     */
    private final Map<FlowEntry, VTNFlow>  vtnFlows =
        new HashMap<FlowEntry, VTNFlow>();

    /**
     * Flow entries in the VTN indexed by the name of the flow entry group.
     */
    private final Map<FlowGroupId, VTNFlow>  groupFlows =
        new HashMap<FlowGroupId, VTNFlow>();

    /**
     * Flow entries in the VTN indexed by related node.
     */
    private final Map<Node, Set<VTNFlow>>  nodeFlows =
        new HashMap<Node, Set<VTNFlow>>();

    /**
     * Flow entries in the VTN indexed by related switch port.
     */
    private final Map<NodeConnector, Set<VTNFlow>>  portFlows =
        new HashMap<NodeConnector, Set<VTNFlow>>();

    /**
     * Flow entry collector.
     * <p>
     *   This class is used to collect flow entries in VTN flows with
     *   separating ingress flow from others.
     * </p>
     */
    protected class FlowCollector {
        /**
         * List of ingress flow entries.
         */
        private final List<FlowEntry>  ingressFlows =
            new ArrayList<FlowEntry>();

        /**
         * List of flow entries except for ingress flows.
         */
        private final List<FlowEntry>  flowEntries =
            new ArrayList<FlowEntry>();

        /**
         * Set of VTN flow group IDs.
         */
        private final Set<FlowGroupId>  groupSet = new HashSet<FlowGroupId>();

        /**
         * Collect flow entries in the specified VTN flow.
         *
         * @param mgr    VTN Manager service.
         * @param vflow  A VTN flow.
         */
        private void collect(VTNManagerImpl mgr, VTNFlow vflow) {
            groupSet.add(vflow.getGroupId());

            Iterator<FlowEntry> it = vflow.getFlowEntries().iterator();
            if (!it.hasNext()) {
                // This should never happen.
                LOG.warn("{}: An empty flow found: group={}",
                         mgr.getContainerName(), vflow.getGroupId());
                return;
            }

            FlowEntry ingress = it.next();
            if (match(ingress)) {
                ingressFlows.add(ingress);
            }
            while (it.hasNext()) {
                FlowEntry fent = it.next();
                if (match(fent)) {
                    flowEntries.add(fent);
                }
            }
        }

        /**
         * Uninstall flow entries collected by this object.
         *
         * @param mgr  VTN Manager service.
         * @return  A {@link FlowRemoveTask} object that will execute the
         *          actual work is returned. {@code null} is returned there
         *          is no flow entry to be removed.
         */
        private FlowRemoveTask uninstall(VTNManagerImpl mgr) {
            FlowRemoveTask task;
            if (!groupSet.isEmpty() || !ingressFlows.isEmpty() ||
                !flowEntries.isEmpty()) {
                // Uninstall flow entries in background.
                task = new FlowRemoveTask(mgr, groupSet, ingressFlows,
                                          flowEntries);
                mgr.postFlowTask(task);
            } else {
                task = null;
            }

            return task;
        }

        /**
         * Determine whether the given flow entry should be collected or not.
         *
         * @param fent  A flow entry.
         * @return  {@code true} is returned if the given flow entry should
         *          be collected.
         */
        public boolean match(FlowEntry fent) {
            // Collect all flow entries.
            return true;
        }
    }

    /**
     * Flow collector that excludes flow entries in the specified node.
     */
    private final class ExcludeNodeCollector extends FlowCollector {
        /**
         * A node to be excluded.
         */
        private final Node  excludeNode;

        /**
         * Construct a new collector.
         *
         * @param node  A node to be excluded.
         */
        private ExcludeNodeCollector(Node node) {
            excludeNode = node;
        }

        /**
         * Determine whether the given flow entry should be handled or not.
         *
         * @param fent  A flow entry.
         * @return  {@code true} is returned if the given flow entry should
         *          be handled.
         */
        public boolean match(FlowEntry fent) {
            return !excludeNode.equals(fent.getNode());
        }
    }

    /**
     * Construct a new VTN flow database.
     *
     * @param tname  The name of the virtual tenant.
     */
    VTNFlowDatabase(String tname) {
        tenantName = tname;
    }

    /**
     * Create a new VTN flow object.
     *
     * @param mgr  VTN Manager service.
     * @return  A new VTN flow object.
     */
    public VTNFlow create(VTNManagerImpl mgr) {
        // Determine the name of a new flow entry group.
        ConcurrentMap<FlowGroupId, VTNFlow> db = mgr.getFlowDB();
        FlowGroupId gid;
        do {
            gid = new FlowGroupId(tenantName);
        } while (db.containsKey(gid));

        return new VTNFlow(gid);
    }

    /**
     * Add a VTN flow to this database, and install them to related switches.
     *
     * @param mgr    VTN Manager service.
     * @param vflow  A VTN flow to be added.
     */
    public synchronized void install(VTNManagerImpl mgr, VTNFlow vflow) {
        if (!mgr.isAvailable()) {
            LOG.debug("{}:{}: No more VTN flow is allowed: {}",
                      mgr.getContainerName(), tenantName, vflow.getGroupId());
            return;
        }

        // Create indices for the given VTN flow.
        if (createIndex(mgr, vflow)) {
            // Rest of work will be done by the VTN flow task thread.
            FlowAddTask task = new FlowAddTask(mgr, vflow);
            mgr.postFlowTask(task);
        }
    }

    /**
     * Invoked when a SAL flow has expired.
     *
     * <p>
     *   Note that the given SAL flow must be associated with an ingress flow
     *   of a VTN flow.
     * </p>
     *
     * @param mgr    VTN Manager service.
     * @param entry  A flow entry object which contains expired SAL flow.
     */
    public synchronized void flowRemoved(VTNManagerImpl mgr, FlowEntry entry) {
        VTNFlow vflow = vtnFlows.remove(entry);
        if (vflow == null) {
            return;
        }

        // Remove this VTN flow from the database and the cluster cache.
        FlowGroupId gid = vflow.getGroupId();
        groupFlows.remove(gid);
        removeNodeIndex(vflow);
        removePortIndex(vflow);

        Iterator<FlowEntry> it = vflow.getFlowEntries().iterator();
        if (!it.hasNext()) {
            // This should never happen.
            LOG.warn("{}: An empty flow expired: group={}",
                     mgr.getContainerName(), gid);
            return;
        }

        FlowEntry ingress = it.next();
        if (LOG.isDebugEnabled()) {
            LOG.debug("{}:{}: VTN flow expired: group={}, ingress={}",
                      mgr.getContainerName(), tenantName, gid, ingress);
        }

        if (it.hasNext()) {
            // Uninstall flow entries except for ingress flow.
            FlowRemoveTask task = new FlowRemoveTask(mgr, gid, null, it);
            mgr.postFlowTask(task);
        }
    }

    /**
     * Invoked when a VTN flow has been removed from the cluster cache by
     * a remote cluster node.
     *
     * @param mgr  VTN Manager service.
     * @param gid  Identifier of the flow entry group.
     */
    public synchronized void flowRemoved(VTNManagerImpl mgr, FlowGroupId gid) {
        VTNFlow vflow = groupFlows.remove(gid);
        if (vflow != null) {
            // Clean up indices.
            removeFlowIndex(vflow);
            removeNodeIndex(vflow);
            removePortIndex(vflow);
        }
    }

    /**
     * Invoked when a new node has been detected.
     *
     * <p>
     *   This method removes all VTN flows related to the specified node
     *   because a newly detected node should not have any flow entry.
     * </p>
     *
     * @param mgr   VTN Manager service.
     * @param node  A new node.
     */
    public synchronized void nodeAdded(VTNManagerImpl mgr, Node node) {
        IConnectionManager cnm = mgr.getConnectionManager();
        ConnectionLocality cl = cnm.getLocalityStatus(node);
        if (cl != ConnectionLocality.NOT_LOCAL) {
            removeFlows(mgr, node);
        }
    }

    /**
     * Remove all VTN flows related to the given node.
     *
     * <p>
     *   This method assumes that flow entries in the given node are no longer
     *   available. So this method never try to uninstall flow entries from
     *   the given node.
     * </p>
     *
     * @param mgr   VTN Manager service.
     * @param node  A node associated with a switch.
     * @return  A {@link FlowRemoveTask} object that will execute the actual
     *          work is returned. {@code null} is returned if there is no flow
     *          entry to be removed.
     */
    public synchronized FlowRemoveTask removeFlows(VTNManagerImpl mgr,
                                                   Node node) {
        Set<VTNFlow> vflows = nodeFlows.remove(node);
        if (vflows == null) {
            return null;
        }

        if (LOG.isDebugEnabled()) {
            LOG.debug("{}:{}: Remove VTN flows related to obsolete node {}",
                      mgr.getContainerName(), tenantName, node);
        }

        // Eliminate flow entries in the specified node.
        FlowCollector collector = new ExcludeNodeCollector(node);
        for (VTNFlow vflow: vflows) {
            if (LOG.isDebugEnabled()) {
                LOG.debug("{}:{}: Remove VTN flow related to node {}: " +
                          "group={}", mgr.getContainerName(), tenantName,
                          node, vflow.getGroupId());
            }

            // Remove this VTN flow from the database.
            removeIndex(mgr, vflow);

            // Collect flow entries to be uninstalled.
            collector.collect(mgr, vflow);
        }

        // Uninstall flow entries in background.
        return collector.uninstall(mgr);
    }

    /**
     * Remove all VTN flows related to the given switch port.
     *
     * @param mgr   VTN Manager service.
     * @param port  A node connector associated with a switch port.
     * @return  A {@link FlowRemoveTask} object that will execute the actual
     *          work is returned. {@code null} is returned if there is no flow
     *          entry to be removed.
     */
    public synchronized FlowRemoveTask removeFlows(VTNManagerImpl mgr,
                                                   NodeConnector port) {
        Set<VTNFlow> vflows = portFlows.remove(port);
        if (vflows == null) {
            return null;
        }

        FlowCollector collector = new FlowCollector();
        for (VTNFlow vflow: vflows) {
            if (LOG.isDebugEnabled()) {
                LOG.debug("{}:{}: Remove VTN flow related to port {}: " +
                          "group={}", mgr.getContainerName(), tenantName,
                          port, vflow.getGroupId());
            }

            // Remove this VTN flow from the database.
            removeIndex(mgr, vflow);

            // Collect flow entries to be uninstalled.
            collector.collect(mgr, vflow);
        }

        // Uninstall flow entries in background.
        return collector.uninstall(mgr);
    }

    /**
     * Remove all VTN flows related to the given virtual node.
     *
     * <p>
     *   Flow uninstallation will be executed in background.
     *   The caller can wait for completion of flow uninstallation by using
     *   returned {@link FlowRemoveTask} object.
     * </p>
     *
     * @param mgr   VTN Manager service.
     * @param path  A path to the virtual node.
     * @return  A {@link FlowRemoveTask} object that will execute the actual
     *          work is returned. {@code null} is returned if there is no flow
     *          entry to be removed.
     */
    public FlowRemoveTask removeFlows(VTNManagerImpl mgr, VTenantPath path) {
        FlowCollector collector = new FlowCollector();
        for (Iterator<VTNFlow> it = vtnFlows.values().iterator();
             it.hasNext();) {
            VTNFlow vflow = it.next();
            if (vflow.dependsOn(path)) {
                FlowGroupId gid = vflow.getGroupId();
                if (LOG.isDebugEnabled()) {
                    LOG.debug("{}:{}: Remove VTN flow related to virtual " +
                              "node {}: group={}", mgr.getContainerName(),
                              tenantName, path, gid);
                }

                // Remove this VTN flow from the database.
                groupFlows.remove(gid);
                removeNodeIndex(vflow);
                removePortIndex(vflow);
                it.remove();

                // Collect flow entries to be uninstalled.
                collector.collect(mgr, vflow);
            }
        }

        // Uninstall flow entries in background.
        return collector.uninstall(mgr);
    }

    /**
     * Remove all VTN flows related to the given pair of MAC address and
     * VLAN ID.
     *
     * @param mgr    VTN Manager service.
     * @param mvlan  A pair of MAC address and VLAN ID.
     * @return  A {@link FlowRemoveTask} object that will execute the actual
     *          work is returned. {@code null} is returned if there is no flow
     *          entry to be removed.
     */
    public synchronized FlowRemoveTask removeFlows(VTNManagerImpl mgr,
                                                   MacVlan mvlan) {
        FlowCollector collector = new FlowCollector();
        for (Iterator<VTNFlow> it = vtnFlows.values().iterator();
             it.hasNext();) {
            VTNFlow vflow = it.next();
            if (vflow.dependsOn(mvlan)) {
                FlowGroupId gid = vflow.getGroupId();
                if (LOG.isDebugEnabled()) {
                    LOG.debug("{}:{}: Remove VTN flow related to host {}: " +
                              "group={}", mgr.getContainerName(),
                              tenantName, mvlan, gid);
                }

                // Remove this VTN flow from indices.
                groupFlows.remove(gid);
                removeNodeIndex(vflow);
                removePortIndex(vflow);
                it.remove();

                // Collect flow entries to be uninstalled.
                collector.collect(mgr, vflow);
            }
        }

        // Uninstall flow entries in background.
        return collector.uninstall(mgr);
    }

    /**
     * Remove all VTN flows in the given list.
     *
     * @param mgr     VTN Manager service.
     * @param vflows  A list of VTN flows to be removed.
     * @return  A {@link FlowRemoveTask} object that will execute the actual
     *          work is returned. {@code null} is returned if there is no flow
     *          entry to be removed.
     */
    public synchronized FlowRemoveTask removeFlows(VTNManagerImpl mgr,
                                                   List<VTNFlow> vflows) {
        FlowCollector collector = new FlowCollector();
        for (VTNFlow vflow: vflows) {
            // Remove this VTN flow from the database.
            removeIndex(mgr, vflow);

            // Collect flow entries to be uninstalled.
            collector.collect(mgr, vflow);
        }

        // Uninstall flow entries in background.
        return collector.uninstall(mgr);
    }

    /**
     * Create indices for the specified VTN flow.
     *
     * @param mgr    VTN Manager service.
     * @param vflow  A VTN flow.
     * @return  {@code true} is returned if indices are successfully created.
     *          {@code false} is returned if the specified flow is already
     *          installed.
     */
    public synchronized boolean createIndex(VTNManagerImpl mgr, VTNFlow vflow) {
        // Create index by the group name.
        FlowGroupId gid = vflow.getGroupId();
        VTNFlow old = groupFlows.put(gid, vflow);
        if (old != null) {
            if (LOG.isDebugEnabled()) {
                LOG.debug("{}:{}: VTN flow is already indexed: group={}",
                          mgr.getContainerName(), tenantName, gid);
            }
            groupFlows.put(gid, old);
            return false;
        }

        // Create index by the ingress SAL flow.
        FlowEntry ingress = vflow.getFlowEntries().get(0);
        old = vtnFlows.put(ingress, vflow);
        if (old != null) {
            LOG.info("{}:{}: Ingress flow is already installed: ingress={}",
                     mgr.getContainerName(), tenantName, ingress);
            vtnFlows.put(ingress, old);
            groupFlows.remove(gid);
            return false;
        }

        // Create index by related switches and switch ports.
        for (Node node: vflow.getFlowNodes()) {
            Set<VTNFlow> vflows = nodeFlows.get(node);
            if (vflows == null) {
                vflows = new HashSet<VTNFlow>();
                nodeFlows.put(node, vflows);
            }
            vflows.add(vflow);
        }
        for (NodeConnector port: vflow.getFlowPorts()) {
            Set<VTNFlow> vflows = portFlows.get(port);
            if (vflows == null) {
                vflows = new HashSet<VTNFlow>();
                portFlows.put(port, vflows);
            }
            vflows.add(vflow);
        }

        return true;
    }

    /**
     * Uninstall all flow entries in the virtual tenant.
     *
     * <p>
     *   This methods uninstalls all ingress flows at first, and then
     *   uninstalls other flow entries.
     * </p>
     *
     * @param mgr   VTN Manager service.
     * @return  A {@link FlowRemoveTask} object that will execute the actual
     *          work is returned. {@code null} is returned if there is no flow
     *          entry to be removed.
     */
    public synchronized FlowRemoveTask clear(VTNManagerImpl mgr) {
        FlowCollector collector = new FlowCollector();
        for (Iterator<VTNFlow> it = vtnFlows.values().iterator();
             it.hasNext();) {
            // Remove this VTN flow from the database.
            VTNFlow vflow = it.next();
            it.remove();

            // Collect flow entries to be uninstalled.
            collector.collect(mgr, vflow);
        }

        // Clean up indices.
        groupFlows.clear();
        nodeFlows.clear();
        portFlows.clear();

        // Uninstall flow entries in background.
        return collector.uninstall(mgr);
    }

    /**
     * Uninstall all flows related to the local node.
     *
     * @param mgr  VTN Manager service.
     */
    public synchronized void shutdown(VTNManagerImpl mgr) {
        FlowCollector collector = new FlowCollector();
        for (Iterator<VTNFlow> it = vtnFlows.values().iterator();
             it.hasNext();) {
            VTNFlow vflow = it.next();
            if (vflow.getLocality(mgr) != ConnectionLocality.NOT_LOCAL) {
                // Remove this VTN flow from indices.
                FlowGroupId gid = vflow.getGroupId();
                groupFlows.remove(gid);
                removeNodeIndex(vflow);
                removePortIndex(vflow);
                it.remove();

                // Collect flow entries to be uninstalled.
                collector.collect(mgr, vflow);
            }
        }

        // Uninstall flow entries in background.
        collector.uninstall(mgr);
    }

    /**
     * Determine whether the given SAL flow is contained in the ingress flow
     * map or not.
     *
     * @param entry  A flow entry object.
     * @return  {@code true} is returned only if the given flow entry is
     *          contained in the ingress flow map.
     */
    public synchronized boolean containsIngressFlow(FlowEntry entry) {
        return vtnFlows.containsKey(entry);
    }

    /**
     * Remove the given VTN flow from database indices.
     *
     * @param mgr    VTN Manager service.
     * @param vflow  A VTN flow.
     */
    public synchronized void removeIndex(VTNManagerImpl mgr, VTNFlow vflow) {
        FlowGroupId gid = vflow.getGroupId();
        if (groupFlows.remove(gid) != null) {
            removeFlowIndex(vflow);
            removeNodeIndex(vflow);
            removePortIndex(vflow);
        }
    }

    /**
     * Remove the given VTN flow from ingress flow index.
     *
     * @param vflow  A VTN flow.
     */
    private synchronized void removeFlowIndex(VTNFlow vflow) {
        FlowEntry ingress = vflow.getFlowEntries().get(0);
        vtnFlows.remove(ingress);
    }

    /**
     * Remove the given VTN flow from node index.
     *
     * @param vflow  A VTN flow.
     */
    private synchronized void removeNodeIndex(VTNFlow vflow) {
        for (Node node: vflow.getFlowNodes()) {
            Set<VTNFlow> vflows = nodeFlows.get(node);
            if (vflows != null) {
                vflows.remove(vflow);
                if (vflows.isEmpty()) {
                    nodeFlows.remove(node);
                    return;
                }
            }
        }
    }

    /**
     * Remove the given VTN flow from port index.
     *
     * @param vflow  A VTN flow.
     */
    private synchronized void removePortIndex(VTNFlow vflow) {
        for (NodeConnector port: vflow.getFlowPorts()) {
            Set<VTNFlow> vflows = portFlows.get(port);
            if (vflows != null) {
                vflows.remove(vflow);
                if (vflows.isEmpty()) {
                    portFlows.remove(port);
                    return;
                }
            }
        }
    }
}
