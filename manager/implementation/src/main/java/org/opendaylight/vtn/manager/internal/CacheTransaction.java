/*
 * Copyright (c) 2014, 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal;

import java.util.concurrent.TimeUnit;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.controller.clustering.services.IClusterServicesCommon;

/**
 * This class is used to execute an arbitrary procedure in a cluster cache
 * transaction.
 *
 * A sub-class of this class can define an arbitrary procedure to be executed
 * in a cluster cache transaction by implementing {@link #executeImpl()}.
 *
 * @param <T>  The type of the object returned by {@link #execute()}.
 */
public abstract class CacheTransaction<T> {
    /**
     * Logger instance.
     */
    private static final Logger  LOG =
        LoggerFactory.getLogger(CacheTransaction.class);

    /**
     * The number of milliseconds to be inserted before retrying cache
     * operation.
     */
    private static final long  RETRY_DELAY = 10;

    /**
     * A cluster service instance.
     */
    private final IClusterServicesCommon  cluster;

    /**
     * The number of milliseconds to wait for a cluster cache transaction
     * to be established.
     */
    private final long  timeout;

    /**
     * A boolean value which determines a cluster cache transaction should be
     * aborted or not.
     */
    private boolean  doAbort = false;

    /**
     * Construct a new object.
     *
     * @param cluster  A cluster service instance.
     *                 Specifying {@code null} results in undefined behavior.
     * @param timeout  The number of milliseconds to wait for a cluster cache
     *                 transaction to be established.
     */
    public CacheTransaction(IClusterServicesCommon cluster, long timeout) {
        this.cluster = cluster;
        this.timeout = timeout;
    }

    /**
     * Execute a procedure in a cluster cache transaction.
     *
     * @return  An object returned by {@link #executeImpl()}.
     * @throws VTNException
     *   An exception was thrown by {@link #executeImpl()}, or a cluster cache
     *   transaction could not be established.
     */
    public final T execute() throws VTNException {
        IClusterServicesCommon c = cluster;

        while (true) {
            boolean xact = false;

            try {
                c.tbegin(timeout, TimeUnit.MILLISECONDS);
                xact = true;

                T ret = executeImpl();
                if (!doAbort) {
                    c.tcommit();
                    xact = false;
                }

                return ret;
            } catch (CacheRetryException e) {
                LOG.debug("Retry cache operation after {} milliseconds: {}",
                          RETRY_DELAY, e);
                try {
                    Thread.sleep(RETRY_DELAY);
                } catch (InterruptedException ex) {
                    // Ignore interruption.
                }
            } catch (VTNException e) {
                throw e;
            } catch (NullPointerException e) {
                return handleNull(e);
            } catch (Exception e) {
                String msg = "Cluster cache transaction abort";
                LOG.error(msg, e);
                throw new VTNException(msg, e);
            } finally {
                if (xact) {
                    try {
                        cluster.trollback();
                    } catch (Exception e) {
                        LOG.error("Failed to rollback cluster cache " +
                                  "transaction", e);
                    }
                }
            }
        }
    }

    /**
     * Notify that the current cluster cache transaction should be aborted.
     */
    protected final void abort() {
        doAbort = true;
    }

    /**
     * Handle {@link NullPointerException} caused by {@link #execute()}.
     *
     * @param e  A {@link NullPointerException} caught in {@link #execute()}.
     * @return  An object returned by {@link #executeImpl()}.
     * @throws VTNException
     *   An exception was thrown by {@link #executeImpl()}, or a cluster cache
     *   transaction could not be established.
     * @throws NullPointerException
     *   The specified exception can not be ignored.
     */
    private T handleNull(NullPointerException e) throws VTNException {
        if (cluster != null) {
            throw e;
        }

        // This code is only for unit test.
        return executeImpl();
    }

    /**
     * Implement an arbitrary procedure to be executed in a cluster cache
     * transaction.
     *
     * @return  An arbitrary object.
     * @throws VTNException
     *   A fatal error occurred.
     */
    protected abstract T executeImpl() throws VTNException;
}
