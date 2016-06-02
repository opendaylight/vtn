/*
 * Copyright (c) 2015, 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.ofmock.impl;

import java.util.Collection;

import javax.annotation.Nonnull;

import org.opendaylight.vtn.manager.it.ofmock.DataChangeWaiter;
import org.opendaylight.vtn.manager.it.ofmock.DataStoreUtils;

import org.opendaylight.controller.md.sal.binding.api.DataBroker;
import org.opendaylight.controller.md.sal.binding.api.DataObjectModification.ModificationType;
import org.opendaylight.controller.md.sal.binding.api.DataObjectModification;
import org.opendaylight.controller.md.sal.binding.api.DataTreeChangeListener;
import org.opendaylight.controller.md.sal.binding.api.DataTreeIdentifier;
import org.opendaylight.controller.md.sal.binding.api.DataTreeModification;
import org.opendaylight.controller.md.sal.binding.api.ReadOnlyTransaction;
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
    implements DataChangeWaiter<T>, DataTreeChangeListener<T> {
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
    private final ListenerRegistration<?>  listener;

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
        DataTreeIdentifier<T> ident = new DataTreeIdentifier<>(store, path);
        listener = broker.registerDataTreeChangeListener(ident, this);

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

    // DataTreeChangeListener

    /**
     * Invoked when the specified data tree has been modified.
     *
     * @param changes  A collection of data tree modifications.
     */
    @Override
    public void onDataTreeChanged(
        @Nonnull Collection<DataTreeModification<T>> changes) {
        for (DataTreeModification<T> change: changes) {
            DataObjectModification<T> mod = change.getRootNode();
            ModificationType modType = mod.getModificationType();
            T data = (modType == ModificationType.DELETE)
                ? null : mod.getDataAfter();
            setValue(data);
        }
    }
}
