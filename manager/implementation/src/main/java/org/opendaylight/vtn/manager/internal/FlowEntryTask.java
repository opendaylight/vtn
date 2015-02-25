/*
 * Copyright (c) 2013-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal;

import org.opendaylight.controller.forwardingrulesmanager.FlowEntry;

/**
 * This class implements flow programming task which modifies a single flow
 * entry.
 */
public abstract class FlowEntryTask extends FlowModTask {
    /**
     * A flow entry to be modified.
     */
    private final FlowEntry  flowEntry;

    /**
     * Construct a new task.
     *
     * @param mgr   VTN Manager service.
     * @param ctx   MD-SAL datastore transaction context.
     * @param fent  The target flow entry.
     */
    protected FlowEntryTask(VTNManagerImpl mgr, TxContext ctx,
                            FlowEntry fent) {
        super(mgr, ctx);
        flowEntry = fent;
    }

    /**
     * Return the target flow entry.
     *
     * @return  The target flow entry.
     */
    FlowEntry getFlowEntry() {
        return flowEntry;
    }
}
