/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal;

import org.opendaylight.vtn.manager.internal.util.concurrent.VTNFuture;

/**
 * {@code TxQueue} describes MD-SAL transaction queue.
 * A task to modify MD-SAL datastore needs to be processed via this interface.
 *
 * @see TxTask
 */
public interface TxQueue {
    /**
     * Post a new transaction for MD-SAL datastore.
     *
     * <p>
     *   This method put the given task at the tail of the transaction queue.
     * </p>
     *
     * @param task  A {@link TxTask} instance.
     * @param <T>   The type of the object to be returned by the task.
     * @return  A {@link VTNFuture} instance that returns the result of
     *          the task.
     */
    <T> VTNFuture<T> post(TxTask<T> task);

    /**
     * Post a new transaction for MD-SAL datastore.
     *
     * <p>
     *   This method put the given task at the head of the transaction queue.
     * </p>
     *
     * @param task  A {@link TxTask} instance.
     * @param <T>   The type of the object to be returned by the task.
     * @return  A {@link VTNFuture} instance that returns the result
     *          of the task.
     */
    <T> VTNFuture<T> postFirst(TxTask<T> task);
}
