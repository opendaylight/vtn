/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.routing;

import org.opendaylight.vtn.manager.internal.util.tx.PutDataTask;

import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathmap.rev150328.vtn.path.map.list.VtnPathMap;

/**
 * {@code SetMapTask} describes the MD-SAL datastore transaction task that
 * associates the path map with the specified index.
 *
 * <p>
 *   A task corresponding to this task is used as a sub task for
 *   {@link SetPathMapTask}.
 * </p>
 *
 * @see  SetPathMapTask
 */
public final class SetMapTask extends PutDataTask<VtnPathMap> {
    /**
     * Construct a new instance.
     *
     * @param path  Path to the target path map.
     * @param vpm   A {@link VtnPathMap} instance to be set.
     */
    SetMapTask(InstanceIdentifier<VtnPathMap> path, VtnPathMap vpm) {
        super(LogicalDatastoreType.OPERATIONAL, path, vpm, true);
    }

    /**
     * Return the target map index.
     *
     * @return  A {@link Integer} instance that represents the target map
     *          index.
     */
    public Integer getIndex() {
        return getDataObject().getIndex();
    }

    // PutDataTask

    /**
     * {@inheritDoc}
     */
    @Override
    protected boolean fixMissingParents() {
        // Create the container for path map list if not present.
        return true;
    }
}
