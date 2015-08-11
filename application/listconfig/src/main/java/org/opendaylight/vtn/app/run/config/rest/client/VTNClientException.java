/*
 * Copyright (c) 2014, 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.app.run.config.rest.client;

/**
 * {@code VTNClientException} is an exception for notifying errors that occur in
 *  the VTNClient running application.
 *
 * <p>
 *   This exception is used for notifying the {@link String} which indicates
 *   the result of the operation.
 * </p>
 */
public class VTNClientException extends Exception {

    /**
     * The {@link String} object which indicates the result of the operation.
     */
    private final String status;

    /**
     * Construct a new {@code VTNClientException} object that internally stores
     * the {@link String} object specified by {@code status}.
     *
     * <p>
     *   The message configured in {@code status} will get configured in
     *   the message of the exception.
     * </p>
     *
     * @param status   A {@code String} object to be delivered as exception.
     */
    public VTNClientException(String status) {
        super((status == null) ? null : status);
        this.status = status;
    }

    /**
     * Consturct a new {@code VTNClientException} object that indicates the
     * VTNClient running applicaiton has caught an unexpected exception
     * specified by {@code cause}.
     *
     * <p>
     *   The message configured in {@code status} will get configured in
     *   the message of the exception.
     * </p>
     *
     * @param status   A {@code String} object to be delivered as exception.
     * @param cause    The {@link Throwable} object which indicates the cause
     *                 of an error.
     */
    public VTNClientException(String status, Throwable cause) {
        super((status == null) ? null : status, cause);
        this.status = status;
    }

    /**
     * Return the {@link String} object which indicates the result of the
     * operation.
     *
     * @return  The {@link String} object which indicates the result of the
     *          operation.
     */
    public String getStatus() {
        return status;
    }
}
