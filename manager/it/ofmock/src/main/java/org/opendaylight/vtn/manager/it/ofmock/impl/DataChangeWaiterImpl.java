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
import org.opendaylight.vtn.manager.it.ofmock.DataStoreUtils;

import org.opendaylight.controller.md.sal.binding.api.DataBroker;
import org.opendaylight.controller.md.sal.binding.api.DataChangeListener;
import org.opendaylight.controller.md.sal.binding.api.ReadOnlyTransaction;
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
    private int  revision = 0;

    /**
     * Base revision number of the target data.
     */
    private int  baseRevision;

    /**
     * The data object that contains the latest value.
     */
    private T  latestValue;

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

        // Read the current data.
        try (ReadOnlyTransaction rtx = ofmock.newReadOnlyTransaction()) {
            T data = DataStoreUtils.read(rtx, store, path).orNull();
            synchronized (this) {
                if (revision == 0) {
                    latestValue = data;
                }
            }
        }
    }

    /**
     * Return the target data object if it has been created or updated.
     *
     * @param ev  An {@link AsyncDataChangeEvent} instance.
     * @return  A data object if the specified data has been created or
     *          updated. {@code null} otherwise.
     */
    private T getUpdatedData(
        AsyncDataChangeEvent<InstanceIdentifier<?>, DataObject> ev) {
        Class<T> type = targetPath.getTargetType();

        Map<InstanceIdentifier<?>, DataObject> created = ev.getCreatedData();
        if (created != null) {
            DataObject data = created.get(targetPath);
            if (type.isInstance(data)) {
                return type.cast(data);
            }
        }

        Map<InstanceIdentifier<?>, DataObject> updated = ev.getUpdatedData();
        if (updated != null) {
            DataObject data = updated.get(targetPath);
            if (type.isInstance(data)) {
                return type.cast(data);
            }
        }

        return null;
    }

    /**
     * Determine whether the target data object has been removed or not.
     *
     * @param ev  An {@link AsyncDataChangeEvent} instance.
     * @return  {@code true} if the target data object has been removed.
     *          {@code false} otherwise.
     */
    private boolean isRemoved(
        AsyncDataChangeEvent<InstanceIdentifier<?>, DataObject> ev) {
        Set<InstanceIdentifier<?>> removed = ev.getRemovedPaths();
        return (removed != null && removed.contains(targetPath));
    }

    /**
     * Set the latest value notified by the data change listener.
     *
     * @param data  The data object notified by the data change listener.
     */
    private synchronized void setValue(T data) {
        latestValue = data;
        revision++;
        notifyAll();
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
    public synchronized T getValue() {
        return latestValue;
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
        T data = getUpdatedData(ev);
        if (data != null || isRemoved(ev)) {
            setValue(data);
        }
    }
}
