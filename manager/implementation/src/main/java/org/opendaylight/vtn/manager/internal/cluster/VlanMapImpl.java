/*
 * Copyright (c) 2013-2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.cluster;

import java.util.Set;
import java.util.HashSet;
import java.util.concurrent.ConcurrentMap;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import org.opendaylight.vtn.manager.VBridgePath;
import org.opendaylight.vtn.manager.VNodeRoute;
import org.opendaylight.vtn.manager.VNodeState;
import org.opendaylight.vtn.manager.VTNException;
import org.opendaylight.vtn.manager.VTenantPath;
import org.opendaylight.vtn.manager.VlanMap;
import org.opendaylight.vtn.manager.VlanMapConfig;
import org.opendaylight.vtn.manager.internal.EdgeUpdateState;
import org.opendaylight.vtn.manager.internal.IVTNResourceManager;
import org.opendaylight.vtn.manager.internal.PacketContext;
import org.opendaylight.vtn.manager.internal.VTNManagerImpl;
import org.opendaylight.vtn.manager.internal.VlanMapPortFilter;

import org.opendaylight.controller.sal.core.Node;
import org.opendaylight.controller.sal.core.NodeConnector;
import org.opendaylight.controller.sal.core.UpdateType;
import org.opendaylight.controller.sal.packet.Ethernet;
import org.opendaylight.controller.sal.utils.Status;
import org.opendaylight.controller.sal.utils.StatusCode;

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
    private static final long serialVersionUID = -102685496368865791L;

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
    VNodeState getBridgeState(ConcurrentMap<VTenantPath, Object> db,
                              VNodeState bstate) {
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
     * @param bstate  Current bridge state value.
     * @return  New bridge state value.
     */
    VNodeState resume(VTNManagerImpl mgr, VNodeState bstate) {
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

        boolean valid = (node == null) ? true : checkNode(mgr, node);
        if (valid) {
            ConcurrentMap<VTenantPath, Object> db = mgr.getStateDB();
            db.put(mapPath, Boolean.valueOf(valid));
        }

        return getBridgeState(valid, bstate);
    }

    /**
     * Invoked when a node is added, removed, or changed.
     *
     * @param mgr   VTN Manager service.
     * @param db    Virtual node state DB.
     * @param bstate  Current bridge state value.
     * @param node  Node being updated.
     * @param type  Type of update.
     * @return  New bridge state value.
     */
    VNodeState notifyNode(VTNManagerImpl mgr,
                          ConcurrentMap<VTenantPath, Object> db,
                          VNodeState bstate, Node node, UpdateType type) {
        boolean cur = isValid(db);
        Node myNode = vlanMapConfig.getNode();
        if (myNode == null || !myNode.equals(node)) {
            return getBridgeState(cur, bstate);
        }

        boolean valid;
        switch (type) {
        case ADDED:
            valid = mgr.hasEdgePort(node);
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
     * @param nc      Node connector being updated.
     * @param pstate  The state of the node connector.
     * @param type    Type of update.
     * @return  New bridge state value.
     */
    VNodeState notifyNodeConnector(VTNManagerImpl mgr,
                                   ConcurrentMap<VTenantPath, Object> db,
                                   VNodeState bstate, NodeConnector nc,
                                   VNodeState pstate, UpdateType type) {
        boolean cur = isValid(db);
        Node node = nc.getNode();
        if (node == null) {
            return getBridgeState(cur, bstate);
        }

        Node myNode = vlanMapConfig.getNode();
        if (myNode == null || !myNode.equals(node)) {
            return getBridgeState(cur, bstate);
        }

        // VLAN mapping can work if at least one physical switch port
        // is up.
        boolean valid = (pstate == VNodeState.UP && mgr.isEdgePort(nc))
            ? true : mgr.hasEdgePort(node);

        setValid(db, cur, valid);
        return getBridgeState(valid, bstate);
    }

    /**
     * This method is called when topology graph is changed.
     *
     * @param mgr     VTN Manager service.
     * @param db      Virtual node state DB.
     * @param bstate  Current bridge state value.
     * @param estate  A {@link EdgeUpdateState} instance which contains
     *                information reported by the controller.
     * @return  New bridge state value.
     */
    VNodeState edgeUpdate(VTNManagerImpl mgr,
                          ConcurrentMap<VTenantPath, Object> db,
                          VNodeState bstate, EdgeUpdateState estate) {
        boolean cur = isValid(db);
        Node node = vlanMapConfig.getNode();
        if (node != null && estate.contains(node)) {
            boolean valid = estate.hasEdgePort(mgr, node);
            setValid(db, cur, valid);
            cur = valid;
        }

        return getBridgeState(cur, bstate);
    }

    /**
     * Transmit the specified packet to the network established by this
     * VLAN mapping.
     *
     * @param mgr   VTN manager service.
     * @param pctx  The context of the packet.
     * @param sent  A set of {@link PortVlan} which indicates the network
     *              already processed.
     */
    void transmit(VTNManagerImpl mgr, PacketContext pctx, Set<PortVlan> sent) {
        // Determine edge ports of this VLAN mapping.
        IVTNResourceManager resMgr = mgr.getResourceManager();
        Node node = vlanMapConfig.getNode();
        short vlan = vlanMapConfig.getVlan();
        HashSet<NodeConnector> ports = new HashSet<NodeConnector>();
        VlanMapPortFilter filter =
            VlanMapPortFilter.create(resMgr, node, vlan, sent);
        mgr.collectUpEdgePorts(ports, filter);

        if (ports.isEmpty()) {
            LOG.trace("{}:{}: transmit: No port is available",
                      mgr.getContainerName(), mapPath);
            return;
        }

        Ethernet frame = pctx.createFrame(vlan);
        for (NodeConnector nc: ports) {
            PortVlan pvlan = new PortVlan(nc, vlan);
            if (!sent.add(pvlan)) {
                continue;
            }

            if (LOG.isTraceEnabled()) {
                LOG.trace("{}:{}: Transmit packet to VLAN mapping: {}",
                          mgr.getContainerName(), mapPath,
                          pctx.getDescription(frame, nc, vlan));
            }
            mgr.transmit(nc, frame);
        }
    }

    /**
     * Determine whether the given node is suitable for the VLAN mapping or
     * not.
     *
     * @param mgr   VTN Manager service.
     * @param node  Node to be tested.
     * @return  {@code true} is returned only if the given node is valid.
     */
    private boolean checkNode(VTNManagerImpl mgr, Node node) {
        return (mgr.exists(node) && mgr.hasEdgePort(node));
    }

    /**
     * Derive L2 bridge state from current VLAN mapping state.
     *
     * @param valid   Current valid flag for the VLAN mapping.
     * @param bstate  Current bridge state value.
     * @return  New bridge state value.
     */
    private VNodeState getBridgeState(boolean valid, VNodeState bstate) {
        if (!valid) {
            return VNodeState.DOWN;
        }

        if (bstate == VNodeState.UNKNOWN) {
            return VNodeState.UP;
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
     * @param bpath  Path to the parent bridge.
     * @param nvlan  A {@link NodeVlan} instance which contains a pair of
     *               {@link Node} instance and VLAN ID.
     * @throws VTNException  An error occurred.
     */
    void register(VTNManagerImpl mgr, VBridgePath bpath, NodeVlan nvlan)
        throws VTNException {
        IVTNResourceManager resMgr = mgr.getResourceManager();
        MapReference ref;
        try {
            ref = resMgr.registerVlanMap(mgr, mapPath, nvlan, true);
        } catch (VTNException e) {
            Status status = e.getStatus();
            if (status == null ||
                status.getCode().equals(StatusCode.INTERNALERROR)) {
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
            throw new VTNException(StatusCode.CONFLICT, msg);
        }

        // Initialize the state of the VLAN mapping.
        Node node = vlanMapConfig.getNode();
        boolean valid = (node == null) ? true : checkNode(mgr, node);
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
                Status status = e.getStatus();
                if (status == null ||
                    status.getCode().equals(StatusCode.INTERNALERROR)) {
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
     * @return  A {@link VNodeRoute} instance.
     */
    @Override
    public VNodeRoute getIngressRoute() {
        return new VNodeRoute(mapPath, VNodeRoute.Reason.VLANMAPPED);
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
}
