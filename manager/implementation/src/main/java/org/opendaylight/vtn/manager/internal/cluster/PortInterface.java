/*
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.cluster;

import java.util.Set;
import java.util.concurrent.ConcurrentMap;

import org.slf4j.Logger;

import org.opendaylight.vtn.manager.IVTNManagerAware;
import org.opendaylight.vtn.manager.PortMap;
import org.opendaylight.vtn.manager.PortMapConfig;
import org.opendaylight.vtn.manager.SwitchPort;
import org.opendaylight.vtn.manager.VInterface;
import org.opendaylight.vtn.manager.VInterfaceConfig;
import org.opendaylight.vtn.manager.VInterfacePath;
import org.opendaylight.vtn.manager.VNodePath;
import org.opendaylight.vtn.manager.VNodeRoute;
import org.opendaylight.vtn.manager.VNodeState;
import org.opendaylight.vtn.manager.VTNException;
import org.opendaylight.vtn.manager.VTenantPath;
import org.opendaylight.vtn.manager.internal.EdgeUpdateState;
import org.opendaylight.vtn.manager.internal.IVTNResourceManager;
import org.opendaylight.vtn.manager.internal.MiscUtils;
import org.opendaylight.vtn.manager.internal.NodeUtils;
import org.opendaylight.vtn.manager.internal.PacketContext;
import org.opendaylight.vtn.manager.internal.VTNFlowDatabase;
import org.opendaylight.vtn.manager.internal.VTNManagerImpl;
import org.opendaylight.vtn.manager.internal.VTNThreadData;

import org.opendaylight.controller.sal.core.Node;
import org.opendaylight.controller.sal.core.NodeConnector;
import org.opendaylight.controller.sal.core.UpdateType;
import org.opendaylight.controller.sal.match.Match;
import org.opendaylight.controller.sal.match.MatchType;
import org.opendaylight.controller.sal.packet.Ethernet;
import org.opendaylight.controller.sal.utils.Status;
import org.opendaylight.controller.sal.utils.StatusCode;

/**
 * Implementation of virtual interface that can have port mapping
 * configuration.
 *
 * <p>
 *   Although this class is public to other packages, this class does not
 *   provide any API. Applications other than VTN Manager must not use this
 *   class.
 * </p>
 */
public abstract class PortInterface extends AbstractInterface
    implements VirtualMapNode, FlowFilterNode {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = 8583202785488078241L;

    /**
     * Port mapping configuration.
     */
    private PortMapConfig  portMapConfig;

    /**
     * Flow filters for incoming packets.
     */
    private final FlowFilterMap  inFlowFilters;

    /**
     * Flow filters for outgoing packets.
     */
    private final FlowFilterMap  outFlowFilters;

    /**
     * Construct a virtual node instance that can have port mapping.
     *
     * @param parent  The parent node that contains this node.
     * @param name    The name of this node.
     * @param iconf   Configuration for the interface.
     */
    protected PortInterface(AbstractBridge parent, String name,
                            VInterfaceConfig iconf) {
        super(parent, name, iconf);
        inFlowFilters = new FlowFilterMap(this, false);
        outFlowFilters = new FlowFilterMap(this, true);
    }

    /**
     * Return the port mapping information.
     *
     * <p>
     *   This method assumes that the call of this method is synchronized
     *   by the parent node.
     * </p>
     *
     * @param mgr  VTN Manager service.
     * @return  Port mapping information. {@code null} is returned if the
     *          port mapping is not configured on this node.
     */
    final PortMap getPortMap(VTNManagerImpl mgr) {
        PortMapConfig pmconf = portMapConfig;
        if (pmconf == null) {
            return null;
        }

        return new PortMap(pmconf, getMappedPort(mgr));
    }

    /**
     * Return a node connector associated with the switch port mapped to
     * this node.
     *
     * <p>
     *   This method assumes that the call of this method is synchronized
     *   by the parent node.
     * </p>
     *
     * @param mgr  VTN Manager service.
     * @return  A node connector. {@code null} is returned if no swtich port
     *          is mapped.
     */
    final NodeConnector getMappedPort(VTNManagerImpl mgr) {
        VInterfaceState ist = getIfState(mgr);
        return ist.getMappedPort();
    }

    /**
     * Create or destroy mapping between the physical switch port and this
     * virtual node.
     *
     * <p>
     *   This method assumes that the call of this method is synchronized
     *   by the parent node.
     * </p>
     *
     * @param mgr     VTN Manager service.
     * @param pmconf  Port mapping configuration to be set.
     *                If {@code null} is specified, port mapping on the
     *                specified interface is destroyed.
     * @return  New state of this virtual interface.
     *          {@code null} is returned if the port mapping configuration
     *          was not changed.
     * @throws VTNException  An error occurred.
     */
    final VNodeState setPortMap(VTNManagerImpl mgr, PortMapConfig pmconf)
        throws VTNException {
        ConcurrentMap<VTenantPath, Object> db = mgr.getStateDB();
        VInterfaceState ist = getIfState(db);

        if (pmconf == null) {
            // Destroy port mapping.
            return unsetPortMap(mgr, db, ist);
        }

        PortMapConfig oldconf = portMapConfig;
        if (pmconf.equals(oldconf)) {
            // Nothing to do.
            return null;
        }

        Node node = pmconf.getNode();
        NodeUtils.checkNode(node);

        short vlan = pmconf.getVlan();
        MiscUtils.checkVlan(vlan);

        SwitchPort port = pmconf.getPort();
        NodeUtils.checkSwitchPort(port, node);

        NodeConnector mapped = ist.getMappedPort();
        IVTNResourceManager resMgr = mgr.getResourceManager();
        PortVlan pvlan = null;

        // Search for the node connector specified by the configuration.
        NodeConnector nc = NodeUtils.findPort(mgr, node, port);
        if (nc != null) {
            if (nc.equals(mapped) && oldconf.getVlan() == vlan) {
                // No need to change switch port mapping.
                portMapConfig = pmconf;
                return ist.getState();
            }

            pvlan = new PortVlan(nc, vlan);
        }

        PortVlan rmlan = (mapped == null)
            ? null
            : new PortVlan(mapped, oldconf.getVlan());

        // Register port mapping, and unregister old port mapping if needed.
        VInterfacePath path = getInterfacePath();
        MapReference ref;
        try {
            ref = resMgr.registerPortMap(mgr, path, pvlan, rmlan, true);
        } catch (VTNException e) {
            Status status = e.getStatus();
            if (status == null ||
                status.getCode().equals(StatusCode.INTERNALERROR)) {
                mgr.logException(getLogger(), (VNodePath)path, e, pvlan,
                                 rmlan);
            }
            throw e;
        }

        if (ref != null) {
            assert ref.getMapType() == MapType.PORT;

            throw new VTNException(StatusCode.CONFLICT,
                                   "Specified port is mapped to " +
                                   ref.getAbsolutePath());
        }

        // Update the state of this node.
        portMapConfig = pmconf;
        ist.setMappedPort(nc);
        PortMap pmap = new PortMap(pmconf, nc);
        if (oldconf == null) {
            PortMapEvent.added(mgr, path, pmap);
        } else {
            PortMapEvent.changed(mgr, path, pmap, true);
        }

        VNodeState state;
        if (isEnabled()) {
            state = getNewState(mgr, ist);
        } else {
            state = VNodeState.DOWN;
            VNodeState pstate = getPortState(mgr, nc);
            ist.setPortState(pstate);
        }
        ist.setState(state);
        db.put((VNodePath)path, ist);
        if (ist.isDirty()) {
            VNodeState pstate = ist.getPortState();
            VInterface viface = new VInterface(getName(), state, pstate,
                                               getVInterfaceConfig());
            VInterfaceEvent.changed(mgr, path, viface, false);
        }

        return state;
    }

    /**
     * Invoked when a node is added, removed, or changed.
     *
     * @param mgr     VTN Manager service.
     * @param db      Virtual node state DB.
     * @param prstate Current state of the parent node.
     * @param node    Node being updated.
     * @param type    Type of update.
     * @return  New state of the parent node.
     */
    final VNodeState notifyNode(VTNManagerImpl mgr,
                                ConcurrentMap<VTenantPath, Object> db,
                                VNodeState prstate, Node node,
                                UpdateType type) {
        VNodeState state = notifyNode(mgr, db, node, type);
        return getParentState(state, prstate);
    }

    /**
     * This method is called when some properties of a node connector are
     * added/deleted/changed.
     *
     * @param mgr      VTN Manager service.
     * @param db       Virtual node state DB.
     * @param prstate  Current state of the parent node.
     * @param nc       Node connector being updated.
     * @param pstate   The state of the node connector.
     * @param type     Type of update.
     * @return  New state of this virtual node.
     */
    final VNodeState notifyNodeConnector(VTNManagerImpl mgr,
                                         ConcurrentMap<VTenantPath, Object> db,
                                         VNodeState prstate, NodeConnector nc,
                                         VNodeState pstate, UpdateType type) {
        VNodeState state = notifyNodeConnector(mgr, db, nc, pstate, type);
        return getParentState(state, prstate);
    }

    /**
     * This method is called when topology graph is changed.
     *
     * @param mgr      VTN Manager service.
     * @param db       Virtual node state DB.
     * @param prstate  Current state of the parent node.
     * @param estate   A {@link EdgeUpdateState} instance which contains
     *                 information reported by the controller.
     * @return  New state of this virtual node.
     */
    final VNodeState edgeUpdate(VTNManagerImpl mgr,
                                ConcurrentMap<VTenantPath, Object> db,
                                VNodeState prstate, EdgeUpdateState estate) {
        VNodeState state = edgeUpdate(mgr, db, estate);
        return getParentState(state, prstate);
    }

    /**
     * Determine whether the received packet should be treated as input of
     * this virtual node or not.
     *
     * <p>
     *   Note that this method does not see the state of this node.
     *   This method returns {@code true} if the network specified by the
     *   switch port and the VLAN ID is mapped to this node.
     * </p>
     *
     * @param mgr   VTN Manager service.
     * @param nc    A node connector where the packet was received.
     * @param vlan  VLAN ID in the received packet.
     * @return  {@code true} is returned only if the received packet should be
     *          treated as input of this node.
     */
    final boolean match(VTNManagerImpl mgr, NodeConnector nc, short vlan) {
        PortMapConfig pmconf = portMapConfig;
        if (pmconf == null) {
            return false;
        }

        VInterfaceState ist = getIfState(mgr);
        NodeConnector mapped = ist.getMappedPort();
        return (nc.equals(mapped) && pmconf.getVlan() == vlan);
    }

    /**
     * Transmit the specified packet to this node.
     *
     * @param mgr   VTN manager service.
     * @param pctx  The context of the packet.
     * @param sent  A set of {@link PortVlan} which indicates the network
     *              already processed.
     */
    final void transmit(VTNManagerImpl mgr, PacketContext pctx,
                        Set<PortVlan> sent) {
        PortMapConfig pmconf = portMapConfig;
        if (pmconf == null) {
            return;
        }

        VInterfaceState ist = getIfState(mgr);
        VNodeState state = ist.getState();
        if (state != VNodeState.UP) {
            return;
        }

        NodeConnector mapped = ist.getMappedPort();
        if (mapped == null) {
            return;
        }

        short vlan = pmconf.getVlan();
        PortVlan pvlan = new PortVlan(mapped, vlan);
        if (!sent.add(pvlan)) {
            // Already sent.
            return;
        }

        Ethernet frame = pctx.createFrame(vlan);
        Logger logger = getLogger();
        if (logger.isTraceEnabled()) {
            VInterfacePath path = getInterfacePath();
            logger.trace("{}:{}: Transmit packet to {} interface: {}",
                         getContainerName(), path, path.getNodeType(),
                         pctx.getDescription(frame, mapped, vlan));
        }

        mgr.transmit(mapped, frame);
    }

    /**
     * Return the flow filter instance configured in this virtual interface.
     *
     * <p>
     *   This method must be called with holding the node lock.
     * </p>
     *
     * @param out  {@code true} means that the outgoing flow filter.
     *             {@code false} means that the incoming flow filter.
     * @return  A {@link FlowFilterMap} instance.
     */
    final FlowFilterMap getFlowFilterMap(boolean out) {
        return (out) ? outFlowFilters : inFlowFilters;
    }

    /**
     * Derive new node state from the specified node connector.
     *
     * <p>
     *   Note that this method may update the state of the mapped switch port
     *   in {@code ist}.
     * </p>
     *
     * @param mgr  VTN Manager service.
     * @param ist  Runtime state of this node.
     * @return  New node state.
     */
    private VNodeState getNewState(VTNManagerImpl mgr, VInterfaceState ist) {
        NodeConnector nc = ist.getMappedPort();
        if (nc == null) {
            return VNodeState.DOWN;
        }

        VNodeState pstate = getPortState(mgr, nc);
        return getNewState(mgr, ist, nc, pstate);
    }

    /**
     * Derive new node state from the specified node connector.
     *
     * <p>
     *   Note that this method always updates the state of the mapped switch
     *   port in {@code ist}.
     * </p>
     *
     * @param mgr     VTN Manager service.
     * @param ist     Runtime state of this node.
     * @param nc      Node connector associated with the switch port mapped to
     *                this node.
     * @param pstate  State of the switch port mapped to this node.
     * @return  New node state.
     */
    private VNodeState getNewState(VTNManagerImpl mgr, VInterfaceState ist,
                                   NodeConnector nc, VNodeState pstate) {
        // Update the state of the mapped port.
        ist.setPortState(pstate);

        if (pstate == VNodeState.UP && mgr.isEdgePort(nc)) {
            return VNodeState.UP;
        }

        return VNodeState.DOWN;
    }

    /**
     * Get state of the switch port.
     *
     * @param mgr  VTN Manager service.
     * @param nc   Node connector mapped to this node.
     * @return  New virtual node state.
     */
    private VNodeState getPortState(VTNManagerImpl mgr, NodeConnector nc) {
        if (nc == null) {
            return VNodeState.UNKNOWN;
        }

        return (mgr.isEnabled(nc)) ? VNodeState.UP : VNodeState.DOWN;
    }

    /**
     * Destroy mapping between the physical switch port and this node.
     *
     * @param mgr   VTN Manager service.
     * @param db    Virtual node state DB.
     * @param ist   Runtime state of this node.
     * @return  New state of this virtual node.
     *          {@code null} is returned if the port mapping configuration
     *          was not changed.
     */
    private VNodeState unsetPortMap(VTNManagerImpl mgr,
                                    ConcurrentMap<VTenantPath, Object> db,
                                    VInterfaceState ist) {
        PortMapConfig pmconf = portMapConfig;
        if (pmconf == null) {
            return null;
        }

        NodeConnector mapped = ist.getMappedPort();
        if (mapped != null) {
            unmapPort(mgr, db, ist, false, true);
        }

        portMapConfig = null;
        PortMap pmap = new PortMap(pmconf, mapped);
        VInterfacePath path = getInterfacePath();
        PortMapEvent.removed(mgr, path, pmap, true);
        setState(mgr, db, ist, VNodeState.UNKNOWN);

        return VNodeState.UNKNOWN;
    }

    /**
     * Determine whether the node connector satisfies the condition specified
     * by the port map configuration.
     *
     * @param mgr     VTN Manager service.
     * @param pmconf  Port mapping configuration.
     * @param nc      Node connector to be tested.
     * @return  {@code true} is returned only if the given node connector
     *          satisfies the condition.
     */
    private boolean portMatch(VTNManagerImpl mgr, PortMapConfig pmconf,
                              NodeConnector nc) {
        PortProperty pp = mgr.getPortProperty(nc);
        if (pp == null) {
            // Non-physical port cannot be mapped.
            return false;
        }

        Node pnode = pmconf.getNode();
        SwitchPort port = pmconf.getPort();
        String type = port.getType();
        String id = port.getId();
        if (type != null && id != null) {
            // This should never return null.
            NodeConnector target =
                NodeConnector.fromStringNoNode(type, id, pnode);
            if (!nc.equals(target)) {
                return false;
            }
        } else if (!pnode.equals(nc.getNode())) {
            return false;
        }

        String name = port.getName();
        return (name == null || name.equals(pp.getName()));
    }

    /**
     * Establish port mapping.
     *
     * @param mgr       VTN Manager service.
     * @param db        Virtual node state DB.
     * @param ist       Runtime state of this node.
     * @param nc        Node connector to be mapped.
     * @param resuming
     *     {@code true} if this method is called by
     *     {@link #resuming(VTNManagerImpl, ConcurrentMap, VInterfaceState)}.
     * @return  {@code true} is returned if the given port is successfully
     *          mapped. {@code false} is returned if a switch port is already
     *          mapped to this node.
     */
    private boolean mapPort(VTNManagerImpl mgr,
                            ConcurrentMap<VTenantPath, Object> db,
                            VInterfaceState ist, NodeConnector nc,
                            boolean resuming) {
        short vlan = portMapConfig.getVlan();
        PortVlan pvlan = new PortVlan(nc, vlan);
        IVTNResourceManager resMgr = mgr.getResourceManager();
        MapReference ref;
        VInterfacePath path = getInterfacePath();

        try {
            ref = resMgr.registerPortMap(mgr, path, pvlan, null, !resuming);
        } catch (VTNException e) {
            Logger logger = getLogger();
            logger.error(getContainerName() + ":" + path +
                         ": Failed to map port to virtual interface: " +
                         pvlan, e);
            return false;
        }

        if (ref == null) {
            ist.setMappedPort(nc);
            db.put((VNodePath)path, ist);

            // Clear dirty flag.
            ist.isDirty();

            if (!resuming) {
                PortMap pmap = new PortMap(portMapConfig, nc);
                PortMapEvent.changed(mgr, path, pmap, false);
            }
            return true;
        }

        Logger logger = getLogger();
        logger.error("{}:{}: Switch port is already mapped to {}: {}",
                     getContainerName(), path, ref.getAbsolutePath(), pvlan);
        return false;
    }

    /**
     * Unmap the physical switch port currently mapped.
     *
     * @param mgr    VTN Manager service.
     * @param db     Virtual node state DB.
     * @param ist    Runtime state of this node.
     * @param event  If {@code true} is specified, a port map event is
     *               notified.
     * @param purge  If {@code true} is specified, network caches originated
     *               by the port mapping will be purged.
     */
    private void unmapPort(VTNManagerImpl mgr,
                           ConcurrentMap<VTenantPath, Object> db,
                           VInterfaceState ist,
                           boolean event, boolean purge) {
        // Unregister port mapping.
        short vlan = portMapConfig.getVlan();
        PortVlan pvlan = new PortVlan(ist.getMappedPort(), vlan);
        VInterfacePath path = getInterfacePath();
        IVTNResourceManager resMgr = mgr.getResourceManager();
        try {
            resMgr.unregisterPortMap(mgr, path, pvlan, purge);
        } catch (VTNException e) {
            Logger logger = getLogger();
            logger.error(getContainerName() + ":" + path +
                         ": Failed to unmap port: " + pvlan, e);
            // FALLTHROUGH
        }

        ist.setMappedPort(null);
        db.put((VNodePath)path, ist);

        // Clear dirty flag.
        ist.isDirty();

        if (event) {
            PortMap pmap = new PortMap(portMapConfig, null);
            PortMapEvent.changed(mgr, path, pmap, false);
        }
    }

    /**
     * Flush cached network data associated with the network specified by
     * a pair of switch port and VLAN ID.
     *
     * <p>
     *   This method is used to flush network data cached by obsolete port
     *   mapping.
     * </p>
     *
     * @param mgr    VTN Manager service.
     * @param port   A node connector associated with a switch port.
     * @param vlan   A VLAN ID.
     */
    protected void flushCache(VTNManagerImpl mgr, NodeConnector port,
                              short vlan) {
        VNodePath npath = getPath();
        if (port != null) {
            // Remove flow entries relevant to obsolete port mapping.
            VTNThreadData.removeFlows(mgr, npath.getTenantName(), port, vlan);
        }

        // Remove flow entries affected by this virtual node.
        VTNThreadData.removeFlows(mgr, npath);
    }

    /**
     * Invoked when a node is added, removed, or changed.
     *
     * @param mgr   VTN Manager service.
     * @param db    Virtual node state DB.
     * @param node  Node being updated.
     * @param type  Type of update.
     * @return  New state of this interface.
     */
    private VNodeState notifyNode(VTNManagerImpl mgr,
                                  ConcurrentMap<VTenantPath, Object> db,
                                  Node node, UpdateType type) {
        VInterfaceState ist = getIfState(db);
        VNodeState cur = ist.getState();
        if (type != UpdateType.REMOVED) {
            return cur;
        }

        PortMapConfig pmconf = portMapConfig;
        if (pmconf == null) {
            return cur;
        }

        NodeConnector mapped = ist.getMappedPort();
        if (mapped == null) {
            return cur;
        }

        Node pnode = pmconf.getNode();
        if (pnode.equals(node)) {
            // No need to flush MAC address table entries and flow entries.
            // It will be done by the VTN Manager service.
            unmapPort(mgr, db, ist, true, false);
            cur = VNodeState.DOWN;
            setState(mgr, db, ist, cur);
        }

        return cur;
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
     * @return  New state of this virtual interface.
     */
    private VNodeState notifyNodeConnector(
        VTNManagerImpl mgr, ConcurrentMap<VTenantPath, Object> db,
        NodeConnector nc, VNodeState pstate, UpdateType type) {

        VInterfaceState ist = getIfState(db);
        VNodeState cur = ist.getState();
        PortMapConfig pmconf = portMapConfig;
        if (pmconf == null) {
            return cur;
        }

        VInterfacePath path = getInterfacePath();
        NodeConnector mapped = ist.getMappedPort();
        VNodeState state = VNodeState.UNKNOWN;
        switch (type) {
        case ADDED:
            if (mapped != null || !portMatch(mgr, pmconf, nc)) {
                return cur;
            }

            // Try to establish port mapping.
            if (!mapPort(mgr, db, ist, nc, false)) {
                return cur;
            }
            state = getNewState(mgr, ist, nc, pstate);
            break;

        case CHANGED:
            if (mapped == null) {
                // Check whether the port name was changed.
                // Map the port if its name matches the configuration.
                if (pmconf.getPort().getName() == null ||
                    !portMatch(mgr, pmconf, nc) ||
                    !mapPort(mgr, db, ist, nc, false)) {
                    return cur;
                }

                state = getNewState(mgr, ist, nc, pstate);
            } else if (nc.equals(mapped)) {
                if (pmconf.getPort().getName() != null &&
                    !portMatch(mgr, pmconf, nc)) {
                    // This port should be unmapped because its name has been
                    // changed. In this case flow and MAC address table entries
                    // need to be purged.
                    unmapPort(mgr, db, ist, true, true);
                    state = VNodeState.DOWN;
                } else {
                    state = getNewState(mgr, ist, nc, pstate);
                }
            } else {
                return cur;
            }
            break;

        case REMOVED:
            if (nc.equals(mapped)) {
                // No need to flush MAC address table entries and flow entries.
                // It will be done by the VTN Manager service.
                unmapPort(mgr, db, ist, true, false);
                state = VNodeState.DOWN;
            } else {
                return cur;
            }
            break;

        default:
            return cur;
        }

        setState(mgr, db, ist, state);

        return state;
    }

    /**
     * This method is called when topology graph is changed.
     *
     * @param mgr     VTN Manager service.
     * @param db      Virtual node state DB.
     * @param estate  A {@link EdgeUpdateState} instance which contains
     *                information reported by the controller.
     * @return  New state of this virtual interface.
     */
    private VNodeState edgeUpdate(VTNManagerImpl mgr,
                                  ConcurrentMap<VTenantPath, Object> db,
                                  EdgeUpdateState estate) {
        VInterfaceState ist = getIfState(db);
        VNodeState cur = ist.getState();
        PortMapConfig pmconf = portMapConfig;
        if (pmconf != null) {
            NodeConnector mapped = ist.getMappedPort();
            Boolean changed = estate.getPortState(mapped);
            if (changed != null) {
                assert mapped != null;
                VNodeState pstate = getPortState(mgr, mapped);
                ist.setPortState(pstate);

                VNodeState state =
                    (pstate == VNodeState.UP && !changed.booleanValue())
                    ? VNodeState.UP : VNodeState.DOWN;
                setState(mgr, db, ist, state);
                cur = state;
            }
        }

        return cur;
    }

    /**
     * Determine whether the given object is identical to this object.
     *
     * @param o  An object to be compared.
     * @return   {@code true} if identical. Otherwise {@code false}.
     */
    @Override
    public final boolean equals(Object o) {
        if (o == this) {
            return true;
        }
        if (!super.equals(o)) {
            return false;
        }

        PortInterface pi = (PortInterface)o;
        if (portMapConfig == null) {
            return (pi.portMapConfig == null);
        }

        return portMapConfig.equals(pi.portMapConfig);
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public final int hashCode() {
        int h = super.hashCode();
        if (portMapConfig != null) {
            h += (portMapConfig.hashCode() * 13);
        }

        return h;
    }

    // VirtualMapNode

    /**
     * Return path to this node.
     *
     * @return  Path to this node.
     */
    @Override
    public VNodePath getPath() {
        return (VNodePath)getInterfacePath();
    }

    /**
     * Return a {@link VNodeRoute} instance which indicates the packet was
     * mapped by the port mapping.
     *
     * @return  A {@link VNodeRoute} instance.
     */
    @Override
    public final VNodeRoute getIngressRoute() {
        return new VNodeRoute((VNodePath)getInterfacePath(),
                              VNodeRoute.Reason.PORTMAPPED);
    }

    /**
     * Install a flow entry which drops every incoming packet.
     *
     * <p>
     *   This method must be called with holding the node lock.
     * </p>
     *
     * @param mgr   VTN Manager service.
     * @param pctx  The context of the received packet.
     */
    @Override
    public final void disableInput(VTNManagerImpl mgr, PacketContext pctx) {
        PortMapConfig pmconf = portMapConfig;
        if (pmconf == null) {
            return;
        }

        VInterfaceState ist = getIfState(mgr);
        NodeConnector mapped = ist.getMappedPort();
        if (mapped == null) {
            return;
        }

        VTNFlowDatabase fdb = mgr.getTenantFlowDB(getTenantName());
        if (fdb == null) {
            // This should never happen.
            Logger logger = getLogger();
            logger.warn("{}:{}: No flow database", getContainerName(),
                        getInterfacePath());
            return;
        }

        VTNFlow vflow = fdb.create(mgr);
        Match match = new Match();
        match.setField(MatchType.IN_PORT, mapped);

        // This code expects MatchType.DL_VLAN_NONE is zero.
        match.setField(MatchType.DL_VLAN, pmconf.getVlan());

        int pri = mgr.getVTNConfig().getL2FlowPriority();
        vflow.addFlow(mgr, match, pri);
        vflow.addVirtualRoute(getIngressRoute());
        vflow.setEgressVNodePath(null);
        vflow.setTimeout(pctx.getIdleTimeout(), pctx.getHardTimeout());
        fdb.install(mgr, vflow);
    }

    // AbstractInterface

    /**
     * Initialize virtual interface path for this instance.
     *
     * @param parent  The parent node that contains this interface.
     * @param name    The name of this interface.
     */
    @Override
    void setPath(AbstractBridge parent, String name) {
        super.setPath(parent, name);

        // Initialize parent path for flow filter.
        inFlowFilters.setParent(this);
        outFlowFilters.setParent(this);
    }

    /**
     * Change enable/disable configuration of this interface.
     *
     * <p>
     *   This method assumes that the call of this method is synchronized
     *   by the parent node.
     * </p>
     *
     * @param mgr      VTN Manager service.
     * @param db       Virtual node state DB.
     * @param ist      Current runtime state of this interface.
     * @param enabled  {@code true} is passed if this interface has been
     *                 enabled.
     *                 {@code false} is passed if this interface has been
     *                 disabled.
     * @return  New state of this interface.
     */
    @Override
    protected final VNodeState changeEnableState(
        VTNManagerImpl mgr, ConcurrentMap<VTenantPath, Object> db,
        VInterfaceState ist, boolean enabled) {
        VNodeState state;
        if (enabled) {
            // Determine new state by the state of mapped port.
            state = (portMapConfig == null)
                ? VNodeState.UNKNOWN
                : getNewState(mgr, ist);
        } else {
            // State of disabled interface must be DOWN.
            state = VNodeState.DOWN;
        }

        ist.setState(state);
        if (ist.isDirty()) {
            PortMapConfig pmconf = portMapConfig;
            if (pmconf != null) {
                // Invalidate all cached data added by the port mapping.
                NodeConnector mapped = ist.getMappedPort();
                short vlan = pmconf.getVlan();
                flushCache(mgr, mapped, vlan);
            }
            db.put((VNodePath)getInterfacePath(), ist);
        }

        return state;
    }

    /**
     * Invoked when this interface is going to be resuming from the
     * configuration file.
     *
     * @param mgr  VTN Manager service.
     * @param db   Virtual node state DB.
     * @param ist  Current state of this interface.
     * @return  New state of this interface.
     */
    @Override
    protected final VNodeState resuming(VTNManagerImpl mgr,
                                        ConcurrentMap<VTenantPath, Object> db,
                                        VInterfaceState ist) {
        PortMapConfig pmconf = portMapConfig;
        if (pmconf == null) {
            return VNodeState.UNKNOWN;
        }

        NodeConnector mapped = ist.getMappedPort();
        if (mapped == null) {
            // Try to establish port mapping.
            Node node = pmconf.getNode();
            SwitchPort port = pmconf.getPort();
            NodeConnector nc = NodeUtils.findPort(mgr, node, port);
            if (nc != null) {
                mapPort(mgr, db, ist, nc, true);
            }
        }

        return getNewState(mgr, ist);
    }

    /**
     * Invoked when this interface is going to be destroyed.
     *
     * @param mgr     VTN Manager service.
     * @param db      Virtual node state DB.
     * @param ist     Current state of this interface.
     * @param retain  {@code true} means that the parent node will be retained.
     *                {@code false} means that the parent node is being
     *                destroyed.
     */
    protected final void destroying(VTNManagerImpl mgr,
                                    ConcurrentMap<VTenantPath, Object> db,
                                    VInterfaceState ist, boolean retain) {
        if (retain && !(inFlowFilters.isEmpty() && outFlowFilters.isEmpty())) {
            // REVISIT: Select flow entries affected by obsolete flow filters.
            VTNFlowDatabase fdb = mgr.getTenantFlowDB(getTenantName());
            VTNThreadData.removeFlows(mgr, fdb);
        }

        PortMapConfig pmconf = portMapConfig;
        if (pmconf != null) {
            // Destroy port mapping.
            NodeConnector mapped = ist.getMappedPort();
            PortMap pmap = new PortMap(pmconf, mapped);
            if (mapped != null) {
                unmapPort(mgr, db, ist, false, retain);
            }
            PortMapEvent.removed(mgr, getInterfacePath(), pmap, false);
        }
    }

    /**
     * Notify the listener of current configuration.
     *
     * @param mgr       VTN Manager service.
     * @param listener  VTN manager listener service.
     */
    @Override
    final void notifyConfiguration(VTNManagerImpl mgr,
                                   IVTNManagerAware listener) {
        super.notifyConfiguration(mgr, listener);

        PortMapConfig pmconf = portMapConfig;
        if (pmconf != null) {
            VInterfaceState ist = getIfState(mgr);
            NodeConnector mapped = ist.getMappedPort();
            PortMap pmap = new PortMap(pmconf, mapped);
            mgr.notifyChange(listener, getInterfacePath(), pmap,
                             UpdateType.ADDED);
        }
    }
}
