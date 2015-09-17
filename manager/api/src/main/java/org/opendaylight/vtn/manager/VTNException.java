/*
 * Copyright (c) 2013, 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager;

import org.opendaylight.controller.sal.utils.Status;
import org.opendaylight.controller.sal.utils.StatusCode;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnErrorTag;

/**
 * {@code VTNException} is an exception for notifying errors that occur in
 *  OSGi services provided by the VTN Manager.
 *
 * <p>
 *   This exception is used for notifying the {@link Status} which indicates
 *   the result of the operation.
 * </p>
 */
public class VTNException extends Exception {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = -1262222199567926358L;

    /**
     * The {@link VtnErrorTag} object which indicates the result of the
     * operation.
     */
    private final VtnErrorTag  errorTag;

    /**
     * Consturct a new {@code VTNException} instance that internally stores
     * the given message and {@link VtnErrorTag}.
     *
     * @param etag  The error tag which indicates the cause of error.
     * @param msg   The detailed message.
     */
    public VTNException(VtnErrorTag etag, String msg) {
        super(msg);
        errorTag = (etag == null) ? VtnErrorTag.INTERNALERROR : etag;
    }

    /**
     * Consturct a new {@code VTNException} instance that internally stores
     * the given message {@link VtnErrorTag}.
     *
     * @param etag   The error tag which indicates the cause of error.
     * @param msg    The detailed message.
     * @param cause  The {@link Throwable} object which indicates the cause
     *               of error.
     */
    public VTNException(VtnErrorTag etag, String msg, Throwable cause) {
        super(msg, cause);
        errorTag = (etag == null) ? VtnErrorTag.INTERNALERROR : etag;
    }

    /**
     * Consturct a new {@code VTNException} that indicates an internal error.
     *
     * <ul>
     *   <li>
     *     A new exception internally stores {@link VtnErrorTag#INTERNALERROR}.
     *   </li>
     * </ul>
     *
     * @param msg  The detailed message.
     * @since  Lithium
     */
    public VTNException(String msg) {
        this(null, msg);
    }

    /**
     * Consturct a new {@code VTNException} that indicates an internal error.
     *
     * <ul>
     *   <li>
     *     A new exception internally stores {@link VtnErrorTag#INTERNALERROR}.
     *   </li>
     * </ul>
     *
     * @param msg    The detailed message.
     * @param cause  The {@link Throwable} object which indicates the cause
     *               of error.
     */
    public VTNException(String msg, Throwable cause) {
        this(null, msg, cause);
    }

    /**
     * Construct a new instance from the given {@link Status} instance.
     *
     * @param status  A {@link Status} instance.
     */
    public VTNException(Status status) {
        super(status.getDescription());
        errorTag = toVtnErrorTag(status.getCode());
    }

    /**
     * Return the {@link VtnErrorTag} instance which indicates the result of
     * the operation.
     *
     * @return  A {@link VtnErrorTag} instance.
     */
    public final VtnErrorTag getVtnErrorTag() {
        return errorTag;
    }

    /**
     * Return the {@link Status} object which indicates the result of the
     * operation.
     *
     * @return  The {@link Status} object which indicates the result of the
     *          operation.
     */
    public final Status getStatus() {
        StatusCode code;
        try {
            code = StatusCode.valueOf(errorTag.name());
        } catch (Exception e) {
            // This should never happen.
            code = StatusCode.UNDEFINED;
        }

        return new Status(code, getMessage());
    }

    /**
     * Convert the given {@link StatusCode} into a {@link VtnErrorTag}.
     *
     * @param code  A {@link StatusCode}.
     * @return  A {@link VtnErrorTag} instance associated with {@code code}.
     */
    private VtnErrorTag toVtnErrorTag(StatusCode code) {
        try {
            return VtnErrorTag.valueOf(code.name());
        } catch (Exception e) {
            return VtnErrorTag.INTERNALERROR;
        }
    }
}
