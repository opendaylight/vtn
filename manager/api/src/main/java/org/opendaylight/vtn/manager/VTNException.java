/*
 * Copyright (c) 2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager;

import org.opendaylight.controller.sal.utils.Status;
import org.opendaylight.controller.sal.utils.StatusCode;

/**
 * The {@code VTNException} class is an exception thrown by VTN Manager API.
 *
 * <p>
 *   This exception is used to throw a failure status as an exception.
 * </p>
 */
public class VTNException extends Exception {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = -3983343760793384085L;

    /**
     * Operation status.
     */
    private final Status  status;

    /**
     * Construct a VTN manager exception.
     *
     * @param status   A status to be delivered as exception.
     */
    public VTNException(Status status) {
        super((status == null) ? null : status.toString());
        this.status = status;
    }

    /**
     * Consturct a VTN exception with SAL status and cause of exception.
     *
     * @param status   A status to be delivered as exception.
     * @param cause    The cause of an error.
     */
    public VTNException(Status status, Throwable cause) {
        super((status == null) ? null : status.toString(), cause);
        this.status = status;
    }

    /**
     * Construct a VTN exception.
     *
     * @param code  The status code.
     * @param desc  Description about the status.
     */
    public VTNException(StatusCode code, String desc) {
        this(new Status(code, desc));
    }

    /**
     * Construct a new VTN exception which represents an error caused by
     * unexpected exception.
     *
     * @param message  The detailed message.
     * @param cause    The cause of an error.
     */
    public VTNException(String message, Throwable cause) {
        this(new Status(StatusCode.INTERNALERROR, message), cause);
    }

    /**
     * Get status which indicates the cause of error.
     *
     * @return A status.
     */
    public Status getStatus() {
        return status;
    }
}
