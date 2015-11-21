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

import org.opendaylight.vtn.manager.internal.util.flow.filter.DropFlowException;
import org.opendaylight.vtn.manager.internal.util.flow.filter.FlowFilterList;
import org.opendaylight.vtn.manager.internal.util.flow.filter.RedirectFlowException;
import org.opendaylight.vtn.manager.internal.util.log.VTNLogLevel;
import org.opendaylight.vtn.manager.internal.util.vnode.TenantNodeIdentifier;
import org.opendaylight.vtn.manager.internal.util.vnode.VInterfaceIdentifier;
import org.opendaylight.vtn.manager.internal.util.vnode.VTenantIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.vtn.info.VtenantConfig;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.vtns.Vtn;

/**
 * {@code VTenant} describes configuration for the VTN.
 *
 * <p>
 *   Note that this class does not manage virtual nodes inside the VTN.
 * </p>
 */
public final class VTenant extends VirtualNode<Vtn, VTenantIdentifier> {
    /**
     * A logger instance.
     */
    private static final Logger  LOG = LoggerFactory.getLogger(VTenant.class);

    /**
     * A list of VTN input filters.
     */
    private final FlowFilterList  inputFilter;

    /**
     * Construct a new instance.
     *
     * @param pctx   A runtime context for a received packet.
     * @param vtnId  The identifier of the VTN.
     * @param vtn    A {@link Vtn} instance read from the MD-SAL datastore.
     */
    public VTenant(PacketContext pctx, VTenantIdentifier vtnId, Vtn vtn) {
        super(vtnId, vtn);

        // Initialize flow timeout.
        VtenantConfig cfg = vtn.getVtenantConfig();
        pctx.setFlowTimeout(cfg.getIdleTimeout(), cfg.getHardTimeout());

        // Initialize VTN input filter.
        inputFilter = getFlowFilterList(
            pctx.getTxContext(), false, vtn.getVtnInputFilter());
    }

    /**
     * Receive a packet mapped to the VTN.
     *
     * @param pctx  A runtime context for a received packet.
     * @param ref   A {@link TenantNodeIdentifier} instance that specifies
     *              the virtual mapping that maps the packet.
     * @throws VTNException  An error occurred.
     */
    public void receive(PacketContext pctx, TenantNodeIdentifier<?, ?> ref)
        throws VTNException {
        // Initialize the received packet information.
        pctx.initPacketIn(ref);

        try {
            // Evaluate VTN input flow filters.
            // This should never clone the packet context.
            pctx.evaluate(inputFilter, FlowFilterList.VLAN_UNSPEC);

            // Determine the target bridge.
            VNodeReader reader = pctx.getVNodeReader();
            VirtualBridge<?> bridge = reader.getBridge(ref);
            if (bridge == null) {
                // This should never happen.
                pctx.getTxContext().
                    log(LOG, VTNLogLevel.ERROR, "Bridge not found: {}", ref);
            } else {
                bridge.receive(pctx, ref);
            }
        } catch (DropFlowException e) {
            // The given packet was discarded by a flow filter.
        } catch (RedirectFlowException e) {
            // The given packet was redirected by a flow filter.
            redirect(pctx, e);
        }
    }

    /**
     * Handle packet redirection caused by a REDIRECT flow filter.
     *
     * @param pctx  A runtime context for a received packet.
     * @param rex   An exception that keeps information about the packet
     *              redirection.
     * @throws VTNException  An error occurred.
     */
    private void redirect(PacketContext pctx, RedirectFlowException rex)
        throws VTNException {
        RedirectFlowException current = rex;
        while (true) {
            try {
                redirectImpl(pctx, current);
                break;
            } catch (DropFlowException e) {
                // The given packet was discarded by a flow filter.
                RedirectFlowException first = pctx.getFirstRedirection();
                pctx.getTxContext().
                    log(LOG, VTNLogLevel.WARN,
                        "{}: Packet was discarded: packet={}",
                        first.getFilterPath(), pctx.getDescription());
                break;
            } catch (RedirectFlowException e) {
                current = e;
            }
        }
    }

    /**
     * Handle packet redirection caused by a REDIRECT flow filter.
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
    private void redirectImpl(PacketContext pctx, RedirectFlowException rex)
        throws DropFlowException, RedirectFlowException, VTNException {
        // Determine the destination of the redirection.
        VInterfaceIdentifier<?> dst = rex.getDestination();
        VNodeReader reader = pctx.getVNodeReader();
        VirtualBridge<?> bridge = reader.getBridge(dst);
        if (bridge == null) {
            pctx.destinationNotFound(rex, "Destination bridge not found");
            throw new DropFlowException();
        }

        bridge.redirect(pctx, rex);
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
