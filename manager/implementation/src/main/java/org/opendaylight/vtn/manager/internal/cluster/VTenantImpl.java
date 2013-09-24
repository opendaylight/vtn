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
import java.io.File;
import java.util.Iterator;
import java.util.Map;
import java.util.TreeMap;
import java.util.List;
import java.util.ArrayList;
import java.util.Timer;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantReadWriteLock;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import org.opendaylight.vtn.manager.IVTNManagerAware;
import org.opendaylight.vtn.manager.MacAddressEntry;
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
import org.opendaylight.vtn.manager.VTenant;
import org.opendaylight.vtn.manager.VTenantConfig;
import org.opendaylight.vtn.manager.VTenantPath;
import org.opendaylight.vtn.manager.VlanMap;
import org.opendaylight.vtn.manager.VlanMapConfig;
import org.opendaylight.vtn.manager.internal.IVTNResourceManager;
import org.opendaylight.vtn.manager.internal.MacAddressTable;
import org.opendaylight.vtn.manager.internal.MapType;
import org.opendaylight.vtn.manager.internal.PacketContext;
import org.opendaylight.vtn.manager.internal.VTNFlowDatabase;
import org.opendaylight.vtn.manager.internal.VTNManagerImpl;

import org.opendaylight.controller.sal.core.Node;
import org.opendaylight.controller.sal.core.NodeConnector;
import org.opendaylight.controller.sal.core.UpdateType;
import org.opendaylight.controller.sal.packet.PacketResult;
import org.opendaylight.controller.sal.packet.address.DataLinkAddress;
import org.opendaylight.controller.sal.topology.TopoEdgeUpdate;
import org.opendaylight.controller.sal.utils.GlobalConstants;
import org.opendaylight.controller.sal.utils.ObjectWriter;
import org.opendaylight.controller.sal.utils.Status;
import org.opendaylight.controller.sal.utils.StatusCode;

/**
 * Implementation of virtual tenant.
 *
 * <p>
 *   Although this class is public to other packages, this class does not
 *   provide any API. Applications other than VTN Manager must not use this
 *   class.
 * </p>
 */
public final class VTenantImpl implements Serializable {
    private static final long serialVersionUID = 1119058818818252108L;

    /**
     * Logger instance.
     */
    private static final Logger  LOG =
        LoggerFactory.getLogger(VTenantImpl.class);

    /**
     * Maximum value of flow timeout value.
     */
    private static final int  MAX_FLOW_TIMEOUT = 65535;

    /**
     * Default value of {@code idle_timeout} of flow entries.
     */
    private static final int  DEFAULT_IDLE_TIMEOUT = 300;

    /**
     * Default value of {@code hard_timeout} of flow entries.
     */
    private static final int  DEFAULT_HARD_TIMEOUT = 0;    // Infinite

    /**
     * The name of the container to which this tenant belongs.
     */
    private final String  containerName;

    /**
     * Tenant name.
     */
    private final String  tenantName;

    /**
     * Configuration for the tenant.
     */
    private VTenantConfig  tenantConfig;

    /**
     * Virtual layer 2 bridges.
     */
    private final Map<String, VBridgeImpl> vBridges =
        new TreeMap<String, VBridgeImpl>();

    /**
     * Read write lock to synchronize per-tenant resources.
     */
    private transient ReentrantReadWriteLock  rwLock =
        new ReentrantReadWriteLock();

    /**
     * Return a path to the configuration file associated with the specified
     * virtual tenant.
     *
     * @param containerName  The name of the container to which the tenant
     *                       belongs.
     * @param tenantName     The name of the tenant.
     * @return  A path to the tenant configuration file.
     */
    public static String getConfigFilePath(String containerName,
                                           String tenantName) {
        String root = GlobalConstants.STARTUPHOME.toString();
        return root + "vtn-" + containerName + "-" + tenantName + ".conf";
    }

    /**
     * Remove the configuration file for the specified virtual tenant.
     * @param containerName  The name of the container to which the tenant
     *                       belongs.
     * @param tenantName     The name of the tenant.
     */
    public static void deleteConfigFile(String containerName,
                                        String tenantName) {
        String path = VTenantImpl.getConfigFilePath(containerName, tenantName);
        File file = new File(path);
        file.delete();
    }

    /**
     * Construct a virtual tenant instance.
     *
     * @param containerName  The name of the container to which the tenant
     *                       belongs.
     * @param tenantName     The name of the tenant.
     * @param tconf          Configuration for the tenant.
     * @throws VTNException  An error occurred.
     */
    public VTenantImpl(String containerName, String tenantName,
                       VTenantConfig tconf) throws VTNException {
        tconf = resolve(tconf);
        checkConfig(tconf);
        this.containerName = containerName;
        this.tenantName = tenantName;
        this.tenantConfig = tconf;
    }

    /**
     * Return the name of the container to which the tenant belongs.
     *
     * @return  The name of the container.
     */
    public String getContainerName() {
        return containerName;
    }

    /**
     * Return the name of the tenant.
     *
     * @return  The name of the tenant.
     */
    public String getName() {
        return tenantName;
    }

    /**
     * Return information about the tenant.
     *
     * @return  Information about the tenant.
     */
    public synchronized VTenant getVTenant() {
        return new VTenant(tenantName, tenantConfig);
    }

    /**
     * Return tenant configuration.
     *
     * @return  Configuration for the tenant.
     */
    public synchronized VTenantConfig getVTenantConfig() {
        return tenantConfig;
    }

    /**
     * Set tenant configuration.
     *
     * @param mgr    VTN Manager service.
     * @param path   Path to the virtual tenant.
     * @param tconf  Tenant configuration
     * @param all    If {@code true} is specified, all attributes of the
     *               tenant are modified. In this case, {@code null} in
     *               {@code tconf} is interpreted as default value.
     *               If {@code false} is specified, an attribute is not
     *               modified if its value in {@code tconf} is {@code null}.
     * @return  {@code true} if the configuration is actually changed.
     *          Otherwise {@code false}.
     * @throws VTNException  An error occurred.
     */
    public synchronized boolean setVTenantConfig(VTNManagerImpl mgr,
                                                 VTenantPath path,
                                                 VTenantConfig tconf,
                                                 boolean all)
        throws VTNException {
        tconf = (all) ? resolve(tconf) : merge(tconf);
        if (tconf.equals(tenantConfig)) {
            return false;
        }

        checkConfig(tconf);
        tenantConfig = tconf;
        VTenant vtenant = new VTenant(tenantName, tconf);
        mgr.enqueueEvent(path, vtenant, UpdateType.CHANGED);
        return true;
    }

    /**
     * Add a new virtual L2 bridge to this tenant.
     *
     * @param mgr    VTN Manager service.
     * @param path   Path to the bridge.
     * @param bconf  Bridge configuration.
     * @throws VTNException  An error occurred.
     */
    public void addBridge(VTNManagerImpl mgr, VBridgePath path,
                          VBridgeConfig bconf) throws VTNException {
        // Ensure the given bridge name is valid.
        String bridgeName = path.getBridgeName();
        VTNManagerImpl.checkName("Bridge", bridgeName);

        if (bconf == null) {
            Status status = VTNManagerImpl.
                argumentIsNull("Bridge configuration");
            throw new VTNException(status);
        }

        VBridgeImpl vbr = new VBridgeImpl(this, bridgeName, bconf);
        Lock wrlock = rwLock.writeLock();
        wrlock.lock();
        try {
            VBridgeImpl old = vBridges.put(bridgeName, vbr);
            if (old != null) {
                vBridges.put(bridgeName, old);
                String msg = bridgeName + ": Bridge name already exists";
                throw new VTNException(StatusCode.CONFLICT, msg);
            }

            VBridge vbridge = vbr.getVBridge(mgr);
            mgr.enqueueEvent(path, vbridge, UpdateType.ADDED);

            // Create a MAC address table for this bridge.
            vbr.initMacTableAging(mgr);
        } finally {
            wrlock.unlock();
        }
    }

    /**
     * Change configuration of existing virtual L2 bridge.
     *
     * @param mgr    VTN Manager service.
     * @param path   Path to the virtual bridge.
     * @param bconf  Bridge configuration.
     * @param all    If {@code true} is specified, all attributes of the
     *               bridge are modified. In this case, {@code null} in
     *               {@code bconf} is interpreted as default value.
     *               If {@code false} is specified, an attribute is not
     *               modified if its value in {@code bconf} is {@code null}.
     * @return  {@code true} is returned only if the bridge configuration is
     *          actually changed.
     * @throws VTNException  An error occurred.
     */
    public boolean modifyBridge(VTNManagerImpl mgr, VBridgePath path,
                                VBridgeConfig bconf, boolean all)
        throws VTNException {
        if (bconf == null) {
            Status status = VTNManagerImpl.
                argumentIsNull("Bridge configuration");
            throw new VTNException(status);
        }

        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            VBridgeImpl vbr = getBridgeImpl(path);
            return vbr.setVBridgeConfig(mgr, bconf, all);
        } finally {
            rdlock.unlock();
        }
    }

    /**
     * Remove the specified virtual L2 bridge.
     *
     * @param mgr   VTN Manager service.
     * @param path  Path to the virtual bridge.
     * @throws VTNException  An error occurred.
     */
    public void removeBridge(VTNManagerImpl mgr, VBridgePath path)
        throws VTNException {
        Lock wrlock = rwLock.writeLock();
        wrlock.lock();
        try {
            String bridgeName = path.getBridgeName();
            if (bridgeName == null) {
                Status status = VTNManagerImpl.argumentIsNull("Bridge name");
                throw new VTNException(status);
            }

            VBridgeImpl vbr = vBridges.remove(bridgeName);
            if (vbr == null) {
                Status status = bridgeNotFound(bridgeName);
                throw new VTNException(status);
            }

            vbr.destroy(mgr, false);
        } finally {
            wrlock.unlock();
        }
    }

    /**
     * Return a list of virtual bridge information.
     *
     * @param mgr  VTN Manager service.
     * @return  A list of vbridge information.
     */
    public List<VBridge> getBridges(VTNManagerImpl mgr) {
        ArrayList<VBridge> list = new ArrayList<VBridge>();
        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            for (VBridgeImpl vbr: vBridges.values()) {
                list.add(vbr.getVBridge(mgr));
            }
            list.trimToSize();
        } finally {
            rdlock.unlock();
        }

        return list;
    }

    /**
     * Return the virtual L2 bridge information associated with the given name.
     *
     * @param mgr   VTN Manager service.
     * @param path  Path to the bridge.
     * @return  The virtual L2 bridge information associated with the given
     *          name.
     * @throws VTNException  An error occurred.
     */
    public VBridge getBridge(VTNManagerImpl mgr, VBridgePath path)
        throws VTNException {
        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            VBridgeImpl vbr = getBridgeImpl(path);
            return vbr.getVBridge(mgr);
        } finally {
            rdlock.unlock();
        }
    }

    /**
     * Add a new virtual interface to the specified L2 bridge.
     *
     * @param mgr    VTN Manager service.
     * @param path   Path to the interface to be added.
     * @param iconf  Interface configuration.
     * @throws VTNException  An error occurred.
     */
    public void addBridgeInterface(VTNManagerImpl mgr, VBridgeIfPath path,
                                   VInterfaceConfig iconf)
        throws VTNException {
        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            VBridgeImpl vbr = getBridgeImpl(path);
            vbr.addInterface(mgr, path, iconf);
        } finally {
            rdlock.unlock();
        }
    }

    /**
     * Change configuration of existing virtual bridge interface.
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
    public boolean modifyBridgeInterface(VTNManagerImpl mgr, VBridgeIfPath path,
                                         VInterfaceConfig iconf, boolean all)
        throws VTNException {
        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            VBridgeImpl vbr = getBridgeImpl(path);
            return vbr.modifyInterface(mgr, path, iconf, all);
        } finally {
            rdlock.unlock();
        }
    }

    /**
     * Remove the specified virtual interface from the bridge.
     *
     * @param mgr   VTN Manager service.
     * @param path  Path to the interface.
     * @throws VTNException  An error occurred.
     */
    public void removeBridgeInterface(VTNManagerImpl mgr, VBridgeIfPath path)
        throws VTNException {
        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            VBridgeImpl vbr = getBridgeImpl(path);
            vbr.removeInterface(mgr, path);
        } finally {
            rdlock.unlock();
        }
    }

    /**
     * Return a list of virtual interface information in the specified
     * virtual bridge.
     *
     * @param mgr   VTN Manager service.
     * @param path  Path to the bridge.
     * @return  A list of bridge interface information.
     * @throws VTNException  An error occurred.
     */
    public List<VInterface> getBridgeInterfaces(VTNManagerImpl mgr,
                                                VBridgePath path)
        throws VTNException {
        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            VBridgeImpl vbr = getBridgeImpl(path);
            return vbr.getInterfaces(mgr);
        } finally {
            rdlock.unlock();
        }
    }

    /**
     * Return information about the specified virtual bridge interface.
     *
     * @param mgr   VTN Manager service.
     * @param path  Path to the interface.
     * @return  The virtual interface information associated with the given
     *          name.
     * @throws VTNException  An error occurred.
     */
    public VInterface getBridgeInterface(VTNManagerImpl mgr, VBridgeIfPath path)
        throws VTNException {
        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            VBridgeImpl vbr = getBridgeImpl(path);
            return vbr.getInterface(mgr, path);
        } finally {
            rdlock.unlock();
        }
    }

    /**
     * Add a new VLAN mapping to to the specified L2 bridge.
     *
     * @param mgr     VTN Manager service.
     * @param path    Path to the bridge.
     * @param vlconf  VLAN mapping configuration.
     * @return  Information about the added VLAN mapping is returned.
     * @throws VTNException  An error occurred.
     */
    public VlanMap addVlanMap(VTNManagerImpl mgr, VBridgePath path,
                              VlanMapConfig vlconf) throws VTNException {
        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            VBridgeImpl vbr = getBridgeImpl(path);
            return vbr.addVlanMap(mgr, vlconf);
        } finally {
            rdlock.unlock();
        }
    }

    /**
     * Remove the specified VLAN mapping from from the bridge.
     *
     * @param mgr    VTN Manager service.
     * @param path   Path to the bridge.
     * @param mapId  The identifier of the VLAN mapping.
     * @throws VTNException  An error occurred.
     */
    public void removeVlanMap(VTNManagerImpl mgr, VBridgePath path,
                              String mapId) throws VTNException {
        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            VBridgeImpl vbr = getBridgeImpl(path);
            vbr.removeVlanMap(mgr, mapId);
        } finally {
            rdlock.unlock();
        }
    }

    /**
     * Return a list of virtual interface information in the specified
     * virtual bridge.
     *
     * @param path  Path to the bridge.
     * @return  A list of bridge interface information.
     * @throws VTNException  An error occurred.
     */
    public List<VlanMap> getVlanMaps(VBridgePath path) throws VTNException {
        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            VBridgeImpl vbr = getBridgeImpl(path);
            return vbr.getVlanMaps();
        } finally {
            rdlock.unlock();
        }
    }

    /**
     * Return information about the specified VLAN mapping in the virtual
     * bridge.
     *
     * @param path   Path to the bridge.
     * @param mapId  The identifier of the VLAN mapping.
     * @return  VLAN mapping information associated with the given ID.
     * @throws VTNException  An error occurred.
     */
    public VlanMap getVlanMap(VBridgePath path, String mapId)
        throws VTNException {
        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            VBridgeImpl vbr = getBridgeImpl(path);
            return vbr.getVlanMap(mapId);
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
     * @throws VTNException  An error occurred.
     */
    public PortMap getPortMap(VTNManagerImpl mgr, VBridgeIfPath path)
        throws VTNException {
        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            VBridgeImpl vbr = getBridgeImpl(path);
            return vbr.getPortMap(mgr, path);
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
    public void setPortMap(VTNManagerImpl mgr, VBridgeIfPath path,
                           PortMapConfig pmconf) throws VTNException {
        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            VBridgeImpl vbr = getBridgeImpl(path);
            vbr.setPortMap(mgr, path, pmconf);
        } finally {
            rdlock.unlock();
        }
    }

    /**
     * Return a list of MAC address entries learned by the specified virtual
     * L2 bridge.
     *
     * @param mgr   VTN Manager service.
     * @param path  Path to the bridge.
     * @return  A list of MAC address entries.
     * @throws VTNException  An error occurred.
     */
    public List<MacAddressEntry> getMacEntries(VTNManagerImpl mgr,
                                               VBridgePath path)
        throws VTNException {
        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            MacAddressTable table = getMacAddressTable(mgr, path);
            return table.getEntries();
        } finally {
            rdlock.unlock();
        }
    }

    /**
     * Search for a MAC address entry from the MAC address table in the
     * specified virtual L2 bridge.
     *
     * @param mgr   VTN Manager service.
     * @param path  Path to the bridge.
     * @param addr  MAC address.
     * @return  A MAC address entry associated with the specified MAC address.
     *          {@code null} is returned if not found.
     * @throws VTNException  An error occurred.
     */
    public MacAddressEntry getMacEntry(VTNManagerImpl mgr, VBridgePath path,
                                       DataLinkAddress addr)
        throws VTNException {
        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            MacAddressTable table = getMacAddressTable(mgr, path);
            return table.getEntry(addr);
        } finally {
            rdlock.unlock();
        }
    }

    /**
     * Remove a MAC address entry from the MAC address table in the virtual L2
     * bridge.
     *
     * @param mgr   VTN Manager service.
     * @param path  Path to the bridge.
     * @param addr  MAC address.
     * @return  A MAC address entry actually removed is returned.
     *          {@code null} is returned if not found.
     * @throws VTNException  An error occurred.
     */
    public MacAddressEntry removeMacEntry(VTNManagerImpl mgr, VBridgePath path,
                                          DataLinkAddress addr)
        throws VTNException {
        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            MacAddressTable table = getMacAddressTable(mgr, path);
            return table.removeEntry(addr);
        } finally {
            rdlock.unlock();
        }
    }

    /**
     * Flush all MAC address table entries in the specified virtual L2 bridge.
     *
     * @param mgr   VTN Manager service.
     * @param path  Path to the bridge.
     * @throws VTNException  An error occurred.
     */
    public void flushMacEntries(VTNManagerImpl mgr, VBridgePath path)
        throws VTNException {
        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            MacAddressTable table = getMacAddressTable(mgr, path);
            table.flush();
        } finally {
            rdlock.unlock();
        }
    }

    /**
     * Save tenant configuration to the configuration file.
     *
     * @param mgr  VTN Manager service.
     *             If a non-{@code null} value is specified, this method checks
     *             whether the current configuration is applied correctly.
     * @return  "Success" or failure reason.
     */
    public Status saveConfig(VTNManagerImpl mgr) {
        ObjectWriter wtr = new ObjectWriter();
        String path = VTenantImpl.getConfigFilePath(containerName, tenantName);
        Status status;
        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            if (mgr != null) {
                // Adjust interval of MAC address table aging.
                for (VBridgeImpl vbr: vBridges.values()) {
                    vbr.initMacTableAging(mgr);
                }
            }

            status = wtr.write(this, path);
            if (status.isSuccess()) {
                return status;
            }
        } finally {
            rdlock.unlock();
        }

        String msg = "Failed to save tenant configuration";
        LOG.error("{}:{}: {}: {}", containerName, tenantName, msg, status);
        return new Status(StatusCode.INTERNALERROR, msg);
    }

    /**
     * Resume the virtual tenant.
     *
     * <p>
     *   This method is called just after this tenant is instantiated from
     *   the configuration file.
     * </p>
     *
     * @param mgr   VTN Manager service.
     */
    public void resume(VTNManagerImpl mgr) {
        Lock wrlock = rwLock.writeLock();
        wrlock.lock();
        try {
            // Create flow database for this tenant.
            mgr.createTenantFlowDB(tenantName);

            for (VBridgeImpl vbr: vBridges.values()) {
                vbr.resume(mgr);
            }
        } finally {
            wrlock.unlock();
        }
    }

    /**
     * Invoked when a node is added, removed, or changed.
     *
     * @param mgr   VTN Manager service.
     * @param node  Node being updated.
     * @param type  Type of update.
     */
    public void notifyNode(VTNManagerImpl mgr, Node node, UpdateType type) {
        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            if (type == UpdateType.ADDED) {
                // Collect garbages in the VTN flow database.
                VTNFlowDatabase fdb = mgr.getTenantFlowDB(tenantName);
                if (fdb != null) {
                    fdb.nodeAdded(mgr, node);
                }
            } else if (type == UpdateType.REMOVED) {
                // Uninstall VTN flows related to the removed node.
                VTNFlowDatabase fdb = mgr.getTenantFlowDB(tenantName);
                if (fdb != null) {
                    fdb.removeFlows(mgr, node);
                }
            }

            for (VBridgeImpl vbr: vBridges.values()) {
                vbr.notifyNode(mgr, node, type);
            }
        } finally {
            rdlock.unlock();
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
    public void notifyNodeConnector(VTNManagerImpl mgr, NodeConnector nc,
                                    UpdateType type) {
        VNodeState pstate;
        if (type == UpdateType.REMOVED) {
            pstate = VNodeState.UNKNOWN;
        } else {
            // Determine whether the port is up or not.
            pstate = (mgr.isEnabled(nc)) ? VNodeState.UP : VNodeState.DOWN;
        }

        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            if (pstate != VNodeState.UP) {
                // Uninstall VTN flows related to the switch port.
                VTNFlowDatabase fdb = mgr.getTenantFlowDB(tenantName);
                if (fdb != null) {
                    fdb.removeFlows(mgr, nc);
                }
            }
            for (VBridgeImpl vbr: vBridges.values()) {
                vbr.notifyNodeConnector(mgr, nc, pstate, type);
            }
        } finally {
            rdlock.unlock();
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
    public void edgeUpdate(VTNManagerImpl mgr, List<TopoEdgeUpdate> topoList) {
        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            for (VBridgeImpl vbr: vBridges.values()) {
                vbr.edgeUpdate(mgr, topoList);
            }
        } finally {
            rdlock.unlock();
        }
    }

    /**
     * Notify the listener of current configuration.
     *
     * @param mgr       VTN Manager service.
     * @param listener  VTN manager listener service.
     */
    public void notifyConfiguration(VTNManagerImpl mgr,
                                    IVTNManagerAware listener) {
        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            for (VBridgeImpl vbr: vBridges.values()) {
                vbr.notifyConfiguration(mgr, listener);
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
     * @param path  Path to the target bridge.
     * @throws VTNException  An error occurred.
     */
    public void findHost(VTNManagerImpl mgr, PacketContext pctx,
                         VBridgePath path) throws VTNException {
        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            VBridgeImpl vbr = getBridgeImpl(path);
            vbr.findHost(mgr, pctx);
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
    public void findHost(VTNManagerImpl mgr, PacketContext pctx) {
        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            for (VBridgeImpl vbr: vBridges.values()) {
                vbr.findHost(mgr, pctx);
            }
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
     *          belongs to this tenant. If a ARP request was actually sent to
     *          the network, {@code Boolean.TRUE} is returned.
     *          {@code null} is returned if the specified host does not
     *          belong to this tenant.
     */
    public Boolean probeHost(VTNManagerImpl mgr, MapType type,
                             PacketContext pctx) {
        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            for (VBridgeImpl vbr: vBridges.values()) {
                Boolean res = vbr.probeHost(mgr, type, pctx);
                if (res != null) {
                    return res;
                }
            }
        } finally {
            rdlock.unlock();
        }

        return null;
    }

    /**
     * Handler for receiving the packet.
     *
     * @param mgr   VTN manager service.
     * @param type  Mapping type to be tested.
     * @param pctx  The context of the received packet.
     * @return  A {@code PacketResult} which indicates the result of handler.
     */
    public PacketResult receive(VTNManagerImpl mgr, MapType type,
                                PacketContext pctx) {
        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            for (VBridgeImpl vbr: vBridges.values()) {
                PacketResult res = vbr.receive(mgr, type, pctx);
                if (res != PacketResult.IGNORED) {
                    pctx.purgeObsoleteFlow(mgr, tenantName);
                    return res;
                }
            }
        } finally {
            rdlock.unlock();
        }

        return PacketResult.IGNORED;
    }

    /**
     * Invoked when the recalculation of the all shortest path tree is done.
     *
     * @param mgr  VTN manager service.
     */
    public void recalculateDone(VTNManagerImpl mgr) {
        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            for (VBridgeImpl vbr: vBridges.values()) {
                vbr.recalculateDone(mgr);
            }
        } finally {
            rdlock.unlock();
        }
    }

    /**
     * Destroy the virtual tenant.
     *
     * @param mgr  VTN manager service.
     */
    public void destroy(VTNManagerImpl mgr) {
        VTenantImpl.deleteConfigFile(containerName, tenantName);

        Lock wrlock = rwLock.writeLock();
        wrlock.lock();
        try {
            // Destroy all bridges.
            for (Iterator<VBridgeImpl> it = vBridges.values().iterator();
                 it.hasNext();) {
                VBridgeImpl vbr = it.next();
                vbr.destroy(mgr, true);
                it.remove();
            }
        } finally {
            wrlock.unlock();
        }

        // Purge global timer task queue.
        IVTNResourceManager resMgr = mgr.getResourceManager();
        Timer timer = resMgr.getTimer();
        timer.purge();
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
        if (!(o instanceof VTenantImpl)) {
            return false;
        }

        VTenantImpl vtn = (VTenantImpl)o;
        if (!containerName.equals(vtn.containerName)) {
            return false;
        }
        if (!tenantName.equals(vtn.tenantName)) {
            return false;
        }

        VTenantConfig tconf = getVTenantConfig();
        VTenantConfig otherTconf = vtn.getVTenantConfig();
        if (!tconf.equals(otherTconf)) {
            return false;
        }

        // Use copy of bridge map in order to avoid deadlock.
        Map<String, VBridgeImpl> bridges = getBridgeMap();
        Map<String, VBridgeImpl> otherBridges = vtn.getBridgeMap();

        return bridges.equals(otherBridges);
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        int h = containerName.hashCode() ^ tenantName.hashCode() ^
            getVTenantConfig().hashCode();

        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            h += vBridges.hashCode();
        } finally {
            rdlock.unlock();
        }

        return h;
    }

    /**
     * Merge the given VTN configuration to the current configuration.
     *
     * <p>
     *   If at least one field in {@code tconf} keeps a valid value, this
     *   method creates a shallow copy of the current configuration, and set
     *   valid values in {@code tconf} to the copy.
     * </p>
     *
     * @param tconf  Configuration to be merged.
     * @return  A merged {@code VTenantConfig} object.
     */
    private VTenantConfig merge(VTenantConfig tconf) {
        String desc = tconf.getDescription();
        int idle = tconf.getIdleTimeout();
        int hard = tconf.getHardTimeout();
        if (desc == null && idle < 0 && hard < 0) {
            return tenantConfig;
        }

        if (desc == null) {
            desc = tenantConfig.getDescription();
        }
        if (idle < 0) {
            idle = tenantConfig.getIdleTimeout();
        }
        if (hard < 0) {
            hard = tenantConfig.getHardTimeout();
        }

        return new VTenantConfig(desc, idle, hard);
    }

    /**
     * Resolve undefined attributes in the specified tenant configuration.
     *
     * @param tconf  The tenant configuration.
     * @return       {@code VTenantConfig} to be applied.
     */
    private VTenantConfig resolve(VTenantConfig tconf) {
        int idle = tconf.getIdleTimeout();
        int hard = tconf.getHardTimeout();
        if (idle < 0) {
            idle = DEFAULT_IDLE_TIMEOUT;
            if (hard < 0) {
                hard = DEFAULT_HARD_TIMEOUT;
            }
        } else if (hard < 0) {
            hard = DEFAULT_HARD_TIMEOUT;
        } else {
            return tconf;
        }

        return new VTenantConfig(tconf.getDescription(), idle, hard);
    }

    /**
     * Ensure that the specified tenant configuration is valid.
     *
     * @param tconf  The tenant configuration to be tested.
     * @throws VTNException  An error occurred.
     */
    private void checkConfig(VTenantConfig tconf) throws VTNException {
        int idle = tconf.getIdleTimeout();
        int hard = tconf.getHardTimeout();
        if (idle > MAX_FLOW_TIMEOUT) {
            throw new VTNException(StatusCode.BADREQUEST,
                                   "Invalid idle timeout");
        }
        if (hard > MAX_FLOW_TIMEOUT) {
            throw new VTNException(StatusCode.BADREQUEST,
                                   "Invalid hard timeout");
        }
        if (idle != 0 && hard != 0 && idle >= hard) {
            String msg = "Idle timeout must be less than hard timeout";
            throw new VTNException(StatusCode.BADREQUEST, msg);
        }
    }

    /**
     * Return a shallow copy of the virtual bridge map.
     *
     * @return  Pairs of bridge name and bridge instance.
     */
    private Map<String, VBridgeImpl> getBridgeMap() {
        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            return new TreeMap<String, VBridgeImpl>(vBridges);
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

        // Set this tenant as parent of bridges.
        for (Map.Entry<String, VBridgeImpl> entry: vBridges.entrySet()) {
            String name = entry.getKey();
            VBridgeImpl vbr = entry.getValue();
            vbr.setPath(this, name);
        }
    }

    /**
     * Return a failure status that indicates the specified bridge does not
     * exist.
     *
     * @param bridgeName  The name of the bridge.
     * @return  A failure status.
     */
    private Status bridgeNotFound(String bridgeName) {
        String msg = bridgeName + ": Bridge does not exist";
        return new Status(StatusCode.NOTFOUND, msg);
    }

    /**
     * Return the virtual bridge instance associated with the given name.
     *
     * <p>
     *   This method must be called with the tenant lock.
     * </p>
     *
     * @param path  Path to the bridge.
     * @return  Virtual bridge instance is returned.
     * @throws VTNException  An error occurred.
     * @throws NullPointerException  {@code path} is {@code null}.
     */
    private VBridgeImpl getBridgeImpl(VBridgePath path) throws VTNException {
        String bridgeName = path.getBridgeName();
        if (bridgeName == null) {
            Status status = VTNManagerImpl.argumentIsNull("Bridge name");
            throw new VTNException(status);
        }

        VBridgeImpl vbr = vBridges.get(bridgeName);
        if (vbr == null) {
            Status status = bridgeNotFound(bridgeName);
            throw new VTNException(status);
        }

        return vbr;
    }

    /**
     * Return the MAC address table for the specified virtual bridge.
     *
     * <p>
     *   This method must be called with the tenant lock.
     * </p>
     *
     * @param mgr   VTN manager service.
     * @param path  Path to the bridge.
     * @return  MAC address table for the specified bridge.
     * @throws VTNException  An error occurred.
     * @throws NullPointerException  {@code path} is {@code null}.
     */
    private MacAddressTable getMacAddressTable(VTNManagerImpl mgr,
                                               VBridgePath path)
        throws VTNException {
        MacAddressTable table = mgr.getMacAddressTable(path);
        if (table != null) {
            return table;
        }

        String bridgeName = path.getBridgeName();
        Status status = (bridgeName == null)
            ? VTNManagerImpl.argumentIsNull("Bridge name")
            : bridgeNotFound(bridgeName);

        throw new VTNException(status);
    }
}
