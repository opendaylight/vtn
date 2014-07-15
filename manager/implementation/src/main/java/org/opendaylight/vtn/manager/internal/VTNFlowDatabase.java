/*
 * Copyright (c) 2013-2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.ListIterator;
import java.util.Map;
import java.util.Set;
import java.util.concurrent.ConcurrentMap;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import org.opendaylight.vtn.manager.VTenantPath;
import org.opendaylight.vtn.manager.internal.cluster.FlowGroupId;
import org.opendaylight.vtn.manager.internal.cluster.MacVlan;
import org.opendaylight.vtn.manager.internal.cluster.VTNFlow;

import org.opendaylight.controller.forwardingrulesmanager.FlowEntry;
import org.opendaylight.controller.sal.core.Node;
import org.opendaylight.controller.sal.core.NodeConnector;
import org.opendaylight.controller.sal.flowprogrammer.Flow;
import org.opendaylight.controller.sal.match.Match;
import org.opendaylight.controller.sal.match.MatchType;

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
     * Fix broken flow entry originated by Open vSwitch.
     *
     * <p>
     *   Old version of Open vSwitch may clear OFPFW_DL_VLAN_PCP wildcard bit
     *   in flow entry incorrectly. This method removes VLAN_PCP condition
     *   from the specified flow entry.
     * </p>
     *
     * @param flow  A {@link Flow} instance converted from an OpenFlow flow
     *              entry.
     * @return  A {@link Flow} instance if the specified flow entry contains
     *          VLAN_PCP condition. {@code false} is returned if not.
     */
    public static Flow fixBrokenOvsFlow(Flow flow) {
        Match match = flow.getMatch();
        if (match.isPresent(MatchType.DL_VLAN_PR)) {
            // VTN Manager never set DL_VLAN_PR match field into flow entry.
            // So we can simply remove it.
            match.clearField(MatchType.DL_VLAN_PR);

            // We don't need to copy action list.
            // A Flow instance returned here is configured into a FlowEntry
            // instance, and it is used as a search key for vtnFlows lookup.
            // But only flow match and priority affect identity of FlowEntry
            // instance.
            Flow f = new Flow(match, null);
            f.setPriority(flow.getPriority());
            return f;
        }

        return null;
    }

    /**
     * Flow entry collector.
     * <p>
     *   This class is used to collect flow entries in VTN flows with
     *   separating ingress flow from others.
     * </p>
     */
    private final class FlowCollector {
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
            if (match(mgr, ingress)) {
                ingressFlows.add(ingress);
            }
            while (it.hasNext()) {
                FlowEntry fent = it.next();
                if (match(mgr, fent)) {
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
         * @param mgr   VTN Manager service.
         * @param fent  A flow entry.
         * @return  {@code true} is returned if the given flow entry should
         *          be collected.
         */
        private boolean match(VTNManagerImpl mgr, FlowEntry fent) {
            // Don't collect flow entry in the removed node.
            Node node = fent.getNode();
            boolean ret = mgr.exists(node);
            if (!ret && LOG.isTraceEnabled()) {
                LOG.trace("{}: Ignore flow entry in the removed node: {}",
                          mgr.getContainerName(), fent);
            }

            return ret;
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
     * @param rmIn   If {@code true} is specified, this method tries to remove
     *               ingress flow from the forwarding rule manager.
     * @return  {@code true} is returned if the specified flow was actually
     *          removed from this instance.
     *          {@code false} is returned if the specified flow is not
     *          contained in this instance.
     */
    public synchronized boolean flowRemoved(VTNManagerImpl mgr,
                                            FlowEntry entry, boolean rmIn) {
        VTNFlow vflow = vtnFlows.remove(entry);
        if (vflow == null) {
            return false;
        }

        // Remove this VTN flow from the database and the cluster cache.
        FlowGroupId gid = vflow.getGroupId();
        groupFlows.remove(gid);
        removeNodeIndex(vflow);
        removePortIndex(vflow);

        ListIterator<FlowEntry> it = vflow.getFlowEntries().listIterator();
        if (!it.hasNext()) {
            // This should never happen.
            LOG.warn("{}: An empty flow expired: group={}",
                     mgr.getContainerName(), gid);
            return true;
        }

        FlowEntry ingress = it.next();
        if (LOG.isDebugEnabled()) {
            LOG.debug("{}:{}: VTN flow expired: group={}, ingress={}",
                      mgr.getContainerName(), tenantName, gid, ingress);
        }

        boolean hasNext;
        if (rmIn) {
            // Move iterator cursor backwards in order to remove ingress flow.
            it.previous();
            hasNext = true;
        } else {
            hasNext = it.hasNext();
        }

        if (hasNext) {
            // Uninstall flow entries.
            FlowRemoveTask task = new FlowRemoveTask(mgr, gid, null, it);
            mgr.postFlowTask(task);
        }

        return true;
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
     * Remove all VTN flows related to the given node.
     *
     * @param mgr        VTN Manager service.
     * @param node       A node associated with a switch.
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

        // Eliminate flow entries in the specified node.
        FlowCollector collector = new FlowCollector();
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
     * Remove all VTN flows related to the given edge network.
     *
     * <p>
     *   The edge network is specified by a pair of node connector and VLAN ID.
     *   This method searches for VTN flows whose incoming or outgoing network
     *   matches the specified pair of node connector and VLAN ID.
     * </p>
     *
     * @param mgr   VTN Manager service.
     * @param port  A node connector associated with a switch port.
     * @param vlan  A VLAN ID.
     * @return  A {@link FlowRemoveTask} object that will execute the actual
     *          work is returned. {@code null} is returned if there is no flow
     *          entry to be removed.
     */
    public synchronized FlowRemoveTask removeFlows(VTNManagerImpl mgr,
                                                   NodeConnector port,
                                                   short vlan) {
        Set<VTNFlow> vflows = portFlows.get(port);
        if (vflows == null) {
            return null;
        }

        FlowCollector collector = new FlowCollector();
        for (Iterator<VTNFlow> it = vflows.iterator(); it.hasNext();) {
            VTNFlow vflow = it.next();
            String type;
            if (vflow.isIncomingNetwork(port, vlan)) {
                type = "incoming";
            } else if (vflow.isOutgoingNetwork(port, vlan)) {
                type = "outgoing";
            } else {
                continue;
            }

            FlowGroupId gid = vflow.getGroupId();
            if (LOG.isDebugEnabled()) {
                LOG.debug("{}:{}: Remove VTN flow which contains {} network:" +
                          " port={}, vlan={}, group={}",
                          mgr.getContainerName(), tenantName, type, port, vlan,
                          gid);
            }

            // Remove this VTN flow from the database.
            groupFlows.remove(gid);
            removeFlowIndex(vflow);
            removeNodeIndex(vflow);
            it.remove();

            // Collect flow entries to be uninstalled.
            collector.collect(mgr, vflow);
        }

        if (vflows.isEmpty()) {
            portFlows.remove(port);
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
    public synchronized FlowRemoveTask removeFlows(VTNManagerImpl mgr,
                                                   VTenantPath path) {
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
     * Remove all VTN flows related to the given layer 2 host entry.
     *
     * <p>
     *   A layer 2 host entry is specified by a pair of {@link MacVlan} and
     *   {@link NodeConnector} object.
     * </p>
     *
     * @param mgr    VTN Manager service.
     * @param mvlan  A pair of MAC address and VLAN ID.
     * @param port   A node connector associated with a switch port.
     * @return  A {@link FlowRemoveTask} object that will execute the actual
     *          work is returned. {@code null} is returned if there is no flow
     *          entry to be removed.
     */
    public synchronized FlowRemoveTask removeFlows(VTNManagerImpl mgr,
                                                   MacVlan mvlan,
                                                   NodeConnector port) {
        Set<VTNFlow> vflows = portFlows.get(port);
        if (vflows == null) {
            return null;
        }

        FlowCollector collector = new FlowCollector();
        for (Iterator<VTNFlow> it = vflows.iterator(); it.hasNext();) {
            VTNFlow vflow = it.next();
            if (vflow.dependsOn(mvlan)) {
                FlowGroupId gid = vflow.getGroupId();
                if (LOG.isDebugEnabled()) {
                    LOG.debug("{}:{}: Remove VTN flow related to L2 host: " +
                              "host={}, port={}, group={}",
                              mgr.getContainerName(), tenantName,
                              mvlan, port, gid);
                }

                // Remove this VTN flow from the database.
                groupFlows.remove(gid);
                removeFlowIndex(vflow);
                removeNodeIndex(vflow);
                it.remove();

                // Collect flow entries to be uninstalled.
                collector.collect(mgr, vflow);
            }
        }

        if (vflows.isEmpty()) {
            portFlows.remove(port);
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
            if (removeIndex(mgr, vflow)) {
                // Collect flow entries to be uninstalled.
                collector.collect(mgr, vflow);
            }
        }

        // Uninstall flow entries in background.
        return collector.uninstall(mgr);
    }

    /**
     * Remove all VTN flows accepted by the specified {@link VTNFlowMatch}
     * instance.
     *
     * <p>
     *   Flow uninstallation will be executed in background.
     *   The caller can wait for completion of flow uninstallation by using
     *   returned {@link FlowRemoveTask} object.
     * </p>
     *
     * @param mgr     VTN Manager service.
     * @param fmatch  A {@link VTNFlowMatch} instance which determines VTN
     *                flows to be removed.
     *                Specifying {@code null} results in undefined behavior.
     * @return  A {@link FlowRemoveTask} object that will execute the actual
     *          work is returned. {@code null} is returned if there is no flow
     *          entry to be removed.
     */
    public synchronized FlowRemoveTask removeFlows(VTNManagerImpl mgr,
                                                   VTNFlowMatch fmatch) {
        FlowCollector collector = new FlowCollector();
        for (Iterator<VTNFlow> it = vtnFlows.values().iterator();
             it.hasNext();) {
            VTNFlow vflow = it.next();
            if (fmatch.accept(vflow)) {
                FlowGroupId gid = vflow.getGroupId();
                if (LOG.isDebugEnabled()) {
                    LOG.debug("{}:{}: Remove VTN flow accepted by filter" +
                              "({}): group={}", mgr.getContainerName(),
                              tenantName, fmatch.getDescription(), gid);
                }

                // Remove this VTN flow from the database.
                groupFlows.remove(gid.getEventId());
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
            if (LOG.isDebugEnabled()) {
                LOG.debug("{}:{}: Ingress flow is already installed: " +
                          "ingress={}", mgr.getContainerName(), tenantName,
                          ingress);
            }
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
     * @return  {@code true} is returned if the specified VTN flow was actually
     *          removed. Otherwise {@code false} is returned.
     */
    public synchronized boolean removeIndex(VTNManagerImpl mgr, VTNFlow vflow) {
        FlowGroupId gid = vflow.getGroupId();
        if (groupFlows.remove(gid) != null) {
            removeFlowIndex(vflow);
            removeNodeIndex(vflow);
            removePortIndex(vflow);
            return true;
        }

        return false;
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
