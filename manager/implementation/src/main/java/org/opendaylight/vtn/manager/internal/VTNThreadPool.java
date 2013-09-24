/*
 * Copyright (c) 2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal;

import java.util.Deque;
import java.util.LinkedList;
import java.util.Set;
import java.util.HashSet;
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
public class VTNThreadPool {
    /**
     * Logger instance.
     */
    private static final Logger  LOG =
        LoggerFactory.getLogger(VTNThreadPool.class);

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
    private volatile int  waiting;

    /**
     * State of the thread pool.
     */
    private volatile int  poolState = STATE_RUNNING;

    /**
     * Construct a new thread pool
     *
     * @param prefix  A prefix for the name of worker threads.
     * @param size    The maximum number of threads in the pool.
     * @param keep    Timeout value in milliseconds for idle threads waiting
     *                for task.
     */
    VTNThreadPool(String prefix, int size, long keep) {
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
     */
    synchronized boolean execute(Runnable task) {
        if (poolState != STATE_RUNNING) {
            return false;
        }

        taskQueue.addLast(task);
        if (waiting != 0) {
            // The task will be executed on one of existing worker thread.
            notify();
        } else if (workerThreads.size() < poolSize) {
            // Expand the pool size.
            createWorker();
        }

        return true;
    }

    /**
     * Shut down the thread pool.
     *
     * <p>
     *   After calling method, new tasks will be rejected.
     *   Note that this method does not wait for completion of tasks
     *   submitted previously.
     * </p>
     */
    synchronized void shutdown() {
        if (poolState == STATE_RUNNING) {
            poolState = STATE_SHUTDOWN;
            if (waiting != 0) {
                notifyAll();
            }
        }
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

        if (workerThreads.size() != 0) {
            if (timeout == 0) {
                do {
                    try {
                        wait(0);
                    } catch (InterruptedException e) {
                        LOG.error("join: Interrupted", e);
                        return (workerThreads.size() == 0);
                    }
                } while (workerThreads.size() != 0);
            } else {
                long limit = System.currentTimeMillis() + timeout;
                do {
                    try {
                        wait(timeout);
                    } catch (InterruptedException e) {
                        LOG.error("join: Interrupted", e);
                        return (workerThreads.size() == 0);
                    }
                    timeout = limit - System.currentTimeMillis();
                    if (timeout <= 0) {
                        LOG.error("join: Timed out");
                        return (workerThreads.size() == 0);
                    }
                } while (workerThreads.size() != 0);
            }
        }

        return true;
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
            do {
                if (poolState != STATE_RUNNING) {
                    removeWorker(worker);
                    return null;
                }

                waiting++;
                try {
                    wait(timeout);
                } catch (InterruptedException e) {
                } finally {
                    waiting--;
                }

                if (taskQueue.size() != 0) {
                    break;
                }

                timeout = limit - System.currentTimeMillis();
                if (timeout <= 0) {
                    removeWorker(worker);
                    return null;
                }
            } while (true);
        }

        return taskQueue.remove();
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

            waiting++;
            try {
                wait();
            } catch (InterruptedException e) {
            } finally {
                waiting--;
            }
        }

        return taskQueue.remove();
    }

    /**
     * Finalize the thread pool.
     *
     * <p>
     *   This method calls {@link #terminate()} to terminate all worker
     *   threads.
     * </p>
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
        public void run() {
            if (LOG.isDebugEnabled()) {
                LOG.debug("Start");
            }

            for (Runnable r = waitFor(); r != null; r = waitFor()) {
                try {
                    r.run();
                } catch (Exception e) {
                    LOG.error("Exception occurred on a thread pool worker", e);
                }
            }

            if (LOG.isDebugEnabled()) {
                LOG.debug("Exit");
            }
        }
    }

    /**
     * {@code MainThread} class implements the main thread of the pool.
     */
    protected class MainThread extends WorkerThread {
        /**
         * Construct a main thread.
         *
         * @param name  The name of this thread.
         */
        protected MainThread(String name) {
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
}
