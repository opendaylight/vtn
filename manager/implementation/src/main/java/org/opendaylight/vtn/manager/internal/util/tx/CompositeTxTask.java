/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.tx;

import java.util.ArrayList;
import java.util.Collections;
import java.util.Iterator;
import java.util.List;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.TxContext;
import org.opendaylight.vtn.manager.internal.VTNManagerProvider;

/**
 * An implementation of {@link AbstractTxTask} that executes multiple tasks
 * on a single transaction.
 *
 * <p>
 *   This task returns a list of results of sub tasks.
 *   The order of results in the returned list is the same as the sub tasks
 *   configured in this task.
 *   Note that all sub tasks are required to return the same type of
 *   objects.
 * </p>
 *
 * @param <V>  The type of the object returned by the sub tasks.
 * @param <T>  The type of the sub task.
 */
public class CompositeTxTask<V, T extends AbstractTxTask<V>>
    extends AbstractTxTask<List<V>> {
    /**
     * A list of transaction tasks to be executed.
     */
    private final List<T>  subTasks;

    /**
     * Construct a new task.
     *
     * @param tasks  A list of tasks to be executed on a single transaction.
     * @throws IllegalArgumentException
     *    The given task list is empty.
     */
    public CompositeTxTask(List<T> tasks) {
        if (tasks == null || tasks.isEmpty()) {
            throw new IllegalArgumentException("Sub task cannot be empty.");
        }

        subTasks = Collections.unmodifiableList(tasks);
    }

    /**
     * Return an immutable list of sub tasks.
     *
     * @return  An immutable list of sub tasks.
     */
    public final List<T> getSubTasks() {
        return subTasks;
    }

    /**
     * Invoked when the MD-SAL datastore transaction has started.
     *
     * @param ctx  A runtime context for transaction task.
     * @throws VTNException  An error occurred.
     */
    protected void onStarted(TxContext ctx) throws VTNException {
        // Nothing to do. Subclass may override this method.
    }

    /**
     * Invoked when all the sub tasks have completed.
     *
     * @param ctx      A runtime context for transaction task.
     * @param results  A list of sub task results.
     * @throws VTNException  An error occurred.
     */
    protected void onCompleted(TxContext ctx, List<V> results)
        throws VTNException {
        // Nothing to do. Subclass may override this method.
    }

    // AbstractTxTask

    /**
     * Execute all sub tasks on the given MD-SAL transaction context.
     *
     * <p>
     *   Note that this method may be called more than once when a MD-SAL
     *   datastore transaction is aborted due to data conflict.
     * </p>
     *
     * @param ctx  A runtime context for transaction task.
     * @return  Result of this task.
     * @throws VTNException  An error occurred.
     */
    @Override
    protected final List<V> execute(TxContext ctx) throws VTNException {
        onStarted(ctx);

        List<V> result = new ArrayList<>();
        for (T task: subTasks) {
            result.add(task.execute(ctx));
        }

        onCompleted(ctx, result);

        return result;
    }

    // TxTask

    /**
     * {@inheritDoc}
     */
    @Override
    public boolean needErrorLog(Throwable t) {
        for (T task: subTasks) {
            if (task.needErrorLog(t)) {
                return true;
            }
        }

        return false;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void onSuccess(VTNManagerProvider provider, List<V> result) {
        Iterator<V> it = result.iterator();
        for (T task: subTasks) {
            task.onSuccess(provider, it.next());
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void onFailure(VTNManagerProvider provider, Throwable t) {
        for (T task: subTasks) {
            task.onFailure(provider, t);
        }
    }
}
