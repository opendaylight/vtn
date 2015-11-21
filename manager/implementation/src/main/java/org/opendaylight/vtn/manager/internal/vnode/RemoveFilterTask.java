/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.vnode;

import org.opendaylight.vtn.manager.internal.util.tx.DeleteDataTask;

import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.flow.filter.list.VtnFlowFilter;

/**
 * {@code RemoveFilterTask} describes the MD-SAL datastore transaction task
 * that deletes the flow filter configuration associated with the given index.
 *
 * <p>
 *   A task corresponding to this task is used as a sub task for
 *   {@link RemoveFlowFilterTask}.
 * </p>
 *
 * @see RemoveFlowFilterTask
 */
public final class RemoveFilterTask extends DeleteDataTask<VtnFlowFilter> {
    /**
     * The index that specifies the flow filter to be removed.
     */
    private final Integer  index;

    /**
     * Construct a new instance.
     *
     * @param path  Path to the target flow filter.
     * @param idx   An {@link Integer} instance which specifies the flow
     *              filter to be removed.
     */
    RemoveFilterTask(InstanceIdentifier<VtnFlowFilter> path, Integer idx) {
        super(LogicalDatastoreType.OPERATIONAL, path);
        index = idx;
    }

    /**
     * Return the index number for the target flow filter.
     *
     * @return  An {@link Integer} instance which represents the index number
     *          for the target flow filter.
     */
    public Integer getIndex() {
        return index;
    }
}
