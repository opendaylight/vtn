/*
 * Copyright (c) 2013-2014 NEC Corporation
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

import org.opendaylight.vtn.manager.VBridgePath;
import org.opendaylight.vtn.manager.VTenantPath;
import org.opendaylight.vtn.manager.internal.ActionList;
import org.opendaylight.vtn.manager.internal.L2Host;
import org.opendaylight.vtn.manager.internal.NodeUtils;
import org.opendaylight.vtn.manager.internal.PortFilter;
import org.opendaylight.vtn.manager.internal.VTNManagerImpl;

import org.opendaylight.controller.connectionmanager.IConnectionManager;
import org.opendaylight.controller.forwardingrulesmanager.FlowEntry;
import org.opendaylight.controller.sal.action.Action;
import org.opendaylight.controller.sal.action.Output;
import org.opendaylight.controller.sal.action.PopVlan;
import org.opendaylight.controller.sal.action.SetDlDst;
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
    private static final long serialVersionUID = -8156281482065842424L;

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
     *
     * <p>
     *   Note that this field does not affect identify of this instance.
     * </p>
     */
    private final Set<VTenantPath>  dependNodes = new HashSet<VTenantPath>();

    /**
     * A path to virtual node which maps the incoming flow.
     *
     * <p>
     *   Note that this field does not affect identify of this instance.
     * </p>
     */
    private VBridgePath  ingressPath;

    /**
     * A path to virtual node which maps the egress flow.
     *
     * <p>
     *   Note that this field does not affect identify of this instance.
     * </p>
     */
    private VBridgePath  egressPath;

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
        if (!flowEntries.isEmpty()) {
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
     * Determine whether this flow depends on the specified virtual node
     * or not.
     *
     * @param path  Path to the virtual node.
     * @return  {@code true} is returned only if this flow depends on the
     *          virtual node specified by {@code path}.
     */
    public boolean dependsOn(VTenantPath path) {
        return (path != null &&
                (path.contains(ingressPath) || path.contains(egressPath) ||
                 dependNodes.contains(path)));
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
        ObjectPair<L2Host, L2Host> pair = getEdgeHosts();
        if (pair == null) {
            return false;
        }

        L2Host in = pair.getLeft();
        if (mvlan.equals(in.getHost())) {
            return true;
        }

        L2Host out = pair.getRight();
        return (out != null && mvlan.equals(out.getHost()));
    }

    /**
     * Return a path to virtual node which maps the ingress flow.
     *
     * @return  A path to virtual node which maps the ingress flow.
     */
    public VBridgePath getIngressPath() {
        return ingressPath;
    }

    /**
     * Set a path to virtual node which maps the ingress flow.
     *
     * @param path  A path to virtual node which maps the ingress flow.
     */
    public void setIngressPath(VBridgePath path) {
        ingressPath = path;
    }

    /**
     * Return a path to virtual node which maps the egress flow.
     *
     * @return  A path to virtual node which maps the egress flow.
     *          {@code null} is returned if this flow does not contain a
     *          egress flow.
     */
    public VBridgePath getEgressPath() {
        return egressPath;
    }

    /**
     * Set a path to virtual node which maps the egress flow.
     *
     * @param path  A path to virtual node which maps the egress flow.
     */
    public void setEgressPath(VBridgePath path) {
        egressPath = path;
    }

    /**
     * Determine whether the specified network matches the network configured
     * in the ingress flow or not.
     *
     * <p>
     *   The network to be tested is specified by a pair of {@link PortFilter}
     *   instance and VLAN ID. This method passes a {@link NodeConnector}
     *   instance configured in the ingress flow to
     *   {@link PortFilter#accept(NodeConnector, PortProperty)}, and
     *   returns {@code false} if it returns {@code false}.
     *   Note that {@code null} is always passed as port property.
     * </p>
     *
     * @param filter  A {@link PortFilter} instance.
     *                Specifying {@code null} results in undefined behavior.
     * @param vlan    A VLAN ID.
     * @return  {@code true} is returned if the specified network matches
     *          the network configured in the ingress flow.
     *          Otherwise {@code false} is returned.
     */
    public boolean isIncomingNetwork(PortFilter filter, short vlan) {
        if (flowEntries.isEmpty()) {
            return false;
        }

        // Both IN_PORT and DL_VLAN fields should be contained in an ingress
        // flow entry.
        Flow flow = flowEntries.get(0).getFlow();
        Match match = flow.getMatch();
        MatchField mf = match.getField(MatchType.IN_PORT);
        NodeConnector port = (NodeConnector)mf.getValue();
        if (!filter.accept(port, null)) {
            return false;
        }

        mf = match.getField(MatchType.DL_VLAN);
        Short vid = (Short)mf.getValue();
        return (vid.shortValue() == vlan);
    }

    /**
     * Determine whether the specified network matches the network configured
     * in the egress flow or not.
     *
     * <p>
     *   The network to be tested is specified by a pair of {@link PortFilter}
     *   instance and VLAN ID. This method passes a {@link NodeConnector}
     *   instance configured in the ingress flow to
     *   {@link PortFilter#accept(NodeConnector, PortProperty)}, and
     *   returns {@code false} if it returns {@code false}.
     *   Note that {@code null} is always passed as port property.
     * </p>
     *
     * @param filter  A {@link PortFilter} instance.
     *                Specifying {@code null} results in undefined behavior.
     * @param vlan    A VLAN ID.
     * @return  {@code true} is returned if the specified network matches
     *          the network configured in the egress flow.
     *          Otherwise {@code false} is returned.
     */
    public boolean isOutgoingNetwork(PortFilter filter, short vlan) {
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
                if (filter.accept(out.getPort(), null)) {
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
                    if (swMgr == null ||
                        !NodeUtils.isSpecial(swMgr, port, null)) {
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
     * Get two {@link L2Host} instance which represents host information
     * matched by edge flow entries.
     *
     * @return  A {@link ObjectPair} instance which contains two
     *          {@link L2Host} instances. A {@link L2Host} instance which
     *          represents incoming host address is set to the left side,
     *          and outgoing host address is set to the right side.
     *          Node that {@code null} may be set for outgoing host.
     *          {@code null} is returned if not found.
     */
    public ObjectPair<L2Host, L2Host> getEdgeHosts() {
        int sz = flowEntries.size();
        if (sz <= 0) {
            return null;
        }

        // Check ingress flow.
        // DL_SRC, DL_DST, IN_PORT, and DL_VLAN fields should be contained
        // in the ingress flow.
        Match match = flowEntries.get(0).getFlow().getMatch();
        MatchField mf = match.getField(MatchType.IN_PORT);
        NodeConnector inPort = (NodeConnector)mf.getValue();
        mf = match.getField(MatchType.DL_SRC);
        byte[] src = (byte[])mf.getValue();
        mf = match.getField(MatchType.DL_VLAN);
        short vlan = ((Short)mf.getValue()).shortValue();
        L2Host in = new L2Host(src, vlan, inPort);

        mf = match.getField(MatchType.DL_DST);
        byte[] dst = (byte[])mf.getValue();

        // Check egress flow.
        List<Action> actions = flowEntries.get(sz - 1).getFlow().getActions();
        L2Host out = getDestinationHost(actions, dst, vlan);

        return new ObjectPair<L2Host, L2Host>(in, out);
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
     * Determine the destination host configured in the specified list of
     * flow actions.
     *
     * @param actions  A list of flow actions.
     * @param addr     Destination address configured in the ingress flow.
     * @param inVlan   VLAN ID configured in the ingress flow.
     * @return  A {@link L2Host} instance is returned if the destination host
     *          is configured in this flow.
     *          {@code null} is returned if not configured.
     */
    private L2Host getDestinationHost(List<Action> actions, byte[] addr,
                                      short inVlan) {
        if (actions == null) {
            return null;
        }

        byte[] dst = addr;
        NodeConnector port = null;
        short vlan = inVlan;
        boolean vlanSet = false;
        for (Action action: actions) {
            if (action instanceof SetDlDst) {
                dst = ((SetDlDst)action).getDlAddress();
            } else if (action instanceof Output) {
                port = ((Output)action).getPort();
            } else {
                short v = getOutputVlan(action);
                if (v != -1) {
                    vlan = v;
                    vlanSet = true;
                }
            }

            // We assume that there is no duplicate OUTPUT, DL_DST, VLAN_VID
            // actions in the action list.
            if (dst != addr && port != null && vlanSet) {
                break;
            }
        }

        return (port == null) ? null : new L2Host(dst, vlan, port);
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
