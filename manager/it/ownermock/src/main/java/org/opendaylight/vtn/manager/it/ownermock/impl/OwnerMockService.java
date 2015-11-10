/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.ownermock.impl;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.TimeUnit;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import com.google.common.base.Optional;

import org.opendaylight.controller.md.sal.common.api.clustering.CandidateAlreadyRegisteredException;
import org.opendaylight.controller.md.sal.common.api.clustering.Entity;
import org.opendaylight.controller.md.sal.common.api.clustering.EntityOwnershipCandidateRegistration;
import org.opendaylight.controller.md.sal.common.api.clustering.EntityOwnershipChange;
import org.opendaylight.controller.md.sal.common.api.clustering.EntityOwnershipListener;
import org.opendaylight.controller.md.sal.common.api.clustering.EntityOwnershipListenerRegistration;
import org.opendaylight.controller.md.sal.common.api.clustering.EntityOwnershipService;
import org.opendaylight.controller.md.sal.common.api.clustering.EntityOwnershipState;

/**
 * The mock-up of the entity ownership service.
 */
public final class OwnerMockService
    implements EntityOwnershipService, AutoCloseable {
    /**
     * Logger instance.
     */
    private static final Logger  LOG =
        LoggerFactory.getLogger(OwnerMockService.class);

    /**
     * The number of seconds to wait for completion of executor shutdown.
     */
    private static final long  SHUTDOWN_TIMEOUT = 10L;

    /**
     * The executor to deliver ownership change.
     */
    private final ExecutorService  eventExecutor =
        Executors.newSingleThreadExecutor();

    /**
     * Registered entity candidates.
     */
    private final Map<String, Set<Entity>>  candidateMap = new HashMap<>();

    /**
     * Registered entity ownership listeners.
     */
    private final Map<String, List<ListenerRegistration>> listenerMap =
        new HashMap<>();

    /**
     * {@code Invocation} describes an invocation of entity ownership event.
     */
    private static final class Invocation implements Runnable {
        /**
         * An entity ownership change to be delivered.
         */
        private final EntityOwnershipChange  ownerChange;

        /**
         * An entity ownership listener.
         */
        private final EntityOwnershipListener  listener;

        /**
         * Construct a new instance.
         *
         * @param change  An entity ownership change.
         * @param l       An entity ownership listener.
         */
        private Invocation(EntityOwnershipChange change,
                           EntityOwnershipListener l) {
            ownerChange = change;
            listener = l;
        }

        // Runnable

        /**
         * Deliver an entity ownership change.
         */
        @Override
        public void run() {
            try {
                listener.ownershipChanged(ownerChange);
            } catch (RuntimeException e) {
                LOG.error("Ownership listener throws an exception: change=" +
                          ownerChange + ", listener=" + listener, e);
            }
        }
    }

    /**
     * Remove the given entity candidate.
     *
     * @param ent  The entity to be removed.
     */
    synchronized void removeCandidate(Entity ent) {
        String type = ent.getType();
        Set<Entity> entities = candidateMap.get(type);
        if (entities != null && entities.remove(ent) && entities.isEmpty()) {
            candidateMap.remove(type);
        }
    }

    /**
     * Remove the given entity ownership listener.
     *
     * @param reg  The listener registration to be removed.
     */
    synchronized void removeListener(ListenerRegistration reg) {
        String type = reg.getEntityType();
        List<ListenerRegistration> listeners = listenerMap.get(type);
        if (listeners != null && listeners.remove(reg) &&
            listeners.isEmpty()) {
            listenerMap.remove(type);
        }
    }

    /**
     * Return a list of listeners that are interested in ownership of the
     * specified entity type.
     *
     * @param type  The type of entity.
     * @return  A list of entity ownership listeners or {@code null}.
     */
    private synchronized List<EntityOwnershipListener> getListeners(
        String type) {
        List<ListenerRegistration> list = listenerMap.get(type);
        List<EntityOwnershipListener> listeners = null;
        if (list != null && !list.isEmpty()) {
            listeners = new ArrayList<>(list.size());
            for (ListenerRegistration reg: list) {
                listeners.add(reg.getInstance());
            }
        }

        return listeners;
    }

    /**
     * Post an entity ownership change event.
     *
     * @param ent        The entity to be delivered.
     * @param listeners  A list of entity ownership listeners.
     */
    private void post(Entity ent, List<EntityOwnershipListener> listeners) {
        // This process always becomes the owner.
        EntityOwnershipChange change =
            new EntityOwnershipChange(ent, false, true, true);
        for (EntityOwnershipListener l: listeners) {
            eventExecutor.execute(new Invocation(change, l));
        }
    }

    // EntityOwnershipService

    /**
     * Registers a candidate for ownership of the given entity.
     *
     * @param ent  The entity to be registered.
     * @return A registration of the given entity.
     * @throws CandidateAlreadyRegisteredException
     *    The given entity is already registered.
     */
    @Override
    public synchronized EntityOwnershipCandidateRegistration registerCandidate(
        Entity ent) throws CandidateAlreadyRegisteredException {
        String type = ent.getType();
        Set<Entity> entities = candidateMap.get(type);
        if (entities == null) {
            entities = new HashSet<>();
            candidateMap.put(type, entities);
        }
        if (!entities.add(ent)) {
            throw new CandidateAlreadyRegisteredException(ent);
        }

        CandidateRegistration reg = new CandidateRegistration(this, ent);
        List<EntityOwnershipListener> listeners = getListeners(type);
        if (listeners != null) {
            post(ent, listeners);
        }

        return reg;
    }

    /**
     * Registers a listener that is interested in ownership changes for
     * entities of the given entity type.
     *
     * @param type      The type of entities.
     * @param listener  The listener that is interested in the entities.
     * @return  A registration of the given listener.
     */
    @Override
    public synchronized EntityOwnershipListenerRegistration registerListener(
        String type, EntityOwnershipListener listener) {
        ListenerRegistration reg =
            new ListenerRegistration(this, type, listener);
        List<ListenerRegistration> list = listenerMap.get(type);
        if (list == null) {
            list = new ArrayList<>();
            listenerMap.put(type, list);
        }

        list.add(reg);

        Set<Entity> entities = candidateMap.get(type);
        if (entities != null) {
            List<EntityOwnershipListener> listeners =
                Collections.singletonList(listener);
            for (Entity ent: entities) {
                post(ent, listeners);
            }
        }

        return reg;
    }

    /**
     * Return the current ownership state information for the given entity.
     *
     * @param ent  The entity to query.
     * @return  An Optional for entity ownership state.
     */
    @Override
    public synchronized Optional<EntityOwnershipState> getOwnershipState(
        Entity ent) {
        String type = ent.getType();
        EntityOwnershipState state = null;
        Set<Entity> entities = candidateMap.get(type);
        if (entities != null && entities.contains(ent)) {
            // This process is the owner of every entity.
            state = new EntityOwnershipState(true, true);
        }

        return Optional.fromNullable(state);
    }

    // AutoCloseable

    /**
     * Terminate the entity ownership mock-up service.
     *
     * @throws InterruptedException  The calling thread was interrupted.
     */
    @Override
    public void close() throws InterruptedException {
        // Terminate the event executor.
        eventExecutor.shutdown();

        TimeUnit sec = TimeUnit.SECONDS;
        if (!eventExecutor.awaitTermination(SHUTDOWN_TIMEOUT, sec)) {
            LOG.error("Event executor did not stop.");
            eventExecutor.shutdownNow();
        }
    }
}
