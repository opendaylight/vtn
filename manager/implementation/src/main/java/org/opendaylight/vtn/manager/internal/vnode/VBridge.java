/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.vnode;

import static org.opendaylight.vtn.manager.internal.util.flow.filter.FlowFilterList.VLAN_UNSPEC;
import static org.opendaylight.vtn.manager.internal.util.vnode.VBridgeIdentifier.getMacEntryPath;
import static org.opendaylight.vtn.manager.internal.util.vnode.VBridgeIdentifier.getMacTablePath;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import com.google.common.base.Optional;

import org.opendaylight.vtn.manager.VTNException;
import org.opendaylight.vtn.manager.util.EtherAddress;
import org.opendaylight.vtn.manager.util.Ip4Network;

import org.opendaylight.vtn.manager.internal.TxContext;
import org.opendaylight.vtn.manager.internal.inventory.VtnNodeEvent;
import org.opendaylight.vtn.manager.internal.inventory.VtnPortEvent;
import org.opendaylight.vtn.manager.internal.util.CacheMap;
import org.opendaylight.vtn.manager.internal.util.DataStoreUtils;
import org.opendaylight.vtn.manager.internal.util.MiscUtils;
import org.opendaylight.vtn.manager.internal.util.flow.VNodeHop;
import org.opendaylight.vtn.manager.internal.util.flow.filter.DropFlowException;
import org.opendaylight.vtn.manager.internal.util.flow.filter.FlowFilterList;
import org.opendaylight.vtn.manager.internal.util.flow.filter.RedirectFlowException;
import org.opendaylight.vtn.manager.internal.util.inventory.InventoryReader;
import org.opendaylight.vtn.manager.internal.util.inventory.PortVlan;
import org.opendaylight.vtn.manager.internal.util.inventory.SalPort;
import org.opendaylight.vtn.manager.internal.util.log.VTNLogLevel;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;
import org.opendaylight.vtn.manager.internal.util.vnode.BridgeIdentifier;
import org.opendaylight.vtn.manager.internal.util.vnode.TenantNodeIdentifier;
import org.opendaylight.vtn.manager.internal.util.vnode.VInterfaceIdentifier;
import org.opendaylight.vtn.manager.internal.util.vnode.VNodeType;
import org.opendaylight.vtn.manager.internal.util.vnode.VTNMacMapConfig;
import org.opendaylight.vtn.manager.internal.util.vnode.VTNPortMapConfig;
import org.opendaylight.vtn.manager.internal.util.vnode.VTNVlanMapConfig;
import org.opendaylight.vtn.manager.internal.util.vnode.VlanMapIdentifier;
import org.opendaylight.vtn.manager.internal.util.vnode.mac.CurrentMacEntry;
import org.opendaylight.vtn.manager.internal.util.vnode.mac.MacEntry;
import org.opendaylight.vtn.manager.internal.util.vnode.mac.NewMacEntry;
import org.opendaylight.vtn.manager.internal.vnode.xml.XmlLogger;
import org.opendaylight.vtn.manager.internal.vnode.xml.XmlVBridge;

import org.opendaylight.controller.md.sal.binding.api.ReadTransaction;
import org.opendaylight.controller.md.sal.binding.api.ReadWriteTransaction;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.VirtualRouteReason;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.mac.rev150907.vtn.mac.mappable.MacMap;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.vlan.rev150907.vtn.vlan.mappable.VlanMap;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeState;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.mac.rev150907.vtn.mac.table.entry.MacTableEntry;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.mac.rev150907.vtn.mac.table.list.MacAddressTable;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.VtnVbridgeConfig;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.vbridge.list.Vbridge;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.vbridge.list.VbridgeBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.VtnVinterfaceConfig;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.vtn.mappable.vinterface.list.Vinterface;

import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.yang.types.rev100924.MacAddress;

/**
 * {@code VBridge} describes a configuration and runtime status for a vBridge.
 */
public final class VBridge extends VirtualBridge<Vbridge> {
    /**
     * A logger instance.
     */
    private static final Logger  LOG = LoggerFactory.getLogger(VBridge.class);

    /**
     * The default value for age-interval.
     */
    public static final int  DEFAULT_AGE_INTERVAL = 600;

    /**
     * The number of seconds between MAC address table aging.
     */
    private int  ageInterval;

    /**
     * VLAN mappings configured in this vBridge.
     */
    private VlanMapCache  vlanMaps;

    /**
     * MAC mapping configuration.
     */
    private VTNMacMap  macMap;

    /**
     * True if the MAC mapping is cached.
     */
    private boolean  macMapCached;

    /**
     * A list of vBridge input filters.
     */
    private FlowFilterList  inputFilter;

    /**
     * A list of vBridge output filters.
     */
    private FlowFilterList  outputFilter;

    /**
     * A map that keeps VLAN mappings.
     */
    private final class VlanMapCache
        extends CacheMap<VlanMap, String, VTNVlanMap> {
        /**
         * Construct a new instance.
         *
         * @param list  A list of {@link VlanMap} instances read from the
         *              MD-SAL datastore.
         */
        private VlanMapCache(List<VlanMap> list) {
            super(list);
        }

        // CacheMap

        /**
         * Return a key for the given VLAN mapping instance.
         *
         * @param vmp  A {@link VlanMap} instance.
         * @return  The identifier for the given VLAN mapping.
         */
        @Override
        protected String getKey(VlanMap vmp) {
            return vmp.getMapId();
        }

        /**
         * Create a new VLAN mapping instance.
         *
         * @param data  A {@link VlanMap} instance read from the MD-SAL
         *              datastore.
         * @return  A VLAN mapping instance.
         */
        @Override
        protected VTNVlanMap newCache(VlanMap data) {
            return new VTNVlanMap(getIdentifier(), data);
        }
    }

    /**
     * Construct a new instance.
     *
     * <p>
     *   This constructor is used to resume runtime status of the virtual
     *   bridge on bootstrap.
     * </p>
     *
     * @param vbrId  The identifier for the vBridge.
     */
    public VBridge(BridgeIdentifier<Vbridge> vbrId) {
        // vlanMaps field is never used in resume().
        super(vbrId);
    }

    /**
     * Construct a new instance.
     *
     * @param vbrId  The identifier for the vBridge.
     * @param vbr    A {@link Vbridge} instance read from the MD-SAL datastore.
     */
    public VBridge(BridgeIdentifier<Vbridge> vbrId, Vbridge vbr) {
        super(vbrId, vbr);
        initConfig(vbr.getVbridgeConfig());
        vlanMaps = new VlanMapCache(vbr.getVlanMap());
    }

    /**
     * Return the vbridge container associated with this instance.
     *
     * @return  A {@link Vbridge} instance.
     */
    public Vbridge getVbridge() {
        return new VbridgeBuilder(getInitialValue()).
            setBridgeStatus(getBridgeStatus()).
            build();
    }

    /**
     * Resume this vBridge.
     *
     * @param ctx      MD-SAL datastore transaction context.
     * @param xlogger  A {@link XmlLogger} instance.
     * @param xvbr     A {@link XmlVBridge} instance that contains vBridge
     *                 configuration.
     * @return  A {@link Vbridge} instance that contains vBridge configuration
     *          and runtime status.
     * @throws VTNException  An error occurred.
     */
    public Vbridge resume(TxContext ctx, XmlLogger xlogger, XmlVBridge xvbr)
        throws VTNException {
        // Initialize vBridge configuration.
        BridgeIdentifier<Vbridge> vbrId = getIdentifier();
        VbridgeBuilder builder = xvbr.toVbridgeBuilder(xlogger, vbrId);
        initConfig(builder.getVbridgeConfig());

        // Resume MAC mapping.
        VTNMacMapConfig cfg = xvbr.getMacMap();
        if (cfg != null) {
            VTNMacMap mmap = new VTNMacMap(vbrId, cfg);
            MacMap mmc = mmap.resume(ctx);
            if (mmc != null) {
                macMap = mmap;
                macMapCached = true;
                builder.setMacMap(mmc);

                // Set DOWN to vBridge state because no host is mapped by
                // MAC mapping on bootstrap.
                updateState(mmap, VnodeState.DOWN);
            }
        }

        // Resume VLAN mappings.
        List<VTNVlanMapConfig> vmcs = xvbr.getVlanMaps();
        if (!MiscUtils.isEmpty(vmcs)) {
            Set<String> map = new HashSet<>();
            List<VlanMap> vmapList = new ArrayList<>(vmcs.size());
            for (VTNVlanMapConfig vmc: vmcs) {
                // Duplicate mappings will be ignored.
                VTNVlanMap vmap = new VTNVlanMap(vbrId, vmc);
                if (map.add(vmap.getMapId())) {
                    VnodeState st = vmap.resume(ctx, vmapList);
                    if (st != null) {
                        updateState(vmap, st);
                    }
                }
            }
            builder.setVlanMap(vmapList);
        }

        // Resume virtual interfaces.
        builder.setVinterface(resume(ctx, xlogger, xvbr.getInterfaces())).
            setBridgeStatus(getBridgeStatus());

        xlogger.log(VTNLogLevel.DEBUG, "{}: vBridge has been loaded: state={}",
                    vbrId, getState());

        return builder.build();
    }

    /**
     * Initialize configuration of the vBridge.
     *
     * @param vbrc   Configuration of the vBridge.
     */
    private void initConfig(VtnVbridgeConfig vbrc) {
        Integer age = vbrc.getAgeInterval();
        ageInterval = (age == null) ? DEFAULT_AGE_INTERVAL : age.intValue();
        setDescription(vbrc.getDescription());
    }

    /**
     * Return the MAC mapping configured in this vBridge.
     *
     * @return  A {@link VTNMacMap} instance or {@code null}.
     */
    private VTNMacMap getMacMap() {
        VTNMacMap mmap = macMap;
        if (!macMapCached) {
            MacMap mmp = getInitialValue().getMacMap();
            if (mmp != null) {
                try {
                    mmap = new VTNMacMap(getIdentifier(), mmp);
                    macMap = mmap;
                } catch (RpcException e) {
                    throw new IllegalStateException(
                        "Broken MAC mapping configuration.", e);
                }
            }
            macMapCached = true;
        }

        return mmap;
    }

    /**
     * Return the vBridge input filter list.
     *
     * @param ctx  MD-SAL datastore transaction context.
     * @return  A {@link FlowFilterList} instance associated with the vBridge
     *          input filter list.
     */
    private FlowFilterList getInputFilter(TxContext ctx) {
        FlowFilterList ffl = inputFilter;
        if (ffl == null) {
            ffl = getFlowFilterList(
                ctx, false, getInitialValue().getVbridgeInputFilter());
            inputFilter = ffl;
        }

        return ffl;
    }

    /**
     * Return the vBridge output filter list.
     *
     * @param ctx  MD-SAL datastore transaction context.
     * @return  A {@link FlowFilterList} instance associated with the vBridge
     *          output filter list.
     */
    private FlowFilterList getOutputFilter(TxContext ctx) {
        FlowFilterList ffl = outputFilter;
        if (ffl == null) {
            ffl = getFlowFilterList(
                ctx, true, getInitialValue().getVbridgeOutputFilter());
            outputFilter = ffl;
        }

        return ffl;
    }

    /**
     * Determine whether the input flow filter list is empty or not.
     *
     * @return  {@code true} if this vBridge does not contain input flow
     *          filter. {@code false} otherwise.
     */
    private boolean isInputFilterEmpty() {
        FlowFilterList ffl = inputFilter;
        return (ffl == null)
            ? FlowFilterList.isEmpty(getInitialValue().getVbridgeInputFilter())
            : ffl.isEmpty();
    }

    /**
     * Determine whether the output flow filter list is empty or not.
     *
     * @return  {@code true} if this vBridge does not contain output flow
     *          filter. {@code false} otherwise.
     */
    private boolean isOutputFilterEmpty() {
        FlowFilterList ffl = outputFilter;
        return (ffl == null)
            ? FlowFilterList.isEmpty(
                getInitialValue().getVbridgeOutputFilter())
            : ffl.isEmpty();
    }

    /**
     * Return the VLAN mapping specified by the given identifier.
     *
     * @param pctx   A runtime context for a received packet.
     * @param mapId  The identifier for the VLAN mapping in this vBridge.
     * @return  A {@link VTNVlanMap} instance if found.
     *          {@code null} if not found.
     * @throws VTNException  An error occurred.
     */
    private VTNVlanMap getVlanMap(PacketContext pctx, VlanMapIdentifier mapId)
        throws VTNException {
        String id = mapId.getMapId();
        VTNVlanMap vmap = vlanMaps.get(id);
        if (vmap == null) {
            // Read the specified VLAN mapping in the MD-SAL datastore.
            ReadTransaction rtx = pctx.getTxContext().getTransaction();
            Optional<VlanMap> opt = mapId.read(rtx);
            if (opt.isPresent()) {
                vmap = new VTNVlanMap(mapId, opt.get());
                vlanMaps.put(id, vmap);
            }
        }

        return vmap;
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
     * @param pctx  A runtime context for a received packet.
     * @return  A {@link MacEntry} instance if found.
     *          {@code null} if not found or if the received packet should not
     *          be sent to this bridge.
     * @throws DropFlowException
     *    The given packet was discarded by a flow filter.
     * @throws RedirectFlowException
     *    The given packet was redirected by a flow filter.
     * @throws VTNException
     *    An error occurred.
     */
    private MacEntry getDestination(PacketContext pctx)
        throws DropFlowException, RedirectFlowException, VTNException {
        // Check to see if the destination address is learned.
        TxContext ctx = pctx.getTxContext();
        EtherAddress dst = pctx.getDestinationAddress();
        MacEntry ment = getMacEntry(ctx, dst);
        VirtualMapNode vnode = getMapping(pctx, dst, ment);
        if (vnode == null) {
            // Broadcast the destination MAC address if not learned in the
            // MAC address table.
            broadcast(pctx);
            return null;
        }

        SalPort egress = ment.getPort();
        if (vnode.isEnabled()) {
            InventoryReader reader = pctx.getInventoryReader();
            if (reader.isEnabled(egress)) {
                int vid = ment.getVlanId();
                TenantNodeIdentifier<?, ?> mapPath =
                    vnode.getIdentifier(dst, vid);
                pctx.setEgressVNodeHop(
                    new VNodeHop(mapPath, VirtualRouteReason.FORWARDED));

                // Evaluate output flow filters.
                // Note that this should never clone a PacketContext.
                vnode.filterPacket(pctx, vid, this);
            } else {
                LOG.warn("{}: Drop packet because egress port is down: {}",
                         getIdentifier(), pctx.getDescription(egress));
                ment = null;
            }
        } else if (LOG.isDebugEnabled()) {
            LOG.debug("{}: Drop packet because egress network is disabled: {}",
                      vnode.getIdentifier(), pctx.getDescription(egress));
            ment = null;
        }

        return ment;
    }

    /**
     * Search for a MAC address table entry specified by the given MAC address
     * in the MAC address table.
     *
     * @param ctx    MD-SAL datastore transaction context.
     * @param eaddr  An {@link EtherAddress} instance that specifies the
     *               MAC address.
     * @return  A {@link MacEntry} instance if found.
     *          {@code null} if not found.
     * @throws VTNException  An error occurred.
     */
    private MacEntry getMacEntry(TxContext ctx, EtherAddress eaddr)
        throws VTNException {
        // Multicast address should never be found in the MAC address table.
        MacEntry ment = null;
        if (eaddr.isUnicast()) {
            BridgeIdentifier<Vbridge> vbrId = getIdentifier();
            ReadWriteTransaction tx = ctx.getReadWriteTransaction();
            InstanceIdentifier<MacTableEntry> path =
                getMacEntryPath(vbrId, eaddr.getMacAddress());
            Optional<MacTableEntry> opt = DataStoreUtils.read(tx, path);
            if (opt.isPresent()) {
                ment = new CurrentMacEntry(eaddr, opt.get());
            }
        }

        return ment;
    }

    /**
     * Return the virtual mapping that maps the given MAC address table entry.
     *
     * @param pctx   A runtime context for a received packet.
     * @param eaddr  An {@link EtherAddress} instance that specifies the
     *               MAC address.
     * @param ment   A MAC address table entry.
     * @return  A {@link VirtualMapNode} associated with the virtual mapping
     *          if found. {@code null} if not found.
     * @throws VTNException  An error occurred.
     */
    private VirtualMapNode getMapping(PacketContext pctx, EtherAddress eaddr,
                                      MacEntry ment) throws VTNException {
        VirtualMapNode vnode;
        if (ment == null) {
            vnode = null;
        } else {
            SalPort sport = ment.getPort();
            int vid = ment.getVlanId();
            vnode = match(pctx, eaddr, sport, vid);
            if (vnode == null) {
                BridgeIdentifier<Vbridge> vbrId = getIdentifier();
                LOG.warn("{}: Unexpected MAC address entry: {}", vbrId, ment);
                pctx.removeFlows(ment);
                LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
                ReadWriteTransaction tx = pctx.getTxContext().
                    getReadWriteTransaction();
                tx.delete(oper, getMacEntryPath(vbrId, eaddr.getMacAddress()));
            }
        }

        return vnode;
    }

    /**
     * Broadcast the given packet to all the virtual mappings configured in
     * this vBridge.
     *
     * @param pctx  A runtime context for a received packet.
     * @throws VTNException  An error occurred.
     */
    private void broadcast(PacketContext pctx) throws VTNException {
        // From here, REDIRECT flow filters are ignored.
        pctx.setFlooding(true);

        RedirectFlowException rex = pctx.getFirstRedirection();
        BridgeIdentifier<Vbridge> vbrId = getIdentifier();
        if (rex != null) {
            pctx.broadcasted(rex, vbrId);
        }

        try {
            // Don't send the packet to the incoming network.
            Set<PortVlan> sent = new HashSet<>();
            PortVlan innw = pctx.getIncomingNetwork();
            sent.add(innw);

            // Forward packet to the network established by the port mapping.
            broadcastInterfaces(pctx, sent);

            // Forward packet to the network established by the MAC mapping.
            VTNMacMap mmap = getMacMap();
            if (mmap != null) {
                mmap.transmit(pctx, this, sent);
            }

            // Forward packet to the network established by the VLAN mapping.
            for (VTNVlanMap vmap: vlanMaps) {
                vmap.transmit(pctx, this, sent);
            }

            if (LOG.isDebugEnabled() && sent.size() == 1 &&
                sent.contains(innw)) {
                LOG.debug("{}: No packet was broadcasted: {}",
                          getIdentifier(), pctx.getDescription());
            }
        } catch (RedirectFlowException e) {
            // This should never happen.
            pctx.getTxContext().
                log(LOG, VTNLogLevel.ERROR, e,
                    "{}: Unexpected redirection.", getIdentifier());
        }
    }

    /**
     * Learn the source MAC address of the given packet.
     *
     * @param pctx   A runtime context for a received packet.
     * @param vnode  A {@link VirtualMapNode} instance that specifies the
     *               virtual mapping that maps the packet.
     * @throws VTNException  An error occurred.
     */
    private void learnMacAddress(PacketContext pctx, VirtualMapNode vnode)
        throws VTNException {
        EtherAddress src = pctx.getSourceAddress();
        if (isMacAddressValid(pctx, src)) {
            MacAddress mac = src.getMacAddress();
            InstanceIdentifier<MacTableEntry> path =
                getMacEntryPath(getIdentifier(), mac);
            MacEntry ment = addMacEntry(pctx, path, src, vnode);
            if (pctx.isIPv4() && ment.needIpProbe()) {
                // Try to detect IP address of the given host.
                pctx.probeInetAddress();
            }

            MacTableEntry newEnt = ment.getNewEntry();
            if (newEnt != null) {
                // Put the new entry.
                TxContext ctx = pctx.getTxContext();
                ReadWriteTransaction tx = ctx.getReadWriteTransaction();
                LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
                tx.put(oper, path, newEnt, false);
            }
        }
    }

    /**
     * Determine whether the given MAC address should be learned in the
     * MAC address table or not.
     *
     * @param pctx   A runtime context for a received packet.
     * @param eaddr  An {@link EtherAddress} to be tested.
     * @return  {@code true} only if the given MAC address should be learned
     *          in the MAC address.
     */
    private boolean isMacAddressValid(PacketContext pctx, EtherAddress eaddr) {
        // Multicast address should be ignored.
        boolean valid = eaddr.isUnicast();
        if (valid) {
            valid = (eaddr.getAddress() != 0L);
            if (!valid) {
                pctx.getTxContext().
                    log(LOG, VTNLogLevel.WARN,
                        "{}: Ignore zero MAC address: {}",
                        getIdentifier(), pctx.getFrame());
            }
        }

        return valid;
    }

    /**
     * Learn the given host into theMAC address table.
     *
     * @param pctx   A runtime context for a received packet.
     * @param path   Path to the MAC address table entry in the MD-SAL
     *               datastore.
     * @param src    An {@link EtherAddress} instance that indicates the
     *               source MAC address.
     * @param vnode  A {@link VirtualMapNode} instance that specifies the
     *               virtual mapping that maps the packet.
     * @return  A MAC address table entry associated with the given packet.
     * @throws VTNException  An error occurred.
     */
    private MacEntry addMacEntry(
        PacketContext pctx, InstanceIdentifier<MacTableEntry> path,
        EtherAddress src, VirtualMapNode vnode) throws VTNException {
        Ip4Network ip = pctx.getSenderIp4Address();
        SalPort sport = pctx.getIngressPort();
        int vid = pctx.getVlanId();

        // Search for a MAC address in the MAC address table.
        TxContext ctx = pctx.getTxContext();
        TenantNodeIdentifier<?, ?> mapPath = vnode.getIdentifier();
        ReadWriteTransaction tx = ctx.getReadWriteTransaction();
        Optional<MacTableEntry> opt = DataStoreUtils.read(tx, path);
        MacEntry ment;
        if (opt.isPresent()) {
            // Check to see if the entry needs to be updated.
            ment = new CurrentMacEntry(src, opt.get());
            if (ment.hasMoved(sport, vid, mapPath)) {
                LOG.trace("{}: MAC address has moved: old={}",
                          getIdentifier(), ment);

                // Remove obsolete flow entries.
                pctx.removeFlows(ment);

                ment = new NewMacEntry(src, sport, pctx.getIngressPortName(),
                                       vid, ip, mapPath);

                // Remove obsolete flow entries associated with a new entry.
                pctx.removeFlows(ment);
            } else {
                if (ip != null) {
                    // Associate the IP address with the MAC address.
                    ment.addIpAddress(ip);
                }

                // Turn the dirty flag on so that the used flag is set.
                ment.setDirty();
            }
        } else {
            // Put a new MAC address into the MAC address table.
            ment = new NewMacEntry(src, sport, pctx.getIngressPortName(), vid,
                                   ip, mapPath);
        }

        return ment;
    }

    /**
     * Check to see if the given host is mapped by the MAC mapping.
     *
     * @param pctx   A runtime context for a received packet.
     * @param ref    The identifier for the MAC mapping expected to map the
     *               given host.
     * @param eaddr  The MAC address of the host.
     * @param sport  A {@link SalPort} instance that specifies the switch port
     *               where the host was detected.
     * @param vid    The VLAN ID associated with the specified host.
     * @param pktin  {@code true} indicates the caller is the PACKET_IN
     *               handler.
     * @return  A {@code VirtualMapNode} if the specified host is mapped to
     *          this virtual bridge. {@code null} otherwise.
     * @throws VTNException  An error occurred.
     */
    private VTNMacMap matchMacMap(
        PacketContext pctx, TenantNodeIdentifier<?, ?> ref, EtherAddress eaddr,
        SalPort sport, int vid, boolean pktin) throws VTNException {
        TxContext ctx = pctx.getTxContext();
        VTNMacMap mmap = getMacMap();
        if (mmap == null) {
            ctx.log(LOG, VTNLogLevel.WARN,
                    "{}: Failed to resolve MAC mapping reference: {}",
                    getIdentifier(), ref);
        } else if (pktin) {
            // Activate the MAC mapping that maps the given packet.
            if (!mmap.activate(ctx, this, eaddr, sport, vid)) {
                // Failed.
                mmap = null;
            }
        } else if (!mmap.isActive(ctx, eaddr, sport, vid)) {
            // The given host is not mapped by the MAC mapping.
            mmap = null;
        }

        return mmap;
    }

    // VirtualBridge

    /**
     * Determine whether this vBridge contains no flow filter or not.
     *
     * @return  {@code true} if this virtual bridge contains no flow filter.
     *          {@code false} otherwise.
     */
    @Override
    public boolean isFilterEmpty() {
        return (isInputFilterEmpty() && isOutputFilterEmpty() &&
                super.isFilterEmpty());
    }

    /**
     * Return the virtual network node in this vBridge which maps the specified
     * host.
     *
     * @param pctx   A runtime context for a received packet.
     * @param ref    Reference to the virtual network mapping that maps the
     *               host. Note that the specified virtual mapping must be
     *               present in this virtual bridge.
     * @param eaddr  The MAC address of the host.
     * @param sport  A {@link SalPort} instance that specifies the switch port
     *               where the host was detected.
     * @param vid    The VLAN ID associated with the specified host.
     * @param pktin  {@code true} indicates the caller is the PACKET_IN
     *               handler.
     * @return  A {@code VirtualMapNode} if the specified host is mapped to
     *          this virtual bridge. {@code null} otherwise.
     * @throws VTNException  An error occurred.
     */
    @Override
    protected VirtualMapNode match(
        PacketContext pctx, TenantNodeIdentifier<?, ?> ref, EtherAddress eaddr,
        SalPort sport, int vid, boolean pktin) throws VTNException {
        assert getIdentifier().contains(ref);

        VNodeType type = ref.getType();
        VirtualMapNode vnode;
        if (type.isMacMap()) {
            // Check to see if the given host is mapped by MAC mapping.
            vnode = matchMacMap(pctx, ref, eaddr, sport, vid, pktin);
        } else if (type == VNodeType.VLANMAP) {
            @SuppressWarnings("unchecked")
            VlanMapIdentifier mapId = (VlanMapIdentifier)ref;
            vnode = getVlanMap(pctx, mapId);
            if (vnode == null) {
                pctx.getTxContext().
                    log(LOG, VTNLogLevel.WARN,
                        "{}: Failed to resolve VLAN mapping reference: {}",
                        getIdentifier(), ref);
            }
        } else {
            vnode = super.match(pctx, ref, eaddr, sport, vid, pktin);
        }

        return vnode;
    }

    /**
     * Construct a new virtual interface instance associated with the interface
     * inside this vBridge.
     *
     * @param ifId  The identifier for a new virtual interface.
     * @param vif   A {@link Vinterface} instance read from the MD-SAL
     *              datastore.
     * @return  A {@link VBridgeInterface} instance.
     */
    @Override
    public VBridgeInterface newInterface(VInterfaceIdentifier<Vbridge> ifId,
                                         Vinterface vif) {
        return new VBridgeInterface(ifId, vif);
    }

    /**
     * Construct a new virtual interface instance associated with the interface
     * inside this vBridge.
     *
     * <p>
     *   This method is used to resume runtime status of the virtual interface
     *   on bootstrap.
     * </p>
     *
     * @param ifId  The identifier for a new virtual interface.
     * @param vifc  Configuration of the virtual interface.
     * @param pmap  Port mapping configuration.
     * @return  A {@link VBridgeInterface} instance.
     */
    @Override
    public VBridgeInterface newInterface(
        VInterfaceIdentifier<Vbridge> ifId, VtnVinterfaceConfig vifc,
        VTNPortMapConfig pmap) {
        return new VBridgeInterface(ifId, vifc, pmap);
    }

    /**
     * Destroy the vBridge.
     *
     * @param ctx     MD-SAL datastore transaction context.
     * @param retain  {@code true} means that the parent virtual node will be
     *                retained. {@code false} means that the parent virtual
     *                node is being destroyed.
     * @throws VTNException  An error occurred.
     */
    @Override
    public void destroy(TxContext ctx, boolean retain) throws VTNException {
        // Destroy all the virtual interfaces.
        destroyInterfaces(ctx);

        // Destroy all the VLAN mappings configured in this vBridge.
        for (VTNVlanMap vmap: vlanMaps) {
            vmap.destroy(ctx, false);
        }

        VTNMacMap mmap = getMacMap();
        if (mmap != null) {
            // Destroy the MAC mapping.
            mmap.destroy(ctx, false);
        }

        BridgeIdentifier<Vbridge> vbrId = getIdentifier();
        if (retain) {
            // Remove the MAC address table associated with this vBridge.
            ReadWriteTransaction tx = ctx.getReadWriteTransaction();
            InstanceIdentifier<MacAddressTable> mpath = getMacTablePath(vbrId);
            DataStoreUtils.delete(tx, mpath);
        }
    }

    /**
     * Return the state of the virtual mapping configured to this bridge.
     *
     * @param ctx  A runtime context for transaction task.
     * @return  The state of the virtual mapping configured to this bridge.
     */
    @Override
    protected VnodeState getMapState(TxContext ctx) {
        VnodeState st = VnodeState.UNKNOWN;

        // Check to see if the MAC mapping is active.
        VTNMacMap mmap = getMacMap();
        if (mmap != null) {
            st = VirtualBridge.updateState(st, mmap.getState(ctx));
        }

        if (st != VnodeState.DOWN) {
            // Scan VLAN mappings.
            for (VTNVlanMap vmap: vlanMaps) {
                st = VirtualBridge.updateState(st, vmap.getState());
                if (st == VnodeState.DOWN) {
                    break;
                }
            }
        }

        return st;
    }

    /**
     * Notify virtual mappings in this bridge of node event.
     *
     * @param ctx  MD-SAL datastore transaction context.
     * @param ev   A {@link VtnNodeEvent} instance.
     * @return  The state of the virtual mapping configured to this bridge.
     */
    @Override
    protected VnodeState eventReceived(TxContext ctx, VtnNodeEvent ev) {
        VnodeState st = VnodeState.UNKNOWN;

        // Deliver the event to the MAC mapping.
        VTNMacMap mmap = getMacMap();
        if (mmap != null) {
            VnodeState cst = mmap.notifyNode(ctx, ev);
            st = VirtualBridge.updateState(st, cst);
            traceState(mmap, cst, st);
        }

        // Deliver the event to all the VLAN mappings.
        for (VTNVlanMap vmap: vlanMaps) {
            VnodeState cst = vmap.notifyNode(ctx, ev);
            st = VirtualBridge.updateState(st, cst);
            traceState(vmap, cst, st);
        }

        return st;
    }

    /**
     * Notify virtual mappings in this bridge of port event.
     *
     * @param ctx  MD-SAL datastore transaction context.
     * @param ev   A {@link VtnPortEvent} instance.
     * @return  The state of the virtual mapping configured to this bridge.
     */
    @Override
    protected VnodeState eventReceived(TxContext ctx, VtnPortEvent ev) {
        VnodeState st = VnodeState.UNKNOWN;

        // Deliver the event to the MAC mapping.
        VTNMacMap mmap = getMacMap();
        if (mmap != null) {
            VnodeState cst = mmap.notifyPort(ctx, ev);
            st = VirtualBridge.updateState(st, cst);
            traceState(mmap, cst, st);
        }

        // Deliver the event to all the VLAN mappings.
        for (VTNVlanMap vmap: vlanMaps) {
            VnodeState cst = vmap.notifyPort(ctx, ev);
            st = VirtualBridge.updateState(st, cst);
            traceState(vmap, cst, st);
        }

        return st;
    }

    /**
     * Evaluate flow filters configured in this vBridge against the given
     * outgoing packet.
     *
     * @param pctx  A runtime context for a received packet.
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
    protected PacketContext evaluateOutputFilter(PacketContext pctx, int vid)
        throws DropFlowException, RedirectFlowException {
        return pctx.evaluate(getOutputFilter(pctx.getTxContext()), vid);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void forward(PacketContext pctx, VirtualMapNode vnode)
        throws DropFlowException, RedirectFlowException, VTNException {
        TxContext ctx = pctx.getTxContext();
        if (pctx.getFirstRedirection() == null) {
            // Learn the source MAC address if needed.
            learnMacAddress(pctx, vnode);

            if (pctx.isToController()) {
                if (LOG.isTraceEnabled()) {
                    LOG.trace("{}: Ignore packet sent to controller: {}",
                              getIdentifier(), pctx.getDescription());
                }
                return;
            }
        }

        // Evaluate vBridge flow filters for incoming packets.
        pctx.evaluate(getInputFilter(ctx), VLAN_UNSPEC);

        // Determine whether the destination address is known or not.
        MacEntry ment = getDestination(pctx);
        if (ment != null) {
            // Forward the packet.
            forward(pctx, ment.getPort(), ment.getVlanId());
        }
    }

    // VirtualElement

    /**
     * {@inheritDoc}
     */
    @Override
    protected Logger getLogger() {
        return LOG;
    }
}
