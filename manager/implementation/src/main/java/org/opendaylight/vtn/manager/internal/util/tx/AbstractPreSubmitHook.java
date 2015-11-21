/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.tx;

import org.opendaylight.vtn.manager.internal.TxContext;
import org.opendaylight.vtn.manager.internal.TxHook;

/**
 * {@code AbstractPreSubmitHook} describes an abstract implementation of
 * a pre-submit hook for the MD-SAL datastore transaction.
 *
 * <p>
 *   {@link TxHook#run(TxContext, TxTask)} on this instance will be invoked
 *   when the MD-SAL datastore transaction is going to be submitted.
 * </p>
 */
public abstract class AbstractPreSubmitHook implements TxHook {
    /**
     * Runtime context for MD-SAL datastore transaction.
     */
    private final TxContext  context;

    /**
     * Construct a new instance.
     *
     * @param ctx  A runtime context for transaction task.
     */
    protected AbstractPreSubmitHook(TxContext ctx) {
        context = ctx;

        // Register this instance as a pre-submit hook.
        ctx.addPreSubmitHook(this);
    }

    /**
     * Return the runtime context for the MD-SAL datastore transaction.
     *
     * @return  A {@link TxContext} instance.
     */
    public final TxContext getContext() {
        return context;
    }
}
