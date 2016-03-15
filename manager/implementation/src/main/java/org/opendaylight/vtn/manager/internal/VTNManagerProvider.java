/*
 * Copyright (c) 2015, 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal;

import java.util.List;
import java.util.Timer;
import java.util.concurrent.Executor;
import java.util.concurrent.Future;

import com.google.common.base.Optional;
import com.google.common.util.concurrent.FutureCallback;

import org.apache.commons.lang3.tuple.Pair;

import org.opendaylight.vtn.manager.VTNException;
import org.opendaylight.vtn.manager.packet.Packet;

import org.opendaylight.vtn.manager.internal.util.VTNEntityType;
import org.opendaylight.vtn.manager.internal.util.concurrent.VTNFuture;
import org.opendaylight.vtn.manager.internal.util.flow.FlowRpcWatcher;
import org.opendaylight.vtn.manager.internal.util.flow.VTNFlowBuilder;
import org.opendaylight.vtn.manager.internal.util.inventory.SalPort;

import org.opendaylight.controller.md.sal.binding.api.DataBroker;
import org.opendaylight.controller.md.sal.common.api.clustering.CandidateAlreadyRegisteredException;
import org.opendaylight.controller.md.sal.common.api.clustering.Entity;
import org.opendaylight.controller.md.sal.common.api.clustering.EntityOwnershipCandidateRegistration;
import org.opendaylight.controller.md.sal.common.api.clustering.EntityOwnershipListener;
import org.opendaylight.controller.md.sal.common.api.clustering.EntityOwnershipListenerRegistration;
import org.opendaylight.controller.md.sal.common.api.clustering.EntityOwnershipState;

import org.opendaylight.yangtools.yang.binding.Notification;
import org.opendaylight.yangtools.yang.binding.RpcService;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.VtnFlowId;

/**
 * This interface defines an internal OSGi service which provides MD-SAL
 * VTN Manager services used by AD-SAL interface.
 */
public interface VTNManagerProvider
    extends AutoCloseable, Executor, TxQueue, FlowRpcWatcher {
    /**
     * Return a {@link VTNConfig} instance that contains current configuration.
     *
     * @return  A {@link VTNConfig} instance.
     */
    VTNConfig getVTNConfig();

    /**
     * Create a new read-only MD-SAL datastore transaction context.
     *
     * @return  A read-only MD-SAL datastore transaction context.
     */
    TxContext newTxContext();

    /**
     * Return the global timer.
     *
     * @return  The global timer.
     */
    Timer getTimer();

    /**
     * Execute the specified task asynchronously.
     *
     * @param task  A task to be executed on this thread pool.
     * @return  {@code true} is returned if the specified task was submitted.
     *          {@code false} is returned if the specified tas was rejected.
     * @throws NullPointerException
     *    {@code task} is {@code null}.
     */
    boolean executeTask(Runnable task);

    /**
     * Set the given future callback to the given future.
     *
     * <p>
     *   If the given future implements {@code ListenableFuture}, this method
     *   adds the given callback to the given future, and the callback will
     *   be invoked by a thread which is executing the future.
     *   If not, the given callback will be invoked on another thread
     *   asynchronously.
     * </p>
     *
     * @param future  A {@link Future} instance to wait for its completion.
     * @param cb      A {@link FutureCallback} instance.
     * @param <T>     The type of the object returned by the given future.
     */
    <T> void setCallback(Future<T> future, FutureCallback<? super T> cb);

    /**
     * Return a data broker service instance.
     *
     * @return  A {@link DataBroker} instance.
     */
    DataBroker getDataBroker();

    /**
     * Transmit the given packet.
     *
     * @param egress  A {@link SalPort} instance which specifies the egress
     *                switch port.
     * @param packet  A {@link Packet} instance to transmit.
     */
    void transmit(SalPort egress, Packet packet);

    /**
     * Transmit all the given packets.
     *
     * @param packets  A list of packets to transmit.
     *                 The left of each elements indicates the egress switch
     *                 port, and the right indiciates a packet to transmit.
     */
    void transmit(List<Pair<SalPort, Packet>> packets);

    /**
     * Return the packet route resolver associated with the system default
     * routing policy.
     *
     * @return  A {@link RouteResolver} instance on success.
     *          {@code null} if the routing management is already closed.
     */
    RouteResolver getRouteResolver();

    /**
     * Return a packet route resolver.
     *
     * @param id  The identifier of the path policy.
     * @return  A {@link RouteResolver} instance if found.
     *          {@code null} if not fonud.
     */
    RouteResolver getRouteResolver(Integer id);

    /**
     * Add the given VTN data flow.
     *
     * @param builder  A {@link VTNFlowBuilder} instance which contains
     *                 data flow to be installed.
     * @return  A future associated with the task which adds a VTN data flow.
     */
    VTNFuture<VtnFlowId> addFlow(VTNFlowBuilder builder);

    /**
     * Remove flow entries that match the given condition.
     *
     * @param remover  A {@link FlowRemover} instance which determines VTN data
     *                 flows to be removed.
     * @return  A future associated with the task which removes VTN data flows.
     */
    VTNFuture<Void> removeFlows(FlowRemover remover);

    /**
     * Register a candidate for ownership of the given entity.
     *
     * @param ent  The entity to be registered as a candidate of ownership.
     * @return  An {@link EntityOwnershipCandidateRegistration} instance.
     * @throws CandidateAlreadyRegisteredException
     *    The given entity is already registered.
     */
    EntityOwnershipCandidateRegistration registerEntity(Entity ent)
        throws CandidateAlreadyRegisteredException;

    /**
     * Register a listener that listens the ownership status of the entity
     * specified by the given type.
     *
     * @param etype     The type of the VTN global entity.
     * @param listener  An {@link EntityOwnershipListener} instance.
     * @return  An {@link EntityOwnershipListenerRegistration} instance.
     */
    EntityOwnershipListenerRegistration registerListener(
        VTNEntityType etype, EntityOwnershipListener listener);

    /**
     * Return the current ownership state information about the given entity.
     *
     * @param ent  The entity to query.
     * @return An {@link Optional} instance that contains the entity ownership
     *         state if present.
     */
    Optional<EntityOwnershipState> getOwnershipState(Entity ent);

    /**
     * Determine whether this process process is the owner of the given
     * global entity.
     *
     * @param etype  The type of VTN global entity.
     *               Specifying non-global entity type results in undefined
     *               behavior.
     * @return  {@code true} if this process is the owner of the given global
     *          entity. {@code false} otherwise.
     */
    boolean isOwner(VTNEntityType etype);

    /**
     * Return an implementation of the specified RPC service.
     *
     * @param type  A class which specifies the RPC service.
     * @param <T>   The type of the RPC service.
     * @return  The proxy for the given RPC service.
     * @throws VTNException
     *    The specified RPC service was not found.
     */
    <T extends RpcService> T getRpcService(Class<T> type) throws VTNException;

    /**
     * Return an implementation of the RPC service provided by the VTN Manager.
     *
     * @param type  A class which specifies the RPC service.
     * @param <T>   The type of the RPC service.
     * @return  The proxy for the given RPC service.
     * @throws VTNException
     *    The specified RPC service was not found.
     */
    <T extends RpcService> T getVtnRpcService(Class<T> type)
        throws VTNException;

    /**
     * Publish the given MD-SAL notification.
     *
     * @param n  A {@link Notification} instance to be published.
     */
    void publish(Notification n);

    /**
     * Post a new MD-SAL transaction task.
     *
     * <p>
     *   This method associates a new {@link VTNFuture} instance with the
     *   given task, including all background tasks started by the task.
     *   Note that the given task is never canceled even if the returned
     *   {@link VTNFuture} is cancelled. {@link VTNFuture#cancel(boolean)} on
     *   the returned value only cancels the wait for background tasks.
     * </p>
     *
     * @param task  A {@link TxTask} instance.
     * @param <T>   The type of the object to be returned by the task.
     * @return  A {@link VTNFuture} associated with the given task and its
     *          background tasks.
     */
    <T> VTNFuture<T> postSync(TxTask<T> task);

    // AutoCloseable

    /**
     * Close the VTN Manager provider service.
     */
    @Override
    void close();
}
