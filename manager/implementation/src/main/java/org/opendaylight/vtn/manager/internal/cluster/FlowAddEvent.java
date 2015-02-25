/*
 * Copyright (c) 2013-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.cluster;

import java.util.List;

import org.opendaylight.controller.forwardingrulesmanager.FlowEntry;

import org.opendaylight.vtn.manager.internal.ClusterFlowAddTask;
import org.opendaylight.vtn.manager.internal.ClusterFlowModTask;
import org.opendaylight.vtn.manager.internal.TxContext;
import org.opendaylight.vtn.manager.internal.VTNManagerImpl;

/**
 * {@code FlowRemoveEvent} describes an cluster event object which directs
 * remote cluster node to install the given flow entries.
 *
 * <p>
 *   Although this class is public to other packages, this class does not
 *   provide any API. Applications other than VTN Manager must not use this
 *   class.
 * </p>
 */
public class FlowAddEvent extends FlowModEvent {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = 8331943297735778886L;

    /**
     * Construct a new flow install event.
     *
     * @param entries  List of flow entries to be installed.
     */
    public FlowAddEvent(List<FlowEntry> entries) {
        super(entries);
    }

    /**
     * Create a flow mod task to modify the given flow entry.
     *
     * @param mgr   VTN Manager service.
     * @param ctx   MD-SAL datastore transaction context.
     * @param fent  A flow entry to be modified.
     * @return  A flow mod task to add the given flow entry.
     */
    @Override
    protected ClusterFlowModTask createTask(VTNManagerImpl mgr, TxContext ctx,
                                            FlowEntry fent) {
        return new ClusterFlowAddTask(mgr, ctx, fent);
    }
}
