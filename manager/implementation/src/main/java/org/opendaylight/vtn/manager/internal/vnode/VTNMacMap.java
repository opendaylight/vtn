/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.vnode;

import static org.opendaylight.vtn.manager.internal.util.MiscUtils.toVnodeState;
import static org.opendaylight.vtn.manager.internal.util.vnode.MacMapChange.DONT_PURGE;
import static org.opendaylight.vtn.manager.internal.util.vnode.MacMapChange.REMOVING;
import static org.opendaylight.vtn.manager.internal.vnode.MappingRegistry.inactivateMacMap;
import static org.opendaylight.vtn.manager.internal.vnode.MappingRegistry.registerMacMap;

import java.util.Set;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import org.opendaylight.vtn.manager.VTNException;
import org.opendaylight.vtn.manager.packet.Ethernet;
import org.opendaylight.vtn.manager.util.EtherAddress;

import org.opendaylight.vtn.manager.internal.TxContext;
import org.opendaylight.vtn.manager.internal.inventory.VtnNodeEvent;
import org.opendaylight.vtn.manager.internal.inventory.VtnPortEvent;
import org.opendaylight.vtn.manager.internal.util.MiscUtils;
import org.opendaylight.vtn.manager.internal.util.flow.VNodeHop;
import org.opendaylight.vtn.manager.internal.util.flow.filter.DropFlowException;
import org.opendaylight.vtn.manager.internal.util.flow.filter.RedirectFlowException;
import org.opendaylight.vtn.manager.internal.util.inventory.InventoryUtils;
import org.opendaylight.vtn.manager.internal.util.inventory.MacVlan;
import org.opendaylight.vtn.manager.internal.util.inventory.NodePortFilter;
import org.opendaylight.vtn.manager.internal.util.inventory.PortFilter;
import org.opendaylight.vtn.manager.internal.util.inventory.PortVlan;
import org.opendaylight.vtn.manager.internal.util.inventory.SalPort;
import org.opendaylight.vtn.manager.internal.util.inventory.SpecificPortFilter;
import org.opendaylight.vtn.manager.internal.util.log.VTNLogLevel;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;
import org.opendaylight.vtn.manager.internal.util.vnode.BridgeIdentifier;
import org.opendaylight.vtn.manager.internal.util.vnode.MacMapChange;
import org.opendaylight.vtn.manager.internal.util.vnode.MacMapException;
import org.opendaylight.vtn.manager.internal.util.vnode.MacMapHostIdentifier;
import org.opendaylight.vtn.manager.internal.util.vnode.MacMapIdentifier;
import org.opendaylight.vtn.manager.internal.util.vnode.VBridgeMapIdentifier;
import org.opendaylight.vtn.manager.internal.util.vnode.VTNMacMapConfig;
import org.opendaylight.vtn.manager.internal.util.vnode.VTNMacMapStatus;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.VirtualRouteReason;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.mac.rev150907.vtn.mac.map.info.MacMapStatus;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.mac.rev150907.vtn.mac.map.info.MacMapStatusBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.mac.rev150907.vtn.mac.mappable.MacMap;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.mac.rev150907.vtn.mac.mappable.MacMapBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeState;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.vbridge.list.Vbridge;

/**
 * {@code VTNMacMap} describes a configuration and runtime status for a
 * MAC mapping.
 */
public final class VTNMacMap extends VirtualElement<MacMap, MacMapIdentifier>
    implements VirtualMapNode {
    /**
     * A logger instance.
     */
    private static final Logger  LOG =
        LoggerFactory.getLogger(VTNMacMap.class);

    /**
     * MAC mapping configuration.
     */
    private VTNMacMapConfig  config;

    /**
     * Construct a new instance.
     *
     * <p>
     *   This constructor is used to resume runtime status of the MAC mapping
     *   on bootstrap.
     * </p>
     *
     * @param vbrId  The identifier for a vBridge that contains this MAC
     *               mapping.
     * @param mmc    MAC mapping configuration.
     */
    public VTNMacMap(BridgeIdentifier<Vbridge> vbrId, VTNMacMapConfig mmc) {
        super(new MacMapIdentifier(vbrId));
        config = mmc;
    }

    /**
     * Construct a new instance.
     *
     * @param vbrId  The identifier for a vBridge that contains this MAC
     *               mapping.
     * @param mmp    A {@link MacMap} instance read from the MD-SAL datastore.
     * @throws RpcException
     *    {@code mmp} contains invalid configuration.
     */
    public VTNMacMap(BridgeIdentifier<Vbridge> vbrId, MacMap mmp)
        throws RpcException {
        super(new MacMapIdentifier(vbrId), mmp);
    }

    /**
     * Return the state of this MAC mapping.
     *
     * @param ctx  MD-SAL datastore transaction context.
     * @return  {@link VnodeState#UP} if this MAC mapping is active.
     *          {@link VnodeState#DOWN} otherwise.
     */
    public VnodeState getState(TxContext ctx) {
        return toVnodeState(isActive(ctx));
    }

    /**
     * Resume the MAC mapping.
     *
     * @param ctx  MD-SAL datastore transaction context.
     * @return  A {@link MacMap} instance on success.
     *          {@code null} on failure.
     */
    public MacMap resume(TxContext ctx) {
        // Prepare the status for this MAC mapping.
        MacMapIdentifier mapId = getIdentifier();
        MacMapStatusReader reader = ctx.getSpecific(MacMapStatusReader.class);
        VTNMacMapStatus vmst = new VTNMacMapStatus();
        reader.put(mapId, vmst);

        // Register this MAC mapping.
        MacMapChange change = new MacMapChange(
            config.getAllowedHosts(), config.getDeniedHosts(), DONT_PURGE);
        try {
            registerMacMap(ctx, mapId, change);
        } catch (VTNException | RuntimeException e) {
            ctx.log(LOG, VTNLogLevel.ERROR, e,
                    "%s: Failed to resume MAC mapping.", mapId);
            return null;
        }

        // Set empty mac-map-status container because no host is mapped by
        // MAC mapping on startup.
        return new MacMapBuilder().
            setMacMapConfig(config.toMacMapConfig()).
            setMacMapStatus(new MacMapStatusBuilder().build()).
            build();
    }

    /**
     * Destroy the MAC mapping.
     *
     * @param ctx     MD-SAL datastore transaction context.
     * @param retain  {@code true} means that the parent vBridge will be
     *                retained. {@code false} means that the parent vBridge
     *                is being destroyed.
     * @throws VTNException  An error occurred.
     */
    public void destroy(TxContext ctx, boolean retain) {
        VTNMacMapConfig cfg = getConfig();
        if (!cfg.isEmpty()) {
            // Unregister MAC mapping.
            int flags = REMOVING;
            if (!retain) {
                flags |= DONT_PURGE;
            }

            MacMapIdentifier mapId = getIdentifier();
            MacMapChange change = new MacMapChange(
                cfg.getAllowedHosts(), cfg.getDeniedHosts(), flags);
            try {
                registerMacMap(ctx, mapId, change);
            } catch (VTNException | RuntimeException e) {
                ctx.log(LOG, VTNLogLevel.ERROR, e,
                        "%s: Failed to unregister MAC mapping.", mapId);
                // FALLTHROUGH
            }
        }
    }

    /**
     * Invoked when a physical switch is added, removed, or changed.
     *
     * @param ctx  MD-SAL datastore transaction context.
     * @param ev   A {@link VtnNodeEvent} instance.
     * @return  New state of this virtual interface.
     */
    public VnodeState notifyNode(TxContext ctx, VtnNodeEvent ev) {
        VnodeState state;
        VtnUpdateType utype = ev.getUpdateType();
        if (utype == VtnUpdateType.REMOVED) {
            // Inactivate all the MAC mappings corresponding to the removed
            // node.
            NodePortFilter filter = new NodePortFilter(ev.getSalNode());
            state = inactivate(ctx, filter);
        } else {
            state = getState(ctx);
        }

        if (LOG.isTraceEnabled()) {
            LOG.trace("{}: notifyNode: {}: {}: state={}",
                      getIdentifier(), ev.getSalNode(), utype, state);
        }

        return state;
    }

    /**
     * Invoked when a physical switch port is added, removed, or changed.
     *
     * @param ctx  MD-SAL datastore transaction context.
     * @param ev   A {@link VtnPortEvent} instance.
     * @return  New state of this virtual interface.
     */
    public VnodeState notifyPort(TxContext ctx, VtnPortEvent ev) {
        VnodeState state;
        VtnUpdateType utype = ev.getUpdateType();
        if (utype == VtnUpdateType.REMOVED ||
            !InventoryUtils.isEnabledEdge(ev.getVtnPort())) {
            // Inactivate all the MAC mappings corresponding to the removed
            // or inactive switch port.
            SpecificPortFilter filter =
                new SpecificPortFilter(ev.getSalPort());
            state = inactivate(ctx, filter);
        } else {
            state = getState(ctx);
        }

        if (LOG.isTraceEnabled()) {
            LOG.trace("{}: notifyPort: {}: {}: state={}",
                      getIdentifier(), ev.getSalPort(), utype, state);
        }

        return state;
    }

    /**
     * Activate the specified host in the MAC mapping.
     *
     * @param ctx    MD-SAL datastore transaction context.
     * @param vbr    The parent vBridge.
     * @param eaddr  The MAC address of the host.
     * @param sport  A {@link SalPort} instance that specifies the switch port
     *               where the host was detected.
     * @param vid    The VLAN ID associated with the specified host.
     * @return  {@code true} on success.
     *          {@code false} on failure.
     * @throws VTNException  An error occurred.
     */
    public boolean activate(TxContext ctx, VBridge vbr, EtherAddress eaddr,
                            SalPort sport, int vid) throws VTNException {
        MacMapIdentifier mapId = getIdentifier();
        MacVlan mv = new MacVlan(eaddr.getAddress(), vid);
        try {
            boolean activated = new MacMapActivation(mapId, mv, sport).
                activate(ctx);
            if (activated) {
                // The parent vBridge status must be updated because the status
                // of the MAC mapping has been changed.
                vbr.updateState(ctx);
            }

            return true;
        } catch (MacMapException e) {
            ctx.log(LOG, VTNLogLevel.ERROR, e,
                    "%s: %s: host=%s, port=%s", mapId, e.getErrorMessage(),
                    mv, sport);
        }

        return false;
    }

    /**
     * Determine whether the given host is currently mapped by the MAC mapping
     * or not.
     *
     * @param ctx    MD-SAL datastore transaction context.
     * @param eaddr  The MAC address of the host.
     * @param sport  A {@link SalPort} instance that specifies the switch port
     *               where the host was detected.
     * @param vid    The VLAN ID associated with the specified host.
     * @return  {@code true} if the given host is mapped by the MAC mapping.
     *          {@code false} if not mapped.
     * @throws VTNException  An error occurred.
     */
    public boolean isActive(TxContext ctx, EtherAddress eaddr, SalPort sport,
                            int vid) throws VTNException {
        MacMapStatusReader reader = ctx.getSpecific(MacMapStatusReader.class);
        MacMapIdentifier mapId = getIdentifier();
        VTNMacMapStatus vmst = reader.get(mapId);
        MacVlan mv = new MacVlan(eaddr.getAddress(), vid);
        SalPort mapped = vmst.getPort(mv);
        boolean active = sport.equalsPort(mapped);
        if (!active && mapped != null) {
            ctx.log(LOG, VTNLogLevel.WARN,
                    "{}: Unexpected port is mapped: host={}, port={}, " +
                    "mapped={}", mapId, mv, sport, mapped);
        }

        return active;
    }

    /**
     * Transmit the given packet to the physical network mapped by this
     * VLAN mapping.
     *
     * @param pctx  A runtime context for a received packet.
     * @param vbr   A {@link VBridge} instance that contains this VLAN mapping.
     * @param sent  A set of {@link PortVlan} which indicates the network
     *              already processed.
     * @throws RedirectFlowException
     *    The given packet was redirected by a flow filter.
     * @throws VTNException  An error occurred.
     */
    public void transmit(PacketContext pctx, VBridge vbr, Set<PortVlan> sent)
        throws RedirectFlowException, VTNException {
        // Determine all the networks mapped by this MAC mapping.
        TxContext ctx = pctx.getTxContext();
        MacMapIdentifier mapId = getIdentifier();
        MacMapStatusReader reader = ctx.getSpecific(MacMapStatusReader.class);
        VTNMacMapStatus vmst = reader.get(mapId);
        Set<PortVlan> networks = vmst.getNetworks();
        if (networks == null) {
            LOG.trace("{}: transmit: MAC mapping is not active", mapId);
        } else {
            for (PortVlan pv: networks) {
                if (sent.add(pv)) {
                    int vid = pv.getVlanId();
                    PacketContext pc;
                    Ethernet frame;
                    try {
                        // Apply output flow filters.
                        pc = vbr.evaluateOutputFilter(pctx, vid);

                        // Create a new Ethernet frame to be transmitted.
                        frame = pc.createFrame(vid);
                    } catch (DropFlowException e) {
                        // Filtered out by DROP filter.
                        continue;
                    }

                    SalPort sport = pv.getPort();
                    if (LOG.isTraceEnabled()) {
                        LOG.trace("{}: Transmit packet to MAC mapping: {}",
                                  mapId,
                                  pc.getDescription(frame, sport, vid));
                    }

                    pc.transmit(sport, frame);
                }
            }
        }
    }

    /**
     * Return the configuration of the MAC mapping.
     *
     * @return  A {@link VTNMacMapConfig} instance.
     */
    private VTNMacMapConfig getConfig() {
        VTNMacMapConfig cfg = config;
        if (cfg == null) {
            try {
                cfg = new VTNMacMapConfig(getInitialValue().getMacMapConfig());
            } catch (RpcException e) {
                // This should never happen.
                throw new IllegalStateException(
                    "Broken MAC mapping configuration", e);
            }
            config = cfg;
        }

        return cfg;
    }

    /**
     * Determine whether at least one host is actually mapped by this
     * MAC mapping or not.
     *
     * @param ctx  A runtime context for transaction task.
     * @return  {@code true} if at least one host is actually mapped by
     *          this MAC mapping. {@code false} otherwise.
     */
    private boolean isActive(TxContext ctx) {
        MacMapStatusReader reader = ctx.getSpecific(MacMapStatusReader.class);
        VTNMacMapStatus vmst = reader.getCached(getIdentifier());
        boolean active;
        if (vmst == null) {
            // Determine the status using the initial data.
            MacMapStatus mms = getInitialValue().getMacMapStatus();
            active = !MiscUtils.isEmpty(mms.getMappedHost());
        } else {
            active = vmst.hasMapping();
        }

        return active;
    }

    /**
     * Inactivate all the MAC mappings detected on switch ports specified by
     * the port filter.
     *
     * @param ctx     MD-SAL datastore transaction context.
     * @param filter  A {@link PortFilter} instance which selects switch ports.
     * @return  A {@link VnodeState} instance that indicates the MAC mapping
     *          state.
     * @throws VTNException  Anerror occurred.
     */
    private VnodeState inactivate(TxContext ctx, PortFilter filter) {
        MacMapIdentifier mapId = getIdentifier();
        try {
            return toVnodeState(inactivateMacMap(ctx, mapId, filter));
        } catch (VTNException | RuntimeException e) {
            ctx.log(LOG, VTNLogLevel.ERROR, e,
                    "%s: Unable to inactivate MAC mapping: filter=%s",
                    mapId, filter);
            return getState(ctx);
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

    // VirtualMapNode

    /**
     * Return the identifier for the virtual mapping which maps the given host.
     *
     * @param eaddr  An {@link EtherAddress} instance that specifies the MAC
     *               address of the mapped host.
     *               {@code null} should be treated as if the mapped host is
     *               not specified.
     * @param vid    The VLAN ID of the mapped host.
     * @return  The identifier for the virtual network mapping.
     */
    @Override
    public VBridgeMapIdentifier<?> getIdentifier(EtherAddress eaddr, int vid) {
        MacMapIdentifier mapId = getIdentifier();
        return (eaddr == null)
            ? mapId
            : new MacMapHostIdentifier(
                mapId, new MacVlan(eaddr.getAddress(), vid));
    }

    /**
     * Determine whether this MAC mapping is administravely enabled or not.
     *
     * @return  Always {@code true} because the MAC mapping cannot be
     *          disabled administratively.
     */
    @Override
    public boolean isEnabled() {
        return true;
    }

    /**
     * Return a {@link VNodeHop} instance that indicates the packet was mapped
     * by the MAC mapping.
     *
     * @param eaddr  An {@link EtherAddress} instance that specifies the MAC
     *               address of the mapped host.
     *               {@code null} should be treated as if the mapped host is
     *               not specified.
     * @param vid    The VLAN ID of the mapped host.
     * @return  A {@link VNodeHop} instance.
     */
    @Override
    public VNodeHop getIngressHop(EtherAddress eaddr, int vid) {
        return new VNodeHop(getIdentifier(eaddr, vid),
                            VirtualRouteReason.MACMAPPED);
    }

    /**
     * This method does nothing.
     *
     * @param pctx  Unused.
     */
    @Override
    public void disableInput(PacketContext pctx) {
    }

    /**
     * This method does nothing.
     *
     * <p>
     *   The input filters configured in the vBridge should be evaluated
     *   by {@link VBridge}.
     * </p>
     *
     * @param pctx  Unused.
     * @param vid   Unused.
     */
    @Override
    public void filterPacket(PacketContext pctx, int vid) {
    }

    /**
     * Evaluate flow filters for outgoing packet to be transmitted by this
     * MAC mapping.
     *
     * @param pctx    A runtime context for a received packet.
     * @param vid     A VLAN ID to be used for packet matching.
     *                A VLAN ID configured in the given packet is used if a
     *                negative value is specified.
     * @param bridge  A {@link VirtualBridge} instance associated with this
     *                virtual mapping.
     * @return  A {@link PacketContext} to be used for transmitting packet.
     * @throws DropFlowException
     *    The given packet was discarded by a flow filter.
     * @throws RedirectFlowException
     *    The given packet was redirected by a flow filter.
     */
    @Override
    public PacketContext filterPacket(PacketContext pctx, int vid,
                                      VirtualBridge<?> bridge)
        throws DropFlowException, RedirectFlowException {
        return bridge.evaluateOutputFilter(pctx, vid);
    }
}
