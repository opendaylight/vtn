/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal;

import org.opendaylight.vtn.manager.VTNException;

/**
 * {@code FlowRemover} provides interfaces to be implemented by classes which
 * removes VTN data flow.
 */
public interface FlowRemover {
    /**
     * Remove VTN data flows from the MD-SAL datastore.
     *
     * @param ctx    A runtime context for transaction task.
     * @return  A {@link RemovedFlows} instance which represents removed flow
     *          entries.
     * @throws VTNException  An error occurred.
     */
    RemovedFlows removeDataFlow(TxContext ctx) throws VTNException;

    /**
     * Return a brief description about this flow remover.
     *
     * @return  A brief description about this flow remover.
     */
    String getDescription();
}
