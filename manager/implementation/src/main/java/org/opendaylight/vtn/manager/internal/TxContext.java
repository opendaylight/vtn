/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal;

import org.slf4j.Logger;

import org.opendaylight.vtn.manager.internal.util.log.VTNLogLevel;

import org.opendaylight.controller.md.sal.binding.api.ReadTransaction;
import org.opendaylight.controller.md.sal.binding.api.ReadWriteTransaction;

/**
 * {@code TxContext} represents a runtime context used by tasks that uses
 * MD-SAL datastore.
 *
 * @see TxTask
 */
public interface TxContext {
    /**
     * Return read-only transaction for MD-SAL datastore.
     *
     * @return  A {@link ReadTransaction} instance.
     */
    ReadTransaction getTransaction();

    /**
     * Return read/write transaction for MD-SAL datastore.
     *
     * @return  A {@link ReadWriteTransaction} instance.
     */
    ReadWriteTransaction getReadWriteTransaction();

    /**
     * Cancel current transaction for MD-SAL datastore.
     */
    void cancelTransaction();

    /**
     * Return read-only data specific to the current transaction.
     *
     * <p>
     *   If an object specified by the given class is not associated with the
     *   current transaction, a new object is instantiated and it is associated
     *   with the current transaction. All the transaction specific data will
     *   be discarded when the transaction is closed.
     * </p>
     *
     * @param type  A class that specifies the type of data.
     *              Note that the class must have a public constructor that
     *              takes one {@link ReadTransaction} instance.
     * @param <T>   The type of the transaction specific data.
     * @return  An object associated with the current transaction.
     */
    <T> T getReadSpecific(Class<T> type);

    /**
     * Return writable data specific to the current transaction.
     *
     * <p>
     *   If an object specified by the given class is not associated with the
     *   current transaction, a new object is instantiated and it is associated
     *   with the current transaction. All the transaction specific data will
     *   be discarded when the transaction is closed.
     * </p>
     *
     * @param type  A class that specifies the type of data.
     *              Note that the class must have a public constructor that
     *              takes one {@link TxContext} instance.
     * @param <T>   The type of the transaction specific data.
     * @return  An object associated with the current transaction.
     */
    <T> T getSpecific(Class<T> type);

    /**
     * Add the given transaction pre-submit hook to this context.
     *
     * <p>
     *   {@link TxHook#run(TxContext, TxTask)} on the given hook instance will
     *   be invoked just before the transaction is submitted.
     * </p>
     * <p>
     *   Note that all the registered hooks are removed from this context
     *   when the transaction is closed.
     * </p>
     *
     * @param hook  A hook to be invoked when the transaction is going to be
     *              submitted.
     */
    void addPreSubmitHook(TxHook hook);

    /**
     * Add the given transaction post-submit hook to this context.
     *
     * <p>
     *   {@link TxHook#run(TxContext, TxTask)} on the given hook instance will
     *   be invoked just after the successful completion of the transaction.
     * </p>
     * <p>
     *   Note that all the registered hooks are removed from this context
     *   when the transaction is closed.
     * </p>
     *
     * @param hook  A hook to be invoked after the successful completion of
     *              the transaction.
     */
    void addPostSubmitHook(TxHook hook);

    /**
     * Return a {@link VTNManagerProvider} service instance.
     *
     * @return  A {@link VTNManagerProvider} instance.
     */
    VTNManagerProvider getProvider();

    /**
     * Log the specified message.
     *
     * <p>
     *   If the MD-SAL transaction associated with this context is writable,
     *   the specified message will be logged when the transaction completes.
     *   Note that the specified message will be discarded when the transaction
     *   is restarted due to data confliction.
     * </p>
     *
     * @param logger  A {@link Logger} instance.
     * @param level   A {@link VTNLogLevel} instance that specifies the
     *                logging level.
     * @param msg     A message to be logged.
     */
    void log(Logger logger, VTNLogLevel level, String msg);

    /**
     * Log a message according to the specified format string and arguments.
     *
     * <p>
     *   This method constructs a message using SLF4J log format.
     * </p>
     * <p>
     *   If the MD-SAL transaction associated with this context is writable,
     *   the specified message will be logged when the transaction completes.
     *   Note that the specified message will be discarded when the transaction
     *   is restarted due to data confliction.
     * </p>
     *
     * @param logger  A {@link Logger} instance.
     * @param level   A {@link VTNLogLevel} instance that specifies the
     *                logging level.
     * @param format  A format string used to construct log message.
     * @param args    An object array used to construct log message.
     */
    void log(Logger logger, VTNLogLevel level, String format, Object ... args);

    /**
     * Log the specified message and exception.
     *
     * <p>
     *   If the MD-SAL transaction associated with this context is writable,
     *   the specified message will be logged when the transaction completes.
     *   Note that the specified message will be discarded when the transaction
     *   is restarted due to data confliction.
     * </p>
     *
     * @param logger  A {@link Logger} instance.
     * @param level   A {@link VTNLogLevel} instance that specifies the
     *                logging level.
     * @param msg     A message to be logged.
     * @param t       A {@link Throwable} to be logged.
     */
    void log(Logger logger, VTNLogLevel level, String msg, Throwable t);

    /**
     * Log the specified throwable and a message according to the given
     * format string and arguments.
     *
     * <p>
     *   Note that this method constructs a log message using
     *   {@link String#format(String, Object[])}.
     * </p>
     * <p>
     *   If the MD-SAL transaction associated with this context is writable,
     *   the specified message will be logged when the transaction completes.
     *   Note that the specified message will be discarded when the transaction
     *   is restarted due to data confliction.
     * </p>
     *
     * @param logger  A {@link Logger} instance.
     * @param level   A {@link VTNLogLevel} instance that specifies the
     *                logging level.
     * @param t       A {@link Throwable} to be logged.
     * @param format  A format string used to construct log message.
     * @param args    An object array used to construct log message.
     */
    void log(Logger logger, VTNLogLevel level, Throwable t, String format,
             Object ... args);
}
