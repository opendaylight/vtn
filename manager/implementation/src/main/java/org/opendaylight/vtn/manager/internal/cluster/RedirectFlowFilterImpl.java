/*
 * Copyright (c) 2014, 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.cluster;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import org.opendaylight.vtn.manager.ErrorVNodePath;
import org.opendaylight.vtn.manager.VInterfacePath;
import org.opendaylight.vtn.manager.VNodePath;
import org.opendaylight.vtn.manager.VTNException;
import org.opendaylight.vtn.manager.VTenantPath;
import org.opendaylight.vtn.manager.flow.filter.FlowFilter;
import org.opendaylight.vtn.manager.flow.filter.RedirectFilter;

import org.opendaylight.vtn.manager.internal.PacketContext;
import org.opendaylight.vtn.manager.internal.VTNManagerImpl;
import org.opendaylight.vtn.manager.internal.util.MiscUtils;

import org.opendaylight.controller.sal.utils.Status;
import org.opendaylight.controller.sal.utils.StatusCode;

/**
 * This class describes REDIRECT flow filter, which forwards packet to
 * another virtual interface.
 *
 * <p>
 *   Although this class is public to other packages, this class does not
 *   provide any API. Applications other than VTN Manager must not use this
 *   class.
 * </p>
 */
public final class RedirectFlowFilterImpl extends FlowFilterImpl {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = -2751399201024680813L;

    /**
     * Logger instance.
     */
    private static final Logger  LOG =
        LoggerFactory.getLogger(RedirectFlowFilterImpl.class);

    /**
     * The location of the destination virtual interface.
     *
     * <p>
     *   Note that the VTN name is not configured in this instance.
     * </p>
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
     * @param fnode   Virtual node that contains this flow filter.
     * @param idx     An index number to be assigned.
     * @param filter  A {@link FlowFilter} instance.
     * @throws VTNException
     *    {@code filter} contains invalid value.
     */
    protected RedirectFlowFilterImpl(FlowFilterNode fnode, int idx,
                                     FlowFilter filter)
        throws VTNException {
        super(idx, filter);

        RedirectFilter redirect = (RedirectFilter)filter.getFilterType();
        output = redirect.isOutput();

        VInterfacePath path = redirect.getDestination();
        if (path == null) {
            Status st =
                MiscUtils.argumentIsNull("RedirectFilter: Destination");
            throw new VTNException(st);
        }
        if (path instanceof ErrorVNodePath) {
            ErrorVNodePath epath = (ErrorVNodePath)path;
            throw new VTNException(StatusCode.BADREQUEST, epath.getError());
        }

        // Set null as the VTN name because the VTN name is always determined
        // by the virtual node path.
        path = path.replaceTenantName(null);

        try {
            // Ensure that the node and interface name is valid.
            MiscUtils.checkName("Virtual node", path.getTenantNodeName());
            MiscUtils.checkName("Interface", path.getInterfaceName());
        } catch (VTNException e) {
            Status st = e.getStatus();
            StringBuilder builder =
                new StringBuilder("RedirectFilter: Invalid destination: ");
            builder.append(st.getDescription());
            Status newst = new Status(st.getCode(), builder.toString());
            throw new VTNException(newst, e);
        }

        // Reject self-redirection.
        VTenantPath ppath = fnode.getPath();
        String tenant = ppath.getTenantName();
        VNodePath dst = (VNodePath)path.replaceTenantName(tenant);
        if (ppath instanceof VInterfacePath && ppath.contains(dst)) {
            StringBuilder builder =
                new StringBuilder("RedirectFilter: Self-redirection: ");
            builder.append(dst);
            throw new VTNException(StatusCode.BADREQUEST, builder.toString());
        }

        destination = path;
    }

    /**
     * Return the location of the destination virtual interface.
     *
     * <p>
     *   Note that the VTN name is not configured in the returned location.
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
        if (!super.equals(o)) {
            return false;
        }

        RedirectFlowFilterImpl rfi = (RedirectFlowFilterImpl)o;
        return (destination.equals(rfi.destination) && output == rfi.output);
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        int h = super.hashCode() + (destination.hashCode() * 29);
        if (output) {
            h += 7;
        }

        return h;
    }

    // FlowFilterImpl

    /**
     * Append the contents of this instance to the given {@link StringBuilder}
     * instance.
     *
     * @param builder  A {@link StringBuilder} instance.
     */
    @Override
    protected void appendContents(StringBuilder builder) {
        super.appendContents(builder);
        builder.append(",destination=").append(destination).
            append(",direction=").append((output) ? "out" : "in");
    }

    /**
     * Determine whether this flow filter supports packet flooding or not.
     *
     * <p>
     *   This method returns {@code false} because REDIRECT filter does not
     *   support broadcast packets.
     * </p>
     *
     * @return  {@code false}.
     */
    @Override
    protected boolean isFloodingSuppoted() {
        return false;
    }

    /**
     * Apply this REDIRECT flow filter to the given packet.
     *
     * @param mgr    VTN Manager service.
     * @param pctx   A packet context which contains the packet.
     * @param ffmap  A {@link FlowFilterMap} instance that contains this
     *               flow filter.
     * @throws RedirectFlowException  Always thrown.
     */
    @Override
    protected void apply(VTNManagerImpl mgr, PacketContext pctx,
                         FlowFilterMap ffmap) throws RedirectFlowException {
        RedirectFlowException e = new RedirectFlowException(ffmap, this);
        String format =
            "{}: Redirect packet: cond={}, to={}, direction={}, packet={}";
        String cond = getFlowConditionName();
        String direction = FlowFilterMap.getFlowDirectionName(output);
        String desc = pctx.getDescription();
        VInterfacePath dst = e.getDestination();
        if (pctx.setFirstRedirection(e)) {
            LOG.debug(format, e.getLogPrefix(), cond, dst, direction, desc);
        } else if (LOG.isTraceEnabled()) {
            LOG.trace(format, e.getLogPrefix(), cond, dst, direction, desc);
        }

        throw e;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected RedirectFilter getFilterType() {
        return destination.getRedirectFilter(output);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected Logger getLogger() {
        return LOG;
    }
}
