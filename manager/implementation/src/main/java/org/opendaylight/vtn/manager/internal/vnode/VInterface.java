/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.vnode;

import static org.opendaylight.vtn.manager.internal.util.flow.filter.FlowFilterList.VLAN_UNSPEC;
import static org.opendaylight.vtn.manager.internal.vnode.MappingRegistry.registerPortMap;

import java.util.Objects;
import java.util.Set;

import org.slf4j.Logger;

import org.opendaylight.vtn.manager.VTNException;
import org.opendaylight.vtn.manager.packet.Ethernet;
import org.opendaylight.vtn.manager.util.EtherAddress;

import org.opendaylight.vtn.manager.internal.TxContext;
import org.opendaylight.vtn.manager.internal.flow.remove.EdgePortFlowRemover;
import org.opendaylight.vtn.manager.internal.flow.remove.FlowRemoverQueue;
import org.opendaylight.vtn.manager.internal.flow.remove.VNodeFlowRemover;
import org.opendaylight.vtn.manager.internal.inventory.VtnNodeEvent;
import org.opendaylight.vtn.manager.internal.inventory.VtnPortEvent;
import org.opendaylight.vtn.manager.internal.util.MiscUtils;
import org.opendaylight.vtn.manager.internal.util.flow.VNodeHop;
import org.opendaylight.vtn.manager.internal.util.flow.VTNFlowBuilder;
import org.opendaylight.vtn.manager.internal.util.flow.filter.DropFlowException;
import org.opendaylight.vtn.manager.internal.util.flow.filter.FlowFilterList;
import org.opendaylight.vtn.manager.internal.util.flow.filter.RedirectFlowException;
import org.opendaylight.vtn.manager.internal.util.flow.match.VTNEtherMatch;
import org.opendaylight.vtn.manager.internal.util.flow.match.VTNMatch;
import org.opendaylight.vtn.manager.internal.util.inventory.InventoryReader;
import org.opendaylight.vtn.manager.internal.util.inventory.InventoryUtils;
import org.opendaylight.vtn.manager.internal.util.inventory.PortVlan;
import org.opendaylight.vtn.manager.internal.util.inventory.SalNode;
import org.opendaylight.vtn.manager.internal.util.inventory.SalPort;
import org.opendaylight.vtn.manager.internal.util.log.VTNLogLevel;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;
import org.opendaylight.vtn.manager.internal.util.vnode.VBridgeIfIdentifier;
import org.opendaylight.vtn.manager.internal.util.vnode.VInterfaceIdentifier;
import org.opendaylight.vtn.manager.internal.util.vnode.VNodeType;
import org.opendaylight.vtn.manager.internal.util.vnode.VTNPortMapConfig;
import org.opendaylight.vtn.manager.internal.util.vnode.VTerminalIfIdentifier;

import org.opendaylight.controller.md.sal.binding.api.ReadWriteTransaction;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.VirtualRouteReason;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.node.info.VtnPort;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.port.rev150907.vtn.port.mappable.PortMapConfig;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeState;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.VtnPortMappableBridge;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.VtnVinterfaceConfig;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.vtn.mappable.vinterface.VinterfaceStatus;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.vtn.mappable.vinterface.VinterfaceStatusBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.vtn.mappable.vinterface.list.Vinterface;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.vtn.mappable.vinterface.list.VinterfaceBuilder;

/**
 * {@code VInterface} describes a configuration and runtime status for a
 * virtual interface.
 *
 * @param <B>  The type of the virtual bridge that contains the virtual
 *             interrace.
 */
public abstract class VInterface<B extends VtnPortMappableBridge>
    extends VirtualNode<Vinterface, VInterfaceIdentifier<B>>
    implements VirtualMapNode {
    /**
     * A boolean value which determines whether the virtual interface is to be
     * enabled or not.
     */
    private boolean  enabled;

    /**
     * The current state of the virtual interface.
     */
    private VnodeState  state;

    /**
     * The state of the physical switch port mapped to this virtual interface.
     */
    private VnodeState  portState;

    /**
     * Port mapping configuration.
     */
    private VTNPortMapConfig  portMap;

    /**
     * True if the port mapping configuration is cached.
     */
    private boolean  portMapCached;

    /**
     * A physical swtich port acctually mapped to this virtual interface.
     */
    private SalPort  mappedPort;

    /**
     * A list of vInterface input filters.
     */
    private FlowFilterList  inputFilter;

    /**
     * A list of vInterface output filters.
     */
    private FlowFilterList  outputFilter;

    /**
     * Construct a new virtual interface instance.
     *
     * @param ident  The virtual interface identifier.
     * @param vintf  The virtual interface.
     * @param <T>    The type of the parent virtual bridge.
     * @return  A {@link VInterface} instance.
     */
    public static final <T extends VtnPortMappableBridge> VInterface<T> create(
        VInterfaceIdentifier<T> ident, Vinterface vintf) {
        VNodeType type = ident.getType();
        if (type == VNodeType.VBRIDGE_IF) {
            @SuppressWarnings("unchecked")
            VBridgeIfIdentifier ifId = (VBridgeIfIdentifier)ident;
            @SuppressWarnings("unchecked")
            VInterface<T> vif =
                (VInterface<T>)new VBridgeInterface(ifId, vintf);
            return vif;
        }
        if (type == VNodeType.VTERMINAL_IF) {
            @SuppressWarnings("unchecked")
            VTerminalIfIdentifier ifId = (VTerminalIfIdentifier)ident;
            @SuppressWarnings("unchecked")
            VInterface<T> vif =
                (VInterface<T>)new VTerminalInterface(ifId, vintf);
            return vif;
        }

        // This should never happen.
        throw new IllegalArgumentException(
            "Unexpected interface identifier: " + ident);
    }

    /**
     * Construct a new instance.
     *
     * <p>
     *   This constructor is used to resume runtime status of the virtual
     *   interface on bootstrap.
     * </p>
     *
     * @param ifId  The location of the virtual interface.
     * @param vifc  Configuration of the virtual interface.
     * @param pmap  Port mapping configuration.
     */
    protected VInterface(VInterfaceIdentifier<B> ifId,
                         VtnVinterfaceConfig vifc, VTNPortMapConfig pmap) {
        super(ifId);

        // Initialize node state.
        state = VnodeState.UNKNOWN;
        portState = VnodeState.UNKNOWN;

        // Initialize configuration.
        initConfig(vifc);
        setPortMapConfig(pmap);
    }

    /**
     * Construct a new instance.
     *
     * @param ifId  The location of the virtual interface.
     * @param vif   A {@link Vinterface} instance read from the MD-SAL
     *              datastore.
     */
    protected VInterface(VInterfaceIdentifier<B> ifId, Vinterface vif) {
        super(ifId, vif);

        // Initialize confoiguration.
        initConfig(vif.getVinterfaceConfig());

        // Initialize runtime status.
        VinterfaceStatus ist = vif.getVinterfaceStatus();
        state = ist.getState();
        portState = ist.getEntityState();
        mappedPort = SalPort.create(ist.getMappedPort());
    }

    /**
     * Return the current state of this virtual interface.
     *
     * @return  The current state of this virtual interface.
     */
    public final VnodeState getState() {
        return state;
    }

    /**
     * Return the state of the physical switch port mapped to this virtual
     * interface.
     *
     * @return  The state of the switch port mapped to this virtual interface.
     */
    public final VnodeState getPortState() {
        return portState;
    }

    /**
     * Return the vinterface container associated with this instance.
     *
     * @return  A {@link Vinterface} instance.
     */
    public final Vinterface getVinterface() {
        // Update the port mapping configuration.
        VTNPortMapConfig pmap = getPortMap();
        PortMapConfig pmc = (pmap == null) ? null : pmap.toPortMapConfig();

        return new VinterfaceBuilder(getInitialValue()).
            setPortMapConfig(pmc).
            setVinterfaceStatus(getVinterfaceStatus()).
            build();
    }

    /**
     * Return the runtime status information of the virtual interface.
     *
     * @return  A {@link VinterfaceStatus} instance.
     */
    public final VinterfaceStatus getVinterfaceStatus() {
        VinterfaceStatusBuilder builder = new VinterfaceStatusBuilder().
            setState(state).
            setEntityState(portState);
        if (mappedPort != null) {
            builder.setMappedPort(mappedPort.getNodeConnectorId());
        }

        return builder.build();
    }

    /**
     * Change the configuration for the port mapping.
     *
     * @param ctx   MD-SAL datastore transaction context.
     * @param pmap  A {@link VTNPortMapConfig} that contains the port mapping
     *              configuration to be applied.
     *              Specifying {@code null} results in undefined behavior.
     * @return  A {@link VtnUpdateType} instance if the port mapping
     *          configuration has been changed. {@code null} if not changed.
     * @throws VTNException  An error occurred.
     */
    public final VtnUpdateType setPortMap(TxContext ctx, VTNPortMapConfig pmap)
        throws VTNException {
        VTNPortMapConfig oldConf = getPortMap();
        VtnUpdateType result;
        if (pmap.equals(oldConf)) {
            // No change.
            result = null;
        } else {
            PortVlan oldPv;
            if (oldConf == null) {
                result = VtnUpdateType.CREATED;
                oldPv = null;
            } else {
                result = VtnUpdateType.CHANGED;
                oldPv = oldConf.getMappedVlan(mappedPort);
            }
            setPortMap(ctx, pmap, oldPv, true);
            setPortMapConfig(pmap);
        }

        return result;
    }

    /**
     * Remove the port mapping from this interface.
     *
     * @param ctx   MD-SAL datastore transaction context.
     * @return  {@link VtnUpdateType#REMOVED} if the port mapping configuration
     *          has been removed. {@code null} if not changed.
     * @throws VTNException  An error occurred.
     */
    public final VtnUpdateType removePortMap(TxContext ctx)
        throws VTNException {
        VtnUpdateType result;
        VTNPortMapConfig pmap = getPortMap();
        if (pmap == null) {
            // No port mapping is configured.
            result = null;
        } else {
            result = VtnUpdateType.REMOVED;
            PortVlan oldPv = pmap.getMappedVlan(mappedPort);
            VInterfaceIdentifier<B> ifId = getIdentifier();
            registerPortMap(ctx, ifId, null, oldPv, true);
            setPortMapConfig(null);
            mappedPort = null;
            portState = VnodeState.UNKNOWN;
            setState(VnodeState.UNKNOWN);
        }

        return result;
    }

    /**
     * Set the enabled state to this virtual interface, and update the
     * vinterface-status container in the MD-SAL datastore.
     *
     * @param ctx    MD-SAL datastore transaction context.
     * @param value  A new value of the enabled state.
     * @throws VTNException  An error occurred.
     */
    public final void setEnabled(TxContext ctx, boolean value)
        throws VTNException {
        enabled = value;

        VTNPortMapConfig pmap = getPortMap();
        if (value) {
            // Determine new state by the state of the mapped port.
            if (pmap == null) {
                state = VnodeState.UNKNOWN;
            } else {
                updateState(ctx);
            }
        } else {
            // State of disabled interface must be DOWN.
            state = VnodeState.DOWN;
        }

        if (pmap != null) {
            // Invalidate all the network caches created by the port mapping.
            purgeCache(ctx, mappedPort, pmap.getVlanId());
        }

        // Put the status into the DS.
        putState(ctx);
    }

    /**
     * Destroy the virtual interface.
     *
     * @param ctx     MD-SAL datastore transaction context.
     * @param retain  {@code true} means that the parent virtual node will be
     *                retained. {@code false} means that the parent virtual
     *                node is being destroyed.
     */
    public final void destroy(TxContext ctx, boolean retain) {
        // Destroy the port mapping.
        VTNPortMapConfig pmap = getPortMap();
        if (pmap != null) {
            unmapPort(ctx, pmap.getMappedVlan(mappedPort), retain);
        }
    }

    /**
     * Determine whether flow filter list in this virtual interface is empty
     * or not.
     *
     * @return  {@code true} if this virtual interface contains no flow filter.
     *          {@code false} otherwise.
     */
    public final boolean isFilterEmpty() {
        return (isInputFilterEmpty() && isOutputFilterEmpty());
    }

    /**
     * Resume the port mapping.
     *
     * @param ctx  MD-SAL datastore transaction context.
     * @throws VTNException  An error occurred.
     */
    public final void resume(TxContext ctx) throws VTNException {
        VTNPortMapConfig pmap = portMap;
        if (pmap != null) {
            // Try to establish port mapping.
            setPortMap(ctx, pmap, null, false);
        }
    }

    /**
     * Invoked when a physical switch is added, removed, or changed.
     *
     * @param ctx  MD-SAL datastore transaction context.
     * @param ev   A {@link VtnNodeEvent} instance.
     * @return  New state of this virtual interface.
     */
    public final VnodeState notifyNode(TxContext ctx, VtnNodeEvent ev) {
        VtnUpdateType utype = ev.getUpdateType();
        if (utype == VtnUpdateType.REMOVED) {
            VTNPortMapConfig pmap = getPortMap();
            if (pmap != null) {
                SalPort mapped = mappedPort;
                SalNode snode = ev.getSalNode();
                if (mapped != null &&
                    mapped.getNodeNumber() == snode.getNodeNumber()) {
                    // Port mapping needs to be removed because the mapped port
                    // has been removed. MAC address table and flow entries
                    // corresponding to the removed node will be removed by the
                    // caller.
                    unmapPort(ctx, pmap.getMappedVlan(mapped), false);
                    putState(ctx);

                    getLogger().
                        trace("{}: notifyNode: {}: {}: enabled={}, state={}",
                              getIdentifier(), utype, snode, enabled, state);
                }
            }
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
    public final VnodeState notifyPort(TxContext ctx, VtnPortEvent ev) {
        VTNPortMapConfig pmap = getPortMap();
        if (pmap != null) {
            SalPort mapped = mappedPort;
            SalPort sport = ev.getSalPort();
            VtnPort vport = ev.getVtnPort();
            VtnUpdateType utype = ev.getUpdateType();
            if (mapped == null) {
                if (utype != VtnUpdateType.REMOVED &&
                    pmap.match(sport, vport)) {
                    // Try establish port mapping.
                    mapPort(ctx, pmap, sport, vport, true);
                }
            } else if (mapped.equals(sport)) {
                if (utype == VtnUpdateType.REMOVED) {
                    // The mapped port has been removed.
                    // MAC address table and flow entries corresponding to
                    // the removed port will be removed by the caller.
                    unmapPort(ctx, pmap.getMappedVlan(mapped), false);
                    putState(ctx);
                } else if (utype == VtnUpdateType.CHANGED) {
                    if (pmap.match(mapped, vport)) {
                        // Need to update the port state.
                        setMappedPort(mapped, vport);
                    } else {
                        // The mapped port no longer satisfies the condition
                        // specified by the port mapping configuration.
                        // In this case MAC address table and flow entries
                        // need to be purged here.
                        unmapPort(ctx, pmap.getMappedVlan(mapped), true);
                    }
                    putState(ctx);
                }
            }

            getLogger().trace("{}: notifyPort: {}: {}: enabled={}, state={}",
                              getIdentifier(), utype, sport, enabled, state);
        }

        return state;
    }

    /**
     * Determine whether the specified VLAN is mapped to this virtual interface
     * or not.
     *
     * <p>
     *   Note that this method does not see the state of this node.
     *   This method returns {@code true} if the specified VLAN is mapped to
     *   this virtual interface by port mapping.
     * </p>
     *
     * @param sport  A {@link SalPort} instance that specifies the physical
     * @param vid    The VLAN ID.
     * @return  {@code true} only if the specified VLAN is mapped to this
     *          virtual interface.
     *          treated as input of this node.
     */
    public final boolean match(SalPort sport, int vid) {
        VTNPortMapConfig pmap = getPortMap();
        return (pmap != null && pmap.getVlanId() == vid &&
                sport.equalsPort(mappedPort));
    }

    /**
     * Transmit the given packet to the physical network mapped to this
     * virtual interface.
     *
     * @param pctx  A runtime context for a received packet.
     * @param sent  A set of {@link PortVlan} which indicates the network
     *              already processed.
     * @throws RedirectFlowException
     *    The given packet was redirected by a flow filter.
     * @throws VTNException  An error occurred.
     */
    public final void transmit(PacketContext pctx, Set<PortVlan> sent)
        throws RedirectFlowException, VTNException {
        VTNPortMapConfig pmap = getPortMap();
        SalPort egress = mappedPort;
        if (pmap != null && state == VnodeState.UP && egress != null) {
            int vid = pmap.getVlanId();
            PortVlan pv = new PortVlan(egress, vid);
            if (sent.add(pv)) {
                Logger logger = getLogger();
                PacketContext pc;
                Ethernet frame;
                TxContext ctx = pctx.getTxContext();
                try {
                    // Apply output flow filters.
                    pc = pctx.evaluate(getOutputFilter(ctx), vid);

                    // Create a new Ethernet frame to be transmitted.
                    frame = pc.createFrame(vid);
                } catch (DropFlowException e) {
                    // Filtered out by DROP filter.
                    return;
                }

                if (logger.isTraceEnabled()) {
                    logger.trace("{}: Transmit packet to the interface: {}",
                                 getIdentifier(),
                                 pc.getDescription(frame, egress, vid));
                }

                pc.transmit(egress, frame);
            }
        }
    }

    /**
     * Return the VLAN ID mapped to this virtual interface.
     *
     * @return  A VLAN ID mapped to this interface.
     *          A negative value is returned if port mapping is not configured.
     */
    public int getVlanId() {
        VTNPortMapConfig pmap = getPortMap();
        return (pmap == null) ? VLAN_UNSPEC : pmap.getVlanId();
    }

    /**
     * Redirect the given packet to this virtual interface as outgoing packet.
     *
     * @param pctx    A runtime context for a received packet.
     * @param rex     An exception that keeps information about the packet
     *                redirection.
     * @param bridge  A {@link VirtualBridge} instance associated with this
     *                virtual interface.
     * @throws DropFlowException
     *    The given packet was discarded by a DROP flow filter.
     * @throws RedirectFlowException
     *    The given packet was redirected by a REDIRECT flow filter.
     * @throws VTNException  An error occurred.
     */
    public final void redirect(PacketContext pctx, RedirectFlowException rex,
                               VirtualBridge<B> bridge)
        throws DropFlowException, RedirectFlowException, VTNException {
        // Ensure that a physical switch port is mapped by port mapping.
        VTNPortMapConfig pmap = getPortMap();
        SalPort mapped = mappedPort;
        if (pmap == null || mapped == null) {
            pctx.notMapped(rex);
            throw new DropFlowException();
        }

        // Evaluate output flow filters.
        // Note that this should never clone a PacketContext.
        int vid = pmap.getVlanId();
        pctx.evaluate(getOutputFilter(pctx.getTxContext()), vid);

        // Forward the packet.
        bridge.forward(pctx, mapped, vid);
    }

    /**
     * Purge all the network caches associated with the VLAN specified by
     * a pair of physical switch port and VLAN ID.
     *
     * <p>
     *   This method is used to purge network data cached by obsolete port
     *   mapping.
     * </p>
     *
     * @param ctx    MD-SAL datastore transaction context.
     * @param sport  A {@link SalPort} instance that specifies the physical
     *               switch port mapped by the port mapping.
     *               Node that {@code null} is specified if no switch port
     *               is mapped by the port mapping.
     * @param vid    The VLAN ID specified by the port mapping configuration.
     * @throws VTNException  An error occurred.
     */
    protected final void purgeCache(TxContext ctx, SalPort sport, int vid)
        throws VTNException {
        VInterfaceIdentifier<B> ifId = getIdentifier();
        FlowRemoverQueue frq = ctx.getSpecific(FlowRemoverQueue.class);
        if (sport != null) {
            // Remove flow entries relevant to obsolete port mapping.
            String tname = ifId.getTenantNameString();
            EdgePortFlowRemover remover =
                new EdgePortFlowRemover(tname, sport, vid);
            frq.enqueue(remover);

            purgePortCache(ctx, sport, vid);
        }

        // Remove flow entries affected by this virtual interface.
        frq.enqueue(new VNodeFlowRemover(ifId));
    }

    /**
     * Purge all the network caches associated with the VLAN specified by
     * a pair of physical switch port and VLAN ID.
     *
     * <p>
     *   This method in this class does nothing.
     *   Subclass can override this method to purge network data cached by
     *   obsolete port mapping.
     * </p>
     *
     * @param ctx    MD-SAL datastore transaction context.
     * @param sport  A {@link SalPort} instance that specifies the physical
     *               switch port mapped by the port mapping.
     *               Specifying {@code null} results in undefined behavior.
     * @param vid    The VLAN ID specified by the port mapping configuration.
     * @throws VTNException  An error occurred.
     */
    protected abstract void purgePortCache(TxContext ctx, SalPort sport,
                                           int vid) throws VTNException;

    /**
     * Initialize configuration of the virtual interface.
     *
     * @param vifc  Configuration of the virtual interface.
     */
    private void initConfig(VtnVinterfaceConfig vifc) {
        Boolean en = vifc.isEnabled();
        boolean newEn = (en == null || en.booleanValue());
        enabled = newEn;

        // State of disabled interface must be DOWN.
        if (!newEn) {
            state = VnodeState.DOWN;
        }
    }

    /**
     * Set physical switch port mapped by port mapping.
     *
     * <p>
     *   The caller must guarantee that the port mapping is configured to
     *   this virtual interface.
     * </p>
     *
     * @param sport  A {@link SalPort} which specifies the switch port to be
     *               mapped.
     * @param vport  A {@link VtnPort} instance corresponding to {@code sport}.
     */
    private void setMappedPort(SalPort sport, VtnPort vport) {
        mappedPort = sport;
        updateState(sport, vport);
    }

    /**
     * Update the state of this virtual interface.
     *
     * <p>
     *   The caller must guarantee that the port mapping is configured to
     *   this virtual interface.
     * </p>
     *
     * @param ctx  MD-SAL datastore transaction context.
     * @throws VTNException  An error occurred.
     */
    private void updateState(TxContext ctx) throws VTNException {
        SalPort sport = mappedPort;
        if (sport == null) {
            state = VnodeState.DOWN;
        } else {
            InventoryReader reader =
                ctx.getReadSpecific(InventoryReader.class);
            updateState(sport, reader.get(sport));
        }
    }

    /**
     * Update the state of this virtual interface.
     *
     * <p>
     *   The caller must guarantee that the port mapping is configured to
     *   this virtual interface.
     * </p>
     *
     * @param sport  A {@link SalPort} which specifies the switch port mapped
     *               to this virtual interface.
     * @param vport  A {@link VtnPort} instance corresponding to {@code sport}.
     */
    private void updateState(SalPort sport, VtnPort vport) {
        VnodeState newState;
        if (sport == null) {
            portState = VnodeState.UNKNOWN;
            newState = VnodeState.DOWN;
        } else {
            portState = InventoryUtils.getPortState(vport);
            if (portState == VnodeState.UP && InventoryUtils.isEdge(vport)) {
                newState = VnodeState.UP;
            } else {
                newState = VnodeState.DOWN;
            }
        }

        setState(newState);
    }

    /**
     * Set the virtual interface state.
     *
     * @param st  New state of the virtual interface.
     */
    private void setState(VnodeState st) {
        // State of disabled interface must be DOWN.
        state = (enabled) ? st : VnodeState.DOWN;
    }

    /**
     * Map the specified physical switch port to this virtual interface, and
     * update the status information in the MD-SAL datastore.
     *
     * <p>
     *   The caller must guarantee that no physical switch port is mapped to
     *   this virtual interface.
     * </p>
     *
     * @param ctx    MD-SAL datastore transaction context.
     * @param pmap   The current port mapping configuration.
     * @param sport  A {@link SalPort} to be mapped.
     * @param vport  A {@link VtnPort} instance corresponding to {@code sport}.
     * @param purge  If {@code true}, this method purges obsolete network
     *               caches as appropriate.
     */
    private void mapPort(TxContext ctx, VTNPortMapConfig pmap, SalPort sport,
                         VtnPort vport, boolean purge) {
        PortVlan pv = pmap.getMappedVlan(sport);
        VInterfaceIdentifier<B> ifId = getIdentifier();
        boolean changed;
        try {
            changed = registerPortMap(ctx, ifId, pv, null, purge);
        } catch (VTNException | RuntimeException e) {
            ctx.log(getLogger(), VTNLogLevel.ERROR, e,
                    "%s: Unable to map switch port: %s",
                    getIdentifier(), pv);
            return;
        }

        if (changed) {
            setMappedPort(sport, vport);
            putState(ctx);
        } else {
            ctx.log(getLogger(), VTNLogLevel.ERROR,
                    "{}: Switch port is already mapped: {}",
                    getIdentifier(), pv);
        }
    }

    /**
     * Unmap the physical switch port currently mapped.
     *
     * @param ctx    MD-SAL datastore transaction context.
     * @param oldPv  A {@link PortVlan} instance that specifies the VLAN
     *               currently mapped by the port mapping.
     *               This method does nothing if {@code null} is specified.
     * @param purge  If {@code true} is specified, network caches originated
     *               by the port mapping will be purged.
     */
    private void unmapPort(TxContext ctx, PortVlan oldPv, boolean purge) {
        VInterfaceIdentifier<B> ifId = getIdentifier();
        try {
            if (registerPortMap(ctx, ifId, null, oldPv, purge)) {
                setMappedPort(null, null);
            }
        } catch (VTNException | RuntimeException e) {
            ctx.log(getLogger(), VTNLogLevel.ERROR, e,
                    "%s: Unable to unmap switch port: %s", ifId, oldPv);
            // FALLTHROUGH
        }
    }

    /**
     * Update the registration of the port mapping.
     *
     * @param ctx     MD-SAL datastore transaction context.
     * @param pmap    A {@link VTNPortMapConfig} instance that contains the
     *                port mapping configuration to be applied.
     *                Specifying {@code null} results in undefined behavior.
     * @param oldPv   A {@link PortVlan} instance that specifies the VLAN
     *                currently mapped to this virtual interface.
     *                {@code null} implies that no switch port is currently
     *                mapped.
     * @param purge   If {@code true}, this method purges obsolete network
     *                caches as appropriate.
     * @throws VTNException  An error occurred.
     */
    private void setPortMap(TxContext ctx, VTNPortMapConfig pmap,
                            PortVlan oldPv, boolean purge)
        throws VTNException {
        // Search for the switch port specified by the configuration.
        InventoryReader reader = ctx.getReadSpecific(InventoryReader.class);
        SalPort sport = reader.findPort(pmap.getTargetNode(), pmap);
        PortVlan newPv;
        VtnPort vport;
        if (sport == null) {
            // Physical switch port was not found.
            newPv = null;
            vport = null;
        } else {
            newPv = pmap.getMappedVlan(sport);
            vport = reader.get(sport);
        }

        VInterfaceIdentifier<B> ifId = getIdentifier();
        registerPortMap(ctx, ifId, newPv, oldPv, purge);
        setMappedPort(sport, vport);
    }

    /**
     * Return the port mapping configured in this virtual interface.
     *
     * @return  A {@link VTNPortMapConfig} instance or {@code null}.
     */
    private VTNPortMapConfig getPortMap() {
        VTNPortMapConfig pmap = portMap;
        if (!portMapCached) {
            PortMapConfig pmc = getInitialValue().getPortMapConfig();
            if (pmc == null) {
                pmap = null;
            } else {
                try {
                    pmap = new VTNPortMapConfig(pmc);
                } catch (RpcException e) {
                    throw new IllegalStateException(
                        "Broken port mapping configuration.", e);
                }
            }
            setPortMapConfig(pmap);
        }

        return pmap;
    }

    /**
     * Set the given port mapping configuration.
     *
     * @param pmap  A {@link VTNPortMapConfig} instance or {@code null}.
     */
    private void setPortMapConfig(VTNPortMapConfig pmap) {
        portMap = pmap;
        portMapCached = true;
    }

    /**
     * Determine whether the status of the virtual interface is changed or not.
     *
     * @return  {@code true} only if the status of the virtual interface is
     *          changed.
     */
    private boolean isDirty() {
        VinterfaceStatus ist = getInitialValue().getVinterfaceStatus();
        boolean ret = (state != ist.getState() ||
                       portState != ist.getEntityState());
        if (!ret) {
            String mapped = (mappedPort == null)
                ? null : mappedPort.toString();
            ret = !Objects.equals(mapped,
                                  MiscUtils.getValue(ist.getMappedPort()));
        }

        return ret;
    }

    /**
     * Update the runtime status information in the MD-SAL datastore.
     *
     * @param ctx  A runtime context for transaction task.
     */
    private void putState(TxContext ctx) {
        if (isDirty()) {
            ReadWriteTransaction tx = ctx.getReadWriteTransaction();
            InstanceIdentifier<VinterfaceStatus> path =
                getIdentifier().getStatusPath();
            LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
            tx.put(oper, path, getVinterfaceStatus(), false);
        }
    }

    /**
     * Return the vInterface input filter list.
     *
     * @param ctx  MD-SAL datastore transaction context.
     * @return  A {@link FlowFilterList} instance associated with the
     *          vInterface input filter list.
     */
    private FlowFilterList getInputFilter(TxContext ctx) {
        FlowFilterList ffl = inputFilter;
        if (ffl == null) {
            ffl = getFlowFilterList(
                ctx, false, getInitialValue().getVinterfaceInputFilter());
            inputFilter = ffl;
        }

        return ffl;
    }

    /**
     * Return the vInterface output filter list.
     *
     * @param ctx  MD-SAL datastore transaction context.
     * @return  A {@link FlowFilterList} instance associated with the
     *          vInterface output filter list.
     */
    private FlowFilterList getOutputFilter(TxContext ctx) {
        FlowFilterList ffl = outputFilter;
        if (ffl == null) {
            ffl = getFlowFilterList(
                ctx, true, getInitialValue().getVinterfaceOutputFilter());
            outputFilter = ffl;
        }

        return ffl;
    }

    /**
     * Determine whether the input flow filter list is empty or not.
     *
     * @return  {@code true} if this virtual interface does not contain
     *          input flow filter. {@code false} otherwise.
     */
    private boolean isInputFilterEmpty() {
        FlowFilterList ffl = inputFilter;
        return (ffl == null)
            ? FlowFilterList.isEmpty(
                getInitialValue().getVinterfaceInputFilter())
            : ffl.isEmpty();
    }

    /**
     * Determine whether the output flow filter list is empty or not.
     *
     * @return  {@code true} if this virtual interface does not contain
     *          output flow filter. {@code false} otherwise.
     */
    private boolean isOutputFilterEmpty() {
        FlowFilterList ffl = outputFilter;
        return (ffl == null)
            ? FlowFilterList.isEmpty(
                getInitialValue().getVinterfaceOutputFilter())
            : ffl.isEmpty();
    }

    /**
     * Construct a flow match that matches packets on the specifies VLAN.
     *
     * @param pctx  A runtime context for a received packet.
     * @param vid   A VLAN ID.
     * @return  A {@link VTNMatch} instance on success.
     *          {@code null} on failure.
     */
    private VTNMatch createMatch(PacketContext pctx, int vid) {
        try {
            VTNEtherMatch ematch =
                new VTNEtherMatch(null, null, null, vid, null);
            return new VTNMatch(ematch, null, null);
        } catch (RpcException e) {
            // This should never happen.
            pctx.getTxContext().
                log(getLogger(), VTNLogLevel.ERROR, e,
                    "%s: Failed to create VLAN match: %s",
                    getIdentifier(), vid);
        }

        return null;
    }

    // VirtualMapNode

    /**
     * Return the identifier for the virtual mapping which maps the given host.
     *
     * <p>
     *   This method always returns the identifier for this virtual interface
     *   because port mapping does not specifies the host to be mapped.
     * </p>
     *
     * @param eaddr  Unused.
     * @param vid    Unused.
     * @return  The identifier for the virtual interface.
     */
    @Override
    public final VInterfaceIdentifier<B> getIdentifier(EtherAddress eaddr,
                                                       int vid) {
        return getIdentifier();
    }

    /**
     * Determine whether this virtual interface is enabled or not.
     *
     * @return  {@code true} only if this virtual interface is enabled.
     */
    @Override
    public final boolean isEnabled() {
        return enabled;
    }

    /**
     * Return a {@link VNodeHop} instance that indicates the packet was mapped
     * by the port mapping.
     *
     * @param eaddr  Unused.
     * @param vid    Unused.
     * @return  A {@link VNodeHop} instance.
     */
    @Override
    public VNodeHop getIngressHop(EtherAddress eaddr, int vid) {
        return new VNodeHop(getIdentifier(), VirtualRouteReason.PORTMAPPED);
    }

    /**
     * Install a flow entry that drops every incoming packet.
     *
     * @param pctx  A runtime context for a received packet.
     */
    @Override
    public void disableInput(PacketContext pctx) {
        VTNPortMapConfig pmap = getPortMap();
        SalPort mapped = mappedPort;
        if (pmap != null && mapped != null) {
            VTNMatch vmatch = createMatch(pctx, pmap.getVlanId());
            if (vmatch != null) {
                String tname = getIdentifier().getTenantNameString();
                VTNFlowBuilder builder = pctx.createFlowBuilder(tname, vmatch).
                    addVirtualRoute(getIngressHop(null, 0)).
                    setEgressVNodeHop(null).
                    addDropFlow(mapped);
                pctx.addFlow(builder);
            }
        }
    }

    /**
     * Evaluate flow filters for incoming packet received from this virtual
     * interface.
     *
     * @param pctx  A runtime context for a received packet.
     * @param vid   A VLAN ID to be used for packet matching.
     *              A VLAN ID configured in the given packet is used if a
     *              negative value is specified.
     * @throws DropFlowException
     *    The given packet was discarded by a flow filter.
     * @throws RedirectFlowException
     *    The given packet was redirected by a flow filter.
     */
    @Override
    public void filterPacket(PacketContext pctx, int vid)
        throws DropFlowException, RedirectFlowException {
        // Evaluate vInterface input flow filters.
        // This should never clone the packet context.
        pctx.evaluate(getInputFilter(pctx.getTxContext()), vid);
    }

    /**
     * Evaluate flow filters for outgoing packet to be transmitted by this
     * virtual interface.
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
        // Evaluate vInterface output flow filters.
        // This should never clone the packet context.
        return pctx.evaluate(getOutputFilter(pctx.getTxContext()), vid);
    }
}
