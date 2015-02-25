/*
 * Copyright (c) 2013-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal;

import java.util.List;

import org.opendaylight.controller.forwardingrulesmanager.FlowEntry;

/**
 * {@code RemoteFlowModTask} is a base class if flow programming task which
 * directs remote cluster nodes to modify flow entries.
 */
public abstract class RemoteFlowModTask extends FlowModTask {
    /**
     * Construct a new task.
     *
     * @param mgr  VTN Manager service.
     * @param ctx  MD-SAL datastore transaction context.
     */
    protected RemoteFlowModTask(VTNManagerImpl mgr, TxContext ctx) {
        super(mgr, ctx);
    }

    /**
     * Modify flow entries controlled by remote cluster node.
     *
     * @param entries  A list of flow entries.
     * @param all      If {@code true} is passed, this method waits for all
     *                 requests of flow entry modification to complete.
     *                 If {@code false} is passed, this method returns
     *                 immediately when at least one modification failure
     *                 is detected.
     * @param limit    System absolute time in milliseconds which represents
     *                 deadline of the wait.
     * @return  {@code true} is returned only if all flow entries were
     *          successfully modified.
     */
    protected boolean modifyRemoteFlow(List<FlowEntry> entries, boolean all,
                                       long limit) {
        // Register flow modification request to VTN flow programmer.
        RemoteFlowRequest req = new RemoteFlowRequest(entries);
        VTNManagerImpl mgr = getVTNManager();
        mgr.addRemoteFlowRequest(req);

        try {
            // Issue flow modification request.
            sendRequest(entries);

            // Wait for completion of flow modification by remote cluster
            // nodes.
            boolean ret = req.getResultAbs(limit, all);
            if (!ret) {
                req.logError(getLogger());
            }
            return ret;
        } finally {
            mgr.removeRemoteFlowRequest(req);
        }
    }

    /**
     * Initiate request for flow modification to remote cluster nodes.
     *
     * @param entries  A list of flow entries to be modified.
     */
    protected abstract void sendRequest(List<FlowEntry> entries);
}
