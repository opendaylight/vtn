/*
 * Copyright (c) 2014, 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal;

import java.util.ArrayList;
import java.util.List;
import java.util.Set;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.CopyOnWriteArraySet;

import org.opendaylight.vtn.manager.internal.cluster.ClusterEvent;
import org.opendaylight.vtn.manager.internal.cluster.ClusterEventId;

/**
 * An instance of this class is used as a cluster cache which keeps
 * event instances to be delivered to remote cluster nodes.
 *
 * <p>
 *   The purpose of this class is to test events before they are removed
 *   by timer task.
 * </p>
 */
public class ClusterEventMap
    extends ConcurrentHashMap<ClusterEventId, ClusterEvent> {
    /**
     * A list of {@link ClusterEvent} instances once added to this map.
     */
    private final List<ClusterEvent>  postedEvents =
        new ArrayList<ClusterEvent>();

    /**
     * Cluster event listeners.
     */
    private final Set<ClusterEventListener>  listeners =
        new CopyOnWriteArraySet<ClusterEventListener>();

    /**
     * Construct a new object.
     */
    public ClusterEventMap() {
        super();
    }

    /**
     * Add the specified cluster event listener.
     *
     * @param l  Cluster event listener to be added.
     */
    public void addListener(ClusterEventListener l) {
        listeners.add(l);
    }

    /**
     * Remove the specified cluster event listener.
     *
     * @param l  Cluster event listener to be removed.
     */
    public void removeListener(ClusterEventListener l) {
        listeners.remove(l);
    }

    /**
     * Associated a value to the specified key.
     *
     * @param key    A key with which the given value is associated.
     * @param value  A value to be associated with the given key.
     * @return       The previous value associated with the given key, or
     *               {@code null} if there was no mapping for the key.
     */
    @Override
    public ClusterEvent put(ClusterEventId key, ClusterEvent value) {
        ClusterEvent old = super.put(key, value);
        if (old == null) {
            addEvent(key, value);
        }

        return old;
    }

    /**
     * Associated a value to the specified key only if this map does not
     * contain the key,
     *
     * @param key    A key with which the given value is associated.
     * @param value  A value to be associated with the given key.
     * @return       The previous value associated with the given key, or
     *               {@code null} if there was no mapping for the key.
     */
    @Override
    public ClusterEvent putIfAbsent(ClusterEventId key, ClusterEvent value) {
        ClusterEvent old = super.putIfAbsent(key, value);
        if (old == null) {
            addEvent(key, value);
        }

        return old;
    }

    /**
     * Remove the specified key from this map.
     *
     * @param key  A key to be removed from this map.
     * @return  The previous value associated with the given key, or
     *          {@code null} if there was no mapping for the key.
     */
    @Override
    public ClusterEvent remove(Object key) {
        ClusterEvent old = super.remove(key);
        if (old != null) {
            synchronized (this) {
                if (isEmpty()) {
                    notifyAll();
                }
            }
        }

        return old;
    }

    /**
     * Remove all mappings in this map.
     */
    @Override
    public synchronized void clear() {
        super.clear();
        notifyAll();
    }

    /**
     * Return a list of events posted after the previous call of this method.
     *
     * @return  A list of {@link ClusterEvent} instances posted to this map
     *          after the previous call of this method.
     */
    public synchronized List<ClusterEvent> getPostedEvents() {
        ArrayList<ClusterEvent> list =
            new ArrayList<ClusterEvent>(postedEvents);
        postedEvents.clear();

        return list;
    }

    /**
     * Block the calling thread until all mappings in this map are removed.
     *
     * @param timeout  The maximum number of milliseconds to wait.
     * @return  {@code true} only if all mappings were removed.
     * @throws InterruptedException  The calling thread was interrupted.
     */
    public boolean waitForCleared(long timeout) throws InterruptedException {
        if (isEmpty()) {
            return true;
        }

        long limit = System.currentTimeMillis() + timeout;
        long t = timeout;
        synchronized (this) {
            while (true) {
                wait(t);
                if (isEmpty()) {
                    return true;
                }

                t = limit - System.currentTimeMillis();
                if (t <= 0) {
                    break;
                }
            }
        }

        return isEmpty();
    }

    /**
     * Add a new event to {@link #postedEvents}.
     *
     * @param id     Identifier of the event.
     * @param event  A {@link ClusterEvent} instance.
     * @throws IllegalStateException  The specified event is already posted.
     */
    private void addEvent(ClusterEventId id, ClusterEvent event) {
        synchronized (this) {
            if (!postedEvents.add(event)) {
                // This should never happen.
                throw new IllegalStateException("Already posted: " + event);
            }
        }

        for (ClusterEventListener l: listeners) {
            l.posted(id, event);
        }
    }
}
