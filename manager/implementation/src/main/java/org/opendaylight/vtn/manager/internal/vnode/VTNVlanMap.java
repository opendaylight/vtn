/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.vnode;

import static org.opendaylight.vtn.manager.internal.util.MiscUtils.toVnodeState;
import static org.opendaylight.vtn.manager.internal.vnode.MappingRegistry.registerVlanMap;
import static org.opendaylight.vtn.manager.internal.vnode.MappingRegistry.unregisterVlanMap;

import java.util.HashSet;
import java.util.List;
import java.util.Set;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import org.opendaylight.vtn.manager.VTNException;
import org.opendaylight.vtn.manager.packet.Ethernet;
import org.opendaylight.vtn.manager.util.EtherAddress;

import org.opendaylight.vtn.manager.internal.TxContext;
import org.opendaylight.vtn.manager.internal.inventory.VtnNodeEvent;
import org.opendaylight.vtn.manager.internal.inventory.VtnPortEvent;
import org.opendaylight.vtn.manager.internal.util.flow.VNodeHop;
import org.opendaylight.vtn.manager.internal.util.flow.filter.DropFlowException;
import org.opendaylight.vtn.manager.internal.util.flow.filter.RedirectFlowException;
import org.opendaylight.vtn.manager.internal.util.inventory.InventoryReader;
import org.opendaylight.vtn.manager.internal.util.inventory.InventoryUtils;
import org.opendaylight.vtn.manager.internal.util.inventory.NodeVlan;
import org.opendaylight.vtn.manager.internal.util.inventory.PortVlan;
import org.opendaylight.vtn.manager.internal.util.inventory.SalNode;
import org.opendaylight.vtn.manager.internal.util.inventory.SalPort;
import org.opendaylight.vtn.manager.internal.util.log.VTNLogLevel;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;
import org.opendaylight.vtn.manager.internal.util.vnode.BridgeIdentifier;
import org.opendaylight.vtn.manager.internal.util.vnode.VTNVlanMapConfig;
import org.opendaylight.vtn.manager.internal.util.vnode.VlanMapIdentifier;

import org.opendaylight.controller.md.sal.binding.api.ReadWriteTransaction;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.VirtualRouteReason;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.node.info.VtnPort;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.nodes.VtnNode;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.vlan.rev150907.vtn.vlan.map.info.VlanMapStatus;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.vlan.rev150907.vtn.vlan.map.info.VlanMapStatusBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.vlan.rev150907.vtn.vlan.mappable.VlanMap;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.vlan.rev150907.vtn.vlan.mappable.VlanMapBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeState;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.vbridge.list.Vbridge;

/**
 * {@code VTNVlanMap} describes a configuration and runtime status for a
 * VLAN mapping.
 */
public final class VTNVlanMap
    extends VirtualElement<VlanMap, VlanMapIdentifier>
    implements VirtualMapNode {
    /**
     * A logger instance.
     */
    private static final Logger  LOG =
        LoggerFactory.getLogger(VTNVlanMap.class);

    /**
     * VLAN mapping configuration.
     */
    private final VTNVlanMapConfig  config;

    /**
     * Determine whether the VLAN mapping is active or not.
     */
    private boolean  active;

    /**
     * Determine whether the given node is suitable for the VLAN mapping or
     * not.
     *
     * @param ctx    A runtime context for MD-SAL datastore transaction task.
     * @param snode  A {@link SalNode} instance which specifies the node
     *               to be tested. {@code null} means that all the switches
     *               are targeted.
     * @return  {@code true} is returned only if the given node is valid.
     * @throws VTNException  An error occurred.
     */
    public static boolean checkNode(TxContext ctx, SalNode snode)
        throws VTNException {
        boolean ret = (snode == null);
        if (!ret) {
            InventoryReader reader =
                ctx.getReadSpecific(InventoryReader.class);
            VtnNode vnode = reader.get(snode);
            ret = InventoryUtils.hasEdgePort(vnode);
        }

        return ret;
    }

    /**
     * Construct a new instance.
     *
     * @param vbrId  The identifier for a vBridge that contains this VLAN
     *               mapping.
     * @param vmc    VLAN mapping configuration.
     */
    public VTNVlanMap(BridgeIdentifier<Vbridge> vbrId, VTNVlanMapConfig vmc) {
        super(new VlanMapIdentifier(vbrId, vmc.getMapId()));
        config = vmc;
    }

    /**
     * Construct a new instance.
     *
     * @param vbrId  The identifier for a vBridge that contains this VLAN
     *               mapping.
     * @param vlmap  A {@link VlanMap} instance read from the MD-SAL datastore.
     */
    public VTNVlanMap(BridgeIdentifier<Vbridge> vbrId, VlanMap vlmap) {
        this(new VlanMapIdentifier(vbrId, vlmap.getMapId()), vlmap);
    }

    /**
     * Construct a new instance.
     *
     * @param vmapId  The identifier for the VLAN mapping.
     * @param vlmap   A {@link VlanMap} instance.
     */
    public VTNVlanMap(VlanMapIdentifier vmapId, VlanMap vlmap) {
        super(vmapId, vlmap);
        try {
            config = new VTNVlanMapConfig(vlmap.getVlanMapConfig());
        } catch (RpcException e) {
            // This should never happen.
            throw new IllegalStateException(
                "Broken VLAN mapping configuration", e);
        }
        active = vlmap.getVlanMapStatus().isActive().booleanValue();

        assert vmapId.getMapId().equals(config.getMapId());
    }

    /**
     * Return the mapping identifier assigned to this VLAN mapping.
     *
     * @return  A mapping identifier assigned to this VLAN mapping.
     */
    public String getMapId() {
        return config.getMapId();
    }

    /**
     * Return the state of this VLAN mapping.
     *
     * @return  {@link VnodeState#UP} if this VLAN mapping is active.
     *          {@link VnodeState#DOWN} otherwise.
     */
    public VnodeState getState() {
        return toVnodeState(active);
    }

    /**
     * Return the VLAN mapping container.
     *
     * @return  A {@link VlanMap} instance.
     */
    public VlanMap getVlanMap() {
        return new VlanMapBuilder().
            setMapId(config.getMapId()).
            setVlanMapConfig(config.toVlanMapConfig()).
            setVlanMapStatus(getVlanMapStatus()).
            build();
    }

    /**
     * Return the runtime status information of the VLAN mapping.
     *
     * @return  A {@link VlanMapStatus} instance.
     */
    public VlanMapStatus getVlanMapStatus() {
        return new VlanMapStatusBuilder().setActive(active).build();
    }

    /**
     * Register the VLAN mapping.
     *
     * @param ctx    MD-SAL datastore transaction context.
     * @param purge  If {@code true}, this method purges obsolete network
     *               caches as appropriate.
     * @return {@link VnodeState#UP} if this VLAN mapping has been activated.
     *         {@code VnodeState#DOWN} otherwise.
     * @throws VTNException  An error occurred.
     */
    public VnodeState register(TxContext ctx, boolean purge)
        throws VTNException {
        // Register this VLAN mapping.
        NodeVlan nv = config.getNodeVlan();
        VlanMapIdentifier mapId = getIdentifier();
        registerVlanMap(ctx, mapId, nv, purge);

        // Update the status of the VLAN mapping.
        SalNode snode = config.getTargetNode();
        active = checkNode(ctx, snode);
        return getState();
    }

    /**
     * Resume the VLAN mapping.
     *
     * @param ctx    MD-SAL datastore transaction context.
     * @param list   A list of {@link VlanMap} instances to store resumed
     *               VLAN mapping.
     * @return  A {@link VnodeState} instance on success.
     *          {@code null} on failure.
     */
    public VnodeState resume(TxContext ctx, List<VlanMap> list) {
        VnodeState state;
        try {
            register(ctx, false);
            list.add(getVlanMap());
            state = getState();
        } catch (VTNException | RuntimeException e) {
            ctx.log(LOG, VTNLogLevel.ERROR, e,
                    "%s: Failed to resume VLAN mapping: %s",
                    getIdentifier(), e.getMessage());
            state = null;
        }

        return state;
    }

    /**
     * Submit runtime status changes to the MD-SAL datastore.
     *
     * @param ctx  D-SAL datastore transaction context.
     */
    public void submit(TxContext ctx) {
        VlanMapStatus vmst = getInitialValue().getVlanMapStatus();
        if (vmst.isActive().booleanValue() != active) {
            // Need to update status.
            VlanMapIdentifier mapId = getIdentifier();
            InstanceIdentifier<VlanMapStatus> path = mapId.getStatusPath();
            LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
            ReadWriteTransaction tx = ctx.getReadWriteTransaction();
            tx.put(oper, path, getVlanMapStatus(), false);
        }
    }

    /**
     * Destroy the VLAN mapping.
     *
     * @param ctx     MD-SAL datastore transaction context.
     * @param retain  {@code true} means that the parent vBridge will be
     *                retained. {@code false} means that the parent vBridge
     *                is being destroyed.
     */
    public void destroy(TxContext ctx, boolean retain) {
        // Unregister the VLAN mapping.
        NodeVlan nv = config.getNodeVlan();
        VlanMapIdentifier mapId = getIdentifier();
        try {
            unregisterVlanMap(ctx, mapId, nv, retain);
        } catch (VTNException | RuntimeException e) {
            ctx.log(LOG, VTNLogLevel.ERROR, e,
                    "%s: Failed to unregister VLAN mapping.", mapId);
            // FALLTHROUGH
        }
    }

    /**
     * Invoked when a physical switch is added, removed, or changed.
     *
     * @param ctx  MD-SAL datastore transaction context.
     * @param ev   A {@link VtnNodeEvent} instance.
     * @return  New state of this VLAN mapping.
     */
    public VnodeState notifyNode(TxContext ctx, VtnNodeEvent ev) {
        SalNode snode = ev.getSalNode();
        if (snode.equalsNode(config.getTargetNode())) {
            VtnUpdateType utype = ev.getUpdateType();
            if (utype == VtnUpdateType.CREATED) {
                // The target node has been created.
                try {
                    active = checkNode(ctx, snode);
                } catch (VTNException | RuntimeException e) {
                    ctx.log(LOG, VTNLogLevel.ERROR, e,
                            "%s: Unable to update the node: %s",
                            getIdentifier(), snode);
                    // FALLTHROUGH
                }
            } else if (utype == VtnUpdateType.REMOVED) {
                // The target node has been removed.
                active = false;
            }

            if (LOG.isTraceEnabled()) {
                LOG.trace("{}: notifyNode: {}: {}: active={}",
                          getIdentifier(), snode, utype, active);
            }
            submit(ctx);
        }

        return getState();
    }

    /**
     * Invoked when a physical switch port is added, removed, or changed.
     *
     * @param ctx  MD-SAL datastore transaction context.
     * @param ev   A {@link VtnPortEvent} instance.
     * @return  New state of this virtual interface.
     */
    public VnodeState notifyPort(TxContext ctx, VtnPortEvent ev) {
        SalPort sport = ev.getSalPort();
        SalNode snode = sport.getSalNode();
        try {
            if (snode.equalsNode(config.getTargetNode())) {
                // VLAN mapping can work if at least one physical switch port
                // is up.
                VtnPort vport = ev.getVtnPort();
                boolean valid = InventoryUtils.isEnabledEdge(vport);
                if (!valid) {
                    InventoryReader reader = ctx.
                        getReadSpecific(InventoryReader.class);
                    VtnNode vnode = reader.get(snode);
                    valid = InventoryUtils.hasEdgePort(vnode);
                }

                active = valid;

                if (LOG.isTraceEnabled()) {
                    LOG.trace("{}: notifyPort: {}: {}: active={}",
                              getIdentifier(), sport, ev.getUpdateType(),
                              active);
                }

                submit(ctx);
            }
        } catch (VTNException | RuntimeException e) {
            ctx.log(LOG, VTNLogLevel.ERROR, e,
                    "%s: Unable to update the VLAN mapping status: port=%s",
                    getIdentifier(), sport);
            // FALLTHROUGH
        }

        return getState();
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
        // Determine edge ports of this VLAN mapping.
        SalNode snode = config.getTargetNode();
        int vid = config.getVlanId();

        Set<SalPort> ports = new HashSet<>();
        TxContext ctx = pctx.getTxContext();
        VlanMapPortFilter filter =
            VlanMapPortFilter.create(ctx, snode, vid, sent);
        InventoryReader reader = pctx.getInventoryReader();
        reader.collectUpEdgePorts(ports, filter);

        if (ports.isEmpty()) {
            LOG.trace("{}: transmit: No port is available", getIdentifier());
        } else {
            PacketContext pc;
            Ethernet frame;
            try {
                // Apply output flow filters.
                pc = vbr.evaluateOutputFilter(pctx, vid);

                // Create a new Ethernet frame to be transmitted.
                frame = pc.createFrame(vid);
            } catch (DropFlowException e) {
                // Filtered out by DROP filter.
                return;
            }

            for (SalPort sport: ports) {
                PortVlan pv = new PortVlan(sport, vid);
                if (sent.add(pv)) {
                    if (LOG.isTraceEnabled()) {
                        LOG.trace("{}: Transmit packet to VLAN mapping: {}",
                                  getIdentifier(),
                                  pc.getDescription(frame, sport, vid));
                    }

                    pc.transmit(sport, frame);
                }
            }
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
     * <p>
     *   This method always returns the identifier for this VLAN mapping
     *   because VLAN mapping does not specifies the host to be mapped.
     * </p>
     *
     * @param eaddr  Unused.
     * @param vid    Unused.
     * @return  The identifier for the VLAN mapping.
     */
    @Override
    public VlanMapIdentifier getIdentifier(EtherAddress eaddr, int vid) {
        return getIdentifier();
    }

    /**
     * Determine whether this VLAN mapping is administravely enabled or not.
     *
     * @return  Always {@code true} because the VLAN mapping cannot be
     *          disabled administratively.
     */
    @Override
    public boolean isEnabled() {
        return true;
    }

    /**
     * Return a {@link VNodeHop} instance that indicates the packet was mapped
     * by the VLAN mapping.
     *
     * @param eaddr  Unused.
     * @param vid    Unused.
     * @return  A {@link VNodeHop} instance.
     */
    @Override
    public VNodeHop getIngressHop(EtherAddress eaddr, int vid) {
        return new VNodeHop(getIdentifier(), VirtualRouteReason.VLANMAPPED);
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
     * VLAN mapping.
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
