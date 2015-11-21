/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.tx;

import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.yangtools.yang.binding.DataObject;
import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

/**
 * An abstract implementation of
 * {@link org.opendaylight.vtn.manager.internal.TxTask} used to update
 * a data object in the MD-SAL datastore.
 *
 * @param <D>  The type of the target data object in MD-SAL datastore.
 * @param <V>  The type of the object to be returned by the task.
 */
public abstract class AbstractDataTask<D extends DataObject, V>
    extends AbstractRpcTask<V> {
    /**
     * The target type of the MD-SAL datastore.
     */
    private final LogicalDatastoreType  storeType;

    /**
     * The instance identifier that specifies the target data object
     * in the MD-SAL datastore.
     */
    private final InstanceIdentifier<D>  targetPath;

    /**
     * Construct a new instance.
     *
     * @param store  The target type of the MD-SAL datastore.
     * @param path   The path to the target object.
     */
    public AbstractDataTask(LogicalDatastoreType store,
                            InstanceIdentifier<D> path) {
        storeType = store;
        targetPath = path;
    }

    /**
     * Return the instance identifier that specifies the target data object
     * in the MD-SAL datastore.
     *
     * @return  An {@link InstanceIdentifier} instance.
     */
    public final InstanceIdentifier<D> getTargetPath() {
        return targetPath;
    }

    /**
     * Return the target type of the MD-SAL datastore.
     *
     * @return  A {@link LogicalDatastoreType} instance.
     */
    public final LogicalDatastoreType getDatastoreType() {
        return storeType;
    }
}
