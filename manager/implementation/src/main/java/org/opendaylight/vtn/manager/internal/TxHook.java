/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal;

/**
 * {@code TxHook} describes a hook for the MD-SAL datastore transaction.
 */
public interface TxHook {
    /**
     * Run this hook associated with the MD-SAL datastore transaction.
     *
     * @param ctx   A runtime context for transaction task.
     * @param task  The current transaction task.
     */
    void run(TxContext ctx, TxTask<?> task);

    /**
     * Return an integer that determines the order of execution.
     *
     * <p>
     *   Registered hooks are executed in ascending order of order values.
     * </p>
     *
     * @return  An integer that determines the order of execution.
     */
    int getOrder();
}
