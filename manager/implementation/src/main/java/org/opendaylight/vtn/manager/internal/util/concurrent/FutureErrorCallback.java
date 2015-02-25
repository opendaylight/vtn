/*
 * Copyright (c) 2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.concurrent;

import org.slf4j.Logger;

import com.google.common.util.concurrent.FutureCallback;

/**
 * An implementation of {@link FutureCallback} that records an error log
 * when the future has failed.
 *
 * @param <T>  The type of objects to be returned by the future.
 */
public class FutureErrorCallback<T> implements FutureCallback<T> {
    /**
     * A logger instance.
     */
    private final Logger  logger;

    /**
     * A format string to construct an error message.
     */
    private final String  format;

    /**
     * An array of objects used to construct an error message.
     */
    private final Object[]  arguments;

    /**
     * Construct a new instance.
     *
     * <p>
     *   This constructor takes a format string and variable-sized arguments.
     *   An error message is created by {@link String#format(String, Object[])}
     *   with specifying the given format string and arguments.
     * </p>
     *
     * @param log   A logger instance.
     * @param fmt   A format string.
     * @param args  Arguments for an error message.
     */
    public FutureErrorCallback(Logger log, String fmt, Object ... args) {
        logger = log;
        format = fmt;
        arguments = (args == null) ? null : args.clone();
    }

    // FutureCallback

    /**
     * Invoked when the future has completed successfully.
     *
     * <p>
     *   This method defined in this class does nothing.
     * </p>
     *
     * @param result  An object returned by the future.
     */
    @Override
    public void onSuccess(T result) {
    }

    /**
     * Invoked when the future has failed.
     *
     * @param t  A {@link Throwable} caught by the future.
     */
    @Override
    public void onFailure(Throwable t) {
        String msg = String.format(format, arguments);
        logger.error(msg, t);
    }
}
