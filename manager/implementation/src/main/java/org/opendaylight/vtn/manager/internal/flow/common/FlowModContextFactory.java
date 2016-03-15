/*
 * Copyright (c) 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.flow.common;

import org.opendaylight.vtn.manager.internal.util.concurrent.VTNThreadPool;

import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.service.rev130819.SalFlowService;

/**
 * Describes an interface for a factory class that instantiate
 * {@link FlowModContext}.
 *
 * @param <T>  The type of the value to be returned by the future associated
 *             with the flow modification context.
 * @param <D>  The type of the value to be returned by the MD-SAL DS
 *             transaction task.
 */
public interface FlowModContextFactory<T, D> {
    /**
     * Construct a new context for modifying VTN data flows.
     *
     * @param sfs     The MD-SAL flow service.
     * @param thread  A thread pool to run tasks for modifying flow entries.
     * @return  A new context for modifying VTN data flows.
     */
    FlowModContext<T, D> newContext(SalFlowService sfs, VTNThreadPool thread);
}
