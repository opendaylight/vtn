/*
 * Copyright (c) 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.vnode;

import static org.opendaylight.vtn.manager.internal.util.MiscUtils.LOG_SEPARATOR;
import static org.opendaylight.vtn.manager.internal.vnode.VTenantManager.TAG_COND;
import static org.opendaylight.vtn.manager.internal.vnode.VTenantManager.TAG_INDEX;

import java.util.Objects;

import org.opendaylight.vtn.manager.internal.util.ChangedData;
import org.opendaylight.vtn.manager.internal.util.flow.action.FlowActionConverter;
import org.opendaylight.vtn.manager.internal.util.flow.filter.VTNFlowFilter;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.flow.action.list.VtnFlowAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.flow.filter.list.VtnFlowFilter;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.flow.filter.list.VtnFlowFilterKey;

/**
 * Data change listener that logs changes of flow filter configuration.
 */
final class VtnFlowFilterListener extends VNodeLogListener<VtnFlowFilter> {
    /**
     * Data change listener for vtn-flow-action in vtn-flow-filter.
     */
    static final class VtnFlowActionListener
        extends VNodeLogListener<VtnFlowAction> {
        /**
         * Construct a new instance.
         */
        VtnFlowActionListener() {
            super(VtnFlowAction.class, true);
        }

        // VNodeLogListener

        /**
         * {@inheritDoc}
         */
        @Override
        String getDescription(InstanceIdentifier<VtnFlowAction> path) {
            String direction = (VTNFlowFilter.isOutput(path))
                ? "out" : "in";
            VtnFlowFilterKey key = path.firstKeyOf(VtnFlowFilter.class);
            Integer index = (key == null) ? null : key.getIndex();
            return direction + "." + index + ": Flow filter action";
        }

        /**
         * {@inheritDoc}
         */
        @Override
        String toString(VtnFlowAction value) {
            FlowActionConverter conv = FlowActionConverter.getInstance();
            return "order=" + value.getOrder() +
                ", action=" + conv.getDescription(value.getVtnAction());
        }

        // VNodeChangeListener

        /**
         * Determine whether the specified virtual node was updated or not.
         *
         * @param data  A {@link ChangedData} instance that contains values
         *              before and after modification.
         * @return  {@code true} if the target data was updated.
         *          {@code false} otherwise.
         */
        @Override
        boolean isUpdated(ChangedData<?> data) {
            ChangedData<VtnFlowAction> cdata = cast(data);
            VtnFlowAction old = cdata.getOldValue();
            VtnFlowAction vact = cdata.getValue();
            return !Objects.equals(old.getVtnAction(), vact.getVtnAction());
        }
    }

    /**
     * Construct a new instance.
     */
    VtnFlowFilterListener() {
        super(VtnFlowFilter.class, true);
    }

    // VNodeLogListener

    /**
     * {@inheritDoc}
     */
    @Override
    String getDescription(InstanceIdentifier<VtnFlowFilter> path) {
        return (VTNFlowFilter.isOutput(path))
            ? "Output flow filter" : "Input flow filter";
    }

    /**
     * {@inheritDoc}
     */
    @Override
    String toString(VtnFlowFilter value) {
        return TAG_INDEX + value.getIndex() + LOG_SEPARATOR +
            TAG_COND + value.getCondition().getValue() +
            ", type=" + VTNFlowFilter.getTypeDescription(value);
    }

    // VNodeChangeListener

    /**
     * Determine whether the specified virtual node was updated or not.
     *
     * @param data  A {@link ChangedData} instance that contains values
     *              before and after modification.
     * @return  {@code true} if the target data was updated.
     *          {@code false} otherwise.
     */
    @Override
    boolean isUpdated(ChangedData<?> data) {
        ChangedData<VtnFlowFilter> cdata = cast(data);
        VtnFlowFilter old = cdata.getOldValue();
        VtnFlowFilter vff = cdata.getValue();

        // We don't need to check vtn-flow-action list because it will be
        // done by VtnFlowActionListener.
        return !(Objects.equals(old.getCondition(), vff.getCondition()) &&
                 Objects.equals(old.getVtnFlowFilterType(),
                                vff.getVtnFlowFilterType()));
    }

    /**
     * Determine whether the tatget virtual node should be treated as a
     * leaf node or not.
     *
     * <p>
     *   This method always returns {@code false} because vtn-flow-filter
     *   contains vtn-flow-action list.
     * </p>
     *
     * @return  {@code false}.
     */
    @Override
    boolean isLeafNode() {
        return false;
    }
}
