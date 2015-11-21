/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.flow.filter;

/**
 * An exception which indicates a packet was discarded by a DROP flow filter.
 */
public final class DropFlowException extends Exception {
    /**
     * Construct a new instance.
     */
    public DropFlowException() {
        super();
    }

    /**
     * Construct a new instance.
     *
     * @param cause  The {@link Throwable} object which indicates the cause
     *               of error.
     */
    public DropFlowException(Throwable cause) {
        super(cause);
    }
}
