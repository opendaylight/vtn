/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.ofmock.impl;

import java.util.Map;
import java.util.Set;

import org.opendaylight.vtn.manager.it.ofmock.DataChangeWaiter;

import org.opendaylight.controller.md.sal.binding.api.DataBroker;
import org.opendaylight.controller.md.sal.binding.api.DataChangeListener;
import org.opendaylight.controller.md.sal.common.api.data.AsyncDataBroker.DataChangeScope;
import org.opendaylight.controller.md.sal.common.api.data.AsyncDataChangeEvent;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.yangtools.concepts.ListenerRegistration;
import org.opendaylight.yangtools.yang.binding.DataObject;
import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

/**
 * Implementation of {@link DataChangeWaiter}.
 *
 * @param <T>  Type of the target object.
 */
public final class DataChangeWaiterImpl<T extends DataObject>
    implements DataChangeWaiter<T>, DataChangeListener {
    /**
     * The ofmock provider service.
     */
    private final OfMockProvider  ofMock;

    /**
     * Path to the target data object.
     */
    private final InstanceIdentifier<T>  targetPath;

    /**
     * Registration of the data change listener.
     */
    private final ListenerRegistration<DataChangeListener>  listener;

    /**
     * Revision number of the target data.
     */
    private int  revision;

    /**
     * Base revision number of the target data.
     */
    private int  baseRevision;

    /**
     * Construct a new instance.
     *
     * @param ofmock  ofmock provider service.
     * @param store   The type of the logica datastore.
     * @param path    Path to the target data object.
     */
    DataChangeWaiterImpl(OfMockProvider ofmock, LogicalDatastoreType store,
                         InstanceIdentifier<T> path) {
        ofMock = ofmock;
        targetPath = path;

        DataBroker broker = ofmock.getDataBroker();
        listener = broker.registerDataChangeListener(
            store, path, this, DataChangeScope.SUBTREE);
    }

    /**
     * Determine whether the given event contains the target data object
     * or not.
     *
     * @param ev  An {@link AsyncDataChangeEvent} instance.
     * @return  {@code true} only if the given event contains the target data
     *          object.
     */
    private boolean contains(
        AsyncDataChangeEvent<InstanceIdentifier<?>, DataObject> ev) {
        Map<InstanceIdentifier<?>, DataObject> created = ev.getCreatedData();
        if (created != null && created.containsKey(targetPath)) {
            return true;
        }

        Map<InstanceIdentifier<?>, DataObject> updated = ev.getUpdatedData();
        if (updated != null && updated.containsKey(targetPath)) {
            return true;
        }

        Set<InstanceIdentifier<?>> removed = ev.getRemovedPaths();
        return (removed != null && removed.contains(targetPath));
    }

    // DataChangeWaiter

    /**
     * {@inheritDoc}
     */
    @Override
    public InstanceIdentifier<T> getPath() {
        return targetPath;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public synchronized boolean await(long limit) throws InterruptedException {
        int base = baseRevision;
        while (base == revision) {
            long timeout = limit - System.currentTimeMillis();
            if (timeout <= 0) {
                return false;
            }
            wait(timeout);
        }

        baseRevision = revision;
        return true;
    }

    // AutoCloseable

    /**
     * Close this instance.
     */
    @Override
    public void close() {
        listener.close();
        ofMock.remove(this);
    }

    // DataChangeListener

    /**
     * Invoked when at least an entry in the datastore has been changed.
     *
     * @param ev  An {@link AsyncDataChangeEvent} instance.
     */
    @Override
    public void onDataChanged(
        AsyncDataChangeEvent<InstanceIdentifier<?>, DataObject> ev) {
        if (contains(ev)) {
            synchronized (this) {
                revision++;
                notifyAll();
            }
        }
    }
}
