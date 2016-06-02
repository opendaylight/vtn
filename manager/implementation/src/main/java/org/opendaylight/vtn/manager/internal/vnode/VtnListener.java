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

import java.util.Objects;

import org.opendaylight.vtn.manager.internal.util.ChangedData;
import org.opendaylight.vtn.manager.internal.util.IdentifiedData;
import org.opendaylight.vtn.manager.internal.util.MiscUtils;
import org.opendaylight.vtn.manager.internal.util.flow.FlowUtils;
import org.opendaylight.vtn.manager.internal.util.pathmap.PathMapUtils;
import org.opendaylight.vtn.manager.internal.util.vnode.VNodeType;
import org.opendaylight.vtn.manager.internal.util.vnode.VTenantIdentifier;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathmap.rev150328.vtn.path.map.list.VtnPathMap;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.vtn.info.VtenantConfig;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.vtns.Vtn;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;

/**
 * Data change listener that listens changes of VTN configuration.
 */
final class VtnListener extends VNodeChangeListener<Vtn> {
    /**
     * Convert the given vtenant-config into a string.
     *
     * @param value  A {@link VtenantConfig} instance.
     * @return  A string that represents the specified data.
     */
    private static String toString(VtenantConfig value) {
        StringBuilder builder = new StringBuilder();
        String desc = value.getDescription();
        if (desc != null) {
            builder.append(TAG_DESC).append(desc);
        }
        FlowUtils.setDescription(builder, value, LOG_SEPARATOR);

        return builder.toString();
    }

    /**
     * Data change listener for vtenant-config.
     */
    static final class VtenantConfigListener
        extends VNodeLogListener<VtenantConfig> {
        /**
         * Construct a new instance.
         */
        VtenantConfigListener() {
            super(VtenantConfig.class, true);
        }

        // VNodeLogListener

        /**
         * {@inheritDoc}
         */
        @Override
        String getDescription(InstanceIdentifier<VtenantConfig> path) {
            return VNodeType.VTN.toString();
        }

        /**
         * {@inheritDoc}
         */
        @Override
        String toString(VtenantConfig value) {
            return VtnListener.toString(value);
        }

        /**
         * This method does nothing.
         *
         * <p>
         *   Creation event will be handled by {@link VtnListener}.
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
         *   Removal event will be handled by {@link VtnListener}.
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
            ChangedData<VtenantConfig> cdata = cast(data);
            VtenantConfig old = cdata.getOldValue();
            VtenantConfig vtnc = cdata.getValue();
            return !(Objects.equals(old.getDescription(),
                                    vtnc.getDescription()) &&
                     FlowUtils.equalsFlowTimeoutConfig(old, vtnc));
        }
    }

    /**
     * Data change listener for vtn-path-map.
     */
    static final class VtnPathMapListener
        extends VNodeChangeListener<VtnPathMap> {
        /**
         * Construct a new instance.
         */
        VtnPathMapListener() {
            super(VtnPathMap.class, true);
        }

        // VNodeChangeListener

        /**
         * {@inheritDoc}
         */
        @Override
        void onCreated(VTenantChange ectx, IdentifiedData<?> data) {
            PathMapUtils.log(LOG, data, VtnUpdateType.CREATED);
        }

        /**
         * {@inheritDoc}
         */
        @Override
        void onUpdated(VTenantChange ectx, ChangedData<?> data) {
            PathMapUtils.log(LOG, data);
        }

        /**
         * {@inheritDoc}
         */
        @Override
        void onRemoved(VTenantChange ectx, IdentifiedData<?> data) {
            PathMapUtils.log(LOG, data, VtnUpdateType.REMOVED);
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
            ChangedData<VtnPathMap> cdata = cast(data);
            return PathMapUtils.isUpdated(
                cdata.getOldValue(), cdata.getValue());
        }
    }

    /**
     * Construct a new instance.
     */
    VtnListener() {
        super(Vtn.class, false);
    }

    /**
     * Record a log message that indicates the VTN has been created or removed.
     *
     * @param vtn   A {@link Vtn} instance.
     * @param type  {@link VtnUpdateType#CREATED} on created.
     *              {@link VtnUpdateType#REMOVED} on removed.
     */
    private void log(Vtn vtn, VtnUpdateType type) {
        if (LOG.isInfoEnabled()) {
            VTenantIdentifier vtnId = new VTenantIdentifier(vtn.getName());
            LOG.info("{}: VTN has been {}: value={{}}",
                     vtnId, MiscUtils.toLowerCase(type),
                     toString(vtn.getVtenantConfig()));
        }
    }

    /**
     * Record the given VTN tree for saving configuration.
     *
     * @param ectx     A {@link VTenantChange} instance.
     * @param data     A {@link IdentifiedData} instance that contains a
     *                 VTN data tree.
     * @param created  {@code true} indicates that the given VTN has been
     *                 created.
     * @return  The VTN in {@code data}.
     */
    private Vtn onChanged(VTenantChange ectx, IdentifiedData<?> data,
                          boolean created) {
        IdentifiedData<Vtn> cdata = cast(data);
        Vtn vtn = cdata.getValue();
        String name = vtn.getName().getValue();
        ectx.addUpdatedVtn(name, vtn, created);
        return vtn;
    }

    // VNodeChangeListener

    /**
     * {@inheritDoc}
     */
    @Override
    void onCreated(VTenantChange ectx, IdentifiedData<?> data) {
        Vtn vtn = onChanged(ectx, data, true);
        log(vtn, VtnUpdateType.CREATED);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    void onUpdated(VTenantChange ectx, ChangedData<?> data) {
        onChanged(ectx, data, false);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    void onRemoved(VTenantChange ectx, IdentifiedData<?> data) {
        IdentifiedData<Vtn> cdata = cast(data);
        Vtn vtn = cdata.getValue();
        String name = vtn.getName().getValue();
        ectx.addRemoved(name);
        log(vtn, VtnUpdateType.REMOVED);
    }

    /**
     * Determine whether the specified data object was updated or not.
     *
     * <p>
     *   This method always returns {@code true}.
     *   {@link VTenantChange} will save the VTN configuration only if
     *   needed.
     * </p>
     *
     * @param data  A {@link ChangedData} instance that contains values before
     *              and after modification.
     * @return  {@code true}.
     */
    @Override
    boolean isUpdated(ChangedData<?> data) {
        return true;
    }

    /**
     * Determine whether the tatget virtual node should be treated as a
     * leaf node or not.
     *
     * <p>
     *   This method always returns {@code false} because vtn contains
     *   other virtual nodes.
     * </p>
     *
     * @return  {@code false}.
     */
    @Override
    boolean isLeafNode() {
        return false;
    }
}
