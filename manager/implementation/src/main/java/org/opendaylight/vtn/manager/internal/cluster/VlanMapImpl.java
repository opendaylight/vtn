/*
 * Copyright (c) 2013, 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.cluster;

import java.util.Set;
import java.util.HashSet;
import java.util.concurrent.ConcurrentMap;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import org.opendaylight.vtn.manager.VBridgePath;
import org.opendaylight.vtn.manager.VNodeRoute;
import org.opendaylight.vtn.manager.VTNException;
import org.opendaylight.vtn.manager.VTenantPath;
import org.opendaylight.vtn.manager.VlanMap;
import org.opendaylight.vtn.manager.VlanMapConfig;
import org.opendaylight.vtn.manager.packet.Ethernet;

import org.opendaylight.vtn.manager.internal.IVTNResourceManager;
import org.opendaylight.vtn.manager.internal.PacketContext;
import org.opendaylight.vtn.manager.internal.TxContext;
import org.opendaylight.vtn.manager.internal.VTNManagerImpl;
import org.opendaylight.vtn.manager.internal.VlanMapPortFilter;
import org.opendaylight.vtn.manager.internal.inventory.VtnNodeEvent;
import org.opendaylight.vtn.manager.internal.inventory.VtnPortEvent;
import org.opendaylight.vtn.manager.internal.util.inventory.InventoryReader;
import org.opendaylight.vtn.manager.internal.util.inventory.InventoryUtils;
import org.opendaylight.vtn.manager.internal.util.inventory.SalNode;
import org.opendaylight.vtn.manager.internal.util.inventory.SalPort;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.node.info.VtnPort;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.nodes.VtnNode;

import org.opendaylight.controller.sal.core.Node;
import org.opendaylight.controller.sal.core.NodeConnector;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.VirtualRouteReason;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeState;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnErrorTag;

/**
 * Implementation of VLAN mapping to the virtual L2 bridge.
 *
 * <p>
 *   This class keeps VLAN mapping configuration and availability of the Node
 *   associated with the VLAN mapping.
 * </p>
 * <p>
 *   Although this class is public to other packages, this class does not
 *   provide any API. Applications other than VTN Manager must not use this
 *   class.
 * </p>
 */
public final class VlanMapImpl implements VBridgeNode {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = -8379366464925651620L;

    /**
     * Logger instance.
     */
    private static final Logger  LOG =
        LoggerFactory.getLogger(VlanMapImpl.class);

    /**
     * Configuration for the VLAN mapping.
     */
    private final VlanMapConfig  vlanMapConfig;

    /**
     * Path to the VLAN mapping.
     */
    private transient VlanMapPath  mapPath;

    /**
     * Construct a VLAN mapping instance.
     *
     * @param bpath  Path to the virtual L2 bridge which includes this mapping.
     * @param mapId  Identifier of the VLAN mapping.
     * @param vlconf  VLAN mapping configuration.
     */
    VlanMapImpl(VBridgePath bpath, String mapId, VlanMapConfig vlconf) {
        vlanMapConfig = vlconf;
        setPath(bpath, mapId);
    }

    /**
     * Return the identifier of this VLAN mapping.
     *
     * @return  The identifier of this VLAN mapping.
     */
    String getMapId() {
        return mapPath.getMapId();
    }

    /**
     * Set VLAN mapping path.
     *
     * @param bpath  Path to the virtual L2 bridge which includes this mapping.
     * @param mapId  Identifier of the VLAN mapping.
     */
    void setPath(VBridgePath bpath, String mapId) {
        mapPath = new VlanMapPath(bpath, mapId);
    }

    /**
     * Return VLAN mapping configuration.
     *
     * @return  VLAN mapping configuration.
     */
    VlanMapConfig getVlanMapConfig() {
        return vlanMapConfig;
    }

    /**
     * Determine whether this VLAN mapping is valid or not.
     *
     * @param db  Virtual node state DB.
     * @return {@code true} is returned only if this VLAN mapping is valid.
     */
    boolean isValid(ConcurrentMap<VTenantPath, Object> db) {
        Boolean b = (Boolean)db.get(mapPath);
        return (b == null) ? false : b.booleanValue();
    }

    /**
     * Derive L2 bridge state from current VLAN mapping state.
     *
     * @param db     Virtual node state DB.
     * @param bstate Current bridge state value.
     * @return  New bridge state value.
     */
    VnodeState getBridgeState(ConcurrentMap<VTenantPath, Object> db,
                              VnodeState bstate) {
        boolean valid = isValid(db);
        return getBridgeState(valid, bstate);
    }

    /**
     * Resume VLAN mapping.
     *
     * <p>
     *   This method is called just after this bridge is instantiated from
     *   the configuration file.
     * </p>
     *
     * @param mgr     VTN Manager service.
     * @param ctx    A runtime context for MD-SAL datastore transaction task.
     * @param bstate  Current bridge state value.
     * @return  New bridge state value.
     */
    VnodeState resume(VTNManagerImpl mgr, TxContext ctx, VnodeState bstate) {
        // Register this VLAN mapping to the global resource manager.
        IVTNResourceManager resMgr = mgr.getResourceManager();
        short vlan = vlanMapConfig.getVlan();
        Node node = vlanMapConfig.getNode();
        NodeVlan nvlan = new NodeVlan(node, vlan);

        try {
            resMgr.registerVlanMap(mgr, mapPath, nvlan, false);
        } catch (Exception e) {
            LOG.error(mgr.getContainerName() + ":" + mapPath +
                      ": Failed to resume VLAN mapping: " + nvlan, e);
            // FALLTHROUGH
        }

        boolean valid;
        if (node == null) {
            valid = true;
        } else {
            try {
                valid = checkNode(ctx, node);
            } catch (Exception e) {
                LOG.error(mgr.getContainerName() + ":" + mapPath +
                          ": Failed to determine state of VLAN mapping: " +
                          nvlan, e);
                valid = false;
            }
        }

        if (valid) {
            ConcurrentMap<VTenantPath, Object> db = mgr.getStateDB();
            db.put(mapPath, Boolean.valueOf(valid));
        }

        return getBridgeState(valid, bstate);
    }

    /**
     * Invoked when a node has been added or removed.
     *
     * @param mgr     VTN Manager service.
     * @param db      Virtual node state DB.
     * @param bstate  Current bridge state value.
     * @param ev      A {@link VtnNodeEvent} instance.
     * @return  New bridge state value.
     */
    VnodeState notifyNode(VTNManagerImpl mgr,
                          ConcurrentMap<VTenantPath, Object> db,
                          VnodeState bstate, VtnNodeEvent ev) {
        boolean cur = isValid(db);
        Node myNode = vlanMapConfig.getNode();
        SalNode snode = ev.getSalNode();
        Node node = snode.getAdNode();
        if (myNode == null || !myNode.equals(node)) {
            return getBridgeState(cur, bstate);
        }

        boolean valid;
        switch (ev.getUpdateType()) {
        case CREATED:
            valid = InventoryUtils.hasEdgePort(ev.getVtnNode());
            break;

        case REMOVED:
            valid = false;
            break;

        default:
            return getBridgeState(cur, bstate);
        }

        setValid(db, cur, valid);
        return getBridgeState(valid, bstate);
    }

    /**
     * This method is called when some properties of a node connector are
     * added/deleted/changed.
     *
     * @param mgr     VTN Manager service.
     * @param db      Virtual node state DB.
     * @param bstate  Current bridge state value.
     * @param ev      A {@link VtnPortEvent} instance.
     * @return  New bridge state value.
     * @throws VTNException  An error occurred.
     */
    VnodeState notifyNodeConnector(VTNManagerImpl mgr,
                                   ConcurrentMap<VTenantPath, Object> db,
                                   VnodeState bstate, VtnPortEvent ev)
        throws VTNException {
        boolean cur = isValid(db);
        SalPort sport = ev.getSalPort();
        SalNode snode = sport.getSalNode();
        Node node = snode.getAdNode();
        Node myNode = vlanMapConfig.getNode();
        if (myNode == null || !myNode.equals(node)) {
            return getBridgeState(cur, bstate);
        }

        // VLAN mapping can work if at least one physical switch port is up.
        VtnPort vport = ev.getVtnPort();
        boolean valid = InventoryUtils.isEnabledEdge(vport);
        if (!valid) {
            InventoryReader reader = ev.getTxContext().getInventoryReader();
            VtnNode vnode = reader.get(snode);
            valid = InventoryUtils.hasEdgePort(vnode);
        }

        setValid(db, cur, valid);
        return getBridgeState(valid, bstate);
    }

    /**
     * Transmit the specified packet to the network established by this
     * VLAN mapping.
     *
     * @param mgr    VTN manager service.
     * @param pctx   The context of the packet.
     * @param vbr    A {@link VBridgeImpl} instance which contains this
     *               VLAN mapping.
     * @param sent   A set of {@link PortVlan} which indicates the network
     *               already processed.
     * @throws RedirectFlowException
     *    The given packet was redirected by a flow filter.
     */
    void transmit(VTNManagerImpl mgr, PacketContext pctx, VBridgeImpl vbr,
                  Set<PortVlan> sent) throws RedirectFlowException {
        // Determine edge ports of this VLAN mapping.
        IVTNResourceManager resMgr = mgr.getResourceManager();
        Node node = vlanMapConfig.getNode();
        short vlan = vlanMapConfig.getVlan();
        HashSet<NodeConnector> ports = new HashSet<NodeConnector>();
        VlanMapPortFilter filter =
            VlanMapPortFilter.create(resMgr, node, vlan, sent);
        InventoryReader reader = pctx.getTxContext().getInventoryReader();
        try {
            reader.collectUpEdgePorts(ports, filter);
        } catch (Exception e) {
            mgr.logException(LOG, mapPath, e);
            return;
        }

        if (ports.isEmpty()) {
            LOG.trace("{}:{}: transmit: No port is available",
                      mgr.getContainerName(), mapPath);
            return;
        }

        PacketContext pc;
        Ethernet frame;
        try {
            // Apply outgoing flow filters.
            pc = vbr.filterOutgoingPacket(mgr, pctx, vlan);

            // Create a new Ethernet frame to be transmitted.
            frame = pc.createFrame(vlan);
        } catch (DropFlowException e) {
            // Filtered out by DROP filter.
            return;
        } catch (Exception e) {
            mgr.logException(LOG, mapPath, e);
            return;
        }

        for (NodeConnector nc: ports) {
            PortVlan pvlan = new PortVlan(nc, vlan);
            if (!sent.add(pvlan)) {
                continue;
            }

            if (LOG.isTraceEnabled()) {
                SalPort sport = SalPort.create(nc);
                LOG.trace("{}:{}: Transmit packet to VLAN mapping: {}",
                          mgr.getContainerName(), mapPath,
                          pc.getDescription(frame, sport, vlan));
            }
            mgr.transmit(nc, frame);
        }
    }

    /**
     * Determine whether the given node is suitable for the VLAN mapping or
     * not.
     *
     * @param ctx    A runtime context for MD-SAL datastore transaction task.
     * @param node  Node to be tested.
     * @return  {@code true} is returned only if the given node is valid.
     * @throws VTNException  An error occurred.
     */
    private boolean checkNode(TxContext ctx, Node node)
        throws VTNException {
        SalNode snode = SalNode.create(node);
        InventoryReader reader = ctx.getInventoryReader();
        VtnNode vnode = reader.get(snode);
        return InventoryUtils.hasEdgePort(vnode);
    }

    /**
     * Derive L2 bridge state from current VLAN mapping state.
     *
     * @param valid   Current valid flag for the VLAN mapping.
     * @param bstate  Current bridge state value.
     * @return  New bridge state value.
     */
    private VnodeState getBridgeState(boolean valid, VnodeState bstate) {
        if (!valid) {
            return VnodeState.DOWN;
        }

        if (bstate == VnodeState.UNKNOWN) {
            return VnodeState.UP;
        }

        return bstate;
    }

    /**
     * Set valid flag for the VLAN mapping.
     *
     * @param db     Virtual node state DB.
     * @param cur    Current valid flag for the VLAN mapping.
     * @param valid  {@code true} if the VLAN mapping is valid.
     */
    private void setValid(ConcurrentMap<VTenantPath, Object> db, boolean cur,
                          boolean valid) {
        boolean changed = (cur != valid);
        if (changed) {
            db.put(mapPath, Boolean.valueOf(valid));
        }
    }

    /**
     * Register a new VLAN mapping to the resource manager.
     *
     * @param mgr    VTN manager service.
     * @param ctx    MD-SAL datastore transaction context.
     * @param bpath  Path to the parent bridge.
     * @param nvlan  A {@link NodeVlan} instance which contains a pair of
     *               {@link Node} instance and VLAN ID.
     * @throws VTNException  An error occurred.
     */
    void register(VTNManagerImpl mgr, TxContext ctx, VBridgePath bpath,
                  NodeVlan nvlan)
        throws VTNException {
        IVTNResourceManager resMgr = mgr.getResourceManager();
        MapReference ref;
        try {
            ref = resMgr.registerVlanMap(mgr, mapPath, nvlan, true);
        } catch (VTNException e) {
            if (e.getVtnErrorTag() == VtnErrorTag.INTERNALERROR) {
                mgr.logException(LOG, mapPath, e, nvlan);
            }
            throw e;
        }

        if (ref != null) {
            assert ref.getMapType() == MapType.VLAN;

            String msg;
            if (ref.isContained(mgr.getContainerName(), bpath)) {
                msg = "Already mapped to this bridge";
            } else {
                StringBuilder builder = new StringBuilder("VLAN(");
                nvlan.appendContents(builder);
                builder.append(") is mapped to ").
                    append(ref.getAbsolutePath());
                msg = builder.toString();
            }
            throw RpcException.getDataExistsException(msg);
        }

        // Initialize the state of the VLAN mapping.
        Node node = vlanMapConfig.getNode();
        boolean valid = (node == null) ? true : checkNode(ctx, node);
        ConcurrentMap<VTenantPath, Object> db = mgr.getStateDB();
        db.put(mapPath, Boolean.valueOf(valid));
    }

    /**
     * Destroy the VLAN mapping.
     *
     * @param mgr     VTN manager service.
     * @param bpath   Path to the parent bridge.
     * @param retain  {@code true} means that the parent bridge will be
     *                retained. {@code false} means that the parent bridge
     *                is being destroyed.
     * @throws VTNException  An error occurred.
     */
    void destroy(VTNManagerImpl mgr, VBridgePath bpath, boolean retain)
        throws VTNException {
        // Unregister VLAN mapping from the resource manager.
        short vlan = vlanMapConfig.getVlan();
        Node node = vlanMapConfig.getNode();
        NodeVlan nvlan = new NodeVlan(node, vlan);
        IVTNResourceManager resMgr = mgr.getResourceManager();
        try {
            resMgr.unregisterVlanMap(mgr, mapPath, nvlan, retain);
        } catch (VTNException e) {
            if (retain) {
                if (e.getVtnErrorTag() == VtnErrorTag.INTERNALERROR) {
                    mgr.logException(LOG, bpath, e, nvlan);
                }
                throw e;
            } else {
                LOG.error(mgr.getContainerName() + ":" + bpath +
                          ": Failed to unregister VLAN mapping: " + nvlan, e);
                // FALLTHROUGH
            }
        }

        // Remove VLAN mapping state.
        ConcurrentMap<VTenantPath, Object> db = mgr.getStateDB();
        db.remove(mapPath);

        // Generate a VLAN mapping event.
        VlanMap vlmap = new VlanMap(getMapId(), node, vlan);
        VlanMapEvent.removed(mgr, bpath, vlmap, retain);
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
        if (!(o instanceof VlanMapImpl)) {
            return false;
        }

        VlanMapImpl vmap = (VlanMapImpl)o;
        return vlanMapConfig.equals(vmap.vlanMapConfig);
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        return vlanMapConfig.hashCode();
    }

    // VBridgeNode

    /**
     * Return path to this VLAN mapping.
     *
     * @return  Path to this VLAN mapping.
     */
    @Override
    public VlanMapPath getPath() {
        return mapPath;
    }

    /**
     * Return path to the virtual mapping which maps the given host.
     *
     * @param mac   Unused.
     * @param vlan  Unused.
     * @return  Path to this VLAN mapping.
     */
    @Override
    public VlanMapPath getPath(long mac, short vlan) {
        return mapPath;
    }

    /**
     * Determine whether this VLAN mapping is administravely enabled or not.
     *
     * @return {@code true} is always returned because the VLAN mapping can not
     *         be disabled.
     */
    @Override
    public boolean isEnabled() {
        return true;
    }

    /**
     * Return a {@link VNodeRoute} instance which indicates the packet was
     * mapped by the VLAN mapping.
     *
     * @param mac   Unused.
     * @param vlan  Unused.
     * @return  A {@link VNodeRoute} instance.
     */
    @Override
    public VNodeRoute getIngressRoute(long mac, short vlan) {
        return new VNodeRoute(mapPath, VirtualRouteReason.VLANMAPPED);
    }

    /**
     * Install a flow entry which drops every incoming packet.
     *
     * @param mgr   VTN Manager service.
     * @param pctx  The context of the received packet.
     */
    @Override
    public void disableInput(VTNManagerImpl mgr, PacketContext pctx) {
        // Not supported.
    }

    /**
     * Evaluate flow filters for incoming packet mapped by this VLAN mapping.
     *
     * @param mgr   Never used.
     * @param pctx  Never used.
     * @param vid   Never used.
     */
    @Override
    public void filterPacket(VTNManagerImpl mgr, PacketContext pctx,
                             short vid) {
        // Nothing to do.
    }

    /**
     * Evaluate flow filters for outgoing packet to be transmitted by this
     * VLAN mapping.
     *
     * @param mgr     VTN Manager service.
     * @param pctx    The context of the received packet.
     * @param vid     A VLAN ID to be used for packet matching.
     *                A VLAN ID configured in the given packet is used if a
     *                negative value is specified.
     * @param bridge  A {@link PortBridge} instance associated with this
     *                virtual mapping.
     * @return  A {@link PacketContext} to be used for transmitting packet.
     * @throws DropFlowException
     *    The given packet was discarded by a flow filter.
     * @throws RedirectFlowException
     *    The given packet was redirected by a flow filter.
     */
    @Override
    public PacketContext filterPacket(VTNManagerImpl mgr, PacketContext pctx,
                                      short vid, PortBridge<?> bridge)
        throws DropFlowException, RedirectFlowException {
        return bridge.filterOutgoingPacket(mgr, pctx, vid);
    }
}
