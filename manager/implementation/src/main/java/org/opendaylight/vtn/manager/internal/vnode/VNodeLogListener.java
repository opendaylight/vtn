/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.vnode;

import org.slf4j.Logger;

import org.opendaylight.vtn.manager.internal.util.ChangedData;
import org.opendaylight.vtn.manager.internal.util.IdentifiedData;
import org.opendaylight.vtn.manager.internal.util.MiscUtils;

import org.opendaylight.yangtools.yang.binding.DataObject;
import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;

/**
 * {@code VNodeLogListener} describes a data change listener that logs
 * changes against virtual node in the VTN data model tree.
 *
 * @param <T>  The type of the virtual node.
 */
abstract class VNodeLogListener<T extends DataObject>
    extends VNodeChangeListener<T> {
    /**
     * {@code VNodeLogListener.Simple} describes a data change listener that
     * logs changes against virtual node in the VTN data model tree.
     *
     * <p>
     *   Unlike {@link VNodeLogListener}, this listener does not log old data
     *   even on changed event.
     * </p>
     *
     * @param <T>  The type of the virtual node.
     */
    abstract static class Simple<T extends DataObject>
        extends VNodeLogListener<T> {
        /**
         * Construct a new instance.
         *
         * @param type    A class which indicates the target data type.
         * @param config  {@code true} means that this data model represents
         *                configuration data.
         */
        Simple(Class<T> type, boolean config) {
            super(type, config);
        }

        // VNodeChangeListener

        /**
         * {@inheritDoc}
         */
        @Override
        void onUpdated(VTenantChange ectx, Logger logger, ChangedData<?> data) {
            onChanged(logger, data, VtnUpdateType.CHANGED);
        }
    }

    /**
     * Construct a new instance.
     *
     * @param type    A class which indicates the target data type.
     * @param config  {@code true} means that this data model represents
     *                configuration data.
     */
    VNodeLogListener(Class<T> type, boolean config) {
        super(type, config);
    }

    /**
     * Invoked when a virtual node has been updated.
     *
     * @param logger  A logger instance.
     * @param data    An {@link IdentifiedData} instance which contains the
     *                created data.
     * @param type    {@link VtnUpdateType#CREATED} on added,
     *                {@link VtnUpdateType#CHANGED} on changed.
     *                {@link VtnUpdateType#REMOVED} on removed.
     */
    protected void onChanged(Logger logger, IdentifiedData<?> data,
                             VtnUpdateType type) {
        if (logger.isInfoEnabled()) {
            IdentifiedData<T> cdata = cast(data);
            InstanceIdentifier<T> path = cdata.getIdentifier();
            T value = cdata.getValue();
            logger.info("{}: {} has been {}: value={{}}",
                        getVNodeIdentifier(path), getDescription(path),
                        MiscUtils.toLowerCase(type), toString(value));
        }
    }

    /**
     * Return a brief description about the data model.
     *
     * @param path  Path to the data model.
     * @return  A brief description about the specified data model.
     */
    abstract String getDescription(InstanceIdentifier<T> path);

    /**
     * Convert the given data into a string for logging.
     *
     * @param value  A data object.
     * @return  A string that represents the given data.
     */
    abstract String toString(T value);

    // VNodeChangeListener

    /**
     * {@inheritDoc}
     */
    @Override
    void onCreated(VTenantChange ectx, Logger logger, IdentifiedData<?> data) {
        onChanged(logger, data, VtnUpdateType.CREATED);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    void onUpdated(VTenantChange ectx, Logger logger, ChangedData<?> data) {
        if (logger.isInfoEnabled()) {
            ChangedData<T> cdata = cast(data);
            InstanceIdentifier<T> path = cdata.getIdentifier();
            T value = cdata.getValue();
            T old = cdata.getOldValue();
            logger.info("{}: {} has been changed: old={{}}, new={{}}",
                        getVNodeIdentifier(path), getDescription(path),
                        toString(old), toString(value));
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    void onRemoved(VTenantChange ectx, Logger logger, IdentifiedData<?> data) {
        onChanged(logger, data, VtnUpdateType.REMOVED);
    }
}
