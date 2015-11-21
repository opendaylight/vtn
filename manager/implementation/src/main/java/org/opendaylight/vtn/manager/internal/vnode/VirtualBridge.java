/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.vnode;

import static org.opendaylight.vtn.manager.internal.util.flow.filter.FlowFilterList.VLAN_UNSPEC;
import static org.opendaylight.vtn.manager.internal.vnode.MappingRegistry.getMapping;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

import org.slf4j.Logger;

import com.google.common.base.Optional;

import org.opendaylight.vtn.manager.VTNException;
import org.opendaylight.vtn.manager.packet.Ethernet;
import org.opendaylight.vtn.manager.util.EtherAddress;

import org.opendaylight.vtn.manager.internal.RouteResolver;
import org.opendaylight.vtn.manager.internal.TxContext;
import org.opendaylight.vtn.manager.internal.inventory.VtnNodeEvent;
import org.opendaylight.vtn.manager.internal.inventory.VtnPortEvent;
import org.opendaylight.vtn.manager.internal.util.CacheMap;
import org.opendaylight.vtn.manager.internal.util.DataStoreUtils;
import org.opendaylight.vtn.manager.internal.util.MiscUtils;
import org.opendaylight.vtn.manager.internal.util.flow.VNodeHop;
import org.opendaylight.vtn.manager.internal.util.flow.filter.DropFlowException;
import org.opendaylight.vtn.manager.internal.util.flow.filter.RedirectFlowException;
import org.opendaylight.vtn.manager.internal.util.inventory.InventoryReader;
import org.opendaylight.vtn.manager.internal.util.inventory.LinkEdge;
import org.opendaylight.vtn.manager.internal.util.inventory.PortVlan;
import org.opendaylight.vtn.manager.internal.util.inventory.SalNode;
import org.opendaylight.vtn.manager.internal.util.inventory.SalPort;
import org.opendaylight.vtn.manager.internal.util.log.VTNLogLevel;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;
import org.opendaylight.vtn.manager.internal.util.vnode.BridgeIdentifier;
import org.opendaylight.vtn.manager.internal.util.vnode.TenantNodeIdentifier;
import org.opendaylight.vtn.manager.internal.util.vnode.VBridgeIdentifier;
import org.opendaylight.vtn.manager.internal.util.vnode.VInterfaceIdentifier;
import org.opendaylight.vtn.manager.internal.util.vnode.VNodeIdentifier;
import org.opendaylight.vtn.manager.internal.util.vnode.VNodeType;
import org.opendaylight.vtn.manager.internal.util.vnode.VTNPortMapConfig;
import org.opendaylight.vtn.manager.internal.util.vnode.VTerminalIdentifier;
import org.opendaylight.vtn.manager.internal.vnode.xml.XmlLogger;
import org.opendaylight.vtn.manager.internal.vnode.xml.XmlVInterface;

import org.opendaylight.controller.md.sal.binding.api.ReadTransaction;
import org.opendaylight.controller.md.sal.binding.api.ReadWriteTransaction;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeName;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeState;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.VtnPortMappableBridge;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.bridge.status.FaultedPaths;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.bridge.status.FaultedPathsBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.port.mappable.bridge.BridgeStatus;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.port.mappable.bridge.BridgeStatusBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.vbridge.list.Vbridge;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.VtnVinterfaceConfig;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.vtn.mappable.vinterface.list.Vinterface;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.vtn.mappable.vinterface.list.VinterfaceBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vterminal.rev150907.vtn.vterminal.list.Vterminal;

import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.NodeId;

/**
 * {@code VInterface} describes a configuration and runtime status for a
 * virtual bridge inside VTN.
 *
 * @param <T>  The type of the target data model.
 */
public abstract class VirtualBridge<T extends VtnPortMappableBridge>
    extends VirtualNode<T, BridgeIdentifier<T>> {
    /**
     * The current state of the virtual bridge.
     */
    private VnodeState  state;

    /**
     * A list of faulted paths in the initial data.
     */
    private List<FaultedPaths>  faultedPaths;

    /**
     * A set of faulted paths to be added.
     */
    private Set<FaultedPaths>  newFaultedPaths;

    /**
     * Cache for the virtual interfaces inside this bridge.
     */
    private InterfaceCache  vInterfaces;

    /**
     * Set true if the bridge status needs to be updated.
     */
    private boolean  forceUpdate;

    /**
     * A map that keeps virtual interfaces inside this virtual bridge.
     */
    private final class InterfaceCache
        extends CacheMap<Vinterface, String, VInterface<T>> {
        /**
         * Construct a new instance.
         *
         * @param list  A list of {@link Vinterface} instances read from the
         *              MD-SAL datastore.
         */
        private InterfaceCache(List<Vinterface> list) {
            super(list);
        }

        // CacheMap

        /**
         * Return a key for the given virtual interface.
         *
         * @param vintf  A {@link Vinterface} instance.
         * @return  The name of the given virtual interface.
         */
        @Override
        protected String getKey(Vinterface vintf) {
            return vintf.getName().getValue();
        }

        /**
         * Create a new virtual interface instance.
         *
         * @param data  A {@link Vinterface} instance read from the MD-SAL
         *              datastore.
         * @return  A virtual interface instance.
         */
        @Override
        protected VInterface<T> newCache(Vinterface data) {
            VInterfaceIdentifier<T> ifId = getIdentifier().
                childInterface(data.getName());
            return newInterface(ifId, data);
        }
    }

    /**
     * Update the state of this virtual bridge using the child node state.
     *
     * @param bstate  The current state of the virtual bridge.
     * @param cstate  The state of the child node inside this virtual bridge.
     * @return  A new state for the virtual bridge.
     */
    public static final VnodeState updateState(VnodeState bstate,
                                               VnodeState cstate) {
        VnodeState newState;
        if (bstate != VnodeState.DOWN && cstate != VnodeState.UNKNOWN) {
            newState = cstate;
        } else {
            newState = bstate;
        }

        return newState;
    }

    /**
     * Create a default virtual bridge status.
     *
     * @return  A {@link BridgeStatus} instance.
     */
    public static final BridgeStatus newBridgeStatus() {
        return new BridgeStatusBuilder().
            setState(VnodeState.UNKNOWN).
            setPathFaults(0).
            build();
    }

    /**
     * Create a new virtual bridge status.
     *
     * @param st      The state of the virtual bridge.
     * @param faults  A list of faulted paths.
     * @return  A {@link BridgeStatus} instance.
     */
    public static final BridgeStatus newBridgeStatus(
        VnodeState st, List<FaultedPaths> faults) {
        int nfaults = (faults == null) ? 0 : faults.size();
        BridgeStatusBuilder builder = new BridgeStatusBuilder().
            setState(st).setPathFaults(nfaults);
        if (nfaults > 0) {
            builder.setFaultedPaths(faults);
        }

        return builder.build();
    }

    /**
     * Construct a new virtual bridge instance.
     *
     * @param rtx    A read transaction for the MD-SAL datastore.
     * @param ident  The virtual bridge identifier.
     * @param <B>    The type of the virtual bridge.
     * @return  A {@link VirtualBridge} instance.
     * @throws VTNException  An error occurred.
     */
    public static final <B extends VtnPortMappableBridge> VirtualBridge<B> create(
        ReadTransaction rtx, BridgeIdentifier<B> ident) throws VTNException {
        VNodeType type = ident.getType();
        if (type == VNodeType.VBRIDGE) {
            @SuppressWarnings("unchecked")
            VBridgeIdentifier vbrId = (VBridgeIdentifier)ident;
            Vbridge vbridge = vbrId.fetch(rtx);
            @SuppressWarnings("unchecked")
            VirtualBridge<B> bridge =
                (VirtualBridge<B>)new VBridge(vbrId, vbridge);
            return bridge;
        }
        if (type == VNodeType.VTERMINAL) {
            @SuppressWarnings("unchecked")
            VTerminalIdentifier vtermId = (VTerminalIdentifier)ident;
            Vterminal vterminal = vtermId.fetch(rtx);
            @SuppressWarnings("unchecked")
            VirtualBridge<B> bridge =
                (VirtualBridge<B>)new VTerminal(vtermId, vterminal);
            return bridge;
        }

        // This should never happen.
        throw new IllegalArgumentException("Unexpected bridge identifier: " +
                                           ident);
    }

    /**
     * Construct a new instance.
     *
     * <p>
     *   This constructor is used to resume runtime status of the virtual
     *   bridge on bootstrap.
     * </p>
     *
     * @param brId  The identifier for the virtual bridge.
     */
    protected VirtualBridge(BridgeIdentifier<T> brId) {
        super(brId);

        // Initialize node state.
        state = VnodeState.UNKNOWN;
    }

    /**
     * Construct a new instance.
     *
     * @param brId   The identifier for the virtual bridge.
     * @param value  A data object read from the MD-SAL datastore.
     */
    protected VirtualBridge(BridgeIdentifier<T> brId, T value) {
        super(brId, value);

        BridgeStatus bst = value.getBridgeStatus();
        state = bst.getState();
        vInterfaces = new InterfaceCache(value.getVinterface());
        faultedPaths = bst.getFaultedPaths();
    }

    /**
     * Return the current state of this virtual bridge.
     *
     * @return  The current state of this virtual bridge.
     */
    public final VnodeState getState() {
        return state;
    }

    /**
     * Determine whether at least one path fault is detected in this vBridge
     * or not.
     *
     * @return  {@code true} only if at least one path fault is detected.
     */
    public final boolean hasPathFault() {
        return (newFaultedPaths != null || !MiscUtils.isEmpty(faultedPaths));
    }

    /**
     * Return the runtime status information of the virtual bridge.
     *
     * @return  A {@link BridgeStatus} instance.
     */
    public final BridgeStatus getBridgeStatus() {
        List<FaultedPaths> faults = faultedPaths;
        if (newFaultedPaths != null) {
            // Append added faulted paths.
            List<FaultedPaths> newFaults = new ArrayList<>(newFaultedPaths);
            if (faults != null) {
                newFaults.addAll(faults);
            }
            faults = newFaults;
        }

        return newBridgeStatus(state, faults);
    }

    /**
     * Scan virtual mappings configured in this virtual bridge, and update the
     * bridge status in this instance.
     *
     * @param ctx  MD-SAL datastore transaction context.
     */
    public final void updateState(TxContext ctx) {
        state = checkState(ctx, hasPathFault());
    }

    /**
     * Update the state of this virtual bridge using the child node state.
     *
     * @param child   The child node in this virtual bridge.
     * @param cstate  The state of the child node inside this virtual bridge.
     */
    public final void updateState(VirtualElement<?, ?> child,
                                  VnodeState cstate) {
        state = updateState(state, cstate);
        traceState(child, cstate, state);
    }

    /**
     * Scan virtual mappings configured in this virtual bridge, and update the
     * bridge status in the MD-SAL datastore.
     *
     * @param ctx  MD-SAL datastore transaction context.
     */
    public final void putState(TxContext ctx) {
        updateState(ctx);
        submit(ctx);
    }

    /**
     * Scan virtual mappings configured in this virtual bridge, and update the
     * bridge status in the MD-SAL datastore.
     *
     * @param ctx  MD-SAL datastore transaction context.
     * @param vif  A {@link VInterface} instance that was changed its status.
     *             This is used as a hint for determining the virtual bridge
     *             status.
     */
    public final void putState(TxContext ctx, VInterface vif) {
        if (vif.isEnabled()) {
            putState(ctx, vif.getState());
        } else {
            putState(ctx);
        }
        submit(ctx);
    }

    /**
     * Scan virtual mappings configured in this virtual bridge, and update the
     * bridge status in the MD-SAL datastore.
     *
     * @param ctx     MD-SAL datastore transaction context.
     * @param cstate  The state of the child node inside this virtual bridge.
     *                This is used as a hint for determining the virtual bridge
     *                status.
     */
    public final void putState(TxContext ctx, VnodeState cstate) {
        if (cstate == VnodeState.DOWN) {
            state = cstate;
        } else {
            updateState(ctx);
        }
        submit(ctx);
    }

    /**
     * Update the runtime status information in the MD-SAL datastore.
     *
     * @param ctx  MD-SAL datastore transaction context.
     */
    public final void submit(TxContext ctx) {
        if (isDirty()) {
            ReadWriteTransaction tx = ctx.getReadWriteTransaction();
            InstanceIdentifier<BridgeStatus> path =
                getIdentifier().getStatusPath();
            LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
            tx.put(oper, path, getBridgeStatus(), false);
        }
    }

    /**
     * Set the enabled state to the specified virtual interface.
     *
     * @param ctx    MD-SAL datastore transaction context.
     * @param ifId   The identifier for the target virtual interface.
     *               The specified interface must be present in this virtual
     *               bridge.
     * @param value  A new value of the enabled state.
     * @throws VTNException  An error occurred.
     */
    public final void setInterfaceEnabled(
        TxContext ctx, VInterfaceIdentifier<T> ifId, boolean value)
        throws VTNException {
        // This should never return null.
        VInterface<T> vif = getInterface(ctx, ifId);
        vif.setEnabled(ctx, value);

        // Update the status of this vBridge.
        putState(ctx, vif);
    }

    /**
     * Resume the runtime status information about virtual interfaces inside
     * this virtual bridge.
     *
     * @param ctx      MD-SAL datastore transaction context.
     * @param xlogger  A {@link XmlLogger} instance.
     * @param xiflist  A list of {@link XmlVInterface} which represents the
     *                 configuration of virtual interfaces inside this virtual
     *                 bridge.
     * @return  A list of {@link Vinterface} instances or {@code null}.
     * @throws VTNException
     *    Failed to resume the vBridge.
     */
    public final List<Vinterface> resume(TxContext ctx, XmlLogger xlogger,
                                         List<XmlVInterface> xiflist)
        throws VTNException {
        List<Vinterface> ifList;
        if (xiflist == null) {
            ifList = null;
        } else {
            ifList = new ArrayList<>();
            Set<VnodeName> nameSet = new HashSet<>();
            for (XmlVInterface xvif: xiflist) {
                VnodeName vname = xvif.getName();
                if (nameSet.add(vname)) {
                    try {
                        ifList.add(resume(ctx, xlogger, xvif));
                    } catch (RpcException | RuntimeException e) {
                        xlogger.log(VTNLogLevel.WARN, e,
                                    "%s: Ignore broken virtual interface: %s",
                                    getIdentifier(), MiscUtils.getValue(vname));
                    }
                } else {
                    xlogger.log(VTNLogLevel.WARN,
                                "{}: Ignore duplicate virtual interface: {}",
                                getIdentifier(), MiscUtils.getValue(vname));
                }
            }

            if (ifList.isEmpty()) {
                ifList = null;
            }
        }

        return ifList;
    }

    /**
     * Determine whether this virtual bridge contains no flow filter or not.
     *
     * @return  {@code true} if this virtual bridge contains no flow filter.
     *          {@code false} otherwise.
     */
    public boolean isFilterEmpty() {
        for (VInterface<T> vif: vInterfaces) {
            if (vif.isFilterEmpty()) {
                return false;
            }
        }

        return true;
    }

    /**
     * Receive a packet mapped to the virtual bridge.
     *
     * @param pctx  A runtime context for a received packet.
     * @param ref   A {@link TenantNodeIdentifier} instance that specifies
     *              the virtual mapping that maps the packet.
     *              Note that the specified virtual mapping must be present
     *              in this virtual bridge.
     * @throws DropFlowException
     *    The given packet was discarded by a flow filter.
     * @throws RedirectFlowException
     *    The given packet was redirected by a flow filter.
     * @throws VTNException  An error occurred.
     */
    public final void receive(PacketContext pctx,
                              TenantNodeIdentifier<?, ?> ref)
        throws VTNException, DropFlowException, RedirectFlowException {
        // Determine the virtual mapping that maps the given packet.
        SalPort ingress = pctx.getIngressPort();
        int vid = pctx.getVlanId();
        EtherAddress src = pctx.getSourceAddress();
        VirtualMapNode vnode = match(pctx, ref, src, ingress, vid, true);
        if (vnode == null) {
            // This should never happen.
            pctx.getTxContext().
                log(getLogger(), VTNLogLevel.ERROR,
                    "{}: Not mapped: ref={}, packet={}", getIdentifier(),
                    ref, pctx.getDescription());
        } else if (vnode.isEnabled()) {
            pctx.addVNodeHop(vnode.getIngressHop(src, vid));

            // Evaluate flow filters configured in the virtual mapping.
            // Actually this evaluates vInterface input flow filters against
            // the packet only if it is mapped by the port mapping.
            vnode.filterPacket(pctx, VLAN_UNSPEC);

            // Forward the given packet to this virtual bridge.
            forward(pctx, vnode);
        } else {
            Logger logger = getLogger();
            if (logger.isDebugEnabled()) {
                pctx.getTxContext().
                    log(logger, VTNLogLevel.DEBUG,
                        "{}: Ignore packet received from disabled network: {}",
                        vnode.getIdentifier(), pctx.getDescription());
            }
            vnode.disableInput(pctx);
        }
    }

    /**
     * Redirect the given packet to the specified virtual interface.
     *
     * @param pctx  A runtime context for a received packet.
     * @param rex   An exception that keeps information about the packet
     *              redirection.
     * @throws DropFlowException
     *    The given packet was discarded by a DROP flow filter.
     * @throws RedirectFlowException
     *    The given packet was redirected by a REDIRECT flow filter.
     * @throws VTNException  An error occurred.
     */
    public final void redirect(PacketContext pctx, RedirectFlowException rex)
        throws DropFlowException, RedirectFlowException, VTNException {
        @SuppressWarnings("unchecked")
        VInterfaceIdentifier<T> ifId =
            (VInterfaceIdentifier<T>)rex.getDestination();

        // Determine the destination of the redirection.
        TxContext ctx = pctx.getTxContext();
        VInterface<T> vif = getInterface(ctx, ifId);
        if (vif == null) {
            pctx.destinationNotFound(rex, "Destination interface not found");
        } else if (!vif.isEnabled()) {
            pctx.destinationDisabled(rex);
        } else if (pctx.redirect(ifId, rex)) {
            if (rex.isOutgoing()) {
                // Redirect as outgoing packet.
                vif.redirect(pctx, rex, this);
            } else {
                // Use the VLAN ID mapped to the virtual interface for
                // packet matching.
                int vid = vif.getVlanId();

                // Evaluate input flow filters configured in the
                // destination virtual interface.
                vif.filterPacket(pctx, vid);

                // Forward the packet to the bridge.
                forward(pctx, vif);
            }

            return;
        }

        throw new DropFlowException();
    }

    /**
     * Invoked when a physical switch is added, removed, or changed.
     *
     * @param ctx  MD-SAL datastore transaction context.
     * @param ev   A {@link VtnNodeEvent} instance.
     */
    public final void notifyNode(TxContext ctx, VtnNodeEvent ev) {
        if (ev.getUpdateType() == VtnUpdateType.REMOVED) {
            // Remove faulted paths that contain removed node.
            SalNode snode = ev.getSalNode();
            int removed = removeFaultedPath(snode);
            if (removed != 0) {
                ctx.log(getLogger(), VTNLogLevel.INFO,
                        "{}: Removed faulted path that contains removed node" +
                        ": node={}, removed={}", getIdentifier(), snode,
                        removed);
            }
        }

        // Deliver the event to all the virtual mappings in this bridge.
        VnodeState st = eventReceived(ctx, ev);

        // Deliver the event to all the virtual interfaces.
        for (VInterface<T> vif: vInterfaces) {
            VnodeState cst = vif.notifyNode(ctx, ev);
            st = updateState(st, cst);
            traceState(vif, cst, st);
        }

        if (hasPathFault()) {
            st = VnodeState.DOWN;
        }

        state = st;
        submit(ctx);
    }

    /**
     * Invoked when a physical switch port is added, removed, or changed.
     *
     * @param ctx  MD-SAL datastore transaction context.
     * @param ev   A {@link VtnPortEvent} instance.
     */
    public final void notifyPort(TxContext ctx, VtnPortEvent ev) {
        if (ev.getUpdateType() == VtnUpdateType.REMOVED) {
            // Remove path faults that may be caused by removed port.
            // Even if removed paths are still broken, they will be detected
            // by succeeding PACKET_IN.
            SalPort sport = ev.getSalPort();
            int removed = removeFaultedPath(sport.getSalNode());
            if (removed != 0) {
                ctx.log(getLogger(), VTNLogLevel.INFO,
                        "{}: Removed faulted path that contains removed port" +
                        ": port={}, removed={}", getIdentifier(), sport,
                        removed);
            }
        }

        // Deliver the event to all the virtual mappings in this bridge.
        VnodeState st = eventReceived(ctx, ev);

        // Deliver the event to all the virtual interfaces.
        for (VInterface<T> vif: vInterfaces) {
            VnodeState cst = vif.notifyPort(ctx, ev);
            st = VirtualBridge.updateState(st, cst);
            traceState(vif, cst, st);
        }

        if (hasPathFault()) {
            st = VnodeState.DOWN;
        }

        state = st;
        submit(ctx);
    }

    /**
     * Invoked when the packet routing table has been updated.
     *
     * <p>
     *   Note that the caller must guarantee that a set of faulted paths in
     *   this instance is not yet modified.
     * </p>
     *
     * @param ctx  MD-SAL datastore transaction context.
     */
    public final void routingUpdated(TxContext ctx) {
        assert newFaultedPaths == null;
        if (!MiscUtils.isEmpty(faultedPaths)) {
            // Remove resolved faulted path from a set of faulted paths.
            List<FaultedPaths> newList = new ArrayList<>();
            RouteResolver rr = ctx.getProvider().getRouteResolver();
            InventoryReader reader =
                ctx.getReadSpecific(InventoryReader.class);
            boolean removed = false;
            for (FaultedPaths fp: faultedPaths) {
                SalNode src = SalNode.create(fp.getSource());
                SalNode dst = SalNode.create(fp.getDestination());
                List<LinkEdge> edge = rr.getRoute(reader, src, dst);
                if (edge == null) {
                    newList.add(fp);
                } else {
                    removed = true;
                }
            }

            if (removed) {
                // Update the bridge status.
                VnodeState st = checkState(ctx, !newList.isEmpty());
                BridgeStatus bst = newBridgeStatus(st, newList);
                ReadWriteTransaction tx = ctx.getReadWriteTransaction();
                InstanceIdentifier<BridgeStatus> path = getIdentifier().
                    getStatusPath();
                LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
                tx.put(oper, path, bst, false);
            }
        }
    }

    /**
     * Destroy all the virtual interfaces in this virtual bridge.
     *
     * <p>
     *   This method is called only when this virtual bridge is being
     *   destroyed.
     * </p>
     *
     * @param ctx  MD-SAL datastore transaction context.
     * @throws VTNException  An error occurred.
     */
    protected final void destroyInterfaces(TxContext ctx) throws VTNException {
        for (VInterface<T> vif: vInterfaces) {
            vif.destroy(ctx, false);
        }
    }

    /**
     * Record a trace log for transition of the virtual bridge status.
     *
     * @param child   The child node in this virtual bridge.
     * @param cst     The state of the child node.
     * @param st      The current state of this virtual bridge.
     */
    protected final void traceState(
        VirtualElement<?, ?> child, VnodeState cst, VnodeState st) {
        Logger log = getLogger();
        if (log.isTraceEnabled()) {
            log.trace("{}: child={{}, {}}, state={}",
                      getIdentifier(), child.getIdentifier(), cst, st);
        }
    }

    /**
     * Return the virtual network node in this virtual bridge that maps the
     * specified host.
     *
     * @param pctx   A runtime context for a received packet.
     * @param eaddr  The MAC address of the host.
     * @param sport  A {@link SalPort} instance that specifies the switch port
     *               where the host was detected.
     * @param vid    The VLAN ID associated with the specified host.
     * @return  A {@code VirtualMapNode} if the specified VLAN is mapped to
     *          this virtual bridge. {@code null} otherwise.
     * @throws VTNException  An error occurred.
     */
    protected final VirtualMapNode match(
        PacketContext pctx, EtherAddress eaddr, SalPort sport, int vid)
        throws VTNException {
        TxContext ctx = pctx.getTxContext();
        BridgeIdentifier<T> ident = getIdentifier();
        TenantNodeIdentifier<?, ?> ref = getMapping(ctx, eaddr, sport, vid);
        return (ident.contains(ref))
            ? match(pctx, ref, eaddr, sport, vid, false)
            : null;
    }

    /**
     * Return the virtual network node in this bridge which maps the specified
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
    protected VirtualMapNode match(
        PacketContext pctx, TenantNodeIdentifier<?, ?> ref, EtherAddress eaddr,
        SalPort sport, int vid, boolean pktin) throws VTNException {
        TxContext ctx = pctx.getTxContext();
        VNodeType type = ref.getType();
        if (type.isInterface()) {
            // Determine the interface that maps the given host.
            @SuppressWarnings("unchecked")
            VInterfaceIdentifier<T> ifId = (VInterfaceIdentifier<T>)ref;
            VInterface<T> vif = getInterface(ctx, ifId);
            if (vif == null) {
                ctx.log(getLogger(), VTNLogLevel.WARN,
                        "{}: Failed to resolve port mapping reference: {}",
                        getIdentifier(), ref);
            } else if (!vif.match(sport, vid)) {
                // This interface does not map the specified VLAN.
                vif = null;
            }

            return vif;
        }

        // This should never happen.
        ctx.log(getLogger(), VTNLogLevel.ERROR,
                "{}: Unexpected mapping reference: {}", getIdentifier(),
                ref);
        return null;
    }

    /**
     * Forward the given unicast packet to the specified physical network, and
     * establish the data flow for the given packet.
     *
     * @param pctx    A runtime context for a received packet.
     * @param egress  A {@link SalPort} corresponding to the outgoing physical
     *                switch port.
     * @param outVid  A VLAN ID for the outgoing packet.
     * @throws VTNException  An error occurred.
     */
    protected final void forward(PacketContext pctx, SalPort egress,
                                 int outVid) throws VTNException {
        RedirectFlowException rex = pctx.getFirstRedirection();
        if (rex != null) {
            // Record the final destination of the packet redirection.
            VNodeIdentifier<?> to;
            VNodeHop vhop = pctx.getEgressVNodeHop();
            if (vhop == null) {
                // This should never happen.
                to = null;
            } else {
                to = vhop.getPath();
            }
            pctx.forwarded(rex, to, egress, outVid);
        }

        // Ensure that the destination host is reachable.
        SalPort ingress = pctx.getIngressPort();
        SalNode snode = ingress.getSalNode();
        SalNode dnode = egress.getSalNode();
        RouteResolver rr = pctx.getRouteResolver();
        InventoryReader reader = pctx.getInventoryReader();
        TxContext ctx = pctx.getTxContext();
        List<LinkEdge> path = rr.getRoute(reader, snode, dnode);
        if (path == null) {
            addFaultedPath(ctx, snode, dnode);
        } else {
            Logger logger = getLogger();
            logger.trace("{}: Packet route resolved: {} -> {} -> {}",
                         rr.getPathPolicyId(), ingress, path, egress);

            // Forward the packet.
            Ethernet frame = pctx.createFrame(outVid);
            if (logger.isTraceEnabled()) {
                logger.trace("{}: Forward packet to known host: {}",
                             getIdentifier(),
                             pctx.getDescription(frame, egress, outVid));
            }
            pctx.transmit(egress, frame);

            // Install VTN flow.
            pctx.installFlow(egress, outVid, path);
        }
    }

    /**
     * Broadcast the given packet to all the virtual interfaces configured in
     * this virtual bridge.
     *
     * @param pctx  A runtime context for a received packet.
     * @param sent  A set of {@link PortVlan} which indicates the network
     *              already processed.
     * @throws RedirectFlowException
     *    The given packet was redirected by a flow filter.
     * @throws VTNException  An error occurred.
     */
    protected final void broadcastInterfaces(PacketContext pctx,
                                             Set<PortVlan> sent)
        throws RedirectFlowException, VTNException {
        for (VInterface<T> vif: vInterfaces) {
            vif.transmit(pctx, sent);
        }
    }

    /**
     * Destroy the virtual bridge.
     *
     * @param ctx     MD-SAL datastore transaction context.
     * @param retain  {@code true} means that the parent virtual node will be
     *                retained. {@code false} means that the parent virtual
     *                node is being destroyed.
     * @throws VTNException  An error occurred.
     */
    public abstract void destroy(TxContext ctx, boolean retain)
        throws VTNException;

    /**
     * Construct a new virtual interface instance associated with the interface
     * inside this virtual bridge.
     *
     * @param ifId  The identifier for a new virtual interface.
     * @param vif   A {@link Vinterface} instance read from the MD-SAL
     *              datastore.
     * @return  A {@link VInterface} instance.
     */
    public abstract VInterface<T> newInterface(
        VInterfaceIdentifier<T> ifId, Vinterface vif);

    /**
     * Construct a new virtual interface instance associated with the interface
     * inside this virtual bridge.
     *
     * <p>
     *   This method is used to resume runtime status of the virtual interface
     *   on bootstrap.
     * </p>
     *
     * @param ifId  The identifier for a new virtual interface.
     * @param vifc  Configuration of the virtual interface.
     * @param pmap  Port mapping configuration.
     * @return  A {@link VInterface} instance.
     */
    public abstract VInterface<T> newInterface(
        VInterfaceIdentifier<T> ifId, VtnVinterfaceConfig vifc,
        VTNPortMapConfig pmap);

    /**
     * Return the state of the virtual mapping configured to this bridge.
     *
     * @param ctx  MD-SAL datastore transaction context.
     * @return  The state of the virtual mapping configured to this bridge.
     */
    protected abstract VnodeState getMapState(TxContext ctx);

    /**
     * Notify virtual mappings in this bridge of node event.
     *
     * @param ctx  MD-SAL datastore transaction context.
     * @param ev   A {@link VtnNodeEvent} instance.
     * @return  The state of the virtual mapping configured to this bridge.
     */
    protected abstract VnodeState eventReceived(TxContext ctx,
                                                VtnNodeEvent ev);

    /**
     * Notify virtual mappings in this bridge of port event.
     *
     * @param ctx  MD-SAL datastore transaction context.
     * @param ev   A {@link VtnPortEvent} instance.
     * @return  The state of the virtual mapping configured to this bridge.
     */
    protected abstract VnodeState eventReceived(TxContext ctx,
                                                VtnPortEvent ev);

    /**
     * Evaluate flow filters configured in this bridge against the given
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
    protected abstract PacketContext evaluateOutputFilter(
        PacketContext pctx, int vid)
        throws DropFlowException, RedirectFlowException;

    /**
     * Forward the specified packet to the virtual mapping configured in this
     * virtual bridge.
     *
     * @param pctx   A runtime context for a received packet.
     * @param vnode  A {@link VirtualMapNode} instance that maps the given
     *               packet.
     * @throws DropFlowException
     *    The given packet was discarded by a flow filter.
     * @throws RedirectFlowException
     *    The given packet was redirected by a flow filter.
     * @throws VTNException  An error occurred.
     */
    protected abstract void forward(PacketContext pctx, VirtualMapNode vnode)
        throws DropFlowException, RedirectFlowException, VTNException;

    /**
     * Resume the runtime status information about the virtual interface
     * inside this virtual bridge.
     *
     * @param ctx      MD-SAL datastore transaction context.
     * @param xlogger  A {@link XmlLogger} instance.
     * @param xvif     A {@link XmlVInterface} instance that contains the
     *                 configuration for the virtual iterface.
     * @return  A {@link Vinterface} instance.
     * @throws VTNException
     *    Failed to resume the given virtual interface
     */
    private Vinterface resume(TxContext ctx, XmlLogger xlogger,
                              XmlVInterface xvif) throws VTNException {
        VInterfaceIdentifier<T> ifId = getIdentifier().
            childInterface(xvif.getName());
        VinterfaceBuilder builder = xvif.toVinterfaceBuilder(xlogger, ifId);
        VInterface<T> vif = newInterface(ifId, builder.getVinterfaceConfig(),
                                         xvif.getPortMap());
        vif.resume(ctx);

        builder.setVinterfaceStatus(vif.getVinterfaceStatus());
        boolean enabled = vif.isEnabled();
        ctx.log(getLogger(), VTNLogLevel.DEBUG,
                "{}: Virtual interface has been loaded: enabled={}, " +
                "state={}, port-state={}", ifId, enabled, vif.getState(),
                vif.getPortState());

        // Update the state of the parent bridge.
        if (enabled) {
            updateState(vif, vif.getState());
        }

        return builder.build();
    }

    /**
     * Scan virtual mappings configured in this virtual bridge, and determine
     * the state of this virtual bridge.
     *
     * @param ctx    MD-SAL datastore transaction context.
     * @param fault  {@code true} indicates that at least one path fault is
     *               detected on this virtual bridge.
     * @return  A new state of this virtual bridge.
     */
    private VnodeState checkState(TxContext ctx, boolean fault) {
        VnodeState st = getMapState(ctx);
        if (st != VnodeState.DOWN) {
            // Scan virtual interfaces.
            for (VInterface<T> vif: vInterfaces) {
                if (vif.isEnabled()) {
                    st = updateState(st, vif.getState());
                    if (st == VnodeState.DOWN) {
                        break;
                    }
                }
            }
        }

        if (fault) {
            if (st == VnodeState.UNKNOWN) {
                // No network element is mapped to this virtual bridge.
                // So we need to clear path faults detected on this bridge.
                faultedPaths = null;
                forceUpdate = true;
            } else {
                st = VnodeState.DOWN;
            }
        }

        return st;
    }

    /**
     * Determine whether the status of the virtual bridge is changed or not.
     *
     * @return  {@code true} only if the status of the virtual bridge is
     *          changed.
     */
    private boolean isDirty() {
        boolean dirty = (forceUpdate || newFaultedPaths != null);
        if (!dirty) {
            VnodeState initial = getInitialValue().
                getBridgeStatus().getState();
            dirty = (state != initial);
        }

        return dirty;
    }

    /**
     * Add a node path to the set of faulted node paths.
     *
     * @param ctx  MD-SAL datastore transaction context.
     * @param src  The source node.
     * @param dst  The destination node.
     * @throws VTNException  An error occurred.
     */
    private void addFaultedPath(TxContext ctx, SalNode src, SalNode dst)
        throws VTNException {
        // Change bridge state to DOWN.
        state = VnodeState.DOWN;

        FaultedPaths fpath = new FaultedPathsBuilder().
            setSource(src.getNodeId()).
            setDestination(dst.getNodeId()).
            build();

        // Determine whether the given path is recorded or not.
        InstanceIdentifier<FaultedPaths> path = getIdentifier().
            getStatusPath().
            child(FaultedPaths.class, fpath.getKey());
        ReadTransaction rtx = ctx.getTransaction();
        Optional<FaultedPaths> opt = DataStoreUtils.read(rtx, path);
        if (!opt.isPresent()) {
            // The given path needs to be put into the faulted-paths.
            Set<FaultedPaths> set = newFaultedPaths;
            if (set == null) {
                set = new HashSet<>();
                newFaultedPaths = set;
            }

            set.add(fpath);
        }
    }

    /**
     * Remove paths that contain the specified switch from the set of
     * faulted paths.
     *
     * <p>
     *   Note that the caller must guarantee that a set of faulted paths in
     *   this instance is not yet modified.
     * </p>
     *
     * @param snode  A {@link SalNode} instance that specifies the switch.
     * @return  The number of removed faulted paths.
     */
    private int removeFaultedPath(SalNode snode) {
        assert newFaultedPaths == null;
        int removed = 0;
        NodeId nodeId = snode.getNodeId();
        if (!MiscUtils.isEmpty(faultedPaths)) {
            List<FaultedPaths> newList = new ArrayList<>();
            for (FaultedPaths fp: faultedPaths) {
                if (nodeId.equals(fp.getSource()) ||
                    nodeId.equals(fp.getDestination())) {
                    removed++;
                } else {
                    newList.add(fp);
                }
            }

            if (removed != 0) {
                faultedPaths = newList;
                forceUpdate = true;
            }
        }

        return removed;
    }

    /**
     * Return the virtual interface specified by the given identifier.
     *
     * @param ctx   MD-SAL datastore transaction context.
     * @param ifId  The identifier for the virtual interface in this
     *              virtual bridge.
     * @return  A {@link VInterface} instance if found.
     *          {@code null} if not found.
     * @throws VTNException  An error occurred.
     */
    private VInterface<T> getInterface(TxContext ctx,
                                       VInterfaceIdentifier<T> ifId)
        throws VTNException {
        assert getIdentifier().contains(ifId);

        String iname = ifId.getInterfaceNameString();
        VInterface<T> vif = vInterfaces.getCached(iname);
        if (vif == null) {
            // Read the specified interface in the MD-SAL datastore.
            ReadTransaction rtx = ctx.getTransaction();
            Optional<Vinterface> opt = ifId.read(rtx);
            if (opt.isPresent()) {
                vif = newInterface(ifId, opt.get());
                vInterfaces.put(iname, vif);
            }
        }

        return vif;
    }
}
