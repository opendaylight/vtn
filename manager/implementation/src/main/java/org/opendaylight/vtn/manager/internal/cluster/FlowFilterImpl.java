/*
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
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

import org.opendaylight.vtn.manager.internal.MiscUtils;

import org.opendaylight.controller.sal.utils.Status;
import org.opendaylight.controller.sal.utils.StatusCode;

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
    private static final long serialVersionUID = 3869049816189688050L;

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
     * Create a new flow filter implementation.
     *
     * @param idx     An index number to be assigned.
     * @param filter  A {@link FlowFilter} instance.
     * @return  A new {@link FlowFilterImpl} instance.
     * @throws VTNException
     *    {@code filter} contains invalid value.
     */
    public static FlowFilterImpl create(int idx, FlowFilter filter)
        throws VTNException {
        if (filter == null) {
            Status st = MiscUtils.argumentIsNull("Flow filter");
            throw new VTNException(st);
        }

        FilterType type = filter.getFilterType();
        if (type == null) {
            Status st = MiscUtils.argumentIsNull("Flow filter type");
            throw new VTNException(st);
        }

        if (type instanceof PassFilter) {
            return new PassFlowFilterImpl(idx, filter);
        }
        if (type instanceof DropFilter) {
            return new DropFlowFilterImpl(idx, filter);
        }
        if (type instanceof RedirectFilter) {
            return new RedirectFlowFilterImpl(idx, filter);
        }

        String msg = "Unexpected flow filter type: " + type;
        throw new VTNException(StatusCode.BADREQUEST, msg);
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
            String msg = "Invalid index: " + idx;
            throw new VTNException(StatusCode.BADREQUEST, msg);
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
     * Return a flow index assigned to this instance.
     *
     * @return  A flow index.
     */
    public final int getIndex() {
        return index;
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
