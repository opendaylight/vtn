/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.flow.action;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

import org.opendaylight.vtn.manager.internal.util.MiscUtils;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.VtnAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.flow.action.list.VtnFlowActionBuilder;

import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.Action;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.list.ActionBuilder;

/**
 * {@code VTNFlowAction} descrbes the abstracted flow action.
 */
public abstract class VTNFlowAction {
    /**
     * Construct an empty instance.
     */
    VTNFlowAction() {
    }

    /**
     * Convert this instance into a VTN flow action builder instance.
     *
     * @param ord  An integer which determines the order of flow actions
     *             in a flow entry.
     * @return  A {@link VtnFlowActionBuilder} instance.
     */
    public final VtnFlowActionBuilder toVtnFlowActionBuilder(Integer ord) {
        return set(new VtnFlowActionBuilder().setOrder(ord));
    }

    /**
     * Convert this instance into a MD-SAL action builder instance.
     *
     * @param ord  An integer which determines the order of flow actions
     *             in a flow entry.
     * @return  An {@link ActionBuilder} instance.
     */
    public final ActionBuilder toActionBuilder(Integer ord) {
        return set(new ActionBuilder().setOrder(ord));
    }

    /**
     * Return a message which indicates an error caused by this class.
     *
     * @param args  Objects to be embedded in an error message.
     * @return  An error message.
     */
    protected final String getErrorMessage(Object ... args) {
        List<Object> list = new ArrayList<>();
        list.add(getClass().getSimpleName());
        Collections.addAll(list, args);
        return MiscUtils.join(": ", list);
    }

    /**
     * Append strings which represents the contents of this instance into the
     * given {@link StringBuilder} instance.
     *
     * @param builder  A {@link StringBuilder} instance.
     */
    protected void appendContents(StringBuilder builder) {
        // Nothing to do.
    }

    /**
     * Cast the given object as the given type.
     *
     * @param type  A class that indicates the target type.
     * @param obj   An object to be converted.
     * @param <T>   The target type.
     * @return  {@code obj} casted as the given type.
     * @throws RpcException
     *    Unable to cast the given instance.
     */
    protected final <T> T cast(Class<T> type, Object obj) throws RpcException {
        T value = MiscUtils.cast(type, obj);
        if (value == null) {
            String msg = getErrorMessage("Unexpected type", obj);
            throw RpcException.getBadArgumentException(msg);
        }

        return value;
    }

    /**
     * Convert the given MD-SAL action instance into a {@link VtnAction}
     * instance.
     *
     * <p>
     *   Note that this method must not affect instance variables.
     * </p>
     *
     * @param act  An {@link Action} instance.
     * @return  A {@link VtnAction} instance.
     * @throws RpcException
     *    Failed to convert the given instance.
     */
    public abstract VtnAction toVtnAction(Action act) throws RpcException;

    /**
     * Return a brief description about the specified MD-SAL action.
     *
     * <p>
     *   Note that this method must not affect instance variables.
     * </p>
     *
     * @param act  An {@link Action} instance.
     * @return  A brief description about the given MD-SAL action.
     * @throws RpcException  An error occurred.
     */
    public abstract String getDescription(Action act) throws RpcException;

    /**
     * Set a VTN action into the given VTN flow action builder.
     *
     * @param builder  A {@link VtnFlowActionBuilder} instance.
     * @return  {@code builder}.
     */
    protected abstract VtnFlowActionBuilder set(VtnFlowActionBuilder builder);

    /**
     * Set a MD-SAL action into the given action builder.
     *
     * @param builder  An {@link ActionBuilder} instance.
     * @return  {@code builder}.
     */
    protected abstract ActionBuilder set(ActionBuilder builder);

    // Object

    /**
     * Determine whether the given object is identical to this object.
     *
     * @param o  An object to be compared.
     * @return   {@code true} if identical. Otherwise {@code false}.
     */
    @Override
    public boolean equals(Object o) {
        return (o != null && getClass().equals(o.getClass()));
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        return getClass().hashCode();
    }

    /**
     * Return a string representation of this instance.
     *
     * @return  A string representation of this instance.
     */
    @Override
    public final String toString() {
        StringBuilder builder = new StringBuilder(getClass().getSimpleName()).
            append('[');
        appendContents(builder);
        return builder.append(']').toString();
    }
}
