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

import org.opendaylight.vtn.manager.VTNException;
import org.opendaylight.vtn.manager.flow.filter.DropFilter;
import org.opendaylight.vtn.manager.flow.filter.FlowFilter;

import org.opendaylight.vtn.manager.internal.PacketContext;
import org.opendaylight.vtn.manager.internal.VTNManagerImpl;

/**
 * This class describes DROP flow filter, which discards packet.
 *
 * <p>
 *   Although this class is public to other packages, this class does not
 *   provide any API. Applications other than VTN Manager must not use this
 *   class.
 * </p>
 */
public final class DropFlowFilterImpl extends FlowFilterImpl {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = -4839160630229249797L;

    /**
     * Logger instance.
     */
    private static final Logger  LOG =
        LoggerFactory.getLogger(DropFlowFilterImpl.class);

    /**
     * Construct a new instance.
     *
     * @param idx     An index number to be assigned.
     * @param filter  A {@link FlowFilter} instance.
     * @throws VTNException
     *    {@code filter} contains invalid value.
     */
    protected DropFlowFilterImpl(int idx, FlowFilter filter)
        throws VTNException {
        super(idx, filter);
    }

    // FlowFilterImpl

    /**
     * Determine whether this flow filter can handle multicast packets or not.
     *
     * <p>
     *   This method always returns {@code true} because DROP filter can
     *   discard multicast packets.
     * </p>
     *
     * @return  {@code true}.
     */
    @Override
    protected boolean isMulticastSupported() {
        return true;
    }

    /**
     * Determine whether this flow filter needs to apply flow actions to the
     * packet.
     *
     * <p>
     *   This method always returns {@code false} because applying flow actions
     *   to the packet to be discarded is meaningless.
     * </p>
     *
     * @return  {@code false}.
     */
    protected boolean needFlowAction() {
        return false;
    }

    /**
     * Apply this DROP flow filter to the given packet.
     *
     * @param mgr    VTN Manager service.
     * @param pctx   A packet context which contains the packet.
     * @param ffmap  A {@link FlowFilterMap} instance that contains this
     *               flow filter.
     * @throws DropFlowException  Always thrown.
     */
    @Override
    protected void apply(VTNManagerImpl mgr, PacketContext pctx,
                         final FlowFilterMap ffmap) throws DropFlowException {
        if (!pctx.isFlooding()) {
            LOG.info("{}: Discard packet: cond={}, packet={}",
                     ffmap.getLogPrefix(getIndex()), getFlowConditionName(),
                     pctx.getDescription());
        }

        // Install a flow entry that discards the given packet.
        pctx.installDropFlow();

        throw new DropFlowException();
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected DropFilter getFilterType() {
        return new DropFilter();
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected Logger getLogger() {
        return LOG;
    }
}

/**
 * An exception which indicates a packet was discarded by a DROP flow filter.
 */
final class DropFlowException extends Exception {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = 1L;

    /**
     * Construct a new instance.
     */
    DropFlowException() {
        super();
    }

    /**
     * Construct a new instance.
     *
     * @param cause  The {@link Throwable} object which indicates the cause
     *               of error.
     */
    DropFlowException(Throwable cause) {
        super(cause);
    }
}
