/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.routing;

import org.opendaylight.vtn.manager.internal.util.pathpolicy.PathPolicyUtils;
import org.opendaylight.vtn.manager.internal.util.tx.DeleteDataTask;

import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.vtn.path.policy.config.VtnPathCost;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnPortDesc;

/**
 * {@code RemoveCostTask} describes the MD-SAL transaction task that deletes
 * the link cost configuration for the specified switch port.
 *
 * <p>
 *   A task corresponding to this task is used as a sub task for
 *   {@link RemovePathCostTask}.
 * </p>
 *
 * @see RemovePathCostTask
 */
public final class RemoveCostTask extends DeleteDataTask<VtnPathCost> {
    /**
     * The switch port descriptor.
     */
    private final VtnPortDesc  portDesc;

    /**
     * Construct a new instance.
     *
     * @param id     The identifier of the target path policy.
     * @param vdesc  A {@link VtnPortDesc} instance which specifies the
     *               switch port location.
     */
    RemoveCostTask(Integer id, VtnPortDesc vdesc) {
        super(LogicalDatastoreType.OPERATIONAL,
              PathPolicyUtils.getIdentifier(id, vdesc));
        portDesc = vdesc;
    }

    /**
     * Return the target switch port descriptor.
     *
     * @return  A {@link VtnPortDesc} instance.
     */
    public VtnPortDesc getPortDesc() {
        return portDesc;
    }
}
