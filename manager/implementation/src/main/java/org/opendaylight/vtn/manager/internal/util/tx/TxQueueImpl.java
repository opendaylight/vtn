/*
 * Copyright (c) 2015, 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.tx;

import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.Deque;
import java.util.LinkedList;
import java.util.List;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.TimeoutException;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import com.google.common.util.concurrent.CheckedFuture;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.TxContext;
import org.opendaylight.vtn.manager.internal.TxHook;
import org.opendaylight.vtn.manager.internal.TxQueue;
import org.opendaylight.vtn.manager.internal.TxTask;
import org.opendaylight.vtn.manager.internal.VTNManagerProvider;
import org.opendaylight.vtn.manager.internal.util.concurrent.SettableVTNFuture;
import org.opendaylight.vtn.manager.internal.util.concurrent.VTNFuture;
import org.opendaylight.vtn.manager.internal.util.log.FixedLogger;
import org.opendaylight.vtn.manager.internal.util.log.FixedLoggerCache;
import org.opendaylight.vtn.manager.internal.util.log.LogRecord;
import org.opendaylight.vtn.manager.internal.util.log.VTNLogLevel;

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
     * The timeout in milliseconds for graceful shutdown.
     */
    private static final long  SHUTDOWN_TIMEOUT =
        TimeUnit.SECONDS.toMillis(10L);

    /**
     * The timeout in milliseconds for shutdown by force.
     */
    private static final long  CLOSE_TIMEOUT =
        TimeUnit.SECONDS.toMillis(3L);

    /**
     * VTN Manager provider service.
     */
    private final VTNManagerProvider  vtnProvider;

    /**
     * A transaction queue.
     */
    private final Deque<TxFuture>  txQueue = new LinkedList<>();

    /**
     * A runner thread.
     */
    private final Thread  runnerThread;

    /**
     * Keep {@code true} while this queue is available.
     */
    private boolean  available = true;

    /**
     * A {@link VTNFuture} implementation to wait for the completion of
     * {@link TxTask}.
     *
     * @param <T>  The type of the object to be returned by the task.
     */
    private static final class TxFuture<T> extends SettableVTNFuture<T>
        implements Comparator<TxHook>, TxContext {
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
         * Read-only data specific to the current transaction.
         */
        private final TxSpecific<ReadTransaction>  readSpecific =
            new TxSpecific<>(ReadTransaction.class);

        /**
         * Writable data specific to the current transaction.
         */
        private final TxSpecific<TxContext>  specific =
            new TxSpecific<>(TxContext.class);

        /**
         * Cached for logger instances.
         */
        private final FixedLoggerCache  loggerCache = new FixedLoggerCache();

        /**
         * Cached log records.
         */
        private final Deque<LogRecord>  logRecords = new LinkedList<>();

        /**
         * Transaction pre-submit hooks.
         */
        private final List<TxHook>  preSubmitHooks = new ArrayList<>();

        /**
         * Transaction post-submit hooks.
         */
        private final List<TxHook>  postSubmitHooks = new ArrayList<>();

        /**
         * Set {@code true} if the MD-SAL DS transaction has been submitted.
         */
        private boolean  submitted;

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
            submitted = true;
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
            // Run pre-submit hooks.
            runHooks(preSubmitHooks);

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

        /**
         * Set the cause of the failure of this task.
         *
         * @param cause  A {@link Throwable} instance which indicates the cause
         *               of the failure.
         */
        private void setFailure(Throwable cause) {
            if (txTask.needErrorLog(cause)) {
                LOG.error("DS transaction failed.", cause);
            }
            setException(cause);
        }

        /**
         * Logs a log messages recorded by the transaction task.
         */
        private void flushLogRecords() {
            for (LogRecord lr = logRecords.pollFirst(); lr != null;
                 lr = logRecords.pollFirst()) {
                lr.log();
            }
        }

        /**
         * Run hooks in the given list, and make it empty.
         *
         * @param hooks  A list of hooks to be invoked.
         */
        private void runHooks(List<TxHook> hooks) {
            // Sort hooks in ascending order of order value.
            Collections.sort(hooks, this);

            for (TxHook hook: hooks) {
                hook.run(this, txTask);
            }

            hooks.clear();
        }

        // SettableVTNFuture

        /**
         * {@inheritDoc}
         */
        @Override
        protected void onFutureSucceeded(T result) {
            // Run post-submit hooks.
            runHooks(postSubmitHooks);

            flushLogRecords();
            txTask.onSuccess(vtnProvider, result);
        }

        /**
         * {@inheritDoc}
         */
        @Override
        protected void onFutureFailed(Throwable cause) {
            flushLogRecords();
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
        public void cancelTransaction() {
            ReadWriteTransaction tx = transaction;
            if (tx != null) {
                transaction = null;
                readSpecific.clear();
                specific.clear();
                if (!submitted) {
                    tx.cancel();
                }
            }

            // Clear submit hooks registered by the previous transaction.
            preSubmitHooks.clear();
            postSubmitHooks.clear();

            // Clear log records.
            logRecords.clear();
        }

        /**
         * {@inheritDoc}
         */
        @Override
        public <T> T getReadSpecific(Class<T> type) {
            return readSpecific.get(type, getReadWriteTransaction());
        }

        /**
         * {@inheritDoc}
         */
        @Override
        public <T> T getSpecific(Class<T> type) {
            return specific.get(type, this);
        }

        /**
         * {@inheritDoc}
         */
        @Override
        public void addPreSubmitHook(TxHook hook) {
            preSubmitHooks.add(hook);
        }

        /**
         * {@inheritDoc}
         */
        @Override
        public void addPostSubmitHook(TxHook hook) {
            postSubmitHooks.add(hook);
        }

        /**
         * {@inheritDoc}
         */
        @Override
        public VTNManagerProvider getProvider() {
            return vtnProvider;
        }

        /**
         * {@inheritDoc}
         */
        @Override
        public void log(Logger logger, VTNLogLevel level, String msg) {
            if (level.isEnabled(logger)) {
                FixedLogger flogger = loggerCache.get(logger, level);
                logRecords.addLast(new LogRecord(flogger, msg));
            }
        }

        /**
         * {@inheritDoc}
         */
        @Override
        public void log(Logger logger, VTNLogLevel level, String format,
                        Object ... args) {
            if (level.isEnabled(logger)) {
                FixedLogger flogger = loggerCache.get(logger, level);
                logRecords.addLast(new LogRecord(flogger, format, args));
            }
        }

        /**
         * {@inheritDoc}
         */
        @Override
        public void log(Logger logger, VTNLogLevel level, String msg,
                        Throwable t) {
            if (level.isEnabled(logger)) {
                FixedLogger flogger = loggerCache.get(logger, level);
                logRecords.addLast(new LogRecord(flogger, msg, t));
            }
        }

        /**
         * {@inheritDoc}
         */
        @Override
        public void log(Logger logger, VTNLogLevel level, Throwable t,
                        String format, Object ... args) {
            if (level.isEnabled(logger)) {
                FixedLogger flogger = loggerCache.get(logger, level);
                logRecords.addLast(new LogRecord(flogger, t, format, args));
            }
        }

        // Comparator

        /**
         * Compare the given {@link TxHook} instances.
         *
         * @param h1  The first object to be compared.
         * @param h2  The second object to be compared.
         * @return    A negative integer, zero, or a positive integer as the
         *            first argument is less than, equal to, or greater than
         *            the second.
         */
        @Override
        public int compare(TxHook h1, TxHook h2) {
            return Integer.compare(h1.getOrder(), h2.getOrder());
        }

        // AutoCloseable

        /**
         * {@inheritDoc}
         */
        @Override
        public void close() {
            cancelTransaction();
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
        while (available) {
            for (TxFuture<?> future = txQueue.pollFirst(); future != null;
                 future = txQueue.pollFirst()) {
                // Ignore canceled task.
                if (!future.isCancelled()) {
                    return future;
                }
            }

            try {
                wait();
            } catch (InterruptedException e) {
                // Ignore interruption.
            }
        }

        return null;
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
                // In this case the transaction should be retried.
                LOG.trace("Transaction failed due to data conflict.", e);
                if (future.isTxEvent()) {
                    LOG.warn("{}: Event will be dispatched again because of " +
                             "data conflict.", future.getSimpleTaskName());
                }
            } catch (Exception e) {
                future.setFailure(e);
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
        if (!available) {
            // This queue has already been closed.
            future.cancel(false);
        } else {
            txQueue.addLast(future);
            notifyAll();
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
        if (!available) {
            // This queue has already been closed.
            future.cancel(false);
        } else {
            txQueue.addFirst(future);
            notifyAll();
        }

        return future;
    }

    // AutoCloseable

    /**
     * Close this transaction queue.
     */
    @Override
    public void close() {
        // At first, wait for all tasks to complete without cancellation.
        if (awaitShutdown()) {
            return;
        }

        // Cancel all tasks on the queue.
        List<TxFuture> queue;
        synchronized (this) {
            queue = new ArrayList<>(txQueue);
            if (!queue.isEmpty()) {
                txQueue.clear();
                notifyAll();
            }
        }

        for (TxFuture future: queue) {
            future.cancel(true);
        }

        try {
            runnerThread.join(CLOSE_TIMEOUT);
        } catch (InterruptedException e) {
            // Ignore interruption.
        }

        if (runnerThread.isAlive()) {
            runnerThread.interrupt();
            LOG.warn("The runner thread did not complete.");
        }
    }

    /**
     * Wait for all tasks previously queued to complete.
     *
     * @return  {@code true} if the runner thread was terminated.
     *          Otherwise {@code false}.
     */
    private boolean awaitShutdown() {
        // Reject new tasks.
        synchronized (this) {
            available = false;
            notifyAll();
        }

        try {
            runnerThread.join(SHUTDOWN_TIMEOUT);
        } catch (InterruptedException e) {
            // Ignore interruption.
        }

        return !runnerThread.isAlive();
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
