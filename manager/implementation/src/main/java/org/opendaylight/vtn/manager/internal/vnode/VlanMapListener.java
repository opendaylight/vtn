/*
 * Copyright (c) 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.vnode;

import static org.opendaylight.vtn.manager.internal.util.MiscUtils.LOG_SEPARATOR;
import static org.opendaylight.vtn.manager.internal.vnode.VTenantManager.LOG;
import static org.opendaylight.vtn.manager.internal.vnode.VTenantManager.TAG_NODE;

import java.util.Objects;

import org.opendaylight.vtn.manager.internal.util.ChangedData;
import org.opendaylight.vtn.manager.internal.util.IdentifiedData;
import org.opendaylight.vtn.manager.internal.util.MiscUtils;
import org.opendaylight.vtn.manager.internal.util.vnode.VNodeType;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.vlan.rev150907.vtn.vlan.map.info.VlanMapConfig;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.vlan.rev150907.vtn.vlan.map.info.VlanMapStatus;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.vlan.rev150907.vtn.vlan.mappable.VlanMap;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.vlan.rev150907.vtn.vlan.mappable.VlanMapKey;

/**
 * Data change listener that logs changes of VLAN mapping configuration.
 */
final class VlanMapListener extends VNodeLogListener<VlanMap> {
    /**
     * Convert the given vlan-map-config into a string.
     *
     * @param value  A {@link VlanMapConfig} instance.
     * @return  A string that represents the specified data.
     */
    private static String toString(VlanMapConfig value) {
        StringBuilder builder = new StringBuilder();
        String node = MiscUtils.getValue(value.getNode());
        if (node != null) {
            builder.append(TAG_NODE).append(node).append(LOG_SEPARATOR);
        }

        return builder.append("vlan-id=").append(value.getVlanId().getValue()).
            toString();
    }

    /**
     * Data change listener for vlan-map-status.
     */
    static final class VlanMapStatusListener
        extends VNodeChangeListener<VlanMapStatus> {
        /**
         * Construct a new instance.
         */
        VlanMapStatusListener() {
            super(VlanMapStatus.class, false);
        }

        /**
         * Log the given status of VLAN mapping.
         *
         * @param data  A {@link IdentifiedData} instance.
         */
        private void onChanged(IdentifiedData<?> data) {
            IdentifiedData<VlanMapStatus> cdata = cast(data);
            InstanceIdentifier<VlanMapStatus> path = cdata.getIdentifier();
            VlanMapKey key = path.firstKeyOf(VlanMap.class);
            VlanMapStatus status = cdata.getValue();
            Boolean active = status.isActive();
            String desc = (Boolean.TRUE.equals(active))
                ? "activated" : "inactivated";
            LOG.info("{}: VLAN mapping has been {}: map-id={}",
                     getVNodeIdentifier(path), desc, key.getMapId());
        }

        // VNodeChangeListener

        /**
         * {@inheritDoc}
         */
        @Override
        void onCreated(VTenantChange ectx, IdentifiedData<?> data) {
            if (LOG.isInfoEnabled()) {
                onChanged(data);
            }
        }

        /**
         * {@inheritDoc}
         */
        @Override
        void onUpdated(VTenantChange ectx, ChangedData<?> data) {
            onChanged(data);
        }

        /**
         * {@inheritDoc}
         */
        @Override
        void onRemoved(VTenantChange ectx, IdentifiedData<?> data) {
            // Nothing to do.
        }

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
            boolean changed;
            if (LOG.isInfoEnabled()) {
                ChangedData<VlanMapStatus> cdata = cast(data);
                VlanMapStatus old = cdata.getOldValue();
                VlanMapStatus vms = cdata.getValue();
                changed = !Objects.equals(old.isActive(), vms.isActive());
            } else {
                // No need to call onUpdated() if info log is disabled.
                changed = false;
            }

            return changed;
        }
    }

    /**
     * Data change listener for vlan-map-config.
     */
    static final class VlanMapConfigListener
        extends VNodeLogListener<VlanMapConfig> {
        /**
         * Construct a new instance.
         */
        VlanMapConfigListener() {
            super(VlanMapConfig.class, true);
        }

        // VNodeLogListener

        /**
         * {@inheritDoc}
         */
        @Override
        String getDescription(InstanceIdentifier<VlanMapConfig> path) {
            return VNodeType.VLANMAP.getDescription();
        }

        /**
         * {@inheritDoc}
         */
        @Override
        String toString(VlanMapConfig value) {
            return VlanMapListener.toString(value);
        }

        /**
         * This method does nothing.
         *
         * <p>
         *   Creation event will be handled by {@link VlanMapListener}.
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
         *   Removal event will be handled by {@link VlanMapListener}.
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
            ChangedData<VlanMapConfig> cdata = cast(data);
            VlanMapConfig old = cdata.getOldValue();
            VlanMapConfig vmc = cdata.getValue();
            return !(Objects.equals(old.getNode(), vmc.getNode()) &&
                     Objects.equals(old.getVlanId(), vmc.getVlanId()));
        }
    }

    /**
     * Construct a new instance.
     */
    VlanMapListener() {
        super(VlanMap.class, true);
    }

    // VNodeLogListener

    /**
     * {@inheritDoc}
     */
    @Override
    String getDescription(InstanceIdentifier<VlanMap> path) {
        return VNodeType.VLANMAP.getDescription();
    }

    /**
     * {@inheritDoc}
     */
    @Override
    String toString(VlanMap value) {
        return toString(value.getVlanMapConfig());
    }

    // VNodeChangeListener

    /**
     * Determine whether the tatget virtual node should be treated as a
     * leaf node or not.
     *
     * <p>
     *   This method always returns {@code false} because vlan-map
     *   contains vlan-map-config and vlan-map-status.
     * </p>
     *
     * @return  {@code false}.
     */
    @Override
    boolean isLeafNode() {
        return false;
    }
}
