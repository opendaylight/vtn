/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.tx;

import org.opendaylight.vtn.manager.internal.util.MiscUtils;

/**
 * An abstract implementation of
 * {@link org.opendaylight.vtn.manager.internal.TxTask} used to implement
 * RPC task.
 *
 * @param <T>  The type of the object to be returned by the task.
 */
public abstract class AbstractRpcTask<T> extends AbstractTxTask<T> {
    /**
     * Determine whether the transaction queue should log the given error
     * or not.
     *
     * <p>
     *   This method returns {@code false} if a
     *   {@link org.opendaylight.vtn.manager.VTNException} is passed and
     *   it seems to be caused by a bad request.
     * </p>
     *
     * @param t  A {@link Throwable} that is going to be thrown.
     * @return   {@code true} if the transaction queue should log the given
     *           {@link Throwable}. Otherwise {@code false}.
     */
    @Override
    public boolean needErrorLog(Throwable t) {
        return !MiscUtils.isBadRequest(t);
    }
}
