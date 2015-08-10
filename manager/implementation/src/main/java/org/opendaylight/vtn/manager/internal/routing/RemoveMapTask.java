/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.routing;

import org.opendaylight.vtn.manager.internal.util.tx.DeleteDataTask;

import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathmap.rev150328.vtn.path.map.list.VtnPathMap;

/**
 * {@code RemoveMapTask} describes the MD-SAL datastore transaction task that
 * deletes the path map configuration associated with the given index.
 *
 * <p>
 *   A task corresponding to this task is used as a sub task for
 *   {@link RemovePathMapTask}.
 * </p>
 *
 * @see RemovePathMapTask
 */
public final class RemoveMapTask extends DeleteDataTask<VtnPathMap> {
    /**
     * The index that specifies the path map to be removed.
     */
    private final Integer  index;

    /**
     * Construct a new instance.
     *
     * @param path  Path to the target path map.
     * @param idx   An {@link Integer} instance which specifies the path map.
     */
    RemoveMapTask(InstanceIdentifier<VtnPathMap> path, Integer idx) {
        super(LogicalDatastoreType.OPERATIONAL, path);
        index = idx;
    }

    /**
     * Return the index number for the target path map.
     *
     * @return  An {@link Integer} instance which represents the index number
     *          for the target path map.
     */
    public Integer getIndex() {
        return index;
    }
}
