/*
 * Copyright (c) 2014, 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.cluster;

import org.slf4j.Logger;

import org.opendaylight.vtn.manager.VInterfacePath;
import org.opendaylight.vtn.manager.VNodePath;
import org.opendaylight.vtn.manager.VNodeRoute;
import org.opendaylight.vtn.manager.VTenantPath;

import org.opendaylight.vtn.manager.internal.LogProvider;
import org.opendaylight.vtn.manager.internal.PacketContext;
import org.opendaylight.vtn.manager.internal.VTNManagerImpl;

import org.opendaylight.controller.sal.core.NodeConnector;

/**
 * An exception which indicates a packet was redirected to the virtual
 * interface in the same VTN.
 *
 * <p>
 *   Although this class is public to other packages, this class does not
 *   provide any API. Applications other than VTN Manager must not use this
 *   class.
 * </p>
 */
public final class RedirectFlowException extends Exception
    implements LogProvider {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = 1L;

    /**
     * A {@link Logger} instance to record logs of the packet redirection.
     */
    private final Logger  logger;

    /**
     * A prefix string for a log record.
     */
    private final String  prefix;

    /**
     * The name of the flow condition.
     */
    private final String  condition;

    /**
     * The location of the virtual node that contains the REDIRECT flow filter.
     */
    private final VTenantPath  filterPath;

    /**
     * The location of the destination virtual interface.
     */
    private final VInterfacePath  destination;

    /**
     * Determine the direction of the packet redirection.
     * {@code true} means the packet is redirected as outgoing packet.
     */
    private final boolean  output;

    /**
     * Construct a new instance.
     *
     * @param ffmap  A {@link FlowFilterMap} instance that contains the
     *               REDIRECT flow filter.
     * @param rff    A {@link RedirectFlowFilterImpl} instance.
     */
    RedirectFlowException(FlowFilterMap ffmap, RedirectFlowFilterImpl rff) {
        super();

        logger = rff.getLogger();
        condition = rff.getFlowConditionName();
        output = rff.isOutput();
        prefix = ffmap.getLogPrefix(rff.getIndex());

        FlowFilterNode fnode = ffmap.getParent();
        filterPath = fnode.getPath();

        String tname = filterPath.getTenantName();
        destination = rff.getDestination().replaceTenantName(tname);
    }

    /**
     * Return the location of the virtual interface where the packet should
     * be redirected.
     *
     * <p>
     *   Note that the VTN name is not configured in the returned
     *   {@link VInterfacePath} instance.
     * </p>
     *
     * @return  The location of the destination virtual interface.
     */
    public VInterfacePath getDestination() {
        return destination;
    }

    /**
     * Determine whether the direction of packet redirection.
     *
     * @return  {@code true} is returned if the redirected packet should be
     *          treated as outgoing packet.
     *          {@code false} is returned if the redirected packet should be
     *          treated as incoming packet.
     */
    public boolean isOutput() {
        return output;
    }

    /**
     * Called when the destination virtual interface of the packet redirection
     * was not found.
     *
     * @param pctx  A packet context which contains the packet.
     * @param msg   An error message.
     */
    public void destinationNotFound(PacketContext pctx, String msg) {
        logger.error("{}: Discard packet: {}: to={}, cond={}, packet={}",
                     prefix, msg, destination, condition,
                     pctx.getDescription());
        pctx.installDropFlow();
    }

    /**
     * Called when the destination virtual interface of the packet redirection
     * was disabled.
     *
     * @param pctx  A packet context which contains the packet.
     */
    public void destinationDisabled(PacketContext pctx) {
        logger.error("{}: Discard packet: Destination is disabled: " +
                     "to={}, cond={}, packet={}", prefix, destination,
                     condition, pctx.getDescription());
    }

    /**
     * Called when the number of packet redirections exceeded the limit.
     *
     * @param pctx  A packet context which contains the packet.
     * @param hops  The number of virtual node hops.
     */
    public void tooManyHops(PacketContext pctx, int hops) {
        logger.error("{}: Discard packet: Too many hops: to={}, cond={}, " +
                     "hops={}, packet={}", prefix, destination, condition,
                     hops, pctx.getDescription());
        pctx.installDropFlow();
    }

    /**
     * Called when the physical switch port is not mapped to the destination
     * virtual interface.
     *
     * @param pctx  A packet context which contains the packet.
     */
    public void notMapped(PacketContext pctx) {
        logger.error("{}: Discard packet: Switch port is not mapped: " +
                     "to={}, cond={}, packet={}", prefix, destination,
                     condition, pctx.getDescription());
        pctx.installDropFlow();
    }

    /**
     * Called when the final destination of the unicast packet redirection is
     * determined.
     *
     * @param mgr   VTN Manager service.
     * @param pctx  A packet context which contains the packet.
     * @param port  A {@link NodeConnector} corresponding to the outgoing
     *              physical switch port.
     * @param vlan  A VLAN ID to be set to the outgoing packet.
     */
    public void forwarded(VTNManagerImpl mgr, PacketContext pctx,
                          NodeConnector port, short vlan) {
        if (logger.isDebugEnabled()) {
            VNodePath to;
            VNodeRoute vroute = pctx.getEgressVNodeRoute();
            if (vroute == null) {
                // This should never happen.
                to = null;
            } else {
                to = vroute.getPath();
            }

            logger.debug("{}: Packet was redirected to {}: port={}, " +
                         "vlan={}, packet={}", prefix, to, port, vlan,
                         pctx.getDescription());
        }
    }

    /**
     * Called when the redirected unicast packet was broadcasted to the
     * bridge.
     *
     * @param mgr   VTN Manager service.
     * @param pctx  A packet context which contains the packet.
     * @param path  Path to the bridge.
     */
    public void flooded(VTNManagerImpl mgr, PacketContext pctx,
                        VNodePath path) {
        if (logger.isDebugEnabled()) {
            logger.debug("{}: Packet was broadcasted into {}: packet={}",
                         prefix, path, pctx.getDescription());
        }
    }

    // LogProvider

    /**
     * Return a logger instance.
     *
     * @return  A {@link Logger} instance.
     */
    @Override
    public Logger getLogger() {
        return logger;
    }

    /**
     * Return a prefix string for a log record.
     *
     * @return  A prefix for a log record.
     */
    @Override
    public String getLogPrefix() {
        return prefix;
    }
}
