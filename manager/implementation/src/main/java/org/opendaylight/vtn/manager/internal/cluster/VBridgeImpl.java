/*
 * Copyright (c) 2013-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
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
import java.util.HashSet;
import java.util.concurrent.ConcurrentMap;
import java.util.concurrent.locks.Lock;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import org.opendaylight.vtn.manager.DataLinkHost;
import org.opendaylight.vtn.manager.IVTNManagerAware;
import org.opendaylight.vtn.manager.MacAddressEntry;
import org.opendaylight.vtn.manager.MacMap;
import org.opendaylight.vtn.manager.MacMapConfig;
import org.opendaylight.vtn.manager.VBridge;
import org.opendaylight.vtn.manager.VBridgeConfig;
import org.opendaylight.vtn.manager.VBridgeIfPath;
import org.opendaylight.vtn.manager.VBridgePath;
import org.opendaylight.vtn.manager.VInterfaceConfig;
import org.opendaylight.vtn.manager.VInterfacePath;
import org.opendaylight.vtn.manager.VNodePath;
import org.opendaylight.vtn.manager.VNodeRoute;
import org.opendaylight.vtn.manager.VTNException;
import org.opendaylight.vtn.manager.VTenantPath;
import org.opendaylight.vtn.manager.VlanMap;
import org.opendaylight.vtn.manager.VlanMapConfig;
import org.opendaylight.vtn.manager.util.EtherAddress;

import org.opendaylight.vtn.manager.internal.LockStack;
import org.opendaylight.vtn.manager.internal.MacAddressTable;
import org.opendaylight.vtn.manager.internal.PacketContext;
import org.opendaylight.vtn.manager.internal.TxContext;
import org.opendaylight.vtn.manager.internal.VTNManagerImpl;
import org.opendaylight.vtn.manager.internal.VTNThreadData;
import org.opendaylight.vtn.manager.internal.inventory.VtnNodeEvent;
import org.opendaylight.vtn.manager.internal.inventory.VtnPortEvent;
import org.opendaylight.vtn.manager.internal.util.MiscUtils;
import org.opendaylight.vtn.manager.internal.util.ProtocolUtils;
import org.opendaylight.vtn.manager.internal.util.inventory.InventoryReader;
import org.opendaylight.vtn.manager.internal.util.inventory.NodeUtils;
import org.opendaylight.vtn.manager.internal.util.inventory.SalPort;

import org.opendaylight.controller.sal.core.Node;
import org.opendaylight.controller.sal.core.NodeConnector;
import org.opendaylight.controller.sal.core.UpdateType;
import org.opendaylight.controller.sal.packet.address.DataLinkAddress;
import org.opendaylight.controller.sal.utils.Status;
import org.opendaylight.controller.sal.utils.StatusCode;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.VirtualRouteReason;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeState;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnAclType;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateOperationType;

/**
 * Implementation of vBridge (virtual layer 2 bridge).
 *
 * <p>
 *   Although this class is public to other packages, this class does not
 *   provide any API. Applications other than VTN Manager must not use this
 *   class.
 * </p>
 */
public final class VBridgeImpl extends PortBridge<VBridgeIfImpl>
    implements FlowFilterNode {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = 2720765815523032298L;

    /**
     * Logger instance.
     */
    private static final Logger  LOG =
        LoggerFactory.getLogger(VBridgeImpl.class);

    /**
     * Default interval of MAC address table aging.
     */
    private static final int  DEFAULT_AGE_INTERVAL = 600;

    /**
     * Maximum interval of MAC address table aging.
     */
    private static final int  MAX_AGE_INTERVAL = 1000000;

    /**
     * Minimum interval of MAC address table aging.
     */
    private static final int  MIN_AGE_INTERVAL = 10;

    /**
     * Pseudo node identifier which indicates that the node is unspecified.
     */
    private static final String  NODEID_ANY = "ANY";

    /**
     * Configuration for the bridge.
     */
    private VBridgeConfig  bridgeConfig;

    /**
     * VLAN mappings applied to this bridge.
     */
    private final Map<String, VlanMapImpl> vlanMaps =
        new TreeMap<String, VlanMapImpl>();

    /**
     * MAC mapping applied to this bridge.
     */
    private MacMapImpl  macMap;

    /**
     * Flow filters for incoming packets.
     */
    private final FlowFilterMap  inFlowFilters;

    /**
     * Flow filters for outgoing packets.
     */
    private final FlowFilterMap  outFlowFilters;

    /**
     * Construct a vBridge instance.
     *
     * @param vtn   The virtual tenant to which a new vBridge belongs.
     * @param name  The name of the vBridge.
     * @param bconf Configuration for the vBridge.
     * @throws VTNException  An error occurred.
     */
    VBridgeImpl(VTenantImpl vtn, String name, VBridgeConfig bconf)
        throws VTNException {
        super(vtn, name);
        VBridgeConfig cf = resolve(bconf);
        checkConfig(cf);
        bridgeConfig = cf;
        inFlowFilters = FlowFilterMap.createIncoming(this);
        outFlowFilters = FlowFilterMap.createOutgoing(this);
    }

    /**
     * Return information about the vBridge.
     *
     * @param mgr  VTN Manager service.
     * @return  Information about the vBridge.
     */
    VBridge getVBridge(VTNManagerImpl mgr) {
        Lock rdlock = readLock();
        try {
            return getVBridge(mgr, getName(), bridgeConfig);
        } finally {
            rdlock.unlock();
        }
    }

    /**
     * Set vBridge configuration.
     *
     * @param mgr    VTN Manager service.
     * @param bconf  vBridge configuration.
     * @param all    If {@code true} is specified, all attributes of the
     *               vBridge are modified. In this case, {@code null} in
     *               {@code bconf} is interpreted as default value.
     *               If {@code false} is specified, an attribute is not
     *               modified if its value in {@code bconf} is {@code null}.
     * @return  {@code true} if the configuration is actually changed.
     *          Otherwise {@code false}.
     * @throws VTNException  An error occurred.
     */
    boolean setVBridgeConfig(VTNManagerImpl mgr, VBridgeConfig bconf,
                             boolean all)
        throws VTNException {
        Lock wrlock = writeLock();
        try {
            VBridgeConfig cf = (all) ? resolve(bconf) : merge(bconf);
            if (cf.equals(bridgeConfig)) {
                return false;
            }

            checkConfig(cf);
            bridgeConfig = cf;
            VBridgePath path = getPath();
            String name = path.getBridgeName();
            VBridge vbridge = getVBridge(mgr, name, cf);
            VBridgeEvent.changed(mgr, path, vbridge, true);

            initMacTableAging(mgr);
            return true;
        } finally {
            wrlock.unlock();
        }
    }

    /**
     * Add VLAN mapping to this vBridge.
     *
     * @param mgr     VTN Manager serivce.
     * @param ctx     MD-SAL datastore transaction context.
     * @param vlconf  Configuration for the VLAN mapping.
     * @return  Information about the added VLAN mapping is returned.
     * @throws VTNException  An error occurred.
     */
    VlanMap addVlanMap(VTNManagerImpl mgr, TxContext ctx, VlanMapConfig vlconf)
        throws VTNException {
        if (vlconf == null) {
            Status status = MiscUtils.
                argumentIsNull("VLAN mapping configiguration");
            throw new VTNException(status);
        }

        short vlan = vlconf.getVlan();
        ProtocolUtils.checkVlan(vlan);

        // Create ID for this VLAN mapping.
        Node node = vlconf.getNode();
        if (node != null) {
            NodeUtils.checkNode(node);
        }
        String id = createVlanMapId(node, vlan);

        // Create a VLAN mapping instance.
        VBridgePath path = getPath();
        VlanMapImpl vmap = new VlanMapImpl(path, id, vlconf);
        NodeVlan nvlan = new NodeVlan(node, vlan);

        Lock wrlock = writeLock();
        try {
            // Register a new VLAN mapping to the resource manager.
            vmap.register(mgr, ctx, path, nvlan);
            vlanMaps.put(id, vmap);

            VlanMap vlmap = new VlanMap(id, node, vlan);
            VlanMapEvent.added(mgr, path, vlmap);
            if (vmap.isValid(mgr.getStateDB())) {
                updateState(mgr);
            } else {
                setState(mgr, VnodeState.DOWN);
            }
            return vlmap;
        } finally {
            wrlock.unlock();
        }
    }

    /**
     * Remove VLAN mapping from this vBridge.
     *
     * @param mgr     VTN Manager serivce.
     * @param mapId   The identifier of the VLAN mapping.
     * @throws VTNException  An error occurred.
     */
    void removeVlanMap(VTNManagerImpl mgr, String mapId)
        throws VTNException {
        if (mapId == null) {
            Status status = MiscUtils.argumentIsNull("Mapping ID");
            throw new VTNException(status);
        }

        Lock wrlock = writeLock();
        try {
            VlanMapImpl vmap = vlanMaps.remove(mapId);
            if (vmap == null) {
                Status status = vlanMapNotFound(mapId);
                throw new VTNException(status);
            }

            // Destroy VLAN mapping.
            vmap.destroy(mgr, getPath(), true);
            updateState(mgr);
        } finally {
            wrlock.unlock();
        }
    }

    /**
     * Return a list of VLAN mappings in the vBridge.
     *
     * @return  A list of VLAN mapping information.
     */
    List<VlanMap> getVlanMaps() {
        Lock rdlock = readLock();
        try {
            List<VlanMap> list = new ArrayList<VlanMap>(vlanMaps.size());
            for (Map.Entry<String, VlanMapImpl> entry: vlanMaps.entrySet()) {
                String id = entry.getKey();
                VlanMapImpl vmap = entry.getValue();
                VlanMapConfig vlconf = vmap.getVlanMapConfig();
                VlanMap vlmap = new VlanMap(id, vlconf.getNode(),
                                            vlconf.getVlan());
                list.add(vlmap);
            }

            return list;
        } finally {
            rdlock.unlock();
        }
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
            Status status = MiscUtils.argumentIsNull("Mapping ID");
            throw new VTNException(status);
        }

        Lock rdlock = readLock();
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
     * Return information about the VLAN mapping associated with the
     * specified VLAN mapping configuration.
     *
     * @param vlconf  VLAN mapping configuration.
     * @return  VLAN mapping information associated with the given VLAN mapping
     *          configuration.
     * @throws VTNException  An error occurred.
     */
    VlanMap getVlanMap(VlanMapConfig vlconf) throws VTNException {
        if (vlconf == null) {
            Status status = MiscUtils.
                argumentIsNull("VLAN map configiguration");
            throw new VTNException(status);
        }

        Node node = vlconf.getNode();
        short vlan = vlconf.getVlan();
        String id = createVlanMapId(node, vlan);

        return getVlanMap(id);
    }

    /**
     * Return information about the MAC mapping configured in this vBridge.
     *
     * @param mgr   VTN Manager service.
     * @return  A {@link MacMap} object which represents information about
     *          the MAC mapping configured in this vBridge.
     *          {@code null} is returned if the MAC mapping is not configured
     *          in this vBridge.
     * @throws VTNException  An error occurred.
     */
    MacMap getMacMap(VTNManagerImpl mgr) throws VTNException {
        Lock rdlock = readLock();
        try {
            MacMapImpl mmap = macMap;
            return (mmap == null) ? null : mmap.getMacMap(mgr);
        } finally {
            rdlock.unlock();
        }
    }

    /**
     * Return configuration information about MAC mapping in the specified
     * vBridge.
     *
     * @param aclType  The type of access control list.
     * @return  A set of {@link DataLinkHost} instances which contains host
     *          information in the specified access control list is returned.
     *          {@code null} is returned if MAC mapping is not configured in
     *          the specified vBridge.
     * @throws VTNException  An error occurred.
     */
    Set<DataLinkHost> getMacMapConfig(VtnAclType aclType)
        throws VTNException {
        Lock rdlock = readLock();
        try {
            MacMapImpl mmap = macMap;
            return (mmap == null) ? null : mmap.getMacMapConfig(aclType);
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
     * @return  A list of {@link MacAddressEntry} instances corresponding to
     *          all the MAC address information actually mapped to the vBridge
     *          specified by {@code path}.
     *          {@code null} is returned if MAC mapping is not configured
     *          in the specified vBridge.
     * @throws VTNException  An error occurred.
     */
    List<MacAddressEntry> getMacMappedHosts(VTNManagerImpl mgr)
        throws VTNException {
        Lock rdlock = readLock();
        try {
            MacMapImpl mmap = macMap;
            return (mmap == null) ? null : mmap.getMacMappedHosts(mgr);
        } finally {
            rdlock.unlock();
        }
    }

    /**
     * Determine whether the host specified by the MAC address is actually
     * mapped by MAC mapping configured in the specified vBridge.
     *
     * @param mgr   VTN Manager service.
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
    MacAddressEntry getMacMappedHost(VTNManagerImpl mgr, DataLinkAddress addr)
        throws VTNException {
        Lock rdlock = readLock();
        try {
            MacMapImpl mmap = macMap;
            return (mmap == null) ? null : mmap.getMacMappedHost(mgr, addr);
        } finally {
            rdlock.unlock();
        }
    }

    /**
     * Change MAC mapping configuration as specified by {@link MacMapConfig}
     * instance.
     *
     * @param mgr     VTN Manager service.
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
    UpdateType setMacMap(VTNManagerImpl mgr, VtnUpdateOperationType op,
                         MacMapConfig mcconf) throws VTNException {
        Lock wrlock = writeLock();
        try {
            MacMapImpl mmap = prepareMacMap();
            MacMapConfig newconf = mmap.setMacMap(mgr, op, mcconf);
            return commitMacMap(mgr, mmap, newconf);
        } finally {
            wrlock.unlock();
        }
    }

    /**
     * Change the access controll list for the specified MAC mapping.
     *
     * @param mgr       VTN Manager service.
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
    UpdateType setMacMap(VTNManagerImpl mgr, VtnUpdateOperationType op,
                         VtnAclType aclType,
                         Set<? extends DataLinkHost> dlhosts)
        throws VTNException {
        Lock wrlock = writeLock();
        try {
            MacMapImpl mmap = prepareMacMap();
            MacMapConfig newconf = mmap.setMacMap(mgr, op, aclType, dlhosts);
            return commitMacMap(mgr, mmap, newconf);
        } finally {
            wrlock.unlock();
        }
    }

    /**
     * Initialize MAC address table aging.
     *
     * @param mgr  VTN Manager service.
     */
    void initMacTableAging(VTNManagerImpl mgr) {
        Lock rdlock = readLock();
        try {
            int age = bridgeConfig.getAgeInterval();
            VBridgePath path = getPath();
            MacAddressTable table = mgr.getMacAddressTable(path);
            if (table == null) {
                mgr.addMacAddressTable(path, age);
            } else {
                table.setAgeInterval(age);
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
        Lock rdlock = readLock();
        try {
            // Flood the specified ARP request.
            flood(mgr, pctx);
        } finally {
            rdlock.unlock();
        }
    }

    /**
     * Scan mappings configured in this vBridge, and determine current state
     * of this vBridge.
     *
     * @param mgr   VTN Manager service.
     */
    void update(VTNManagerImpl mgr) {
        Lock wrlock = writeLock();
        try {
            updateState(mgr);
        } catch (Exception e) {
            // This should never happen.
            mgr.logException(LOG, getNodePath(), e);
        } finally {
            wrlock.unlock();
        }
    }

    /**
     * Return the specified flow filter instance.
     *
     * @param lstack  A {@link LockStack} instance to hold acquired locks.
     * @param path    A path to the target virtual node.
     * @param out     {@code true} means that the outgoing flow filter.
     *                {@code false} means that the incoming flow filter.
     * @param writer  {@code true} means the writer lock is required.
     * @return  A {@link FlowFilterMap} instance.
     * @throws VTNException  An error occurred.
     */
    FlowFilterMap getFlowFilterMap(LockStack lstack, VBridgePath path,
                                   boolean out, boolean writer)
        throws VTNException {
        if (path instanceof VBridgeIfPath) {
            VInterfacePath ipath = (VInterfacePath)path;
            return getFlowFilterMap(lstack, ipath, out, writer);
        }

        lstack.push(getLock(writer));
        return (out) ? outFlowFilters : inFlowFilters;
    }

    /**
     * Merge the given vBridge configuration to the current configuration.
     *
     * <p>
     *   If at least one field in {@code bconf} keeps a valid value, this
     *   method creates a shallow copy of the current configuration, and set
     *   valid values in {@code bconf} to the copy.
     * </p>
     * <p>
     *   Note that this method must be called with holding the bridge write
     *   lock.
     * </p>
     *
     * @param bconf  Configuration to be merged.
     * @return  A merged {@code VBridgeConfig} object.
     */
    private VBridgeConfig merge(VBridgeConfig bconf) {
        String desc = bconf.getDescription();
        int age = bconf.getAgeInterval();
        if (desc == null) {
            return (age < 0)
                ? bridgeConfig
                : new VBridgeConfig(bridgeConfig.getDescription(), age);
        } else if (age < 0) {
            return new VBridgeConfig(desc, bridgeConfig.getAgeInterval());
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
            return new VBridgeConfig(bconf.getDescription(),
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
     * Return a {@link MacMapImpl} instance to change the MAC mapping
     * configuration.
     *
     * <p>
     *   Note that this method must be called with holding the bridge write
     *   lock.
     * </p>
     *
     * @return    A {@link MacMapImpl} instance.
     */
    private MacMapImpl prepareMacMap() {
        MacMapImpl mmap = macMap;
        if (mmap == null) {
            mmap = new MacMapImpl(getPath());
        }

        return mmap;
    }

    /**
     * Install new MAC mapping configuration.
     *
     * <p>
     *   Note that this method must be called with holding the bridge write
     *   lock.
     * </p>
     *
     * @param mgr      VTN Manager service.
     * @param mmap     A {@link MacMapImpl} instance.
     * @param newconf  A {@link MacMapConfig} instance to be delivered as
     *                 manager event. {@code null} must be specified if the
     *                 the MAC mapping configuration was not changed.
     * @return         A {@link UpdateType} instance to be returned.
     */
    private UpdateType commitMacMap(VTNManagerImpl mgr, MacMapImpl mmap,
                                    MacMapConfig newconf) {
        if (newconf == null) {
            return null;
        }

        UpdateType type;
        VBridgePath path = getPath();
        if (macMap == null) {
            assert !mmap.isEmpty();
            MacMapEvent.added(mgr, path, newconf);
            macMap = mmap;
            type = UpdateType.ADDED;
        } else if (mmap.isEmpty()) {
            mmap.destroy(mgr, path, newconf, true);
            macMap = null;
            type = UpdateType.REMOVED;
        } else {
            assert macMap == mmap;
            MacMapEvent.changed(mgr, path, newconf);
            type = UpdateType.CHANGED;
        }

        updateState(mgr);

        return type;
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
        Lock rdlock = readLock();
        try {
            out.defaultWriteObject();
        } finally {
            rdlock.unlock();
        }
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
     * Return information about the vBridge.
     *
     * <p>
     *   Note that this method must be called with holding the bridge write
     *   lock.
     * </p>
     *
     * @param mgr    VTN Manager service.
     * @param name   The name of the bridge.
     * @param bconf  Bridge configuration.
     * @return  Information about the bridge.
     */
    private VBridge getVBridge(VTNManagerImpl mgr, String name,
                               VBridgeConfig bconf) {
        BridgeState bst = getBridgeState(mgr);
        int faulted = bst.getFaultedPathSize();
        return new VBridge(name, bst.getState(), faulted, bconf);
    }

    /**
     * Return the MAC address table entry associated with the destination
     * address of the received packet.
     *
     * <p>
     *   Note that this method may flood the received packet to this bridge
     *   if needed.
     * </p>
     *
     * @param mgr    VTN Manager service.
     * @param pctx   The context of the received packet.
     * @param table  The MAC address table for this bridge.
     * @return  A {@link MacTableEntry} object is returned if found.
     *          {@code null} is returned if not found or if the received packet
     *          should not be sent to this bridge.
     * @throws DropFlowException
     *    The given packet was discarded by a flow filter.
     * @throws RedirectFlowException
     *    The given packet was redirected by a flow filter.
     * @throws VTNException
     *    An error occurred.
     */
    private MacTableEntry getDestination(VTNManagerImpl mgr,
                                         PacketContext pctx,
                                         MacAddressTable table)
        throws DropFlowException, RedirectFlowException, VTNException {
        EtherAddress dst = pctx.getDestinationAddress();
        if (!dst.isUnicast()) {
            // Flood the non-unicast packet.
            flood(mgr, pctx);
            return null;
        }

        Long key = Long.valueOf(dst.getAddress());
        MacTableEntry tent = table.get(key);
        if (tent == null) {
            // Flood the received packet.
            flood(mgr, pctx);
            return null;
        }

        // Ensure that the outgoing network is mapped to this bridge.
        NodeConnector outgoing = tent.getPort();
        long mac = key.longValue();
        short outVlan = tent.getVlan();
        VBridgeNode bnode = match(mgr, mac, outgoing, outVlan);
        if (bnode == null) {
            LOG.warn("{}:{}: Unexpected MAC address entry: {}",
                     getContainerName(), getNodePath(), tent);
            pctx.addObsoleteEntry(tent);
            table.remove(key);
            flood(mgr, pctx);
            return null;
        }

        SalPort egress = SalPort.create(outgoing);
        if (!bnode.isEnabled()) {
            VBridgePath bpath = bnode.getPath();
            if (LOG.isDebugEnabled()) {
                LOG.debug("{}:{}: Drop packet due to disabled network: {}",
                          getContainerName(), bpath,
                          pctx.getDescription(egress));
            }
            return null;
        }

        InventoryReader reader = pctx.getTxContext().getInventoryReader();
        if (!reader.isEnabled(egress)) {
            if (LOG.isWarnEnabled()) {
                LOG.warn("{}:{}: Drop packet because outgoing port is down: " +
                         "{}", getContainerName(), getNodePath(),
                         pctx.getDescription(egress));
            }
            return null;
        }

        VNodePath mapPath = bnode.getPath(mac, outVlan);
        pctx.setEgressVNodeRoute(new VNodeRoute(mapPath,
                                                VirtualRouteReason.FORWARDED));

        // Evaluate flow filters for outgoing packets.
        // Note that this should never clone a PacketContext.
        bnode.filterPacket(mgr, pctx, outVlan, this);

        return tent;
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
        // From here, REDIRECT flow filters are ignored.
        pctx.setFlooding(true);

        RedirectFlowException rex = pctx.getFirstRedirection();
        VNodePath bpath = getNodePath();
        if (rex != null) {
            rex.flooded(mgr, pctx, bpath);
        }

        try {
            // Don't send the packet to the incoming network.
            Set<PortVlan> sent = new HashSet<PortVlan>();
            PortVlan innw = pctx.getIncomingNetwork();
            if (innw != null) {
                sent.add(innw);
            }

            // Forward packet to the network established by the port mapping.
            for (VBridgeIfImpl vif: getInterfaceMap().values()) {
                vif.transmit(mgr, pctx, sent);
            }

            // Forward packet to the network established by the MAC mapping.
            MacMapImpl mmap = macMap;
            if (mmap != null) {
                mmap.transmit(mgr, pctx, this, sent);
            }

            // Forward packet to the network established by the VLAN mapping.
            for (VlanMapImpl vmap: vlanMaps.values()) {
                vmap.transmit(mgr, pctx, this, sent);
            }

            if (LOG.isDebugEnabled() && sent.size() == 1 &&
                sent.contains(innw)) {
                SalPort sport = SalPort.create(innw.getNodeConnector());
                LOG.debug("{}:{}: No packet was broadcasted: {}",
                          getContainerName(), bpath,
                          pctx.getDescription(sport));
            }
        } catch (RedirectFlowException e) {
            // This should never happen.
            mgr.logException(LOG, bpath, e);
        }
    }

    /**
     * Create an identifier for the VLAN mapping.
     *
     * @param node  A {@link Node} object corresponding to the physical switch.
     * @param vlan  A VLAN ID to be mapped.
     * @return  A string which represents an identifier of the VLAN mapping.
     */
    private String createVlanMapId(Node node, short vlan) {
        StringBuilder idBuilder = new StringBuilder();
        if (node == null) {
            // Node is unspecified.
            idBuilder.append(NODEID_ANY);
        } else {
            idBuilder.append(node.getType()).append('-').
                append(node.getNodeIDString());
        }

        idBuilder.append('.').append((int)vlan);

        return idBuilder.toString();
    }

    // VTenantNode

    /**
     * Set virtual bridge path.
     *
     * @param vtn   Virtual tenant which contains this bridge.
     * @param name  The name of this bridge.
     */
    @Override
    void setPath(VTenantImpl vtn, String name) {
        super.setPath(vtn, name);
        VBridgePath path = getPath();

        // Initialize MAC mapping path.
        MacMapImpl mmap = macMap;
        if (mmap != null) {
            mmap.setPath(path);
        }

        // Initialize VLAN mapping path.
        for (Map.Entry<String, VlanMapImpl> entry: vlanMaps.entrySet()) {
            String mapId = entry.getKey();
            VlanMapImpl vmap = entry.getValue();
            vmap.setPath(path, mapId);
        }

        // Initialize parent path for flow filter map.
        inFlowFilters.setParent(this);
        outFlowFilters.setParent(this);
    }

    // AbstractBridge

    /**
     * Return path to this bridge.
     *
     * @return  Path to this bridge.
     */
    @Override
    public VBridgePath getPath() {
        return (VBridgePath)getNodePath();
    }

    /**
     * Notify the listener of current configuration.
     *
     * @param mgr       VTN Manager service.
     * @param listener  VTN manager listener service.
     */
    @Override
    void notifyConfiguration(VTNManagerImpl mgr, IVTNManagerAware listener) {
        VBridgePath path = getPath();
        UpdateType type = UpdateType.ADDED;
        Lock rdlock = readLock();
        try {
            VBridge vbridge = getVBridge(mgr);
            mgr.notifyChange(listener, path, vbridge, type);

            notifyIfConfig(mgr, listener);

            for (Map.Entry<String, VlanMapImpl> entry: vlanMaps.entrySet()) {
                String id = entry.getKey();
                VlanMapImpl vmap = entry.getValue();
                VlanMapConfig vlconf = vmap.getVlanMapConfig();
                VlanMap vlmap = new VlanMap(id, vlconf.getNode(),
                                            vlconf.getVlan());
                mgr.notifyChange(listener, path, vlmap, type);
            }

            MacMapImpl mmap = macMap;
            if (mmap != null) {
                MacMapConfig mcconf = mmap.getMacMapConfig();
                mgr.notifyChange(listener, path, mcconf, type);
            }
        } finally {
            rdlock.unlock();
        }
    }

    /**
     * Destroy the vBridge.
     *
     * @param mgr     VTN manager service.
     * @param retain  {@code true} means that the parent tenant will be
     *                retained. {@code false} means that the parent tenant
     *                is being destroyed.
     */
    @Override
    void destroy(VTNManagerImpl mgr, boolean retain) {
        VBridge vbridge;
        VBridgePath path = getPath();
        Lock wrlock = writeLock();
        try {
            vbridge = getVBridge(mgr);

            // Destroy all VLAN mappings.
            for (Iterator<Map.Entry<String, VlanMapImpl>> it =
                     vlanMaps.entrySet().iterator(); it.hasNext();) {
                Map.Entry<String, VlanMapImpl> entry = it.next();
                VlanMapImpl vmap = entry.getValue();
                try {
                    vmap.destroy(mgr, path, false);
                } catch (Exception e) {
                    LOG.error(getContainerName() + ":" + path +
                              ": Failed to destroy VLAN mapping: " +
                              entry.getKey(), e);
                    // FALLTHROUGH
                }
                it.remove();
            }

            // Destroy MAC mapping.
            MacMapImpl mmap = macMap;
            if (mmap != null) {
                macMap = null;
                mmap.destroy(mgr, path, null, false);
            }

            // Destroy MAC address table.
            mgr.removeMacAddressTable(path, retain);

            // Destroy all interfaces.
            destroyInterfaces(mgr);

            if (retain) {
                if (inFlowFilters.isEmpty() && outFlowFilters.isEmpty()) {
                    // Purge all VTN flows related to this bridge.
                    VTNThreadData.removeFlows(mgr, path);
                } else {
                    // REVISIT: Select flow entries affected by obsolete
                    // flow filters.
                    VTNThreadData.removeFlows(mgr, getTenantName());
                }
            }

            destroy();
        } finally {
            wrlock.unlock();
        }

        ConcurrentMap<VTenantPath, Object> db = mgr.getStateDB();
        db.remove(path);
        VBridgeEvent.removed(mgr, path, vbridge, retain);
    }

    /**
     * Return a logger instance.
     *
     * @return  A logger instance.
     */
    @Override
    protected Logger getLogger() {
        return LOG;
    }

    /**
     * Create path to this node.
     *
     * @param vtn   A virtual tenant that contains this node.
     * @param name  The name of this node.
     * @return  A path to this node.
     */
    @Override
    protected VBridgePath createPath(VTenantImpl vtn, String name) {
        return new VBridgePath(vtn.getName(), name);
    }

    /**
     * Create path to the virtual interface.
     *
     * @param name  The name of the virtual interface.
     * @return  Path to the specified virtual interface.
     */
    @Override
    protected VInterfacePath createIfPath(String name) {
        return new VBridgeIfPath(getPath(), name);
    }

    /**
     * Create a new virtual interface instance.
     *
     * @param name   The name of the virtual interface.
     * @param iconf  Interface configuration.
     * @return  An instance of virtual interface implementation.
     */
    @Override
    protected VBridgeIfImpl createInterface(String name,
                                            VInterfaceConfig iconf) {
        return new VBridgeIfImpl(this, name, iconf);
    }

    /**
     * Invoked when this node is going to be resumed from the configuration
     * file.
     *
     * @param mgr    VTN Manager service.
     * @param ctx    A runtime context for MD-SAL datastore transaction task.
     * @param state  Current state of this node.
     * @return  New state of this node.
     */
    @Override
    protected VnodeState resuming(VTNManagerImpl mgr, TxContext ctx,
                                  VnodeState state) {
        // Resume MAC mapping.
        VnodeState cur = state;
        MacMapImpl mmap = macMap;
        if (mmap != null) {
            cur = mmap.resume(mgr, ctx, cur);
        }

        // Resume VLAN mappings.
        for (VlanMapImpl vmap: vlanMaps.values()) {
            cur = vmap.resume(mgr, ctx, cur);
        }

        return cur;
    }

    /**
     * Invoked when this node is resumed from the configuration file.
     *
     * @param mgr    VTN Manager service.
     */
    @Override
    protected void resumed(VTNManagerImpl mgr) {
        // Create a MAC address table for this bridge.
        initMacTableAging(mgr);
    }

    /**
     * Invoked when the state of this node has been changed.
     *
     * <p>
     *   Note that this method must be called with holding the bridge write
     *   lock.
     * </p>
     *
     * @param mgr    VTN Manager service.
     * @param bst    Runtime state of this node.
     * @param state  New state of this node.
     */
    @Override
    protected void stateChanged(VTNManagerImpl mgr, BridgeState bst,
                                VnodeState state) {
        VBridgePath path = getPath();
        int faulted = bst.getFaultedPathSize();
        VBridge vbridge = new VBridge(path.getTenantNodeName(), state, faulted,
                                      bridgeConfig);
        VBridgeEvent.changed(mgr, path, vbridge, false);
    }

    /**
     * Scan virtual mappings configured in this node, and determine state
     * of this node.
     *
     * <p>
     *   Note that this method must be called with holding the bridge write
     *   lock.
     * </p>
     *
     * @param mgr    VTN Manager service.
     * @param db     Runtime state DB.
     * @return  New state of this bridge.
     */
    @Override
    protected VnodeState updateStateImpl(
        VTNManagerImpl mgr, ConcurrentMap<VTenantPath, Object> db) {
        VnodeState state = VnodeState.UNKNOWN;

        // Check to see if the MAC mapping is active.
        MacMapImpl mmap = macMap;
        if (mmap != null) {
            state = mmap.getBridgeState(mgr, state);
            if (state == VnodeState.DOWN) {
                return state;
            }
        }

        // Scan VLAN mappings.
        for (VlanMapImpl vmap: vlanMaps.values()) {
            state = vmap.getBridgeState(db, state);
            if (state == VnodeState.DOWN) {
                break;
            }
        }

        return state;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected List<Object> getContentsList(boolean copy) {
        List<Object> list = super.getContentsList(copy);
        if (copy) {
            list.add(new TreeMap<String, VlanMapImpl>(vlanMaps));
            MacMapImpl mmap = (macMap == null) ? null : macMap.clone();
            list.add(mmap);
            list.add(inFlowFilters.clone());
            list.add(outFlowFilters.clone());
        } else {
            list.add(vlanMaps);
            list.add(macMap);
            list.add(inFlowFilters);
            list.add(outFlowFilters);
        }

        return list;
    }

    // PortBridge

    /**
     * Evaluate flow filters configured in this vBridge against the given
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
    @Override
    PacketContext filterOutgoingPacket(VTNManagerImpl mgr, PacketContext pctx,
                                       short vid)
        throws DropFlowException, RedirectFlowException {
        return outFlowFilters.evaluate(mgr, pctx, vid);
    }

    /**
     * Invoked when a node is added, removed, or changed.
     *
     * @param mgr  VTN Manager service.
     * @param db   Virtual node state DB.
     * @param ev   A {@link VtnNodeEvent} instance.
     * @throws VTNException  An error occurred.
     */
    @Override
    void notifyNode(VTNManagerImpl mgr, ConcurrentMap<VTenantPath, Object> db,
                    VtnNodeEvent ev) throws VTNException {
        Lock wrlock = writeLock();
        try {
            BridgeState bst = getBridgeState(db);
            VnodeState state = notifyIfNode(mgr, db, bst, ev);

            MacMapImpl mmap = macMap;
            if (mmap != null) {
                VnodeState s = mmap.notifyNode(mgr, state, ev);
                if (LOG.isTraceEnabled()) {
                    LOG.trace("{}:{}: notifyNode(macmap): {} -> {}",
                              getContainerName(), getNodePath(), state, s);
                }
                state = s;
            }

            for (VlanMapImpl vmap: vlanMaps.values()) {
                VnodeState s = vmap.notifyNode(mgr, db, state, ev);
                if (LOG.isTraceEnabled()) {
                    LOG.trace("{}:{}: notifyNode(vmap:{}): {} -> {}",
                              getContainerName(), getNodePath(),
                              vmap.getMapId(), state, s);
                }
                state = s;
            }

            setState(mgr, db, bst, state);
        } finally {
            wrlock.unlock();
        }
    }

    /**
     * This method is called when some properties of a node connector are
     * added/deleted/changed.
     *
     * @param mgr  VTN Manager service.
     * @param db   Virtual node state DB.
     * @param ev   A {@link VtnPortEvent} instance.
     * @throws VTNException  An error occurred.
     */
    @Override
    void notifyNodeConnector(VTNManagerImpl mgr,
                             ConcurrentMap<VTenantPath, Object> db,
                             VtnPortEvent ev) throws VTNException {
        Lock wrlock = writeLock();
        try {
            BridgeState bst = getBridgeState(db);
            VnodeState state = notifyIfNodeConnector(mgr, db, bst, ev);

            MacMapImpl mmap = macMap;
            if (mmap != null) {
                VnodeState s = mmap.notifyNodeConnector(mgr, state, ev);
                if (LOG.isTraceEnabled()) {
                    LOG.trace("{}:{}: notifyNodeConnector(macmap): {} -> {}",
                              getContainerName(), getNodePath(), state, s);
                }
                state = s;
            }

            for (VlanMapImpl vmap: vlanMaps.values()) {
                VnodeState s = vmap.notifyNodeConnector(mgr, db, state, ev);
                if (LOG.isTraceEnabled()) {
                    LOG.trace("{}:{}: notifyNodeConnector(vmap:{}): {} -> {}",
                              getContainerName(), getNodePath(),
                              vmap.getMapId(), state, s);
                }
                state = s;
            }

            setState(mgr, db, bst, state);
        } finally {
            wrlock.unlock();
        }
    }

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
     * @throws DropFlowException
     *    The given packet was discarded by a flow filter.
     * @throws RedirectFlowException
     *    The given packet was redirected by a flow filter.
     * @throws VTNException
     *    An error occurred.
     */
    @Override
    protected void handlePacket(VTNManagerImpl mgr, PacketContext pctx,
                                VirtualMapNode vnode)
        throws DropFlowException, RedirectFlowException, VTNException {
        VBridgePath bpath = getPath();
        MacAddressTable table = mgr.getMacAddressTable(bpath);

        if (pctx.getFirstRedirection() == null) {
            // Learn the source MAC address if needed.
            table.add(pctx, (VBridgeNode)vnode);

            if (pctx.isToController()) {
                if (LOG.isTraceEnabled()) {
                    LOG.trace("{}:{}: Ignore packet sent to controller: {}",
                              getContainerName(), getNodePath(),
                              pctx.getDescription());
                }
                return;
            }
        }

        // Evaluate vBridge flow filters for incoming packets.
        inFlowFilters.evaluate(mgr, pctx, FlowFilterMap.VLAN_UNSPEC);

        // Determine whether the destination address is known or not.
        MacTableEntry tent = getDestination(mgr, pctx, table);
        if (tent != null) {
            // Forward the packet.
            forward(mgr, pctx, tent.getPort(), tent.getVlan());
        }

        return;
    }

    /**
     * Return the virtual network node in this vBridge which maps the specified
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
     * @return  A {@code VBridgeNode} is returned if the network specified
     *          by {@code nc} and {@code vlan} is mapped to this vBridge.
     *          Otherwise {@code null} is returned.
     */
    @Override
    protected VBridgeNode match(VTNManagerImpl mgr, long mac,
                                NodeConnector nc, short vlan) {
        // Check whether the packet is mapped by port mapping or not.
        VBridgeNode bnode = (VBridgeNode)super.match(mgr, mac, nc, vlan);
        if (bnode == null) {
            // Check whether the packet is mapped by MAC mapping or not.
            MacMapImpl mmap = macMap;
            if (mmap != null) {
                MacVlan mvlan = new MacVlan(mac, vlan);
                if (mmap.isActive(mgr, mvlan, nc)) {
                    return mmap;
                }
            }

            // Check whether the packet is mapped by VLAN mapping or not.
            String id = createVlanMapId(nc.getNode(), vlan);
            bnode = vlanMaps.get(id);
            if (bnode == null) {
                // Try VLAN mapping which maps all switches.
                id = createVlanMapId(null, vlan);
                bnode = vlanMaps.get(id);
            }
        }

        return bnode;
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
     * @return  A {@code VBridgeNode} is returned if the network specified
     *          by {@code nc} and {@code vlan} is mapped to this bridge.
     *          Otherwise {@code null} is returned.
     */
    @Override
    protected VBridgeNode match(VTNManagerImpl mgr, MapReference ref, long mac,
                                NodeConnector nc, short vlan,
                                boolean incoming) {
        MapType type = ref.getMapType();
        if (type == MapType.MAC) {
            MacMapImpl mmap = macMap;
            if (mmap == null) {
                // This may happen if the virtual network mapping is changed
                // by another cluster node.
                LOG.warn("{}:{}: Failed to resolve MAC mapping reference: {}",
                         getContainerName(), getNodePath(), ref);
                return null;
            }

            // Activate the specified host in the MAC mapping if the specified
            // packet is an incoming packet.
            MacVlan mvlan = new MacVlan(mac, vlan);
            if (incoming) {
                Boolean ret = mmap.activate(mgr, mvlan, nc);
                if (ret == null) {
                    return null;
                }

                if (ret.booleanValue()) {
                    // This MAC mapping has been activated.
                    // So the vBridge state must be updated.
                    updateState(mgr);
                }
            }

            return mmap;
        }

        if (type == MapType.VLAN) {
            // Determine the VLAN mapping instance specified by the mapping
            // reference.
            VlanMapPath vpath = (VlanMapPath)ref.getPath();
            String id = vpath.getMapId();
            VlanMapImpl vmap = vlanMaps.get(id);
            if (vmap == null) {
                // This may happen if the virtual network mapping is changed
                // by another cluster node.
                LOG.warn("{}:{}: Failed to resolve VLAN mapping reference: {}",
                         getContainerName(), getNodePath(), ref);
                // FALLTHROUGH
            }

            return vmap;
        }

        return (VBridgeNode)super.match(mgr, ref, mac, nc, vlan, incoming);
    }
}
