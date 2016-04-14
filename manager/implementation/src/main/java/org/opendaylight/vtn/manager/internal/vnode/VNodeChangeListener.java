/*
 * Copyright (c) 2015, 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.vnode;

import org.opendaylight.vtn.manager.internal.util.ChangedData;
import org.opendaylight.vtn.manager.internal.util.IdentifiedData;
import org.opendaylight.vtn.manager.internal.util.vnode.VNodeIdentifier;
import org.opendaylight.vtn.manager.internal.util.vnode.VNodePathConverter;

import org.opendaylight.yangtools.yang.binding.DataObject;
import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

/**
 * {@code VNodeChangeListener} describes a data change listener that listens
 * changes against virtual node in the VTN data model tree.
 *
 * @param <T>  The type of the virtual node.
 */
abstract class VNodeChangeListener<T extends DataObject> {
    /**
     * A class which indicates the target data type.
     */
    private final Class<T>  targetType;

    /**
     * Determine whether this data model represents configuration data
     * or not.
     */
    private final boolean  configuration;

    /**
     * Construct a new instance.
     *
     * @param type    A class which indicates the target data type.
     * @param config  {@code true} means that this data model represents
     *                configuration data.
     */
    VNodeChangeListener(Class<T> type, boolean config) {
        targetType = type;
        configuration = config;
    }

    /**
     * Convert the given instance identifier into a {@link VNodeIdentifier}
     * instance.
     *
     * @param path  An instance identifier to be converted.
     * @return  A {@link VNodeIdentifier} instance.
     */
    static final VNodeIdentifier<?> getVNodeIdentifier(
        InstanceIdentifier<?> path) {
        return new VNodePathConverter(path).getIdentifier().
            getVNodeIdentifier();
    }

    /**
     * Determine whether this data model represents configuration data
     * or not.
     *
     * @return  {@code true} only if this data model represents configuration
     *          data.
     */
    final boolean isConfiguration() {
        return configuration;
    }

    /**
     * Cast the given data.
     *
     * @param data  An {@link IdentifiedData} instance which contains the
     *              created or removed data.
     * @return  Casted data.
     */
    final IdentifiedData<T> cast(IdentifiedData<?> data) {
        return data.checkType(targetType);
    }

    /**
     * Cast the given data.
     *
     * @param data    A {@link ChangedData} instance which contains the
     *                changed data.
     * @return  Casted data.
     */
    final ChangedData<T> cast(ChangedData<?> data) {
        return data.checkType(targetType);
    }

    /**
     * Invoked when a virtual node has been created.
     *
     * @param ectx  A {@link VTenantChange} instance.
     * @param data  An {@link IdentifiedData} instance which contains the
     *              created data.
     */
    abstract void onCreated(VTenantChange ectx, IdentifiedData<?> data);

    /**
     * Invoked when a virtual node has been updated.
     *
     * @param ectx  A {@link VTenantChange} instance.
     * @param data  A {@link ChangedData} instance which contains the
     *              changed data.
     */
    abstract void onUpdated(VTenantChange ectx, ChangedData<?> data);

    /**
     * Invoked when a virtual node has been removed.
     *
     * @param ectx  A {@link VTenantChange} instance.
     * @param data  An {@link IdentifiedData} instance which contains the
     *              removed data.
     */
    abstract void onRemoved(VTenantChange ectx, IdentifiedData<?> data);
}
