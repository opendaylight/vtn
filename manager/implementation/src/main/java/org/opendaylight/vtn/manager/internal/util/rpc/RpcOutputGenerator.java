/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.rpc;

/**
 * {@code RpcOutputGenerator} describes interfaces to be implemented by the
 * class that generates output of the RPC request.
 *
 * @param <I>   The type of the result of the RPC main procedure.
 * @param <O>   The type of the RPC output.
 */
public interface RpcOutputGenerator<I, O> {
    /**
     * Return a class which indicates the type of the RPC output.
     *
     * @return  A class which indicates the type of the RPC output.
     */
    Class<O> getOutputType();

    /**
     * Generate the output of the RPC request.
     *
     * @param result  The result of the RPC main procedure.
     * @return  The output of the RPC request.
     */
    O createOutput(I result);
}
