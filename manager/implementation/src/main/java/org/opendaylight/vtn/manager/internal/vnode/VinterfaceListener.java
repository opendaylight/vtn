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
import static org.opendaylight.vtn.manager.internal.vnode.VTenantManager.TAG_DESC;
import static org.opendaylight.vtn.manager.internal.vnode.VTenantManager.TAG_NODE;
import static org.opendaylight.vtn.manager.internal.vnode.VTenantManager.TAG_STATE;

import java.util.Objects;

import org.opendaylight.vtn.manager.internal.util.ChangedData;
import org.opendaylight.vtn.manager.internal.util.IdentifiedData;
import org.opendaylight.vtn.manager.internal.util.MiscUtils;
import org.opendaylight.vtn.manager.internal.util.vnode.VInterfaceIdentifier;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.port.rev150907.vtn.port.mappable.PortMapConfig;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.vtn.mappable.vinterface.VinterfaceStatus;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.vtn.mappable.vinterface.list.Vinterface;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.vtn.vinterface.info.VinterfaceConfig;

/**
 * Data change listener that logs changes of virtual interface configuration.
 */
final class VinterfaceListener extends VNodeLogListener<Vinterface> {
    /**
     * Convert the given vinterface-config into a string.
     *
     * @param value  A {@link VinterfaceConfig} instance.
     * @return  A string that represents the specified data.
     */
    private static String toString(VinterfaceConfig value) {
        String desc = value.getDescription();
        StringBuilder builder = new StringBuilder();
        if (desc != null) {
            builder.append(TAG_DESC).append(desc).append(LOG_SEPARATOR);
        }

        return builder.append("enabled=").append(value.isEnabled()).toString();
    }

    /**
     * Data change listener for port-map-config.
     */
    static final class PortMapConfigListener
        extends VNodeLogListener<PortMapConfig> {
        /**
         * Construct a new instance.
         */
        PortMapConfigListener() {
            super(PortMapConfig.class, true);
        }

        // VNodeLogListener

        /**
         * {@inheritDoc}
         */
        @Override
        String getDescription(InstanceIdentifier<PortMapConfig> path) {
            return "Port mapping";
        }

        /**
         * {@inheritDoc}
         */
        @Override
        String toString(PortMapConfig value) {
            StringBuilder builder = new StringBuilder(TAG_NODE).
                append(MiscUtils.getValue(value.getNode()));
            String id = value.getPortId();
            if (id != null) {
                builder.append(", port-id=").append(id);
            }
            String name = value.getPortName();
            if (name != null) {
                builder.append(", port-name=").append(name);
            }

            return builder.toString();
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
            ChangedData<PortMapConfig> cdata = cast(data);
            PortMapConfig old = cdata.getOldValue();
            PortMapConfig pmc = cdata.getValue();

            // Compare node and vlan-id.
            boolean changed = !(Objects.equals(old.getNode(), pmc.getNode()) &&
                                Objects.equals(old.getVlanId(),
                                               pmc.getVlanId()));
            if (!changed) {
                // Compare port-id and port-name.
                changed = !(Objects.equals(old.getPortId(), pmc.getPortId()) &&
                            Objects.equals(old.getPortName(),
                                           pmc.getPortName()));
            }

            return changed;
        }
    }

    /**
     * Data change listener for vinterface-config.
     */
    static final class VinterfaceConfigListener
        extends VNodeLogListener<VinterfaceConfig> {
        /**
         * Construct a new instance.
         */
        VinterfaceConfigListener() {
            super(VinterfaceConfig.class, true);
        }

        // VNodeLogListener

        /**
         * {@inheritDoc}
         */
        @Override
        String getDescription(InstanceIdentifier<VinterfaceConfig> path) {
            return VInterfaceIdentifier.DESCRIPTION;
        }

        /**
         * {@inheritDoc}
         */
        @Override
        String toString(VinterfaceConfig value) {
            return VinterfaceListener.toString(value);
        }

        /**
         * This method does nothing.
         *
         * <p>
         *   Creation event will be handled by {@link VinterfaceListener}.
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
         *   Removal event will be handled by {@link VinterfaceListener}.
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
            ChangedData<VinterfaceConfig> cdata = cast(data);
            VinterfaceConfig old = cdata.getOldValue();
            VinterfaceConfig vifc = cdata.getValue();
            return !(Objects.equals(old.getDescription(),
                                    vifc.getDescription()) &&
                     Objects.equals(old.isEnabled(), vifc.isEnabled()));
        }
    }

    /**
     * Data change listener for vinterface-status.
     */
    static final class VinterfaceStatusListener
        extends VNodeLogListener<VinterfaceStatus> {
        /**
         * Construct a new instance.
         */
        VinterfaceStatusListener() {
            super(VinterfaceStatus.class, false);
        }

        // VNodeLogListener

        /**
         * {@inheritDoc}
         */
        @Override
        String getDescription(InstanceIdentifier<VinterfaceStatus> path) {
            return "vInterface status";
        }

        /**
         * {@inheritDoc}
         */
        @Override
        String toString(VinterfaceStatus value) {
            StringBuilder builder = new StringBuilder(TAG_STATE).
                append(value.getState()).
                append(", entity-state=").append(value.getEntityState());
            String port = MiscUtils.getValue(value.getMappedPort());
            if (port != null) {
                builder.append(", mapped-port=").append(port);
            }

            return builder.toString();
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
            boolean changed;
            if (LOG.isInfoEnabled()) {
                ChangedData<VinterfaceStatus> cdata = cast(data);
                VinterfaceStatus old = cdata.getOldValue();
                VinterfaceStatus vifs = cdata.getValue();
                changed = !(old.getState() == vifs.getState() &&
                            old.getEntityState() == vifs.getEntityState() &&
                            Objects.equals(old.getMappedPort(),
                                           vifs.getMappedPort()));
            } else {
                // No need to call onUpdated() if info log is disabled.
                changed = false;
            }

            return changed;
        }
    }

    /**
     * Construct a new instance.
     */
    VinterfaceListener() {
        super(Vinterface.class, true);
    }

    // VNodeLogListener

    /**
     * {@inheritDoc}
     */
    @Override
    String getDescription(InstanceIdentifier<Vinterface> path) {
        return VInterfaceIdentifier.DESCRIPTION;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    String toString(Vinterface value) {
        return toString(value.getVinterfaceConfig());
    }

    // VNodeChangeListener

    /**
     * Determine whether the tatget virtual node should be treated as a
     * leaf node or not.
     *
     * <p>
     *   This method always returns {@code false} because vinterface
     *   contains children such as vinterface-config.
     * </p>
     *
     * @return  {@code false}.
     */
    @Override
    boolean isLeafNode() {
        return false;
    }
}
