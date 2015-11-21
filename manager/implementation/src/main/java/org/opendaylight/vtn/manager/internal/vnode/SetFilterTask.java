/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.vnode;

import org.opendaylight.vtn.manager.internal.util.tx.PutDataTask;

import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.flow.filter.list.VtnFlowFilter;

/**
 * {@code SetFilterTask} describes the MD-SAL datastore transaction task that
 * associates the flow filter with the specified index in the specified
 * flow filter list.
 *
 * <p>
 *   A task corresponding to this task is used as a sub task for
 *   {@link SetFlowFilterTask}.
 * </p>
 *
 * @see  SetFlowFilterTask
 */
public final class SetFilterTask extends PutDataTask<VtnFlowFilter> {
    /**
     * Construct a new instance.
     *
     * @param path  Path to the target flow filter.
     * @param vff   A {@link VtnFlowFilter} instance to be set.
     */
    SetFilterTask(InstanceIdentifier<VtnFlowFilter> path, VtnFlowFilter vff) {
        super(LogicalDatastoreType.OPERATIONAL, path, vff, true);
    }

    /**
     * Return the target flow filter index.
     *
     * @return  A {@link Integer} instance that represents the target flow
     *          filter index.
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
        // Create the container for flow filter list if not present.
        return true;
    }
}
