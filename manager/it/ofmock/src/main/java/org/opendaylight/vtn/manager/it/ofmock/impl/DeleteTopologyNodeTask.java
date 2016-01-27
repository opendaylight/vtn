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
 * {@code DeleteTopologyNodeTask} describes a task that deletes a topology node
 * from the MD-SAL datastore.
 */
public final class DeleteTopologyNodeTask extends TxTask
    implements FutureCallback<Void> {
    /**
     * Logger instance.
     */
    private static final Logger  LOG =
        LoggerFactory.getLogger(DeleteTopologyNodeTask.class);

    /**
     * The MD-SAL node identifier that specifies the target switch.
     */
    private final String  nodeId;

    /**
     * Construct a new instance.
     *
     * @param broker  The data broker service.
     * @param nid     The MD-SAL node identifier that specifies the target
     *                switch.
     */
    public DeleteTopologyNodeTask(DataBroker broker, String nid) {
        super(broker);
        nodeId = nid;
        Futures.addCallback(getFuture(), this);
    }

    // TxTask

    /**
     * {@inheritDoc}
     */
    @Override
    protected void execute(ReadWriteTransaction tx) {
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        DataStoreUtils.delete(
            tx, oper, TopologyUtils.getTopologyNodePath(nodeId));
        new LinkCleaner.Node(nodeId).delete(tx);
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
        LOG.trace("Topology node has been deleted: {}", nodeId);
    }

    /**
     * Invoked when the specified data did not deleted.
     *
     * @param cause  A throwable that indicates the cause of error.
     */
    @Override
    public void onFailure(Throwable cause) {
        String msg = "Failed to delete topology node: " + nodeId;
        LOG.error(msg, cause);
    }
}
