/*
 * Copyright (c) 2014, 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal;

import java.util.HashMap;
import java.util.Map;
import java.util.Set;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ConcurrentMap;
import java.util.concurrent.ConcurrentSkipListSet;
import java.util.concurrent.TimeUnit;

import javax.transaction.Transaction;
import javax.transaction.SystemException;
import javax.transaction.Synchronization;
import javax.transaction.xa.XAResource;

/**
 * An implementation of {@link Transaction} for test.
 */
public class TestTransaction implements Transaction {
    /**
     * Transaction test mode.
     */
    public enum Mode {
        /**
         * All operations should succeed.
         */
        PASS,

        /**
         * Transaction should not start.
         */
        FAIL_BEGIN,

        /**
         * Transaction should abort.
         */
        FAIL_ABORT,
    }

    /**
     * A message for a exception caused due to {@link Mode#FAIL_BEGIN}.
     */
    public static final String  ERR_FAIL_BEGIN =
        "Transaction begin failure test.";

    /**
     * A message for a exception caused due to {@link Mode#FAIL_ABORT}.
     */
    public static final String  ERR_FAIL_ABORT = "Transaction abort test.";

    /**
     * Status value which indicates the transaction is active.
     */
    public static final int  STATUS_ACTIVE = 0;

    /**
     * Status value which indicates the transaction is committed.
     */
    public static final int  STATUS_COMMITTED = 1;

    /**
     * Status value which indicates the transaction is rollbacked.
     */
    public static final int  STATUS_ROLLBACKED = 2;

    /**
     * Keep transaction test mode.
     */
    private final Mode  mode;

    /**
     * Transaction status.
     */
    private int  status = STATUS_ACTIVE;

    /**
     * Timeout value used when this transaction begins.
     */
    private final long  timeout;

    /**
     * A {@link TimeUnit} object used when this transaction begins.
     */
    private final TimeUnit  timeUnit;

    /**
     * Backup of cluster caches.
     */
    private final Map<String, ConcurrentMap<?, ?>> cacheBackup =
        new HashMap<String, ConcurrentMap<?, ?>>();

    /**
     * A set of transactional cache names.
     */
    private final Set<String>  transactionalCaches;

    /**
     * Construct a new transaction.
     *
     * @param mode     Transaction test mode.
     * @param timeout  Timeout value used when this transaction begins.
     * @param unit     A {@link TimeUnit} object used when this transaction
     *                 begins.
     * @param caches   A map which contains all cluster caches.
     * @param xnames   A set of transactional cache names.
     * @throws SystemException
     *    {@link Mode#FAIL_BEGIN} is specified to {@code mode}.
     */
    public TestTransaction(Mode mode, long timeout, TimeUnit unit,
                           Map<String, ConcurrentMap<?, ?>> caches,
                           Set<String> xnames)
        throws SystemException {
        if (mode == Mode.FAIL_BEGIN) {
            throw new SystemException(ERR_FAIL_BEGIN);
        }

        this.mode = (mode == null) ? Mode.PASS : mode;
        this.timeout = timeout;
        this.timeUnit = unit;
        this.transactionalCaches = new ConcurrentSkipListSet<String>(xnames);

        copy(cacheBackup, caches, true);
    }

    /**
     * Rollback changes to cluster caches.
     *
     * @param map  A map which contains cluster caches.
     */
    public void restore(Map<String, ConcurrentMap<?, ?>> map) {
        copy(map, cacheBackup, false);
    }

    /**
     * Return a timeout value used when this transaction begins.
     *
     * @return  A timeout value.
     */
    public long getTimeout() {
        return timeout;
    }

    /**
     * Return a {@link TimeUnit} object used when this transaction begins.
     *
     * @return  A {@link TimeUnit} object.
     */
    public TimeUnit getTimeUnit() {
        return timeUnit;
    }

    /**
     * Commit this transaction.
     *
     * @throws SystemException
     *   Test mode is {@link Mode#FAIL_ABORT}.
     * @throws IllegalStateException
     *   The transaction is not active.
     */
    @Override
    public synchronized void commit() throws SystemException {
        if (status != STATUS_ACTIVE) {
            throw new IllegalStateException("Transaction is not active.");
        }
        if (mode == Mode.FAIL_ABORT) {
            throw new SystemException(ERR_FAIL_ABORT);
        }
        status = STATUS_COMMITTED;
    }

    /**
     * Rollback the transaction.
     *
     * @throws IllegalStateException
     *   The transaction is not active.
     */
    @Override
    public synchronized void rollback() {
        if (status != STATUS_ACTIVE) {
            throw new IllegalStateException("Transaction is not active.");
        }
        status = STATUS_ROLLBACKED;
    }

    /**
     * Return the status of the transaction.
     *
     * @return  One of {@link #STATUS_ACTIVE}, {@link #STATUS_COMMITTED},
     *          {@link #STATUS_ROLLBACKED} is returned.
     */
    @Override
    public synchronized int getStatus() {
        return status;
    }

    /**
     * Do nothing.
     *
     * @return  {@code true} is always returned.
     */
    @Override
    public boolean delistResource(XAResource xaRes, int flag) {
        return true;
    }

    /**
     * Do nothing.
     *
     * @return  {@code true} is always returned.
     */
    @Override
    public boolean enlistResource(XAResource xaRes) {
        return true;
    }

    /**
     * Do nothing.
     */
    @Override
    public void registerSynchronization(Synchronization sync) {
    }

    /**
     * Do nothing.
     */
    @Override
    public void setRollbackOnly() {
    }

    /**
     * Copy the cluster caches.
     *
     * @param dst   A map to store copies of caches.
     * @param src   A map which contains cluster caches.
     * @param deep  Copy caches if {@code true}.
     */
    private void copy(Map<String, ConcurrentMap<?, ?>> dst,
                      Map<String, ConcurrentMap<?, ?>> src, boolean deep) {
        for (Map.Entry<String, ConcurrentMap<?, ?>> entry: src.entrySet()) {
            String name = entry.getKey();
            if (!transactionalCaches.contains(name)) {
                continue;
            }

            ConcurrentMap<Object, Object> cache =
                (ConcurrentMap<Object, Object>)entry.getValue();
            if (deep) {
                cache = new ConcurrentHashMap<Object, Object>(cache);
                dst.put(name, cache);
            } else {
                ConcurrentMap<Object, Object> current =
                    (ConcurrentMap<Object, Object>)dst.get(name);
                if (current != null) {
                    current.clear();
                    current.putAll(cache);
                }
            }
        }
    }
}
