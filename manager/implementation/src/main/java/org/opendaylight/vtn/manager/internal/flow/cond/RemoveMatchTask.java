/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.flow.cond;

import org.opendaylight.vtn.manager.internal.util.flow.cond.FlowCondUtils;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;
import org.opendaylight.vtn.manager.internal.util.tx.DeleteDataTask;

import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.vtn.flow.cond.config.VtnFlowMatch;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeName;

/**
 * {@code RemoveMatchTask} describes the MD-SAL datastore transaction task that
 * deletes the flow match configuration associated with the given index in the
 * flow condition.
 *
 * <p>
 *   A task corresponding to this task is used as a sub task for
 *   {@link RemoveFlowMatchTask}.
 * </p>
 *
 * @see RemoveFlowMatchTask
 */
public final class RemoveMatchTask extends DeleteDataTask<VtnFlowMatch> {
    /**
     * The index that specifies the flow match to be removed.
     */
    private final Integer  index;

    /**
     * Construct a new instance.
     *
     * @param vname  A {@link VnodeName} instance that contains the name of the
     *               target flow condition.
     * @param idx    An {@link Integer} instance which specifies the flow match
     *               in the given flow condition.
     * @throws RpcException
     *    The given match index is invalid.
     */
    RemoveMatchTask(VnodeName vname, Integer idx) throws RpcException {
        super(LogicalDatastoreType.OPERATIONAL,
              FlowCondUtils.getIdentifier(vname, idx));
        index = idx;
    }

    /**
     * Return the index number for the target flow match.
     *
     * @return  An {@link Integer} instance which represents the index number
     *          for the target flow match.
     */
    public Integer getIndex() {
        return index;
    }
}
