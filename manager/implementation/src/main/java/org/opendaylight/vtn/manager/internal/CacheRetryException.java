/*
 * Copyright (c) 2014, 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.controller.sal.utils.Status;

/**
 * An exception which indicates the cluster cache operation should be
 * retried.
 */
public class CacheRetryException extends VTNException {
    /**
     * Construct a new instance.
     */
    public CacheRetryException() {
        super((Status)null);
    }

    /**
     * Construct a new instance.
     *
     * @param cause  The {@link Throwable} object which indicates the cause of
     *               an error.
     */
    public CacheRetryException(Throwable cause) {
        super((Status)null, cause);
    }
}
