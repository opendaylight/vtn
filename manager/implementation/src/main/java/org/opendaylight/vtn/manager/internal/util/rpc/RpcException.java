/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.rpc;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.controller.sal.utils.Status;
import org.opendaylight.controller.sal.utils.StatusCode;

/**
 * {@code RpcException} is an exception which indicates the failure of the
 * RPC request.
 */
public final class RpcException extends VTNException {
    /**
     * An {@link RpcErrorTag} to be returned as error tag.
     */
    private final RpcErrorTag  errorTag;

    /**
     * Return a new {@link RpcException} which notifies an invalid argument.
     *
     * @param desc  Brief description of the argument.
     * @return  An {@link RpcException}.
     */
    public static RpcException getBadArgumentException(String desc) {
        return new RpcException(RpcErrorTag.BAD_ELEMENT,
                                StatusCode.BADREQUEST, desc);
    }

    /**
     * Return a new {@link RpcException} which notifies an missing element
     * in the request.
     *
     * @param desc  Brief description of the argument.
     * @return  An {@link RpcException}.
     */
    public static RpcException getMissingArgumentException(String desc) {
        return new RpcException(RpcErrorTag.MISSING_ELEMENT,
                                StatusCode.BADREQUEST, desc);
    }

    /**
     * Return a new {@link RpcException} which notifies an missing data
     * object.
     *
     * @param desc  Brief description of the missing data.
     * @return  An {@link RpcException}.
     */
    public static RpcException getNotFoundException(String desc) {
        return getNotFoundException(desc, null);
    }

    /**
     * Return a new {@link RpcException} which notifies an missing data
     * object.
     *
     * @param desc   Brief description of the missing data.
     * @param cause  A {@link Throwable} which indicates the cause of error.
     * @return  An {@link RpcException}.
     */
    public static RpcException getNotFoundException(String desc,
                                                    Throwable cause) {
        return new RpcException(RpcErrorTag.DATA_MISSING,
                                new Status(StatusCode.NOTFOUND, desc), cause);
    }

    /**
     * Construct a new exception that internally stores the given
     * {@link Status} and {@link RpcErrorTag} instances.
     *
     * @param tag     A {@link RpcErrorTag} instance.
     * @param status  A {@link Status} instance.
     */
    public RpcException(RpcErrorTag tag, Status status) {
        super(status);
        errorTag = tag;
    }

    /**
     * Construct a new exception that indicates the VTN Manager has caught
     * an unexpected exception while it is processing the RPC request.
     *
     * @param tag     A {@link RpcErrorTag} instance.
     * @param status  A {@link Status} instance.
     * @param cause   The {@link Throwable} instance which indicates the cause
     *                of error.
     */
    public RpcException(RpcErrorTag tag, Status status, Throwable cause) {
        super(status, cause);
        errorTag = tag;
    }

    /**
     * Construct a new exception that internally stores the {@link Status}
     * instance created from the given status code and description.
     *
     * @param tag   A {@link RpcErrorTag} instance.
     * @param code  The status code which indicates the cause of error.
     * @param desc  Description about the status.
     */
    public RpcException(RpcErrorTag tag, StatusCode code, String desc) {
        super(code, desc);
        errorTag = tag;
    }

    /**
     * Construct a new exception that indicates an unexpected internall error
     * has been detected.
     *
     * @param message  The defailed message.
     * @param cause   The {@link Throwable} instance which indicates the cause
     *                of error.
     */
    public RpcException(String message, Throwable cause) {
        super(message, cause);
        errorTag = null;
    }

    /**
     * Return the {@link RpcErrorTag} instance to be set as error-tag in
     * RPC error.
     *
     * @return  An {@link RpcErrorTag} instance.
     */
    public RpcErrorTag getErrorTag() {
        return (errorTag == null) ? RpcErrorTag.OPERATION_FAILED : errorTag;
    }
}
