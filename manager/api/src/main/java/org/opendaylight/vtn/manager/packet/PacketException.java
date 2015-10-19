/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.packet;

import org.opendaylight.vtn.manager.VTNException;

/**
 * {@code VTNException} is an exception raised when the packet serialization
 * or deserialization fails.
 *
 * <p>
 *   This class is provided only for VTN internal use.
 *   This class may be changed without any notice.
 * </p>
 *
 * @since  Beryllium
 */
public final class PacketException extends VTNException {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = -257294779623090915L;

    /**
     * Construct a new exception.
     *
     * @param msg   The detailed message.
     */
    public PacketException(String msg) {
        super(msg);
    }

    /**
     * Construct a new exception with specifying the cause of error.
     *
     * @param msg    The detailed message.
     * @param cause  The {@link Throwable} object which indicates the cause
     *               of error.
     */
    public PacketException(String msg, Throwable cause) {
        super(msg, cause);
    }
}
