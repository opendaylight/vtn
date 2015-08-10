/*
 * Copyright (c) 2013, 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.concurrent;

import java.util.ArrayList;
import java.util.Deque;
import java.util.HashSet;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;
import java.util.Set;
import java.util.concurrent.AbstractExecutorService;
import java.util.concurrent.Callable;
import java.util.concurrent.RejectedExecutionException;
import java.util.concurrent.RunnableFuture;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.TimeoutException;
import java.util.concurrent.atomic.AtomicInteger;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

/**
 * {@code VTNThreadPool} implements thread pool which executes each
 * submitted task using pooled threads.
 *
 * <p>
 *   Submitted tasks are executed on worker threads in the pool.
 *   The first worker thread, called main thread, is created when the pool
 *   is created, and it never exits. When a new task is submitted to
 *   thread pool, and the number of worker threads in the pool is less than
 *   the thread pool size, a new thread is created to execute the task.
 *   Worker threads except for main thread will be terminated if they have
 *   been idle for the keep-alive time.
 * </p>
 */
public class VTNThreadPool extends AbstractExecutorService
    implements AutoCloseable {
    /**
     * Logger instance.
     */
    private static final Logger  LOG =
        LoggerFactory.getLogger(VTNThreadPool.class);

    /**
     * The number of milliseconds to wait for completion of thread pool
     * shutdown.
     */
    private static final long  SHUTDOWN_TIMEOUT = 5000;

    /**
     * Thread pool state which represents the pool is available.
     */
    private static final int STATE_RUNNING = 0;

    /**
     * Thread pool state which represents the pool is shut down.
     */
    private static final int STATE_SHUTDOWN = 1;

    /**
     * Thread pool state which represents the pool is terminated.
     */
    private static final int STATE_TERMINATE = 2;

    /**
     * Prefix of the name of worker threads.
     */
    private final String  namePrefix;

    /**
     * Pool size, which is the maximum number of worker threads.
     */
    private final int  poolSize;

    /**
     * Timeout value in milliseconds for idle threads waiting for task.
     */
    private final long  keepAliveTime;

    /**
     * The queue used to hold submitted tasks.
     */
    private final Deque<Runnable>  taskQueue = new LinkedList<Runnable>();

    /**
     * Index for worker thread.
     * <p>
     *   This is used for creating worker thread name.
     * </p>
     */
    private final AtomicInteger  threadIndex = new AtomicInteger();

    /**
     * Set of worker threads.
     */
    private final Set<WorkerThread>  workerThreads =
        new HashSet<WorkerThread>();

    /**
     * The number of tasks waiting for a new task.
     */
    private int  waiting;

    /**
     * The number of pending wake-up signals.
     */
    private int  pendingSignals;

    /**
     * State of the thread pool.
     */
    private int  poolState = STATE_RUNNING;

    /**
     * Construct a new thread pool which has a single worker thread.
     *
     * @param prefix  A prefix for the name of worker threads.
     */
    public VTNThreadPool(String prefix) {
        this(prefix, 1, 0);
    }

    /**
     * Construct a new thread pool.
     *
     * @param prefix  A prefix for the name of worker threads.
     * @param size    The maximum number of threads in the pool.
     * @param keep    Timeout value in milliseconds for idle threads waiting
     *                for task.
     */
    public VTNThreadPool(String prefix, int size, long keep) {
        namePrefix = prefix;
        poolSize = size;
        keepAliveTime = keep;

        // Create the main thread.
        String name = createName();
        MainThread t = new MainThread(name);
        workerThreads.add(t);
        t.start();
    }

    /**
     * Execute the specified task on one of worker threads in the pool.
     *
     * @param task  A task to be executed on this thread pool.
     * @return  {@code true} is returned if the specified task was submitted.
     *          {@code false} is returned if the specified tas was rejected.
     * @throws IllegalArgumentException
     *    {@code task} is {@code null}.
     */
    public synchronized boolean executeTask(Runnable task) {
        if (task == null) {
            throw new IllegalArgumentException("Task cannot be null.");
        }
        if (poolState != STATE_RUNNING) {
            return false;
        }

        taskQueue.addLast(task);

        int pending = pendingSignals;
        if (waiting > pending) {
            // The task will be executed on one of existing worker thread.
            pendingSignals = pending + 1;
            notify();
        } else if (workerThreads.size() < poolSize) {
            // Expand the pool size.
            createWorker();
        }

        return true;
    }

    /**
     * Determine whether this thread pool is alive or not.
     *
     * @return  {@code true} only if this thread pool is alive.
     */
    public synchronized boolean isAlive() {
        return (poolState == STATE_RUNNING);
    }

    /**
     * Shut down the thread pool, and terminate all submitted tasks.
     *
     * <p>
     *   This method interrupts all worker threads in the pool.
     * </p>
     */
    synchronized void terminate() {
        if (poolState != STATE_TERMINATE) {
            poolState = STATE_TERMINATE;
            taskQueue.clear();
            for (WorkerThread worker: workerThreads) {
                worker.interrupt();
            }
            notifyAll();
        }
    }

    /**
     * Wait for all worker threads to be terminated after shutdown request.
     *
     * <p>
     *   This method implies {@link #shutdown()} call.
     * </p>
     *
     * @param timeout  The maximum timeout in milliseconds to wait.
     *                 Zero means an infinite timeout.
     * @return  {@code true} is returned if all worker threads terminated.
     *          Otherwise {@code false} is returned.
     */
    synchronized boolean join(long timeout) {
        shutdown();

        try {
            boolean ret = awaitTermination(timeout, TimeUnit.MILLISECONDS);
            if (!ret) {
                LOG.error("join: Timed out");
            }

            return ret;
        } catch (InterruptedException e) {
            LOG.error("join: Interrupted", e);
        }

        return (workerThreads.size() == 0);
    }

    /**
     * Create name for a new worker thread.
     *
     * @return  Name for a new worker thread.
     */
    private String createName() {
        int index = threadIndex.getAndIncrement();
        StringBuilder builder = new StringBuilder(namePrefix);
        return builder.append('-').append(index).toString();
    }

    /**
     * Create a new worker thread, and start it.
     */
    private synchronized void createWorker() {
        String name = createName();
        WorkerThread worker = new WorkerThread(name);
        workerThreads.add(worker);
        worker.start();
    }

    /**
     * Remove the given worker thread.
     *
     * @param worker  A worker thread to be removed.
     */
    private synchronized void removeWorker(WorkerThread worker) {
        workerThreads.remove(worker);
        if (poolState != STATE_RUNNING && workerThreads.size() == 0) {
            notifyAll();
        }
    }

    /**
     * Wait for a new task to be submitted.
     *
     * @param worker   Calling worker thread.
     * @param timeout  Timeout in milliseconds to wait for a new task.
     * @return  A submitted task. {@code null} is returned if the calling
     *          worker thread should exit.
     */
    private synchronized Runnable poll(WorkerThread worker, long timeout) {
        if (taskQueue.size() == 0) {
            long limit = System.currentTimeMillis() + timeout;
            long tmout = timeout;
            do {
                if (poolState != STATE_RUNNING) {
                    removeWorker(worker);
                    return null;
                }

                waitForSignal(tmout);
                if (taskQueue.size() != 0) {
                    break;
                }

                tmout = limit - System.currentTimeMillis();
                if (tmout <= 0) {
                    removeWorker(worker);
                    return null;
                }
            } while (true);
        }

        return taskQueue.removeFirst();
    }

    /**
     * Dequeue a task from the task queue with an infinite timeout.
     *
     * @param worker  Calling worker thread.
     * @return  A dequeued task. {@code null} is returned if the calling
     *          worker thread should exit.
     */
    private synchronized Runnable getTask(WorkerThread worker) {
        while (taskQueue.size() == 0) {
            if (poolState != STATE_RUNNING) {
                removeWorker(worker);
                return null;
            }

            waitForSignal(0);
        }

        return taskQueue.removeFirst();
    }

    /**
     * Block the calling thread until the thread receives the wake-up signal.
     *
     * <p>
     *   Note that this method always ignores {@code InterruptedException}.
     * </p>
     *
     * @param timeout   The maximum time to wait in milliseconds.
     *                  Zero means an infinite timeout.
     */
    private synchronized void waitForSignal(long timeout) {
        waiting++;
        try {
            wait(timeout);
        } catch (InterruptedException e) {
            // Ignore interruption.
        } finally {
            waiting--;
            int pending = pendingSignals - 1;
            if (pending >= 0) {
                pendingSignals = pending;
            }
        }
    }

    /**
     * Finalize the thread pool.
     *
     * <p>
     *   This method calls {@link #terminate()} to terminate all worker
     *   threads.
     * </p>
     *
     * @throws Throwable  An error occurred.
     */
    @Override
    protected void finalize() throws Throwable {
        terminate();
        super.finalize();
    }

    /**
     * {@code WorkerThread} class implements worker threads in thread pool.
     */
    protected class WorkerThread extends Thread {
        /**
         * Construct a new worker thread.
         *
         * @param name  The name of this thread.
         */
        protected WorkerThread(String name) {
            super(name);
        }

        /**
         * Wait for a new task.
         *
         * @return  A dequeued task. {@code null} is returned if the worker
         *          thread should exit.
         */
        protected Runnable waitFor() {
            return poll(this, keepAliveTime);
        }

        /**
         * Main routine of worker thread.
         */
        @Override
        public final void run() {
            LOG.trace("Start");

            for (Runnable r = waitFor(); r != null; r = waitFor()) {
                try {
                    r.run();
                } catch (Exception e) {
                    LOG.error("Exception occurred on a thread pool worker", e);
                }
            }

            LOG.trace("Exit");
        }
    }

    /**
     * {@code MainThread} class implements the main thread of the pool.
     */
    private final class MainThread extends WorkerThread {
        /**
         * Construct a main thread.
         *
         * @param name  The name of this thread.
         */
        private MainThread(String name) {
            super(name);
        }

        /**
         * Wait for a new task.
         *
         * @return  A dequeued task. {@code null} is returned if the worker
         *          thread should exit.
         */
        @Override
        protected Runnable waitFor() {
            return getTask(this);
        }
    }

    // AbstractExecutorService

    /**
     * Return a {@link RunnableFuture} instance for the given runnable and
     * return value.
     *
     * @param runnable  The runnable task being wrapped.
     * @param value     The value for the returned future.
     * @param <T>       The type of the value to be returned.
     * @return  A {@link RunnableFuture} instance.
     */
    @Override
    protected <T> RunnableFuture<T> newTaskFor(Runnable runnable, T value) {
        return new VTNFutureTask<T>(runnable, value);
    }

    /**
     * Return a {@link RunnableFuture} instance for the given callable task.
     *
     * @param callable  The callable task being wrapped.
     * @param <T>       The type of the value to be returned.
     * @return  A {@link RunnableFuture} instance.
     */
    @Override
    protected <T> RunnableFuture<T> newTaskFor(Callable<T> callable) {
        return new VTNFutureTask<T>(callable);
    }

    // ExecutorService

    /**
     * Shut down the thread pool.
     *
     * <p>
     *   After calling method, new tasks will be rejected.
     *   Note that this method does not wait for completion of tasks
     *   submitted previously.
     * </p>
     */
    @Override
    public synchronized void shutdown() {
        if (poolState == STATE_RUNNING) {
            poolState = STATE_SHUTDOWN;
            if (waiting != 0) {
                notifyAll();
            }
        }
    }

    /**
     * Shut down the thread pool, and attempt to stop all executing tasks.
     *
     * <p>
     *   This method interrupts all worker threads in the pool.
     * </p>
     *
     * @return  A list of tasks that were never executed.
     */
    @Override
    public synchronized List<Runnable> shutdownNow() {
        List<Runnable> tasks = new ArrayList<>();
        for (Iterator<Runnable> it = taskQueue.iterator(); it.hasNext();) {
            tasks.add(it.next());
            it.remove();
        }
        terminate();

        return tasks;
    }

    /**
     * Determine whether this thread pool is already shut down or not.
     *
     * @return  {@code true} only if this thread pool is already shut down.
     */
    @Override
    public boolean isShutdown() {
        return !isAlive();
    }

    /**
     * Determine whether all tasks in this thread pool have completed after a
     * shut down request.
     *
     * @return  {@code true} only if this thread pool is already shut down and
     *          all tasks have completed.
     */
    @Override
    public synchronized boolean isTerminated() {
        return (isShutdown() && workerThreads.size() == 0);
    }

    /**
     * Block the calling thread until all tasks in this pool have completed
     * execution after a shut down request.
     *
     * @param timeout  The maximum time to wait.
     * @param unit     The time unit of the {@code timeout} argument.
     * @return  {@code true} if all tasks have terminated.
     *          {@code false} if at least one task did not complete.
     * @throws InterruptedException
     *    The calling thread was interrupted.
     */
    @Override
    public synchronized boolean awaitTermination(long timeout, TimeUnit unit)
        throws InterruptedException {
        TimeoutCounter tc = TimeoutCounter.newTimeout(timeout, unit);

        try {
            while (workerThreads.size() != 0) {
                tc.await(this);
            }

            return true;
        } catch (TimeoutException e) {
            // One more check should be done.
        }

        return (workerThreads.size() == 0);
    }

    // Executor

    /**
     * Execute the specified task on one of worker threads in the pool.
     *
     * @param task  A task to be executed on this thread pool.
     * @throws RejectedExecutionException
     *    The specified task was rejected.
     * @throws NullPointerException
     *    {@code task} is {@code null}.
     */
    @Override
    public void execute(Runnable task) {
        if (!executeTask(task)) {
            throw new RejectedExecutionException(
                "This thread pool is already closed.");
        }
    }

    // AutoCloseable

    /**
     * Close this thread pool instance.
     *
     * <p>
     *   Threads that did not exit within {@link #SHUTDOWN_TIMEOUT}
     *   milliseconds will be terminated by force.
     * </p>
     */
    @Override
    public void close() {
        if (!join(SHUTDOWN_TIMEOUT)) {
            LOG.warn("Async thread pool({}) did not terminate within {} msec",
                     namePrefix, SHUTDOWN_TIMEOUT);
            terminate();
        }
    }
}
