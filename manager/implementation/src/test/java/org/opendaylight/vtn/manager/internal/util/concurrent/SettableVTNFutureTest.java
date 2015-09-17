/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.concurrent;

import java.util.ArrayList;
import java.util.Collections;
import java.util.Iterator;
import java.util.List;
import java.util.concurrent.CancellationException;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.TimeoutException;
import java.util.concurrent.atomic.AtomicInteger;

import org.junit.Assert;
import org.junit.Test;

import com.google.common.util.concurrent.Futures;
import com.google.common.util.concurrent.FutureCallback;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.TestBase;

import org.opendaylight.controller.md.sal.common.api.data.OptimisticLockFailedException;
import org.opendaylight.controller.md.sal.common.api.data.TransactionCommitFailedException;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnErrorTag;

/**
 * JUnit test for {@link SettableVTNFuture}.
 */
public class SettableVTNFutureTest extends TestBase {
    /**
     * The number of milliseconds to sleep for a short delay.
     */
    private static final long  SHORT_DELAY = 100L;

    /**
     * Future for test.
     *
     * @param <T>  The type of the value returned by the future.
     */
    private static class TestFuture<T> extends SettableVTNFuture<T> {
        /**
         * The number of {@link #onFutureSucceeded(Object)} call.
         */
        private final AtomicInteger  succeededCount = new AtomicInteger();

        /**
         * The number of {@link #onFutureFailed(Throwable)} call.
         */
        private final AtomicInteger  failedCount = new AtomicInteger();

        /**
         * The result of the future.
         */
        private T  futureResult;

        /**
         * The cause of the failure.
         */
        private Throwable  futureCause;

        /**
         * Return the result of the task.
         *
         * @return  The result of the task.
         */
        private T getFutureResult() {
            return futureResult;
        }

        /**
         * Return the cause of the failure.
         *
         * @return  A {@link Throwable}.
         */
        private Throwable getFutureCause() {
            return futureCause;
        }

        /**
         * Return the number of {@link #onFutureSucceeded(Object)} call.
         *
         * @return  The number of {@link #onFutureSucceeded(Object)} call.
         */
        private int getSucceededCount() {
            return succeededCount.get();
        }

        /**
         * Return the number of {@link #onFutureFailed(Throwable)} call.
         *
         * @return  The number of {@link #onFutureFailed(Throwable)} call.
         */
        private int getFailedCount() {
            return failedCount.get();
        }

        // SettableVTNFuture

        /**
         * {@inheritDoc}
         */
        @Override
        public void onFutureSucceeded(T result) {
            futureResult = result;
            succeededCount.incrementAndGet();
        }

        /**
         * {@inheritDoc}
         */
        @Override
        public void onFutureFailed(Throwable cause) {
            futureCause = cause;
            failedCount.incrementAndGet();
        }
    }

    /**
     * Future callback for test.
     *
     * @param <T>  The type of the value returned by the future.
     */
    private static class TestCallback<T> implements FutureCallback<T> {
        /**
         * The future to be tested.
         */
        private final TestFuture<T>  future;

        /**
         * The number of {@link #onSuccess(Object)} call.
         */
        private final AtomicInteger  succeededCount = new AtomicInteger();

        /**
         * The number of {@link #onFailure(Throwable)} call.
         */
        private final AtomicInteger  failedCount = new AtomicInteger();

        /**
         * The result of the future.
         */
        private T  taskResult;

        /**
         * The cause of the failure.
         */
        private Throwable  taskCause;

        /**
         * Construct a new instance.
         *
         * @param f  The future to be tested.
         */
        protected TestCallback(TestFuture<T> f) {
            future = f;
            Futures.addCallback(f, this);
        }

        /**
         * Return the future to be tested.
         *
         * @return  The future to be tested.
         */
        protected TestFuture<T> getFuture() {
            return future;
        }

        /**
         * Return the result of the task.
         *
         * @return  The result of the task.
         */
        protected T getTaskResult() {
            return taskResult;
        }

        /**
         * Return the cause of the failure.
         *
         * @return  A {@link Throwable}.
         */
        protected Throwable getTaskCause() {
            return taskCause;
        }

        /**
         * Return the number of {@link #onSuccess(Object)} call.
         *
         * @return  The number of {@link #onSuccess(Object)} call.
         */
        protected int getSucceededCount() {
            return succeededCount.get();
        }

        /**
         * Return the number of {@link #onFailure(Throwable)} call.
         *
         * @return  The number of {@link #onFailure(Throwable)} call.
         */
        protected int getFailedCount() {
            return failedCount.get();
        }

        // FutureCallback

        /**
         * Invoked when the target task has successfuly completed.
         *
         * @param result  The result of the target task.
         */
        @Override
        public void onSuccess(T result) {
            // FutureCallback must be invoked after SettableVTNFuture callback.
            Assert.assertEquals(1, future.getSucceededCount());
            Assert.assertSame(result, future.getFutureResult());
            Assert.assertEquals(0, future.getFailedCount());
            Assert.assertEquals(null, future.getFutureCause());

            taskResult = result;
            succeededCount.incrementAndGet();
        }

        /**
         * Invoked when the target task has failed.
         *
         * @param cause  A {@link Throwable} that indicates the cause of
         *               failure.
         */
        @Override
        public void onFailure(Throwable cause) {
            // FutureCallback must be invoked after SettableVTNFuture callback.
            Assert.assertEquals(0, future.getSucceededCount());
            Assert.assertEquals(null, future.getFutureResult());
            Assert.assertEquals(1, future.getFailedCount());
            if (cause instanceof CancellationException) {
                verifyCanceled(cause);
            } else {
                Assert.assertSame(cause, future.getFutureCause());
            }

            taskCause = cause;
            failedCount.incrementAndGet();
        }
    }

    /**
     * A thread that tries to wait for completion of the given future.
     *
     * @param <T>  The type of the value returned by the future.
     */
    private abstract static class AbstractGetter<T> extends Thread {
        /**
         * The future to be tested.
         */
        private final VTNFuture<T>  future;

        /**
         * The result of the future.
         */
        private T  taskResult;

        /**
         * The cause of the failure.
         */
        private Throwable  taskCause;

        /**
         * Contruct a new instance.
         *
         * @param f  The future to be tested.
         */
        private AbstractGetter(VTNFuture<T> f) {
            future = f;
        }

        /**
         * Return the future to be tested.
         *
         * @return  The future to be tested.
         */
        protected VTNFuture<T> getFuture() {
            return future;
        }

        /**
         * Return the result of the task.
         *
         * @return  The result of the task.
         */
        private T getTaskResult() {
            return taskResult;
        }

        /**
         * Return the cause of the failure.
         *
         * @return  A {@link Throwable}.
         */
        private Throwable getTaskCause() {
            return taskCause;
        }

        /**
         * Attempt to get the result of the future.
         *
         * @return  The result of the future.
         * @throws Exception  An error occurred.
         */
        protected abstract T getResult() throws Exception;

        /**
         * Determine whether this class uses checked version of getter method.
         *
         * @return  {@code true} only if this method uses checked version of
         *          getter method.
         */
        protected abstract boolean isChecked();

        // Runnable

        /**
         * Attempt to get the result of the future.
         */
        @Override
        public final void run() {
            try {
                taskResult = getResult();
            } catch (Exception e) {
                taskCause = e;
            }
        }
    }

    /**
     * A runnable that tries to get the value using {@link VTNFuture#get()}.
     *
     * @param <T>  The type of the value returned by the future.
     */
    private static final class Getter<T> extends AbstractGetter<T> {
        /**
         * Contruct a new instance.
         *
         * @param f  The future to be tested.
         */
        private Getter(VTNFuture<T> f) {
            super(f);
        }

        /**
         * {@inheritDoc}
         */
        @Override
        protected T getResult() throws Exception {
            return getFuture().get();
        }

        /**
         * {@inheritDoc}
         */
        @Override
        protected boolean isChecked() {
            return false;
        }
    }

    /**
     * A runnable that tries to get the value using
     * {@link VTNFuture#get(long, TimeUnit)}.
     *
     * @param <T>  The type of the value returned by the future.
     */
    private static final class TimedGetter<T> extends AbstractGetter<T> {
        /**
         * Contruct a new instance.
         *
         * @param f  The future to be tested.
         */
        private TimedGetter(VTNFuture<T> f) {
            super(f);
        }

        /**
         * {@inheritDoc}
         */
        @Override
        protected T getResult() throws Exception {
            return getFuture().get(10000L, TimeUnit.SECONDS);
        }

        /**
         * {@inheritDoc}
         */
        @Override
        protected boolean isChecked() {
            return false;
        }
    }

    /**
     * A runnable that tries to get the value using
     * {@link VTNFuture#checkedGet()}.
     *
     * @param <T>  The type of the value returned by the future.
     */
    private static final class CheckedGetter<T> extends AbstractGetter<T> {
        /**
         * Contruct a new instance.
         *
         * @param f  The future to be tested.
         */
        private CheckedGetter(VTNFuture<T> f) {
            super(f);
        }

        /**
         * {@inheritDoc}
         */
        @Override
        protected T getResult() throws Exception {
            return getFuture().checkedGet();
        }

        /**
         * {@inheritDoc}
         */
        @Override
        protected boolean isChecked() {
            return true;
        }
    }

    /**
     * A runnable that tries to get the value using
     * {@link VTNFuture#checkedGet(long, TimeUnit)}.
     *
     * @param <T>  The type of the value returned by the future.
     */
    private static final class TimedCheckedGetter<T>
        extends AbstractGetter<T> {
        /**
         * Contruct a new instance.
         *
         * @param f  The future to be tested.
         */
        private TimedCheckedGetter(VTNFuture<T> f) {
            super(f);
        }

        /**
         * {@inheritDoc}
         */
        @Override
        protected T getResult() throws Exception {
            return getFuture().checkedGet(10000L, TimeUnit.SECONDS);
        }

        /**
         * {@inheritDoc}
         */
        @Override
        protected boolean isChecked() {
            return true;
        }
    }

    /**
     * Test environment using multiple threads.
     *
     * @param <T>  The type of the value returned by the future.
     */
    private static final class TestEnv<T> extends TestCallback<T> {
        /**
         * A list {@link AbstractGetter} instances.
         */
        private final List<AbstractGetter<T>>  getters = new ArrayList<>();

        /**
         * Construct a new instance.
         *
         * @param f  The future to be tested.
         */
        private TestEnv(TestFuture<T> f) {
            super(f);
            Collections.addAll(getters, new Getter<T>(f),
                               new TimedGetter<T>(f), new CheckedGetter<T>(f),
                               new TimedCheckedGetter<T>(f));
            for (AbstractGetter<T> getter: getters) {
                getter.start();
            }
        }

        /**
         * Return an immutable list of {@link AbstractGetter} instances.
         *
         * @return  A list of {@link AbstractGetter} instances.
         */
        private List<AbstractGetter<T>> getGetters() {
            return Collections.unmodifiableList(getters);
        }
    }

    /**
     * A pair of boolean values that determines how to cancel the future.
     */
    private static class CancelParam {
        /**
         * Determine whether the task thread needs to be interrupted or not.
         */
        private boolean  interrupt;

        /**
         * Determine whether the calling thread needs to be set to the
         * test future as the task thread.
         */
        private boolean  taskThread;

        /**
         * Construct a new instance.
         *
         * @param intr  {@code true} means that the task thread needs to be
         *              interrupted.
         * @param thr   {@code true} means that the calling thread needs to be
         *              set to the test future as the task thread.
         */
        private CancelParam(boolean intr, boolean thr) {
            interrupt = intr;
            taskThread = thr;
        }

        /**
         * Determine whether the task thread needs to be interrupted on
         * cancellation.
         *
         * @return  {@code true} only if the task thread needs to be
         *          interrupted on future cancellation.
         */
        private boolean isInterrupt() {
            return interrupt;
        }

        /**
         * Determine whether the calling thread needs to be set to the test
         * future as the task thread.
         *
         * @return  {@code true} only if the calling thread needs to be set
         *          to the test future.
         */
        private boolean isTaskThread() {
            return taskThread;
        }
    }


    /**
     * Verify the exception thrown by the future.
     *
     * @param cause   The cause of failure thrown by the future.
     * @param actual  Actual cause of failure.
     */
    private static void verifyCause(Throwable cause, Throwable actual) {
        assertTrue(cause instanceof ExecutionException);
        assertEquals("Caught an exception while executing the task.",
                     cause.getMessage());
        assertSame(actual, cause.getCause());
    }

    /**
     * Verify the exception thrown by the future.
     *
     * @param cause   The cause of failure checked by the future.
     * @param actual  Actual cause of failure.
     */
    private static void verifyCheckedCause(Throwable cause, Throwable actual) {
        assertTrue(cause instanceof VTNException);
        if (actual instanceof VTNException) {
            assertEquals(actual, cause);
            return;
        }

        VTNException ve = (VTNException)cause;
        VtnErrorTag etag;
        if (actual instanceof OptimisticLockFailedException) {
            etag = VtnErrorTag.CONFLICT;
        } else if (actual instanceof TimeoutException) {
            etag = VtnErrorTag.TIMEOUT;
        } else {
            etag = VtnErrorTag.INTERNALERROR;
        }

        assertEquals(etag, ve.getVtnErrorTag());
        assertEquals(actual.getMessage(), ve.getMessage());
        assertSame(actual, cause.getCause());
    }

    /**
     * Verify the cause of exception that indicates the future was canceled.
     *
     * @param cause   The cause of failure thrown by the future.
     */
    private static void verifyCanceled(Throwable cause) {
        assertTrue(cause instanceof CancellationException);
        assertEquals(null, cause.getCause());
        assertEquals("The task was canceled.", cause.getMessage());
    }

    /**
     * Verify the cause of exception that indicates the future was canceled.
     *
     * @param cause   The cause of failure checked by the future.
     */
    private static void verifyCheckedCanceled(Throwable cause) {
        Throwable c = cause.getCause();
        verifyCanceled(c);

        assertTrue(cause instanceof VTNException);
        VTNException ve = (VTNException)cause;
        assertEquals(VtnErrorTag.INTERNALERROR, ve.getVtnErrorTag());

    }

    /**
     * Verify the cause of exception that indicates the getter method was
     * interrupted.
     *
     * @param cause   The cause of failure thrown by the future.
     */
    private static void verifyInterrupted(Throwable cause) {
        assertTrue(cause instanceof InterruptedException);
        assertEquals(null, cause.getCause());
    }

    /**
     * Verify the cause of exception that indicates the getter method was
     * interrupted.
     *
     * @param cause   The cause of failure checked by the future.
     */
    private static void verifyCheckedInterrupted(Throwable cause) {
        Throwable c = cause.getCause();
        verifyInterrupted(c);

        assertTrue(cause instanceof VTNException);
        VTNException ve = (VTNException)cause;
        assertEquals(VtnErrorTag.INTERNALERROR, ve.getVtnErrorTag());
        assertEquals("Interrupted.", ve.getMessage());

    }

    /**
     * Ensure that the future does not have the result.
     *
     * @param future  The future to be tested.
     * @param cb      The future callback to be tested.
     * @throws Exception  An error occurred.
     */
    private static void verifyFuture(TestFuture<?> future, TestCallback<?> cb)
        throws Exception {
        long timeout = 10L;
        String timeoutMsg = "The task did not complete within " + timeout +
            " nanoseconds.";

        assertEquals(false, future.isDone());
        assertEquals(false, future.isCancelled());
        assertEquals(0, future.getSucceededCount());
        assertEquals(null, future.getFutureResult());
        assertEquals(0, future.getFailedCount());
        assertEquals(null, future.getFutureCause());
        assertEquals(0, cb.getSucceededCount());
        assertEquals(null, cb.getTaskResult());
        assertEquals(0, cb.getFailedCount());
        assertEquals(null, cb.getTaskCause());

        try {
            future.get(timeout, TimeUnit.NANOSECONDS);
            unexpected();
        } catch (TimeoutException e) {
            assertEquals(timeoutMsg, e.getMessage());
        }
        try {
            future.checkedGet(timeout, TimeUnit.NANOSECONDS);
            unexpected();
        } catch (VTNException e) {
            assertEquals(VtnErrorTag.TIMEOUT, e.getVtnErrorTag());
            assertEquals(timeoutMsg, e.getMessage());
        }
    }

    /**
     * Ensure that the future has the result of the task.
     *
     * @param future  The future to be tested.
     * @param cb      The future callback to be tested.
     * @param result  The expected result of the task.
     * @param <T>     The type of the result.
     * @throws Exception  An error occurred.
     */
    private static <T> void verifyFuture(TestFuture<T> future,
                                         TestCallback<T> cb, T result)
        throws Exception {
        assertEquals(true, future.isDone());
        assertEquals(false, future.isCancelled());
        assertSame(result, future.get());
        assertSame(result, future.get(1L, TimeUnit.NANOSECONDS));
        assertSame(result, future.checkedGet());
        assertSame(result, future.checkedGet(1L, TimeUnit.NANOSECONDS));

        assertEquals(1, future.getSucceededCount());
        assertSame(result, future.getFutureResult());
        assertEquals(0, future.getFailedCount());
        assertEquals(null, future.getFutureCause());
        assertEquals(1, cb.getSucceededCount());
        assertSame(result, cb.getTaskResult());
        assertEquals(0, cb.getFailedCount());
        assertEquals(null, cb.getTaskCause());
        assertEquals(false, Thread.interrupted());
    }

    /**
     * Ensure that the future has the cause of failure.
     *
     * @param future  The future to be tested.
     * @param cb      The future callback to be tested.
     * @param cause   The expected cause of failure.
     * @throws Exception  An error occurred.
     */
    private static void verifyFuture(TestFuture<?> future,
                                     TestCallback<?> cb,
                                     Throwable cause) throws Exception {
        assertEquals(true, future.isDone());
        assertEquals(false, future.isCancelled());

        try {
            future.get();
            unexpected();
        } catch (Exception e) {
            verifyCause(e, cause);
        }

        try {
            future.get(1L, TimeUnit.NANOSECONDS);
            unexpected();
        } catch (Exception e) {
            verifyCause(e, cause);
        }

        try {
            future.checkedGet();
            unexpected();
        } catch (Exception e) {
            verifyCheckedCause(e, cause);
        }

        try {
            future.checkedGet(1L, TimeUnit.NANOSECONDS);
            unexpected();
        } catch (Exception e) {
            verifyCheckedCause(e, cause);
        }

        assertEquals(0, future.getSucceededCount());
        assertEquals(null, future.getFutureResult());
        assertEquals(1, future.getFailedCount());
        assertSame(cause, future.getFutureCause());
        assertEquals(0, cb.getSucceededCount());
        assertEquals(null, cb.getTaskResult());
        assertEquals(1, cb.getFailedCount());
        assertSame(cause, cb.getTaskCause());
        assertEquals(false, Thread.interrupted());
    }

    /**
     * Ensure that the future has already been canceled.
     *
     * @param future  The future to be tested.
     * @param cb      The future callback to be tested.
     * @throws Exception  An error occurred.
     */
    private static void verifyFutureCanceled(TestFuture<?> future,
                                             TestCallback<?> cb)
        throws Exception {
        assertEquals(true, future.isDone());
        assertEquals(true, future.isCancelled());
        assertEquals(true, future.setThread(Thread.currentThread()));
        assertEquals(false, Thread.interrupted());

        try {
            future.get();
            unexpected();
        } catch (Exception e) {
            verifyCanceled(e);
        }

        try {
            future.get(1L, TimeUnit.NANOSECONDS);
            unexpected();
        } catch (Exception e) {
            verifyCanceled(e);
        }

        try {
            future.checkedGet();
            unexpected();
        } catch (Exception e) {
            verifyCheckedCanceled(e);
        }

        try {
            future.checkedGet(1L, TimeUnit.NANOSECONDS);
            unexpected();
        } catch (Exception e) {
            verifyCheckedCanceled(e);
        }

        assertEquals(0, future.getSucceededCount());
        assertEquals(null, future.getFutureResult());
        assertEquals(1, future.getFailedCount());
        verifyCanceled(future.getFutureCause());
        assertEquals(0, cb.getSucceededCount());
        assertEquals(null, cb.getTaskResult());
        assertEquals(1, cb.getFailedCount());
        verifyCanceled(cb.getTaskCause());
        assertEquals(false, Thread.interrupted());
    }

    /**
     * Ensure that a {@link SettableVTNFuture} returns the value if the
     * value is already set.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testAlreadySet() throws Exception {
        List<Integer> values = new ArrayList<>();
        Collections.addAll(values, null, Integer.valueOf(0),
                           Integer.valueOf(1), Integer.valueOf(2),
                           Integer.valueOf(9), Integer.valueOf(12345),
                           Integer.valueOf(77777777));

        for (Integer value: values) {
            TestFuture<Integer> future = new TestFuture<>();
            assertEquals(false, future.isDone());
            assertEquals(false, future.isCancelled());
            assertEquals(true, future.set(value));

            TestCallback<Integer> cb = new TestCallback<>(future);
            for (int i = 0; i < 5; i++) {
                verifyFuture(future, cb, value);

                // Ensure that the result cannot be overwritten.
                assertEquals(false, future.set(Integer.valueOf(1111)));
                assertEquals(false, future.setException(new Exception()));
                assertEquals(false, future.cancel(true));
                assertEquals(false, future.cancel(false));
                assertEquals(false, future.setThread(Thread.currentThread()));
            }
        }
    }

    /**
     * Ensure that a {@link SettableVTNFuture} does not return the value
     * untill the value is set.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testGet() throws Exception {
        List<Integer> values = new ArrayList<>();
        Collections.addAll(values, null, Integer.valueOf(0),
                           Integer.valueOf(1), Integer.valueOf(2),
                           Integer.valueOf(9), Integer.valueOf(12345),
                           Integer.valueOf(77777777));

        // Create threads that try to get future results.
        List<TestEnv<Integer>> testEnvs = new ArrayList<>();
        for (Integer value: values) {
            TestFuture<Integer> future = new TestFuture<>();
            TestEnv<Integer> env = new TestEnv<>(future);
            verifyFuture(future, env);
            testEnvs.add(env);
        }

        sleep(SHORT_DELAY);

        Iterator<Integer> valueIterator = values.iterator();
        for (TestEnv<Integer> env: testEnvs) {
            // Ensure that the future did not return the value.
            TestFuture<Integer> future = env.getFuture();
            verifyFuture(future, env);

            List<AbstractGetter<Integer>> getters = env.getGetters();
            for (AbstractGetter<Integer> getter: getters) {
                assertTrue(getter.isAlive());
                assertEquals(null, getter.getTaskResult());
                assertEquals(null, getter.getTaskCause());
            }

            // Set the value to the future.
            Integer value = valueIterator.next();
            assertEquals(true, future.set(value));

            verifyFuture(future, env, value);

            for (AbstractGetter<Integer> getter: getters) {
                getter.join(10000L);
                assertSame(value, getter.getTaskResult());
                assertEquals(null, getter.getTaskCause());
            }

            // Ensure that the result cannot be overwritten.
            for (int i = 0; i < 5; i++) {
                assertEquals(false, future.set(Integer.valueOf(1111)));
                assertEquals(false, future.setException(new Exception()));
                assertEquals(false, future.cancel(true));
                assertEquals(false, future.cancel(false));
                assertEquals(false, future.setThread(Thread.currentThread()));
                assertEquals(false, Thread.interrupted());

                verifyFuture(future, env, value);
            }
        }
    }

    /**
     * Ensure that a {@link SettableVTNFuture} throws an exception if the
     * cause of failure is already set.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testAlreadyFailed() throws Exception {
        List<Throwable> causes = new ArrayList<>();
        Collections.addAll(
            causes, new IllegalArgumentException("Bad argument"),
            new OptimisticLockFailedException("lock failed"),
            new TransactionCommitFailedException("Transaction abort", null),
            new TimeoutException("Operation timed out."),
            new InterruptedException("Thread was interrupted."),
            new VTNException(VtnErrorTag.BADREQUEST, "Bad request"),
            new VTNException(VtnErrorTag.TIMEOUT, "Timed out"));

        for (Throwable cause: causes) {
            TestFuture<Integer> future = new TestFuture<>();
            assertEquals(false, future.isDone());
            assertEquals(false, future.isCancelled());
            assertEquals(true, future.setException(cause));

            TestCallback<Integer> cb = new TestCallback<>(future);
            for (int i = 0; i < 5; i++) {
                verifyFuture(future, cb, cause);

                // Ensure that the result cannot be overwritten.
                assertEquals(false, future.set(Integer.valueOf(1111)));
                assertEquals(false, future.setException(new Exception()));
                assertEquals(false, future.cancel(true));
                assertEquals(false, future.cancel(false));
                assertEquals(false, future.setThread(Thread.currentThread()));
            }
        }
    }

    /**
     * Ensure that a {@link SettableVTNFuture} does not return the value
     * untill the cause of failure is set.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testFailure() throws Exception {
        List<Throwable> causes = new ArrayList<>();
        Collections.addAll(
            causes, new IllegalArgumentException("Bad argument"),
            new OptimisticLockFailedException("lock failed"),
            new TransactionCommitFailedException("Transaction abort", null),
            new TimeoutException("Operation timed out."),
            new InterruptedException("Thread was interrupted."),
            new VTNException(VtnErrorTag.BADREQUEST, "Bad request"),
            new VTNException(VtnErrorTag.TIMEOUT, "Timed out"));

        // Create threads that try to get future results.
        List<TestEnv<Integer>> testEnvs = new ArrayList<>();
        for (Throwable cause: causes) {
            TestFuture<Integer> future = new TestFuture<>();
            TestEnv<Integer> env = new TestEnv<>(future);
            verifyFuture(future, env);
            testEnvs.add(env);
        }

        sleep(SHORT_DELAY);

        Iterator<Throwable> causeIterator = causes.iterator();
        for (TestEnv<Integer> env: testEnvs) {
            // Ensure that the future did not return the value.
            TestFuture<Integer> future = env.getFuture();
            verifyFuture(future, env);

            List<AbstractGetter<Integer>> getters = env.getGetters();
            for (AbstractGetter<Integer> getter: getters) {
                assertTrue(getter.isAlive());
                assertEquals(null, getter.getTaskResult());
                assertEquals(null, getter.getTaskCause());
            }

            // Set the cause of failure to the future.
            Throwable cause = causeIterator.next();
            assertEquals(true, future.setException(cause));
            verifyFuture(future, env, cause);

            for (AbstractGetter<Integer> getter: getters) {
                getter.join(10000L);
                assertEquals(null, getter.getTaskResult());

                Throwable taskCause = getter.getTaskCause();
                if (getter.isChecked()) {
                    verifyCheckedCause(taskCause, cause);
                } else {
                    verifyCause(taskCause, cause);
                }
            }

            // Ensure that the result cannot be overwritten.
            for (int i = 0; i < 5; i++) {
                assertEquals(false, future.set(Integer.valueOf(1111)));
                assertEquals(false, future.setException(new Exception()));
                assertEquals(false, future.cancel(true));
                assertEquals(false, future.cancel(false));
                assertEquals(false, future.setThread(Thread.currentThread()));
                assertEquals(false, Thread.interrupted());

                verifyFuture(future, env, cause);
            }
        }
    }

    /**
     * Ensure that a {@link SettableVTNFuture} throws a
     * {@link CancellationException} if the future is already canceled.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testAlreadyCanceled() throws Exception {
        Thread current = Thread.currentThread();
        List<CancelParam> parameters = new ArrayList<>();
        Collections.addAll(parameters, new CancelParam(true, true),
                           new CancelParam(true, false),
                           new CancelParam(false, true),
                           new CancelParam(false, false));

        for (CancelParam param: parameters) {
            TestFuture<Integer> future = new TestFuture<>();
            assertEquals(false, future.isDone());
            assertEquals(false, future.isCancelled());
            assertEquals(false, Thread.interrupted());
            if (param.isTaskThread()) {
                assertEquals(false, future.setThread(current));
            }

            assertEquals(true, future.cancel(param.isInterrupt()));
            assertEquals(param.isTaskThread() && param.isInterrupt(),
                         Thread.interrupted());

            TestCallback<Integer> cb = new TestCallback<>(future);
            for (int i = 0; i < 5; i++) {
                verifyFutureCanceled(future, cb);

                // Ensure that the result cannot be overwritten.
                assertEquals(false, future.set(Integer.valueOf(1111)));
                assertEquals(false, future.setException(new Exception()));
                assertEquals(false, future.cancel(true));
                assertEquals(false, future.cancel(false));
                assertEquals(true, future.setThread(Thread.currentThread()));
            }
        }
    }

    /**
     * Ensure that a {@link SettableVTNFuture} does not return the value
     * until the future si canceled.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testCancel() throws Exception {
        Thread current = Thread.currentThread();
        List<CancelParam> parameters = new ArrayList<>();
        Collections.addAll(parameters, new CancelParam(true, true),
                           new CancelParam(true, false),
                           new CancelParam(false, true),
                           new CancelParam(false, false));

        // Create threads that try to get future results.
        List<TestEnv<Integer>> testEnvs = new ArrayList<>();
        for (CancelParam param: parameters) {
            TestFuture<Integer> future = new TestFuture<>();
            TestEnv<Integer> env = new TestEnv<>(future);
            if (param.isTaskThread()) {
                assertEquals(false, future.setThread(current));
            }
            verifyFuture(future, env);
            testEnvs.add(env);
        }

        sleep(SHORT_DELAY);

        Iterator<CancelParam> paramIterator = parameters.iterator();
        for (TestEnv<Integer> env: testEnvs) {
            // Ensure that the future did not return the value.
            TestFuture<Integer> future = env.getFuture();
            verifyFuture(future, env);

            List<AbstractGetter<Integer>> getters = env.getGetters();
            for (AbstractGetter<Integer> getter: getters) {
                assertTrue(getter.isAlive());
                assertEquals(null, getter.getTaskResult());
                assertEquals(null, getter.getTaskCause());
            }

            // Cancel the future.
            CancelParam param = paramIterator.next();
            assertEquals(true, future.cancel(param.isInterrupt()));
            assertEquals(param.isTaskThread() && param.isInterrupt(),
                         Thread.interrupted());
            verifyFutureCanceled(future, env);

            for (AbstractGetter<Integer> getter: getters) {
                getter.join(10000L);
                assertEquals(null, getter.getTaskResult());

                Throwable taskCause = getter.getTaskCause();
                if (getter.isChecked()) {
                    verifyCheckedCanceled(taskCause);
                } else {
                    verifyCanceled(taskCause);
                }
            }

            // Ensure that the result cannot be overwritten.
            for (int i = 0; i < 5; i++) {
                assertEquals(false, future.set(Integer.valueOf(1111)));
                assertEquals(false, future.setException(new Exception()));
                assertEquals(false, future.cancel(true));
                assertEquals(false, future.cancel(false));
                assertEquals(true, future.setThread(Thread.currentThread()));
                assertEquals(false, Thread.interrupted());

                verifyFutureCanceled(future, env);
            }
        }
    }

    /**
     * Ensure that a {@link SettableVTNFuture} throws an
     * {@link InterruptedException} if the blocked thread was interrupted.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testInterrupt() throws Exception {
        // Create threads that try to get future results.
        TestFuture<Integer> future = new TestFuture<>();
        TestEnv<Integer> env = new TestEnv<>(future);
        verifyFuture(future, env);

        sleep(SHORT_DELAY);

        // Ensure that the future did not return the value.
        verifyFuture(future, env);

        List<AbstractGetter<Integer>> getters = env.getGetters();
        for (AbstractGetter<Integer> getter: getters) {
            assertTrue(getter.isAlive());
            assertEquals(null, getter.getTaskResult());
            assertEquals(null, getter.getTaskCause());
        }

        // Interrupt getter threads.
        for (AbstractGetter<Integer> getter: getters) {
            assertTrue(getter.isAlive());
            getter.interrupt();
        }

        for (AbstractGetter<Integer> getter: getters) {
            getter.join(10000L);
            assertEquals(null, getter.getTaskResult());

            Throwable taskCause = getter.getTaskCause();
            if (getter.isChecked()) {
                verifyCheckedInterrupted(taskCause);
            } else {
                verifyInterrupted(taskCause);
            }
        }

        // Future state should not be changed.
        verifyFuture(future, env);

        Integer value = Integer.valueOf(Integer.MAX_VALUE);
        assertEquals(false, future.setThread(Thread.currentThread()));
        assertEquals(true, future.set(value));
        verifyFuture(future, env, value);
    }

    /**
     * Test method for cancellation mask.
     *
     * <ul>
     *   <li>{@link SettableVTNFuture#maskCancel()}</li>
     *   <li>{@link SettableVTNFuture#unmaskCancel()}</li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testMaskCancel() throws Exception {
        Thread current = Thread.currentThread();
        Integer value = Integer.valueOf(0);
        IllegalArgumentException cause =
            new IllegalArgumentException("Bad argument");
        boolean[] bools = {true, false};

        // Set the result after unmasking the cancellation.
        TestFuture<Integer> future = new TestFuture<>();
        TestCallback<Integer> cb = new TestCallback<>(future);
        for (int i = 0; i < 5; i++) {
            assertEquals(true, future.maskCancel());
            assertEquals(true, future.maskCancel());
            assertEquals(true, future.unmaskCancel());
            assertEquals(true, future.unmaskCancel());
        }
        verifyFuture(future, cb);
        assertEquals(true, future.set(value));
        verifyFuture(future, cb, value);

        // Set the cause of failure after unmasking the cancellation.
        future = new TestFuture<>();
        cb = new TestCallback<>(future);
        for (int i = 0; i < 5; i++) {
            assertEquals(true, future.maskCancel());
            assertEquals(true, future.maskCancel());
            assertEquals(true, future.unmaskCancel());
            assertEquals(true, future.unmaskCancel());
        }
        verifyFuture(future, cb);
        assertEquals(true, future.setException(cause));
        verifyFuture(future, cb, cause);

        // Set the result with cancellation mask.
        future = new TestFuture<>();
        cb = new TestCallback<>(future);
        for (int i = 0; i < 5; i++) {
            assertEquals(true, future.maskCancel());
        }
        verifyFuture(future, cb);
        assertEquals(true, future.set(value));
        verifyFuture(future, cb, value);

        // Set the cause of failure with cancellation mask.
        future = new TestFuture<>();
        cb = new TestCallback<>(future);
        for (int i = 0; i < 5; i++) {
            assertEquals(true, future.maskCancel());
        }
        verifyFuture(future, cb);
        assertEquals(true, future.setException(cause));
        verifyFuture(future, cb, cause);

        for (boolean intr: bools) {
            // Set the result with a pending cancel request.
            future = new TestFuture<>();
            cb = new TestCallback<>(future);
            assertEquals(false, future.setThread(current));
            assertEquals(true, future.maskCancel());
            for (int i = 0; i < 5; i++) {
                assertEquals(false, future.cancel(intr));
                assertEquals(false, Thread.interrupted());
            }
            verifyFuture(future, cb);
            assertEquals(true, future.set(value));
            verifyFuture(future, cb, value);

            // Set the cause of failure with a pending cancel request.
            future = new TestFuture<>();
            cb = new TestCallback<>(future);
            assertEquals(false, future.setThread(current));
            assertEquals(true, future.maskCancel());
            for (int i = 0; i < 5; i++) {
                assertEquals(false, future.cancel(intr));
                assertEquals(false, Thread.interrupted());
            }
            verifyFuture(future, cb);
            assertEquals(true, future.setException(cause));
            verifyFuture(future, cb, cause);

            // Unmask the cancellation mask with a pending cancel request.
            future = new TestFuture<>();
            cb = new TestCallback<>(future);
            assertEquals(false, future.setThread(current));
            assertEquals(true, future.maskCancel());
            for (int i = 0; i < 5; i++) {
                assertEquals(false, future.cancel(intr));
                assertEquals(false, Thread.interrupted());
            }

            sleep(SHORT_DELAY);
            verifyFuture(future, cb);

            for (int i = 0; i < 5; i++) {
                try {
                    future.unmaskCancel();
                    unexpected();
                } catch (InterruptedException e) {
                    assertEquals("The task was interrupted.", e.getMessage());
                }

                verifyFutureCanceled(future, cb);
            }
        }
    }
}
