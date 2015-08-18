/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.packet;

/**
 * {@code ProtocolHeader} describes a class that keeps network protocol header.
 */
public interface ProtocolHeader {
    /**
     * Set a brief description into the given string builder.
     *
     * @param builder  A {@link StringBuilder} instance.
     */
    void setDescription(StringBuilder builder);
}
