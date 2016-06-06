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
import static org.opendaylight.vtn.manager.internal.vnode.VTenantManager.TAG_STATE;

import java.util.Objects;

import org.opendaylight.vtn.manager.internal.util.ChangedData;
import org.opendaylight.vtn.manager.internal.util.IdentifiedData;
import org.opendaylight.vtn.manager.internal.util.MiscUtils;
import org.opendaylight.vtn.manager.internal.util.vnode.VBridgeIdentifier;
import org.opendaylight.vtn.manager.internal.util.vnode.VNodeIdentifier;
import org.opendaylight.vtn.manager.internal.util.vnode.VNodeType;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.vtns.Vtn;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeName;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.bridge.status.FaultedPaths;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.port.mappable.bridge.BridgeStatus;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.vbridge.info.VbridgeConfig;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.vbridge.list.Vbridge;

/**
 * Data change listener that listens changes of vBridge configuration.
 */
final class VbridgeListener extends VNodeChangeListener<Vbridge> {
    /**
     * Return the identifier for the vBridge in the specified data.
     *
     * @param data  An {@link IdentifiedData} instance.
     *              Note that this must contain a vBridge or a child node
     *              inside vBridge.
     * @return  A {@link VBridgeIdentifier} instance.
     */
    private static VBridgeIdentifier getVBridgeIdentifier(
        IdentifiedData<?> data) {
        InstanceIdentifier<?> path = data.getIdentifier();
        VnodeName tname = path.firstKeyOf(Vtn.class).getName();
        VnodeName bname = path.firstKeyOf(Vbridge.class).getName();
        return new VBridgeIdentifier(tname, bname);
    }

    /**
     * Convert the given vbridge-config into a string.
     *
     * @param value  A {@link VbridgeConfig} instance.
     * @return  A string that represents the specified data.
     */
    private static String toString(VbridgeConfig value) {
        String desc = value.getDescription();
        StringBuilder builder = new StringBuilder();
        if (desc != null) {
            builder.append(TAG_DESC).append(desc).append(LOG_SEPARATOR);
        }

        return builder.append("age-interval=").append(value.getAgeInterval()).
            toString();
    }

    /**
     * Data change listener for vbridge-config.
     */
    static final class VbridgeConfigListener
        extends VNodeChangeListener<VbridgeConfig> {
        /**
         * Construct a new instance.
         */
        VbridgeConfigListener() {
            super(VbridgeConfig.class, true);
        }

        // VNodeChangeListener

        /**
         * Invoked when a vBridge configuration has been updated.
         *
         * @param ectx  A {@link VTenantChange} instance.
         * @param data  A {@link ChangedData} instance which contains the
         *              changed data.
         */
        @Override
        void onUpdated(VTenantChange ectx, ChangedData<?> data) {
            // Record a log message.
            ChangedData<VbridgeConfig> cdata = cast(data);
            VBridgeIdentifier ident =
                VbridgeListener.getVBridgeIdentifier(cdata);
            VbridgeConfig vbrc = cdata.getValue();
            if (LOG.isInfoEnabled()) {
                VbridgeConfig old = cdata.getOldValue();
                LOG.info("{}: vBridge has been changed: old={{}}, new={{}}",
                         ident, VbridgeListener.toString(old),
                         VbridgeListener.toString(vbrc));
            }

            // Update the entity of the vBridge.
            ectx.updateBridge(ident, vbrc);
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
            ChangedData<VbridgeConfig> cdata = cast(data);
            VbridgeConfig old = cdata.getOldValue();
            VbridgeConfig vbrc = cdata.getValue();
            return !(Objects.equals(old.getDescription(),
                                    vbrc.getDescription()) &&
                     Objects.equals(old.getAgeInterval(),
                                    vbrc.getAgeInterval()));
        }

        /**
         * This method does nothing.
         *
         * <p>
         *   Creation event will be handled by {@link VbridgeListener}.
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
         *   Removal event will be handled by {@link VbridgeListener}.
         * </p>
         *
         * @param ectx  Unused.
         * @param data  Unused.
         */
        @Override
        void onRemoved(VTenantChange ectx, IdentifiedData<?> data) {
        }
    }

    /**
     * Data change listener for bridge-status.
     */
    static final class BridgeStatusListener
        extends VNodeChangeListener<BridgeStatus> {
        /**
         * Construct a new instance.
         */
        BridgeStatusListener() {
            super(BridgeStatus.class, false);
        }

        /**
         * Return a string that represents the contents of bridge-status.
         *
         * @param value  An instance of {@link BridgeStatus}.
         * @return  A string representation of the given value.
         */
        private String toString(BridgeStatus value) {
            // No need to log faulted-paths.
            return TAG_STATE + value.getState() + ", path-faults=" +
                value.getPathFaults();
        }

        // VNodeChangeListener

        /**
         * {@inheritDoc}
         */
        @Override
        void onCreated(VTenantChange ectx, IdentifiedData<?> data) {
            if (LOG.isInfoEnabled()) {
                IdentifiedData<BridgeStatus> cdata = cast(data);
                VNodeIdentifier<?> ident =
                    getVNodeIdentifier(cdata.getIdentifier());
                VNodeType type = ident.getType();
                BridgeStatus value = cdata.getValue();
                LOG.info("{}: Initial {} status: {}",
                         ident, type.toString(), toString(value));
            }
        }

        /**
         * {@inheritDoc}
         */
        @Override
        void onUpdated(VTenantChange ectx, ChangedData<?> data) {
            ChangedData<BridgeStatus> cdata = cast(data);
            VNodeIdentifier<?> ident =
                getVNodeIdentifier(cdata.getIdentifier());

            // Log resolved path faults.
            boolean changed = false;
            for (FaultedPaths fp: ectx.getResolvedPathFaults(ident)) {
                LOG.info("{}: Path fault has been resolved: {} -> {}",
                         ident, MiscUtils.getValue(fp.getSource()),
                         MiscUtils.getValue(fp.getDestination()));
                changed = true;
            }

            VNodeType type = ident.getType();
            BridgeStatus value = cdata.getValue();
            BridgeStatus old = cdata.getOldValue();
            if (changed || old.getState() != value.getState() ||
                old.getPathFaults() != value.getPathFaults()) {
                LOG.info("{}: {} status has been changed: old={{}}, new={{}}",
                         ident, type.toString(), toString(old),
                         toString(value));
            }
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
         * <p>
         *   This method returns {@code true} if info log is enabled.
         *   Difference of the bridge status will be checked by
         *   {@link #onUpdated(VTenantChange, ChangedData)}.
         * </p>
         *
         * @param data  A {@link ChangedData} instance that contains values
         *              before and after modification.
         * @return  {@code true} if the target data was updated.
         *          {@code false} otherwise.
         */
        @Override
        boolean isUpdated(ChangedData<?> data) {
            return LOG.isInfoEnabled();
        }

        /**
         * Determine whether the tatget virtual node should be treated as a
         * leaf node or not.
         *
         * <p>
         *   This method always returns {@code false} because bridge-status
         *   contains faulted-paths.
         * </p>
         *
         * @return  {@code false}.
         */
        @Override
        boolean isLeafNode() {
            return false;
        }
    }

    /**
     * Data change listener for faulted-paths.
     */
    static final class FaultedPathsListener
        extends VNodeChangeListener<FaultedPaths> {
        /**
         * Construct a new instance.
         */
        FaultedPathsListener() {
            super(FaultedPaths.class, false);
        }

        // VNodeChangeListener

        /**
         * {@inheritDoc}
         */
        @Override
        void onCreated(VTenantChange ectx, IdentifiedData<?> data) {
            IdentifiedData<FaultedPaths> cdata = cast(data);
            VNodeIdentifier<?> ident =
                getVNodeIdentifier(cdata.getIdentifier());
            FaultedPaths value = cdata.getValue();
            LOG.warn("{}: Path fault: {} -> {}", ident,
                     MiscUtils.getValue(value.getSource()),
                     MiscUtils.getValue(value.getDestination()));
        }

        /**
         * {@inheritDoc}
         */
        @Override
        void onRemoved(VTenantChange ectx, IdentifiedData<?> data) {
            // Record the given path fault into VTenantChange.
            // It will be logged unless the vBridge was removed.
            IdentifiedData<FaultedPaths> cdata = cast(data);
            VNodeIdentifier<?> ident =
                getVNodeIdentifier(cdata.getIdentifier());
            ectx.addResolvedPathFault(ident, cdata.getValue());
        }
    }

    /**
     * Construct a new instance.
     */
    VbridgeListener() {
        super(Vbridge.class, true);
    }

    /**
     * Handle a vBridge creation or removal event.
     *
     * @param ectx  A {@link VTenantChange} instance.
     * @param data  A {@link IdentifiedData} instance.
     * @param type   {@link VtnUpdateType#CREATED} on added,
     *               {@link VtnUpdateType#REMOVED} on removed.
     */
    private void onNotified(VTenantChange ectx, IdentifiedData<?> data,
                            VtnUpdateType type) {
        IdentifiedData<Vbridge> cdata = cast(data);
        VBridgeIdentifier ident = getVBridgeIdentifier(cdata);
        VbridgeConfig vbrc = cdata.getValue().getVbridgeConfig();

        if (LOG.isInfoEnabled()) {
            // Record a log message.
            LOG.info("{}: vBridge has been {}: config={{}}",
                     ident, MiscUtils.toLowerCase(type), toString(vbrc));
        }

        if (type == VtnUpdateType.CREATED) {
            // Create the entity of the vBridge.
            ectx.updateBridge(ident, vbrc);
        } else {
            // Remove the entity of the vBridge.
            ectx.removeBridge(ident);
        }
    }

    // VNodeChangeListener

    /**
     * {@inheritDoc}
     */
    @Override
    void onCreated(VTenantChange ectx, IdentifiedData<?> data) {
        onNotified(ectx, data, VtnUpdateType.CREATED);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    void onRemoved(VTenantChange ectx, IdentifiedData<?> data) {
        onNotified(ectx, data, VtnUpdateType.REMOVED);
    }

    /**
     * Determine whether the tatget virtual node should be treated as a
     * leaf node or not.
     *
     * <p>
     *   This method always returns {@code false} because vbridge
     *   contains children such as vbridge-config.
     * </p>
     *
     * @return  {@code false}.
     */
    @Override
    boolean isLeafNode() {
        return false;
    }
}

