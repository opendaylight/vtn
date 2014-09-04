/*
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.cluster;

import java.util.List;
import java.util.concurrent.ConcurrentMap;
import java.util.concurrent.locks.Lock;

import org.slf4j.Logger;

import org.opendaylight.vtn.manager.PortMap;
import org.opendaylight.vtn.manager.PortMapConfig;
import org.opendaylight.vtn.manager.VInterfacePath;
import org.opendaylight.vtn.manager.VNodePath;
import org.opendaylight.vtn.manager.VNodeState;
import org.opendaylight.vtn.manager.VTNException;
import org.opendaylight.vtn.manager.VTenantPath;

import org.opendaylight.vtn.manager.internal.ActionList;
import org.opendaylight.vtn.manager.internal.EdgeUpdateState;
import org.opendaylight.vtn.manager.internal.LockStack;
import org.opendaylight.vtn.manager.internal.PacketContext;
import org.opendaylight.vtn.manager.internal.RouteResolver;
import org.opendaylight.vtn.manager.internal.VTNFlowDatabase;
import org.opendaylight.vtn.manager.internal.VTNManagerImpl;

import org.opendaylight.controller.sal.core.Edge;
import org.opendaylight.controller.sal.core.Node;
import org.opendaylight.controller.sal.core.NodeConnector;
import org.opendaylight.controller.sal.core.Path;
import org.opendaylight.controller.sal.core.UpdateType;
import org.opendaylight.controller.sal.match.Match;
import org.opendaylight.controller.sal.packet.Ethernet;
import org.opendaylight.controller.sal.packet.PacketResult;
import org.opendaylight.controller.sal.utils.NetUtils;

/**
 * {@code PortBridge} class describes virtual bridge that contains
 * virtual interfaces that map physical switch port.
 *
 * <p>
 *   Although this class is public to other packages, this class does not
 *   provide any API. Applications other than VTN Manager must not use this
 *   class.
 * </p>
 *
 * @param <T>  Type of virtual interface.
 */
public abstract class PortBridge<T extends PortInterface>
    extends AbstractBridge<T> {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = -3262872639041471371L;

    /**
     * Construct an abstract bridge node that can have port mappings.
     *
     * @param vtn   The virtual tenant to which a new node belongs.
     * @param name  The name of this node.
     */
    PortBridge(VTenantImpl vtn, String name) {
        super(vtn, name);
    }

    /**
     * Return the port mapping configuration applied to the specified virtual
     * interface.
     *
     * @param mgr   VTN Manager service.
     * @param path  Path to the virtual interface.
     * @return  Port mapping information.
     *          {@code null} is returned if port mapping is not configured.
     * @throws VTNException  An error occurred.
     */
    final PortMap getPortMap(VTNManagerImpl mgr, VInterfacePath path)
        throws VTNException {
        Lock rdlock = readLock();
        try {
            T vif = getInterfaceImpl(path);
            return vif.getPortMap(mgr);
        } finally {
            rdlock.unlock();
        }
    }

    /**
     * Create or destroy mapping between the physical switch port and the
     * virtual interface.
     *
     * @param mgr     VTN Manager service.
     * @param path    Path to the virtual interface.
     * @param pmconf  Port mapping configuration to be set.
     *                If {@code null} is specified, port mapping on the
     *                specified interface is destroyed.
     * @return  {@code true} is returned if the port mapping configuration
     *          was changed. Otherwise {@code false} is returned.
     * @throws VTNException  An error occurred.
     */
    final boolean setPortMap(VTNManagerImpl mgr, VInterfacePath path,
                             PortMapConfig pmconf) throws VTNException {
        // Acquire write lock to serialize port mapping change.
        Lock wrlock = writeLock();
        try {
            T vif = getInterfaceImpl(path);
            VNodeState ifState = vif.setPortMap(mgr, pmconf);
            if (ifState == null) {
                // Unchanged.
                return false;
            }

            if (vif.isEnabled()) {
                if (ifState == VNodeState.DOWN) {
                    setState(mgr, VNodeState.DOWN);
                } else {
                    updateState(mgr);
                }
            }

            return true;
        } finally {
            wrlock.unlock();
        }
    }

    /**
     * Invoked when a node is added, removed, or changed.
     *
     * @param mgr   VTN Manager service.
     * @param db    Virtual node state DB.
     * @param node  Node being updated.
     * @param type  Type of update.
     */
    void notifyNode(VTNManagerImpl mgr, ConcurrentMap<VTenantPath, Object> db,
                    Node node, UpdateType type) {
        Lock wrlock = writeLock();
        try {
            BridgeState bst = getBridgeState(db);
            VNodeState state = notifyIfNode(mgr, db, bst, node, type);
            setState(mgr, db, bst, state);
        } finally {
            wrlock.unlock();
        }
    }

    /**
     * This method is called when some properties of a node connector are
     * added/deleted/changed.
     *
     * @param mgr     VTN Manager service.
     * @param db      Virtual node state DB.
     * @param nc      Node connector being updated.
     * @param pstate  The state of the node connector.
     * @param type    Type of update.
     */
    void notifyNodeConnector(VTNManagerImpl mgr,
                             ConcurrentMap<VTenantPath, Object> db,
                             NodeConnector nc, VNodeState pstate,
                             UpdateType type) {
        Lock wrlock = writeLock();
        try {
            BridgeState bst = getBridgeState(db);
            VNodeState state = notifyIfNodeConnector(mgr, db, bst, nc, pstate,
                                                     type);
            setState(mgr, db, bst, state);
        } finally {
            wrlock.unlock();
        }
    }

    /**
     * This method is called when topology graph is changed.
     *
     * @param mgr     VTN Manager service.
     * @param db      Virtual node state DB.
     * @param estate  A {@link EdgeUpdateState} instance which contains
     *                information reported by the controller.
     */
    void edgeUpdate(VTNManagerImpl mgr, ConcurrentMap<VTenantPath, Object> db,
                    EdgeUpdateState estate) {
        Lock wrlock = writeLock();
        try {
            BridgeState bst = getBridgeState(db);
            VNodeState state = edgeIfUpdate(mgr, db, bst, estate);
            setState(mgr, db, bst, state);
        } finally {
            wrlock.unlock();
        }
    }

    /**
     * Send a unicast ARP request to the specified host.
     *
     * @param mgr   VTN manager service.
     * @param ref   Reference to the virtual network mapping.
     *              The specified reference must point the virtual node
     *              contained in this node.
     * @param pctx  The context of the ARP packet to send.
     * @return  {@code true} is returned if a ARP request was actually sent to
     *          the network.
     *          {@code null} is returned if the specified host does not
     *          belong to this bridge.
     * @throws VTNException  An error occurred.
     */
    final boolean probeHost(VTNManagerImpl mgr, MapReference ref,
                            PacketContext pctx) throws VTNException {
        NodeConnector nc = pctx.getOutgoingNodeConnector();
        assert nc != null;
        short vlan = pctx.getVlan();
        long mac = NetUtils.byteArray6ToLong(pctx.getDestinationAddress());

        Lock rdlock = readLock();
        try {
            VirtualMapNode vnode = match(mgr, ref, mac, nc, vlan, false);
            if (vnode == null) {
                return false;
            }

            Logger logger = getLogger();
            if (!vnode.isEnabled()) {
                if (logger.isDebugEnabled()) {
                    logger.debug("{}:{}: Don't send ARP request to " +
                                 "disabled network: {}",
                                 getContainerName(), vnode.getPath(),
                                 pctx.getDescription(nc));
                }
                return false;
            }

            boolean ret = mgr.transmit(nc, pctx.getFrame());
            if (ret && logger.isTraceEnabled()) {
                logger.trace("{}:{}: Send ARP request for probing: {}",
                             getContainerName(), getNodePath(),
                             pctx.getDescription(nc));
            }
            return ret;
        } finally {
            rdlock.unlock();
        }
    }

    /**
     * Handler for receiving the packet.
     *
     * @param mgr   VTN manager service.
     * @param ref   Reference to the virtual network mapping.
     *              The specified reference must point the virtual node
     *              contained in this node.
     * @param pctx  The context of the received packet.
     * @return  A {@code PacketResult} which indicates the result of handler.
     * @throws DropFlowException
     *    The given packet was discarded by a flow filter.
     * @throws RedirectFlowException
     *    The given packet was redirected by a flow filter.
     */
    final PacketResult receive(VTNManagerImpl mgr, MapReference ref,
                               PacketContext pctx)
        throws DropFlowException, RedirectFlowException {
        NodeConnector incoming = pctx.getIncomingNodeConnector();
        short vlan = pctx.getVlan();
        long mac = NetUtils.byteArray6ToLong(pctx.getSourceAddress());

        // Writer lock is required because this method may change the state
        // of the bridge.
        Lock wrlock = writeLock();
        try {
            VirtualMapNode vnode = match(mgr, ref, mac, incoming, vlan, true);
            if (vnode == null) {
                return PacketResult.IGNORED;
            }

            if (vnode.isEnabled()) {
                pctx.addVNodeRoute(vnode.getIngressRoute());

                // Evaluate flow filters configured in the virtual mapping.
                // Actually this evaluates virtual interface flow filters for
                // incoming packets.
                vnode.filterPacket(mgr, pctx, FlowFilterMap.VLAN_UNSPEC);

                return handlePacket(mgr, pctx, vnode);
            } else {
                Logger logger = getLogger();
                if (logger.isDebugEnabled()) {
                    logger.debug("{}:{}: Ignore packet received from " +
                                 "disabled network: {}", getContainerName(),
                                 vnode.getPath(),
                                 pctx.getDescription(incoming));
                }
                vnode.disableInput(mgr, pctx);
                return PacketResult.CONSUME;
            }
        } catch (VTNException e) {
            // This should never happen.
            mgr.logException(getLogger(), getNodePath(), e);
        } finally {
            wrlock.unlock();
        }

        return PacketResult.KEEP_PROCESSING;
    }

    /**
     * Redirect the given packet to the specified virtual interface.
     *
     * @param mgr   VTN Manager service.
     * @param pctx  The context of the packet to be redirected.
     * @param rex   An exception that keeps information about the packet
     *              redirection.
     * @return  A {@code PacketResult} which indicates the result of handler.
     * @throws DropFlowException
     *    The given packet was discarded by a DROP flow filter.
     * @throws RedirectFlowException
     *    The given packet was redirected by a REDIRECT flow filter.
     */
    final PacketResult redirect(VTNManagerImpl mgr, PacketContext pctx,
                                RedirectFlowException rex)
        throws DropFlowException, RedirectFlowException {
        VInterfacePath path = rex.getDestination();

        // Writer lock is required because this method may change the state
        // of the bridge.
        Lock wrlock = writeLock();
        try {
            // Determine the destination of the redirection.
            T vif;
            try {
                vif = getInterfaceImpl(path);
            } catch (VTNException e) {
                String emsg = e.getStatus().getDescription();
                rex.destinationNotFound(mgr, pctx, emsg);
                throw new DropFlowException(e);
            }

            if (!vif.isEnabled()) {
                rex.destinationDisabled(mgr, pctx);
                throw new DropFlowException();
            }

            // Record the packet redirection into the packet context.
            int hops = pctx.redirect((VNodePath)path);
            int hopLimit = mgr.getVTNConfig().getMaxRedirections();
            if (hops > hopLimit) {
                rex.tooManyHops(mgr, pctx, hops);
                throw new DropFlowException();
            }

            if (rex.isOutput()) {
                // Redirect as outgoing packet.
                vif.redirect(mgr, pctx, rex, this);
                return PacketResult.KEEP_PROCESSING;
            }

            // Use the VLAN ID mapped to the virtual interface for
            // packet matching.
            short vid = vif.getVlan();

            // Evaluate flow filters configured in the destination virtual
            // interface for incoming packets.
            vif.filterPacket(mgr, pctx, vid);

            // Forward the packet to the bridge.
            return handlePacket(mgr, pctx, vif);
        } catch (VTNException e) {
            // This should never happen.
            mgr.logException(getLogger(), getNodePath(), e);
        } finally {
            wrlock.unlock();
        }

        return PacketResult.KEEP_PROCESSING;
    }

    /**
     * Invoked when the recalculation of the all shortest path tree is done.
     *
     * @param mgr  VTN manager service.
     */
    final void recalculateDone(VTNManagerImpl mgr) {
        // Writer lock is required because this method may change the state
        // of the bridge.
        Lock wrlock = writeLock();
        try {
            ConcurrentMap<VTenantPath, Object> db = mgr.getStateDB();
            BridgeState bst = getBridgeState(db);
            if (bst.getFaultedPathSize() == 0) {
                return;
            }

            // Remove resolved node paths from the set of faulted node paths.
            // We can use the default route resolver here because it is used
            // to determine only whether a packet is reachable from source to
            // destination.
            List<ObjectPair<Node, Node>> resolved =
                bst.removeResolvedPath(mgr);
            Logger logger = getLogger();
            if (logger.isInfoEnabled()) {
                VNodePath path = getNodePath();
                for (ObjectPair<Node, Node> npath: resolved) {
                    logger.info("{}:{}: Path fault resolved: {} -> {}",
                                getContainerName(), path, npath.getLeft(),
                                npath.getRight());
                }
            }

            if (bst.getFaultedPathSize() == 0) {
                updateState(mgr, db, bst);
            } else {
                setState(mgr, db, bst, VNodeState.DOWN);
            }
        } finally {
            wrlock.unlock();
        }
    }

    /**
     * Return the flow filter instance configured in the specified virtual
     * interface in this bridge.
     *
     * @param lstack  A {@link LockStack} instance to hold acquired locks.
     * @param path    A path to the target virtual interface.
     * @param out     {@code true} means that the outgoing flow filter.
     *                {@code false} means that the incoming flow filter.
     * @param writer  {@code true} means the writer lock is required.
     * @return  A {@link FlowFilterMap} instance.
     * @throws VTNException  An error occurred.
     */
    final FlowFilterMap getFlowFilterMap(LockStack lstack, VInterfacePath path,
                                         boolean out, boolean writer)
        throws VTNException {
        lstack.push(getLock(writer));
        T vif = getInterfaceImpl(path);
        return vif.getFlowFilterMap(out);
    }

    /**
     * Forward an unicast packet to the specified physical network, and
     * install a flow entry.
     *
     * @param mgr       VTN Manager service.
     * @param pctx      The context of the received packet.
     * @param outgoing  A {@link NodeConnector} corresponding to the outgoing
     *                  physical switch port.
     * @param outVlan   A VLAN ID to be set to the outgoing packet.
     * @throws VTNException
     *    An error occurred.
     */
    final void forward(VTNManagerImpl mgr, PacketContext pctx,
                       NodeConnector outgoing, short outVlan)
        throws VTNException {
        RedirectFlowException rex = pctx.getFirstRedirection();
        if (rex != null) {
            // Record the final destination of the packet redirection.
            rex.forwarded(mgr, pctx, outgoing, outVlan);
        }

        // Ensure that the destination host is reachable.
        Logger logger = getLogger();
        NodeConnector incoming = pctx.getIncomingNodeConnector();
        Node snode = incoming.getNode();
        Node dnode = outgoing.getNode();
        Path path;
        if (!snode.equals(dnode)) {
            RouteResolver rr = pctx.getRouteResolver();
            path = rr.getRoute(snode, dnode);
            if (path == null) {
                if (addFaultedPath(mgr, snode, dnode)) {
                    logger.error("{}:{}: Path fault: {} -> {}",
                                 getContainerName(), getNodePath(),
                                 snode, dnode);
                }
                return;
            }
        } else {
            path = null;
        }

        // Forward the packet.
        Ethernet frame = pctx.createFrame(outVlan);
        if (logger.isTraceEnabled()) {
            logger.trace("{}:{}: Forward packet to known host: {}",
                         getContainerName(), getNodePath(),
                         pctx.getDescription(frame, outgoing, outVlan));
        }
        mgr.transmit(outgoing, frame);

        // Install VTN flow.
        installFlow(mgr, pctx, outgoing, outVlan, path);
    }

    /**
     * Install a VTN flow for the received packet.
     *
     * @param mgr       VTN Manager service.
     * @param pctx      The context of the received packet.
     * @param outgoing  A {@link NodeConnector} corresponding to the outgoing
     *                  physical switch port.
     * @param outVlan   A VLAN ID to be set to the outgoing packet.
     * @param path      Path to the destination address of the received packet.
     */
    protected final void installFlow(VTNManagerImpl mgr, PacketContext pctx,
                                     NodeConnector outgoing, short outVlan,
                                     Path path) {
        // Create flow entries.
        VTNFlowDatabase fdb = mgr.getTenantFlowDB(getTenantName());
        if (fdb == null) {
            // This should never happen.
            getLogger().warn("{}:{}: No flow database",
                             getContainerName(), getNodePath());
            return;
        }

        // Purge obsolete flows before installing new flow.
        pctx.purgeObsoleteFlow(mgr, fdb);

        // Prepare to install an unicast flow entry.
        pctx.addUnicastMatchFields();

        NodeConnector incoming = pctx.getIncomingNodeConnector();
        short vlan = pctx.getEtherPacket().getOriginalVlan();
        int pri = pctx.getFlowPriority(mgr);
        VTNFlow vflow = fdb.create(mgr);
        Match match;
        if (path != null) {
            // Create flow entries except for egress flow.
            for (Edge edge: path.getEdges()) {
                match = pctx.createMatch(incoming);
                NodeConnector port = edge.getTailNodeConnector();
                ActionList actions = new ActionList(port.getNode());
                actions.addOutput(port);
                vflow.addFlow(mgr, match, actions, pri);
                incoming = edge.getHeadNodeConnector();
            }
        }

        // Create egress flow entry.
        Node dnode = outgoing.getNode();
        assert incoming.getNode().equals(outgoing.getNode());
        match = pctx.createMatch(incoming);

        // Note that flow action that modifies the VLAN tag has to be set
        // before other actions.
        ActionList actions = new ActionList(dnode);
        actions.addVlanId(outVlan).addAll(pctx.getFilterActions()).
            addOutput(outgoing);
        vflow.addFlow(mgr, match, actions, pri);

        // Fix up the VTN flow.
        pctx.fixUp(vflow);

        // Install flow entries.
        fdb.install(mgr, vflow);
    }

    /**
     * Return the virtual network node in this bridge which maps the specified
     * VLAN network.
     *
     * <p>
     *   Note that this method must be called with holding the bridge lock.
     * </p>
     *
     * @param mgr   VTN Manager service.
     * @param mac   MAC address of the host.
     * @param nc    A {@link NodeConnector} instance corresponding to a
     *              switch port where the host was detected.
     * @param vlan  VLAN ID associated with the specified host..
     * @return  A {@code VirtualMapNode} is returned if the network specified
     *          by {@code nc} and {@code vlan} is mapped to this bridge.
     *          Otherwise {@code null} is returned.
     */
    protected VirtualMapNode match(VTNManagerImpl mgr, long mac,
                                   NodeConnector nc, short vlan) {
        // Check whether the packet is mapped by port mapping or not.
        for (T vif: getInterfaceMap().values()) {
            if (vif.match(mgr, nc, vlan)) {
                return vif;
            }
        }

        return null;
    }

    /**
     * Return the virtual network node in this bridge which maps the specified
     * VLAN network.
     *
     * <p>
     *   Note that this method must be called with holding the bridge lock
     *   in writer mode.
     * </p>
     *
     * @param mgr       VTN Manager service.
     * @param ref       Reference to the virtual network mapping.
     *                  The specified reference must point the virtual node
     *                  contained in this node.
     * @param mac       MAC address of the host.
     * @param nc        A {@link NodeConnector} instance corresponding to a
     *                  switch port where the host was detected.
     * @param vlan      VLAN ID associated with the specified host..
     * @param incoming  Specify {@code true} only if the specified host is
     *                  source address of incoming packet.
     * @return  A {@code VirtualMapNode} is returned if the network specified
     *          by {@code nc} and {@code vlan} is mapped to this bridge.
     *          Otherwise {@code null} is returned.
     */
    protected VirtualMapNode match(VTNManagerImpl mgr, MapReference ref,
                                   long mac, NodeConnector nc, short vlan,
                                   boolean incoming) {
        MapType type = ref.getMapType();
        if (type == MapType.PORT) {
            // Determine the interface specified by the mapping reference.
            VInterfacePath ifPath = (VInterfacePath)ref.getPath();
            String ifName = ifPath.getInterfaceName();
            T vif = getInterfaceMap().get(ifName);
            if (vif == null) {
                // This may happen if the virtual network mapping is changed
                // by another cluster node.
                getLogger().warn("{}:{}: Failed to resolve port mapping " +
                                 "reference: {}", getContainerName(),
                                 getNodePath(), ref);

                return null;
            }

            return (vif.match(mgr, nc, vlan)) ? vif : null;
        }

        // This should never happen.
        getLogger().error("{}:{}: Unexpected mapping reference: {}",
                          getContainerName(), getNodePath(), ref);

        return null;
    }

    /**
     * Notify all virtual interfaces of node changes.
     *
     * <p>
     *   Note that this method must be called with holding the bridge lock.
     * </p>
     *
     * @param mgr   VTN Manager service.
     * @param db    Virtual node state DB.
     * @param bst   Runtime state of the bridge.
     * @param node  Node being updated.
     * @param type  Type of update.
     * @return  New state of this node.
     */
    protected final VNodeState notifyIfNode(
        VTNManagerImpl mgr, ConcurrentMap<VTenantPath, Object> db,
        BridgeState bst, Node node, UpdateType type) {
        if (type == UpdateType.REMOVED) {
            // Remove faulted paths that contain removed node.
            int removed = bst.removeFaultedPath(node);
            if (removed != 0) {
                Logger logger = getLogger();
                logger.info("{}:{}: Remove {} faulted path{} that contains " +
                            "removed node: {}", getContainerName(),
                            getNodePath(), removed, (removed > 1) ? "s" : "",
                            node);
            }
        }

        VNodeState state = (bst.getFaultedPathSize() == 0)
            ? VNodeState.UNKNOWN : VNodeState.DOWN;
        for (T vif: getInterfaceMap().values()) {
            VNodeState s = vif.notifyNode(mgr, db, state, node, type);
            if (vif.isEnabled()) {
                Logger logger = getLogger();
                if (logger.isTraceEnabled()) {
                    logger.trace("{}:{}: notifyNode(if:{}): {} -> {}",
                                 getContainerName(), getNodePath(),
                                 vif.getName(), state, s);
                }
                state = s;
            }
        }

        return state;
    }

    /**
     * Notify all virtual interfaces of node connector changes.
     *
     * <p>
     *   Note that this method must be called with holding the bridge lock.
     * </p>
     *
     * @param mgr     VTN Manager service.
     * @param db      Virtual node state DB.
     * @param bst     Runtime state of the bridge.
     * @param nc      Node connector being updated.
     * @param pstate  The state of the node connector.
     * @param type    Type of update.
     * @return  New state of this node.
     */
    protected final VNodeState notifyIfNodeConnector(
        VTNManagerImpl mgr, ConcurrentMap<VTenantPath, Object> db,
        BridgeState bst, NodeConnector nc, VNodeState pstate,
        UpdateType type) {
        if (type == UpdateType.REMOVED) {
            // Remove path faults that may be caused by removed node
            // connector. Even if removed paths are still broken, they will
            // be detected by succeeding PACKET_IN.
            int removed = bst.removeFaultedPath(nc.getNode());
            if (removed != 0) {
                Logger logger = getLogger();
                logger.info("{}:{}: Remove {} faulted path{} that contains " +
                            "removed node connector: {}", getContainerName(),
                            getNodePath(), removed, (removed > 1) ? "s" : "",
                            nc);
            }
        }

        VNodeState state = (bst.getFaultedPathSize() == 0)
            ? VNodeState.UNKNOWN : VNodeState.DOWN;
        for (T vif: getInterfaceMap().values()) {
            VNodeState s = vif.notifyNodeConnector(mgr, db, state, nc,
                                                   pstate, type);
            if (vif.isEnabled()) {
                Logger logger = getLogger();
                if (logger.isTraceEnabled()) {
                    logger.trace("{}:{}: notifyNodeConnector(if:{}): {} -> {}",
                                 getContainerName(), getNodePath(),
                                 vif.getName(), state, s);
                }
                state = s;
            }
        }

        return state;
    }

    /**
     * Notify all virtual interfaces of edge changes.
     *
     * <p>
     *   Note that this method must be called with holding the bridge lock.
     * </p>
     *
     * @param mgr     VTN Manager service.
     * @param db      Virtual node state DB.
     * @param bst     Runtime state of the bridge.
     * @param estate  A {@link EdgeUpdateState} instance which contains
     *                information reported by the controller.
     * @return  New state of this node.
     */
    protected final VNodeState edgeIfUpdate(
        VTNManagerImpl mgr, ConcurrentMap<VTenantPath, Object> db,
        BridgeState bst, EdgeUpdateState estate) {
        VNodeState state = (bst.getFaultedPathSize() == 0)
            ? VNodeState.UNKNOWN : VNodeState.DOWN;
        for (T vif: getInterfaceMap().values()) {
            VNodeState s = vif.edgeUpdate(mgr, db, state, estate);
            if (vif.isEnabled()) {
                Logger logger = getLogger();
                if (logger.isTraceEnabled()) {
                    logger.trace("{}:{}: edgeUpdate(if:{}): {} -> {}",
                                 getContainerName(), getNodePath(),
                                 vif.getName(), state, s);
                }
                state = s;
            }
        }

        return state;
    }

    /**
     * Evaluate flow filters configured in this bridge against the given
     * outgoing packet.
     *
     * @param mgr   VTN Manager service.
     * @param pctx  The context of the received packet.
     * @param vid   A VLAN ID to be used for packet matching.
     *              A VLAN ID configured in the given packet is used if a
     *              negative value is specified.
     * @return  A {@link PacketContext} to be used for transmitting packet.
     * @throws DropFlowException
     *    The given packet was discarded by a flow filter.
     * @throws RedirectFlowException
     *    The given packet was redirected by a flow filter.
     */
    abstract PacketContext filterOutgoingPacket(VTNManagerImpl mgr,
                                                PacketContext pctx, short vid)
        throws DropFlowException, RedirectFlowException;

    /**
     * Handle the received packet.
     *
     * <p>
     *   Note that this method must be called with holding the bridge lock.
     * </p>
     *
     * @param mgr    VTN Manager service.
     * @param pctx   The context of the received packet.
     * @param vnode  A {@link VirtualMapNode} instance that maps the received
     *               packet.
     * @return  A {@code PacketResult} which indicates the result of handler.
     * @throws DropFlowException
     *    The given packet was discarded by a flow filter.
     * @throws RedirectFlowException
     *    The given packet was redirected by a flow filter.
     * @throws VTNException
     *    An error occurred.
     */
    protected abstract PacketResult handlePacket(VTNManagerImpl mgr,
                                                 PacketContext pctx,
                                                 VirtualMapNode vnode)
        throws DropFlowException, RedirectFlowException, VTNException;
}
