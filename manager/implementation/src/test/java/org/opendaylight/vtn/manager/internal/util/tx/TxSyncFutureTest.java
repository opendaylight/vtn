/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.tx;

import static org.mockito.Mockito.when;
import static org.mockito.MockitoAnnotations.initMocks;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.concurrent.CancellationException;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.TimeUnit;

import com.google.common.util.concurrent.Futures;
import com.google.common.util.concurrent.FutureCallback;

import org.junit.Before;
import org.junit.Test;
import org.junit.experimental.categories.Category;

import org.mockito.Mock;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.TxTask;
import org.opendaylight.vtn.manager.internal.util.concurrent.SettableVTNFuture;
import org.opendaylight.vtn.manager.internal.util.concurrent.VTNFuture;

import org.opendaylight.vtn.manager.internal.CancelFutureTask;
import org.opendaylight.vtn.manager.internal.FailFutureTask;
import org.opendaylight.vtn.manager.internal.InterruptTask;
import org.opendaylight.vtn.manager.internal.SetFutureTask;
import org.opendaylight.vtn.manager.internal.SlowTest;
import org.opendaylight.vtn.manager.internal.TestBase;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnErrorTag;

/**
 * JUnit test for {@link TxSyncFuture}.
 */
@Category(SlowTest.class)
public class TxSyncFutureTest extends TestBase
    implements FutureCallback<Long> {
    /**
     * The target MD-SAL datastore transaction task.
     */
    @Mock
    private TxTask<Long>  targetTask;

    /**
     * A long value returned by the target future.
     */
    private Long  taskResult;

    /**
     * A throwable that indicates the cause of the target future.
     */
    private Throwable  taskCause;

    /**
     * Initialize the test environment.
     */
    @Before
    public void setUp() {
        initMocks(this);
        taskResult = null;
        taskCause = null;
    }

    /**
     * Test case for success completion.
     *
     * <ul>
     *   <li>No background task.</li>
     *   <li>The target task completes before waiting.</li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testSuccessNoBackground1() throws Exception {
        SettableVTNFuture<Long> future = new SettableVTNFuture<>();
        when(targetTask.getBackgroundTasks()).
            thenReturn(Collections.<VTNFuture<?>>emptyList());

        TxSyncFuture<Long> sync = new TxSyncFuture<>(targetTask, future);
        Futures.addCallback(sync, this);
        verifyNotComplete(sync);

        // Interruption test.
        Thread t = new Thread(new InterruptTask(Thread.currentThread(), 1L));
        t.start();
        try {
            sync.get();
            unexpected();
        } catch (InterruptedException e) {
        }

        // Complete the task.
        Long result = System.currentTimeMillis();
        future.set(result);
        verifySuccess(sync, 1L, result, null);
    }

    /**
     * Test case for success completion.
     *
     * <ul>
     *   <li>No background task.</li>
     *   <li>The target task completes while waiting.</li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testSuccessNoBackground2() throws Exception {
        SettableVTNFuture<Long> future = new SettableVTNFuture<>();
        when(targetTask.getBackgroundTasks()).
            thenReturn(Collections.<VTNFuture<?>>emptyList());

        TxSyncFuture<Long> sync = new TxSyncFuture<>(targetTask, future);
        Futures.addCallback(sync, this);
        verifyNotComplete(sync);

        // Interruption test.
        Thread t = new Thread(new InterruptTask(Thread.currentThread(), 1L));
        t.start();
        try {
            sync.get();
            unexpected();
        } catch (InterruptedException e) {
        }

        // Complete the task on background.
        Long result = System.currentTimeMillis();
        t = new Thread(new SetFutureTask<Long>(future, result, 500L));
        t.start();
        verifySuccess(sync, 10L, result, t);
    }

    /**
     * Test case for success completion.
     *
     * <ul>
     *   <li>3 background tasks.</li>
     *   <li>
     *     The target task and all the background tasks complete before
     *     waiting.
     *   </li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testSuccessBackground1() throws Exception {
        SettableVTNFuture<Long> future = new SettableVTNFuture<>();
        List<SettableVTNFuture<Void>> bgTasks = createBackgroundTasks(3);
        when(targetTask.getBackgroundTasks()).
            thenReturn(new ArrayList<VTNFuture<?>>(bgTasks));

        TxSyncFuture<Long> sync = new TxSyncFuture<>(targetTask, future);
        Futures.addCallback(sync, this);
        verifyNotComplete(sync);

        // Interruption test.
        Thread t = new Thread(new InterruptTask(Thread.currentThread(), 1L));
        t.start();
        try {
            sync.get();
            unexpected();
        } catch (InterruptedException e) {
        }

        // Complete the task.
        Long result = System.currentTimeMillis();
        future.set(result);

        // The future should wait for completion of background tasks.
        verifyNotComplete(sync);

        // Complete all the background tasks.
        for (SettableVTNFuture<Void> f: bgTasks) {
            f.set(null);
        }

        verifySuccess(sync, 1L, result, null);
    }

    /**
     * Test case for success completion.
     *
     * <ul>
     *   <li>3 background tasks.</li>
     *   <li>
     *     All the background tasks complete before waiting.
     *   </li>
     *   <li>The target task completes while waiting.</li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testSuccessBackground2() throws Exception {
        SettableVTNFuture<Long> future = new SettableVTNFuture<>();
        List<SettableVTNFuture<Void>> bgTasks = createBackgroundTasks(3);
        when(targetTask.getBackgroundTasks()).
            thenReturn(new ArrayList<VTNFuture<?>>(bgTasks));

        TxSyncFuture<Long> sync = new TxSyncFuture<>(targetTask, future);
        Futures.addCallback(sync, this);
        verifyNotComplete(sync);

        // Interruption test.
        Thread t = new Thread(new InterruptTask(Thread.currentThread(), 1L));
        t.start();
        try {
            sync.get();
            unexpected();
        } catch (InterruptedException e) {
        }

        // Complete all the background tasks.
        for (SettableVTNFuture<Void> f: bgTasks) {
            f.set(null);
        }

        // The future should wait for completion of the target task.
        verifyNotComplete(sync);

        // Complete the task on background.
        Long result = System.currentTimeMillis();
        t = new Thread(new SetFutureTask<Long>(future, result, 500L));
        t.start();
        verifySuccess(sync, 10L, result, t);
    }

    /**
     * Test case for success completion.
     *
     * <ul>
     *   <li>3 background tasks.</li>
     *   <li>The target task completes before waiting.</li>
     *   <li>One background task completes while waiting.</li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testSuccessBackground3() throws Exception {
        SettableVTNFuture<Long> future = new SettableVTNFuture<>();
        List<SettableVTNFuture<Void>> bgTasks = createBackgroundTasks(3);
        when(targetTask.getBackgroundTasks()).
            thenReturn(new ArrayList<VTNFuture<?>>(bgTasks));

        TxSyncFuture<Long> sync = new TxSyncFuture<>(targetTask, future);
        Futures.addCallback(sync, this);
        verifyNotComplete(sync);

        // Interruption test.
        Thread t = new Thread(new InterruptTask(Thread.currentThread(), 1L));
        t.start();
        try {
            sync.get();
            unexpected();
        } catch (InterruptedException e) {
        }

        // Complete the target task.
        Long result = System.currentTimeMillis();
        future.set(result);

        // The future should wait for completion of the target task.
        verifyNotComplete(sync);

        // Complete all the background tasks except one task.
        SettableVTNFuture<Void> bg = null;
        for (SettableVTNFuture<Void> f: bgTasks) {
            if (bg == null) {
                bg = f;
            } else {
                f.set(null);
            }
        }

        // The future should wait for completion of the background task.
        verifyNotComplete(sync);

        // Complete the background task on background.
        t = new Thread(new SetFutureTask<Void>(bg, null, 500L));
        t.start();
        verifySuccess(sync, 10L, result, t);
    }

    /**
     * Test case for success completion.
     *
     * <ul>
     *   <li>3 background tasks.</li>
     *   <li>The target task completes before waiting.</li>
     *   <li>One background task fails while waiting.</li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testSuccessBackground4() throws Exception {
        SettableVTNFuture<Long> future = new SettableVTNFuture<>();
        List<SettableVTNFuture<Void>> bgTasks = createBackgroundTasks(3);
        when(targetTask.getBackgroundTasks()).
            thenReturn(new ArrayList<VTNFuture<?>>(bgTasks));

        TxSyncFuture<Long> sync = new TxSyncFuture<>(targetTask, future);
        Futures.addCallback(sync, this);
        verifyNotComplete(sync);

        // Interruption test.
        Thread t = new Thread(new InterruptTask(Thread.currentThread(), 1L));
        t.start();
        try {
            sync.get();
            unexpected();
        } catch (InterruptedException e) {
        }

        // Complete the target task.
        Long result = System.currentTimeMillis();
        future.set(result);

        // The future should wait for completion of the target task.
        verifyNotComplete(sync);

        // Complete all the background tasks except one task.
        SettableVTNFuture<Void> bg = null;
        for (SettableVTNFuture<Void> f: bgTasks) {
            if (bg == null) {
                bg = f;
            } else {
                f.set(null);
            }
        }

        // The future should wait for completion of the background task.
        verifyNotComplete(sync);

        // Make the background task fail on background.
        // The result of background task should never affect the result of
        // the target future.
        IllegalStateException cause =
            new IllegalStateException("testSuccessBackground4");
        t = new Thread(new FailFutureTask(bg, cause, 500L));
        t.start();
        verifySuccess(sync, 10L, result, t);
    }

    /**
     * Test case for success completion.
     *
     * <ul>
     *   <li>3 background tasks.</li>
     *   <li>The target task completes before waiting.</li>
     *   <li>One background task is canceled while waiting.</li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testSuccessBackground5() throws Exception {
        SettableVTNFuture<Long> future = new SettableVTNFuture<>();
        List<SettableVTNFuture<Void>> bgTasks = createBackgroundTasks(3);
        when(targetTask.getBackgroundTasks()).
            thenReturn(new ArrayList<VTNFuture<?>>(bgTasks));

        TxSyncFuture<Long> sync = new TxSyncFuture<>(targetTask, future);
        Futures.addCallback(sync, this);
        verifyNotComplete(sync);

        // Interruption test.
        Thread t = new Thread(new InterruptTask(Thread.currentThread(), 1L));
        t.start();
        try {
            sync.get();
            unexpected();
        } catch (InterruptedException e) {
        }

        // Complete the target task.
        Long result = System.currentTimeMillis();
        future.set(result);

        // The future should wait for completion of the target task.
        verifyNotComplete(sync);

        // Complete one background task.
        SettableVTNFuture<Void> bg = bgTasks.get(1);
        bg.set(null);

        // Make one background task fail.
        bg = bgTasks.get(2);
        bg.setException(new IllegalStateException("testSuccessBackground5"));

        // The future should wait for completion of the background task.
        verifyNotComplete(sync);

        // Cancel the background task on background.
        // The result of background task should never affect the result of
        // the target future.
        bg = bgTasks.get(0);
        t = new Thread(new CancelFutureTask(bg, 500L));
        t.start();
        verifySuccess(sync, 10L, result, t);
    }

    /**
     * Test case for abnormal completion.
     *
     * <ul>
     *   <li>No background task.</li>
     *   <li>The target task fails before waiting.</li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testFailNoBackground1() throws Exception {
        SettableVTNFuture<Long> future = new SettableVTNFuture<>();
        when(targetTask.getBackgroundTasks()).
            thenReturn(Collections.<VTNFuture<?>>emptyList());

        TxSyncFuture<Long> sync = new TxSyncFuture<>(targetTask, future);
        Futures.addCallback(sync, this);
        verifyNotComplete(sync);

        // Interruption test.
        Thread t = new Thread(new InterruptTask(Thread.currentThread(), 1L));
        t.start();
        try {
            sync.get();
            unexpected();
        } catch (InterruptedException e) {
        }

        // Make the target task fail.
        IllegalStateException cause =
            new IllegalStateException("testFailNoBackground1");
        future.setException(cause);
        verifyFailure(sync, 1L, cause, null);
    }

    /**
     * Test case for abnormal completion.
     *
     * <ul>
     *   <li>No background task.</li>
     *   <li>The target task fails while waiting.</li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testFailNoBackground2() throws Exception {
        SettableVTNFuture<Long> future = new SettableVTNFuture<>();
        when(targetTask.getBackgroundTasks()).
            thenReturn(Collections.<VTNFuture<?>>emptyList());

        TxSyncFuture<Long> sync = new TxSyncFuture<>(targetTask, future);
        Futures.addCallback(sync, this);
        verifyNotComplete(sync);

        // Interruption test.
        Thread t = new Thread(new InterruptTask(Thread.currentThread(), 1L));
        t.start();
        try {
            sync.get();
            unexpected();
        } catch (InterruptedException e) {
        }

        // Make the target task fail on background.
        IllegalStateException cause =
            new IllegalStateException("testFailNoBackground2");
        t = new Thread(new FailFutureTask(future, cause, 500L));
        t.start();
        verifyFailure(sync, 10L, cause, t);
    }

    /**
     * Test case for abnormal completion.
     *
     * <ul>
     *   <li>3 background tasks.</li>
     *   <li>The target task fails before waiting.</li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testFailBackground1() throws Exception {
        SettableVTNFuture<Long> future = new SettableVTNFuture<>();
        List<SettableVTNFuture<Void>> bgTasks = createBackgroundTasks(3);
        when(targetTask.getBackgroundTasks()).
            thenReturn(new ArrayList<VTNFuture<?>>(bgTasks));

        TxSyncFuture<Long> sync = new TxSyncFuture<>(targetTask, future);
        Futures.addCallback(sync, this);
        verifyNotComplete(sync);

        // Interruption test.
        Thread t = new Thread(new InterruptTask(Thread.currentThread(), 1L));
        t.start();
        try {
            sync.get();
            unexpected();
        } catch (InterruptedException e) {
        }

        // Make the target task fail.
        IllegalStateException cause =
            new IllegalStateException("testFailBackground1");
        future.setException(cause);

        // The future should ignore results of background tasks.
        verifyFailure(sync, 1L, cause, null);

        // Complete all the background tasks.
        // This should never affect the result of the target task.
        for (SettableVTNFuture<Void> f: bgTasks) {
            f.set(null);
        }
        verifyFailure(sync, 1L, cause, null);
    }

    /**
     * Test case for abnormal completion.
     *
     * <ul>
     *   <li>3 background tasks.</li>
     *   <li>The target task fails while waiting.</li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testFailBackground2() throws Exception {
        SettableVTNFuture<Long> future = new SettableVTNFuture<>();
        List<SettableVTNFuture<Void>> bgTasks = createBackgroundTasks(3);
        when(targetTask.getBackgroundTasks()).
            thenReturn(new ArrayList<VTNFuture<?>>(bgTasks));

        TxSyncFuture<Long> sync = new TxSyncFuture<>(targetTask, future);
        Futures.addCallback(sync, this);
        verifyNotComplete(sync);

        // Interruption test.
        Thread t = new Thread(new InterruptTask(Thread.currentThread(), 1L));
        t.start();
        try {
            sync.get();
            unexpected();
        } catch (InterruptedException e) {
        }

        // Make the target task fail on background.
        IllegalStateException cause =
            new IllegalStateException("testFailBackground2");
        t = new Thread(new FailFutureTask(future, cause, 500L));
        t.start();
        verifyFailure(sync, 10L, cause, t);

        // Complete all the background tasks.
        // This should never affect the result of the target task.
        SettableVTNFuture<Void> bg = bgTasks.get(1);
        bg.set(null);
        bg = bgTasks.get(0);
        bg.setException(cause);
        bg = bgTasks.get(2);
        assertEquals(true, bg.cancel(true));
        verifyFailure(sync, 1L, cause, null);
    }

    /**
     * Test case for cancellation of the target task.
     *
     * <ul>
     *   <li>No background task.</li>
     *   <li>The target task is canceled before waiting.</li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testCancelNoBackground1() throws Exception {
        SettableVTNFuture<Long> future = new SettableVTNFuture<>();
        when(targetTask.getBackgroundTasks()).
            thenReturn(Collections.<VTNFuture<?>>emptyList());

        TxSyncFuture<Long> sync = new TxSyncFuture<>(targetTask, future);
        Futures.addCallback(sync, this);
        verifyNotComplete(sync);

        // Interruption test.
        Thread t = new Thread(new InterruptTask(Thread.currentThread(), 1L));
        t.start();
        try {
            sync.get();
            unexpected();
        } catch (InterruptedException e) {
        }

        // Cancel the target task.
        assertEquals(true, future.cancel(true));
        verifyTargetCancel(sync, 1L, null);
    }

    /**
     * Test case for cancellation of the target task.
     *
     * <ul>
     *   <li>No background task.</li>
     *   <li>The target task is canceled while waiting.</li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testCancelNoBackground2() throws Exception {
        SettableVTNFuture<Long> future = new SettableVTNFuture<>();
        when(targetTask.getBackgroundTasks()).
            thenReturn(Collections.<VTNFuture<?>>emptyList());

        TxSyncFuture<Long> sync = new TxSyncFuture<>(targetTask, future);
        Futures.addCallback(sync, this);
        verifyNotComplete(sync);

        // Interruption test.
        Thread t = new Thread(new InterruptTask(Thread.currentThread(), 1L));
        t.start();
        try {
            sync.get();
            unexpected();
        } catch (InterruptedException e) {
        }

        // Cancel the target task on background.
        t = new Thread(new CancelFutureTask(future, true, 500L));
        t.start();
        verifyTargetCancel(sync, 10L, t);
    }

    /**
     * Test case for cancellation of the target task.
     *
     * <ul>
     *   <li>3 background tasks.</li>
     *   <li>The target task is canceled before waiting.</li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testCancelBackground1() throws Exception {
        SettableVTNFuture<Long> future = new SettableVTNFuture<>();
        List<SettableVTNFuture<Void>> bgTasks = createBackgroundTasks(3);
        when(targetTask.getBackgroundTasks()).
            thenReturn(new ArrayList<VTNFuture<?>>(bgTasks));

        TxSyncFuture<Long> sync = new TxSyncFuture<>(targetTask, future);
        Futures.addCallback(sync, this);
        verifyNotComplete(sync);

        // Interruption test.
        Thread t = new Thread(new InterruptTask(Thread.currentThread(), 1L));
        t.start();
        try {
            sync.get();
            unexpected();
        } catch (InterruptedException e) {
        }

        // Cancel the target task.
        assertEquals(true, future.cancel(true));

        // The future should ignore results of background tasks.
        verifyTargetCancel(sync, 1L, null);

        // Complete all the background tasks.
        // This should never affect the result of the target task.
        for (SettableVTNFuture<Void> f: bgTasks) {
            f.set(null);
        }
        verifyTargetCancel(sync, 1L, null);
    }

    /**
     * Test case for cancellation of the target task.
     *
     * <ul>
     *   <li>3 background tasks.</li>
     *   <li>The target task is canceled while waiting.</li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testCancelBackground2() throws Exception {
        SettableVTNFuture<Long> future = new SettableVTNFuture<>();
        List<SettableVTNFuture<Void>> bgTasks = createBackgroundTasks(3);
        when(targetTask.getBackgroundTasks()).
            thenReturn(new ArrayList<VTNFuture<?>>(bgTasks));

        TxSyncFuture<Long> sync = new TxSyncFuture<>(targetTask, future);
        Futures.addCallback(sync, this);
        verifyNotComplete(sync);

        // Interruption test.
        Thread t = new Thread(new InterruptTask(Thread.currentThread(), 1L));
        t.start();
        try {
            sync.get();
            unexpected();
        } catch (InterruptedException e) {
        }

        // Cancel the target task on background.
        t = new Thread(new CancelFutureTask(future, 500L));
        t.start();
        verifyTargetCancel(sync, 10L, t);

        // Complete all the background tasks.
        // This should never affect the result of the target task.
        SettableVTNFuture<Void> bg = bgTasks.get(0);
        bg.set(null);
        bg = bgTasks.get(1);
        assertEquals(true, bg.cancel(true));
        bg = bgTasks.get(2);
        IllegalStateException cause =
            new IllegalStateException("testCancelBackground2");
        bg.setException(cause);
        verifyTargetCancel(sync, 1L, null);
    }

    /**
     * Test case for cancellation of the task synchronization.
     *
     * <ul>
     *   <li>No background task.</li>
     *   <li>The target task completes before waiting.</li>
     *   <li>
     *     The task synchronization is canceled before completion of the
     *     target task.
     *   </li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testSyncCancelNoBackground1() throws Exception {
        SettableVTNFuture<Long> future = new SettableVTNFuture<>();
        when(targetTask.getBackgroundTasks()).
            thenReturn(Collections.<VTNFuture<?>>emptyList());

        TxSyncFuture<Long> sync = new TxSyncFuture<>(targetTask, future);
        Futures.addCallback(sync, this);
        verifyNotComplete(sync);

        // Interruption test.
        Thread t = new Thread(new InterruptTask(Thread.currentThread(), 1L));
        t.start();
        try {
            sync.get();
            unexpected();
        } catch (InterruptedException e) {
        }

        // Cancel the task synchronization.
        assertEquals(false, sync.cancel(true));
        verifyNotComplete(sync);

        // Complete the task.
        Long result = System.currentTimeMillis();
        future.set(result);
        verifySuccess(sync, 1L, result, null);

        // Cancel the synchronization again.
        // This should never affect the result.
        assertEquals(false, sync.cancel(true));
        verifySuccess(sync, 1L, result, null);
    }

    /**
     * Test case for cancellation of the task synchronization.
     *
     * <ul>
     *   <li>No background task.</li>
     *   <li>The target task completes while waiting.</li>
     *   <li>The task synchronization is canceled while waiting.</li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testSyncCancelNoBackground2() throws Exception {
        SettableVTNFuture<Long> future = new SettableVTNFuture<>();
        when(targetTask.getBackgroundTasks()).
            thenReturn(Collections.<VTNFuture<?>>emptyList());

        TxSyncFuture<Long> sync = new TxSyncFuture<>(targetTask, future);
        Futures.addCallback(sync, this);
        verifyNotComplete(sync);

        // Interruption test.
        Thread t = new Thread(new InterruptTask(Thread.currentThread(), 1L));
        t.start();
        try {
            sync.get();
            unexpected();
        } catch (InterruptedException e) {
        }
        verifyNotComplete(sync);

        // Complete the task on background.
        Long result = System.currentTimeMillis();
        t = new Thread(new SetFutureTask<Long>(future, result, 500L));
        t.start();

        // Cancel the task synchronization.
        sleep(1L);
        assertEquals(false, sync.cancel(true));
        verifySuccess(sync, 10L, result, t);

        // Cancel the synchronization again.
        // This should never affect the result.
        assertEquals(false, sync.cancel(true));
        verifySuccess(sync, 1L, result, null);
    }

    /**
     * Test case for cancellation of the task synchronization.
     *
     * <ul>
     *   <li>3 background tasks.</li>
     *   <li>The target task completes before waiting.</li>
     *   <li>
     *     The task synchronization is canceled before completion of the
     *     target task.
     *   </li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testSyncCancelBackground1() throws Exception {
        SettableVTNFuture<Long> future = new SettableVTNFuture<>();
        List<SettableVTNFuture<Void>> bgTasks = createBackgroundTasks(3);
        when(targetTask.getBackgroundTasks()).
            thenReturn(new ArrayList<VTNFuture<?>>(bgTasks));

        TxSyncFuture<Long> sync = new TxSyncFuture<>(targetTask, future);
        Futures.addCallback(sync, this);
        verifyNotComplete(sync);

        // Interruption test.
        Thread t = new Thread(new InterruptTask(Thread.currentThread(), 1L));
        t.start();
        try {
            sync.get();
            unexpected();
        } catch (InterruptedException e) {
        }

        // Cancel the task synchronization.
        assertEquals(false, sync.cancel(true));
        verifyNotComplete(sync);

        // Complete the task.
        Long result = System.currentTimeMillis();
        future.set(result);
        verifySuccess(sync, 1L, result, null);

        // Cancel the synchronization again.
        // This should never affect the result.
        assertEquals(false, sync.cancel(true));

        // Complete all the background tasks.
        // This should never affect the result of the target task.
        for (SettableVTNFuture<Void> f: bgTasks) {
            f.set(null);
        }
        verifySuccess(sync, 1L, result, null);
    }

    /**
     * Test case for cancellation of the task synchronization.
     *
     * <ul>
     *   <li>3 background tasks.</li>
     *   <li>The target task completes before waiting.</li>
     *   <li>The task synchronization is canceled while waiting.</li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testSyncCancelBackground2() throws Exception {
        SettableVTNFuture<Long> future = new SettableVTNFuture<>();
        List<SettableVTNFuture<Void>> bgTasks = createBackgroundTasks(3);
        when(targetTask.getBackgroundTasks()).
            thenReturn(new ArrayList<VTNFuture<?>>(bgTasks));

        TxSyncFuture<Long> sync = new TxSyncFuture<>(targetTask, future);
        Futures.addCallback(sync, this);
        verifyNotComplete(sync);

        // Interruption test.
        Thread t = new Thread(new InterruptTask(Thread.currentThread(), 1L));
        t.start();
        try {
            sync.get();
            unexpected();
        } catch (InterruptedException e) {
        }

        // Complete the task.
        Long result = System.currentTimeMillis();
        future.set(result);

        // The future should wait for completion of the target task.
        verifyNotComplete(sync);

        // Cancel the task synchronization on background.
        t = new Thread(new CancelFutureTask(sync, false, 500L));
        t.start();
        verifySuccess(sync, 10L, result, t);

        // Complete all the background tasks.
        // This should never affect the result of the target task.
        SettableVTNFuture<Void> bg = bgTasks.get(2);
        IllegalStateException cause =
            new IllegalStateException("testSyncCancelBackground2");
        bg.setException(cause);
        bg = bgTasks.get(1);
        assertEquals(true, bg.cancel(true));
        bg = bgTasks.get(0);
        bg.set(null);

        // Cancel the synchronization again.
        // This should never affect the result.
        assertEquals(false, sync.cancel(true));
        verifySuccess(sync, 1L, result, null);
    }

    /**
     * Create a list of futures associated with the target task.
     *
     * @param num  The number of background tasks.
     * @return  A list of futures.
     */
    private List<SettableVTNFuture<Void>> createBackgroundTasks(int num) {
        List<SettableVTNFuture<Void>> list = new ArrayList<>(num);
        for (int i = 0; i < num; i++) {
            list.add(new SettableVTNFuture<Void>());
        }

        return list;
    }

    /**
     * Ensure that the specified VTN future does not complete yet.
     *
     * @param sync  A {@link TxSyncFuture} instance to be checked.
     * @throws Exception  An error occurred.
     */
    private void verifyNotComplete(TxSyncFuture<Long> sync) throws Exception {
        TestBase.verifyNotComplete(sync);
        assertEquals(null, taskResult);
        assertEquals(null, taskCause);
    }

    /**
     * Ensure that the given future completes successfully.
     *
     * @param sync    A {@link TxSyncFuture} instance to be checked.
     * @param nsec    The number of seconds to wait for the result.
     * @param result  The expected result of the future.
     * @param t       A thread to be joined.
     * @throws Exception  An error occurred.
     */
    private void verifySuccess(TxSyncFuture<Long> sync, long nsec, Long result,
                               Thread t) throws Exception {
        assertEquals(result, sync.checkedGet(nsec, TimeUnit.SECONDS));
        assertEquals(result, sync.checkedGet());
        assertEquals(result, sync.get(1L, TimeUnit.NANOSECONDS));
        assertEquals(result, sync.get());
        assertEquals(false, sync.isCancelled());
        assertEquals(true, sync.isDone());

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
     * @param sync    A {@link TxSyncFuture} instance to be checked.
     * @param nsec    The number of seconds to wait for completion.
     * @param cause   A throwable that indicates the cause of failure.
     * @param t       A thread to be joined.
     * @throws Exception  An error occurred.
     */
    private void verifyFailure(TxSyncFuture<Long> sync, long nsec,
                               Throwable cause, Thread t) throws Exception {
        try {
            sync.checkedGet(nsec, TimeUnit.SECONDS);
            unexpected();
        } catch (VTNException e) {
            assertEquals(VtnErrorTag.INTERNALERROR, e.getVtnErrorTag());
            assertEquals(cause, e.getCause());
        }

        try {
            sync.checkedGet();
            unexpected();
        } catch (VTNException e) {
            assertEquals(VtnErrorTag.INTERNALERROR, e.getVtnErrorTag());
            assertEquals(cause, e.getCause());
        }

        try {
            sync.get(1L, TimeUnit.NANOSECONDS);
            unexpected();
        } catch (ExecutionException e) {
            assertEquals(cause, e.getCause());
        }

        try {
            sync.get();
            unexpected();
        } catch (ExecutionException e) {
            assertEquals(cause, e.getCause());
        }

        assertEquals(false, sync.isCancelled());
        assertEquals(true, sync.isDone());

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
     * @param sync    A {@link TxSyncFuture} instance to be checked.
     * @param nsec    The number of seconds to wait for completion.
     * @param t       A thread to be joined.
     * @throws Exception  An error occurred.
     */
    private void verifyTargetCancel(TxSyncFuture<Long> sync, long nsec,
                                    Thread t) throws Exception {
        try {
            sync.checkedGet(nsec, TimeUnit.SECONDS);
            unexpected();
        } catch (VTNException e) {
            assertEquals(VtnErrorTag.INTERNALERROR, e.getVtnErrorTag());
            Throwable cause = e.getCause();
            assertEquals(CancellationException.class, cause.getClass());
        }

        try {
            sync.checkedGet();
            unexpected();
        } catch (VTNException e) {
            assertEquals(VtnErrorTag.INTERNALERROR, e.getVtnErrorTag());
            Throwable cause = e.getCause();
            assertEquals(CancellationException.class, cause.getClass());
        }

        try {
            sync.get(1L, TimeUnit.NANOSECONDS);
            unexpected();
        } catch (ExecutionException e) {
            Throwable cause = e.getCause();
            assertEquals(CancellationException.class, cause.getClass());
        }

        try {
            sync.get();
            unexpected();
        } catch (ExecutionException e) {
            Throwable cause = e.getCause();
            assertEquals(CancellationException.class, cause.getClass());
        }

        assertEquals(false, sync.isCancelled());
        assertEquals(true, sync.isDone());

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
