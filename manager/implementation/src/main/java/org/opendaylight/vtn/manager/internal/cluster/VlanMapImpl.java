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
import java.util.List;
import java.util.Set;
import java.util.HashSet;
import java.util.concurrent.ConcurrentMap;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import org.opendaylight.vtn.manager.VNodeState;
import org.opendaylight.vtn.manager.VTenantPath;
import org.opendaylight.vtn.manager.VlanMapConfig;
import org.opendaylight.vtn.manager.internal.IVTNResourceManager;
import org.opendaylight.vtn.manager.internal.PacketContext;
import org.opendaylight.vtn.manager.internal.VTNManagerImpl;
import org.opendaylight.vtn.manager.internal.VTNThreadData;

import org.opendaylight.controller.sal.core.Edge;
import org.opendaylight.controller.sal.core.Node;
import org.opendaylight.controller.sal.core.NodeConnector;
import org.opendaylight.controller.sal.core.UpdateType;
import org.opendaylight.controller.sal.packet.Ethernet;
import org.opendaylight.controller.sal.topology.TopoEdgeUpdate;

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
public final class VlanMapImpl implements VBridgeNode, Serializable {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = -784362913741696063L;

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
     * @param mgr    VTN Manager service.
     * @param vbr    Virtual L2 bridge which includes this mapping.
     * @param mapId  Identifier of the VLAN mapping.
     * @param vlconf  VLAN mapping configuration.
     */
    VlanMapImpl(VTNManagerImpl mgr, VBridgeImpl vbr, String mapId,
                VlanMapConfig vlconf) {
        vlanMapConfig = vlconf;
        setPath(vbr, mapId);

        Node node = vlconf.getNode();
        boolean valid = (node == null) ? true : checkNode(mgr, node);
        ConcurrentMap<VTenantPath, Object> db = mgr.getStateDB();
        db.put(mapPath, Boolean.valueOf(valid));
    }

    /**
     * Set VLAN mapping path.
     *
     * @param vbr    Virtual L2 bridge which includes this mapping.
     * @param mapId  Identifier of the VLAN mapping.
     */
    void setPath(VBridgeImpl vbr, String mapId) {
        mapPath = new VlanMapPath(vbr.getPath(), mapId);
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
        Node node = vlanMapConfig.getNode();
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
     * @param mgr       VTN Manager service.
     * @param db        Virtual node state DB.
     * @param bstate    Current bridge state value.
     * @param topoList  List of topoedgeupdates Each topoedgeupdate includes
     *                  edge, its Properties (BandWidth and/or Latency etc)
     *                  and update type.
     * @return  New bridge state value.
     */
    VNodeState edgeUpdate(VTNManagerImpl mgr,
                          ConcurrentMap<VTenantPath, Object> db,
                          VNodeState bstate, List<TopoEdgeUpdate> topoList) {
        boolean cur = isValid(db);
        Node node = vlanMapConfig.getNode();
        if (node != null) {
            for (TopoEdgeUpdate topo: topoList) {
                Edge e = topo.getEdge();
                NodeConnector head = e.getHeadNodeConnector();
                NodeConnector tail = e.getTailNodeConnector();
                if (node.equals(head.getNode()) ||
                    node.equals(tail.getNode())) {
                    boolean valid = mgr.hasEdgePort(node);
                    setValid(db, cur, valid);
                    cur = valid;
                    break;
                }
            }
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
        Set<NodeConnector> ports = getPorts(mgr);
        if (ports == null) {
            LOG.trace("{}:{}: transmit: No port is available",
                      mgr.getContainerName(), mapPath);
            return;
        }

        IVTNResourceManager resMgr = mgr.getResourceManager();
        short vlan = vlanMapConfig.getVlan();
        Ethernet frame = pctx.createFrame(vlan);
        for (NodeConnector nc: ports) {
            PortVlan pvlan = new PortVlan(nc, vlan);
            if (resMgr.isPortMapped(pvlan)) {
                // This switch port is mapped to virtual interface by port
                // mapping. This packet should not be sent to this port
                // because port mapping always overrides VLAN mapping.
                continue;
            }
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
     * Return a set of node connectors associated with this VLAN mapping.
     *
     * @param mgr  VTN Manager service.
     * @return  A set of node connectors.
     *          Node that {@code null} may be returned.
     */
    private Set<NodeConnector> getPorts(VTNManagerImpl mgr) {
        Node node = vlanMapConfig.getNode();
        HashSet<NodeConnector> ncSet = new HashSet<NodeConnector>();
        mgr.collectUpEdgePorts(ncSet, node);
        return ncSet;
    }

    /**
     * Destroy the VLAN mapping.
     *
     * @param mgr            VTN manager service.
     * @param bridgeDestroy  {@code true} is specified if the parent bridge is
     *                       being destroyed.
     */
    void destroy(VTNManagerImpl mgr, boolean bridgeDestroy) {
        ConcurrentMap<VTenantPath, Object> db = mgr.getStateDB();
        db.remove(mapPath);

        if (!bridgeDestroy) {
            // Purge all VTN flows related to this mapping.
            VTNThreadData.removeFlows(mgr, mapPath);
        }
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
     * Return path to this interface.
     *
     * @return  Path to the interface.
     */
    @Override
    public VTenantPath getPath() {
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
}
