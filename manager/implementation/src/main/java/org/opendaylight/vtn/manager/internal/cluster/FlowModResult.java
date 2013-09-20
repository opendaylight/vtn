/*
 * Copyright (c) 2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.cluster;

import org.opendaylight.controller.sal.utils.Status;
import org.opendaylight.controller.sal.utils.StatusCode;

/**
 * {@code FlowModResult} defines symbols which represent result of
 * flow modification.
 */
public enum FlowModResult {
    /**
     * Indicates that flow modification completed successfully.
     */
    SUCCEEDED,

    /**
     * Indicates that flow modification failed.
     */
    FAILED,

    /**
     * Indicates that flow modification was ignored because the target node
     * is not connected to the local node.
     */
    IGNORED,

    /**
     * Indicates that the operation was timed out.
     */
    TIMEDOUT,

    /**
     * Indicates that the operation was interrupted.
     */
    INTERRUPTED;

    /**
     * Convert flow modification result into SAL status.
     *
     * @return  SAL status.
     */
    public Status toStatus() {
        Status status;

        switch (this) {
        case SUCCEEDED:
            status = new Status(StatusCode.SUCCESS, null);
            break;

        case TIMEDOUT:
            status = new Status(StatusCode.TIMEOUT, "Operation timed out");
            break;

        default:
            status = new Status(StatusCode.INTERNALERROR,
                                "Operation failed: " + this);
            break;
        }

        return status;
    }
}
