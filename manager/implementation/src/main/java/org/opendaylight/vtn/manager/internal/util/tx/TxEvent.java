/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.tx;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.TxContext;
import org.opendaylight.vtn.manager.internal.VTNManagerProvider;

/**
 * A base class for event class delivered on a MD-SAL datastore transaction
 * queue.
 */
public abstract class TxEvent extends AbstractTxTask<Void> {
    /**
     * A runtime context for transaction task.
     */
    private TxContext  txContext;

    /**
     * Deliver this event on a MD-SAL datastore transaction queue.
     * @param ctx  A runtime context for transaction task.
     * @return  {@code null}.
     * @throws VTNException  An error occurred.
     */
    @Override
    protected Void execute(TxContext ctx) throws VTNException {
        txContext = ctx;
        notifyEvent();
        return null;
    }

    /**
     * Return a runtime context for transaction task.
     *
     * @return  A {@link TxContext} instance.
     */
    public final TxContext getTxContext() {
        return txContext;
    }

    /**
     * Notify this event to event listener.
     *
     * @throws VTNException  An error occurred.
     */
    protected abstract void notifyEvent() throws VTNException;

    // TxTask

    /**
     * Determine whether this instance is associated with an asynchronous
     * task or not.
     *
     * <p>
     *   This method always returns {@code true} because this instance is
     *   associated with an asynchronous event task.
     * </p>
     *
     * @return  {@code true}.
     */
    @Override
    public final boolean isAsync() {
        return true;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void onFailure(VTNManagerProvider provider, Throwable t) {
        Logger logger = LoggerFactory.getLogger(getClass());
        logger.error("Uncaught exception while event delivery.", t);
    }
}
