/*
 * Copyright (c) 2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal;

import java.util.concurrent.ExecutionException;

import com.google.common.base.Function;

import org.opendaylight.controller.md.sal.common.api.data.TransactionCommitFailedException;

import org.opendaylight.yangtools.yang.common.RpcError.ErrorType;
import org.opendaylight.yangtools.yang.common.RpcError;
import org.opendaylight.yangtools.yang.common.RpcResultBuilder;

/**
 * Exception mapper which converts a throwable into
 * {@link TransactionCommitFailedException}.
 */
public final class DataStoreExceptionMapper
    implements Function<Throwable, TransactionCommitFailedException> {
    /**
     * The global instance.
     */
    private static final DataStoreExceptionMapper  INSTANCE =
        new DataStoreExceptionMapper();

    /**
     * Return the global instance.
     *
     * @return  A {@link DataStoreExceptionMapper} instance.
     */
    public static DataStoreExceptionMapper getInstance() {
        return INSTANCE;
    }

    /**
     * Private constructor.
     */
    private DataStoreExceptionMapper() {}

    // Function

    /**
     * Convert the given throwable into a
     * {@link TransactionCommitFailedException}.
     *
     * @param t  A throwable to be converted.
     * @return  A {@link TransactionCommitFailedException} instance.
     */
    @Override
    public TransactionCommitFailedException apply(Throwable t) {
        if (t instanceof TransactionCommitFailedException) {
            return (TransactionCommitFailedException)t;
        }

        Throwable cause = t;
        if (cause instanceof ExecutionException) {
            cause = cause.getCause();
        }

        String msg = "DS commit failed";
        RpcError err = RpcResultBuilder.newError(
            ErrorType.APPLICATION, "failed", msg, null, null, cause);
        return new TransactionCommitFailedException(msg, cause, err);
    }
}
