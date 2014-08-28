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
import java.util.Collection;
import java.util.List;
import java.util.ArrayList;
import java.util.Set;
import java.util.HashSet;

import org.opendaylight.vtn.manager.NodeRoute;
import org.opendaylight.vtn.manager.PortLocation;
import org.opendaylight.vtn.manager.VNodePath;
import org.opendaylight.vtn.manager.VNodeRoute.Reason;
import org.opendaylight.vtn.manager.VNodeRoute;
import org.opendaylight.vtn.manager.VTenantPath;
import org.opendaylight.vtn.manager.flow.DataFlow;
import org.opendaylight.vtn.manager.internal.ActionList;
import org.opendaylight.vtn.manager.internal.L2Host;
import org.opendaylight.vtn.manager.internal.PortFilter;
import org.opendaylight.vtn.manager.internal.VTNManagerImpl;

import org.opendaylight.controller.connectionmanager.IConnectionManager;
import org.opendaylight.controller.forwardingrulesmanager.FlowEntry;
import org.opendaylight.controller.sal.action.Action;
import org.opendaylight.controller.sal.action.Drop;
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
    private static final long serialVersionUID = -6210984131359256401L;

    /**
     * The identifier of the flow group.
     */
    private final FlowGroupId  groupId;

    /**
     * Creation time of this VTN flow.
     *
     * <p>
     *   Note that this field does not affect identify of this instance.
     * </p>
     */
    private long  creationTime;

    /**
     * Flow entries.
     */
    private final List<FlowEntry>  flowEntries = new ArrayList<FlowEntry>();

    /**
     * Set of nodes related to this flow.
     *
     * <p>
     *   Note that this field does not affect identify of this instance.
     * </p>
     */
    private transient Set<Node>  flowNodes = new HashSet<Node>();

    /**
     * Set of switch ports related to this flow.
     *
     * <p>
     *   Note that this field does not affect identify of this instance.
     * </p>
     */
    private transient Set<NodeConnector>  flowPorts =
        new HashSet<NodeConnector>();

    /**
     * Path policy identifier that determined the packet route of this flow.
     *
     * <p>
     *   Note that this field does not affect identify of this instance.
     * </p>
     */
    private int  pathPolicy;

    /**
     * Set of virtual node paths on which this flow depends.
     *
     * <p>
     *   Note that this field does not affect identify of this instance.
     * </p>
     */
    private Set<VTenantPath>  dependNodes;

    /**
     * A sequence of virtual packet routing.
     *
     * <p>
     *   Note that this field does not affect identify of this instance.
     * </p>
     */
    private final List<VNodeRoute> virtualRoute = new ArrayList<VNodeRoute>();

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
     * Return the idle timeout configured for the ingress flow.
     *
     * @return  The number of seconds configured as the idle timeout for the
     *          ingress flow.
     *          Note that zero is returned if no flow entry is configured.
     */
    public short getIdleTimeout() {
        if (flowEntries.isEmpty()) {
            return 0;
        }

        Flow ingress = flowEntries.get(0).getFlow();
        return ingress.getIdleTimeout();
    }

    /**
     * Return the hard timeout configured for the ingress flow.
     *
     * @return  The number of seconds configured as the hard timeout for the
     *          ingress flow.
     *          Note that zero is returned if no flow entry is configured.
     */
    public short getHardTimeout() {
        if (flowEntries.isEmpty()) {
            return 0;
        }

        Flow ingress = flowEntries.get(0).getFlow();
        return ingress.getHardTimeout();
    }

    /**
     * Return the creation time of this VTN flow in milliseconds.
     *
     * @return  The number of milliseconds between the creation time of this
     *          VTN flow and 1970-1-1 00:00:00 UTC.
     */
    public long getCreationTime() {
        return creationTime;
    }

    /**
     * Fix up the VTN flow before installation.
     */
    public void fixUp() {
        // Set creation time.
        creationTime = System.currentTimeMillis();
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
     * Add a flow entry that drops matched packets.
     *
     * @param mgr       VTN Manager service.
     * @param match     Match object for a new flow entry.
     * @param priority  Priority value for a new flow entry.
     */
    public void addFlow(VTNManagerImpl mgr, Match match, int priority) {
        // IN_PORT field should be contained in a flow entry.
        MatchField mf = match.getField(MatchType.IN_PORT);
        NodeConnector port = (NodeConnector)mf.getValue();
        List<Action> actions = new ArrayList<Action>(1);
        actions.add(new Drop());
        Flow flow = new Flow(match, actions);
        flow.setPriority((short)priority);
        addFlow(mgr, flow, port.getNode());
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
     * Add the given node path to the additional dependency set.
     *
     * @param path  A path to the virtual node.
     */
    public void addDependency(VTenantPath path) {
        if (dependNodes == null) {
            dependNodes = new HashSet<VTenantPath>();
        }
        dependNodes.add(path);
    }

    /**
     * Add all virtual node paths in the given set to the additional dependency
     * set.
     *
     * @param pathSet  A set of path to the virtual nodes on which this flow
     *                 depends.
     */
    public void addDependency(Set<VTenantPath> pathSet) {
        if (dependNodes == null) {
            dependNodes = new HashSet<VTenantPath>();
        }
        dependNodes.addAll(pathSet);
    }

    /**
     * Add the specified {@link VNodeRoute} to the tail of the virtual packet
     * routing path.
     *
     * @param vroute  A {@link VNodeRoute} instance.
     */
    public void addVirtualRoute(VNodeRoute vroute) {
        virtualRoute.add(vroute);
    }

    /**
     * Add the specified sequence of {@link VNodeRoute} to the tail of the
     * virtual packet routing path.
     *
     * @param c  A collection of {@link VNodeRoute} instances.
     */
    public void addVirtualRoute(Collection<VNodeRoute> c) {
        virtualRoute.addAll(c);
    }

    /**
     * Clear virtual packet route for this VTN flow.
     *
     * <p>
     *   This method is used only for testing.
     * </p>
     */
    public void clearVirtualRoute() {
        virtualRoute.clear();
    }

    /**
     * Set the specified virtual node as the egress node of this VTN flow.
     *
     * <p>
     *   This method must be called after virtual packet routing path is
     *   configured by {@link #addVirtualRoute(Collection)}.
     * </p>
     *
     * @param path  A {@link VNodePath} instance which represents the location
     *              of the egress node. Specifying {@code null} means that
     *              this VTN flow always discards packets.
     */
    public void setEgressVNodePath(VNodePath path) {
        int lastIndex = virtualRoute.size() - 1;
        if (path == null) {
            // Append an empty route which represents a negative flow.
            if (lastIndex < 0 ||
                virtualRoute.get(lastIndex).getPath() != null) {
                virtualRoute.add(new VNodeRoute());
            }
            return;
        }

        // Need to adjust the last element of the virtual node path if
        // it contains more than 1 elements.
        if (lastIndex > 0) {
            // A VNodePath which specifies the virtual mapping which
            // established the egress flow must be always configured in the
            // last VNodeRoute element of the virtual packet routing path.
            VNodeRoute last = virtualRoute.get(lastIndex);
            VNodePath lastPath = last.getPath();
            if (path.equals(lastPath)) {
                // The last route element already points the specified virtual
                // mapping.
                return;
            }

            if ((path instanceof VBridgeMapPath) && lastPath.contains(path)) {
                // A path to the virtual mapping must be configured in the last
                // virtual packet route.
                VNodeRoute newLast = new VNodeRoute(path, last.getReason());
                virtualRoute.set(lastIndex, newLast);
                return;
            }
        }

        virtualRoute.add(new VNodeRoute(path, Reason.FORWARDED));
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
        if (path != null) {
            for (VNodeRoute vroute: virtualRoute) {
                if (path.contains(vroute.getPath())) {
                    return true;
                }
            }
            if (dependNodes != null) {
                return dependNodes.contains(path);
            }
        }

        return false;
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
     *          {@code null} is returned if this flow does not contain any
     *          flow.
     */
    public VNodePath getIngressPath() {
        return (virtualRoute.isEmpty())
            ? null
            : virtualRoute.get(0).getPath();
    }

    /**
     * Return a path to virtual node which maps the egress flow.
     *
     * @return  A path to virtual node which maps the egress flow.
     *          {@code null} is returned if this flow does not contain a
     *          egress flow.
     */
    public VNodePath getEgressPath() {
        int sz = virtualRoute.size();
        return (sz > 0) ? virtualRoute.get(sz - 1).getPath() : null;
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
     * Get a {@link L2Host} instance which represents the incoming host
     * address.
     *
     * @return  A {@link L2Host} instance which represents the incoming host
     *          address. {@code null} is returned if this VTN flow contains
     *          no flow entry.
     */
    public L2Host getSourceHost() {
        if (flowEntries.isEmpty()) {
            return null;
        }

        // Check ingress flow.
        Match match = flowEntries.get(0).getFlow().getMatch();
        return getSourceHost(match);
    }

    /**
     * Get two {@link L2Host} instance which represents host information
     * matched by edge flow entries.
     *
     * @return  A {@link ObjectPair} instance which contains two
     *          {@link L2Host} instances. A {@link L2Host} instance which
     *          represents incoming host address is set to the left side,
     *          and outgoing host address is set to the right side.
     *          Note that {@code null} may be set for outgoing host.
     *          {@code null} is returned if not found.
     */
    public ObjectPair<L2Host, L2Host> getEdgeHosts() {
        int sz = flowEntries.size();
        if (sz <= 0) {
            return null;
        }

        // Check ingress flow.
        Match match = flowEntries.get(0).getFlow().getMatch();
        L2Host in = getSourceHost(match);
        MatchField mf = match.getField(MatchType.DL_DST);
        byte[] dst = (mf == null) ? null : (byte[])mf.getValue();
        short vlan = in.getHost().getVlan();

        // Check egress flow.
        List<Action> actions = flowEntries.get(sz - 1).getFlow().getActions();
        L2Host out = getDestinationHost(actions, dst, vlan);

        return new ObjectPair<L2Host, L2Host>(in, out);
    }

    /**
     * Return a {@link DataFlow} instance which represents information about
     * this VTN flow.
     *
     * @param mgr     VTN Manager service.
     * @param detail  If {@code true} is specified, detailed information
     *                is set into a returned {@link DataFlow} instance.
     *                Otherwise summary of the data flow is set.
     * @return  A {@link DataFlow} instance.
     *          {@code null} is returned if this VTN flow does not contain
     *          SAL flow.
     */
    public DataFlow getDataFlow(VTNManagerImpl mgr, boolean detail) {
        int sz = flowEntries.size();
        if (sz <= 0) {
            return null;
        }

        // Determine ingress port.
        // IN_PORT field should be contained in a flow entry.
        FlowEntry ingress = flowEntries.get(0);
        Flow iflow = ingress.getFlow();
        MatchField mf = iflow.getMatch().getField(MatchType.IN_PORT);
        NodeConnector inPort = (NodeConnector)mf.getValue();
        PortLocation inLoc =
            new PortLocation(inPort, mgr.getPortName(inPort));

        // Flow timeout is configured only in the ingress flow.
        short idle = iflow.getIdleTimeout();
        short hard = iflow.getHardTimeout();

        // Determine egress port.
        Flow eflow = flowEntries.get(sz - 1).getFlow();
        NodeConnector outPort = getOutputPort(eflow.getActions());
        PortLocation outLoc = (outPort == null)
            ? null
            : new PortLocation(outPort, mgr.getPortName(outPort));

        VNodePath inPath = getIngressPath();
        VNodePath outPath = getEgressPath();
        DataFlow df = new DataFlow(groupId.getEventId(), creationTime,
                                   idle, hard, inPath, inLoc, outPath, outLoc);

        if (detail) {
            // Set information about ingress and egress flow entries.
            df.setEdgeFlows(iflow, eflow);

            // Set virtual packet route of the data flow.
            df.setVirtualRoute(virtualRoute);

            // Determine physical packet route of the data flow.
            for (FlowEntry fent: flowEntries) {
                NodeRoute nroute = getNodeRoute(mgr, fent.getFlow());
                if (nroute == null) {
                    df.clearPhysicalRoute();
                    break;
                }
                df.addPhysicalRoute(nroute);
            }
        }

        return df;
    }

    /**
     * Set the identifier of the path policy which routed this flow.
     *
     * @param id  The identifier of the path policy.
     */
    public void setPathPolicy(int id) {
        pathPolicy = id;
    }

    /**
     * Return the identifier of the path policy which routed this flow.
     *
     * @return  The identifier of the path policy.
     */
    public int getPathPolicy() {
        return pathPolicy;
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
        for (Action action: actions) {
            if (action instanceof SetDlDst) {
                dst = ((SetDlDst)action).getDlAddress();
            } else if (action instanceof Output) {
                // We assume that OUTPUT action comes last.
                port = ((Output)action).getPort();
                break;
            } else {
                short v = getOutputVlan(action);
                if (v != -1) {
                    vlan = v;
                }
            }
        }

        if (port == null) {
            return null;
        }
        if (dst == null) {
            return new L2Host(vlan, port);
        }
        return new L2Host(dst, vlan, port);
    }

    /**
     * Determine the physical switch port configured in the first OUTPUT
     * in the given action list.
     *
     * @param actions  A list of SAL actions.
     * @return  A {@link NodeConnector} instance if found.
     *          {@code null} if not found.
     */
    private NodeConnector getOutputPort(List<Action> actions) {
        if (actions != null) {
            for (Action action: actions) {
                if (action instanceof Output) {
                    Output out = (Output)action;
                    return out.getPort();
                }
            }
        }

        return null;
    }

    /**
     * Return a {@link NodeRoute} instance which represents the physical packet
     * routing configured by the given SAL flow.
     *
     * @param mgr   VTN Manager service.
     * @param flow  A SAL flow.
     * @return  A {@link NodeRoute} instance.
     *          {@code null} is returned if the given SAL flow discards the
     *          packet.
     */
    private NodeRoute getNodeRoute(VTNManagerImpl mgr, Flow flow) {
        // Determine egress port.
        NodeConnector out = getOutputPort(flow.getActions());
        if (out == null) {
            return null;
        }

        // Determine ingress port.
        // IN_PORT field should be contained in every flow entry.
        MatchField mf = flow.getMatch().getField(MatchType.IN_PORT);
        NodeConnector in = (NodeConnector)mf.getValue();
        String inName = mgr.getPortName(in);
        String outName = mgr.getPortName(out);

        return new NodeRoute(in, inName, out, outName);
    }

    /**
     * Get a {@link L2Host} instance which represents the incoming host
     * address.
     *
     * @param match  A flow match configured in the ingress flow.
     * @return  A {@link L2Host} instance which represents the incoming host
     *          address.
     */
    private L2Host getSourceHost(Match match) {
        // Both IN_PORT and DL_VLAN fields should be contained in the ingress
        // flow entry.
        MatchField mf = match.getField(MatchType.IN_PORT);
        NodeConnector inPort = (NodeConnector)mf.getValue();
        mf = match.getField(MatchType.DL_VLAN);
        short vlan = ((Short)mf.getValue()).shortValue();

        mf = match.getField(MatchType.DL_SRC);
        if (mf != null) {
            byte[] src = (byte[])mf.getValue();
            return new L2Host(src, vlan, inPort);
        }

        return new L2Host(vlan, inPort);
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
