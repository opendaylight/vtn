/*
 * Copyright (c) 2013, 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.cluster;

import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.io.IOException;
import java.util.Iterator;
import java.util.Map;
import java.util.TreeMap;
import java.util.List;
import java.util.ArrayList;
import java.util.Set;
import java.util.Timer;
import java.util.concurrent.ConcurrentMap;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantReadWriteLock;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import org.opendaylight.vtn.manager.DataLinkHost;
import org.opendaylight.vtn.manager.IVTNManagerAware;
import org.opendaylight.vtn.manager.MacAddressEntry;
import org.opendaylight.vtn.manager.MacMap;
import org.opendaylight.vtn.manager.MacMapConfig;
import org.opendaylight.vtn.manager.PortMap;
import org.opendaylight.vtn.manager.PortMapConfig;
import org.opendaylight.vtn.manager.VBridge;
import org.opendaylight.vtn.manager.VBridgeConfig;
import org.opendaylight.vtn.manager.VBridgeIfPath;
import org.opendaylight.vtn.manager.VBridgePath;
import org.opendaylight.vtn.manager.VInterface;
import org.opendaylight.vtn.manager.VInterfaceConfig;
import org.opendaylight.vtn.manager.VInterfacePath;
import org.opendaylight.vtn.manager.VNodePath;
import org.opendaylight.vtn.manager.VTNException;
import org.opendaylight.vtn.manager.VTenant;
import org.opendaylight.vtn.manager.VTenantConfig;
import org.opendaylight.vtn.manager.VTenantPath;
import org.opendaylight.vtn.manager.VTerminal;
import org.opendaylight.vtn.manager.VTerminalConfig;
import org.opendaylight.vtn.manager.VTerminalIfPath;
import org.opendaylight.vtn.manager.VTerminalPath;
import org.opendaylight.vtn.manager.VlanMap;
import org.opendaylight.vtn.manager.VlanMapConfig;
import org.opendaylight.vtn.manager.flow.filter.FlowFilterId;
import org.opendaylight.vtn.manager.util.EtherAddress;

import org.opendaylight.vtn.manager.internal.ContainerConfig;
import org.opendaylight.vtn.manager.internal.LockStack;
import org.opendaylight.vtn.manager.internal.MacAddressTable;
import org.opendaylight.vtn.manager.internal.PacketContext;
import org.opendaylight.vtn.manager.internal.PortFilter;
import org.opendaylight.vtn.manager.internal.RouteResolver;
import org.opendaylight.vtn.manager.internal.TxContext;
import org.opendaylight.vtn.manager.internal.VTNManagerImpl;
import org.opendaylight.vtn.manager.internal.inventory.VtnNodeEvent;
import org.opendaylight.vtn.manager.internal.inventory.VtnPortEvent;
import org.opendaylight.vtn.manager.internal.routing.PathMapEvaluator;
import org.opendaylight.vtn.manager.internal.routing.RoutingEvent;
import org.opendaylight.vtn.manager.internal.util.MiscUtils;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;

import org.opendaylight.controller.sal.core.NodeConnector;
import org.opendaylight.controller.sal.core.UpdateType;
import org.opendaylight.controller.sal.packet.address.DataLinkAddress;
import org.opendaylight.controller.sal.utils.Status;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnAclType;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateOperationType;

/**
 * Implementation of virtual tenant.
 *
 * <p>
 *   Although this class is public to other packages, this class does not
 *   provide any API. Applications other than VTN Manager must not use this
 *   class.
 * </p>
 */
public final class VTenantImpl implements FlowFilterNode {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = 6897679062908750592L;

    /**
     * Logger instance.
     */
    private static final Logger  LOG =
        LoggerFactory.getLogger(VTenantImpl.class);

    /**
     * A status message that means no resource was modified.
     */
    private static final String  MSG_NOT_MODIFIED = "Not modified";

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
    private static final int  DEFAULT_HARD_TIMEOUT = 0;

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
    private transient Map<String, VBridgeImpl> vBridges =
        new TreeMap<String, VBridgeImpl>();

    /**
     * vTerminal instances configured in this tenant.
     */
    private transient Map<String, VTerminalImpl> vTerminals =
        new TreeMap<String, VTerminalImpl>();

    /**
     * Flow filters configured in this tenant.
     */
    private transient FlowFilterMap  flowFilters;

    /**
     * Read write lock to synchronize per-tenant resources.
     */
    private transient ReentrantReadWriteLock  rwLock =
        new ReentrantReadWriteLock();

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
        VTenantConfig cf = resolve(tconf);
        checkConfig(cf);
        this.containerName = containerName;
        this.tenantName = tenantName;
        this.tenantConfig = cf;
        flowFilters = FlowFilterMap.createIncoming(this);
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
    public VTenant getVTenant() {
        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            return new VTenant(tenantName, tenantConfig);
        } finally {
            rdlock.unlock();
        }
    }

    /**
     * Return tenant configuration.
     *
     * @return  Configuration for the tenant.
     */
    public VTenantConfig getVTenantConfig() {
        // Return without holding any lock.
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
     * @throws VTNException  An error occurred.
     */
    public void setVTenantConfig(VTNManagerImpl mgr, VTenantPath path,
                                 VTenantConfig tconf, boolean all)
        throws VTNException {
        VTenantConfig cf;
        Lock wrlock = rwLock.writeLock();
        wrlock.lock();
        try {
            if (all) {
                cf = resolve(tconf);
            } else {
                cf = merge(tconf);
            }
            if (cf.equals(tenantConfig)) {
                return;
            }

            checkConfig(cf);
            tenantConfig = cf;
            VTenant vtenant = new VTenant(tenantName, cf);
            mgr.enqueueEvent(path, vtenant, UpdateType.CHANGED);

            mgr.export(this);
            saveConfigImpl(null);
        } finally {
            wrlock.unlock();
        }
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
        MiscUtils.checkName("Bridge", bridgeName);

        if (bconf == null) {
            throw RpcException.getNullArgumentException(
                "Bridge configuration");
        }

        VBridgeImpl vbr = new VBridgeImpl(this, bridgeName, bconf);
        Lock wrlock = rwLock.writeLock();
        wrlock.lock();
        try {
            VBridgeImpl old = vBridges.put(bridgeName, vbr);
            if (old != null) {
                vBridges.put(bridgeName, old);
                String msg = bridgeName + ": Bridge name already exists";
                throw RpcException.getDataExistsException(msg);
            }

            VBridge vbridge = vbr.getVBridge(mgr);
            VBridgeEvent.added(mgr, path, vbridge);

            // Create a MAC address table for this bridge.
            vbr.initMacTableAging(mgr);

            mgr.export(this);
            saveConfigImpl(null);
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
     * @throws VTNException  An error occurred.
     */
    public void modifyBridge(VTNManagerImpl mgr, VBridgePath path,
                             VBridgeConfig bconf, boolean all)
        throws VTNException {
        if (bconf == null) {
            throw RpcException.getNullArgumentException(
                "Bridge configuration");
        }

        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            VBridgeImpl vbr = getBridgeImpl(path);
            if (vbr.setVBridgeConfig(mgr, bconf, all)) {
                mgr.export(this);
                saveConfigImpl(null);
            }
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
                throw RpcException.getNullArgumentException("Bridge name");
            }

            VBridgeImpl vbr = vBridges.remove(bridgeName);
            if (vbr == null) {
                throw getBridgeNotFoundException(bridgeName);
            }

            vbr.destroy(mgr, true);

            mgr.export(this);
            saveConfigImpl(null);
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
        List<VBridge> list;
        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            list = new ArrayList<VBridge>(vBridges.size());
            for (VBridgeImpl vbr: vBridges.values()) {
                list.add(vbr.getVBridge(mgr));
            }
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
     * Add a new vTerminal to this tenant.
     *
     * @param mgr     VTN Manager service.
     * @param path    Path to the vTerminal.
     * @param vtconf  vTerminal configuration.
     * @throws VTNException  An error occurred.
     */
    public void addTerminal(VTNManagerImpl mgr, VTerminalPath path,
                            VTerminalConfig vtconf) throws VTNException {
        // Ensure the given terminal name is valid.
        String termName = path.getTerminalName();
        MiscUtils.checkName("vTerminal", termName);

        if (vtconf == null) {
            throw RpcException.getNullArgumentException(
                "vTerminal configuration");
        }

        VTerminalImpl vtm = new VTerminalImpl(this, termName, vtconf);
        Lock wrlock = rwLock.writeLock();
        wrlock.lock();
        try {
            VTerminalImpl old = vTerminals.put(termName, vtm);
            if (old != null) {
                vTerminals.put(termName, old);
                String msg = termName + ": vTerminal name already exists";
                throw RpcException.getDataExistsException(msg);
            }

            VTerminal vterm = vtm.getVTerminal(mgr);
            VTerminalEvent.added(mgr, path, vterm);
            mgr.export(this);
            saveConfigImpl(null);
        } finally {
            wrlock.unlock();
        }
    }

    /**
     * Change configuration of existing vTerminal.
     *
     * @param mgr     VTN Manager service.
     * @param path    Path to the vTerminal.
     * @param vtconf  vTerminal configuration.
     * @param all     If {@code true} is specified, all attributes of the
     *                vTerminal are modified. In this case, {@code null} in
     *                {@code vtconf} is interpreted as default value.
     *                If {@code false} is specified, an attribute is not
     *                modified if its value in {@code vtconf} is {@code null}.
     * @throws VTNException  An error occurred.
     */
    public void modifyTerminal(VTNManagerImpl mgr, VTerminalPath path,
                               VTerminalConfig vtconf, boolean all)
        throws VTNException {
        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            VTerminalImpl vtm = getTerminalImpl(path);
            if (vtm.setVTerminalConfig(mgr, vtconf, all)) {
                mgr.export(this);
                saveConfigImpl(null);
            }
        } finally {
            rdlock.unlock();
        }
    }

    /**
     * Remove the specified vTerminal from the tenant.
     *
     * @param mgr   VTN Manager service.
     * @param path  Path to the vTerminal.
     * @throws VTNException  An error occurred.
     */
    public void removeTerminal(VTNManagerImpl mgr, VTerminalPath path)
        throws VTNException {
        Lock wrlock = rwLock.writeLock();
        wrlock.lock();
        try {
            String termName = path.getTerminalName();
            if (termName == null) {
                throw RpcException.getNullArgumentException("vTerminal name");
            }

            VTerminalImpl vtm = vTerminals.remove(termName);
            if (vtm == null) {
                throw getTerminalNotFoundException(termName);
            }

            vtm.destroy(mgr, true);
            mgr.export(this);
            saveConfigImpl(null);
        } finally {
            wrlock.unlock();
        }
    }

    /**
     * Return a list of vTerminal information in the tenant.
     *
     * @param mgr   VTN Manager service.
     * @return  A list of vTerminal information.
     */
    public List<VTerminal> getTerminals(VTNManagerImpl mgr) {
        List<VTerminal> list;
        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            list = new ArrayList<VTerminal>(vTerminals.size());
            for (VTerminalImpl vtm: vTerminals.values()) {
                list.add(vtm.getVTerminal(mgr));
            }
        } finally {
            rdlock.unlock();
        }

        return list;
    }

    /**
     * Return information about the specified vTerminal.
     *
     * @param mgr   VTN Manager service.
     * @param path  Path to the vTerminal.
     * @return  The vTerminal information associated with the given name.
     * @throws VTNException  An error occurred.
     */
    public VTerminal getTerminal(VTNManagerImpl mgr, VTerminalPath path)
        throws VTNException {
        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            VTerminalImpl vtm = getTerminalImpl(path);
            return vtm.getVTerminal(mgr);
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
    public void addInterface(VTNManagerImpl mgr, VBridgeIfPath path,
                             VInterfaceConfig iconf)
        throws VTNException {
        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            VBridgeImpl vbr = getBridgeImpl(path);
            vbr.addInterface(mgr, path, iconf);

            mgr.export(this);
            saveConfigImpl(null);
        } finally {
            rdlock.unlock();
        }
    }

    /**
     * Change configuration of existing virtual bridge interface.
     *
     * @param mgr    VTN Manager service.
     * @param ctx    MD-SAL datastore transaction context.
     * @param path   Path to the interface.
     * @param iconf  Interface configuration.
     * @param all    If {@code true} is specified, all attributes of the
     *               interface are modified. In this case, {@code null} in
     *               {@code iconf} is interpreted as default value.
     *               If {@code false} is specified, an attribute is not
     *               modified if its value in {@code iconf} is {@code null}.
     * @throws VTNException  An error occurred.
     */
    public void modifyInterface(VTNManagerImpl mgr, TxContext ctx,
                                VBridgeIfPath path, VInterfaceConfig iconf,
                                boolean all)
        throws VTNException {
        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            VBridgeImpl vbr = getBridgeImpl(path);
            if (vbr.modifyInterface(mgr, ctx, path, iconf, all)) {
                mgr.export(this);
                saveConfigImpl(null);
            }
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
    public void removeInterface(VTNManagerImpl mgr, VBridgeIfPath path)
        throws VTNException {
        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            VBridgeImpl vbr = getBridgeImpl(path);
            vbr.removeInterface(mgr, path);

            mgr.export(this);
            saveConfigImpl(null);
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
    public List<VInterface> getInterfaces(VTNManagerImpl mgr, VBridgePath path)
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
    public VInterface getInterface(VTNManagerImpl mgr, VBridgeIfPath path)
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
     * Add a new virtual interface to the specified vTerminal.
     *
     * @param mgr    VTN Manager service.
     * @param path   Path to the interface to be added.
     * @param iconf  Interface configuration.
     * @throws VTNException  An error occurred.
     */
    public void addInterface(VTNManagerImpl mgr, VTerminalIfPath path,
                             VInterfaceConfig iconf)
        throws VTNException {
        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            VTerminalImpl vtm = getTerminalImpl(path);
            vtm.addInterface(mgr, path, iconf);

            mgr.export(this);
            saveConfigImpl(null);
        } finally {
            rdlock.unlock();
        }
    }

    /**
     * Change configuration of existing vTerminal interface.
     *
     * @param mgr    VTN Manager service.
     * @param ctx    MD-SAL datastore transaction context.
     * @param path   Path to the interface.
     * @param iconf  Interface configuration.
     * @param all    If {@code true} is specified, all attributes of the
     *               interface are modified. In this case, {@code null} in
     *               {@code iconf} is interpreted as default value.
     *               If {@code false} is specified, an attribute is not
     *               modified if its value in {@code iconf} is {@code null}.
     * @throws VTNException  An error occurred.
     */
    public void modifyInterface(VTNManagerImpl mgr, TxContext ctx,
                                VTerminalIfPath path, VInterfaceConfig iconf,
                                boolean all)
        throws VTNException {
        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            VTerminalImpl vtm = getTerminalImpl(path);
            if (vtm.modifyInterface(mgr, ctx, path, iconf, all)) {
                mgr.export(this);
                saveConfigImpl(null);
            }
        } finally {
            rdlock.unlock();
        }
    }

    /**
     * Remove the specified virtual interface from the vTerminal.
     *
     * @param mgr   VTN Manager service.
     * @param path  Path to the interface.
     * @throws VTNException  An error occurred.
     */
    public void removeInterface(VTNManagerImpl mgr, VTerminalIfPath path)
        throws VTNException {
        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            VTerminalImpl vtm = getTerminalImpl(path);
            vtm.removeInterface(mgr, path);

            mgr.export(this);
            saveConfigImpl(null);
        } finally {
            rdlock.unlock();
        }
    }

    /**
     * Return a list of virtual interface information in the specified
     * vTerminal.
     *
     * @param mgr   VTN Manager service.
     * @param path  Path to the vTerminal.
     * @return  A list of vTerminal interface information.
     * @throws VTNException  An error occurred.
     */
    public List<VInterface> getInterfaces(VTNManagerImpl mgr,
                                          VTerminalPath path)
        throws VTNException {
        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            VTerminalImpl vtm = getTerminalImpl(path);
            return vtm.getInterfaces(mgr);
        } finally {
            rdlock.unlock();
        }
    }

    /**
     * Return information about the specified vTerminal interface.
     *
     * @param mgr   VTN Manager service.
     * @param path  Path to the interface.
     * @return  The virtual interface information associated with the given
     *          name.
     * @throws VTNException  An error occurred.
     */
    public VInterface getInterface(VTNManagerImpl mgr, VTerminalIfPath path)
        throws VTNException {
        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            VTerminalImpl vtm = getTerminalImpl(path);
            return vtm.getInterface(mgr, path);
        } finally {
            rdlock.unlock();
        }
    }

    /**
     * Add a new VLAN mapping to to the specified L2 bridge.
     *
     * @param mgr     VTN Manager service.
     * @param ctx     MD-SAL datastore transaction context.
     * @param path    Path to the bridge.
     * @param vlconf  VLAN mapping configuration.
     * @return  Information about the added VLAN mapping is returned.
     * @throws VTNException  An error occurred.
     */
    public VlanMap addVlanMap(VTNManagerImpl mgr, TxContext ctx,
                              VBridgePath path, VlanMapConfig vlconf)
        throws VTNException {
        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            VBridgeImpl vbr = getBridgeImpl(path);
            VlanMap vlmap = vbr.addVlanMap(mgr, ctx, vlconf);

            mgr.export(this);
            saveConfigImpl(null);
            return vlmap;
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

            mgr.export(this);
            saveConfigImpl(null);
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
     * Return information about the VLAN mapping which matches the specified
     * VLAN mapping configuration in the specified virtual L2 bridge.
     *
     * @param path    Path to the bridge.
     * @param vlconf  VLAN mapping configuration.
     * @return  Information about the VLAN mapping which matches the specified
     *          VLAN mapping information.
     * @throws VTNException  An error occurred.
     */
    public VlanMap getVlanMap(VBridgePath path, VlanMapConfig vlconf)
        throws VTNException {
        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            VBridgeImpl vbr = getBridgeImpl(path);
            return vbr.getVlanMap(vlconf);
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
     * Return the port mapping configuration applied to the specified
     * vTerminal interface.
     *
     * @param mgr   VTN Manager service.
     * @param path  Path to the vTerminal interface.
     * @return  Port mapping information.
     * @throws VTNException  An error occurred.
     */
    public PortMap getPortMap(VTNManagerImpl mgr, VTerminalIfPath path)
        throws VTNException {
        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            VTerminalImpl vtm = getTerminalImpl(path);
            return vtm.getPortMap(mgr, path);
        } finally {
            rdlock.unlock();
        }
    }

    /**
     * Create or destroy mapping between the physical switch port and the
     * virtual bridge interface.
     *
     * @param mgr     VTN Manager service.
     * @param ctx     MD-SAL datastore transaction context.
     * @param path    Path to the bridge interface.
     * @param pmconf  Port mapping configuration to be set.
     *                If {@code null} is specified, port mapping on the
     *                specified interface is destroyed.
     * @throws VTNException  An error occurred.
     */
    public void setPortMap(VTNManagerImpl mgr, TxContext ctx,
                           VBridgeIfPath path, PortMapConfig pmconf)
        throws VTNException {
        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            VBridgeImpl vbr = getBridgeImpl(path);
            if (vbr.setPortMap(mgr, ctx, path, pmconf)) {
                mgr.export(this);
                saveConfigImpl(null);
            }
        } finally {
            rdlock.unlock();
        }
    }

    /**
     * Create or destroy mapping between the physical switch port and the
     * vTerminal interface.
     *
     * @param mgr     VTN Manager service.
     * @param ctx     MD-SAL datastore transaction context.
     * @param path    Path to the vTerminal interface.
     * @param pmconf  Port mapping configuration to be set.
     *                If {@code null} is specified, port mapping on the
     *                specified interface is destroyed.
     * @throws VTNException  An error occurred.
     */
    public void setPortMap(VTNManagerImpl mgr, TxContext ctx,
                           VTerminalIfPath path, PortMapConfig pmconf)
        throws VTNException {
        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            VTerminalImpl vtm = getTerminalImpl(path);
            if (vtm.setPortMap(mgr, ctx, path, pmconf)) {
                mgr.export(this);
                saveConfigImpl(null);
            }
        } finally {
            rdlock.unlock();
        }
    }

    /**
     * Return information about the MAC mapping configured in the specified
     * vBridge.
     *
     * @param mgr   VTN Manager service.
     * @param path   Path to the bridge.
     * @return  A {@link MacMap} object which represents information about
     *          the MAC mapping specified by {@code path}.
     *          {@code null} is returned if the MAC mapping is not configured
     *          in the specified vBridge.
     * @throws VTNException  An error occurred.
     */
    public MacMap getMacMap(VTNManagerImpl mgr, VBridgePath path)
        throws VTNException {
        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            VBridgeImpl vbr = getBridgeImpl(path);
            return vbr.getMacMap(mgr);
        } finally {
            rdlock.unlock();
        }
    }

    /**
     * Return configuration information about MAC mapping in the specified
     * vBridge.
     *
     * @param path     Path to the vBridge.
     * @param aclType  The type of access control list.
     * @return  A set of {@link DataLinkHost} instances which contains host
     *          information in the specified access control list is returned.
     *          {@code null} is returned if MAC mapping is not configured in
     *          the specified vBridge.
     * @throws VTNException  An error occurred.
     */
    public Set<DataLinkHost> getMacMapConfig(VBridgePath path,
                                             VtnAclType aclType)
        throws VTNException {
        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            VBridgeImpl vbr = getBridgeImpl(path);
            return vbr.getMacMapConfig(aclType);
        } finally {
            rdlock.unlock();
        }
    }

    /**
     * Return a list of {@link MacAddressEntry} instances corresponding to
     * all the MAC address information actually mapped by MAC mapping
     * configured in the specified vBridge.
     *
     * @param mgr   VTN Manager service.
     * @param path  Path to the vBridge.
     * @return  A list of {@link MacAddressEntry} instances corresponding to
     *          all the MAC address information actually mapped to the vBridge
     *          specified by {@code path}.
     *          {@code null} is returned if MAC mapping is not configured
     *          in the specified vBridge.
     * @throws VTNException  An error occurred.
     */
    public List<MacAddressEntry> getMacMappedHosts(VTNManagerImpl mgr,
                                                   VBridgePath path)
        throws VTNException {
        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            VBridgeImpl vbr = getBridgeImpl(path);
            return vbr.getMacMappedHosts(mgr);
        } finally {
            rdlock.unlock();
        }
    }

    /**
     * Determine whether the host specified by the MAC address is actually
     * mapped by MAC mapping configured in the specified vBridge.
     *
     * @param mgr   VTN Manager service.
     * @param path  Path to the vBridge.
     * @param addr  A {@link DataLinkAddress} instance which represents the
     *              MAC address.
     * @return  A {@link MacAddressEntry} instancw which represents information
     *          about the host corresponding to {@code addr} is returned
     *          if it is actually mapped to the specified vBridge by MAC
     *          mapping.
     *          {@code null} is returned if the MAC address specified by
     *          {@code addr} is not mapped by MAC mapping, or MAC mapping is
     *          not configured in the specified vBridge.
     * @throws VTNException  An error occurred.
     */
    public MacAddressEntry getMacMappedHost(VTNManagerImpl mgr,
                                            VBridgePath path,
                                            DataLinkAddress addr)
        throws VTNException {
        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            VBridgeImpl vbr = getBridgeImpl(path);
            return vbr.getMacMappedHost(mgr, addr);
        } finally {
            rdlock.unlock();
        }
    }

    /**
     * Change MAC mapping configuration as specified by {@link MacMapConfig}
     * instance.
     *
     * @param mgr     VTN Manager service.
     * @param path    A {@link VBridgePath} object that specifies the position
     *                of the vBridge.
     * @param op      A {@link VtnUpdateOperationType} instance which indicates
     *                how to change the MAC mapping configuration.
     * @param mcconf  A {@link MacMapConfig} instance which contains the MAC
     *                mapping configuration information.
     * @return        A {@link UpdateType} object which represents the result
     *                of the operation is returned.
     *                {@code null} is returned if the configuration was not
     *                changed.
     * @throws VTNException  An error occurred.
     */
    public UpdateType setMacMap(VTNManagerImpl mgr, VBridgePath path,
                                VtnUpdateOperationType op, MacMapConfig mcconf)
        throws VTNException {
        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            VBridgeImpl vbr = getBridgeImpl(path);
            UpdateType result = vbr.setMacMap(mgr, op, mcconf);
            if (result != null) {
                mgr.export(this);
                saveConfigImpl(null);
            }

            return result;
        } finally {
            rdlock.unlock();
        }
    }

    /**
     * Change the access controll list for the specified MAC mapping.
     *
     * @param mgr       VTN Manager service.
     * @param path      A {@link VBridgePath} object that specifies the
     *                  position of the vBridge.
     * @param op        A {@link VtnUpdateOperationType} instance which
     *                  indicates how to change the MAC mapping configuration.
     * @param aclType   The type of access control list.
     * @param dlhosts   A set of {@link DataLinkHost} instances.
     * @return          A {@link UpdateType} object which represents the result
     *                  of the operation is returned.
     *                  {@code null} is returned if the configuration was not
     *                  changed.
     * @throws VTNException  An error occurred.
     */
    public UpdateType setMacMap(VTNManagerImpl mgr, VBridgePath path,
                                VtnUpdateOperationType op, VtnAclType aclType,
                                Set<? extends DataLinkHost> dlhosts)
        throws VTNException {
        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            VBridgeImpl vbr = getBridgeImpl(path);
            UpdateType result = vbr.setMacMap(mgr, op, aclType, dlhosts);
            if (result != null) {
                mgr.export(this);
                saveConfigImpl(null);
            }

            return result;
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
     * @throws VTNException  An error occurred.
     */
    public void saveConfig(VTNManagerImpl mgr) throws VTNException {
        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            saveConfigImpl(mgr);
        } finally {
            rdlock.unlock();
        }
    }

    /**
     * Resume the virtual tenant.
     *
     * <p>
     *   This method is called just after this tenant is instantiated from
     *   the configuration file.
     * </p>
     *
     * @param mgr  VTN Manager service.
     * @param ctx  A runtime context for MD-SAL datastore transaction task.
     */
    public void resume(VTNManagerImpl mgr, TxContext ctx) {
        Lock wrlock = rwLock.writeLock();
        wrlock.lock();
        try {
            for (VBridgeImpl vbr: vBridges.values()) {
                vbr.resume(mgr, ctx);
            }

            for (VTerminalImpl vtm: vTerminals.values()) {
                vtm.resume(mgr, ctx);
            }
        } finally {
            wrlock.unlock();
        }

        LOG.trace("{}:{}: Tenant was resumed", containerName, tenantName);
    }

    /**
     * Invoked when a node has been added or removed.
     *
     * @param mgr  VTN Manager service.
     * @param ev   A {@link VtnNodeEvent} instance.
     * @throws VTNException  An error occurred.
     */
    public void notifyNode(VTNManagerImpl mgr, VtnNodeEvent ev)
        throws VTNException {
        ConcurrentMap<VTenantPath, Object> db = mgr.getStateDB();

        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            for (VBridgeImpl vbr: vBridges.values()) {
                vbr.notifyNode(mgr, db, ev);
            }

            for (VTerminalImpl vtm: vTerminals.values()) {
                vtm.notifyNode(mgr, db, ev);
            }
        } finally {
            rdlock.unlock();
        }
    }

    /**
     * This method is called when some properties of a node connector are
     * added/deleted/changed.
     *
     * @param mgr     VTN Manager service.
     * @param ev      A {@link VtnPortEvent} instance.
     * @throws VTNException  An error occurred.
     */
    public void notifyNodeConnector(VTNManagerImpl mgr, VtnPortEvent ev)
        throws VTNException {
        ConcurrentMap<VTenantPath, Object> db = mgr.getStateDB();

        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            for (VBridgeImpl vbr: vBridges.values()) {
                vbr.notifyNodeConnector(mgr, db, ev);
            }

            for (VTerminalImpl vtm: vTerminals.values()) {
                vtm.notifyNodeConnector(mgr, db, ev);
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

            for (VTerminalImpl vtm: vTerminals.values()) {
                vtm.notifyConfiguration(mgr, listener);
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

            // Don't search vTerminals.
        } finally {
            rdlock.unlock();
        }
    }

    /**
     * Send a unicast ARP request to the specified host.
     *
     * @param mgr   VTN manager service.
     * @param ref   Reference to the virtual network mapping.
     *              A virtual node pointed by {@code ref} must be contained
     *              in this tenant.
     * @param pctx  The context of the ARP packet to send.
     * @return  {@code true} is returned if an ARP request was actually sent
     *          to the network, Otherwise {@code false} is returned.
     * @throws VTNException  An error occurred.
     */
    public boolean probeHost(VTNManagerImpl mgr, MapReference ref,
                             PacketContext pctx) throws VTNException {
        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            VNodePath path = ref.getPath();
            PortBridge bridge;
            if (path instanceof VBridgePath) {
                bridge = getBridgeImpl((VBridgePath)path);
            } else if (path instanceof VTerminalPath) {
                bridge = getTerminalImpl((VTerminalPath)path);
            } else {
                return false;
            }

            return bridge.probeHost(mgr, ref, pctx);
        } finally {
            rdlock.unlock();
        }
    }

    /**
     * Handler for receiving the packet.
     *
     * @param mgr   VTN manager service.
     * @param ref   Reference to the virtual network mapping.
     *              A virtual node pointed by {@code ref} must be contained
     *              in this tenant.
     * @param pctx  The context of the received packet.
     * @throws VTNException  An error occurred.
     */
    public void receive(VTNManagerImpl mgr, MapReference ref,
                        PacketContext pctx) throws VTNException {
        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            // Initialize flow timeout as specified in the virtual tenant
            // configuration.
            pctx.setFlowTimeout(tenantConfig.getIdleTimeout(),
                                tenantConfig.getHardTimeout());

            // Determine whether the packet is sent to the controller or not.
            boolean toCtlr = isToController(mgr, pctx);
            if (toCtlr) {
                pctx.setToController(toCtlr);
            }

            // Evaluate path maps.
            RouteResolver rr = new PathMapEvaluator(pctx).evaluate(tenantName);
            pctx.setRouteResolver(rr);

            // Evaluate VTN flow filters.
            flowFilters.evaluate(mgr, pctx, FlowFilterMap.VLAN_UNSPEC);

            VNodePath path = ref.getPath();
            PortBridge bridge;
            if (path instanceof VBridgePath) {
                bridge = getBridgeImpl((VBridgePath)path);
            } else if (path instanceof VTerminalPath) {
                bridge = getTerminalImpl((VTerminalPath)path);
            } else {
                return;
            }

            bridge.receive(mgr, ref, pctx);
        } catch (DropFlowException e) {
            // The given packet was discarded by a flow filter.
        } catch (RedirectFlowException e) {
            // The given packet was redirected by a flow filter.
            redirect(mgr, pctx, e);
        } finally {
            try {
                pctx.purgeObsoleteFlow();
            } finally {
                rdlock.unlock();
            }
        }
    }

    /**
     * Invoked when the recalculation of the all shortest path tree is done.
     *
     * @param mgr  VTN manager service.
     * @param ev   A {@link RoutingEvent} instance.
     */
    public void recalculateDone(VTNManagerImpl mgr, RoutingEvent ev) {
        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            for (VBridgeImpl vbr: vBridges.values()) {
                vbr.recalculateDone(mgr, ev);
            }
            for (VTerminalImpl vtm: vTerminals.values()) {
                vtm.recalculateDone(mgr, ev);
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
        ContainerConfig cfg = new ContainerConfig(containerName);
        cfg.delete(ContainerConfig.Type.TENANT, tenantName);

        Lock wrlock = rwLock.writeLock();
        wrlock.lock();
        try {
            // Destroy all bridges.
            for (Iterator<VBridgeImpl> it = vBridges.values().iterator();
                 it.hasNext();) {
                VBridgeImpl vbr = it.next();
                vbr.destroy(mgr, false);
                it.remove();
            }

            // Destroy all vTerminals.
            for (Iterator<VTerminalImpl> it = vTerminals.values().iterator();
                 it.hasNext();) {
                VTerminalImpl vtm = it.next();
                vtm.destroy(mgr, false);
                it.remove();
            }
        } finally {
            wrlock.unlock();
        }

        // Purge global timer task queue.
        Timer timer = mgr.getVTNProvider().getTimer();
        timer.purge();
    }

    /**
     * Update the state of the specified vBridge in this tenant.
     *
     * @param mgr    VTN Manager service.
     * @param path   Path to the target bridge.
     * @throws VTNException  The specified vBridge does not exist.
     */
    public void updateBridgeState(VTNManagerImpl mgr, VBridgePath path)
        throws VTNException {
        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            VBridgeImpl vbr = getBridgeImpl(path);
            vbr.update(mgr);
        } finally {
            rdlock.unlock();
        }
    }

    /**
     * Return the flow filter instance specified by the flow filter identifier.
     *
     * @param lstack  A {@link LockStack} instance to hold acquired locks.
     * @param fid     A {@link FlowFilterId} instance which specifies the
     *                flow filter list in the virtual node.
     * @param writer  {@code true} means the writer lock is required.
     * @return  A {@link FlowFilterMap} instance specified by {@code fid}.
     * @throws VTNException  An error occurred.
     */
    public FlowFilterMap getFlowFilterMap(LockStack lstack, FlowFilterId fid,
                                          boolean writer) throws VTNException {
        if (fid == null) {
            throw RpcException.getNullArgumentException("FlowFilterId");
        }

        VTenantPath path = fid.getPath();
        if (path instanceof VBridgePath) {
            VBridgePath bpath = (VBridgePath)path;
            lstack.push(rwLock.readLock());
            VBridgeImpl vbr = getBridgeImpl(bpath);
            return vbr.getFlowFilterMap(lstack, bpath, fid.isOutput(), writer);
        }
        if (path instanceof VTerminalIfPath) {
            VTerminalIfPath vipath = (VTerminalIfPath)path;
            lstack.push(rwLock.readLock());
            VTerminalImpl vtm = getTerminalImpl(vipath);
            return vtm.getFlowFilterMap(lstack, vipath, fid.isOutput(), writer);
        }

        if (path == null) {
            throw RpcException.getNullArgumentException("Virtual node path");
        }

        Lock lock = (writer) ? rwLock.writeLock() : rwLock.readLock();
        lstack.push(lock);
        return flowFilters;
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

        // Use copy of maps in order to avoid deadlock.
        Map<String, VBridgeImpl> otherBridges;
        Map<String, VTerminalImpl> otherTerminals;
        FlowFilterMap otherFilters;

        Lock rdlock = vtn.rwLock.readLock();
        rdlock.lock();
        try {
            otherBridges = (Map<String, VBridgeImpl>)
                ((TreeMap<String, VBridgeImpl>)vtn.vBridges).clone();
            otherTerminals = (Map<String, VTerminalImpl>)
                ((TreeMap<String, VTerminalImpl>)vtn.vTerminals).clone();
            otherFilters = vtn.flowFilters.clone();
        } finally {
            rdlock.unlock();
        }

        rdlock = rwLock.readLock();
        try {
            return (vBridges.equals(otherBridges) &&
                    vTerminals.equals(otherTerminals) &&
                    flowFilters.equals(otherFilters));
        } finally {
            rdlock.unlock();
        }
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
            h += (vBridges.hashCode() * 17) + (vTerminals.hashCode() * 19) +
                (flowFilters.hashCode() * 47);
        } finally {
            rdlock.unlock();
        }

        return h;
    }

    /**
     * Remove MAC address table entries relevant to the specified pair of
     * switch port and VLAN ID from all existing MAC address tables.
     *
     * <p>
     *   This method must be called with holding the tenant lock.
     * </p>
     *
     * @param mgr   VTN Manager service.
     * @param port  A node connector associated with a switch port.
     * @param vlan  A VLAN ID.
     */
    void removeMacTableEntries(VTNManagerImpl mgr, NodeConnector port,
                               short vlan) {
        for (VBridgeImpl vbr: vBridges.values()) {
            VBridgePath path = vbr.getPath();
            MacAddressTable table = mgr.getMacAddressTable(path);
            table.flush(port, vlan);
        }
    }

    /**
     * Remove MAC address table entries relevant to the specified network
     * from all existing MAC address tables.
     *
     * <p>
     *   The method removes all MAC address table entries which meet all
     *   the following conditions.
     * </p>
     * <ul>
     *   <li>
     *     The specified VLAN ID is configured in the entry.
     *   </li>
     *   <li>
     *     A {@link NodeConnector} instance in the entry is accepted by
     *     {@link PortFilter} instance passed to {@code filter}.
     *   </li>
     * </ul>
     * <p>
     *   This method must be called with holding the tenant lock.
     * </p>
     *
     * @param mgr     VTN Manager service.
     * @param filter  A {@link PortFilter} instance which selects switch ports.
     * @param vlan    A VLAN ID.
     */
    void removeMacTableEntries(VTNManagerImpl mgr, PortFilter filter,
                               short vlan) {
        for (VBridgeImpl vbr: vBridges.values()) {
            VBridgePath path = vbr.getPath();
            MacAddressTable table = mgr.getMacAddressTable(path);
            table.flush(filter, vlan);
        }
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
            throw RpcException.getBadArgumentException("Invalid idle timeout");
        }
        if (hard > MAX_FLOW_TIMEOUT) {
            throw RpcException.getBadArgumentException("Invalid hard timeout");
        }
        if (idle != 0 && hard != 0 && idle >= hard) {
            throw RpcException.getBadArgumentException(
                "Idle timeout must be less than hard timeout");
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
    @SuppressWarnings("unused")
    private void readObject(ObjectInputStream in)
        throws IOException, ClassNotFoundException {
        // Read serialized fields.
        // Note that the lock does not need to be acquired here because this
        // instance is not yet visible.
        in.defaultReadObject();

        // Reset the lock.
        rwLock = new ReentrantReadWriteLock();

        // Read the number of vBridges.
        int size = in.readInt();

        // Read vBridges.
        vBridges = new TreeMap<String, VBridgeImpl>();
        for (int i = 0; i < size; i++) {
            String name = (String)in.readObject();
            VBridgeImpl vbr = (VBridgeImpl)in.readObject();

            // Set this tenant as parent of this vBridge.
            vbr.setPath(this, name);
            vBridges.put(name, vbr);
        }

        // Read the number of vTerminals.
        size = in.readInt();

        // Read vTerminals.
        vTerminals = new TreeMap<String, VTerminalImpl>();
        for (int i = 0; i < size; i++) {
            String name = (String)in.readObject();
            VTerminalImpl vtm = (VTerminalImpl)in.readObject();

            // Set this tenant as parent of this vTerminal.
            vtm.setPath(this, name);
            vTerminals.put(name, vtm);
        }

        // Read flow filters.
        flowFilters = (FlowFilterMap)in.readObject();
        flowFilters.setParent(this);
    }

    /**
     * Serialize this object and write it to the given output stream.
     *
     * @param out  An output stream.
     * @throws IOException
     *    An I/O error occurred.
     */
    @SuppressWarnings("unused")
    private void writeObject(ObjectOutputStream out) throws IOException {
        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            // Write serialized fields.
            out.defaultWriteObject();

            // Write the number of vBridges.
            out.writeInt(vBridges.size());

            // Write vBridges.
            for (Map.Entry<String, VBridgeImpl> entry: vBridges.entrySet()) {
                out.writeObject(entry.getKey());
                out.writeObject(entry.getValue());
            }

            // Write the number of vTerminals.
            out.writeInt(vTerminals.size());

            // Write vTerminals.
            for (Map.Entry<String, VTerminalImpl> entry:
                     vTerminals.entrySet()) {
                out.writeObject(entry.getKey());
                out.writeObject(entry.getValue());
            }

            // Write flow filters.
            out.writeObject(flowFilters);
        } finally {
            rdlock.unlock();
        }
    }

    /**
     * Return an exception that indicates the specified bridge does not
     * exist.
     *
     * @param bridgeName  The name of the bridge.
     * @return  A {@link RpcException} instance.
     */
    private RpcException getBridgeNotFoundException(String bridgeName) {
        String msg = bridgeName + ": Bridge does not exist";
        return RpcException.getNotFoundException(msg);
    }

    /**
     * Return an exception status that indicates the specified vTerminal does
     * not exist.
     *
     * @param termName  The name of the vTerminal.
     * @return  A {@link RpcException} instance.
     */
    private RpcException getTerminalNotFoundException(String termName) {
        String msg = termName + ": vTerminal does not exist";
        return RpcException.getNotFoundException(msg);
    }

    /**
     * Return the virtual bridge instance associated with the given name.
     *
     * <p>
     *   This method must be called with holding the tenant lock.
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
            throw RpcException.getNullArgumentException("Bridge name");
        }

        VBridgeImpl vbr = vBridges.get(bridgeName);
        if (vbr == null) {
            throw getBridgeNotFoundException(bridgeName);
        }

        return vbr;
    }

    /**
     * Return the vTerminal instance associated with the given name.
     *
     * <p>
     *   This method must be called with holding the tenant lock.
     * </p>
     *
     * @param path  Path to the vTerminal.
     * @return  Virtual terminal instance is returned.
     * @throws VTNException  An error occurred.
     * @throws NullPointerException  {@code path} is {@code null}.
     */
    private VTerminalImpl getTerminalImpl(VTerminalPath path)
        throws VTNException {
        String termName = path.getTerminalName();
        if (termName == null) {
            throw RpcException.getNullArgumentException("vTerminal name");
        }

        VTerminalImpl vtm = vTerminals.get(termName);
        if (vtm == null) {
            throw getTerminalNotFoundException(termName);
        }

        return vtm;
    }

    /**
     * Return the MAC address table for the specified virtual bridge.
     *
     * <p>
     *   This method must be called with holding the tenant lock.
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
        RpcException e = (bridgeName == null)
            ? RpcException.getNullArgumentException("Bridge name")
            : getBridgeNotFoundException(bridgeName);

        throw e;
    }

    /**
     * Save tenant configuration to the configuration file.
     *
     * <p>
     *   This method must be called with holding the tenant lock.
     * </p>
     *
     * @param mgr  VTN Manager service.
     *             If a non-{@code null} value is specified, this method checks
     *             whether the current configuration is applied correctly.
     * @throws VTNException  An error occurred.
     */
    public void saveConfigImpl(VTNManagerImpl mgr) throws VTNException {
        ContainerConfig cfg = new ContainerConfig(containerName);
        if (mgr != null) {
            // Adjust interval of MAC address table aging.
            for (VBridgeImpl vbr: vBridges.values()) {
                vbr.initMacTableAging(mgr);
            }
        }

        Status status = cfg.save(ContainerConfig.Type.TENANT, tenantName,
                                 this);
        if (status.isSuccess()) {
            if (LOG.isTraceEnabled()) {
                LOG.trace("{}:{}: Tenant was saved",
                          containerName, tenantName);
            }
            return;
        }

        String msg = "Failed to save tenant configuration";
        LOG.error("{}:{}: {}: {}", containerName, tenantName, msg, status);
        throw new VTNException(msg);
    }

    /**
     * Determine whether the given packet is sent to the controller or not.
     *
     * @param mgr   VTN Manager service.
     * @param pctx  The context of the received packet.
     * @return  {@code true} is returned if this packet is sent to the
     *          controller. Otherwise {@code false} is returned.
     */
    private boolean isToController(VTNManagerImpl mgr, PacketContext pctx) {
        EtherAddress ctlrMac = mgr.getVTNConfig().getControllerMacAddress();
        EtherAddress dst = pctx.getDestinationAddress();
        return (ctlrMac.getAddress() == dst.getAddress());
    }

    /**
     * Handle packet redirection caused by the REDIRECT flow filter.
     *
     * <p>
     *   This method must be called with holding the tenant lock.
     * </p>
     *
     * @param mgr   VTN Manager service.
     * @param pctx  The context of the received packet.
     * @param rex   An exception that keeps information about the packet
     *              redirection.
     */
    private void redirect(VTNManagerImpl mgr, PacketContext pctx,
                          RedirectFlowException rex) {
        RedirectFlowException current = rex;
        while (true) {
            try {
                redirectImpl(mgr, pctx, current);
                return;
            } catch (DropFlowException e) {
                // The given packet was discarded by a flow filter.
                RedirectFlowException first = pctx.getFirstRedirection();
                Logger logger = first.getLogger();
                logger.warn("{}: Packet was discarded: packet={}",
                            first.getLogPrefix(), pctx.getDescription());

                return;
            } catch (RedirectFlowException e) {
                current = e;
            }
        }
    }

    /**
     * Handle packet redirection caused by the REDIRECT flow filter.
     *
     * <p>
     *   This is an internal method only for
     *   {@link #redirect(VTNManagerImpl,PacketContext,RedirectFlowException)}.
     * </p>
     *
     * @param mgr   VTN Manager service.
     * @param pctx  The context of the received packet.
     * @param rex   An exception that keeps information about the packet
     *              redirection.
     * @throws DropFlowException
     *    The given packet was discarded by a DROP flow filter.
     * @throws RedirectFlowException
     *    The given packet was redirected by a REDIRECT flow filter.
     */
    private void redirectImpl(VTNManagerImpl mgr, PacketContext pctx,
                              RedirectFlowException rex)
        throws DropFlowException, RedirectFlowException {
        // Determine the destination of the redirection.
        VInterfacePath path = rex.getDestination();
        PortBridge bridge;
        try {
            if (path instanceof VBridgeIfPath) {
                bridge = getBridgeImpl((VBridgeIfPath)path);
            } else if (path instanceof VTerminalIfPath) {
                bridge = getTerminalImpl((VTerminalIfPath)path);
            } else {
                // This should never happen.
                rex.destinationNotFound(pctx, "Unexpected destination path");
                throw new DropFlowException();
            }
        } catch (VTNException e) {
            String emsg = e.getMessage();
            rex.destinationNotFound(pctx, emsg);
            throw new DropFlowException(e);
        }

        bridge.redirect(mgr, pctx, rex);
    }

    // FlowFilterNode

    /**
     * Return the name of the container to which the tenant belongs.
     *
     * @return  The name of the container.
     */
    public String getContainerName() {
        return containerName;
    }

    /**
     * Return path to this node.
     *
     * @return  Path to the node.
     */
    @Override
    public VTenantPath getPath() {
        return new VTenantPath(tenantName);
    }
}
