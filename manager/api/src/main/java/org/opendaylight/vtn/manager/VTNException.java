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
    private static final long serialVersionUID = -7988902574477146543L;

    /**
     * The {@link Status} object which indicates the result of the operation.
     */
    private final Status  status;

    /**
     * Construct a new {@code VTNException} object that internally stores
     * the {@link Status} object specified by {@code status}.
     *
     * <p>
     *   The message configured in {@code status} will get configured in
     *   the message of the exception.
     * </p>
     *
     * @param status   A {@code Status} object to be delivered as exception.
     */
    public VTNException(Status status) {
        super((status == null) ? null : status.toString());
        this.status = status;
    }

    /**
     * Consturct a new {@code VTNException} object that indicates the
     * VTN Manager has caught an unexpected exception specified by
     * {@code cause}.
     *
     * <p>
     *   The message configured in {@code status} will get configured in
     *   the message of the exception.
     * </p>
     *
     * @param status   A {@code Status} object to be delivered as exception.
     * @param cause    The {@link Throwable} object which indicates the cause
     *                 of an error.
     */
    public VTNException(Status status, Throwable cause) {
        super((status == null) ? null : status.toString(), cause);
        this.status = status;
    }

    /**
     * Consturct a new {@code VTNException} object that internally stores
     * the {@link Status} object created from {@code code} and {@code desc}.
     *
     * <p>
     *   A string specified by {@code desc} will get configured in
     *   the message of the exception.
     * </p>
     *
     * @param code  The status code which indicates the cause of error.
     * @param desc  Description about the status.
     */
    public VTNException(StatusCode code, String desc) {
        this(new Status(code, desc));
    }

    /**
     * Consturct a new {@code VTNException} that indicates an internal error.
     *
     * <ul>
     *   <li>
     *     The {@link Status} object for a new exception is created from
     *     {@link StatusCode#INTERNALERROR} and the message specified by
     *     {@code message}.
     *   </li>
     *   <li>
     *     A string specified by {@code message} will get configured in
     *     the message of the exception.
     *   </li>
     * </ul>
     *
     * @param message  The detailed message.
     * @since  Lithium
     */
    public VTNException(String message) {
        this(new Status(StatusCode.INTERNALERROR, message));
    }

    /**
     * Consturct a new {@code VTNException} that indicates an internal error.
     *
     * <ul>
     *   <li>
     *     The {@link Status} object for a new exception is created from
     *     {@link StatusCode#INTERNALERROR} and the message specified by
     *     {@code message}.
     *   </li>
     *   <li>
     *     A string specified by {@code message} will get configured in
     *     the message of the exception.
     *   </li>
     * </ul>
     *
     * @param message  The detailed message.
     * @param cause    The {@link Throwable} object which indicates the cause
     *                 of error.
     */
    public VTNException(String message, Throwable cause) {
        this(new Status(StatusCode.INTERNALERROR, message), cause);
    }

    /**
     * Return the {@link Status} object which indicates the result of the
     * operation.
     *
     * @return  The {@link Status} object which indicates the result of the
     *          operation.
     */
    public Status getStatus() {
        return status;
    }
}
