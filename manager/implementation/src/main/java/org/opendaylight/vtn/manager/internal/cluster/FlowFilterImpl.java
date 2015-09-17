/*
 * Copyright (c) 2014, 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.cluster;

import java.io.Serializable;
import java.util.ArrayList;
import java.util.List;

import org.slf4j.Logger;

import org.opendaylight.vtn.manager.VTNException;
import org.opendaylight.vtn.manager.flow.action.FlowAction;
import org.opendaylight.vtn.manager.flow.filter.DropFilter;
import org.opendaylight.vtn.manager.flow.filter.FilterType;
import org.opendaylight.vtn.manager.flow.filter.FlowFilter;
import org.opendaylight.vtn.manager.flow.filter.PassFilter;
import org.opendaylight.vtn.manager.flow.filter.RedirectFilter;

import org.opendaylight.vtn.manager.internal.PacketContext;
import org.opendaylight.vtn.manager.internal.VTNManagerImpl;
import org.opendaylight.vtn.manager.internal.util.MiscUtils;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;
import org.opendaylight.vtn.manager.internal.util.flow.cond.FlowCondReader;
import org.opendaylight.vtn.manager.internal.util.flow.cond.VTNFlowCondition;

/**
 * This class provides base implementation of flow filter.
 *
 * <p>
 *   Although this class is public to other packages, this class does not
 *   provide any API. Applications other than VTN Manager must not use this
 *   class.
 * </p>
 */
public abstract class FlowFilterImpl implements Serializable {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = 2005617223101631695L;

    /**
     * The minimum value of filter index.
     */
    private static final int  INDEX_MIN = 1;

    /**
     * The maximum value of filter index.
     */
    private static final int  INDEX_MAX = 65535;

    /**
     * Filter index assigned to this flow filter.
     */
    private final int  index;

    /**
     * The name of the flow condition.
     */
    private final String  condition;

    /**
     * A list of flow actions to modify packet.
     */
    private final List<FlowActionImpl>  actions;

    /**
     * An internal exception to notify that this flow filter does not support
     * the given packet.
     */
    private static final class UnsupportedPacketException extends Exception {
        /**
         * Construct a new exception.
         *
         * @param msg  A message.
         */
        private UnsupportedPacketException(String msg) {
            super(msg);
        }
    }

    /**
     * Create a new flow filter implementation.
     *
     * @param fnode   Virtual node that contains this flow filter.
     * @param idx     An index number to be assigned.
     * @param filter  A {@link FlowFilter} instance.
     * @return  A new {@link FlowFilterImpl} instance.
     * @throws VTNException
     *    {@code filter} contains invalid value.
     */
    public static FlowFilterImpl create(FlowFilterNode fnode, int idx,
                                        FlowFilter filter)
        throws VTNException {
        if (filter == null) {
            throw RpcException.getNullArgumentException("Flow filter");
        }

        FilterType type = filter.getFilterType();
        if (type == null) {
            throw RpcException.getNullArgumentException("Flow filter type");
        }

        if (type instanceof PassFilter) {
            return new PassFlowFilterImpl(idx, filter);
        }
        if (type instanceof DropFilter) {
            return new DropFlowFilterImpl(idx, filter);
        }
        if (type instanceof RedirectFilter) {
            return new RedirectFlowFilterImpl(fnode, idx, filter);
        }

        throw RpcException.getBadArgumentException(
            "Unexpected flow filter type: " + type);
    }

    /**
     * Construct a new instance.
     *
     * @param idx     An index number to be assigned.
     * @param filter  A {@link FlowFilter} instance.
     * @throws VTNException
     *    {@code filter} contains invalid value.
     */
    protected FlowFilterImpl(int idx, FlowFilter filter) throws VTNException {
        if (idx < INDEX_MIN || idx > INDEX_MAX) {
            throw RpcException.getBadArgumentException(
                "Invalid index: " + idx);
        }

        index = idx;
        condition = filter.getFlowConditionName();
        MiscUtils.checkName("Flow condition", condition);

        List<FlowAction> acts = filter.getActions();
        if (acts == null) {
            actions = null;
        } else {
            actions = new ArrayList<FlowActionImpl>(acts.size());
            for (FlowAction act: acts) {
                actions.add(FlowActionImpl.create(act));
            }
        }
    }

    /**
     * Return a flow filter index assigned to this instance.
     *
     * @return  A flow filter index.
     */
    public final int getIndex() {
        return index;
    }

    /**
     * Return the name of the flow condition that selects packets to be
     * filtered.
     *
     * @return  The name of the flow condition.
     */
    public final String getFlowConditionName() {
        return condition;
    }

    /**
     * Return a {@link FlowFilter} instance which represents this filter.
     *
     * @return  A {@link FlowFilter} instance.
     */
    public final FlowFilter getFlowFilter() {
        List<FlowAction> acts;
        if (actions == null) {
            acts = null;
        } else {
            acts = new ArrayList<FlowAction>(actions.size());
            for (FlowActionImpl ai: actions) {
                acts.add(ai.getFlowAction());
            }
        }

        FilterType type = getFilterType();
        return new FlowFilter(index, condition, type, acts);
    }

    /**
     * Evaluate this flow filter against the given packet.
     *
     * @param mgr    VTN Manager service.
     * @param pctx   A packet context which contains the packet.
     * @param ffmap  A {@link FlowFilterMap} instance that contains this
     *               flow filter.
     * @return  {@code true} is returned if this flow filter was applied to
     *          the given packet. Otherwise {@code false} is returned.
     * @throws DropFlowException
     *    The given packet was discarded by this flow filter.
     * @throws RedirectFlowException
     *    The given packet was redirected by this flow filter.
     */
    public final boolean evaluate(VTNManagerImpl mgr, PacketContext pctx,
                                  FlowFilterMap ffmap)
        throws DropFlowException, RedirectFlowException {
        boolean ret = false;
        try {
            VTNFlowCondition vfcond = getCondition(pctx);
            if (vfcond.match(pctx)) {
                // Apply this flow filter.
                if (needFlowAction()) {
                    applyFlowActions(pctx, ffmap);
                }
                apply(mgr, pctx, ffmap);
                ret = true;
            } else {
                Logger logger = getLogger();
                if (logger.isTraceEnabled()) {
                    logger.trace("{}: Packet does not match the condition: {}",
                                 ffmap.getLogPrefix(index), condition);
                }
            }
        } catch (UnsupportedPacketException e) {
            Logger logger = getLogger();
            if (logger.isDebugEnabled()) {
                String msg = e.getMessage();
                logger.debug("{}: Ignore flow filter: {}: {}",
                             ffmap.getLogPrefix(index), msg, condition);
            }
        }

        return ret;
    }

    /**
     * Append the contents of this instance to the given {@link StringBuilder}
     * instance.
     *
     * @param builder  A {@link StringBuilder} instance.
     */
    protected void appendContents(StringBuilder builder) {
        builder.append("index=").append(index).
            append(",cond=").append(condition);
        if (actions != null) {
            builder.append(",actions=").append(actions);
        }
    }

    /**
     * Determine whether this flow filter can handle multicast packets or not.
     *
     * <p>
     *   This method always returns {@code false} which indicates that this
     *   flow filter should ignore multicast packets.
     *   Subclass may override this method to support multicast packets.
     * </p>
     *
     * @return  {@code false}.
     */
    protected boolean isMulticastSupported() {
        return false;
    }

    /**
     * Determine whether this flow filter needs to apply flow actions to the
     * packet.
     *
     * <p>
     *   This method returns {@code true} which indicates that this flow filter
     *   needs to apply flow actions configured in this instance.
     *   Subclass may override this method to ignore flow actions.
     * </p>
     *
     * @return  {@code true}.
     */
    protected boolean needFlowAction() {
        return true;
    }

    /**
     * Determine whether this flow filter supports packet flooding or not.
     *
     * <p>
     *   This method returns {@code true} which indicates that this flow filter
     *   needs to be evaluated even when the packet is going to be broadcasted
     *   in the vBridge.
     *   Subclass may override this method to ignore flow actions.
     * </p>
     *
     * @return  {@code true}.
     */
    protected boolean isFloodingSuppoted() {
        return true;
    }

    /**
     * Apply this flow filter to the given packet.
     *
     * @param mgr    VTN Manager service.
     * @param pctx   A packet context which contains the packet.
     * @param ffmap  A {@link FlowFilterMap} instance that contains this
     *               flow filter.
     * @throws DropFlowException
     *    A packet was discarded by this flow filter.
     * @throws RedirectFlowException
     *    The given packet was redirected by this flow filter.
     */
    protected abstract void apply(VTNManagerImpl mgr, PacketContext pctx,
                                  FlowFilterMap ffmap)
        throws DropFlowException, RedirectFlowException;

    /**
     * Return a {@link FilterType} instance which represents the type of
     * this flow filter.
     *
     * @return  A {@link FilterType} instance.
     */
    protected abstract FilterType getFilterType();

    /**
     * Return a logger instance.
     *
     * @return  A logger instance.
     */
    protected abstract Logger getLogger();

    /**
     * Apply flow actions to the given packet.
     *
     * @param pctx   A packet context which contains the packet.
     * @param ffmap  A {@link FlowFilterMap} instance that contains this
     *               flow filter.
     */
    private void applyFlowActions(PacketContext pctx, FlowFilterMap ffmap) {
        if (actions != null) {
            Logger logger = getLogger();
            boolean doTrace = logger.isTraceEnabled();
            for (FlowActionImpl ai: actions) {
                if (ai.apply(pctx)) {
                    if (doTrace) {
                        logger.trace("{}: Flow action was applied: {}",
                                     ffmap.getLogPrefix(index), ai);
                    }
                } else if (doTrace) {
                    logger.trace("{}: Flow action was ignored: {}",
                                 ffmap.getLogPrefix(index), ai);
                }
            }
        }
    }

    /**
     * Return a {@link VTNFlowCondition} instance which determines whether this
     * flow filter needs to be applied to the given packet.
     *
     * <p>
     *   Note that this method also checks whether this flow filter supports
     *   the given packet or not.
     * </p>
     *
     * @param pctx  A packet context which contains the packet.
     * @return  A {@link VTNFlowCondition} instance which selects the packet.
     * @throws UnsupportedPacketException
     *    This flow filter does not support the given packet.
     */
    private VTNFlowCondition getCondition(PacketContext pctx)
        throws UnsupportedPacketException {
        if (!pctx.isUnicast() && !isMulticastSupported()) {
            throw new UnsupportedPacketException(
                "multicast packet is not supported");
        }

        if (pctx.isFlooding() && !isFloodingSuppoted()) {
            throw new UnsupportedPacketException(
                "flooding packet is not supported");
        }

        FlowCondReader reader = pctx.getTxContext().getFlowCondReader();
        VTNFlowCondition vfcond = reader.get(condition);
        if (vfcond == null) {
            throw new UnsupportedPacketException("Flow condition not found");
        }

        return vfcond;
    }

    /**
     * Determine whether the given object is identical to this object.
     *
     * @param o  An object to be compared.
     * @return   {@code true} if identical. Otherwise {@code false}.
     */
    @Override
    public boolean equals(Object o) {
        if (o == null || !getClass().equals(o.getClass())) {
            return false;
        }

        FlowFilterImpl fi = (FlowFilterImpl)o;
        if (actions == null) {
            if (fi.actions != null) {
                return false;
            }
        } else if (!actions.equals(fi.actions)) {
            return false;
        }

        return (index == fi.index && condition.equals(fi.condition));
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        int h = getClass().getName().hashCode() + (index * 11) +
            (condition.hashCode() * 17);
        if (actions != null) {
            h += (actions.hashCode() * 23);
        }

        return h;
    }

    /**
     * Return a string representation of this object.
     *
     * @return  A string representation of this object.
     */
    @Override
    public final String toString() {
        StringBuilder builder = new StringBuilder(getClass().getSimpleName());
        builder.append('[');
        appendContents(builder);
        return builder.append(']').toString();
    }
}
