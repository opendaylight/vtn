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
import java.util.Set;
import java.util.concurrent.ConcurrentMap;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import org.opendaylight.vtn.manager.IVTNManagerAware;
import org.opendaylight.vtn.manager.PortMap;
import org.opendaylight.vtn.manager.PortMapConfig;
import org.opendaylight.vtn.manager.SwitchPort;
import org.opendaylight.vtn.manager.VBridgeIfPath;
import org.opendaylight.vtn.manager.VInterface;
import org.opendaylight.vtn.manager.VInterfaceConfig;
import org.opendaylight.vtn.manager.VNodeRoute;
import org.opendaylight.vtn.manager.VNodeState;
import org.opendaylight.vtn.manager.VTNException;
import org.opendaylight.vtn.manager.VTenantPath;
import org.opendaylight.vtn.manager.internal.EdgeUpdateState;
import org.opendaylight.vtn.manager.internal.IVTNResourceManager;
import org.opendaylight.vtn.manager.internal.MacAddressTable;
import org.opendaylight.vtn.manager.internal.MiscUtils;
import org.opendaylight.vtn.manager.internal.NodeUtils;
import org.opendaylight.vtn.manager.internal.PacketContext;
import org.opendaylight.vtn.manager.internal.VTNManagerImpl;
import org.opendaylight.vtn.manager.internal.VTNThreadData;

import org.opendaylight.controller.sal.core.Node;
import org.opendaylight.controller.sal.core.NodeConnector;
import org.opendaylight.controller.sal.core.UpdateType;
import org.opendaylight.controller.sal.packet.Ethernet;
import org.opendaylight.controller.sal.utils.Status;
import org.opendaylight.controller.sal.utils.StatusCode;

/**
 * Implementation of virtual interface attached to the virtual layer 2 bridge.
 *
 * <p>
 *   Although this class is public to other packages, this class does not
 *   provide any API. Applications other than VTN Manager must not use this
 *   class.
 * </p>
 */
public final class VBridgeIfImpl implements VBridgeNode, Serializable {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = 837863564886135278L;

    /**
     * Logger instance.
     */
    private static final Logger  LOG =
        LoggerFactory.getLogger(VBridgeIfImpl.class);

    /**
     * Virtual L2 bridge which includes this interface.
     */
    private transient VBridgeImpl  parent;

    /**
     * Path to the bridge.
     */
    private transient VBridgeIfPath  ifPath;

    /**
     * Configuration for the interface.
     */
    private VInterfaceConfig  ifConfig;

    /**
     * Port mapping configuration.
     */
    private PortMapConfig  portMapConfig;

    /**
     * Construct a virtual interface instance.
     *
     * @param vbr   The virtual bridge to which a new interface is attached.
     * @param name  The name of the interface.
     * @param iconf Configuration for the interface.
     */
    VBridgeIfImpl(VBridgeImpl vbr, String name, VInterfaceConfig iconf) {
        ifConfig = resolve(iconf);
        setPath(vbr, name);
    }

    /**
     * Set virtual bridge interface path.
     *
     * @param vbr   Virtual L2 bridge which includes this interface.
     * @param name  The name of this interface.
     */
    void setPath(VBridgeImpl vbr, String name) {
        parent = vbr;
        ifPath = new VBridgeIfPath(vbr.getPath(), name);
    }

    /**
     * Return the name of the container to which the interface belongs.
     *
     * @return  The name of the container.
     */
    String getContainerName() {
        return parent.getContainerName();
    }

    /**
     * Return the name of the tenant to which the interface belongs.
     *
     * @return  The name of the container.
     */
    String getTenantName() {
        return parent.getTenantName();
    }

    /**
     * Return the name of the tenant to which the interface is attached.
     *
     * @return  The name of the bridge.
     */
    String getBridgeName() {
        return parent.getName();
    }

    /**
     * Return the name of the interface.
     *
     * @return  The name of the interface.
     */
    String getName() {
        return ifPath.getInterfaceName();
    }

    /**
     * Return the state of the interface.
     *
     * @param mgr  VTN Manager service.
     * @return The state of the interface.
     */
    VNodeState getState(VTNManagerImpl mgr) {
        VBridgeIfState ist = getIfState(mgr);
        return ist.getState();
    }

    /**
     * Derive L2 bridge state from current interfac state.
     *
     * @param db      Virtual node state DB.
     * @param bstate  Current bridge state value.
     * @return  New bridge state value.
     */
    VNodeState getBridgeState(ConcurrentMap<VTenantPath, Object> db,
                              VNodeState bstate) {
        VBridgeIfState ist = getIfState(db);
        VNodeState state = ist.getState();
        return getBridgeState(state, bstate);
    }

    /**
     * Return information about the interface.
     *
     * @param mgr  VTN Manager service.
     * @return  Information about the interface.
     */
    VInterface getVInterface(VTNManagerImpl mgr) {
        VBridgeIfState ist = getIfState(mgr);
        return new VInterface(getName(), ist.getState(), ist.getPortState(),
                              ifConfig);
    }

    /**
     * Return interface configuration.
     *
     * @return  Configuration for the interface.
     */
    VInterfaceConfig getVInterfaceConfig() {
        return ifConfig;
    }

    /**
     * Set interface configuration.
     *
     * @param mgr    VTN Manager service.
     * @param iconf  Interface configuration.
     * @param all    If {@code true} is specified, all attributes of the
     *               interface are modified. In this case, {@code null} in
     *               {@code iconf} is interpreted as default value.
     *               If {@code false} is specified, an attribute is not
     *               modified if its value in {@code iconf} is {@code null}.
     * @return  {@code true} if the configuration is actually changed.
     *          Otherwise {@code false}.
     */
    boolean setVInterfaceConfig(VTNManagerImpl mgr, VInterfaceConfig iconf,
                                boolean all) {
        VInterfaceConfig cf = (all) ? resolve(iconf) : merge(iconf);
        if (cf.equals(ifConfig)) {
            return false;
        }

        Boolean olden = ifConfig.getEnabled();
        Boolean newen = cf.getEnabled();
        boolean changed = !olden.equals(newen);

        ifConfig = cf;

        ConcurrentMap<VTenantPath, Object> db = mgr.getStateDB();
        VBridgeIfState ist = getIfState(db);
        VNodeState state;
        if (changed) {
            // Need to change interface state.
            if (newen.booleanValue()) {
                // Determine new state by the state of mapped port.
                if (portMapConfig == null) {
                    state = VNodeState.UNKNOWN;
                } else {
                    state = getNewState(mgr, ist);
                }
            } else {
                // State of disabled interface must be DOWN.
                state = VNodeState.DOWN;
            }

            ist.setState(state);
            if (ist.isDirty()) {
                if (state != VNodeState.UP) {
                    NodeConnector mapped = ist.getMappedPort();
                    if (mapped != null) {
                        // Invalidate all cached data added by the port
                        // mapping. Note that portMapConfig must not be null.
                        MacAddressTable table =
                            mgr.getMacAddressTable(parent.getPath());
                        short vlan = portMapConfig.getVlan();
                        flushCache(mgr, table, mapped, vlan);
                    }
                }

                db.put(ifPath, ist);
            }
        } else {
            state = ist.getState();
        }

        VNodeState pstate = ist.getPortState();
        VInterface viface = new VInterface(getName(), state, pstate, cf);
        VBridgeIfEvent.changed(mgr, ifPath, viface, true);
        return true;
    }

    /**
     * Return the port mapping configuration applied to the specified virtual
     * bridge interface.
     *
     * @return  Port mapping configuration.
     *          {@code null} is returned if port mapping is not configured.
     */
    PortMapConfig getPortMapConfig() {
        return portMapConfig;
    }

    /**
     * Return the port mapping information.
     *
     * @param mgr  VTN Manager service.
     * @return  Port mapping information. {@code null} is returned if the
     *          port mapping is not configured on the specified interface.
     */
    PortMap getPortMap(VTNManagerImpl mgr) {
        PortMapConfig pmconf = portMapConfig;
        if (pmconf == null) {
            return null;
        }

        VBridgeIfState ist = getIfState(mgr);
        return new PortMap(pmconf, ist.getMappedPort());
    }

    /**
     * Return a node connector associated with the switch port mapped to
     * this interface.
     *
     * @param mgr  VTN Manager service.
     * @return  A node connector. {@code null} is returned if no swtich port
     *          is mapped.
     */
    NodeConnector getMappedPort(VTNManagerImpl mgr) {
        VBridgeIfState ist = getIfState(mgr);
        return ist.getMappedPort();
    }

    /**
     * Create or destroy mapping between the physical switch port and the
     * virtual bridge interface.
     *
     * @param mgr     VTN Manager service.
     * @param pmconf  Port mapping configuration to be set.
     *                If {@code null} is specified, port mapping on the
     *                specified interface is destroyed.
     * @return New state of the interface.
     * @throws VTNException  An error occurred.
     */
    VNodeState setPortMap(VTNManagerImpl mgr, PortMapConfig pmconf)
        throws VTNException {
        ConcurrentMap<VTenantPath, Object> db = mgr.getStateDB();
        VBridgeIfState ist = getIfState(db);

        if (pmconf == null) {
            // Destroy port mapping.
            unsetPortMap(mgr, db, ist);
            return ist.getState();
        }

        PortMapConfig oldconf = portMapConfig;
        if (pmconf.equals(oldconf)) {
            // Nothing to do.
            return ist.getState();
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
        MapReference ref;
        try {
            ref = resMgr.registerPortMap(mgr, ifPath, pvlan, rmlan, true);
        } catch (VTNException e) {
            Status status = e.getStatus();
            if (status == null ||
                status.getCode().equals(StatusCode.INTERNALERROR)) {
                mgr.logException(LOG, ifPath, e, pvlan, rmlan);
            }
            throw e;
        }

        if (ref != null) {
            assert ref.getMapType() == MapType.PORT;

            throw new VTNException(StatusCode.CONFLICT,
                                   "Specified port is mapped to " +
                                   ref.getAbsolutePath());
        }

        // Update the state of the bridge interface.
        portMapConfig = pmconf;
        ist.setMappedPort(nc);
        PortMap pmap = new PortMap(pmconf, nc);
        if (oldconf == null) {
            PortMapEvent.added(mgr, ifPath, pmap);
        } else {
            PortMapEvent.changed(mgr, ifPath, pmap, true);
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
        db.put(ifPath, ist);
        if (ist.isDirty()) {
            VNodeState pstate = ist.getPortState();
            VInterface viface = new VInterface(getName(), state, pstate,
                                               ifConfig);
            VBridgeIfEvent.changed(mgr, ifPath, viface, false);
        }

        return state;
    }

    /**
     * Resume the virtual bridge interface.
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
        VNodeState state = VNodeState.UNKNOWN;
        ConcurrentMap<VTenantPath, Object> db = mgr.getStateDB();
        VBridgeIfState ist = getIfState(db);

        PortMapConfig pmconf = portMapConfig;
        if (pmconf != null) {
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
            state = getNewState(mgr, ist);
        }

        if (!isEnabled()) {
            // State of disabled interface must be DOWN.
            state = VNodeState.DOWN;
        }

        ist.setState(state);
        if (ist.isDirty()) {
            db.put(ifPath, ist);
        }
        if (LOG.isDebugEnabled()) {
            LOG.debug("{}:{}: Resume interface: state={}",
                      getContainerName(), ifPath, state);
        }

        return getBridgeState(state, bstate);
    }

    /**
     * Invoked when a node is added, removed, or changed.
     *
     * @param mgr     VTN Manager service.
     * @param db      Virtual node state DB.
     * @param bstate  Current bridge state value.
     * @param node    Node being updated.
     * @param type    Type of update.
     * @return  New bridge state value.
     */
    VNodeState notifyNode(VTNManagerImpl mgr,
                          ConcurrentMap<VTenantPath, Object> db,
                          VNodeState bstate, Node node, UpdateType type) {
        VBridgeIfState ist = getIfState(db);
        VNodeState cur = ist.getState();
        if (type != UpdateType.REMOVED) {
            return getBridgeState(cur, bstate);
        }

        PortMapConfig pmconf = portMapConfig;
        if (pmconf == null) {
            return getBridgeState(cur, bstate);
        }

        NodeConnector mapped = ist.getMappedPort();
        if (mapped == null) {
            return getBridgeState(cur, bstate);
        }

        Node pnode = pmconf.getNode();
        if (pnode.equals(node)) {
            // No need to flush MAC address table entries and flow entries.
            // It will be done by the VTN Manager service.
            unmapPort(mgr, db, ist, ifPath, false);
            cur = VNodeState.DOWN;
            setState(mgr, db, ist, cur);
        }

        return getBridgeState(cur, bstate);
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
        VBridgeIfState ist = getIfState(db);
        VNodeState cur = ist.getState();
        PortMapConfig pmconf = portMapConfig;
        if (pmconf == null) {
            return getBridgeState(cur, bstate);
        }

        NodeConnector mapped = ist.getMappedPort();
        VNodeState state = VNodeState.UNKNOWN;
        switch (type) {
        case ADDED:
            if (mapped != null || !portMatch(mgr, pmconf, nc)) {
                return getBridgeState(cur, bstate);
            }

            // Try to establish port mapping.
            if (!mapPort(mgr, db, ist, nc, false)) {
                return getBridgeState(cur, bstate);
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
                    return getBridgeState(cur, bstate);
                }

                state = getNewState(mgr, ist, nc, pstate);
            } else if (nc.equals(mapped)) {
                if (pmconf.getPort().getName() != null &&
                    !portMatch(mgr, pmconf, nc)) {
                    // This port should be unmapped because its name has been
                    // changed. In this case flow and MAC address table entries
                    // need to be purged.
                    unmapPort(mgr, db, ist, ifPath, true);
                    state = VNodeState.DOWN;
                } else {
                    state = getNewState(mgr, ist, nc, pstate);
                }
            } else {
                return getBridgeState(cur, bstate);
            }
            break;

        case REMOVED:
            if (nc.equals(mapped)) {
                // No need to flush MAC address table entries and flow entries.
                // It will be done by the VTN Manager service.
                unmapPort(mgr, db, ist, ifPath, false);
                state = VNodeState.DOWN;
            } else {
                return getBridgeState(cur, bstate);
            }
            break;

        default:
            return getBridgeState(cur, bstate);
        }

        setState(mgr, db, ist, state);

        return getBridgeState(state, bstate);
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
        VBridgeIfState ist = getIfState(db);
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

        return getBridgeState(cur, bstate);
    }

    /**
     * Notify the listener of current configuration.
     *
     * @param mgr       VTN Manager service.
     * @param listener  VTN manager listener service.
     */
    void notifyConfiguration(VTNManagerImpl mgr, IVTNManagerAware listener) {
        UpdateType type = UpdateType.ADDED;
        VInterface viface = getVInterface(mgr);
        mgr.notifyChange(listener, ifPath, viface, type);

        PortMapConfig pmconf = portMapConfig;
        if (pmconf != null) {
            VBridgeIfState ist = getIfState(mgr);
            NodeConnector mapped = ist.getMappedPort();
            PortMap pmap = new PortMap(pmconf, mapped);
            mgr.notifyChange(listener, ifPath, pmap, type);
        }
    }

    /**
     * Determine whether the received packet should be treated as input of
     * this interface or not.
     *
     * <p>
     *   Note that this method does not see the state of the interface.
     *   This method returns {@code true} if the network specified by the
     *   switch port and the VLAN ID is mapped to this interface.
     * </p>
     *
     * @param mgr   VTN Manager service.
     * @param nc    A node connector where the packet was received.
     * @param vlan  VLAN ID in the received packet.
     * @return  {@code true} is returned only if the received packet should be
     *          treated as input of this interface.
     */
    boolean match(VTNManagerImpl mgr, NodeConnector nc, short vlan) {
        PortMapConfig pmconf = portMapConfig;
        if (pmconf == null) {
            return false;
        }

        VBridgeIfState ist = getIfState(mgr);
        NodeConnector mapped = ist.getMappedPort();
        return (nc.equals(mapped) && pmconf.getVlan() == vlan);
    }

    /**
     * Transmit the specified packet to this interface.
     *
     * @param mgr   VTN manager service.
     * @param pctx  The context of the packet.
     * @param sent  A set of {@link PortVlan} which indicates the network
     *              already processed.
     */
    void transmit(VTNManagerImpl mgr, PacketContext pctx,
                  Set<PortVlan> sent) {
        PortMapConfig pmconf = portMapConfig;
        if (pmconf == null) {
            return;
        }

        VBridgeIfState ist = getIfState(mgr);
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
        if (LOG.isTraceEnabled()) {
            LOG.trace("{}:{}: Transmit packet: {}",
                      getContainerName(), ifPath,
                      pctx.getDescription(frame, mapped, vlan));
        }

        mgr.transmit(mapped, frame);
    }

    /**
     * Destroy the virtual bridge interface.
     *
     * @param mgr     VTN manager service.
     * @param retain  {@code true} means that the parent bridge will be
     *                retained. {@code false} means that the parent bridge
     *                is being destroyed.
     */
    void destroy(VTNManagerImpl mgr, boolean retain) {
        ConcurrentMap<VTenantPath, Object> db = mgr.getStateDB();
        VBridgeIfState ist = getIfState(mgr);
        VNodeState state = ist.getState();
        VNodeState pstate = ist.getPortState();
        VInterface viface = new VInterface(getName(), state, pstate, ifConfig);
        PortMapConfig pmconf = portMapConfig;
        if (pmconf != null) {
            NodeConnector mapped = ist.getMappedPort();
            PortMap pmap = new PortMap(pmconf, mapped);
            if (mapped != null) {
                unmapPort(mgr, db, ist, null, retain);
            }

            PortMapEvent.removed(mgr, ifPath, pmap, false);
        }

        db.remove(ifPath);
        VBridgeIfEvent.removed(mgr, ifPath, viface, retain);

        // Unlink parent for GC.
        parent = null;
    }

    /**
     * Merge the specified interface configuration to the current
     * configuration.
     *
     * <p>
     *   If at least one field in {@code iconf} keeps a valid value, this
     *   method creates a shallow copy of this object, and set valid values
     *   in {@code iconf} to the copy.
     * </p>
     *
     * @param iconf  Configuration to be merged.
     * @return  A merged {@code VInterfaceConfig} object.
     */
    VInterfaceConfig merge(VInterfaceConfig iconf) {
        String desc = iconf.getDescription();
        Boolean en = iconf.getEnabled();
        if (desc == null) {
            return (en == null)
                ? ifConfig
                : new VInterfaceConfig(ifConfig.getDescription(), en);
        } else if (en == null) {
            return new VInterfaceConfig(desc, ifConfig.getEnabled());
        }

        return iconf;
    }

    /**
     * Resolve undefined attributes in the specified interface configuration.
     *
     * @param iconf  The interface configuration.
     * @return       {@code VInterfaceConfig} to be applied.
     */
    private VInterfaceConfig resolve(VInterfaceConfig iconf) {
        Boolean enabled = iconf.getEnabled();
        if (enabled == null) {
            // Interface should be enabled by default.
            return new VInterfaceConfig(iconf.getDescription(), Boolean.TRUE);
        }

        return iconf;
    }

    /**
     * Derive L2 bridge state from current interfac state.
     *
     * @param state   Current interface state.
     * @param bstate  Current bridge state value.
     * @return  New bridge state value.
     */
    private VNodeState getBridgeState(VNodeState state, VNodeState bstate) {
        if (state == VNodeState.DOWN) {
            return VNodeState.DOWN;
        }

        if (bstate == VNodeState.UNKNOWN && state == VNodeState.UP) {
            return VNodeState.UP;
        }

        return bstate;
    }

    /**
     * Set state of the bridge interface.
     *
     * @param mgr    VTN Manager service.
     * @param db     Virtual node state DB.
     * @param ist    Runtime state of the interface.
     * @param state  New interface state.
     * @return  {@code true} is returned if the state of this interface was
     *          actually changed. {@code false} is returned if not changed.
     */
    private boolean setState(VTNManagerImpl mgr,
                             ConcurrentMap<VTenantPath, Object> db,
                             VBridgeIfState ist, VNodeState state) {
        VNodeState st;
        if (!isEnabled()) {
            // State of disabled interface must be DOWN.
            st = VNodeState.DOWN;
        } else {
            st = state;
        }

        ist.setState(st);
        boolean dirty = ist.isDirty();
        if (dirty) {
            db.put(ifPath, ist);
            VNodeState pstate = ist.getPortState();
            VInterface viface = new VInterface(getName(), st, pstate,
                                               ifConfig);
            VBridgeIfEvent.changed(mgr, ifPath, viface, false);
        }

        return dirty;
    }

    /**
     * Derive new interface state from the specified node connector.
     *
     * <p>
     *   Note that this method may update the state of the mapped switch port
     *   in {@code ist}.
     * </p>
     *
     * @param mgr  VTN Manager service.
     * @param ist  Runtime state of the interface.
     * @return  New interface state.
     */
    private VNodeState getNewState(VTNManagerImpl mgr, VBridgeIfState ist) {
        NodeConnector nc = ist.getMappedPort();
        if (nc == null) {
            return VNodeState.DOWN;
        }

        VNodeState pstate = getPortState(mgr, nc);
        return getNewState(mgr, ist, nc, pstate);
    }

    /**
     * Derive new interface state from the specified node connector.
     *
     * <p>
     *   Note that this method always updates the state of the mapped switch
     *   port in {@code ist}.
     * </p>
     *
     * @param mgr     VTN Manager service.
     * @param ist     Runtime state of the interface.
     * @param nc      Node connector associated with the switch port mapped to
     *                this interface.
     * @param pstate  State of the switch port mapped to this interface.
     * @return  New interface state.
     */
    private VNodeState getNewState(VTNManagerImpl mgr, VBridgeIfState ist,
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
     * @param nc   Node connector mapped to this interface.
     * @return  New interface state.
     */
    private VNodeState getPortState(VTNManagerImpl mgr, NodeConnector nc) {
        if (nc == null) {
            return VNodeState.UNKNOWN;
        }

        return (mgr.isEnabled(nc)) ? VNodeState.UP : VNodeState.DOWN;
    }

    /**
     * Destroy mapping between the physical switch port and the virtual
     * bridge interface.
     *
     * @param mgr   VTN Manager service.
     * @param db    Virtual node state DB.
     * @param ist   Runtime state of the interface.
     */
    private void unsetPortMap(VTNManagerImpl mgr,
                              ConcurrentMap<VTenantPath, Object> db,
                              VBridgeIfState ist) {
        PortMapConfig pmconf = portMapConfig;
        if (pmconf != null) {
            NodeConnector mapped = ist.getMappedPort();
            if (mapped != null) {
                unmapPort(mgr, db, ist, null, true);
            }

            portMapConfig = null;
            PortMap pmap = new PortMap(pmconf, mapped);
            PortMapEvent.removed(mgr, ifPath, pmap, true);
            setState(mgr, db, ist, VNodeState.UNKNOWN);
        }
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
     * @param ist       Runtime state of the interface.
     * @param nc        Node connector to be mapped.
     * @param resuming  {@code true} if this method is called by
     *                  {@link #resume(VTNManagerImpl, VNodeState)}.
     * @return  {@code true} is returned if the given port is successfully
     *          mapped. {@code false} is returned if a switch port is already
     *          mapped to this interface.
     */
    private boolean mapPort(VTNManagerImpl mgr,
                            ConcurrentMap<VTenantPath, Object> db,
                            VBridgeIfState ist, NodeConnector nc,
                            boolean resuming) {
        short vlan = portMapConfig.getVlan();
        PortVlan pvlan = new PortVlan(nc, vlan);
        IVTNResourceManager resMgr = mgr.getResourceManager();
        MapReference ref;

        try {
            ref = resMgr.registerPortMap(mgr, ifPath, pvlan, null, !resuming);
        } catch (VTNException e) {
            LOG.error(getContainerName() + ":" + ifPath +
                      ": Failed to map port: "+ pvlan, e);
            return false;
        }

        if (ref == null) {
            ist.setMappedPort(nc);
            db.put(ifPath, ist);

            // Clear dirty flag.
            ist.isDirty();

            if (!resuming) {
                PortMap pmap = new PortMap(portMapConfig, nc);
                PortMapEvent.changed(mgr, ifPath, pmap, false);
            }
            return true;
        }

        LOG.error("{}:{}: Switch port is already mapped to {}: {}",
                  getContainerName(), ifPath, ref.getAbsolutePath(), pvlan);
        return false;
    }

    /**
     * Unmap the physical switch port currently mapped.
     *
     * @param mgr    VTN Manager service.
     * @param db     Virtual node state DB.
     * @param ist    Runtime state of the interface.
     * @param path   Path to the bridge interface. {@code null} can be passed
     *               if no need to notify event.
     * @param purge  If {@code true} is specified, network caches originated
     *               by the port mapping will be purged.
     */
    private void unmapPort(VTNManagerImpl mgr,
                           ConcurrentMap<VTenantPath, Object> db,
                           VBridgeIfState ist, VBridgeIfPath path,
                           boolean purge) {
        // Unregister port mapping.
        short vlan = portMapConfig.getVlan();
        PortVlan pvlan = new PortVlan(ist.getMappedPort(), vlan);
        IVTNResourceManager resMgr = mgr.getResourceManager();
        try {
            resMgr.unregisterPortMap(mgr, ifPath, pvlan, purge);
        } catch (VTNException e) {
            LOG.error(getContainerName() + ":" + ifPath +
                      ": Failed to unmap port: "+ pvlan, e);
            // FALLTHROUGH
        }

        ist.setMappedPort(null);
        db.put(ifPath, ist);

        // Clear dirty flag.
        ist.isDirty();

        if (path != null) {
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
     * @param table  MAC address table. Specifying {@code null} means that
     *               there is no need to flush MAC address table entries.
     * @param port   A node connector associated with a switch port.
     * @param vlan   A VLAN ID.
     */
    private void flushCache(VTNManagerImpl mgr, MacAddressTable table,
                            NodeConnector port, short vlan) {
        if (table != null) {
            // Flush MAC address table entries added by obsolete port mapping.
            table.flush(port, vlan);
        }

        // Remove flow entries relevant to obsolete port mapping.
        VTNThreadData.removeFlows(mgr, getTenantName(), port, vlan);
    }

    /**
     * Return a runtime state object for the virtual bridge interface.
     *
     * @param mgr  VTN Manager service.
     * @return  A runtume state object.
     */
    private VBridgeIfState getIfState(VTNManagerImpl mgr) {
        ConcurrentMap<VTenantPath, Object> db = mgr.getStateDB();
        return getIfState(db);
    }

    /**
     * Return a runtime state object for the virtual bridge interface.
     *
     * @param db  Runtime state DB.
     * @return  A runtume state object.
     */
    private VBridgeIfState getIfState(ConcurrentMap<VTenantPath, Object> db) {
        VBridgeIfState ist = (VBridgeIfState)db.get(ifPath);
        if (ist == null) {
            VNodeState state = (isEnabled())
                ? VNodeState.UNKNOWN : VNodeState.DOWN;
            ist = new VBridgeIfState(state);
        }

        return ist;
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
        if (!(o instanceof VBridgeIfImpl)) {
            return false;
        }

        VBridgeIfImpl vif = (VBridgeIfImpl)o;
        if (!ifPath.equals(vif.ifPath)) {
            return false;
        }

        VInterfaceConfig iconf = getVInterfaceConfig();
        VInterfaceConfig otherIconf = vif.getVInterfaceConfig();
        if (!iconf.equals(otherIconf)) {
            return false;
        }

        return true;
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        return ifPath.hashCode() ^ getVInterfaceConfig().hashCode();
    }

    // VBridgeNode

    /**
     * Return path to this interface.
     *
     * @return  Path to the interface.
     */
    @Override
    public VBridgeIfPath getPath() {
        return ifPath;
    }

    /**
     * Determine whether this interface is administravely enabled or not.
     *
     * @return {@code true} is returned only if this interface is enabled.
     */
    @Override
    public boolean isEnabled() {
        return ifConfig.getEnabled().booleanValue();
    }

    /**
     * Return a {@link VNodeRoute} instance which indicates the packet was
     * mapped by the port mapping.
     *
     * @return  A {@link VNodeRoute} instance.
     */
    @Override
    public VNodeRoute getIngressRoute() {
        return new VNodeRoute(ifPath, VNodeRoute.Reason.PORTMAPPED);
    }
}
