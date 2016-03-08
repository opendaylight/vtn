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

import org.opendaylight.vtn.manager.it.ofmock.DataStoreUtils;

import org.opendaylight.controller.md.sal.binding.api.DataBroker;
import org.opendaylight.controller.md.sal.binding.api.ReadWriteTransaction;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.yangtools.yang.binding.DataObject;
import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

/**
 * {@code DeleteDataTask} describes a task that deletes a data model from the
 * MD-SAL operational datastore.
 *
 * @param <T>  The type of the target data model.
 */
public final class DeleteDataTask<T extends DataObject> extends TxTask<Void>
    implements FutureCallback<Void> {
    /**
     * Logger instance.
     */
    private static final Logger  LOG =
        LoggerFactory.getLogger(DeleteDataTask.class);

    /**
     * The path to the target data.
     */
    private final InstanceIdentifier<T>  targetPath;

    /**
     * Construct a new instance.
     *
     * @param broker  The data broker service.
     * @param path    The path to the target data.
     */
    public DeleteDataTask(DataBroker broker, InstanceIdentifier<T> path) {
        super(broker);
        targetPath = path;
        Futures.addCallback(getFuture(), this);
    }

    // TxTask

    /**
     * {@inheritDoc}
     */
    @Override
    protected void execute(ReadWriteTransaction tx) {
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        DataStoreUtils.delete(tx, oper, targetPath);
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
     * Invoked when the specified data has been deleted successfully.
     *
     * @param result  {@code null}.
     */
    @Override
    public void onSuccess(Void result) {
        LOG.trace("Data has been deleted: path={}", targetPath);
    }

    /**
     * Invoked when the specified data did not deleted.
     *
     * @param cause  A throwable that indicates the cause of error.
     */
    @Override
    public void onFailure(Throwable cause) {
        String msg = "Failed to delete data: path=" + targetPath;
        LOG.error(msg, cause);
    }
}
