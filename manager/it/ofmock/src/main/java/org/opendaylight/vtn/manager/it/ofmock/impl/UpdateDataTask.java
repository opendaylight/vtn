/*
 * Copyright (c) 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.ofmock.impl;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import com.google.common.util.concurrent.FutureCallback;
import com.google.common.util.concurrent.Futures;

import org.opendaylight.controller.md.sal.binding.api.DataBroker;
import org.opendaylight.controller.md.sal.binding.api.ReadWriteTransaction;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.yangtools.yang.binding.DataObject;
import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

/**
 * {@code UpdateDataTask} describes a task that updates a data model in the
 * MD-SAL operational datastore.
 *
 * @param <T>  The type of the target data model.
 */
public final class UpdateDataTask<T extends DataObject> extends TxTask
    implements FutureCallback<Void> {
    /**
     * Logger instance.
     */
    private static final Logger  LOG =
        LoggerFactory.getLogger(UpdateDataTask.class);

    /**
     * The path to the target data.
     */
    private final InstanceIdentifier<T>  targetPath;

    /**
     * The data to be set into the MD-SAL operational datastore.
     */
    private final T  targetData;

    /**
     * Construct a new instance.
     *
     * @param broker  The data broker service.
     * @param path    The path to the target data.
     * @param data    The data to be set into the MD-SAL operational datastore.
     */
    public UpdateDataTask(DataBroker broker, InstanceIdentifier<T> path,
                          T data) {
        super(broker);
        targetPath = path;
        targetData = data;
        Futures.addCallback(getFuture(), this);
    }

    // TxTask

    /**
     * {@inheritDoc}
     */
    @Override
    protected void execute(ReadWriteTransaction tx) {
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        tx.merge(oper, targetPath, targetData, true);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected Logger getLogger() {
        return LOG;
    }

    // FutureCallback

    /**
     * Invoked when the specified data has been updated successfully.
     *
     * @param result  {@code null}.
     */
    @Override
    public void onSuccess(Void result) {
        LOG.trace("Data has been updated: path={}, data={}",
                  targetPath, targetData);
    }

    /**
     * Invoked when the specified data did not updated.
     *
     * @param cause  A throwable that indicates the cause of error.
     */
    @Override
    public void onFailure(Throwable cause) {
        String msg = "Failed to update data: path=" + targetPath +
            ", data=" + targetData;
        LOG.error(msg, cause);
    }
}
