/*
 * Copyright (c) 2013-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal;

import org.opendaylight.vtn.manager.internal.cluster.FlowModResult;
import org.opendaylight.vtn.manager.internal.cluster.FlowModResultEvent;

import org.opendaylight.controller.connectionmanager.IConnectionManager;
import org.opendaylight.controller.forwardingrulesmanager.FlowEntry;
import org.opendaylight.controller.sal.connection.ConnectionLocality;

/**
 * This class implements flow programming task which modifies a flow entry
 * requested by a remote cluster node.
 */
public abstract class ClusterFlowModTask extends FlowEntryTask {
    /**
     * Construct a new task.
     *
     * @param mgr   VTN Manager service.
     * @param ctx   MD-SAL datastore transaction context.
     * @param fent  A flow entry to be modified.
     */
    ClusterFlowModTask(VTNManagerImpl mgr, TxContext ctx, FlowEntry fent) {
        super(mgr, ctx, fent);
    }

    /**
     * Send result of flow modification request issued by remote cluster node.
     *
     * @param result  The result of flow modification.
     */
    void sendRemoteFlowModResult(FlowModResult result) {
        String name = getFlowEntry().getFlowName();
        postEvent(new FlowModResultEvent(name, result));
    }

    /**
     * Execute flow modification task requested by a remote cluster node.
     *
     * @return  {@code true} is returned if this task completed successfully.
     *          Otherwise {@code false} is returned.
     */
    @Override
    protected boolean execute() {
        IConnectionManager cnm = getVTNManager().getConnectionManager();
        boolean ret;
        FlowModResult result;
        FlowEntry fent = getFlowEntry();
        ConnectionLocality cl = cnm.getLocalityStatus(fent.getNode());
        if (cl == ConnectionLocality.LOCAL) {
            try {
                ret = modifyFlow();
                result = (ret)
                    ? FlowModResult.SUCCEEDED : FlowModResult.FAILED;
            } catch (Exception e) {
                ret = false;
                result = FlowModResult.FAILED;
            }
        } else {
            ret = true;
            result = FlowModResult.IGNORED;
        }
        sendRemoteFlowModResult(result);

        return ret;
    }

    /**
     * Modify flow entry.
     *
     * @return  {@code true} is returned only if flow modification was
     *          completed successfully.
     */
    protected abstract boolean modifyFlow();
}
