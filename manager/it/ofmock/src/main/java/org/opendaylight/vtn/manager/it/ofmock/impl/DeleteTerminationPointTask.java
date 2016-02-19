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

/**
 * {@code DeleteTerminationPointTask} describes a task that deletes a
 * termination point from the MD-SAL operational datastore for the
 * network topology.
 */
public final class DeleteTerminationPointTask extends TxTask<Void>
    implements FutureCallback<Void> {
    /**
     * Logger instance.
     */
    private static final Logger  LOG =
        LoggerFactory.getLogger(DeleteTerminationPointTask.class);

    /**
     * The port that specifies the termination point.
     */
    private final OfPort  targetPort;

    /**
     * Construct a new instance.
     *
     * @param broker  The data broker service.
     * @param port    An {@link OfPort} instance that specifies the termination
     *                point.
     */
    public DeleteTerminationPointTask(DataBroker broker, OfPort port) {
        super(broker);
        targetPort = port;
        Futures.addCallback(getFuture(), this);
    }

    // TxTask

    /**
     * {@inheritDoc}
     */
    @Override
    protected void execute(ReadWriteTransaction tx) {
        String nid = targetPort.getNodeIdentifier();
        String pid = targetPort.getPortIdentifier();
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        DataStoreUtils.delete(
            tx, oper, TopologyUtils.getTerminationPointPath(nid, pid));
        new LinkCleaner.Port(pid).delete(tx);
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
        LOG.trace("Termination point has been deleted: {}",
                  targetPort.getPortIdentifier());
    }

    /**
     * Invoked when the specified data did not deleted.
     *
     * @param cause  A throwable that indicates the cause of error.
     */
    @Override
    public void onFailure(Throwable cause) {
        String msg = "Failed to delete termination point: " +
            targetPort.getPortIdentifier();
        LOG.error(msg, cause);
    }
}
