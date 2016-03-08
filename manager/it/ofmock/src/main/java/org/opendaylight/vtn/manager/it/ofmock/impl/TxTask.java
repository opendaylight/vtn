/*
 * Copyright (c) 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.ofmock.impl;

import java.util.concurrent.TimeUnit;
import java.util.concurrent.TimeoutException;

import org.slf4j.Logger;

import com.google.common.util.concurrent.CheckedFuture;
import com.google.common.util.concurrent.ListenableFuture;
import com.google.common.util.concurrent.SettableFuture;

import org.opendaylight.controller.md.sal.binding.api.DataBroker;
import org.opendaylight.controller.md.sal.binding.api.ReadWriteTransaction;
import org.opendaylight.controller.md.sal.common.api.data.OptimisticLockFailedException;
import org.opendaylight.controller.md.sal.common.api.data.TransactionCommitFailedException;

/**
 * {@code TxTask} is an abstract task that updates MD-SAL datastore.
 *
 * @param <T>  The type of the value to be returned by the task.
 */
public abstract class TxTask<T> implements Runnable {
    /**
     * The number of times for retrying transaction.
     */
    private static final int  MAX_RETRY = 5;

    /**
     * The number of nanoseconds to wait for completion of transaction submit.
     */
    private static final long  SUBMIT_TIMEOUT = TimeUnit.SECONDS.toNanos(10L);

    /**
     * The data broker service.
     */
    private final DataBroker  dataBroker;

    /**
     * A future associated with this task.
     */
    private SettableFuture<T>  taskFuture = SettableFuture.<T>create();

    /**
     * Construct a new instance.
     *
     * @param broker  The data broker service.
     */
    protected TxTask(DataBroker broker) {
        dataBroker = broker;
    }

    /**
     * Return the future associated with this task.
     *
     * @return  A {@link ListenableFuture} instance.
     */
    public ListenableFuture<T> getFuture() {
        return taskFuture;
    }

    /**
     * Return the data broker service.
     *
     * @return  The data broker service.
     */
    protected final DataBroker getDataBroker() {
        return dataBroker;
    }

    /**
     * Complete the task.
     */
    protected final void complete() {
        taskFuture.set(getResult());
    }

    /**
     * Return the value to be retuned by the task on successful completion.
     *
     * @return  {@code null}.
     */
    protected T getResult() {
        return null;
    }

    /**
     * Update the MD-SAL datastore.
     *
     * @param tx  A read-write MD-SAL datastore transaction.
     */
    protected abstract void execute(ReadWriteTransaction tx);

    /**
     * Return a logger instance.
     *
     * @return  A logger instance.
     */
    protected abstract Logger getLogger();

    /**
     * Submit the given DS transaction.
     *
     * @param tx        A read-write MD-SAL datastore transaction.
     * @param attempts  The number of calls of this method.
     *                  0 is passed for the first call.
     * @return  {@code true} if the transaction was submitted.
     *          {@code false} if the MD-SAL DS transaction should be retried.
     * @throws TransactionCommitFailedException
     *    Failed to submit the transaction.
     * @throws TimeoutException
     *    Operation timed out.
     */
    private boolean submit(ReadWriteTransaction tx, int attempts)
        throws TransactionCommitFailedException, TimeoutException {
        boolean submitted = false;
        try {
            CheckedFuture<Void, TransactionCommitFailedException> future =
                tx.submit();
            future.checkedGet(SUBMIT_TIMEOUT, TimeUnit.NANOSECONDS);
            submitted = true;
        } catch (OptimisticLockFailedException e) {
            if (attempts < MAX_RETRY) {
                // In this case the transaction should be retried.
                Logger logger = getLogger();
                logger.debug("Transaction failed due to data conflict.", e);
            } else {
                throw e;
            }
        }

        return submitted;
    }

    // Runnable

    /**
     * Update MD-SAL datastore in a single transaction.
     */
    @Override
    public void run() {
        for (int i = 0; true; i++) {
            ReadWriteTransaction tx = dataBroker.newReadWriteTransaction();
            boolean submitted = false;
            try {
                execute(tx);
                submitted = submit(tx, i);
                if (submitted) {
                    complete();
                    return;
                }
            } catch (Exception e) {
                String msg = "Failed to update transaction.";
                getLogger().error(msg, e);
                taskFuture.setException(new IllegalStateException(msg, e));
            } finally {
                if (!submitted) {
                    tx.cancel();
                }
            }
        }
    }
}
