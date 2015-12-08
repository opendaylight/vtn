/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.vnode;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.TxContext;
import org.opendaylight.vtn.manager.internal.inventory.VtnNodeEvent;
import org.opendaylight.vtn.manager.internal.inventory.VtnPortEvent;
import org.opendaylight.vtn.manager.internal.util.flow.filter.RedirectFlowException;
import org.opendaylight.vtn.manager.internal.util.flow.match.FlowMatchType;
import org.opendaylight.vtn.manager.internal.util.log.VTNLogLevel;
import org.opendaylight.vtn.manager.internal.util.vnode.BridgeIdentifier;
import org.opendaylight.vtn.manager.internal.util.vnode.VInterfaceIdentifier;
import org.opendaylight.vtn.manager.internal.util.vnode.VTNPortMapConfig;
import org.opendaylight.vtn.manager.internal.vnode.xml.XmlLogger;
import org.opendaylight.vtn.manager.internal.vnode.xml.XmlVTerminal;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeState;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.VtnVinterfaceConfig;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.vtn.mappable.vinterface.list.Vinterface;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vterminal.rev150907.vtn.vterminal.list.Vterminal;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vterminal.rev150907.vtn.vterminal.list.VterminalBuilder;

/**
 * {@code VTerminal} describes a configuration and runtime status for a
 * vTerminal.
 */
public final class VTerminal extends VirtualBridge<Vterminal> {
    /**
     * A logger instance.
     */
    private static final Logger  LOG =
        LoggerFactory.getLogger(VTerminal.class);

    /**
     * Construct a new instance.
     *
     * <p>
     *   This constructor is used to resume runtime status of the virtual
     *   bridge on bootstrap.
     * </p>
     *
     * @param vtmId  The identifier for the vTerminal.
     */
    public VTerminal(BridgeIdentifier<Vterminal> vtmId) {
        super(vtmId);
    }

    /**
     * Construct a new instance.
     *
     * @param vtmId  The identifier for the vTerminal.
     * @param vterm  A {@link Vterminal} instance read from the MD-SAL
     *               datastore.
     */
    public VTerminal(BridgeIdentifier<Vterminal> vtmId, Vterminal vterm) {
        super(vtmId, vterm);
    }

    /**
     * Resume this vTerminal.
     *
     * @param ctx      MD-SAL datastore transaction context.
     * @param xlogger  A {@link XmlLogger} instance.
     * @param xvtm     A {@link XmlVTerminal} instance that contains vTerminal
     *                 configuration.
     * @return  A {@link Vterminal} instance that contains vTerminal
     *          configuration and runtime status.
     * @throws VTNException  An error occurred.
     */
    public Vterminal resume(TxContext ctx, XmlLogger xlogger,
                            XmlVTerminal xvtm) throws VTNException {
        // Initialize vTerminal configuration.
        VterminalBuilder builder = xvtm.toVterminalBuilder();

        // Resume virtual interfaces.
        builder.setVinterface(resume(ctx, xlogger, xvtm.getInterfaces())).
            setBridgeStatus(getBridgeStatus());

        xlogger.log(VTNLogLevel.INFO,
                    "{}: vTerminal has been loaded: state={}",
                    getIdentifier(), getState());

        return builder.build();
    }

    // VirtualBridge

    /**
     * Construct a new virtual interface instance associated with the interface
     * inside this vTerminal.
     *
     * @param ifId  The identifier for a new virtual interface.
     * @param vif   A {@link Vinterface} instance read from the MD-SAL
     *              datastore.
     * @return  A {@link VTerminalInterface} instance.
     */
    @Override
    public VTerminalInterface newInterface(
        VInterfaceIdentifier<Vterminal> ifId, Vinterface vif) {
        return new VTerminalInterface(ifId, vif);
    }

    /**
     * Construct a new virtual interface instance associated with the interface
     * inside this vTerminal.
     *
     * <p>
     *   This method is used to resume runtime status of the virtual interface
     *   on bootstrap.
     * </p>
     *
     * @param ifId  The identifier for a new virtual interface.
     * @param vifc  Configuration of the virtual interface.
     * @param pmap  Port mapping configuration.
     * @return  A {@link VTerminalInterface} instance.
     */
    @Override
    public VTerminalInterface newInterface(
        VInterfaceIdentifier<Vterminal> ifId, VtnVinterfaceConfig vifc,
        VTNPortMapConfig pmap) {
        return new VTerminalInterface(ifId, vifc, pmap);
    }

    /**
     * Destroy the vTerminal.
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
    }

    /**
     * Return the state of the virtual mapping configured to this bridge.
     *
     * @param ctx  A runtime context for transaction task.
     * @return  Always {@link VnodeState#UNKNOWN} because vTerminal has no
     *          virtual mapping.
     */
    @Override
    protected VnodeState getMapState(TxContext ctx) {
        return VnodeState.UNKNOWN;
    }


    /**
     * Notify virtual mappings in this bridge of node event.
     *
     * @param ctx  MD-SAL datastore transaction context.
     * @param ev   A {@link VtnNodeEvent} instance.
     * @return  Always {@link VnodeState#UNKNOWN} because vTerminal has no
     *          virtual mapping.
     */
    @Override
    protected VnodeState eventReceived(TxContext ctx, VtnNodeEvent ev) {
        return VnodeState.UNKNOWN;
    }

    /**
     * Notify virtual mappings in this bridge of port event.
     *
     * @param ctx  MD-SAL datastore transaction context.
     * @param ev   A {@link VtnPortEvent} instance.
     * @return  Always {@link VnodeState#UNKNOWN} because vTerminal has no
     *          virtual mapping.
     */
    @Override
    protected VnodeState eventReceived(TxContext ctx, VtnPortEvent ev) {
        return VnodeState.UNKNOWN;
    }

    /**
     * This method does nothing because vTerminal does not have flow filter.
     *
     * @param pctx  A runtime context for a received packet.
     * @param vid   Unused.
     * @return  {@code pctx} is always returned.
     */
    @Override
    protected PacketContext evaluateOutputFilter(PacketContext pctx, int vid) {
        return pctx;
    }

    /**
     * Discard the specified packet because the vTerminal does not forward
     * any packet.
     *
     * @param pctx   A runtime context for a received packet.
     * @param vnode  A {@link VirtualMapNode} instance that maps the given
     *               packet.
     * @throws DropFlowException
     *    The given packet was discarded by a flow filter.
     * @throws RedirectFlowException
     *    The given packet was redirected by a flow filter.
     * @throws VTNException
     *    An error occurred.
     */
    @Override
    protected void forward(PacketContext pctx, VirtualMapNode vnode) {
        RedirectFlowException rex = pctx.getFirstRedirection();
        TxContext ctx = pctx.getTxContext();
        if (rex == null) {
            if (pctx.isFiltered()) {
                ctx.log(LOG, VTNLogLevel.DEBUG,
                        "{}: Discard packet from vTerminal interface: {}",
                        vnode.getIdentifier(), pctx.getDescription());

                if (!pctx.isUnicast()) {
                    // In this case we should specify multicast address in a
                    // drop flow entry, or it may discard packets to be
                    // filtered by flow filter.
                    pctx.addMatchField(FlowMatchType.DL_TYPE);
                    pctx.addMatchField(FlowMatchType.DL_DST);
                }

                pctx.installDropFlow();
            } else {
                if (LOG.isDebugEnabled()) {
                    ctx.log(LOG, VTNLogLevel.DEBUG,
                            "{}: Disable input from vTerminal interface.",
                            vnode.getIdentifier());
                }
                vnode.disableInput(pctx);
            }
        } else {
            ctx.log(LOG, VTNLogLevel.WARN,
                    "{}: Packet was redirected to {}: {}",
                    rex.getFilterPath(), getIdentifier(),
                    pctx.getDescription());
            pctx.installDropFlow();
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
