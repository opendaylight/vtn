/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.flow.cond;

import org.opendaylight.vtn.manager.internal.util.tx.PutDataTask;

import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.vtn.flow.cond.config.VtnFlowMatch;

/**
 * {@code SetMatchTask} describes the MD-SAL datastore transaction task that
 * associates the flow match with the specified index in a flow condition.
 *
 * <p>
 *   A task corresponding to this task is used as a sub task for
 *   {@link SetFlowMatchTask}.
 * </p>
 *
 * @see  SetFlowMatchTask
 */
public final class SetMatchTask extends PutDataTask<VtnFlowMatch> {
    /**
     * Construct a new instance.
     *
     * @param path  Path to the target flow match.
     * @param vfm   A {@link VtnFlowMatch} instance to be set.
     */
    SetMatchTask(InstanceIdentifier<VtnFlowMatch> path, VtnFlowMatch vfm) {
        super(LogicalDatastoreType.OPERATIONAL, path, vfm, true);
    }

    /**
     * Return the target match index.
     *
     * @return  A {@link Integer} instance that represents the target match
     *          index.
     */
    public Integer getIndex() {
        return getDataObject().getIndex();
    }
}
