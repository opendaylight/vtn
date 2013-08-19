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
import java.io.ObjectInputStream;
import java.io.IOException;
import java.util.Arrays;
import java.util.Iterator;
import java.util.Map;
import java.util.TreeMap;
import java.util.List;
import java.util.ArrayList;
import java.util.Set;
import java.util.HashSet;
import java.util.TreeSet;
import java.util.concurrent.ConcurrentMap;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantReadWriteLock;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import org.opendaylight.vtn.manager.IVTNManagerAware;
import org.opendaylight.vtn.manager.PortMap;
import org.opendaylight.vtn.manager.PortMapConfig;
import org.opendaylight.vtn.manager.VBridge;
import org.opendaylight.vtn.manager.VBridgeConfig;
import org.opendaylight.vtn.manager.VBridgeIfPath;
import org.opendaylight.vtn.manager.VBridgePath;
import org.opendaylight.vtn.manager.VInterface;
import org.opendaylight.vtn.manager.VInterfaceConfig;
import org.opendaylight.vtn.manager.VNodeState;
import org.opendaylight.vtn.manager.VTNException;
import org.opendaylight.vtn.manager.VTenantPath;
import org.opendaylight.vtn.manager.VlanMap;
import org.opendaylight.vtn.manager.VlanMapConfig;
import org.opendaylight.vtn.manager.internal.IVTNResourceManager;
import org.opendaylight.vtn.manager.internal.MacAddressTable;
import org.opendaylight.vtn.manager.internal.MacTableEntry;
import org.opendaylight.vtn.manager.internal.MapType;
import org.opendaylight.vtn.manager.internal.PacketContext;
import org.opendaylight.vtn.manager.internal.VTNManagerImpl;

import org.opendaylight.controller.sal.core.Node;
import org.opendaylight.controller.sal.core.NodeConnector;
import org.opendaylight.controller.sal.core.Path;
import org.opendaylight.controller.sal.core.UpdateType;
import org.opendaylight.controller.sal.packet.Ethernet;
import org.opendaylight.controller.sal.packet.PacketResult;
import org.opendaylight.controller.sal.routing.IRouting;
import org.opendaylight.controller.sal.topology.TopoEdgeUpdate;
import org.opendaylight.controller.sal.utils.NetUtils;
import org.opendaylight.controller.sal.utils.Status;
import org.opendaylight.controller.sal.utils.StatusCode;
import org.opendaylight.controller.switchmanager.ISwitchManager;

/**
 * Implementation of virtual layer 2 bridge.
 *
 * <p>
 *   Although this class is public to other packages, this class does not
 *   provide any API. Applications other than VTN Manager must not use this
 *   class.
 * </p>
 */
public class VBridgeImpl implements Serializable {
    private static final long serialVersionUID = 1;

    /**
     * Logger instance.
     */
    private final static Logger  LOG =
        LoggerFactory.getLogger(VBridgeImpl.class);

    /**
     * Default interval of MAC address table aging.
     */
    private final static int  DEFAULT_AGE_INTERVAL = 600;

    /**
     * Maximum interval of MAC address table aging.
     */
    private final static int  MAX_AGE_INTERVAL = 1000000;

    /**
     * Minimum interval of MAC address table aging.
     */
    private final static int  MIN_AGE_INTERVAL = 10;

    /**
     * Pseudo node identifier which indicates that the node is unspecified.
     */
    private final static String  NODEID_ANY = "ANY";

    /**
     * Virtual tenant which includes this bridge.
     */
    private transient VTenantImpl  parent;

    /**
     * Path to the bridge.
     */
    private transient VBridgePath  bridgePath;

    /**
     * Configuration for the bridge.
     */
    private VBridgeConfig  bridgeConfig;

    /**
     * Attached virtual interfaces.
     */
    private final TreeMap<String, VBridgeIfImpl> vInterfaces =
        new TreeMap<String, VBridgeIfImpl>();

    /**
     * VLAN mappings applied to this bridge.
     */
    private final TreeMap<String, VlanMapImpl> vlanMaps =
        new TreeMap<String, VlanMapImpl>();

    /**
     * Read write lock to synchronize per-bridge resources.
     */
    private transient ReentrantReadWriteLock  rwLock =
        new ReentrantReadWriteLock();

    /**
     * Construct a virtual bridge instance.
     *
     * @param vtn   The virtual tenant to which a new bridge belongs.
     * @param name  The name of the bridge.
     * @param bconf Configuration for the bridge.
     * @throws VTNException  An error occurred.
     */
    VBridgeImpl(VTenantImpl vtn, String name, VBridgeConfig bconf)
        throws VTNException {
        bconf = resolve(bconf);
        checkConfig(bconf);
        bridgeConfig = bconf;
        setPath(vtn, name);
    }

    /**
     * Set virtual bridge path.
     *
     * @param vtn   Virtual tenant which includes this bridge.
     * @param name  The name of this bridge.
     */
    void setPath(VTenantImpl vtn, String name) {
        parent = vtn;
        bridgePath = new VBridgePath(vtn.getName(), name);

        // Set this bridge as parent of interfaces.
        for (Map.Entry<String, VBridgeIfImpl> entry: vInterfaces.entrySet()) {
            String iname = entry.getKey();
            VBridgeIfImpl vif = entry.getValue();
            vif.setPath(this, iname);
        }

        // Initialize VLAN mapping path.
        for (Map.Entry<String, VlanMapImpl> entry: vlanMaps.entrySet()) {
            String mapId = entry.getKey();
            VlanMapImpl vmap = entry.getValue();
            vmap.setPath(this, mapId);
        }
    }

    /**
     * Return the name of the container to which the bridge belongs.
     *
     * @return  The name of the container.
     */
    String getContainerName() {
        return parent.getContainerName();
    }

    /**
     * Return the name of the tenant to which the bridge belongs.
     *
     * @return  The name of the container.
     */
    String getTenantName() {
        return parent.getName();
    }

    /**
     * Return the name of the bridge.
     *
     * @return  The name of the bridge.
     */
    String getName() {
        return bridgePath.getBridgeName();
    }

    /**
     * Return path to this bridge.
     *
     * @return  Path to the bridge.
     */
    VBridgePath getPath() {
        return bridgePath;
    }

    /**
     * Return the state of the bridge.
     *
     * @param mgr  VTN Manager service.
     * @return  The state of the bridge.
     */
    VNodeState getState(VTNManagerImpl mgr) {
        VBridgeState bst = getBridgeState(mgr);
        return bst.getState();
    }

    /**
     * Return information about the bridge.
     *
     * @param mgr  VTN Manager service.
     * @return  Information about the bridge.
     */
    VBridge getVBridge(VTNManagerImpl mgr) {
        return getVBridge(mgr, getName(), getVBridgeConfig());
    }

    /**
     * Return bridge configuration.
     *
     * @return  Configuration for the bridge.
     */
    synchronized VBridgeConfig getVBridgeConfig() {
        return bridgeConfig;
    }

    /**
     * Set bridge configuration.
     *
     * @param mgr    VTN Manager service.
     * @param bconf  Bridge configuration.
     * @param all    If {@code true} is specified, all attributes of the
     *               bridge are modified. In this case, {@code null} in
     *               {@code bconf} is interpreted as default value.
     *               If {@code false} is specified, an attribute is not
     *               modified if its value in {@code bconf} is {@code null}.
     * @return  {@code true} if the configuration is actually changed.
     *          Otherwise {@code false}.
     * @throws VTNException  An error occurred.
     */
    synchronized boolean setVBridgeConfig(VTNManagerImpl mgr,
                                          VBridgeConfig bconf, boolean all)
        throws VTNException {
        bconf = (all) ? resolve(bconf) : merge(bconf);
        if (bconf.equals(bridgeConfig)) {
            return false;
        }

        checkConfig(bconf);
        bridgeConfig = bconf;
        String name = bridgePath.getBridgeName();
        VBridge vbridge = getVBridge(mgr, name, bconf);
        mgr.notifyChange(bridgePath, vbridge, UpdateType.CHANGED);

        initMacTableAging(mgr);
        return true;
    }

    /**
     * Add a new virtual interface to this bridge.
     *
     * @param mgr    VTN Manager service.
     * @param path   Path to the bridge.
     * @param iconf  Interface configuration.
     * @throws VTNException  An error occurred.
     */
    void addInterface(VTNManagerImpl mgr, VBridgeIfPath path,
                      VInterfaceConfig iconf) throws VTNException {
        // Ensure the given interface name is valid.
        String ifName = path.getInterfaceName();
        VTNManagerImpl.checkName("Interface", ifName);

        if (iconf == null) {
            Status status = VTNManagerImpl.
                argumentIsNull("Interface configuration");
            throw new VTNException(status);
        }

        VBridgeIfImpl vif = new VBridgeIfImpl(this, ifName, iconf);
        Lock wrlock = rwLock.writeLock();
        wrlock.lock();
        try {
            VBridgeIfImpl old = vInterfaces.put(ifName, vif);
            if (old != null) {
                vInterfaces.put(ifName, old);
                String msg = ifName + ": Interface name already exists";
                throw new VTNException(StatusCode.CONFLICT, msg);
            }

            VInterface viface = vif.getVInterface(mgr);
            mgr.notifyChange(path, viface, UpdateType.ADDED);
            updateState(mgr);
        } finally {
            wrlock.unlock();
        }
    }

    /**
     * Change configuration of existing virtual interface.
     *
     * @param mgr    VTN Manager service.
     * @param path   Path to the interface.
     * @param iconf  Interface configuration.
     * @param all    If {@code true} is specified, all attributes of the
     *               interface are modified. In this case, {@code null} in
     *               {@code iconf} is interpreted as default value.
     *               If {@code false} is specified, an attribute is not
     *               modified if its value in {@code iconf} is {@code null}.
     * @return  {@code true} is returned only if the interface configuration is
     *          actually changed.
     * @throws VTNException  An error occurred.
     */
    boolean modifyInterface(VTNManagerImpl mgr, VBridgeIfPath path,
                            VInterfaceConfig iconf, boolean all)
        throws VTNException {
        if (iconf == null) {
            Status status = VTNManagerImpl.
                argumentIsNull("Interface configuration");
            throw new VTNException(status);
        }

        // Write lock is needed because this code determines the state of
        // this bridge by scanning interfaces.
        Lock wrlock = rwLock.writeLock();
        wrlock.lock();
        try {
            VBridgeIfImpl vif = getInterfaceImpl(path);
            if (!vif.setVInterfaceConfig(mgr, iconf, all)) {
                return false;
            }
            updateState(mgr);
            return true;
        } finally {
            wrlock.unlock();
        }
    }

    /**
     * Remove the specified virtual interface.
     *
     * @param mgr   VTN Manager service.
     * @param path  Path to the interface.
     * @throws VTNException  An error occurred.
     */
    void removeInterface(VTNManagerImpl mgr, VBridgeIfPath path)
        throws VTNException {
        Lock wrlock = rwLock.writeLock();
        wrlock.lock();
        try {
            String ifName = path.getInterfaceName();
            if (ifName == null) {
                Status status = VTNManagerImpl.
                    argumentIsNull("Interface name");
                throw new VTNException(status);
            }

            VBridgeIfImpl vif = vInterfaces.remove(ifName);
            if (vif == null) {
                Status status = interfaceNotFound(ifName);
                throw new VTNException(status);
            }

            vif.destroy(mgr);
            updateState(mgr);
        } finally {
            wrlock.unlock();
        }
    }

    /**
     * Return a list of all bridge interface information.
     *
     * @param mgr   VTN Manager service.
     * @return  A list of bridge interface information.
     */
    List<VInterface> getInterfaces(VTNManagerImpl mgr) {
        ArrayList<VInterface> list = new ArrayList<VInterface>();
        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            for (VBridgeIfImpl vif: vInterfaces.values()) {
                list.add(vif.getVInterface(mgr));
            }
        } finally {
            rdlock.unlock();
        }

        return list;
    }

    /**
     * Return information about the virtual bridge interface associated with
     * the given name.
     *
     * @param mgr   VTN Manager service.
     * @param path  Path to the interface.
     * @return  The virtual interface information associated with the given
     *          name.
     * @throws VTNException  An error occurred.
     */
    VInterface getInterface(VTNManagerImpl mgr, VBridgeIfPath path)
        throws VTNException {
        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            VBridgeIfImpl vif = getInterfaceImpl(path);
            return vif.getVInterface(mgr);
        } finally {
            rdlock.unlock();
        }
    }

    /**
     * Add VLAN mapping to this bridge.
     *
     * @param mgr     VTN Manager serivce.
     * @param vlconf  Configuration for the VLAN mapping.
     * @return  Information about the added VLAN mapping is returned.
     * @throws VTNException  An error occurred.
     */
    VlanMap addVlanMap(VTNManagerImpl mgr, VlanMapConfig vlconf)
        throws VTNException {
        if (vlconf == null) {
            Status status = VTNManagerImpl.
                argumentIsNull("VLAN map configiguration");
            throw new VTNException(status);
        }

        short vlan = vlconf.getVlan();
        VTNManagerImpl.checkVlan(vlan);

        // Create ID for this VLAN mapping.
        Node node = vlconf.getNode();
        StringBuilder idBuilder = new StringBuilder();
        if (node == null) {
            // Node is unspecified.
            idBuilder.append(NODEID_ANY);
        }
        else {
            VTNManagerImpl.checkNode(node);
            idBuilder.append(node.getType()).append('-').
                append(node.getNodeIDString());
        }

        idBuilder.append('.').append((int)vlan);

        Lock wrlock = rwLock.writeLock();
        wrlock.lock();
        try {
            // Ensure that the specified VLAN mapping is not overlapped.
            boolean needreg = true;
            for (VlanMapImpl vmap: vlanMaps.values()) {
                VlanMapConfig vlc = vmap.getVlanMapConfig();
                if (vlc.isOverlapped(vlconf)) {
                    String msg = "Already mapped to this bridge";
                    throw new VTNException(StatusCode.CONFLICT, msg);
                }
                if (vlc.getVlan() == vlan) {
                    // Already registered to the global resource manager.
                    needreg = false;
                }
            }

            if (needreg) {
                // Ensure that the specified VLAN ID is not mapped to another
                // bridge.
                IVTNResourceManager resMgr = mgr.getResourceManager();
                String anotherBridge =
                    resMgr.registerVlanMap(getContainerName(), bridgePath,
                                           vlan);
                if (anotherBridge != null) {
                    String msg = "VLAN ID " + vlan + " is mapped to " +
                        anotherBridge;
                    throw new VTNException(StatusCode.CONFLICT, msg);
                }
            }

            String id = idBuilder.toString();
            VlanMapImpl vmap = new VlanMapImpl(mgr, this, id, vlconf);
            vlanMaps.put(id, vmap);

            VlanMap vlmap = new VlanMap(id, node, vlan);
            mgr.notifyChange(bridgePath, vlmap, UpdateType.ADDED);
            if (vmap.isValid(mgr.getStateDB())) {
                updateState(mgr);
            } else {
                setState(mgr, VNodeState.DOWN);
            }
            return vlmap;
        } finally {
            wrlock.unlock();
        }
    }

    /**
     * Remove VLAN mapping from this bridge.
     *
     * @param mgr     VTN Manager serivce.
     * @param mapId   The identifier of the VLAN mapping.
     * @throws VTNException  An error occurred.
     */
    void removeVlanMap(VTNManagerImpl mgr, String mapId)
        throws VTNException {
        if (mapId == null) {
            Status status = VTNManagerImpl.argumentIsNull("Mapping ID");
            throw new VTNException(status);
        }

        Lock wrlock = rwLock.writeLock();
        wrlock.lock();
        try {
            VlanMapImpl vmap = vlanMaps.remove(mapId);
            if (vmap == null) {
                Status status = vlanMapNotFound(mapId);
                throw new VTNException(status);
            }

            VlanMapConfig vlconf = vmap.getVlanMapConfig();
            short vlan = vlconf.getVlan();
            boolean found = false;
            for (VlanMapImpl vm: vlanMaps.values()) {
                VlanMapConfig vlc = vm.getVlanMapConfig();
                if (vlc.getVlan() == vlan) {
                    // This VLAN ID is still mapped to this bridge.
                    found = true;
                    break;
                }
            }

            Node node = vlconf.getNode();
            if (!found) {
                // Flush MAC address table associated with this VLAN map.
                // Although this may flush entries associated with the port
                // mapping, it should never cause any problem.
                MacAddressTable table = mgr.getMacAddressTable(bridgePath);
                table.flush(node, vlan);

                // Unregister VLAN mapping.
                IVTNResourceManager resMgr = mgr.getResourceManager();
                resMgr.unregisterVlanMap(vlan);
            }
            ConcurrentMap<VTenantPath, Object> db = mgr.getStateDB();
            vmap.destroy(db);

            VlanMap vlmap = new VlanMap(mapId, node, vlan);
            mgr.notifyChange(bridgePath, vlmap, UpdateType.REMOVED);
            updateState(mgr);
        } finally {
            wrlock.unlock();
        }
    }

    /**
     * Return a list of VLAN mappings in the bridge.
     *
     * @return  A list of VLAN mapping information.
     */
    List<VlanMap> getVlanMaps() {
        ArrayList<VlanMap> list = new ArrayList<VlanMap>();
        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            for (Map.Entry<String, VlanMapImpl> entry: vlanMaps.entrySet()) {
                String id = entry.getKey();
                VlanMapImpl vmap = entry.getValue();
                VlanMapConfig vlconf = vmap.getVlanMapConfig();
                VlanMap vlmap = new VlanMap(id, vlconf.getNode(),
                                            vlconf.getVlan());
                list.add(vlmap);
            }
        } finally {
            rdlock.unlock();
        }

        return list;
     }

    /**
     * Return information about the specified VLAN mapping.
     *
     * @param mapId  The identifier of the VLAN mapping.
     * @return  VLAN mapping information associated with the given ID.
     * @throws VTNException  An error occurred.
     */
    VlanMap getVlanMap(String mapId) throws VTNException {
        if (mapId == null) {
            Status status = VTNManagerImpl.argumentIsNull("Mapping ID");
            throw new VTNException(status);
        }

        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            VlanMapImpl vmap = vlanMaps.get(mapId);
            if (vmap == null) {
                Status status = vlanMapNotFound(mapId);
                throw new VTNException(status);
            }

            VlanMapConfig vlconf = vmap.getVlanMapConfig();
            VlanMap vlmap = new VlanMap(mapId, vlconf.getNode(),
                                        vlconf.getVlan());
            return vlmap;
        } finally {
            rdlock.unlock();
        }
    }

    /**
     * Return the port mapping configuration applied to the specified virtual
     * bridge interface.
     *
     * @param mgr   VTN Manager service.
     * @param path  Path to the bridge interface.
     * @return  Port mapping information.
     *          {@code null} is returned if port mapping is not configured.
     * @throws VTNException  An error occurred.
     */
    PortMap getPortMap(VTNManagerImpl mgr, VBridgeIfPath path)
        throws VTNException {
        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            VBridgeIfImpl vif = getInterfaceImpl(path);
            return vif.getPortMap(mgr);
        } finally {
            rdlock.unlock();
        }
    }

    /**
     * Create or destroy mapping between the physical switch port and the
     * virtual bridge interface.
     *
     * @param mgr     VTN Manager service.
     * @param path    Path to the bridge interface.
     * @param pmconf  Port mapping configuration to be set.
     *                If {@code null} is specified, port mapping on the
     *                specified interface is destroyed.
     * @throws VTNException  An error occurred.
     */
    void setPortMap(VTNManagerImpl mgr, VBridgeIfPath path,
                    PortMapConfig pmconf) throws VTNException {
        // Acquire write lock to serialize port mapping change.
        Lock wrlock = rwLock.writeLock();
        wrlock.lock();
        try {
            VBridgeIfImpl vif = getInterfaceImpl(path);
            VNodeState ifState = vif.setPortMap(mgr, pmconf);
            if (ifState == VNodeState.DOWN) {
                setState(mgr, VNodeState.DOWN);
            } else {
                updateState(mgr);
            }
        } finally {
            wrlock.unlock();
        }
    }

    /**
     * Resume the virtual L2 bridge.
     *
     * <p>
     *   This method is called just after this bridge is instantiated from
     *   the configuration file.
     * </p>
     *
     * @param mgr   VTN Manager service.
     */
    void resume(VTNManagerImpl mgr) {
        VNodeState state = VNodeState.UNKNOWN;
        IVTNResourceManager resMgr = mgr.getResourceManager();
        ConcurrentMap<VTenantPath, Object> db = mgr.getStateDB();
        String containerName = getContainerName();

        Lock wrlock = rwLock.writeLock();
        wrlock.lock();
        try {
            // Resume virtual interfaces.
            for (VBridgeIfImpl vif: vInterfaces.values()) {
                VNodeState s = vif.resume(mgr, state);
                if (vif.isEnabled()) {
                    state = s;
                }
            }

            for (VlanMapImpl vmap: vlanMaps.values()) {
                // Register VLAN mappings to the global resource manager.
                VlanMapConfig vlconf = vmap.getVlanMapConfig();
                short vlan = vlconf.getVlan();
                resMgr.registerVlanMap(containerName, bridgePath, vlan);

                // Resume the VLAN mappings.
                state = vmap.resume(mgr, state);
            }

            VBridgeState bst = getBridgeState(db);
            bst.setState(state);
            db.put(bridgePath, bst);
            if (LOG.isDebugEnabled()) {
                LOG.debug("{}:{}: Resumed bridge: state={}",
                          containerName, bridgePath, state);
            }

            // Create a MAC address table for this bridge.
            initMacTableAging(mgr);
        } finally {
            wrlock.unlock();
        }
    }

    /**
     * Initialize MAC address table aging.
     *
     * @param mgr  VTN Manager service.
     */
    synchronized void initMacTableAging(VTNManagerImpl mgr) {
        int age = bridgeConfig.getAgeInterval();
        MacAddressTable table = mgr.getMacAddressTable(bridgePath);
        if (table == null) {
            mgr.addMacAddressTable(bridgePath, age);
        } else {
            table.setAgeInterval(mgr, age);
        }
    }

    /**
     * Invoked when a node is added, removed, or changed.
     *
     * @param mgr   VTN Manager service.
     * @param node  Node being updated.
     * @param type  Type of update.
     */
    void notifyNode(VTNManagerImpl mgr, Node node, UpdateType type) {
        VNodeState state = VNodeState.UNKNOWN;
        ConcurrentMap<VTenantPath, Object> db = mgr.getStateDB();

        Lock wrlock = rwLock.writeLock();
        wrlock.lock();
        try {
            boolean doflush = false;

            for (VBridgeIfImpl vif: vInterfaces.values()) {
                if (!doflush && type == UpdateType.REMOVED) {
                    PortMapConfig pmconf = vif.getPortMapConfig();
                    doflush = (pmconf != null &&
                               pmconf.getNode().equals(node));
                }

                VNodeState s = vif.notifyNode(mgr, db, state, node, type);
                if (vif.isEnabled()) {
                    if (LOG.isTraceEnabled()) {
                        LOG.trace("{}:{}: notifyNode(if): {} -> {}",
                                  getContainerName(), bridgePath, state, s);
                    }
                    state = s;
                }
            }

            for (VlanMapImpl vmap: vlanMaps.values()) {
                if (!doflush && type == UpdateType.REMOVED) {
                    VlanMapConfig vlconf = vmap.getVlanMapConfig();
                    Node vmnode = vlconf.getNode();
                    doflush = (vmnode == null || vmnode.equals(node));
                }

                VNodeState s = vmap.notifyNode(mgr, db, state, node, type);
                if (LOG.isTraceEnabled()) {
                    LOG.trace("{}:{}: notifyNode(vmap): {} -> {}",
                              getContainerName(), bridgePath, state, s);
                }
                state = s;
            }

            setState(mgr, state);

            if (doflush) {
                // Flush MAC address table entries associated with the
                // removed node.
                MacAddressTable table = mgr.getMacAddressTable(bridgePath);
                table.flush(node);
            }
        } finally {
            wrlock.unlock();
        }
    }

    /**
     * This method is called when some properties of a node connector are
     * added/deleted/changed.
     *
     * @param mgr   VTN Manager service.
     * @param nc    Node connector being updated.
     * @param type  Type of update.
     */
    void notifyNodeConnector(VTNManagerImpl mgr, NodeConnector nc,
                             UpdateType type) {
        VNodeState state = VNodeState.UNKNOWN;
        ConcurrentMap<VTenantPath, Object> db = mgr.getStateDB();

        VNodeState pstate;
        ISwitchManager swMgr = mgr.getSwitchManager();
        if (type == UpdateType.REMOVED) {
            pstate = VNodeState.UNKNOWN;
        } else {
            // Determine whether the port is up or not.
            pstate = (swMgr.isNodeConnectorEnabled(nc))
                ? VNodeState.UP : VNodeState.DOWN;
        }

        Lock wrlock = rwLock.writeLock();
        wrlock.lock();
        try {
            boolean doflush = false;

            for (VBridgeIfImpl vif: vInterfaces.values()) {
                if (!doflush && pstate != VNodeState.UP) {
                    NodeConnector mapped = vif.getMappedPort(mgr);
                    doflush = nc.equals(mapped);
                }

                VNodeState s = vif.notifyNodeConnector(mgr, db, state, nc,
                                                       pstate, type);
                if (vif.isEnabled()) {
                    if (LOG.isTraceEnabled()) {
                        LOG.trace("{}:{}: notifyNodeConnector(if): {} -> {}",
                                  getContainerName(), bridgePath, state, s);
                    }
                    state = s;
                }
            }

            for (VlanMapImpl vmap: vlanMaps.values()) {
                if (!doflush && pstate != VNodeState.UP) {
                    VlanMapConfig vlconf = vmap.getVlanMapConfig();
                    Node vmnode = vlconf.getNode();
                    doflush = (vmnode == null || vmnode.equals(nc.getNode()));
                }

                VNodeState s = vmap.notifyNodeConnector(mgr, db, state, nc,
                                                        pstate, type);
                if (LOG.isTraceEnabled()) {
                    LOG.trace("{}:{}: notifyNodeConnector(vmap): {} -> {}",
                              getContainerName(), bridgePath, state, s);
                }
                state = s;
            }
            setState(mgr, state);

            if (doflush) {
                // Flush MAC address table entries associated with the given
                // node connector.
                MacAddressTable table = mgr.getMacAddressTable(bridgePath);
                table.flush(nc);
            }
        } finally {
            wrlock.unlock();
        }
    }

    /**
     * This method is called when topology graph is changed.
     *
     * @param mgr       VTN Manager service.
     * @param topoList  List of topoedgeupdates Each topoedgeupdate includes
     *                  edge, its Properties (BandWidth and/or Latency etc)
     *                  and update type.
     */
    void edgeUpdate(VTNManagerImpl mgr, List<TopoEdgeUpdate> topoList) {
        ConcurrentMap<VTenantPath, Object> db = mgr.getStateDB();

        Lock wrlock = rwLock.readLock();
        VNodeState state = VNodeState.UNKNOWN;
        wrlock.lock();
        try {
            for (VBridgeIfImpl vif: vInterfaces.values()) {
                VNodeState s = vif.edgeUpdate(mgr, db, state, topoList);
                if (vif.isEnabled()) {
                    if (LOG.isTraceEnabled()) {
                        LOG.trace("{}:{}: edgeUpdate(if): {} -> {}",
                                  getContainerName(), bridgePath, state, s);
                    }
                    state = s;
                }
            }

            for (VlanMapImpl vmap: vlanMaps.values()) {
                VNodeState s = vmap.edgeUpdate(mgr, db, state, topoList);
                if (LOG.isTraceEnabled()) {
                    LOG.trace("{}:{}: edgeUpdate(vmap): {} -> {}",
                              getContainerName(), bridgePath, state, s);
                }
                state = s;
            }
            setState(mgr, state);
        } finally {
            wrlock.unlock();
        }
    }

    /**
     * Notify the listener of current configuration.
     *
     * @param mgr       VTN Manager service.
     * @param listener  VTN manager listener service.
     */
    void notifyConfiguration(VTNManagerImpl mgr, IVTNManagerAware listener) {
        UpdateType type = UpdateType.ADDED;
        VBridge vbridge = getVBridge(mgr);
        mgr.notifyChange(listener, bridgePath, vbridge, type);

        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            for (VBridgeIfImpl vif: vInterfaces.values()) {
                VInterface viface = vif.getVInterface(mgr);
                VBridgeIfPath ipath = (VBridgeIfPath)vif.getPath();
                mgr.notifyChange(listener, ipath, viface, type);
            }

            for (Map.Entry<String, VlanMapImpl> entry: vlanMaps.entrySet()) {
                String id = entry.getKey();
                VlanMapImpl vmap = entry.getValue();
                VlanMapConfig vlconf = vmap.getVlanMapConfig();
                VlanMap vlmap = new VlanMap(id, vlconf.getNode(),
                                            vlconf.getVlan());
                mgr.notifyChange(listener, bridgePath, vlmap, type);
            }
        } finally {
            rdlock.unlock();
        }
    }

    /**
     * Initiate the discovery of a host base on its IP address.
     *
     * @param mgr   VTN manager service.
     * @param pctx  The context of the ARP packet to send.
     */
    void findHost(VTNManagerImpl mgr, PacketContext pctx) {
        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            // Flood the specified ARP request.
            flood(mgr, pctx);
        } finally {
            rdlock.unlock();
        }
    }

    /**
     * Send a unicast ARP request to the specified host.
     *
     * @param mgr   VTN manager service.
     * @param type  Mapping type to be tested.
     * @param pctx  The context of the ARP packet to send.
     * @return  A {@code Boolean} object is returned if the specified host
     *          belongs to this bridge. If a ARP request was actually sent to
     *          the network, {@code Boolean.TRUE} is returned.
     *          {@code null} is returned if the specified host does not
     *          belong to this bridge.
     */
    Boolean probeHost(VTNManagerImpl mgr, MapType type, PacketContext pctx) {
        NodeConnector nc = pctx.getOutgoingNodeConnector();
        assert nc != null;
        short vlan = pctx.getVlan();

        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            VBridgeNode bnode = match(mgr, type, nc, vlan);
            if (bnode == null) {
                return null;
            }

            if (!bnode.isEnabled()) {
                if (LOG.isDebugEnabled()) {
                    LOG.debug("{}:{}: " +
                              "Don't send ARP request to disabled network: {}",
                              getContainerName(), bnode.getPath(),
                              pctx.getDescription(nc));
                }
                return Boolean.FALSE;
            }

            if (LOG.isTraceEnabled()) {
                LOG.trace("{}:{}: Send ARP request for probing: {}",
                          getContainerName(), bridgePath,
                          pctx.getDescription(nc));
            }
            mgr.transmit(nc, pctx.getFrame());
        } finally {
            rdlock.unlock();
        }

        return Boolean.TRUE;
    }

    /**
     * Handler for receiving the packet.
     *
     * @param mgr   VTN manager service.
     * @param type  Mapping type to be tested.
     * @param pctx  The context of the received packet.
     * @return  A {@code PacketResult} which indicates the result of handler.
     */
    PacketResult receive(VTNManagerImpl mgr, MapType type, PacketContext pctx) {
        NodeConnector incoming = pctx.getIncomingNodeConnector();
        short vlan = pctx.getVlan();

        // Writer lock is required because this method may change the state
        // of the bridge.
        Lock wrlock = rwLock.writeLock();
        wrlock.lock();
        try {
            VBridgeNode bnode = match(mgr, type, incoming, vlan);
            if (bnode == null) {
                return PacketResult.IGNORED;
            }

            if (bnode.isEnabled()) {
                handlePacket(mgr, pctx);
                // TODO:
                // Remove flow entries associated with obsolete MAC table
                // entries.
            } else if (LOG.isDebugEnabled()) {
                LOG.debug("{}:{}: " +
                          "Ignore packet received from disabled network: {}",
                          getContainerName(), bnode.getPath(),
                          pctx.getDescription(incoming));
            }
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
    void recalculateDone(VTNManagerImpl mgr) {
        // Writer lock is required because this method may change the state
        // of the bridge.
        Lock wrlock = rwLock.writeLock();
        wrlock.lock();
        try {
            ConcurrentMap<VTenantPath, Object> db = mgr.getStateDB();
            VBridgeState bst = getBridgeState(db);
            Set<ObjectPair<Node, Node>> faulted = bst.getFaultedPaths();
            if (faulted.isEmpty()) {
                return;
            }

            IRouting routing = mgr.getRouting();
            boolean changed = false;
            for (Iterator<ObjectPair<Node, Node>> it = faulted.iterator();
                 it.hasNext();) {
                ObjectPair<Node, Node> npath = it.next();
                Node snode = npath.getLeft();
                Node dnode = npath.getRight();
                Path path = routing.getRoute(snode, dnode);
                if (path != null) {
                    LOG.info("{}:{} Path fault resolved: {} -> {}",
                             getContainerName(), bridgePath, snode, dnode);
                    it.remove();
                    changed = true;
                }
            }

            if (changed) {
                db.put(bridgePath, bst);
                if (faulted.isEmpty()) {
                    updateState(mgr, true);
                } else {
                    VBridge vbridge =
                        new VBridge(getName(), VNodeState.DOWN, faulted.size(),
                                    getVBridgeConfig());
                    mgr.notifyChange(bridgePath, vbridge, UpdateType.CHANGED);
                }
            }
        } finally {
            wrlock.unlock();
        }
    }

    /**
     * Destroy the virtual L2 bridge.
     *
     * @param mgr         VTN manager service.
     * @param vtnDestroy  {@code true} is specified if the parent VTN is being
     *                    destroyed.
     */
    void destroy(VTNManagerImpl mgr, boolean vtnDestroy) {
        VBridge vbridge = getVBridge(mgr);
        IVTNResourceManager resMgr = mgr.getResourceManager();
        ConcurrentMap<VTenantPath, Object> db = mgr.getStateDB();

        Lock wrlock = rwLock.writeLock();
        wrlock.lock();
        try {
            // Destroy all VLAN mappings.
            TreeSet<Short> vlanIDs = new TreeSet<Short>();
            for (Iterator<Map.Entry<String, VlanMapImpl>> it =
                     vlanMaps.entrySet().iterator(); it.hasNext();) {
                Map.Entry<String, VlanMapImpl> entry = it.next();
                String mapId = entry.getKey();
                VlanMapImpl vmap = entry.getValue();
                VlanMapConfig vlconf = vmap.getVlanMapConfig();
                short vlan = vlconf.getVlan();
                Short vid = new Short(vlan);
                if (!vlanIDs.contains(vid)) {
                    resMgr.unregisterVlanMap(vlan);
                    vlanIDs.add(vid);
                }
                vmap.destroy(db);

                VlanMap vlmap = new VlanMap(mapId, vlconf.getNode(), vlan);
                mgr.notifyChange(bridgePath, vlmap, UpdateType.REMOVED);
                it.remove();
            }
            vlanIDs = null;

            // Destroy MAC address table.
            mgr.removeMacAddressTable(bridgePath, !vtnDestroy);

            // Destroy all interfaces.
            for (Iterator<VBridgeIfImpl> it = vInterfaces.values().iterator();
                 it.hasNext();) {
                VBridgeIfImpl vif = it.next();
                vif.destroy(mgr);
                it.remove();
            }

            // Unlink parent for GC.
            parent = null;
        } finally {
            wrlock.unlock();
        }

        db.remove(bridgePath);
        mgr.notifyChange(bridgePath, vbridge, UpdateType.REMOVED);
    }

    /**
     * Return a runtime state object for the virtual bridge.
     *
     * @param mgr  VTN Manager service.
     * @return  A runtume state object.
     */
    private VBridgeState getBridgeState(VTNManagerImpl mgr) {
        ConcurrentMap<VTenantPath, Object> db = mgr.getStateDB();
        return getBridgeState(db);
    }

    /**
     * Return a runtime state object for the virtual bridge.
     *
     * @param db  Runtime state DB.
     * @return  A runtume state object.
     */
    private VBridgeState getBridgeState(ConcurrentMap<VTenantPath, Object> db) {
        VBridgeState bst = (VBridgeState)db.get(bridgePath);
        if (bst == null) {
            bst = new VBridgeState(VNodeState.UNKNOWN);
        }

        return bst;
    }

    /**
     * Determine whether the given object is identical to this object.
     *
     * @param o  An object to be compared.
     * @return   {@code true} if identical. Otherwise {@code false}.
     */
    @Override
    public boolean equals(Object o) {
        if (!(o instanceof VBridgeImpl)) {
            return false;
        }

        VBridgeImpl vbr = (VBridgeImpl)o;
        if (!bridgePath.equals(vbr.bridgePath)) {
            return false;
        }

        VBridgeConfig bconf = getVBridgeConfig();
        VBridgeConfig otherBconf = vbr.getVBridgeConfig();
        if (!bconf.equals(otherBconf)) {
            return false;
        }

        // Compare copied maps in order to avoid deadlock.
        TreeMap<String, VBridgeIfImpl> ifs = getInterfaceMap();
        TreeMap<String, VBridgeIfImpl> otherIfs = vbr.getInterfaceMap();
        if (!ifs.equals(otherIfs)) {
            return false;
        }

        TreeMap<String, VlanMapImpl> vmaps = getVlanMappings();
        TreeMap<String, VlanMapImpl> otherVmaps = vbr.getVlanMappings();

        return vmaps.equals(otherVmaps);
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        int h = bridgePath.hashCode() + getVBridgeConfig().hashCode();

        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            h += vInterfaces.hashCode() + vlanMaps.hashCode();
        } finally {
            rdlock.unlock();
        }

        return h * 17;
    }

    /**
     * Merge the given vBridge configuration to the current configuration.
     *
     * <p>
     *   If at least one field in {@code bconf} keeps a valid value, this
     *   method creates a shallow copy of the current configuration, and set
     *   valid values in {@code bconf} to the copy.
     * </p>
     *
     * @param bconf  Configuration to be merged.
     * @return  A merged {@code VBridgeConfig} object.
     */
    private synchronized VBridgeConfig merge(VBridgeConfig bconf) {
        String desc = bconf.getDescription();
        int age = bconf.getAgeInterval();
        if (desc == null) {
            bconf = (age < 0)
                ? bridgeConfig
                : new VBridgeConfig(bridgeConfig.getDescription(), age);
        } else if (age < 0) {
            bconf = new VBridgeConfig(desc, bridgeConfig.getAgeInterval());
        }

        return bconf;
    }

    /**
     * Resolve undefined attributes in the specified bridge configuration.
     *
     * @param bconf  The bridge configuration.
     * @return       {@code VBridgeConfig} to be applied.
     */
    private VBridgeConfig resolve(VBridgeConfig bconf) {
        int age = bconf.getAgeInterval();
        if (age < 0) {
            bconf = new VBridgeConfig(bconf.getDescription(),
                                      DEFAULT_AGE_INTERVAL);
        }

        return bconf;
    }

    /**
     * Ensure that the specified bridge configuration is valid.
     *
     * @param bconf  The bridge configuration to be tested.
     * @throws VTNException  An error occurred.
     */
    private void checkConfig(VBridgeConfig bconf) throws VTNException {
        int ival = bconf.getAgeInterval();
        if (ival < MIN_AGE_INTERVAL || ival > MAX_AGE_INTERVAL) {
            throw new VTNException(StatusCode.BADREQUEST,
                                   "Invalid MAC address aging interval");
        }
    }

    /**
     * Return a shallow copy of the virtual interface map.
     *
     * @return  Pairs of interface name and interface instance.
     */
    private TreeMap<String, VBridgeIfImpl> getInterfaceMap() {
        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            return new TreeMap<String, VBridgeIfImpl>(vInterfaces);
        } finally {
            rdlock.unlock();
        }
    }

    /**
     * Return a shallow copy of the VLAN mappings.
     *
     * @return Pairs of VLAN mapping ID and VLAN mapping instance.
     */
    private TreeMap<String, VlanMapImpl> getVlanMappings() {
        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            return new TreeMap<String, VlanMapImpl>(vlanMaps);
        } finally {
            rdlock.unlock();
        }
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
    private void readObject(ObjectInputStream in)
        throws IOException, ClassNotFoundException {
        in.defaultReadObject();

        // Reset the lock.
        rwLock = new ReentrantReadWriteLock();
    }

    /**
     * Return a failure status that indicates the specified interface does not
     * exist.
     *
     * @param ifName  The name of the interface.
     * @return  A failure status.
     */
    private Status interfaceNotFound(String ifName) {
        String msg = ifName + ": Interface does not exist";
        return new Status(StatusCode.NOTFOUND, msg);
    }

    /**
     * Return a failure status that indicates the specified VLAN mapping does
     * not exist.
     *
     * @param id      The identifier of the VLAN mapping.
     * @return  A failure status.
     */
    private Status vlanMapNotFound(String id) {
        String msg = id + ": VLAN mapping does not exist";
        return new Status(StatusCode.NOTFOUND, msg);
    }

    /**
     * Return the virtual interface instance associated with the given name.
     *
     * <p>
     *   This method must be called with the bridge lock.
     * </p>
     *
     * @param path  Path to the interface.
     * @return  Virtual interface instance is returned.
     * @throws VTNException  An error occurred.
     */
    private VBridgeIfImpl getInterfaceImpl(VBridgeIfPath path)
        throws VTNException {
        String ifName = path.getInterfaceName();
        if (ifName == null) {
            Status status = VTNManagerImpl.argumentIsNull("Interface name");
            throw new VTNException(status);
        }

        VBridgeIfImpl vif = vInterfaces.get(ifName);
        if (vif == null) {
            Status status = interfaceNotFound(ifName);
            throw new VTNException(status);
        }

        return vif;
    }

    /**
     * Set state of the bridge.
     *
     * <p>
     *   Note that this method must be called with holding the bridge write
     *   lock.
     * </p>
     *
     * @param mgr    VTN Manager service.
     * @param state  New bridge state.
     */
    private void setState(VTNManagerImpl mgr, VNodeState state) {
        setState(mgr, state, false);
    }

    /**
     * Set state of the bridge.
     *
     * <p>
     *   Note that this method must be called with holding the bridge write
     *   lock.
     * </p>
     *
     * @param mgr      VTN Manager service.
     * @param state    New bridge state.
     * @param changed  {@code true} means a bridge changed notification is
     *                 required.
     */
    private void setState(VTNManagerImpl mgr, VNodeState state,
                          boolean changed) {
        ConcurrentMap<VTenantPath, Object> db = mgr.getStateDB();
        VBridgeState bst = getBridgeState(db);
        int faults = bst.getFaultedPaths().size();
        if (faults != 0) {
            state = VNodeState.DOWN;
        }
        if (bst.setState(state)) {
            db.put(bridgePath, bst);
            changed = true;
        }
        if (changed) {
            VBridge vbridge =
                new VBridge(getName(), state, faults, getVBridgeConfig());
            mgr.notifyChange(bridgePath, vbridge, UpdateType.CHANGED);
        }
    }

    /**
     * Add a node path to the set of faulted node paths.
     *
     * <p>
     *   Note that this method must be called with holding the bridge write
     *   lock.
     * </p>
     *
     * @param mgr   VTN Manager service.
     * @param path  A node path.
     */
    private void addFaultedPath(VTNManagerImpl mgr,
                                ObjectPair<Node, Node> path) {
        ConcurrentMap<VTenantPath, Object> db = mgr.getStateDB();
        VBridgeState bst = getBridgeState(db);
        Set<ObjectPair<Node, Node>> faulted = bst.getFaultedPaths();
        boolean added = faulted.add(path);
        boolean changed = bst.setState(VNodeState.DOWN);
        if (added || changed) {
            db.put(bridgePath, bst);
            VBridge vbridge =
                new VBridge(getName(), VNodeState.DOWN, faulted.size(),
                            getVBridgeConfig());
            mgr.notifyChange(bridgePath, vbridge, UpdateType.CHANGED);
        }
    }

    /**
     * Scan interfaces and VLAN mappings, and determine current state of the
     * virtual bridge.
     *
     * <p>
     *   Note that this method must be called with holding the bridge write
     *   lock.
     * </p>
     *
     * @param mgr   VTN Manager service.
     */
    private void updateState(VTNManagerImpl mgr) {
        updateState(mgr, false);
    }

    /**
     * Scan interfaces and VLAN mappings, and determine current state of the
     * virtual bridge.
     *
     * <p>
     *   Note that this method must be called with holding the bridge write
     *   lock.
     * </p>
     *
     * @param mgr      VTN Manager service.
     * @param changed  {@code true} means a bridge changed notification is
     *                 required.
     */
    private void updateState(VTNManagerImpl mgr, boolean changed) {
        ConcurrentMap<VTenantPath, Object> db = mgr.getStateDB();
        VNodeState state = VNodeState.UNKNOWN;

        // Scan virtual interfaces.
        for (VBridgeIfImpl vif: vInterfaces.values()) {
            if (vif.isEnabled()) {
                state = vif.getBridgeState(db, state);
                if (state == VNodeState.DOWN) {
                    setState(mgr, state, changed);
                    return;
                }
            }
        }

        // Scan VLAN mappings.
        for (VlanMapImpl vmap: vlanMaps.values()) {
            state = vmap.getBridgeState(db, state);
            if (state == VNodeState.DOWN) {
                break;
            }
        }

        setState(mgr, state, changed);
    }

    /**
     * Return information about the bridge.
     *
     * @param mgr    VTN Manager service.
     * @param name   The name of the bridge.
     * @param bconf  Bridge configuration.
     * @return  Information about the bridge.
     */
    private VBridge getVBridge(VTNManagerImpl mgr, String name,
                               VBridgeConfig bconf) {
        VBridgeState bst = getBridgeState(mgr);
        Set<ObjectPair<Node, Node>> faulted = bst.getFaultedPaths();
        return new VBridge(name, bst.getState(), faulted.size(), bconf);
    }

    /**
     * Handle the received packet.
     *
     * <p>
     *   Note that this method must be called with holding the bridge lock.
     * </p>
     *
     * @param mgr   VTN Manager service.
     * @param pctx  The context of the received packet.
     */
    private void handlePacket(VTNManagerImpl mgr, PacketContext pctx) {
        // Learn the source MAC address if needed.
        MacAddressTable table = mgr.getMacAddressTable(bridgePath);
        table.add(mgr, pctx);

        ISwitchManager swMgr = mgr.getSwitchManager();
        byte[] ctlrMac = swMgr.getControllerMAC();
        byte[] dst = pctx.getDestinationAddress();
        if (Arrays.equals(ctlrMac, dst)) {
            if (LOG.isTraceEnabled()) {
                NodeConnector incoming = pctx.getIncomingNodeConnector();
                LOG.trace("{}:{}: Ignore packet sent to controller: {}",
                          getContainerName(), bridgePath,
                          pctx.getDescription(incoming));
            }
            return;
        }

        if (!NetUtils.isUnicastMACAddr(dst)) {
            // Flood the non-unicast packet.
            flood(mgr, pctx);
            return;
        }

        // Determine whether the destination address is known or not.
        MacTableEntry tent = table.get(pctx);
        if (tent == null) {
            // Flood the received packet.
            flood(mgr, pctx);
            return;
        }

        // Ensure that the outgoing network is mapped to this bridge.
        NodeConnector outgoing = tent.getPort();
        short outVlan = tent.getVlan();
        VBridgeNode bnode = match(mgr, MapType.ALL, outgoing, outVlan);
        if (bnode == null) {
            LOG.warn("{}:{} Unexpected MAC address entry: {}",
                     getContainerName(), bridgePath, tent);
            Long key = MacAddressTable.getTableKey(dst);
            pctx.addObsoleteEntry(key, tent);
            table.remove(pctx);
            flood(mgr, pctx);
            return;
        }

        if (!bnode.isEnabled()) {
            if (LOG.isDebugEnabled()) {
                LOG.debug("{}:{}: Drop packet due to disabled network: {}",
                          getContainerName(), bnode.getPath(),
                          pctx.getDescription(outgoing));
            }
            return;
        }

        if (!swMgr.isNodeConnectorEnabled(outgoing)) {
            if (LOG.isWarnEnabled()) {
                LOG.warn("{}:{}: Drop packet because outgoing port is down: " +
                         "{}", getContainerName(), bridgePath,
                         pctx.getDescription(outgoing));
            }
            return;
        }

        // Ensure that the destination host is reachable.
        NodeConnector incoming = pctx.getIncomingNodeConnector();
        Node snode = incoming.getNode();
        Node dnode = outgoing.getNode();
        if (!snode.equals(dnode)) {
            IRouting routing = mgr.getRouting();
            Path path = routing.getRoute(snode, dnode);
            if (path == null) {
                LOG.error("{}:{} Path fault: {} -> {}",
                          getContainerName(), bridgePath, snode, dnode);
                ObjectPair<Node, Node> npath =
                    new ObjectPair<Node, Node>(snode, dnode);
                addFaultedPath(mgr, npath);
                return;
            }
        }

        Ethernet frame = pctx.createFrame(outVlan);
        if (LOG.isTraceEnabled()) {
            LOG.trace("{}:{}: Forward packet to known host: {}",
                      getContainerName(), bridgePath,
                      pctx.getDescription(frame, outgoing, outVlan));
        }
        mgr.transmit(outgoing, frame);

        // TODO: Install flow entries.
    }

    /**
     * Determine whether the network specified by the switch port and the
     * VLAN ID is mapped to this bridge or not.
     *
     * <p>
     *   Note that this method must be called with holding the bridge lock.
     * </p>
     *
     * @param mgr   VTN Manager service.
     * @param type  Mapping type to be tested.
     * @param nc    A node connector where the packet was received.
     * @param vlan  VLAN ID in the received packet.
     * @return  A {@code VBridgeNode} is returned if the network specified
     *          by {@code nc} and {@code vlan} is mapped to this bridge.
     *          Otherwise {@code null} is returned.
     */
    private VBridgeNode match(VTNManagerImpl mgr, MapType type,
                              NodeConnector nc, short vlan) {
        if (type.match(MapType.PORT)) {
            // Check whether the packet is mapped by port mappings or not.
            for (VBridgeIfImpl vif: vInterfaces.values()) {
                if (vif.match(mgr, nc, vlan)) {
                    return vif;
                }
            }
        }

        if (type.match(MapType.VLAN)) {
            // Check whether the packet is mapped by VLAN mappings or not.
            Node node = nc.getNode();
            for (VlanMapImpl vmap: vlanMaps.values()) {
                VlanMapConfig vlconf = vmap.getVlanMapConfig();
                Node vmnode = vlconf.getNode();
                if ((vmnode == null || vmnode.equals(node)) &&
                    vlconf.getVlan() == vlan) {
                    return vmap;
                }
            }
        }

        return null;
    }

    /**
     * Flood the specified packet to this bridge.
     *
     * <p>
     *   Note that this method must be called with holding the bridge lock.
     * </p>
     *
     * @param mgr   VTN manager service.
     * @param pctx  The context of the packet.
     */
    private void flood(VTNManagerImpl mgr, PacketContext pctx) {
        // Don't send the packet to the incoming network.
        HashSet<PortVlan> sent = new HashSet<PortVlan>();
        PortVlan innw = pctx.getIncomingNetwork();
        if (innw != null) {
            sent.add(innw);
        }

        // Forward packet to the network established by the port mapping.
        for (VBridgeIfImpl vif: vInterfaces.values()) {
            vif.transmit(mgr, pctx, sent);
        }

        // Forward packet to the network established by the VLAN mapping.
        for (VlanMapImpl vmap: vlanMaps.values()) {
            vmap.transmit(mgr, pctx, sent);
        }
    }
}
