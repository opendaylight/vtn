/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.concurrent;

import static org.mockito.Mockito.doThrow;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.verifyNoMoreInteractions;
import static org.mockito.Mockito.when;
import static org.mockito.MockitoAnnotations.initMocks;

import java.util.concurrent.Callable;
import java.util.concurrent.CancellationException;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.TimeUnit;

import com.google.common.util.concurrent.Futures;
import com.google.common.util.concurrent.FutureCallback;

import org.junit.Before;
import org.junit.Test;

import org.mockito.Mock;
import org.mockito.invocation.InvocationOnMock;
import org.mockito.stubbing.Answer;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.CancelFutureTask;
import org.opendaylight.vtn.manager.internal.DelayedRunnableTask;
import org.opendaylight.vtn.manager.internal.DelayedTask;
import org.opendaylight.vtn.manager.internal.TestBase;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnErrorTag;

/**
 * JUnit test for {@link VTNFutureTask}.
 */
public class VTNFutureTaskTest extends TestBase
    implements FutureCallback<Long> {
    /**
     * The callable used for test.
     */
    @Mock
    private Callable<Long>  callable;

    /**
     * A long value returned by the target future.
     */
    private Long  taskResult;

    /**
     * A throwable that indicates the cause of the target future.
     */
    private Throwable  taskCause;

    /**
     * An implementation of Mockito answer that blocks the calling thread.
     */
    private static final class DeadAnswer implements Answer<Long> {
        /**
         * The number of milliseconds to block.
         */
        private final long  delay;

        /**
         * Set true if the thread was interrupted.
         */
        private boolean  interrupted;

        /**
         * Construct a new instance.
         *
         * @param msec  The number of milliseconds to block the calling thread.
         */
        private DeadAnswer(long msec) {
            delay = msec;
        }

        /**
         * Determine whether the thread was interrupted or not.
         *
         * @return  {@code true} only if the thread was interrupted.
         */
        private boolean wasInterrupted() {
            return interrupted;
        }

        // Answer

        /**
         * Invoked when the target method is invoked.
         *
         * @param inv  Information about the invocation.
         * @return  Always {@code null}.
         */
        @Override
        public Long answer(InvocationOnMock inv) {
            try {
                Thread.sleep(delay);
                unexpected();
            } catch (InterruptedException e) {
                interrupted = true;
            }

            return null;
        }
    }

    /**
     * A task that should never be executed.
     */
    private static final class DeadTask extends DelayedTask {
        /**
         * Construct a new istance.
         *
         * @param msec  The number of milliseconds to block the task thread.
         */
        private DeadTask(long msec) {
            super(msec);
        }

        // DelayedTask

        /**
         * Throws an assertion error.
         */
        @Override
        public void execute() {
            unexpected();
        }
    }

    /**
     * Initialize the test environment.
     */
    @Before
    public void setUp() {
        taskResult = null;
        taskCause = null;
    }

    /**
     * Test case for successful completion.
     *
     * <ul>
     *   <li>A task is implemented by a {@link Callable}.</li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testSuccessCallable() throws Exception {
        initMocks(this);
        Long result = System.currentTimeMillis();
        when(callable.call()).thenReturn(result);

        VTNFutureTask<Long> task = new VTNFutureTask<>(callable);
        Futures.addCallback(task, this);
        verifyNotComplete(task);

        // Execute the task.
        DelayedRunnableTask r = new DelayedRunnableTask(task, 500L);
        Thread t = new Thread(r);
        t.start();
        verifySuccess(task, 10L, result, t);
        verify(callable).call();
        verifyNoMoreInteractions(callable);
        assertEquals(false, r.wasInterrupted());
    }

    /**
     * Test case for successful completion.
     *
     * <ul>
     *   <li>A task is implemented by a {@link Runnable}.</li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testSuccessRunnable() throws Exception {
        Runnable runnable = mock(Runnable.class);
        Long result = System.currentTimeMillis();

        VTNFutureTask<Long> task = new VTNFutureTask<>(runnable, result);
        Futures.addCallback(task, this);
        verifyNotComplete(task);

        // Execute the task.
        DelayedRunnableTask r = new DelayedRunnableTask(task, 500L);
        Thread t = new Thread(r);
        t.start();
        verifySuccess(task, 10L, result, t);
        verify(runnable).run();
        verifyNoMoreInteractions(runnable);
        assertEquals(false, r.wasInterrupted());
    }

    /**
     * Test case for abnormal completion.
     *
     * <ul>
     *   <li>A task is implemented by a {@link Callable}.</li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testFailureCallable() throws Exception {
        initMocks(this);
        IllegalStateException cause =
            new IllegalStateException("testFailureCallable");
        when(callable.call()).thenThrow(cause);

        VTNFutureTask<Long> task = new VTNFutureTask<>(callable);
        Futures.addCallback(task, this);
        verifyNotComplete(task);

        // Execute the task.
        DelayedRunnableTask r = new DelayedRunnableTask(task, 500L);
        Thread t = new Thread(r);
        t.start();
        verifyFailure(task, 10L, cause, t);
        verify(callable).call();
        verifyNoMoreInteractions(callable);
        assertEquals(false, r.wasInterrupted());
    }

    /**
     * Test case for abnormal completion.
     *
     * <ul>
     *   <li>A task is implemented by a {@link Runnable}.</li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testFailureRunnable() throws Exception {
        Runnable runnable = mock(Runnable.class);
        IllegalStateException cause =
            new IllegalStateException("testFailureRunnable");
        doThrow(cause).when(runnable).run();

        VTNFutureTask<Long> task = new VTNFutureTask<>(runnable, null);
        Futures.addCallback(task, this);
        verifyNotComplete(task);

        // Execute the task.
        DelayedRunnableTask r = new DelayedRunnableTask(task, 500L);
        Thread t = new Thread(r);
        t.start();
        verifyFailure(task, 10L, cause, t);
        verify(runnable).run();
        verifyNoMoreInteractions(runnable);
        assertEquals(false, r.wasInterrupted());
    }

    /**
     * Test case for cancellation.
     *
     * <ul>
     *   <li>A task is implemented by a {@link Callable}.</li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testCancelCallable() throws Exception {
        initMocks(this);
        DeadAnswer ans = new DeadAnswer(10000000L);
        when(callable.call()).thenAnswer(ans);

        VTNFutureTask<Long> task = new VTNFutureTask<>(callable);
        Futures.addCallback(task, this);
        verifyNotComplete(task);

        // Execute the task.
        Thread tt = new Thread(task);
        tt.start();

        // Cancel the task.
        Thread t = new Thread(new CancelFutureTask(task, 500L));
        t.start();
        verifyCancel(task, 10L, t);

        // The task thread should be interrupted.
        tt.join(3000L);
        assertEquals(false, tt.isAlive());
        assertEquals(true, ans.wasInterrupted());
    }

    /**
     * Test case for cancellation.
     *
     * <ul>
     *   <li>A task is implemented by a {@link Runnable}.</li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testCancelRunnable() throws Exception {
        DeadTask runnable = new DeadTask(10000000L);
        VTNFutureTask<Long> task = new VTNFutureTask<>(runnable, null);
        Futures.addCallback(task, this);
        verifyNotComplete(task);

        // Execute the task.
        Thread tt = new Thread(task);
        tt.start();

        // Cancel the task.
        Thread t = new Thread(new CancelFutureTask(task, 500L));
        t.start();
        verifyCancel(task, 10L, t);

        // The task thread should be interrupted.
        tt.join(3000L);
        assertEquals(false, tt.isAlive());
        assertEquals(true, runnable.wasInterrupted());
    }

    /**
     * Ensure that the future callback is invoked immediately if the future
     * task already completed successfully.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testSuccessCallback() throws Exception {
        initMocks(this);
        Long result = System.currentTimeMillis();
        when(callable.call()).thenReturn(result);

        // Create a completed task.
        VTNFutureTask<Long> task = new VTNFutureTask<>(callable);
        verifyNotComplete(task);
        task.run();
        verifySuccess(task, 1L, result);
        assertEquals(null, taskResult);
        assertEquals(null, taskCause);

        // Add callback.
        Futures.addCallback(task, this);
        verifySuccess(task, 1L, result, null);
    }

    /**
     * Ensure that the future callback is invoked immediately if the future
     * task already failed.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testFailureCallback() throws Exception {
        initMocks(this);
        IllegalStateException cause =
            new IllegalStateException("testFailureCallback");
        when(callable.call()).thenThrow(cause);

        // Create a failed task.
        VTNFutureTask<Long> task = new VTNFutureTask<>(callable);
        verifyNotComplete(task);
        task.run();
        verifyFailure(task, 1L, cause);

        // Add callback.
        Futures.addCallback(task, this);
        verifyFailure(task, 1L, cause, null);
    }

    /**
     * Ensure that the future callback is invoked immediately if the future
     * task was already cancelled.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testCancelCallback() throws Exception {
        initMocks(this);
        when(callable.call()).thenReturn(null);

        // Create a canceled task.
        VTNFutureTask<Long> task = new VTNFutureTask<>(callable);
        verifyNotComplete(task);
        assertEquals(true, task.cancel(false));
        verifyCancel(task, 1L);

        // Add callback.
        Futures.addCallback(task, this);
        verifyCancel(task, 1L, null);
    }

    /**
     * Ensure that the specified VTN future does not complete yet.
     *
     * @param future  A {@link VTNFutureTask} instance to be checked.
     * @throws Exception  An error occurred.
     */
    private void verifyNotComplete(VTNFutureTask<Long> future)
        throws Exception {
        TestBase.verifyNotComplete(future);
        assertEquals(null, taskResult);
        assertEquals(null, taskCause);
    }

    /**
     * Ensure that the given future completes successfully.
     *
     * @param future  A {@link VTNFutureTask} instance to be checked.
     * @param nsec    The number of seconds to wait for the result.
     * @param result  The expected result of the future.
     * @throws Exception  An error occurred.
     */
    private void verifySuccess(VTNFutureTask<Long> future, long nsec,
                               Long result) throws Exception {
        assertEquals(result, future.checkedGet(nsec, TimeUnit.SECONDS));
        assertEquals(result, future.checkedGet());
        assertEquals(result, future.get(1L, TimeUnit.NANOSECONDS));
        assertEquals(result, future.get());
        assertEquals(false, future.isCancelled());
        assertEquals(true, future.isDone());
    }

    /**
     * Ensure that the given future completes successfully.
     *
     * @param future  A {@link VTNFutureTask} instance to be checked.
     * @param nsec    The number of seconds to wait for the result.
     * @param result  The expected result of the future.
     * @param t       A thread to be joined.
     * @throws Exception  An error occurred.
     */
    private void verifySuccess(VTNFutureTask<Long> future, long nsec,
                               Long result, Thread t)
        throws Exception {
        verifySuccess(future, nsec, result);

        if (t != null) {
            // Ensure that the specified thread is terminated.
            t.join(3000L);
        }

        assertEquals(result, taskResult);
        assertEquals(null, taskCause);
    }

    /**
     * Ensure that the given future fails.
     *
     * @param future  A {@link VTNFutureTask} instance to be checked.
     * @param nsec    The number of seconds to wait for completion.
     * @param cause   A throwable that indicates the cause of failure.
     * @throws Exception  An error occurred.
     */
    private void verifyFailure(VTNFutureTask<Long> future, long nsec,
                               Throwable cause) throws Exception {
        try {
            future.checkedGet(nsec, TimeUnit.SECONDS);
            unexpected();
        } catch (VTNException e) {
            assertEquals(VtnErrorTag.INTERNALERROR, e.getVtnErrorTag());
            assertEquals(cause, e.getCause());
        }

        try {
            future.checkedGet();
            unexpected();
        } catch (VTNException e) {
            assertEquals(VtnErrorTag.INTERNALERROR, e.getVtnErrorTag());
            assertEquals(cause, e.getCause());
        }

        try {
            future.get(1L, TimeUnit.NANOSECONDS);
            unexpected();
        } catch (ExecutionException e) {
            assertEquals(cause, e.getCause());
        }

        try {
            future.get();
            unexpected();
        } catch (ExecutionException e) {
            assertEquals(cause, e.getCause());
        }

        assertEquals(false, future.isCancelled());
        assertEquals(true, future.isDone());
    }

    /**
     * Ensure that the given future fails.
     *
     * @param future  A {@link VTNFutureTask} instance to be checked.
     * @param nsec    The number of seconds to wait for completion.
     * @param cause   A throwable that indicates the cause of failure.
     * @param t       A thread to be joined.
     * @throws Exception  An error occurred.
     */
    private void verifyFailure(VTNFutureTask<Long> future, long nsec,
                               Throwable cause, Thread t) throws Exception {
        verifyFailure(future, nsec, cause);

        if (t != null) {
            // Ensure that the specified thread is terminated.
            t.join(3000L);
        }

        assertEquals(null, taskResult);
        assertEquals(cause, taskCause);
    }

    /**
     * Ensure that the given target future is canceled.
     *
     * @param future  A {@link VTNFutureTask} instance to be checked.
     * @param nsec    The number of seconds to wait for completion.
     * @throws Exception  An error occurred.
     */
    private void verifyCancel(VTNFutureTask<Long> future, long nsec)
        throws Exception {
        try {
            future.checkedGet(nsec, TimeUnit.SECONDS);
            unexpected();
        } catch (VTNException e) {
            assertEquals(VtnErrorTag.INTERNALERROR, e.getVtnErrorTag());
            Throwable cause = e.getCause();
            assertEquals(CancellationException.class, cause.getClass());
        }

        try {
            future.checkedGet();
            unexpected();
        } catch (VTNException e) {
            assertEquals(VtnErrorTag.INTERNALERROR, e.getVtnErrorTag());
            Throwable cause = e.getCause();
            assertEquals(CancellationException.class, cause.getClass());
        }

        try {
            future.get(1L, TimeUnit.NANOSECONDS);
            unexpected();
        } catch (CancellationException e) {
        }

        try {
            future.get();
            unexpected();
        } catch (CancellationException e) {
        }

        assertEquals(true, future.isCancelled());
        assertEquals(true, future.isDone());
    }

    /**
     * Ensure that the given target future is canceled.
     *
     * @param future  A {@link VTNFutureTask} instance to be checked.
     * @param nsec    The number of seconds to wait for completion.
     * @param t       A thread to be joined.
     * @throws Exception  An error occurred.
     */
    private void verifyCancel(VTNFutureTask<Long> future, long nsec, Thread t)
        throws Exception {
        verifyCancel(future, nsec);

        if (t != null) {
            // Ensure that the specified thread is terminated.
            t.join(3000L);
        }

        assertEquals(null, taskResult);
        assertNotNull(taskCause);
        assertEquals(CancellationException.class, taskCause.getClass());
    }

    // FutureCallback

    /**
     * Invoked when the target future has completed successfully.
     *
     * @param result  The result of the target future.
     */
    @Override
    public void onSuccess(Long result) {
        assertEquals(null, taskResult);
        assertEquals(null, taskCause);
        taskResult = result;
    }

    /**
     * Invoked when the target future has completed abnormally.
     *
     * @param cause  A throwable that indicates the cause of failure.
     */
    @Override
    public void onFailure(Throwable cause) {
        assertEquals(null, taskResult);
        assertEquals(null, taskCause);
        taskCause = cause;
    }
}
