/*
 * Copyright (c) 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.vnode;

import static org.opendaylight.vtn.manager.internal.vnode.VTenantManager.TAG_DESC;

import java.util.Objects;

import org.opendaylight.vtn.manager.internal.util.ChangedData;
import org.opendaylight.vtn.manager.internal.util.IdentifiedData;
import org.opendaylight.vtn.manager.internal.util.vnode.VNodeType;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vterminal.rev150907.vtn.vterminal.info.VterminalConfig;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vterminal.rev150907.vtn.vterminal.list.Vterminal;

/**
 * Data change listener that logs changes of vTerminal configuration.
 */
final class VterminalListener extends VNodeLogListener<Vterminal> {
    /**
     * Convert the given vterminal-config into a string.
     *
     * @param value  A {@link VterminalConfig} instance.
     * @return  A string that represents the specified data.
     */
    private static String toString(VterminalConfig value) {
        String desc = value.getDescription();
        StringBuilder builder = new StringBuilder();
        if (desc != null) {
            builder.append(TAG_DESC).append(desc);
        }

        return builder.toString();
    }


    /**
     * Data change listener for vterminal-config.
     */
    static final class VterminalConfigListener
        extends VNodeLogListener<VterminalConfig> {
        /**
         * Construct a new instance.
         */
        VterminalConfigListener() {
            super(VterminalConfig.class, true);
        }

        // VNodeLogListener

        /**
         * {@inheritDoc}
         */
        @Override
        String getDescription(InstanceIdentifier<VterminalConfig> path) {
            return VNodeType.VTERMINAL.toString();
        }

        /**
         * {@inheritDoc}
         */
        @Override
        String toString(VterminalConfig value) {
            return VterminalListener.toString(value);
        }

        /**
         * This method does nothing.
         *
         * <p>
         *   Creation event will be handled by {@link VterminalListener}.
         * </p>
         *
         * @param ectx  Unused.
         * @param data  Unused.
         */
        @Override
        void onCreated(VTenantChange ectx, IdentifiedData<?> data) {
        }

        /**
         * This method does nothing.
         *
         * <p>
         *   Removal event will be handled by {@link VterminalListener}.
         * </p>
         *
         * @param ectx  Unused.
         * @param data  Unused.
         */
        @Override
        void onRemoved(VTenantChange ectx, IdentifiedData<?> data) {
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
            ChangedData<VterminalConfig> cdata = cast(data);
            VterminalConfig old = cdata.getOldValue();
            VterminalConfig vtmc = cdata.getValue();
            return !Objects.equals(old.getDescription(),
                                   vtmc.getDescription());
        }
    }

    /**
     * Construct a new instance.
     */
    VterminalListener() {
        super(Vterminal.class, true);
    }

    // VNodeLogListener

    /**
     * {@inheritDoc}
     */
    @Override
    String getDescription(InstanceIdentifier<Vterminal> path) {
        return VNodeType.VTERMINAL.toString();
    }

    /**
     * {@inheritDoc}
     */
    @Override
    String toString(Vterminal value) {
        return toString(value.getVterminalConfig());
    }

    // VNodeChangeListener

    /**
     * Determine whether the tatget virtual node should be treated as a
     * leaf node or not.
     *
     * <p>
     *   This method always returns {@code false} because vterminal
     *   contains children such as vterminal-config.
     * </p>
     *
     * @return  {@code false}.
     */
    @Override
    boolean isLeafNode() {
        return false;
    }
}
