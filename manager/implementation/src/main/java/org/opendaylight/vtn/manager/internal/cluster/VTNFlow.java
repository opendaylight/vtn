/*
 * Copyright (c) 2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.cluster;

import java.io.Serializable;
import java.io.IOException;
import java.io.ObjectInputStream;
import java.util.List;
import java.util.ArrayList;
import java.util.Set;
import java.util.HashSet;

import org.opendaylight.vtn.manager.VTenantPath;
import org.opendaylight.vtn.manager.internal.ActionList;
import org.opendaylight.vtn.manager.internal.VTNManagerImpl;

import org.opendaylight.controller.connectionmanager.IConnectionManager;
import org.opendaylight.controller.forwardingrulesmanager.FlowEntry;
import org.opendaylight.controller.sal.action.Action;
import org.opendaylight.controller.sal.action.Output;
import org.opendaylight.controller.sal.action.PopVlan;
import org.opendaylight.controller.sal.action.SetVlanId;
import org.opendaylight.controller.sal.connection.ConnectionLocality;
import org.opendaylight.controller.sal.core.Node;
import org.opendaylight.controller.sal.core.NodeConnector;
import org.opendaylight.controller.sal.flowprogrammer.Flow;
import org.opendaylight.controller.sal.match.Match;
import org.opendaylight.controller.sal.match.MatchField;
import org.opendaylight.controller.sal.match.MatchType;
import org.opendaylight.controller.sal.utils.GlobalConstants;
import org.opendaylight.controller.sal.utils.ServiceHelper;
import org.opendaylight.controller.switchmanager.ISwitchManager;

/**
 * {@code VTNFlow} describes a flow in the virtual tenant.
 *
 * <p>
 *   Although this class is public to other packages, this class does not
 *   provide any API. Applications other than VTN Manager must not use this
 *   class.
 * </p>
 * <p>
 *   Note that this class is not synchronized.
 * </p>
 */
public class VTNFlow implements Serializable {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = 4456038162062268062L;

    /**
     * The identifier of the flow group.
     */
    private final FlowGroupId  groupId;

    /**
     * Flow entries.
     */
    private final List<FlowEntry>  flowEntries = new ArrayList<FlowEntry>();

    /**
     * Set of nodes related to this flow.
     */
    private transient Set<Node>  flowNodes = new HashSet<Node>();

    /**
     * Set of switch ports related to this flow.
     */
    private transient Set<NodeConnector>  flowPorts =
        new HashSet<NodeConnector>();

    /**
     * Set of virtual node paths on which this flow depends.
     */
    private final Set<VTenantPath>  dependNodes = new HashSet<VTenantPath>();

    /**
     * Set of host entry, which is a pair of MAC address and VLAN ID,
     * on which this flow depends.
     */
    private final Set<MacVlan>  dependHosts = new HashSet<MacVlan>();

    /**
     * Construct a new VTN flow object.
     *
     * @param gid  Flow group ID.
     */
    public VTNFlow(FlowGroupId gid) {
        groupId = gid;
    }

    /**
     * Return the identifier of the flow group.
     *
     * @return  The identifier of the flow group.
     */
    public FlowGroupId getGroupId() {
        return groupId;
    }

    /**
     * Return a list of flow entries.
     *
     * <p>
     *   Note that modifying returned list affects this object.
     * </p>
     *
     * @return  A list of flow entries.
     */
    public List<FlowEntry> getFlowEntries() {
        return flowEntries;
    }

    /**
     * Return a set of nodes related to this flow.
     *
     * <p>
     *   Note that modifying returned set affects this object.
     * </p>
     *
     * @return  A set of nodes.
     */
    public Set<Node> getFlowNodes() {
        return flowNodes;
    }

    /**
     * Return a set of switch ports related to this flow.
     *
     * <p>
     *   Note that modifying returned set affects this object.
     * </p>
     *
     * @return  A set of ports.
     */
    public Set<NodeConnector> getFlowPorts() {
        return flowPorts;
    }

    /**
     * Set timeout for an ingress flow.
     *
     * <p>
     *   This method does nothing if no flow entry is added.
     * </p>
     *
     * @param idle  An idle timeout for an ingress flow.
     * @param hard  A hard timeout for an ingress flow.
     */
    public void setTimeout(int idle, int hard) {
        if (flowEntries.size() != 0) {
            Flow ingress = flowEntries.get(0).getFlow();
            ingress.setIdleTimeout((short)idle);
            ingress.setHardTimeout((short)hard);
        }
    }

    /**
     * Add a flow entry that transmits a packet to the specified switch port
     * with setting VLAN ID.
     *
     * @param mgr       VTN Manager service.
     * @param match     Match object for a new flow entry.
     * @param actions   A list of actions.
     * @param priority  Priority value for a new flow entry.
     */
    public void addFlow(VTNManagerImpl mgr, Match match, ActionList actions,
                        int priority) {
        Flow flow = new Flow(match, actions.get());
        flow.setPriority((short)priority);
        Node node = actions.getNode();
        addFlow(mgr, flow, node);
    }

    /**
     * Add a flow entry.
     *
     * @param mgr   VTN Manager service.
     * @param flow  A SAL flow to be added.
     * @param node  A node associated with the given flow.
     */
    public void addFlow(VTNManagerImpl mgr, Flow flow, Node node) {
        int index = flowEntries.size();
        String gname = groupId.toString();
        StringBuilder builder = new StringBuilder(gname);
        builder.append(ClusterEventId.SEPARATOR).append(index);
        String name = builder.toString();
        FlowEntry entry = new FlowEntry(gname, name, flow, node);
        flowEntries.add(entry);
        updateIndex(mgr.getSwitchManager(), flow, node);
    }

    /**
     * Add all virtual node paths in the given set to the dependency set.
     *
     * @param pathSet  A set of path to the virtual nodes on which this flow
     *                 depends.
     */
    public void addDependency(Set<VTenantPath> pathSet) {
        dependNodes.addAll(pathSet);
    }

    /**
     * Add a host entry, represented by a pair of MAC address and VLAN ID,
     * to the dependency set.
     *
     * @param mvlan  A pair of MAC address and VLAN ID.
     */
    public void addDependency(MacVlan mvlan) {
        dependHosts.add(mvlan);
    }

    /**
     * Determine whether this flow depends on the specified virtual node
     * or not.
     *
     * @param path  Path to the virtual node.
     * @return  {@code true} is returned only if this flow depends on the
     *          virtual node specified by {@code path}.
     */
    public boolean dependsOn(VTenantPath path) {
        return dependNodes.contains(path);
    }

    /**
     * Determine whether this flow depends on the specified host entry
     * represented by a pair of MAC address and a VLAN ID or not.
     *
     * @param mvlan  A pair of MAC address and VLAN ID.
     * @return  {@code true} is returned only if this flow depends on the
     *          specified host entry.
     */
    public boolean dependsOn(MacVlan mvlan) {
        return dependHosts.contains(mvlan);
    }

    /**
     * Determine whether the given pair of node connector and VLAN ID matches
     * the network at the edge of the ingress flow or not.
     *
     * @param nc    A node connector associated with a switch port.
     * @param vlan  A VLAN ID.
     * @return  {@code true} is returned if the given pair of node connector
     *          and VLAN ID matches the network at the edge of the ingress
     *          flow. Otherwise {@code false} is returned.
     */
    public boolean isIncomingNetwork(NodeConnector nc, short vlan) {
        if (flowEntries.size() == 0) {
            return false;
        }

        // Both IN_PORT and DL_VLAN fields should be contained in an ingress
        // flow entry.
        Flow flow = flowEntries.get(0).getFlow();
        Match match = flow.getMatch();
        MatchField mf = match.getField(MatchType.IN_PORT);
        if (!nc.equals(mf.getValue())) {
            return false;
        }

        mf = match.getField(MatchType.DL_VLAN);
        Short vid = (Short)mf.getValue();
        return (vid.shortValue() == vlan);
    }

    /**
     * Determine whether the given pair of node connector and VLAN ID matches
     * the network at the edge of the egress flow or not.
     *
     * @param nc    A node connector associated with a switch port.
     * @param vlan  A VLAN ID.
     * @return  {@code true} is returned if the given pair of node connector
     *          and VLAN ID matches the network at the edge of the egress flow.
     *          Otherwise {@code false} is returned.
     */
    public boolean isOutgoingNetwork(NodeConnector nc, short vlan) {
        int sz = flowEntries.size();
        if (sz <= 0) {
            return false;
        }

        List<Action> actions = flowEntries.get(sz - 1).getFlow().getActions();
        if (actions == null) {
            return false;
        }

        boolean portMatched = false, vlanMatched = false;
        for (Action action: actions) {
            if (action instanceof Output) {
                Output out = (Output)action;
                if (nc.equals(out.getPort())) {
                    if (vlanMatched) {
                        return true;
                    }
                    portMatched = true;
                }
            } else if (getOutputVlan(action) == vlan) {
                if (portMatched) {
                    return true;
                }
                vlanMatched = true;
            }
        }

        return false;
    }

    /**
     * Update indices of flow entries.
     *
     * @param swMgr  Switch manager service. If a non-{@code null} value is
     *               specified, this method eliminates pseudo ports from
     *               port index.
     * @param flow   A SAL flow.
     * @param node   A node associated with the given flow.
     */
    private void updateIndex(ISwitchManager swMgr, Flow flow, Node node) {
        flowNodes.add(node);

        // Determine incoming switch port.
        // IN_PORT field should be contained in a flow entry.
        Match match = flow.getMatch();
        MatchField mf = match.getField(MatchType.IN_PORT);
        NodeConnector port = (NodeConnector)mf.getValue();
        assert node.equals(port.getNode());
        flowPorts.add(port);

        // Determine outgoing switch port.
        List<Action> actions = flow.getActions();
        if (actions != null) {
            for (Action action: actions) {
                if (action instanceof Output) {
                    Output out = (Output)action;
                    assert node.equals(port.getNode());
                    port = out.getPort();
                    flowNodes.add(port.getNode());

                    // Eliminate special port from port index in order to
                    // reduce index size.
                    if (swMgr == null || !swMgr.isSpecial(port)) {
                        flowPorts.add(port);
                    }
                }
            }
        }
    }

    /**
     * Determine whether this VTN flow is relevant to local nodes.
     *
     * <p>
     *   Note that this method always returns {@code ConnectionLocality.LOCAL}
     *   if this VTN flow is originated by this controller.
     * </p>
     *
     * @param mgr  VTN Manager service.
     * @return  {@code true} is returned if this VTN flow contains at least
     *          one flow entry installed to the switch connected to this
     *          controller.
     *          Otherwise {@code false} is returned.
     */
    public boolean isLocal(VTNManagerImpl mgr) {
        if (groupId.isLocal()) {
            return true;
        }

        IConnectionManager cnm = mgr.getConnectionManager();
        for (Node node: flowNodes) {
            ConnectionLocality cl = cnm.getLocalityStatus(node);
            if (cl == ConnectionLocality.LOCAL) {
                return true;
            }
        }

        return false;
    }

    /**
     * Read data from the given input stream and deserialize.
     *
     * @param in  An input stream.
     * @throws IOException
     *    An I/O error occurred.
     * @throws ClassNotFoundException
     *    At least one necessary class was not found.
     */
    @SuppressWarnings("unused")
    private void readObject(ObjectInputStream in)
        throws IOException, ClassNotFoundException {
        in.defaultReadObject();

        // Update indices for node and port.
        ISwitchManager swMgr = (ISwitchManager)ServiceHelper.
            getInstance(ISwitchManager.class,
                        GlobalConstants.DEFAULT.toString(), this);
        flowNodes = new HashSet<Node>();
        flowPorts = new HashSet<NodeConnector>();
        for (FlowEntry fent: flowEntries) {
            updateIndex(swMgr, fent.getFlow(), fent.getNode());
        }
    }

    /**
     * Return VLAN ID of outgoing packet set by the given action.
     *
     * @param action  An action in a flow entry.
     * @return  If the given action removes the VLAN tag,
     *          {@link MatchType#DL_VLAN_NONE} is returned.
     *          A valid VLAN ID is returned if the given action sets it into
     *          the VLAN TAG.
     *          -1 is returned if the given action never sets a VLAN ID.
     */
    private short getOutputVlan(Action action) {
        if (action instanceof PopVlan) {
            return MatchType.DL_VLAN_NONE;
        }
        if (action instanceof SetVlanId) {
            return (short)((SetVlanId)action).getVlanId();
        }

        return -1;
    }

    /**
     * Determine whether the given object is identical to this object.
     *
     * @param o  An object to be compared.
     * @return   {@code true} if identical. Otherwise {@code false}.
     */
    @Override
    public boolean equals(Object o) {
        if (o == this) {
            return true;
        }
        if (!(o instanceof VTNFlow)) {
            return false;
        }

        VTNFlow vflow = (VTNFlow)o;
        if (!groupId.equals(vflow.groupId)) {
            return false;
        }

        return flowEntries.equals(vflow.flowEntries);
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        return groupId.hashCode() ^ flowEntries.hashCode();
    }
}
