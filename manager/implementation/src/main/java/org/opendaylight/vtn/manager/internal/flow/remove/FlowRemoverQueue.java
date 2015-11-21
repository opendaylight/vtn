/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.flow.remove;

import java.util.Collection;
import java.util.Collections;
import java.util.Deque;
import java.util.LinkedList;

import org.opendaylight.vtn.manager.internal.FlowRemover;
import org.opendaylight.vtn.manager.internal.TxContext;
import org.opendaylight.vtn.manager.internal.TxHook;
import org.opendaylight.vtn.manager.internal.TxTask;
import org.opendaylight.vtn.manager.internal.VTNManagerProvider;
import org.opendaylight.vtn.manager.internal.util.concurrent.VTNFuture;

/**
 * {@code FlowRemoverQueue} describes a queue for {@link FlowRemover} instances
 * that remove the specified flow entries.
 *
 * <p>
 *   An instance of this class must be obtained by
 *   {@link TxContext#getSpecific(Class)}. All the queued {@link FlowRemover}
 *   instances are applied just after the successful completion of the
 *   MD-SAL datastore transaction associated with this instance.
 *   Note that all the queued {@link FlowRemover} instances  will be discarded
 *   when the transaction is canceled.
 * </p>
 */
public final class FlowRemoverQueue implements TxHook {
    /**
     * The order of the post-submit hook.
     *
     * <p>
     *   Flow remover should be processed prior to flow installation.
     * </p>
     */
    private static final int  HOOK_ORDER = 10;

    /**
     * A list of {@link FlowRemover} instances.
     */
    private final Deque<FlowRemover>  requests = new LinkedList<>();

    /**
     * Construct a new instance.
     *
     * @param ctx  A runtime context for transaction task.
     */
    public FlowRemoverQueue(TxContext ctx) {
        // Register this instance as a post-submit hook.
        ctx.addPostSubmitHook(this);
    }

    /**
     * Enqueue the given {@link FlowRemover} instance.
     *
     * @param remover  A {@link FlowRemover} instance that specifies flow
     *                 entries to be removed.
     */
    public void enqueue(FlowRemover remover) {
        requests.addLast(remover);
    }

    /**
     * Return an unmodifiable collection that contains flow removers in the
     * queue.
     *
     * @return  An unmodifiable collection of {@link FlowRemover} instances.
     */
    public Collection<FlowRemover> getRequests() {
        return Collections.unmodifiableCollection(requests);
    }

    // TxHook

    /**
     * Invoked when the MD-SAL datastore transaction has been submitted
     * successfully.
     *
     * <p>
     *   This method removes all the flow entries specified by the queued
     *   {@link FlowRemover} instances.
     * </p>
     *
     * @param ctx   A runtime context for transaction task.
     * @param task  The current transaction task.
     */
    @Override
    public void run(TxContext ctx, TxTask<?> task) {
        VTNManagerProvider provider = ctx.getProvider();
        boolean async = task.isAsync();
        for (FlowRemover fr = requests.pollFirst(); fr != null;
             fr = requests.pollFirst()) {
            VTNFuture<?> future = provider.removeFlows(fr);
            if (!async) {
                // The given transaction task needs to wait for completion of
                // flow removal.
                task.addBackgroundTask(future);
            }
        }
    }

    /**
     * {@inheritDoc}
     */
    public int getOrder() {
        return HOOK_ORDER;
    }
}
