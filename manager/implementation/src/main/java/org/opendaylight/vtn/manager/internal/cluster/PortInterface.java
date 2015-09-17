/*
 * Copyright (c) 2014, 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
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
import org.opendaylight.vtn.manager.VTNException;
import org.opendaylight.vtn.manager.VTenantPath;
import org.opendaylight.vtn.manager.util.EtherAddress;

import org.opendaylight.vtn.manager.internal.IVTNResourceManager;
import org.opendaylight.vtn.manager.internal.VTNManagerProvider;
import org.opendaylight.vtn.manager.internal.PacketContext;
import org.opendaylight.vtn.manager.internal.TxContext;
import org.opendaylight.vtn.manager.internal.VTNManagerImpl;
import org.opendaylight.vtn.manager.internal.VTNConfig;
import org.opendaylight.vtn.manager.internal.VTNThreadData;
import org.opendaylight.vtn.manager.internal.inventory.VtnNodeEvent;
import org.opendaylight.vtn.manager.internal.inventory.VtnPortEvent;
import org.opendaylight.vtn.manager.internal.util.ProtocolUtils;
import org.opendaylight.vtn.manager.internal.util.flow.match.VTNEtherMatch;
import org.opendaylight.vtn.manager.internal.util.flow.match.VTNMatch;
import org.opendaylight.vtn.manager.internal.util.inventory.InventoryReader;
import org.opendaylight.vtn.manager.internal.util.inventory.InventoryUtils;
import org.opendaylight.vtn.manager.internal.util.inventory.NodeUtils;
import org.opendaylight.vtn.manager.internal.util.inventory.SalNode;
import org.opendaylight.vtn.manager.internal.util.inventory.SalPort;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;
import org.opendaylight.vtn.manager.internal.util.flow.VTNFlowBuilder;

import org.opendaylight.controller.sal.core.Node;
import org.opendaylight.controller.sal.core.NodeConnector;
import org.opendaylight.controller.sal.core.UpdateType;
import org.opendaylight.controller.sal.packet.Ethernet;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.VirtualRouteReason;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.node.info.VtnPort;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeState;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnErrorTag;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;

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
    private static final long serialVersionUID = 3346949601083788579L;

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
        inFlowFilters = FlowFilterMap.createIncoming(this);
        outFlowFilters = FlowFilterMap.createOutgoing(this);
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
     * @param ctx     MD-SAL datastore transaction context.
     * @param pmconf  Port mapping configuration to be set.
     *                If {@code null} is specified, port mapping on the
     *                specified interface is destroyed.
     * @return  New state of this virtual interface.
     *          {@code null} is returned if the port mapping configuration
     *          was not changed.
     * @throws VTNException  An error occurred.
     */
    final VnodeState setPortMap(VTNManagerImpl mgr, TxContext ctx,
                                PortMapConfig pmconf)
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
        ProtocolUtils.checkVlan(vlan);

        SwitchPort port = pmconf.getPort();
        NodeUtils.checkSwitchPort(port, node);

        NodeConnector mapped = ist.getMappedPort();
        IVTNResourceManager resMgr = mgr.getResourceManager();
        PortVlan pvlan = null;

        // Search for the switch port specified by the configuration.
        InventoryReader reader = ctx.getInventoryReader();
        NodeConnector nc;
        SalPort sport = reader.findPort(node, port);
        if (sport != null) {
            nc = sport.getAdNodeConnector();
            if (nc.equals(mapped) && oldconf.getVlan() == vlan) {
                // No need to change switch port mapping.
                portMapConfig = pmconf;
                return ist.getState();
            }

            pvlan = new PortVlan(nc, vlan);
        } else {
            nc = null;
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
            VtnErrorTag etag = e.getVtnErrorTag();
            if (etag == VtnErrorTag.INTERNALERROR) {
                mgr.logException(getLogger(), (VNodePath)path, e, pvlan,
                                 rmlan);
            }
            throw e;
        }

        if (ref != null) {
            assert ref.getMapType() == MapType.PORT;

            throw RpcException.getDataExistsException(
                "Specified port is mapped to " + ref.getAbsolutePath());
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

        VnodeState state;
        if (isEnabled()) {
            state = getNewState(reader, ist);
        } else {
            state = VnodeState.DOWN;
            VnodeState pstate = getPortState(reader, sport);
            ist.setPortState(pstate);
        }
        ist.setState(state);
        db.put((VNodePath)path, ist);
        if (ist.isDirty()) {
            VnodeState pstate = ist.getPortState();
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
     * @param ev      A {@link VtnNodeEvent} instance.
     * @return  New state of the parent node.
     */
    final VnodeState notifyNode(VTNManagerImpl mgr,
                                ConcurrentMap<VTenantPath, Object> db,
                                VnodeState prstate, VtnNodeEvent ev) {
        VnodeState state = notifyNode(mgr, db, ev);
        return getParentState(state, prstate);
    }

    /**
     * This method is called when some properties of a node connector are
     * added/deleted/changed.
     *
     * @param mgr      VTN Manager service.
     * @param db       Virtual node state DB.
     * @param prstate  Current state of the parent node.
     * @param ev       {@link VtnPortEvent} instance.
     * @return  New state of this virtual node.
     */
    final VnodeState notifyNodeConnector(VTNManagerImpl mgr,
                                         ConcurrentMap<VTenantPath, Object> db,
                                         VnodeState prstate, VtnPortEvent ev) {
        VnodeState state = notifyNodeConnector(mgr, db, ev);
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
     * @throws RedirectFlowException
     *    The given packet was redirected by a flow filter.
     */
    final void transmit(VTNManagerImpl mgr, PacketContext pctx,
                        Set<PortVlan> sent) throws RedirectFlowException {
        PortMapConfig pmconf = portMapConfig;
        if (pmconf == null) {
            return;
        }

        VInterfaceState ist = getIfState(mgr);
        VnodeState state = ist.getState();
        if (state != VnodeState.UP) {
            return;
        }

        NodeConnector mapped = ist.getMappedPort();
        if (mapped == null) {
            return;
        }
        SalPort egress = SalPort.create(mapped);
        if (egress == null) {
            // This should never happen.
            return;
        }

        short vlan = pmconf.getVlan();
        PortVlan pvlan = new PortVlan(mapped, vlan);
        if (!sent.add(pvlan)) {
            // Already sent.
            return;
        }

        Logger logger = getLogger();
        PacketContext pc;
        Ethernet frame;
        try {
            // Apply outgoing flow filters.
            pc = outFlowFilters.evaluate(mgr, pctx, vlan);

            // Create a new Ethernet frame to be transmitted.
            frame = pc.createFrame(vlan);
        } catch (DropFlowException e) {
            // Filtered out by DROP filter.
            return;
        } catch (Exception e) {
            mgr.logException(logger, getPath(), e);
            return;
        }

        if (logger.isTraceEnabled()) {
            VInterfacePath path = getInterfacePath();
            logger.trace("{}:{}: Transmit packet to {} interface: {}",
                         getContainerName(), path, path.getNodeType(),
                         pc.getDescription(frame, egress, vlan));
        }

        mgr.transmit(egress, frame);
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
     * Redirect the packet to this virtual interface as outgoing packet.
     *
     * @param mgr     VTN Manager service.
     * @param pctx    The context of the packet to be redirected.
     * @param rex     An exception that keeps information about the packet
     *                redirection.
     * @param bridge  A {@link PortBridge} instance associated with this
     *                virtual interface.
     * @throws DropFlowException
     *    The given packet was discarded by a DROP flow filter.
     * @throws RedirectFlowException
     *    The given packet was redirected by a REDIRECT flow filter.
     * @throws VTNException  An error occurred.
     */
    protected void redirect(VTNManagerImpl mgr, PacketContext pctx,
                            RedirectFlowException rex, PortBridge<?> bridge)
        throws DropFlowException, RedirectFlowException, VTNException {
        // Ensure that a physical switch port is mapped by port mapping.
        PortMapConfig pmconf = portMapConfig;
        NodeConnector mapped = getMappedPort(mgr);
        if (pmconf == null || mapped == null) {
            rex.notMapped(pctx);
            throw new DropFlowException();
        }

        // Evaluate flow filters for outgoing packets.
        // Note that this should never clone a PacketContext.
        short vlan = pmconf.getVlan();
        outFlowFilters.evaluate(mgr, pctx, vlan);

        // Forward the packet.
        bridge.forward(mgr, pctx, mapped, vlan);
    }

    /**
     * Return a VLAN ID mapped to this virtual interface.
     *
     * @return  A VLAN ID mapped to this interface.
     *          A negative value is returned if port mapping is not configured.
     */
    public short getVlan() {
        PortMapConfig pmconf = portMapConfig;
        return (pmconf == null) ? -1 : pmconf.getVlan();
    }

    /**
     * Derive new node state from the specified node connector.
     *
     * <p>
     *   Note that this method may update the state of the mapped switch port
     *   in {@code ist}.
     * </p>
     *
     * @param rdr  An {@link InventoryReader} instance.
     * @param ist  Runtime state of this node.
     * @return  New node state.
     * @throws VTNException  An error occurred.
     */
    private VnodeState getNewState(InventoryReader rdr, VInterfaceState ist)
        throws VTNException {
        NodeConnector nc = ist.getMappedPort();
        if (nc == null) {
            return VnodeState.DOWN;
        }

        SalPort sport = SalPort.create(nc);
        if (sport == null) {
            // This should never happen.
            return VnodeState.DOWN;
        }

        VtnPort vport = rdr.get(sport);
        return getNewState(ist, vport);
    }

    /**
     * Derive new node state from the specified node connector.
     *
     * <p>
     *   Note that this method always updates the state of the mapped switch
     *   port in {@code ist}.
     * </p>
     *
     * @param ist     Runtime state of this node.
     * @param vport   A {@link VtnPort} instance associated with the switch
     *                port mapped to this node.
     * @return  New node state.
     */
    private VnodeState getNewState(VInterfaceState ist, VtnPort vport) {
        // Update the state of the mapped port.
        VnodeState pstate = getPortState(vport);
        ist.setPortState(pstate);

        if (pstate == VnodeState.UP && InventoryUtils.isEdge(vport)) {
            return VnodeState.UP;
        }

        return VnodeState.DOWN;
    }

    /**
     * Get state of the switch port.
     *
     * @param reader  A {@link InventoryReader} instance.
     * @param sport   A {@link SalPort} instance.
     * @return  A {@link VnodeState} instance which represents the state of
     *          the specified switc port.
     * @throws VTNException  An error occurred.
     */
    private VnodeState getPortState(InventoryReader reader, SalPort sport)
        throws VTNException {
        if (sport == null) {
            return VnodeState.UNKNOWN;
        }

        VtnPort vport = reader.get(sport);
        return getPortState(vport);
    }

    /**
     * Get state of the switch port.
     *
     * @param vport   A {@link VtnPort} instance.
     * @return  A {@link VnodeState} instance which represents the state of
     *          the specified switc port.
     */
    private VnodeState getPortState(VtnPort vport) {
        if (vport == null) {
            return VnodeState.UNKNOWN;
        }

        return (InventoryUtils.isEnabled(vport))
            ? VnodeState.UP : VnodeState.DOWN;
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
    private VnodeState unsetPortMap(VTNManagerImpl mgr,
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
        setState(mgr, db, ist, VnodeState.UNKNOWN);

        return VnodeState.UNKNOWN;
    }

    /**
     * Determine whether the node connector satisfies the condition specified
     * by the port map configuration.
     *
     * @param pmconf  Port mapping configuration.
     * @param sport   A {@link SalPort} instance.
     * @param vport   A {@link VtnPort} instance.
     * @return  {@code true} is returned only if the given node connector
     *          satisfies the condition.
     */
    private boolean portMatch(PortMapConfig pmconf, SalPort sport,
                              VtnPort vport) {
        Node pnode = pmconf.getNode();
        SwitchPort port = pmconf.getPort();
        String type = port.getType();
        String id = port.getId();
        NodeConnector nc = sport.getAdNodeConnector();
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
        return (name == null || name.equals(vport.getName()));
    }

    /**
     * Establish port mapping.
     *
     * @param mgr       VTN Manager service.
     * @param db        Virtual node state DB.
     * @param ist       Runtime state of this node.
     * @param sport     A {@link SalPort} to be mapped.
     * @param resuming
     *     {@code true} if this method is called by
     *     {@link #resuming(VTNManagerImpl, ConcurrentMap, TxContext, VInterfaceState)}.
     * @return  {@code true} is returned if the given port is successfully
     *          mapped. {@code false} is returned if a switch port is already
     *          mapped to this node.
     */
    private boolean mapPort(VTNManagerImpl mgr,
                            ConcurrentMap<VTenantPath, Object> db,
                            VInterfaceState ist, SalPort sport,
                            boolean resuming) {
        NodeConnector nc = sport.getAdNodeConnector();
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
     * Invoked when a node is added, removed, or changed.
     *
     * @param mgr  VTN Manager service.
     * @param db   Virtual node state DB.
     * @param ev   A {@link VtnNodeEvent} instance.
     * @return  New state of this interface.
     */
    private VnodeState notifyNode(VTNManagerImpl mgr,
                                  ConcurrentMap<VTenantPath, Object> db,
                                  VtnNodeEvent ev) {
        VInterfaceState ist = getIfState(db);
        VnodeState cur = ist.getState();
        if (ev.getUpdateType() != VtnUpdateType.REMOVED) {
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

        SalNode snode = ev.getSalNode();
        Node pnode = pmconf.getNode();
        if (pnode.equals(snode.getAdNode())) {
            // No need to flush MAC address table entries and flow entries.
            // It will be done by the VTN Manager service.
            unmapPort(mgr, db, ist, true, false);
            cur = VnodeState.DOWN;
            setState(mgr, db, ist, cur);
        }

        return cur;
    }

    /**
     * This method is called when some properties of a node connector are
     * added/deleted/changed.
     *
     * @param mgr    VTN Manager service.
     * @param db     Virtual node state DB.
     * @param ev       {@link VtnPortEvent} instance.
     * @return  New state of this virtual interface.
     */
    private VnodeState notifyNodeConnector(
        VTNManagerImpl mgr, ConcurrentMap<VTenantPath, Object> db,
        VtnPortEvent ev) {
        VInterfaceState ist = getIfState(db);
        VnodeState cur = ist.getState();
        PortMapConfig pmconf = portMapConfig;
        if (pmconf == null) {
            return cur;
        }

        NodeConnector mapped = ist.getMappedPort();
        VnodeState state = VnodeState.UNKNOWN;
        SalPort sport = ev.getSalPort();
        VtnPort vport = ev.getVtnPort();
        switch (ev.getUpdateType()) {
        case CREATED:
            if (mapped != null || !portMatch(pmconf, sport, vport)) {
                return cur;
            }

            // Try to establish port mapping.
            if (!mapPort(mgr, db, ist, sport, false)) {
                return cur;
            }
            state = getNewState(ist, vport);
            break;

        case CHANGED:
            if (mapped == null) {
                // Check whether the port name was changed.
                // Map the port if its name matches the configuration.
                if (pmconf.getPort().getName() == null ||
                    !portMatch(pmconf, sport, vport) ||
                    !mapPort(mgr, db, ist, sport, false)) {
                    return cur;
                }

                state = getNewState(ist, vport);
            } else if (sport.getAdNodeConnector().equals(mapped)) {
                if (pmconf.getPort().getName() != null &&
                    !portMatch(pmconf, sport, vport)) {
                    // This port should be unmapped because its name has been
                    // changed. In this case flow and MAC address table entries
                    // need to be purged.
                    unmapPort(mgr, db, ist, true, true);
                    state = VnodeState.DOWN;
                } else {
                    state = getNewState(ist, vport);
                }
            } else {
                return cur;
            }
            break;

        case REMOVED:
            if (sport.getAdNodeConnector().equals(mapped)) {
                // No need to flush MAC address table entries and flow entries.
                // It will be done by the VTN Manager service.
                unmapPort(mgr, db, ist, true, false);
                state = VnodeState.DOWN;
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
     * Construct a flow match which specifies the given VLAN.
     *
     * @param vid  A VLAN ID.
     * @return  A {@link VTNMatch} instance on success.
     *          {@code null} on failure.
     */
    private VTNMatch createMatch(int vid) {
        try {
            VTNEtherMatch ematch =
                new VTNEtherMatch(null, null, null, vid, null);
            return new VTNMatch(ematch, null, null);
        } catch (RpcException e) {
            // This should never happen.
            getLogger().error("Failed to create VLAN match: " + vid, e);
            return null;
        }
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
     * Return path to the virtual mapping which maps the given host.
     *
     * @param mac   Unused.
     * @param vlan  Unused.
     * @return  Path to this interface.
     */
    @Override
    public VNodePath getPath(long mac, short vlan) {
        return getPath();
    }

    /**
     * Return a {@link VNodeRoute} instance which indicates the packet was
     * mapped by the port mapping.
     *
     * @param mac   Unused.
     * @param vlan  Unused.
     * @return  A {@link VNodeRoute} instance.
     */
    @Override
    public final VNodeRoute getIngressRoute(long mac, short vlan) {
        return new VNodeRoute((VNodePath)getInterfacePath(),
                              VirtualRouteReason.PORTMAPPED);
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
        SalPort mapped = SalPort.create(ist.getMappedPort());
        if (mapped == null) {
            return;
        }

        VTNMatch vmatch = createMatch((int)pmconf.getVlan());
        if (vmatch == null) {
            return;
        }

        VTNManagerProvider provider = pctx.getTxContext().getProvider();
        VTNConfig vcfg = provider.getVTNConfig();
        EtherAddress mac = vcfg.getControllerMacAddress();
        int pri = vcfg.getL2FlowPriority();
        int idle = pctx.getIdleTimeout();
        int hard = pctx.getHardTimeout();
        String tname = getTenantName();

        VTNFlowBuilder builder =
            new VTNFlowBuilder(tname, mac, vmatch, pri, idle, hard);
        builder.addVirtualRoute(getIngressRoute(MacVlan.UNDEFINED, (short)0)).
            setEgressVNodeRoute(null).
            addDropFlow(mapped);
        provider.addFlow(builder);
    }

    /**
     * Evaluate flow filters for incoming packet configured in this virtual
     * interface.
     *
     * @param mgr   VTN Manager service.
     * @param pctx  The context of the received packet.
     * @param vid   A VLAN ID to be used for packet matching.
     *              A VLAN ID configured in the given packet is used if a
     *              negative value is specified.
     * @throws DropFlowException
     *    The given packet was discarded by a flow filter.
     * @throws RedirectFlowException
     *    The given packet was redirected by a flow filter.
     */
    @Override
    public final void filterPacket(VTNManagerImpl mgr, PacketContext pctx,
                                   short vid)
        throws DropFlowException, RedirectFlowException {
        // Note that this call always returns `pctx'.
        inFlowFilters.evaluate(mgr, pctx, vid);
    }

    /**
     * Evaluate flow filters for outgoing packet configured in this virtual
     * interface.
     *
     * @param mgr     VTN Manager service.
     * @param pctx    The context of the received packet.
     * @param vid     A VLAN ID to be used for packet matching.
     *                A VLAN ID configured in the given packet is used if a
     *                negative value is specified.
     * @param bridge  Never used.
     * @return  A {@link PacketContext} to be used for transmitting packet.
     * @throws DropFlowException
     *    The given packet was discarded by a flow filter.
     * @throws RedirectFlowException
     *    The given packet was redirected by a flow filter.
     */
    @Override
    public final PacketContext filterPacket(VTNManagerImpl mgr,
                                            PacketContext pctx, short vid,
                                            PortBridge<?> bridge)
        throws DropFlowException, RedirectFlowException {
        return outFlowFilters.evaluate(mgr, pctx, vid);
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
     * @param ctx      MD-SAL datastore transaction context.
     * @param db       Virtual node state DB.
     * @param ist      Current runtime state of this interface.
     * @param enabled  {@code true} is passed if this interface has been
     *                 enabled.
     *                 {@code false} is passed if this interface has been
     *                 disabled.
     * @return  New state of this interface.
     */
    @Override
    protected final VnodeState changeEnableState(
        VTNManagerImpl mgr, TxContext ctx,
        ConcurrentMap<VTenantPath, Object> db, VInterfaceState ist,
        boolean enabled) {
        VnodeState state;
        if (enabled) {
            // Determine new state by the state of mapped port.
            if (portMapConfig == null) {
                state = VnodeState.UNKNOWN;
            } else {
                InventoryReader reader = ctx.getInventoryReader();
                try {
                    state = getNewState(reader, ist);
                } catch (Exception e) {
                    mgr.logException(getLogger(), getPath(), e);
                    state = VnodeState.UNKNOWN;
                }
            }
        } else {
            // State of disabled interface must be DOWN.
            state = VnodeState.DOWN;
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
     * @param ctx  A runtime context for MD-SAL datastore transaction task.
     * @param ist  Current state of this interface.
     * @return  New state of this interface.
     */
    @Override
    protected final VnodeState resuming(VTNManagerImpl mgr,
                                        ConcurrentMap<VTenantPath, Object> db,
                                        TxContext ctx, VInterfaceState ist) {
        PortMapConfig pmconf = portMapConfig;
        if (pmconf == null) {
            return VnodeState.UNKNOWN;
        }

        NodeConnector mapped = ist.getMappedPort();
        if (mapped == null) {
            // Try to establish port mapping.
            Node node = pmconf.getNode();
            InventoryReader reader = ctx.getInventoryReader();
            SwitchPort port = pmconf.getPort();
            try {
                SalPort sport = reader.findPort(node, port);
                if (sport != null) {
                    mapPort(mgr, db, ist, sport, true);
                }

                return getNewState(reader, ist);
            } catch (Exception e) {
                mgr.logException(getLogger(), getPath(), e);
            }
        }

        return VnodeState.UNKNOWN;
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
            VTNThreadData.removeFlows(mgr, getTenantName());
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
