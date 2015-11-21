/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.flow.add;

import java.util.Collection;
import java.util.Collections;
import java.util.Deque;
import java.util.LinkedList;

import org.opendaylight.vtn.manager.internal.TxContext;
import org.opendaylight.vtn.manager.internal.TxHook;
import org.opendaylight.vtn.manager.internal.TxTask;
import org.opendaylight.vtn.manager.internal.VTNManagerProvider;
import org.opendaylight.vtn.manager.internal.util.flow.VTNFlowBuilder;

/**
 * {@code FlowAddQueue} describes a queue for flow entries to be installed.
 *
 * <p>
 *   An instance of this class must be obtained by
 *   {@link TxContext#getSpecific(Class)}. All the queued flow entries are
 *   installed asynchronously just after the successful completion of the
 *   MD-SAL datastore transaction associated with this instance.
 *   Note that requests will be discarded when the transaction is canceled.
 * </p>
 */
public final class FlowAddQueue implements TxHook {
    /**
     * The order of the post-submit hook.
     *
     * <p>
     *   Flow installation should be the last hook to execute.
     * </p>
     */
    private static final int  HOOK_ORDER = Integer.MAX_VALUE;

    /**
     * A list of flow entries to be installed.
     */
    private final Deque<VTNFlowBuilder>  requests = new LinkedList<>();

    /**
     * Construct a new instance.
     *
     * @param ctx  A runtime context for transaction task.
     */
    public FlowAddQueue(TxContext ctx) {
        // Register this instance as a post-submit hook.
        ctx.addPostSubmitHook(this);
    }

    /**
     * Enqueue the given data flow to the request queue.
     *
     * @param builder  A {@link VTNFlowBuilder} instance that contains
     *                 data flow to be installed.
     */
    public void enqueue(VTNFlowBuilder builder) {
        requests.addLast(builder);
    }

    /**
     * Return an unmodifiable collection that contains flow entries in the
     * queue.
     *
     * @return  An unmodifiable collection of {@link VTNFlowBuilder} instances.
     */
    public Collection<VTNFlowBuilder> getRequests() {
        return Collections.unmodifiableCollection(requests);
    }

    // TxHook

    /**
     * Invoked when the MD-SAL datastore transaction has been submitted
     * successfully.
     *
     * <p>
     *   This method installs all the flow entries in the request queue.
     * </p>
     *
     * @param ctx   A runtime context for transaction task.
     * @param task  The current transaction task.
     */
    @Override
    public void run(TxContext ctx, TxTask<?> task) {
        VTNManagerProvider provider = ctx.getProvider();
        for (VTNFlowBuilder builder = requests.pollFirst(); builder != null;
             builder = requests.pollFirst()) {
            provider.addFlow(builder);
        }
    }

    /**
     * {@inheritDoc}
     */
    public int getOrder() {
        return HOOK_ORDER;
    }
}
