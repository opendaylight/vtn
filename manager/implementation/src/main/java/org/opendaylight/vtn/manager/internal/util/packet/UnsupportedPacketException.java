/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.packet;

/**
 * An exception which indicates the specified packet is not supported.
 */
public final class UnsupportedPacketException extends Exception {
    /**
     * Construct a new exception.
     *
     * @param msg  A message.
     */
    public UnsupportedPacketException(String msg) {
        super(msg);
    }
}
