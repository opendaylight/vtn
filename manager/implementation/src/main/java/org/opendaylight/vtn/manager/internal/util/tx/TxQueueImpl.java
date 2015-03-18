/*
 * Copyright (c) 2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.tx;

import java.util.Deque;
import java.util.LinkedList;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.TimeoutException;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import com.google.common.util.concurrent.CheckedFuture;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.TxContext;
import org.opendaylight.vtn.manager.internal.TxQueue;
import org.opendaylight.vtn.manager.internal.TxTask;
import org.opendaylight.vtn.manager.internal.VTNManagerProvider;
import org.opendaylight.vtn.manager.internal.util.concurrent.SettableVTNFuture;
import org.opendaylight.vtn.manager.internal.util.concurrent.VTNFuture;
import org.opendaylight.vtn.manager.internal.util.flow.cond.FlowCondReader;
import org.opendaylight.vtn.manager.internal.util.inventory.InventoryReader;

import org.opendaylight.controller.md.sal.binding.api.ReadTransaction;
import org.opendaylight.controller.md.sal.binding.api.ReadWriteTransaction;
import org.opendaylight.controller.md.sal.common.api.data.OptimisticLockFailedException;
import org.opendaylight.controller.md.sal.common.api.data.TransactionCommitFailedException;

/**
 * Implementation of {@link TxQueue}.
 *
 * <p>
 *   MD-SAL datastore transaction may fail if the same data tree is modified
 *   by multiple transactions concurrently. This class is used to serialize
 *   modification to the same MD-SAL data tree. An instance of this class
 *   has a single transaction queue and runner thread. Queued transactions
 *   are executed sequentially on the runner thread.
 * </p>
 *
 * @see TxTask
 */
public final class TxQueueImpl implements TxQueue, Runnable, AutoCloseable {
    /**
     * Logger instance.
     */
    private static final Logger  LOG =
        LoggerFactory.getLogger(TxQueueImpl.class);

    /**
     * The number of nanoseconds to wait for completion of transaction submit.
     */
    private static final long  SUBMIT_TIMEOUT = TimeUnit.SECONDS.toNanos(10L);

    /**
     * The number of milliseconds to wait for completion of runner thread.
     */
    private static final long  RUNNER_JOIN_TIMEOUT =
        TimeUnit.SECONDS.toMillis(10L);

    /**
     * VTN Manager provider service.
     */
    private final VTNManagerProvider  vtnProvider;

    /**
     * A transaction queue.
     */
    private Deque<TxFuture>  txQueue = new LinkedList<TxFuture>();

    /**
     * A runner thread.
     */
    private final Thread  runnerThread;

    /**
     * A {@link VTNFuture} implementation to wait for the completion of
     * {@link TxTask}.
     *
     * @param <T>  The type of the object to be returned by the task.
     */
    private static final class TxFuture<T> extends SettableVTNFuture<T>
        implements TxContext {
        /**
         * VTN Manager provider service.
         */
        private final VTNManagerProvider  vtnProvider;

        /**
         * A task associated with this future.
         */
        private final TxTask<T>  txTask;

        /**
         * Current MD-SAL datastore transaction.
         */
        private ReadWriteTransaction  transaction;

        /**
         * A VTN inventory reader.
         */
        private InventoryReader  inventoryReader;

        /**
         * A flow condition reader.
         */
        private FlowCondReader  flowCondReader;

        /**
         * Construct a new instance.
         *
         * @param provider  A {@link VTNManagerProvider} instance.
         * @param task      A {@link TxTask} instance.
         */
        private TxFuture(VTNManagerProvider provider, TxTask<T> task) {
            vtnProvider = provider;
            txTask = task;
        }

        /**
         * Execute the task.
         *
         * @param attempts  The number of calls of this method.
         *                  0 is passed for the first call.
         * @throws TransactionCommitFailedException
         *    Failed to submit transaction.
         * @throws TimeoutException
         *    Transaction did not complete within the timeout.
         * @throws InterruptedException
         *    The task was canceled by another thread.
         * @throws VTNException
         *    An error occurred.
         */
        private void execute(int attempts)
            throws TransactionCommitFailedException, TimeoutException,
                   InterruptedException, VTNException {
            // Enable cancellation.
            unmaskCancel();

            // Execute the task.
            T res = txTask.execute(this, attempts);

            // Submit the transaction if needed.
            ReadWriteTransaction tx = transaction;
            if (tx != null) {
                submit(tx);
            }

            // Complete the task.
            set(res);
        }

        /**
         * Submit the transaction.
         *
         * @param tx  A {@link ReadWriteTransaction} to submit.
         * @throws TransactionCommitFailedException
         *    Failed to submit transaction.
         * @throws TimeoutException
         *    Transaction did not complete within the timeout.
         * @throws InterruptedException
         *    The task was canceled by another thread.
         */
        private void submit(ReadWriteTransaction tx)
            throws TransactionCommitFailedException, TimeoutException,
                   InterruptedException {
            // Disable cancellation while submitting.
            maskCancel();

            // Submit the transaction.
            CheckedFuture<Void, TransactionCommitFailedException> future =
                tx.submit();

            // Wait for the transaction to be submitted.
            long nanos = SUBMIT_TIMEOUT;
            future.checkedGet(nanos, TimeUnit.NANOSECONDS);
        }

        /**
         * Determine whether this future is associated with a {@link TxEvent}
         * or not.
         *
         * @return  {@code true} only if this future is associated with a
         *          {@link TxEvent}.
         */
        private boolean isTxEvent() {
            return (txTask instanceof TxEvent);
        }

        /**
         * Return the simple class name of the task.
         *
         * @return  The simple class name of the task.
         */
        private String getSimpleTaskName() {
            return txTask.getClass().getSimpleName();
        }

        // AbstractVTNFuture

        /**
         * {@inheritDoc}
         */
        @Override
        protected void onFutureSucceeded(T result) {
            txTask.onSuccess(vtnProvider, result);
        }

        /**
         * {@inheritDoc}
         */
        @Override
        protected void onFutureFailed(Throwable cause) {
            txTask.onFailure(vtnProvider, cause);
        }

        // TxContext

        /**
         * {@inheritDoc}
         */
        @Override
        public ReadTransaction getTransaction() {
            return getReadWriteTransaction();
        }

        /**
         * {@inheritDoc}
         */
        @Override
        public ReadWriteTransaction getReadWriteTransaction() {
            ReadWriteTransaction tx = transaction;
            if (tx == null) {
                tx = vtnProvider.getDataBroker().newReadWriteTransaction();
                transaction = tx;
            }

            return tx;
        }

        /**
         * {@inheritDoc}
         */
        @Override
        public InventoryReader getInventoryReader() {
            InventoryReader reader = inventoryReader;
            if (reader == null) {
                reader = new InventoryReader(getReadWriteTransaction());
                inventoryReader = reader;
            }

            return reader;
        }

        /**
         * {@inheritDoc}
         */
        @Override
        public FlowCondReader getFlowCondReader() {
            FlowCondReader reader = flowCondReader;
            if (reader == null) {
                reader = new FlowCondReader(getReadWriteTransaction());
                flowCondReader = reader;
            }

            return reader;
        }

        /**
         * {@inheritDoc}
         */
        @Override
        public void cancelTransaction() {
            ReadWriteTransaction tx = transaction;
            if (tx != null) {
                transaction = null;
                inventoryReader = null;
                tx.cancel();
            }
        }

        /**
         * {@inheritDoc}
         */
        @Override
        public VTNManagerProvider getProvider() {
            return vtnProvider;
        }
    }

    /**
     * Construct a new instance.
     *
     * @param name      The name of this queue.
     * @param provider  A {@link VTNManagerProvider} service instance.
     */
    public TxQueueImpl(String name, VTNManagerProvider provider) {
        vtnProvider = provider;
        runnerThread = new Thread(this, "Transaction Queue Runner: " + name);
    }

    /**
     * Start the transaction queue processing.
     */
    public void start() {
        runnerThread.start();
    }

    /**
     * Deque one transaction task.
     *
     * @return  A {@link TxFuture} instance.
     *          {@code null} if this queue was closed.
     */
    private synchronized TxFuture<?> getTask() {
        while (true) {
            if (txQueue == null) {
                return null;
            }

            while (txQueue.size() != 0) {
                // Ignore canceled task.
                TxFuture<?> future = txQueue.removeFirst();
                if (!future.isCancelled()) {
                    return future;
                }
            }

            try {
                wait();
            } catch (InterruptedException e) {
            }
        }
    }

    /**
     * Execute the given task.
     *
     * @param future  A {@link TxFuture} to execute.
     */
    private void execute(TxFuture<?> future) {
        if (future.setThread(runnerThread)) {
            // Already canceled.
            return;
        }

        for (int attempts = 0; true; attempts++) {
            try {
                future.execute(attempts);
                break;
            } catch (OptimisticLockFailedException e) {
                // Transaction failed due to data conflict.
                // In that case the transaction should be retried.
                if (future.isTxEvent()) {
                    LOG.warn("{}: Event will be dispatched again because of " +
                             "data conflict.", future.getSimpleTaskName());
                }
            } catch (Throwable t) {
                future.setException(t);
                break;
            } finally {
                future.cancelTransaction();
            }
        }
    }

    // TxQueue

    /**
     * Post a new transaction.
     *
     * @param task  A {@link TxTask} instance.
     * @param <T>   The type of the object to be returned by the task.
     * @return  A {@link VTNFuture} instance that returns the result of
     *          the task.
     */
    @Override
    public synchronized <T> VTNFuture<T> post(TxTask<T> task) {
        TxFuture<T> future = new TxFuture<T>(vtnProvider, task);
        if (txQueue == null) {
            // This queue has been already closed.
            future.cancel(false);
        } else {
            txQueue.addLast(future);
            notify();
        }

        return future;
    }

    /**
     * Post a new transaction.
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
    @Override
    public synchronized <T> VTNFuture<T> postFirst(TxTask<T> task) {
        TxFuture<T> future = new TxFuture<T>(vtnProvider, task);
        if (txQueue == null) {
            // This queue has already been closed.
            future.cancel(false);
        } else {
            txQueue.addFirst(future);
            notify();
        }

        return future;
    }

    // AutoCloseable

    /**
     * Close this transaction queue.
     */
    @Override
    public void close() {
        Deque<TxFuture> queue;

        synchronized (this) {
            queue = txQueue;
            if (queue != null) {
                txQueue = null;
                notify();
            }
        }

        if (queue != null) {
            for (TxFuture future: queue) {
                future.cancel(true);
            }
        }

        Thread t = runnerThread;
        if (t != null) {
            try {
                t.join(RUNNER_JOIN_TIMEOUT);
            } catch (InterruptedException e) {
            }

            if (t.isAlive()) {
                t.interrupt();
                LOG.warn("The runner thread did not complete.");
            }
        }
    }

    // Runnable

    /**
     * Main routine of the runner thread.
     */
    @Override
    public void run() {
        LOG.trace("Started.");

        for (TxFuture future = getTask(); future != null; future = getTask()) {
            execute(future);
        }

        LOG.trace("Stopped.");
    }
}
