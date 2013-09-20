/*
 * Copyright (c) 2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import org.opendaylight.controller.forwardingrulesmanager.FlowEntry;

/**
 * This class implements flow programming task which uninstalls a flow entry
 * requested by a remote cluster node.
 *
 * <p>
 *   If the given task is associated with a local node, this task uninstalls
 *   the given flow entry and sends result of uninstallation to remote cluster
 *   node.
 * </p>
 */
public class ClusterFlowRemoveTask extends ClusterFlowModTask {
    /**
     * Logger instance.
     */
    private final static Logger  LOG =
        LoggerFactory.getLogger(ClusterFlowRemoveTask.class);

    /**
     * Construct a new task.
     *
     * @param mgr    VTN Manager service.
     * @param fent   A flow entry to be uninstalled.
     */
    public ClusterFlowRemoveTask(VTNManagerImpl mgr, FlowEntry fent) {
        super(mgr, fent);
    }

    /**
     * Modify flow entry.
     *
     * @return  {@code true} is returned only if flow modification was
     *          completed successfully.
     */
    @Override
    protected boolean modifyFlow() {
        return uninstallLocal(flowEntry);
    }

    /**
     * Return a logger object for this class.
     *
     * @return  A logger object.
     */
    @Override
    protected Logger getLogger() {
        return LOG;
    }
}
