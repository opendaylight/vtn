/*
 * Copyright (c) 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.rpc;

import java.util.Collection;

import javax.annotation.Nonnull;

import org.opendaylight.yangtools.yang.common.RpcError;

/**
 * {@code RpcRequest} describes an RPC request.
 *
 * <p>
 *   This interface is used to embed RPC request information into a log
 *   message.
 * </p>
 */
public interface RpcRequest {
    /**
     * Return the name of the RPC.
     *
     * @return  The name of the RPC.
     */
    String getName();

    /**
     * Return an object that indicates an RPC input.
     *
     * <p>
     *   Returned object is embedded into a log message.
     * </p>
     *
     * @return  An object that indicates an RPC input.
     */
    Object getInputForLog();

    /**
     * Determine whether the RPC failure should be logged or not.
     *
     * @param errors  A collection of RPC errors returned by the RPC
     *                implementation.
     * @return  {@code true} if the RPC failure should be logged.
     *          {@code false} otherwise.
     */
    boolean needErrorLog(@Nonnull Collection<RpcError> errors);
}
